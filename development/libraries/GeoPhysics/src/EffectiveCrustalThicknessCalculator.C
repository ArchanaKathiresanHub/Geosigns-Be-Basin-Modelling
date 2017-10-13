//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "EffectiveCrustalThicknessCalculator.h"
using namespace GeoPhysics;

// std library
#include <memory>

// DataAccess library
#include "Interface/Snapshot.h"
#include "Interface/Grid.h"
#include "Interface/OceanicCrustThicknessHistoryData.h"
using namespace DataAccess::Interface;

// utilities library
#include "LogHandler.h"
#include "ConstantsNumerical.h"
using Utilities::Numerical::IbsNoDataValue;

// The minimum effective crustal thickness is set to 1km
const double EffectiveCrustalThicknessCalculator::s_minimumEffectiveCrustalThickness = 1000;
const bool EffectiveCrustalThicknessCalculator::s_gosthNodes = true;

EffectiveCrustalThicknessCalculator::EffectiveCrustalThicknessCalculator(
   const PaleoFormationPropertyList*        continentalCrustThicknessHistory,
   const TableOceanicCrustThicknessHistory& oceanicCrustThicknessHistory,
   const PolyFunction2DArray&               continentalCrustThicknessPolyfunction,
   const GridMap*                           presentDayBasaltThickness,
   const GridMap*                           crustMeltOnsetMap,
   const double                             initialLithosphericMantleThickness,
   const double                             initialCrustThickness,
   const AbstractValidator&                 validator ):
      m_continentalCrustThicknessHistory  ( continentalCrustThicknessHistory      ),
      m_oceanicCrustThicknessHistory      ( oceanicCrustThicknessHistory          ),
      m_contCrustThicknessPolyfunction    ( continentalCrustThicknessPolyfunction ),
      m_presentDayBasaltThickness         ( presentDayBasaltThickness             ),
      m_crustThicknessMeltOnset           ( crustMeltOnsetMap                     ),
      m_initialLithosphericMantleThickness( initialLithosphericMantleThickness    ),
      m_initialCrustThickness             ( initialCrustThickness ),
      m_validator                         ( validator )
{

   // Detecting algorithm version according to inputs
   if (presentDayBasaltThickness != nullptr or crustMeltOnsetMap != nullptr) {
      LogHandler( LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_STEP ) << "ALC is running with legacy version (v2016.11)";
      m_version = LEGACY;
   }
   else {
      LogHandler( LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_STEP ) << "ALC is running with newest version (v2017.05)";
      m_version = V2017_05;
   }

   // Checking arguments validity
   // a. Continental crust thickness is always needed
   if (m_continentalCrustThicknessHistory == nullptr) {
      throw std::invalid_argument("No continental crustal thickness history was provided to the effective crustal thickness calculator (null pointer)");
   }
   else if (m_continentalCrustThicknessHistory->empty()) {
      throw std::invalid_argument( "No continental crustal thickness history was provided to the effective crustal thickness calculator (empty data vector)" );
   }
   // b. Initial lithospheric mantle thickness must be positive
   else if (m_initialLithosphericMantleThickness < 0.0) {
      throw std::invalid_argument( "The initial lithospheric mantle thickness is negative" );
   }
   // c. Initial crust thickness must be positive
   else if (m_initialCrustThickness < 0.0) {
      throw std::invalid_argument( "The initial crust thickness is negative" );
   }
   // d. The inital crust and lithospheric mantle thicknesses must sum to greater than 0
   else if (m_initialCrustThickness == 0 and m_initialLithosphericMantleThickness == 0) {
      throw std::invalid_argument( "Both the inital crust and the lithospheric mantle thicknesses have a zero thickness" );
   }
   // e. Inputs needed for legacy algorithm (as for v2016.11 release)
   else if (m_version == LEGACY) {
      if (presentDayBasaltThickness == nullptr) {
         throw std::invalid_argument( "No present day basalt thickness history was provided to the effective crustal thickness calculator (null pointer)" );
      }
      else if (crustMeltOnsetMap == nullptr) {
         throw std::invalid_argument( "No crustal thickness at melt onset was provided to the effective crustal thickness calculator (null pointer)" );
      }
   }
   // f. Inputs needed for v2017.05 release algorithm
   else if (m_version == V2017_05) {
      if (oceanicCrustThicknessHistory.data().empty()) {
         throw std::invalid_argument( "No oceanic crustal thickness history was provided to the effective crustal thickness calculator (empty data vector)" );
      }
   }
}

//------------------------------------------------------------//

void EffectiveCrustalThicknessCalculator::compute( PolyFunction2DArray& effectiveCrustThicknessHistory,
                                                   PolyFunction2DArray& oceanicCrustThicknessHistory,
                                                   Local2DArray <double>& endOfRiftEvent ) {

   ///1. Define some variables
   // Inputs
   double presentDayContinentalCrustThicknessValue, continentalCrustThicknessValue, presentDayBasaltThicknessValue, crustThicknessAtMeltOnsetValue;
   // Outputs
   double effectiveCrustalThicknessValue, basaltThicknessValue;
   // Temporary data
   double previousContinentalCrustThicknessValue = 0, previousBasaltThicknessValue = 0, agePrev = 0;
   const GridMap* prevContCrustThicknessMap = nullptr;
   bool basaltStatus = true, onsetStatus = true;

   ///2. Retreive data
   retrieveData();

   ///3. Precompute constant for calculation
   // zero division already checked by the constructor
   double coeff = m_initialCrustThickness / (m_initialLithosphericMantleThickness + m_initialCrustThickness);

   ///4. Compute the effective crustal thickness and associated properties (basalt thickness if needed and end of rift)
   for (auto continentalCrustReverseIter = m_continentalCrustThicknessHistory->rbegin(); continentalCrustReverseIter != m_continentalCrustThicknessHistory->rend(); ++continentalCrustReverseIter) {

      const PaleoFormationProperty* contCrustThicknessInstance = *continentalCrustReverseIter;
      const GridMap* contCrustThicknessMap = contCrustThicknessInstance->getMap( CrustThinningHistoryInstanceThicknessMap );
      if (prevContCrustThicknessMap == nullptr) prevContCrustThicknessMap = contCrustThicknessMap;
      const double age = contCrustThicknessInstance->getSnapshot()->getTime();
      if (agePrev == 0) agePrev = age;

      // If we use the latest version of the ALC then we first need to find
      //    the oceanic crustal thickness corresponding to the current contiental crust thickness
      auto oceanicCrustData = m_oceanicCrustThicknessHistory.data();
      auto oceanicCrustThicknessIt = oceanicCrustData.end();
      if (m_version == V2017_05) {
         oceanicCrustThicknessIt = std::find_if( oceanicCrustData.begin(), oceanicCrustData.end(), [&age]( std::shared_ptr<const OceanicCrustThicknessHistoryData> obj ) {return obj->getAge() == age; } );
         if (oceanicCrustThicknessIt == oceanicCrustData.end()) {
            // since the inputs of the continental crust have been modified by the project handle, this can happen
            // in that casewe just skip this age, it will be linearly interpolated fom the other ages
            continue;
          }
      }
      for (unsigned int i = static_cast<unsigned int>(contCrustThicknessMap->getGrid()->firstI( s_gosthNodes )); i <= static_cast<unsigned int>(contCrustThicknessMap->getGrid()->lastI( s_gosthNodes )); ++i) {

         for (unsigned int j = static_cast<unsigned int>( contCrustThicknessMap->getGrid()->firstJ( s_gosthNodes )); j <= static_cast<unsigned int>(contCrustThicknessMap->getGrid()->lastJ( s_gosthNodes )); ++j) {

            if (m_validator.isValid( i, j )) {

               // Initialise inputs and outputs used by all ALC versions
               basaltThicknessValue           = 0.0;
               effectiveCrustalThicknessValue = 0.0;
               continentalCrustThicknessValue = contCrustThicknessMap->getValue( i, j );
               checkThicknessValue( "Continental crustal thickness", i, j, age, continentalCrustThicknessValue );

               // If there is a crust thickness at melt onset defined, then compute the effective crustal thickness from it (Legacy ALC)
               if (m_version == LEGACY) {
                  // get inputs
                  crustThicknessAtMeltOnsetValue           = m_crustThicknessMeltOnset->getValue  ( i, j );
                  presentDayBasaltThicknessValue           = m_presentDayBasaltThickness->getValue( i, j );
                  previousBasaltThicknessValue             = oceanicCrustThicknessHistory         ( i, j ).GetPoint( agePrev );
                  previousContinentalCrustThicknessValue   = prevContCrustThicknessMap->getValue  ( i, j );
                  presentDayContinentalCrustThicknessValue = m_contCrustThicknessPolyfunction     ( i, j ).F(0.0);

                  checkThicknessValue( "Crustal thickness at melt onset"        , i, j, age, crustThicknessAtMeltOnsetValue           );
                  checkThicknessValue( "Previous basalt thickness thickness"    , i, j, age, previousBasaltThicknessValue             );
                  checkThicknessValue( "Previous continental crustal thickness" , i, j, age, previousContinentalCrustThicknessValue   );
                  checkThicknessValue( "Present day basalt thickness"           , i, j, age, presentDayBasaltThicknessValue           );
                  checkThicknessValue( "Present day continental crust thickness", i, j, age, presentDayContinentalCrustThicknessValue );
                  EffectiveCrustalThicknessCalculator::Output result = calculateBasaltThicknessFromMeltOnset(
                     crustThicknessAtMeltOnsetValue,
                     continentalCrustThicknessValue,
                     presentDayContinentalCrustThicknessValue,
                     previousContinentalCrustThicknessValue,
                     presentDayBasaltThicknessValue,
                     previousBasaltThicknessValue,
                     age );
                  basaltThicknessValue = result.basaltThickness;
                  basaltStatus         = result.basaltStatus;
                  onsetStatus          = result.onsetStatus;
               }
               // If there is no crust thickness at melt onset defined, then look for the basalt crustal thickness history and compute
               // the effective crustal thickness from it (Legacy ALC)
               else if ( m_version == V2017_05 ) {
                  basaltThicknessValue = oceanicCrustThicknessIt->get()->getMap()->getValue( i, j );
               }
               // If this happens, then an implementation is missing
               else {
                  throw std::runtime_error( "Unkown ALC version" );
               }

               // Compute effective crustal thickness
               effectiveCrustalThicknessValue = calculateEffectiveCrustalThickness(continentalCrustThicknessValue, basaltThicknessValue, coeff);

               // Compute end of rift age
               EffectiveCrustalThicknessCalculator::Node node(i,j);
               updateEndOfRift( continentalCrustThicknessValue,
                                previousContinentalCrustThicknessValue,
                                age,
                                node,
                                endOfRiftEvent);

               // Assign results to output
               oceanicCrustThicknessHistory  ( i, j ).AddPoint( age, basaltThicknessValue           );
               effectiveCrustThicknessHistory( i, j ).AddPoint( age, effectiveCrustalThicknessValue );

            }
         }
      }
      agePrev = age;
      prevContCrustThicknessMap = contCrustThicknessMap;
   }

   if (not basaltStatus) {
      LogHandler( LogHandler::WARNING_SEVERITY ) << "Calculated basalt had some non-positive values which have been set to 0m";
   }
   if (not onsetStatus) {
      LogHandler( LogHandler::WARNING_SEVERITY ) << "Present day continental crustal thickness was equal to crustal thickness at melt onset in the parts of the model, "
         << ", the basalt thickness has been set to its previous value (age n-1) in that case";
   }
}

//------------------------------------------------------------//

EffectiveCrustalThicknessCalculator::Output GeoPhysics::EffectiveCrustalThicknessCalculator::calculateBasaltThicknessFromMeltOnset( 
   const double crustThicknessAtMeltOnset,
   const double continentalCrustThickness,
   const double presentDayContinentalCrustThickness,
   const double previousContinentalCrustThickness,
   const double presentDayBasaltThickness,
   const double previousBasaltThickness,
   const double age ) const noexcept
{
   bool basaltStatus = true;
   bool onsetStatus  = true;
   double basaltThickness = 0.0;
   EffectiveCrustalThicknessCalculator::Output result;
   if (m_initialCrustThickness < continentalCrustThickness or crustThicknessAtMeltOnset <= continentalCrustThickness) {
      basaltThickness = 0.0;
   }
   else if (crustThicknessAtMeltOnset > continentalCrustThickness or continentalCrustThickness <= 0) {
      if (continentalCrustThickness < previousContinentalCrustThickness) {

         double coeff = crustThicknessAtMeltOnset - presentDayContinentalCrustThickness;
         if (coeff != 0.0) {
            basaltThickness = presentDayBasaltThickness * ((crustThicknessAtMeltOnset - continentalCrustThickness) / coeff);
         }
         else {
            onsetStatus = false;
            basaltThickness = previousBasaltThickness;
         }

      }
      else {
         // stop generate basalt if continentalCrustThickness reachs or drops below the previous minimun
         basaltThickness = previousBasaltThickness;
      }
   }
   else {
      // we shouldn't be here
      basaltThickness = 0.0;
   }

   if (basaltThickness < 0.0) {
      basaltStatus = false;
      basaltThickness = 0.0;
   }

   result.basaltThickness       = basaltThickness;
   result.basaltStatus          = basaltStatus;
   result.onsetStatus           = onsetStatus;
   return result;
}

//------------------------------------------------------------//

double GeoPhysics::EffectiveCrustalThicknessCalculator::calculateEffectiveCrustalThickness( const double continentalCrustThickness,
                                                                                            const double basaltThickness,
                                                                                            const double coeff ) const noexcept {
   double result = continentalCrustThickness + basaltThickness * coeff;
   if (result < s_minimumEffectiveCrustalThickness) {
      result = s_minimumEffectiveCrustalThickness;
   }
   return result;
}

//------------------------------------------------------------//

void GeoPhysics::EffectiveCrustalThicknessCalculator::updateEndOfRift( const double continentalCrustThickness,
                                                                         const double previousContinentalCrustThickness,
                                                                         const double age,
                                                                         const EffectiveCrustalThicknessCalculator::Node& node,
                                                                         Local2DArray <double>& endOfRiftEvent ) const noexcept{
   if (continentalCrustThickness < previousContinentalCrustThickness and continentalCrustThickness < m_initialCrustThickness) {
      endOfRiftEvent(node.i, node.j) = age;
   }
}

//------------------------------------------------------------//

void GeoPhysics::EffectiveCrustalThicknessCalculator::retrieveData() {
  
   std::for_each( m_continentalCrustThicknessHistory->begin(), m_continentalCrustThicknessHistory->end(), []( const PaleoFormationProperty* obj )
   {
      obj->getMap(CrustThinningHistoryInstanceThicknessMap)->retrieveData( s_gosthNodes );
   } );

   if (m_version == LEGACY) {
      m_presentDayBasaltThickness   ->retrieveData( s_gosthNodes );
      m_crustThicknessMeltOnset     ->retrieveData( s_gosthNodes );
   }
   else if (m_version == V2017_05) {
      auto oceanicCrustData = m_oceanicCrustThicknessHistory.data();
      std::for_each( oceanicCrustData.begin(), oceanicCrustData.end(), []( std::shared_ptr<const OceanicCrustThicknessHistoryData> obj ) { obj->getMap()->retrieveData( s_gosthNodes ); } );
   }
   else {
      throw std::runtime_error( "Unkown ALC version" );
   }
}

//------------------------------------------------------------//

void GeoPhysics::EffectiveCrustalThicknessCalculator::restoreData() {
  
   std::for_each( m_continentalCrustThicknessHistory->begin(), m_continentalCrustThicknessHistory->end(), []( const PaleoFormationProperty* obj )
   {
      obj->getMap(CrustThinningHistoryInstanceThicknessMap)->restoreData(false, s_gosthNodes );
   } );
  
   if (m_version == LEGACY) {
      m_presentDayBasaltThickness   ->restoreData(false, s_gosthNodes );
      m_crustThicknessMeltOnset     ->restoreData(false, s_gosthNodes );
   }
   else if (m_version == V2017_05) {
      auto oceanicCrustData = m_oceanicCrustThicknessHistory.data();
      std::for_each( oceanicCrustData.begin(), oceanicCrustData.end(), []( std::shared_ptr<const OceanicCrustThicknessHistoryData> obj ) { obj->getMap()->restoreData(false, s_gosthNodes ); } );
   }
   else {
      throw std::runtime_error( "Unkown ALC version" );
   }
}

//------------------------------------------------------------//

void GeoPhysics::EffectiveCrustalThicknessCalculator::checkThicknessValue( const char*        thicknessMapName,
                                                                           const unsigned int i,
                                                                           const unsigned int j,
                                                                           const double       age,
                                                                           const double       value ) const {
   if (value < 0.0) {
      std::ostringstream error;
      error << thicknessMapName << " is negative for "
         << " age " << age << "Ma, (" << i << "," << j << ") = " << value << ".";
      throw std::invalid_argument( error.str() );
   }
}


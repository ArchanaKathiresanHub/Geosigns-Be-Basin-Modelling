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
   const double                             initialLithosphericMantleThickness,
   const double                             initialCrustThickness,
   const AbstractValidator&                 validator ):
      m_continentalCrustThicknessHistory  ( continentalCrustThicknessHistory      ),
      m_oceanicCrustThicknessHistory      ( oceanicCrustThicknessHistory          ),
      m_contCrustThicknessPolyfunction    ( continentalCrustThicknessPolyfunction ),
      m_initialLithosphericMantleThickness( initialLithosphericMantleThickness    ),
      m_initialCrustThickness             ( initialCrustThickness ),
      m_validator                         ( validator )
{
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
   // e. Oceanic crustal thickness history must not be empty
   else if (oceanicCrustThicknessHistory.data().empty()) {
      throw std::invalid_argument( "No oceanic crustal thickness history was provided to the effective crustal thickness calculator (empty data vector)" );
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
      oceanicCrustThicknessIt = std::find_if( oceanicCrustData.begin(), oceanicCrustData.end(), [&age]( std::shared_ptr<const OceanicCrustThicknessHistoryData> obj ) {return obj->getAge() == age; } );
      if (oceanicCrustThicknessIt == oceanicCrustData.end()) {
         // since the inputs of the continental crust have been modified by the project handle, this can happen
         // in that casewe just skip this age, it will be linearly interpolated fom the other ages
         continue;
      }
      for (unsigned int i = static_cast<unsigned int>(contCrustThicknessMap->getGrid()->firstI( s_gosthNodes )); i <= static_cast<unsigned int>(contCrustThicknessMap->getGrid()->lastI( s_gosthNodes )); ++i) {

         for (unsigned int j = static_cast<unsigned int>( contCrustThicknessMap->getGrid()->firstJ( s_gosthNodes )); j <= static_cast<unsigned int>(contCrustThicknessMap->getGrid()->lastJ( s_gosthNodes )); ++j) {

            if (m_validator.isValid( i, j )) {

               // Initialise inputs and outputs used by all ALC versions
               basaltThicknessValue           = 0.0;
               effectiveCrustalThicknessValue = 0.0;
               continentalCrustThicknessValue = contCrustThicknessMap->getValue( i, j );
               checkThicknessValue( "Continental crustal thickness", i, j, age, continentalCrustThicknessValue );
               basaltThicknessValue = oceanicCrustThicknessIt->get()->getMap()->getValue( i, j );
               checkThicknessValue( "Oceanic crustal thickness", i, j, age, basaltThicknessValue );

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

   auto oceanicCrustData = m_oceanicCrustThicknessHistory.data();
   std::for_each( oceanicCrustData.begin(), oceanicCrustData.end(), []( std::shared_ptr<const OceanicCrustThicknessHistoryData> obj ) { obj->getMap()->retrieveData( s_gosthNodes ); } );
}

//------------------------------------------------------------//

void GeoPhysics::EffectiveCrustalThicknessCalculator::restoreData() {
  
   std::for_each( m_continentalCrustThicknessHistory->begin(), m_continentalCrustThicknessHistory->end(), []( const PaleoFormationProperty* obj )
   {
      obj->getMap(CrustThinningHistoryInstanceThicknessMap)->restoreData(false, s_gosthNodes );
   } );
  
   auto oceanicCrustData = m_oceanicCrustThicknessHistory.data();
   std::for_each( oceanicCrustData.begin(), oceanicCrustData.end(), []( std::shared_ptr<const OceanicCrustThicknessHistoryData> obj ) { obj->getMap()->restoreData(false, s_gosthNodes ); } );

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


//
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "LegacyBasaltThicknessCalculator.h"

// DataAccess library
#include "Interface/Grid.h"
#include "Interface/GridMap.h"
#include "Interface/ObjectFactory.h"
#include "Interface/PaleoFormationProperty.h"
#include "Interface/Snapshot.h"
using namespace DataAccess::Interface;

// utilities library
#include "LogHandler.h"
#include "ConstantsNumerical.h"

// DataModel library
#include "AbstractValidator.h"
using namespace DataModel;

//std library
#include <sstream>

const bool Prograde::LegacyBasaltThicknessCalculator::s_gosthNodes = true;

Prograde::LegacyBasaltThicknessCalculator::LegacyBasaltThicknessCalculator(
   const AbstractSnapshotVsGridMap&            continentalCrustThicknessHistory,
   const PolyFunction2DArray&                  continentalCrustThicknessPolyfunction,
   const GridMap*                              presentDayBasaltThickness,
   const GridMap*                              crustMeltOnsetMap,
   const double                                initialCrustThickness,
   const DataModel::AbstractValidator&         validator,
   const Grid*                                 referenceGrid,
   const ObjectFactory*                        factory):
      m_contCrustThicknessPolyfunction    ( continentalCrustThicknessPolyfunction ),
      m_continentalCrustThicknessHistory  ( continentalCrustThicknessHistory ),
      m_presentDayBasaltThickness         ( presentDayBasaltThickness ),
      m_crustThicknessMeltOnset           ( crustMeltOnsetMap ),
      m_initialCrustThickness             ( initialCrustThickness ),
      m_validator                         ( validator ),
      m_referenceGrid                     ( referenceGrid ),
      m_factory                           ( factory )
{
   // Checking arguments validity
   // a. Continental crust thickness is always needed
   if (m_continentalCrustThicknessHistory.empty()) {
      throw std::invalid_argument( "No continental crustal thickness history was provided to the effective crustal thickness calculator (empty data vector)" );
   }
   std::for_each(m_continentalCrustThicknessHistory.begin(), m_continentalCrustThicknessHistory.end(), [] (const std::pair<const AbstractSnapshot*,const GridMap*> item)
   {
      if (item.second == nullptr) {
         throw std::invalid_argument("No continental crustal thickness history was provided to the effective crustal thickness calculator (null pointer)");
      }
      if (item.first == nullptr) {
         throw std::invalid_argument("No snapshot was provided to the effective crustal thickness calculator (null pointer)");
      }
      if (item.first->getTime() < 0) {
         throw std::invalid_argument("At least one snapshot of the crustal thickness history has a negative age");
      }
   });
   // b. Initial crust thickness must be positive
   if (m_initialCrustThickness < 0.0) {
      throw std::invalid_argument( "The initial crust thickness is negative" );
   }
   // c. Inputs needed for legacy algorithm (as for v2016.11 release)
   if (presentDayBasaltThickness == nullptr) {
      throw std::invalid_argument( "No present day basalt thickness history was provided to the effective crustal thickness calculator (null pointer)" );
   }
   if (crustMeltOnsetMap == nullptr) {
      throw std::invalid_argument( "No crustal thickness at melt onset was provided to the effective crustal thickness calculator (null pointer)" );
   }
   if (referenceGrid == nullptr) {
      throw std::invalid_argument( "No reference grid was provided to the effective crustal thickness calculator (null pointer)" );
   }
   if (factory == nullptr) {
      throw std::invalid_argument( "No object factory was provided to the effective crustal thickness calculator (null pointer)" );
   }
}

//------------------------------------------------------------//

void Prograde::LegacyBasaltThicknessCalculator::compute( SmartAbstractSnapshotVsSmartGridMap& oceanicCrustThicknessHistory) const
{
   // Temporary data
   double agePrev = 0;
   const GridMap* prevContCrustThicknessMap = nullptr;
   bool basaltStatus = true, onsetStatus = true;

   ///2. Retreive data
   retrieveData();

   ///3. Compute the effective crustal thickness and associated properties (basalt thickness if needed and end of rift)
   std::for_each (m_continentalCrustThicknessHistory.rbegin(), m_continentalCrustThicknessHistory.rend(),
      [this, &agePrev, &prevContCrustThicknessMap, &basaltStatus, &onsetStatus, &oceanicCrustThicknessHistory] (const std::pair<const AbstractSnapshot*,const GridMap*> item)
   {
      const double age = item.first->getTime();
      const GridMap* const contCrustThicknessMap = item.second;
      if (prevContCrustThicknessMap == nullptr) prevContCrustThicknessMap = contCrustThicknessMap;
      if (agePrev == 0) agePrev = age;
      const std::shared_ptr<GridMap> gridMap(const_cast<DataAccess::Interface::ObjectFactory*>(m_factory)->produceGridMap(
         nullptr, 0, m_referenceGrid, Utilities::Numerical::CauldronNoDataValue));
      const std::shared_ptr<const AbstractSnapshot> snapshot(const_cast<DataAccess::Interface::ObjectFactory*>(m_factory)->produceSnapshot(
         nullptr, age));
      const std::shared_ptr<const AbstractSnapshot> snapshotPrev(const_cast<DataAccess::Interface::ObjectFactory*>(m_factory)->produceSnapshot(
         nullptr, agePrev));

      gridMap->retrieveData(s_gosthNodes);

      for (auto i = static_cast<unsigned int>(contCrustThicknessMap->getGrid()->firstI( s_gosthNodes )); i <= static_cast<unsigned int>(contCrustThicknessMap->getGrid()->lastI( s_gosthNodes )); ++i) {

         for (auto j = static_cast<unsigned int>( contCrustThicknessMap->getGrid()->firstJ( s_gosthNodes )); j <= static_cast<unsigned int>(contCrustThicknessMap->getGrid()->lastJ( s_gosthNodes )); ++j) {

            if (m_validator.isValid( i, j )) {
               const double continentalCrustThicknessValue = contCrustThicknessMap->getValue( i, j );
               checkThicknessValue( "Continental crustal thickness", i, j, age, continentalCrustThicknessValue );

               // Get inputs
               const double crustThicknessAtMeltOnsetValue           = m_crustThicknessMeltOnset->getValue  ( i, j );
               const double presentDayBasaltThicknessValue           = m_presentDayBasaltThickness->getValue( i, j );
               const double previousContinentalCrustThicknessValue   = prevContCrustThicknessMap->getValue  ( i, j );
               const double presentDayContinentalCrustThicknessValue = m_contCrustThicknessPolyfunction     ( i, j ).F(0.0);
               double previousBasaltThicknessValue;
               const auto it = oceanicCrustThicknessHistory.find(snapshotPrev);
               if(it != oceanicCrustThicknessHistory.end()){
                  previousBasaltThicknessValue = oceanicCrustThicknessHistory[snapshotPrev]->getValue( i, j );
               }
               else {
                  previousBasaltThicknessValue = 0;
               }

               checkThicknessValue( "Crustal thickness at melt onset"        , i, j, age, crustThicknessAtMeltOnsetValue           );
               checkThicknessValue( "Previous basalt thickness thickness"    , i, j, age, previousBasaltThicknessValue             );
               checkThicknessValue( "Previous continental crustal thickness" , i, j, age, previousContinentalCrustThicknessValue   );
               checkThicknessValue( "Present day basalt thickness"           , i, j, age, presentDayBasaltThicknessValue           );
               checkThicknessValue( "Present day continental crust thickness", i, j, age, presentDayContinentalCrustThicknessValue );

               // Compute
               const Output result = calculateBasaltThicknessFromMeltOnset(
                  crustThicknessAtMeltOnsetValue,
                  continentalCrustThicknessValue,
                  presentDayContinentalCrustThicknessValue,
                  previousContinentalCrustThicknessValue,
                  presentDayBasaltThicknessValue,
                  previousBasaltThicknessValue);
               const double basaltThicknessValue = result.basaltThickness;
               basaltStatus         = result.basaltStatus;
               onsetStatus          = result.onsetStatus;

               // Assign results to output
               const bool setStatus = gridMap->setValue( i, j, basaltThicknessValue );
               if ( not setStatus  ) {
                  std::ostringstream error;
                  error << "Cannot set basalt thickness value to " << basaltThicknessValue << " at point [" << i << "," << j << "]";
                  throw std::runtime_error( error.str() );
               };
            }
         }
      }
      gridMap->restoreData(s_gosthNodes);

      oceanicCrustThicknessHistory.insert( std::pair<const std::shared_ptr<const AbstractSnapshot>,const std::shared_ptr<const GridMap>>(snapshot, gridMap) );
      agePrev = age;
      prevContCrustThicknessMap = contCrustThicknessMap;
   });

   if (not basaltStatus) {
      LogHandler( LogHandler::WARNING_SEVERITY ) << "Calculated basalt had some non-positive values which have been set to 0m";
   }
   if (not onsetStatus) {
      LogHandler( LogHandler::WARNING_SEVERITY ) << "Present day continental crustal thickness was equal to crustal thickness at melt onset in the parts of the model, "
         << ", the basalt thickness has been set to its previous value (age n-1) in that case";
   }
}

//------------------------------------------------------------//

Prograde::LegacyBasaltThicknessCalculator::Output Prograde::LegacyBasaltThicknessCalculator::calculateBasaltThicknessFromMeltOnset(
   const double crustThicknessAtMeltOnset,
   const double continentalCrustThickness,
   const double presentDayContinentalCrustThickness,
   const double previousContinentalCrustThickness,
   const double presentDayBasaltThickness,
   const double previousBasaltThickness) const noexcept
{
   bool basaltStatus = true;
   bool onsetStatus  = true;
   double basaltThickness = 0;
   Output result{};
   if (m_initialCrustThickness < continentalCrustThickness or crustThicknessAtMeltOnset <= continentalCrustThickness) {
      basaltThickness = 0.0;
   }
   else if (crustThicknessAtMeltOnset > continentalCrustThickness or continentalCrustThickness <= 0) {
      if (continentalCrustThickness < previousContinentalCrustThickness) {

         const double coeff = crustThicknessAtMeltOnset - presentDayContinentalCrustThickness;
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

void Prograde::LegacyBasaltThicknessCalculator::retrieveData() const
{
   std::for_each( m_continentalCrustThicknessHistory.begin(), m_continentalCrustThicknessHistory.end(), []( const std::pair<const AbstractSnapshot* const,const GridMap* const> item )
   {
      item.second->retrieveData( s_gosthNodes );
   } );

   m_presentDayBasaltThickness ->retrieveData( s_gosthNodes );
   m_crustThicknessMeltOnset   ->retrieveData( s_gosthNodes );
}

//------------------------------------------------------------//

void Prograde::LegacyBasaltThicknessCalculator::restoreData() const
{
   std::for_each( m_continentalCrustThicknessHistory.begin(), m_continentalCrustThicknessHistory.end(), []( const std::pair<const AbstractSnapshot* const,const GridMap* const> item )
   {
      item.second->restoreData(false, s_gosthNodes );
   } );

   m_presentDayBasaltThickness ->restoreData(false, s_gosthNodes );
   m_crustThicknessMeltOnset   ->restoreData(false, s_gosthNodes );
}

//------------------------------------------------------------//

void Prograde::LegacyBasaltThicknessCalculator::checkThicknessValue( const char*        thicknessMapName,
                                                                     const unsigned int i,
                                                                     const unsigned int j,
                                                                     const double       age,
                                                                     const double       value )
{
   if (value < 0.0) {
      std::ostringstream error;
      error << thicknessMapName << " is negative for "
         << " age " << age << "Ma, (" << i << "," << j << ") = " << value << ".";
      throw std::invalid_argument( error.str() );
   }
}


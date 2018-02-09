//
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include "Interface/Validator.h"

//DataAccess
#include "Interface/ProjectHandle.h"
#include "Interface/GridMap.h"
#include "Interface/Formation.h"
#include "Interface/MantleFormation.h"
#include "Interface/CrustFormation.h"
#include "Interface/PropertyValue.h"
#include "Interface/PaleoFormationProperty.h"
#include "Interface/PaleoSurfaceProperty.h"
#include "Interface/SourceRock.h"
#include "Interface/Reservoir.h"
#include "Interface/AllochthonousLithology.h"
#include "Interface/AllochthonousLithologyDistribution.h"
#include "Interface/MobileLayer.h"
#include "Interface/OceanicCrustThicknessHistoryData.h"

//----------------------------------------------------------------------------

bool DataAccess::Interface::Validator::isValid( const unsigned int i, const unsigned int j ) const{
   return m_validNodes( i, j );
}

//----------------------------------------------------------------------------

void DataAccess::Interface::Validator::addUndefinedAreas( const GridMap* theMap ) {

   if (theMap == nullptr) {
      return;
   }

   const bool dataIsRetrieved = theMap->retrieved();

   if (not dataIsRetrieved) {
      theMap->retrieveGhostedData();
   }

   for (unsigned int i = m_validNodes.first( 0 ); i <= m_validNodes.last( 0 ); ++i) {

      for (unsigned int j = m_validNodes.first( 1 ); j <= m_validNodes.last( 1 ); ++j) {

         if (theMap->getValue( i, j ) == theMap->getUndefinedValue()) {
            m_validNodes( i, j ) = false;
         }

      }

   }

   if (not dataIsRetrieved) {
      // If the data was not retrived then restore the map back to its original state.
      theMap->restoreData( false, true );
   }

}


//------------------------------------------------------------//

void DataAccess::Interface::Validator::filterValidNodesByValidElements() {
   /// An active element is a 4-points element with valid nodes ONLY.
   /// It is identified by its leftmost-bottomost node, the other nodes
   /// can be accessed as shown in the following scheme.
   ///
   /// (i+1,j)  (i+1,j+1)
   ///   O---------O
   ///   |         |
   ///   |         |
   ///   |         |
   ///   O---------O
   /// (i,j)     (i,j+1)
   ///
   /// NB: This is a temporary local struct
   struct ActiveElem {
      ActiveElem() : m_i( 0 ), m_j( 0 ) {}
      ActiveElem( const int i, const int j ) : m_i( i ), m_j( j ) {}
      const int m_i, m_j; // leftmost-bottommost element node indeces
   };

   const Grid * grid = m_projectHandle.getActivityOutputGrid();
   const bool useGhosts = true;
   const unsigned int firstIgh = grid->firstI( useGhosts );
   const unsigned int lastIgh = grid->lastI( useGhosts );
   const unsigned int firstJgh = grid->firstJ( useGhosts );
   const unsigned int lastJgh = grid->lastJ( useGhosts );

   // Create vector of all local active elements (including ghost nodes)
   // The criteria is that all the 4 nodes belonging to the element must be valid
   unsigned int activeCounter = 0;
   std::vector<ActiveElem> activeElemVec;
   activeElemVec.reserve( (lastIgh - firstIgh - 1) * (lastJgh - firstJgh - 1) );
   for (unsigned int i = firstIgh; i < lastIgh; ++i) {
      for (unsigned int j = firstJgh; j < lastJgh; ++j) {
         if (m_validNodes( i, j ) and m_validNodes( i + 1, j ) and m_validNodes( i, j + 1 ) and m_validNodes( i + 1, j + 1 )) {
            activeElemVec.emplace_back( i, j );
            ++activeCounter;
         }
      }
   }
   activeElemVec.resize( activeCounter );

   // Now that we have stored all the information about the active elements we can
   // reset the valid node 2D array to false e fill it in again using the above information
   m_validNodes.fill( false );
   for (unsigned int elemIdx = 0; elemIdx < activeCounter; ++elemIdx) {
      const ActiveElem & elem = activeElemVec[elemIdx];
      m_validNodes( elem.m_i, elem.m_j ) = true;
      m_validNodes( elem.m_i + 1, elem.m_j ) = true;
      m_validNodes( elem.m_i, elem.m_j + 1 ) = true;
      m_validNodes( elem.m_i + 1, elem.m_j + 1 ) = true;
   }

   // Now the information about the ghost nodes has to be communicated to the other processors (if any)
   // Create a GridMap for the nodes validity, initialised to 0 (not valid)
   GridMap * validGridMap = m_projectHandle.getFactory()->produceGridMap( nullptr, 0, grid, 0.0 );
   validGridMap->retrieveData( useGhosts );
   for (unsigned int i = firstIgh; i <= lastIgh; ++i) {
      for (unsigned int j = firstJgh; j <= lastJgh; ++j) {
         if (m_validNodes( i, j )) validGridMap->setValue( i, j, 1.0 );
         else validGridMap->setValue( i, j, 0.0 ); // Temporary workaround for a bug in DistributedGridMap::restoreData
      }
   }
   validGridMap->restoreData( true, useGhosts );

   // Check if the current local grid has ghost nodes in the 4 directions
   // If in one direction there are no ghost nodes it means that it's a real boundary
   const bool hasLeftGhost   = firstIgh != static_cast<unsigned int>(grid->firstI( !useGhosts ));
   const bool hasRightGhost  = lastIgh  != static_cast<unsigned int>(grid->lastI( !useGhosts ));
   const bool hasBottomGhost = firstJgh != static_cast<unsigned int>(grid->firstJ( !useGhosts ));
   const bool hasTopGhost    = lastJgh  != static_cast<unsigned int>(grid->lastJ( !useGhosts ));

   // Now loop over ghost nodes only:
   // The ghost node for the local grid will be set valid only if the gridMap has
   // a value greater than zero, this means that at least one processor
   // has set to 1 (eg valid) its local node
   validGridMap->retrieveData( useGhosts );
   if (hasLeftGhost) {
      for (unsigned int j = firstJgh; j <= lastJgh; ++j) {
         m_validNodes( firstIgh, j ) = (validGridMap->getValue( firstIgh, j ) > 0.0);
      }
   }
   if (hasRightGhost) {
      for (unsigned int j = firstJgh; j <= lastJgh; ++j) {
         m_validNodes( lastIgh, j ) = (validGridMap->getValue( lastIgh, j ) > 0.0);
      }
   }
   if (hasBottomGhost) {
      for (unsigned int i = firstIgh; i <= lastIgh; ++i) {
         m_validNodes( i, firstJgh ) = (validGridMap->getValue( i, firstJgh ) > 0.0);
      }
   }
   if (hasTopGhost) {
      for (unsigned int i = firstIgh; i <= lastIgh; ++i) {
         m_validNodes( i, lastJgh ) = (validGridMap->getValue( i, lastJgh ) > 0.0);
      }
   }
   validGridMap->restoreData( false, useGhosts );

   delete validGridMap;
}

//------------------------------------------------------------//

bool DataAccess::Interface::Validator::initialiseValidNodes( const bool readSizeFromVolumeData ) {

   m_validNodes.reallocate( m_projectHandle.getActivityOutputGrid() );
   m_validNodes.fill( true );

   // Add sediment-formation undefined areas:
   //
   //     o depth/thickness maps;
   //     o lithology maps;
   //     o mobile-layer maps;
   //     o allochthonous-lithology distribution maps;
   //     o all reservoir maps, if reservoir layer;
   const auto sedimentFormationList = m_projectHandle.getFormations( nullptr, false );
   for (const auto formation : *sedimentFormationList) {

      if (formation->kind() == SEDIMENT_FORMATION) {
         addFormationUndefinedAreas( formation );
      }

   }
   delete sedimentFormationList;

   // Add crust-formation undefined areas.
   addCrustUndefinedAreas( m_projectHandle.getCrustFormation() );

   // Add mantle-formation undefined areas.
   addMantleUndefinedAreas( m_projectHandle.getMantleFormation() );

   // Add undefined areas from surface-depth maps.
   PaleoPropertyList * surfaceDepthMaps = m_projectHandle.getSurfaceDepthHistory();
   for (PaleoPropertyList::const_iterator surfaceDepthIter = surfaceDepthMaps->begin(); surfaceDepthIter != surfaceDepthMaps->end(); ++surfaceDepthIter) {
      const auto surfaceDepthHistoryMap = dynamic_cast<const GridMap *>((*surfaceDepthIter)->getMap( SurfaceDepthHistoryInstanceMap ));
      addUndefinedAreas( surfaceDepthHistoryMap );
      if (surfaceDepthHistoryMap) surfaceDepthHistoryMap->release();
   }
   delete surfaceDepthMaps;


   // Add undefined areas from surface-temperature maps.
   PaleoPropertyList * surfaceTemperatureMaps( m_projectHandle.getSurfaceTemperatureHistory() );
   for (PaleoPropertyList::const_iterator surfaceTemperatureIter = surfaceTemperatureMaps->begin(); surfaceTemperatureIter != surfaceTemperatureMaps->end(); ++surfaceTemperatureIter) {
      const auto surfaceTemperatureHistoryMap = dynamic_cast<const GridMap *>((*surfaceTemperatureIter)->getMap( SurfaceTemperatureHistoryInstanceMap ));
      addUndefinedAreas( surfaceTemperatureHistoryMap );
      if (surfaceTemperatureHistoryMap) surfaceTemperatureHistoryMap->release();
   }
   delete surfaceTemperatureMaps;


   if (readSizeFromVolumeData) {
      // Since the input depends on some results, these need to be
      // taken into account when setting the valid node array.
      // And, since the number of element depends on the results volume-file
      // there must be some results already.

      // Any property could be used here that is always output, Ves is always output.
      const Property* vesProperty = m_projectHandle.findProperty( "Ves" );

      // Any sediment-formation will do, so use the first on the list.
      const auto formationList = m_projectHandle.getFormations( nullptr, true );

      PropertyValueList* vesValueList = m_projectHandle.getPropertyValues( FORMATION,
         vesProperty,
         m_projectHandle.findSnapshot( 0.0, MAJOR ),
         nullptr,
         *formationList->begin(),
         nullptr,
         VOLUME );

      assert( vesValueList->size() == 1 );

      const PropertyValue* ves = *vesValueList->begin();


      if (ves != nullptr and ves->getGridMap() != nullptr) {
         const GridMap* vesGridMap = ves->getGridMap();
         if (vesGridMap) {
            addUndefinedAreas( dynamic_cast<const GridMap*>(ves->getGridMap()) );
            vesGridMap->release();
         }
      }

      delete vesValueList;

   }

   filterValidNodesByValidElements();

   return true;
}

//------------------------------------------------------------//

void DataAccess::Interface::Validator::addCrustUndefinedAreas( const CrustFormation* crust ) {

   if (crust != nullptr) {
      PaleoFormationPropertyList* thicknesses = crust->getPaleoThicknessHistory();

      for (PaleoFormationPropertyList::const_iterator thicknessIter = thicknesses->begin(); thicknessIter != thicknesses->end(); ++thicknessIter) {
         addUndefinedAreas( dynamic_cast<const GridMap*>((*thicknessIter)->getMap( CrustThinningHistoryInstanceThicknessMap )) );
      }

      if (m_projectHandle.getBottomBoundaryConditions() == ADVANCED_LITHOSPHERE_CALCULATOR) {
         auto oceaData = m_projectHandle.getTableOceanicCrustThicknessHistory().data();
         std::for_each( oceaData.begin(), oceaData.end(), [&]( std::shared_ptr<const OceanicCrustThicknessHistoryData> oceanicCrust )
         {
            addUndefinedAreas( oceanicCrust->getMap() );
         } );
      }
      addUndefinedAreas( dynamic_cast<const GridMap*>(crust->getCrustHeatProductionMap()) );

      delete thicknesses;
   }

}

//------------------------------------------------------------//

void DataAccess::Interface::Validator::addMantleUndefinedAreas( const MantleFormation* mantle ) {

   if (mantle != nullptr) {
      PaleoFormationPropertyList* thicknesses = mantle->getPaleoThicknessHistory();
      if (thicknesses != nullptr) {
         for (PaleoFormationPropertyList::const_iterator thicknessIter = thicknesses->begin(); thicknessIter != thicknesses->end(); ++thicknessIter) {
            addUndefinedAreas( dynamic_cast<const GridMap*>((*thicknessIter)->getMap( MantleThicknessHistoryInstanceThicknessMap )) );
         }
         delete thicknesses;
      }

      // Add undefined areas from heat flow maps.
      // Not explicitly dependent on the mantle.
      PaleoSurfacePropertyList* heatFlowMaps = m_projectHandle.getHeatFlowHistory();

      for (PaleoSurfacePropertyList::const_iterator heatFlowIter = heatFlowMaps->begin(); heatFlowIter != heatFlowMaps->end(); ++heatFlowIter) {
         addUndefinedAreas( dynamic_cast<const GridMap*>((*heatFlowIter)->getMap( HeatFlowHistoryInstanceHeatFlowMap )) );
      }
      delete heatFlowMaps;
   }

}

//------------------------------------------------------------//

void DataAccess::Interface::Validator::addFormationUndefinedAreas( const Formation* formation ) {

   if (formation != nullptr and formation->kind() == SEDIMENT_FORMATION) {

      const auto lithoType1PercentageMap = dynamic_cast<const GridMap*>(formation->getLithoType1PercentageMap());
      const auto lithoType2PercentageMap = dynamic_cast<const GridMap*>(formation->getLithoType2PercentageMap());
      const auto lithoType3PercentageMap = dynamic_cast<const GridMap*>(formation->getLithoType3PercentageMap());
      addUndefinedAreas( lithoType1PercentageMap );
      addUndefinedAreas( lithoType2PercentageMap );
      addUndefinedAreas( lithoType3PercentageMap );


      const auto inputThicknessMap = dynamic_cast<const GridMap*> (formation->getInputThicknessMap());
      addUndefinedAreas( inputThicknessMap );

      if (formation->isSourceRock()) {
         const auto sourceRock1 = dynamic_cast<const GridMap*>(formation->getSourceRock1()->getMap( TocIni ));
         addUndefinedAreas( sourceRock1 );
         if (sourceRock1) sourceRock1->release();
      }

      if (formation->isMobileLayer()) {
         MobileLayerList* mobileLayers( formation->getMobileLayers() );
         for (MobileLayerList::const_iterator mobIt = mobileLayers->begin(); mobIt != mobileLayers->end(); ++mobIt) {
            const auto mobLayerThickness = dynamic_cast<const GridMap*>((*mobIt)->getMap( MobileLayerThicknessMap ));
            addUndefinedAreas( mobLayerThickness );
            if (mobLayerThickness) mobLayerThickness->release();
         }
         delete mobileLayers;
      }

      if (formation->hasAllochthonousLithology()) {
         AllochthonousLithologyDistributionList * allochthonousDistributions( formation->getAllochthonousLithology()->getAllochthonousLithologyDistributions() );
         for (AllochthonousLithologyDistributionList::const_iterator allochIt = allochthonousDistributions->begin(); allochIt != allochthonousDistributions->end(); ++allochIt) {
            const auto allochthonousLithologyDistributionMap = dynamic_cast<const GridMap*> ((*allochIt)->getMap( AllochthonousLithologyDistributionMap ));
            addUndefinedAreas( allochthonousLithologyDistributionMap );
            if (allochthonousLithologyDistributionMap) allochthonousLithologyDistributionMap->release();
         }
         delete allochthonousDistributions;
      }

      ReservoirList * reservoirs( formation->getReservoirs() );

      for (ReservoirList::const_iterator resIt = reservoirs->begin(); resIt != reservoirs->end(); ++resIt) {
         const auto netToGrossMap = dynamic_cast<const GridMap*>((*resIt)->getMap( NetToGross ));
         addUndefinedAreas( netToGrossMap );
         if (netToGrossMap) netToGrossMap->release();
      }
      delete reservoirs;


      if (lithoType1PercentageMap) lithoType1PercentageMap->release();
      if (lithoType2PercentageMap) lithoType2PercentageMap->release();
      if (lithoType3PercentageMap) lithoType3PercentageMap->release();
   }

}
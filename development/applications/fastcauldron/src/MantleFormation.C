//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "MantleFormation.h"

#include <cmath>

#include "FastcauldronSimulator.h"
#include "propinterface.h"

// DAL
#include "Interface/Interface.h"
#include "Interface/PaleoFormationProperty.h"
#include "Interface/GridMap.h"
#include "Interface/Surface.h"
#include "Interface/BasementSurface.h"
#include "Interface/Snapshot.h"

#include "PetscBlockVector.h"

MantleFormation::MantleFormation ( Interface::ProjectHandle * projectHandle,
                                   database::Record *              record ) :
   DataAccess::Interface::Formation ( projectHandle, record ),
   GeoPhysics::Formation ( projectHandle, record ),
   LayerProps ( projectHandle, record ),
   DataAccess::Interface::BasementFormation ( projectHandle, record, Interface::MantleFormationName, projectHandle->getMantleLithoName() ),
   DataAccess::Interface::MantleFormation ( projectHandle, record ),
   GeoPhysics::GeoPhysicsMantleFormation ( projectHandle, record ) {

   fluid = 0;

   depoage = -1;
   TopSurface_DepoSeq = -2;
   Layer_Depo_Seq_Nb = -2;
   BottSurface_DepoSeq = -3;
   Calculate_Chemical_Compaction = false;

   UpliftedOrigMantleDepth = NULL;
   LithosphereThicknessMod = NULL;

   initialiseBasementVecs();
   setBasementVectorList();
}

//------------------------------------------------------------//
MantleFormation::~MantleFormation () {
   Destroy_Petsc_Vector ( LithosphereThicknessMod );
   Destroy_Petsc_Vector ( UpliftedOrigMantleDepth );
}

//------------------------------------------------------------//
void MantleFormation::cleanVectors() {

   if(((GeoPhysics::ProjectHandle*)(GeoPhysics::Formation::m_projectHandle))->isALC() ) {
      previousBasaltMap = BasaltMap;
      BasaltMap.fill( false );
   }
}

//------------------------------------------------------------//
void MantleFormation::initialise () {

   layername             = Interface::MantleFormation::getName ();
   m_lithoMixModel       = Interface::MantleFormation::getMixModelStr ();
   m_presentDayThickness = Interface::MantleFormation::getInputThicknessMap ();
   depthGridMap          = Interface::MantleFormation::getTopSurface ()->getInputDepthMap ();

   setLayerElements ();
}

//------------------------------------------------------------//
void MantleFormation::setMaximumThicknessValue ( const double newThickness ) {
   m_maximumDepositedThickness = newThickness;
}

//------------------------------------------------------------//
const CompoundLithology* MantleFormation::getBasaltLithology(const int iPosition, const int jPosition) const {

   return m_basaltLithology(iPosition, jPosition);
}

//------------------------------------------------------------//
const CompoundLithology*  MantleFormation::getLithology(const int iPosition, const int jPosition, const int kPosition ) const {
   if( FastcauldronSimulator::getInstance ().getCauldron ()->isALC() ) {
      if( BasaltMap( iPosition, jPosition, kPosition ) ) {
         return getBasaltLithology( iPosition, jPosition );
      } else {
         return LayerProps::getLithology( iPosition, jPosition );
      }
   } else {
      return LayerProps::getLithology( iPosition, jPosition );
   }
}

//------------------------------------------------------------//
bool MantleFormation::setLithologiesFromStratTable () {

   bool createdLithologies = true;

   if( GeoPhysics::GeoPhysicsMantleFormation::setLithologiesFromStratTable () == false ) {
      return false;
   }
   if(((GeoPhysics::ProjectHandle*)(GeoPhysics::Formation::m_projectHandle))->isALC() ) {

      m_basaltLithology.allocate ( GeoPhysics::Formation::m_projectHandle->getActivityOutputGrid ());

      CompoundLithologyComposition lc ( DataAccess::Interface::ALCBasalt, "",  "",
                                        100.0, 0.0, 0.0,
                                        DataAccess::Interface::MantleFormation::getMixModelStr (),
                                        DataAccess::Interface::MantleFormation::getLayeringIndex());

      lc.setThermalModel( m_projectHandle->getMantlePropertyModel() );

      CompoundLithology* pMixedLitho = ((GeoPhysics::ProjectHandle*)(GeoPhysics::Formation::m_projectHandle))->getLithologyManager ().getCompoundLithology ( lc );
      createdLithologies = pMixedLitho != 0;
      m_basaltLithology.fillWithLithology ( pMixedLitho );
      // if( dynamic_cast<GeoPhysics::ProjectHandle*>(m_projectHandle)->isALC() && m_projectHandle->getRank() == 0 ) {
      //    cout << "Mantle basalt property model = " << pMixedLitho->getThermalModel() << endl;
      // }
   }
   return createdLithologies;

}

//------------------------------------------------------------//
const CompoundLithology* MantleFormation::getLithology( const double aTime, const int iPosition, const int jPosition, const double aOffset ) {
   // If offset from the top of the Mantle (aOffset) goes above the bottom of Basalt, then we are in Basalt
   const AppCtx* aBasinModel =  FastcauldronSimulator::getInstance ().getCauldron ();

   if( aBasinModel->isALC() ) { 
      double partOfBasaltInMantle = ((GeoPhysics::ProjectHandle*)(GeoPhysics::Formation::m_projectHandle))->getBasaltThickness( iPosition, jPosition, aTime ) +
         ((GeoPhysics::ProjectHandle*)(GeoPhysics::Formation::m_projectHandle))->getContCrustThickness( iPosition, jPosition, aTime) -
         ((GeoPhysics::ProjectHandle*)(GeoPhysics::Formation::m_projectHandle))->getCrustThickness( iPosition, jPosition, aTime );
      if(partOfBasaltInMantle != 0.0) {
         if( aOffset <= partOfBasaltInMantle ) {
            isBasaltLayer = true;
            return getBasaltLithology(iPosition, jPosition);
         } else {
            isBasaltLayer = false;
            return LayerProps::getLithology(iPosition, jPosition);
         }
      } else {
         isBasaltLayer = false;
         return LayerProps::getLithology(iPosition, jPosition);
      }
   } else {
      isBasaltLayer = false;
      return  LayerProps::getLithology(iPosition, jPosition);
   }
}

//------------------------------------------------------------//
void MantleFormation::allocateBasementVecs( ) {

   const AppCtx* basinModel =  FastcauldronSimulator::getInstance ().getCauldron ();

   if( basinModel->isALC()) {
      assert(NULL == UpliftedOrigMantleDepth);
      createCount++;
      DMCreateGlobalVector( * basinModel->mapDA, &UpliftedOrigMantleDepth );

      assert( NULL == LithosphereThicknessMod );
      createCount++;
      DMCreateGlobalVector( * basinModel->mapDA, &LithosphereThicknessMod );

      if ( BasaltMap.isNull ()) {
         BasaltMap.create ( layerDA );
      }
      if ( previousBasaltMap.isNull ()) {
         previousBasaltMap.create ( layerDA );
         previousBasaltMap.fill( false );
      }
  }
}

//------------------------------------------------------------//
void MantleFormation::initialiseBasementVecs() {
   UpliftedOrigMantleDepth = NULL;
   LithosphereThicknessMod = NULL;
}

//------------------------------------------------------------//
void MantleFormation::reInitialiseBasementVecs() {
   Destroy_Petsc_Vector ( UpliftedOrigMantleDepth );
   Destroy_Petsc_Vector ( LithosphereThicknessMod );
}

//------------------------------------------------------------//

void MantleFormation::setBasementVectorList() {
   vectorList.VecArray[ALC_ORIGINAL_MANTLE] = &UpliftedOrigMantleDepth; UpliftedOrigMantleDepth = NULL;
   vectorList.VecArray[ALC_MAX_MANTLE_DEPTH]= &LithosphereThicknessMod; LithosphereThicknessMod = NULL;
}


//------------------------------------------------------------//

void MantleFormation::setLayerElementActivity ( const double age ) {

   const FastcauldronSimulator& fastcauldron = FastcauldronSimulator::getInstance ();
   const MapElementArray&       mapElements  = fastcauldron.getMapElementArray ();

   unsigned int i;
   unsigned int j;
   unsigned int k;

   bool activeSegment1;
   bool activeSegment2;
   bool activeSegment3;
   bool activeSegment4;

   PETSC_3D_Array depth ( layerDA, Current_Properties ( Basin_Modelling::Depth ), INSERT_VALUES, true );

   for ( i = mapElements.firstI ( true ); i <= mapElements.lastI ( true ); ++i ) {

      for ( j = mapElements.firstJ ( true ); j <= mapElements.lastJ ( true ); ++j ) {
         const MapElement& mapElement = mapElements ( i, j );

         if ( mapElement.isValid () and isActive ()) {

            for ( k = 0; k < getMaximumNumberOfElements (); ++k ) {

               LayerElement& volumeElement = getLayerElement ( i, j, k );

               if ( mapElement.isOnProcessor ()) {

                  activeSegment1 = std::fabs ( depth ( k,     mapElement.getNodeJPosition ( 0 ), mapElement.getNodeIPosition ( 0 )) -
                                               depth ( k + 1, mapElement.getNodeJPosition ( 0 ), mapElement.getNodeIPosition ( 0 ))) > DepositingThicknessTolerance;

                  activeSegment2 = std::fabs ( depth ( k,     mapElement.getNodeJPosition ( 1 ), mapElement.getNodeIPosition ( 1 )) -
                                               depth ( k + 1, mapElement.getNodeJPosition ( 1 ), mapElement.getNodeIPosition ( 1 ))) > DepositingThicknessTolerance;

                  activeSegment3 = std::fabs ( depth ( k,     mapElement.getNodeJPosition ( 2 ), mapElement.getNodeIPosition ( 2 )) -
                                               depth ( k + 1, mapElement.getNodeJPosition ( 2 ), mapElement.getNodeIPosition ( 2 ))) > DepositingThicknessTolerance;

                  activeSegment4 = std::fabs ( depth ( k,     mapElement.getNodeJPosition ( 3 ), mapElement.getNodeIPosition ( 3 )) -
                                               depth ( k + 1, mapElement.getNodeJPosition ( 3 ), mapElement.getNodeIPosition ( 3 ))) > DepositingThicknessTolerance;

                  // if any segment is active then the element is active.
                  if ( activeSegment1 or activeSegment2 or activeSegment3 or activeSegment4 ) {
                     volumeElement.setIsActive ( true );

                     // Set face activity.
                     volumeElement.setIsActiveBoundary ( VolumeData::ShallowFace, true );
                     volumeElement.setIsActiveBoundary ( VolumeData::DeepFace, true );
                     volumeElement.setIsActiveBoundary ( VolumeData::Front, activeSegment1 or activeSegment2 );
                     volumeElement.setIsActiveBoundary ( VolumeData::Right, activeSegment2 or activeSegment3 );
                     volumeElement.setIsActiveBoundary ( VolumeData::Back,  activeSegment3 or activeSegment4 );
                     volumeElement.setIsActiveBoundary ( VolumeData::Left,  activeSegment4 or activeSegment1 );

                  } else {
                     volumeElement.setIsActive ( false );
                  }

               } else {

                  // Here we need only check to see if the face is active.
                  // If the face is active then the element must be active.
                  // If the face is not active then there can be no transport between
                  // the two elements.

                  if ( i > mapElements.lastI ( false )) {

                     if ( NumericFunctions::inRange<unsigned int> ( j, mapElements.firstJ ( false ), mapElements.lastJ ( false ))) {

                        activeSegment1 = std::fabs ( depth ( k, j,     i ) - depth ( k + 1, j,     i )) > DepositingThicknessTolerance;
                        activeSegment2 = std::fabs ( depth ( k, j + 1, i ) - depth ( k + 1, j + 1, i )) > DepositingThicknessTolerance;

                        volumeElement.setIsActive ( activeSegment1 or activeSegment2 );
                        volumeElement.setIsActiveBoundary ( VolumeData::GAMMA_5, activeSegment1 or activeSegment2 );
                     }

                  } else if ( mapElements.firstI ( false ) != 0 and i < mapElements.firstI ( false )) {

                     if ( NumericFunctions::inRange<unsigned int> ( j, mapElements.firstJ ( false ), mapElements.lastJ ( false ))) {

                        activeSegment1 = std::fabs ( depth ( k, j,     i ) - depth ( k + 1, j,     i )) > DepositingThicknessTolerance;
                        activeSegment2 = std::fabs ( depth ( k, j + 1, i ) - depth ( k + 1, j + 1, i )) > DepositingThicknessTolerance;

                        volumeElement.setIsActive ( activeSegment1 or activeSegment2 );
                        volumeElement.setIsActiveBoundary ( VolumeData::GAMMA_3, activeSegment1 or activeSegment2 );
                     }

                  }

                  if ( j > mapElements.lastJ ( false )) {

                     if ( NumericFunctions::inRange<unsigned int> ( i, mapElements.firstI ( false ), mapElements.lastI ( false ))) {

                        activeSegment1 = std::fabs ( depth ( k, j, i     ) - depth ( k + 1, j, i     )) > DepositingThicknessTolerance;
                        activeSegment2 = std::fabs ( depth ( k, j, i + 1 ) - depth ( k + 1, j, i + 1 )) > DepositingThicknessTolerance;

                        volumeElement.setIsActive ( activeSegment1 or activeSegment2 );
                        volumeElement.setIsActiveBoundary ( VolumeData::GAMMA_4, activeSegment1 or activeSegment2 );
                     }


                  } else if ( mapElements.firstJ ( false ) != 0 and j < mapElements.firstJ ( false )) {

                     if ( NumericFunctions::inRange<unsigned int> ( i, mapElements.firstI ( false ), mapElements.lastI ( false ))) {

                        activeSegment1 = std::fabs ( depth ( k, j, i     ) - depth ( k + 1, j, i     )) > DepositingThicknessTolerance;
                        activeSegment2 = std::fabs ( depth ( k, j, i + 1 ) - depth ( k + 1, j, i + 1 )) > DepositingThicknessTolerance;

                        volumeElement.setIsActive ( activeSegment1 or activeSegment2 );
                        volumeElement.setIsActiveBoundary ( VolumeData::GAMMA_4, activeSegment1 or activeSegment2 );
                     }

                  }

               }


            }

         } else {

            // The whole column of elements is inactive.
            for ( k = 0; k < getMaximumNumberOfElements (); ++k ) {
               LayerElement& volumeElement = getLayerElement ( i, j, k );

               volumeElement.setIsActive ( false );
            }

         }

      }

   }

   depth.Restore_Global_Array ( No_Update );
}

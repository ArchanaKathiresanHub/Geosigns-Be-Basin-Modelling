//
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include <cassert>

#include "CrustFormation.h"
#include "GeoPhysicsCrustFormation.h"
#include "CompoundLithology.h"

#include "FastcauldronSimulator.h"
#include "propinterface.h"

// DAL
#include "Interface/Interface.h"
#include "Interface/GridMap.h"
#include "Interface/Snapshot.h"
#include "Interface/Surface.h"

// utilities library
#include "ConstantsNumerical.h"
using Utilities::Numerical::CauldronNoDataValue;
using Utilities::Numerical::IbsNoDataValue;
#include "ConstantsMathematics.h"
using Utilities::Maths::Zero;

CrustFormation::CrustFormation ( Interface::ProjectHandle * projectHandle, database::Record * record ) : 
   DataAccess::Interface::Formation ( projectHandle, record ),
   GeoPhysics::Formation ( projectHandle, record ),
   LayerProps ( projectHandle, record ), 
   DataAccess::Interface::BasementFormation ( projectHandle, record, Interface::CrustFormationName, projectHandle->getCrustLithoName() ),
   DataAccess::Interface::CrustFormation ( projectHandle, record ),
   GeoPhysics::GeoPhysicsCrustFormation ( projectHandle, record ) {


   fluid = nullptr;
   TopSurface_DepoSeq = 0;
   Layer_Depo_Seq_Nb = -1;
   Calculate_Chemical_Compaction = false;

   m_heatProductionMap = nullptr;

   m_basaltThickness           = nullptr;
   m_crustalThicknessMeltOnset = nullptr;

   TopBasaltDepth      = nullptr;
   BasaltThickness     = nullptr;
   BottomBasaltDepth   = nullptr;
   ThicknessBasaltALC  = nullptr;
   ThicknessCCrustALC  = nullptr;
   SmCCrustThickness   = nullptr;
   SmTopBasaltDepth    = nullptr;
   SmBottomBasaltDepth = nullptr;

   initialiseBasementVecs();
   setBasementVectorList();
 }

//------------------------------------------------------------//
CrustFormation::~CrustFormation () {

   if ( m_heatProductionMap != nullptr and m_heatProductionMap->retrieved ()) {
      m_heatProductionMap->restoreData ( false, true );
   }
   
   delete m_crustalThicknessMeltOnset;
   delete m_basaltThickness;

   Destroy_Petsc_Vector ( TopBasaltDepth );
   Destroy_Petsc_Vector ( BasaltThickness );
   Destroy_Petsc_Vector ( BottomBasaltDepth );
   Destroy_Petsc_Vector ( ThicknessBasaltALC );
   Destroy_Petsc_Vector ( ThicknessCCrustALC );
   Destroy_Petsc_Vector ( SmCCrustThickness );
   Destroy_Petsc_Vector ( SmTopBasaltDepth );
   Destroy_Petsc_Vector ( SmBottomBasaltDepth );
 }

//------------------------------------------------------------//
void CrustFormation::initialise () {

   layername           = Interface::CrustFormation::getName ();
   depoage             = Interface::CrustFormation::getTopSurface ()->getSnapshot ()->getTime ();
   m_lithoMixModel     = Interface::CrustFormation::getMixModelStr ();
   presentDayThickness = Interface::CrustFormation::getInputThicknessMap ();
   depthGridMap        = Interface::CrustFormation::getTopSurface ()->getInputDepthMap ();

   m_heatProductionMap = getCrustHeatProductionMap ();

   if ( not m_heatProductionMap->retrieved ()) {
      m_heatProductionMap->retrieveGhostedData ();
   }

   setLayerElements ();
}

//------------------------------------------------------------//
void CrustFormation::cleanVectors() {

   if(dynamic_cast<GeoPhysics::ProjectHandle*>(GeoPhysics::Formation::m_projectHandle)->isALC() ) {
      setVec ( TopBasaltDepth, CauldronNoDataValue );
      setVec ( BottomBasaltDepth, CauldronNoDataValue );
      setVec ( ThicknessBasaltALC, Zero );
      setVec ( SmTopBasaltDepth, CauldronNoDataValue );
      setVec ( SmBottomBasaltDepth, CauldronNoDataValue );
      
      previousBasaltMap = BasaltMap;
      BasaltMap.fill( false );

   }
}

//------------------------------------------------------------//
void CrustFormation::allocateBasementVecs() {

   const AppCtx* basinModel =  FastcauldronSimulator::getInstance ().getCauldron ();

   if( basinModel->isALC()) {
      assert( nullptr == TopBasaltDepth );
      createCount++;
      DMCreateGlobalVector( * basinModel->mapDA, &TopBasaltDepth );
      setVec ( TopBasaltDepth, CauldronNoDataValue );

      assert( nullptr == BasaltThickness );
      createCount++;
      DMCreateGlobalVector( * basinModel->mapDA, &BasaltThickness );

      assert( nullptr == BottomBasaltDepth );
      createCount++;
      DMCreateGlobalVector( * basinModel->mapDA, &BottomBasaltDepth );
      setVec ( BottomBasaltDepth, CauldronNoDataValue );

      assert( nullptr == ThicknessBasaltALC );
      createCount++;
      DMCreateGlobalVector( * basinModel->mapDA, &ThicknessBasaltALC );
      setVec ( ThicknessBasaltALC, Zero );

      assert( nullptr == ThicknessCCrustALC );
      createCount++;
      DMCreateGlobalVector( * basinModel->mapDA, &ThicknessCCrustALC );

      assert( nullptr == SmCCrustThickness );
      createCount++;
      DMCreateGlobalVector( * basinModel->mapDA, &SmCCrustThickness );

      assert( nullptr == SmTopBasaltDepth );
      createCount++;
      DMCreateGlobalVector( * basinModel->mapDA, &SmTopBasaltDepth );

      assert( nullptr == SmBottomBasaltDepth );
      createCount++;
      DMCreateGlobalVector( * basinModel->mapDA, &SmBottomBasaltDepth );

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
void CrustFormation::initialiseBasementVecs() {
   TopBasaltDepth           = nullptr;
   BasaltThickness          = nullptr;
   BottomBasaltDepth        = nullptr;
   ThicknessBasaltALC       = nullptr;
   ThicknessCCrustALC       = nullptr;
   SmCCrustThickness        = nullptr;
   SmTopBasaltDepth         = nullptr;
   SmBottomBasaltDepth      = nullptr;
}

//------------------------------------------------------------//
void CrustFormation::reInitialiseBasementVecs() {

   Destroy_Petsc_Vector ( TopBasaltDepth );
   Destroy_Petsc_Vector ( BasaltThickness );
   Destroy_Petsc_Vector ( ThicknessBasaltALC );
   Destroy_Petsc_Vector ( ThicknessCCrustALC );
   Destroy_Petsc_Vector ( BottomBasaltDepth );
   Destroy_Petsc_Vector ( SmCCrustThickness );
   Destroy_Petsc_Vector ( SmTopBasaltDepth );
   Destroy_Petsc_Vector ( SmBottomBasaltDepth );
}

//------------------------------------------------------------//
void CrustFormation::setBasementVectorList() {

   vectorList.VecArray[TOP_BASALT_ALC]                     = &TopBasaltDepth     ; TopBasaltDepth      = nullptr;
   vectorList.VecArray[ALC_SM_THICKNESS_OCEANIC_CRUST]     = &BasaltThickness    ; BasaltThickness     = nullptr;
   vectorList.VecArray[MOHO_ALC]                           = &BottomBasaltDepth  ; BottomBasaltDepth   = nullptr;
   vectorList.VecArray[THICKNESS_CONTINENTAL_CRUST_ALC]    = &ThicknessCCrustALC ; ThicknessCCrustALC  = nullptr;
   vectorList.VecArray[THICKNESS_OCEANIC_CRUST_ALC]        = &ThicknessBasaltALC ; ThicknessBasaltALC  = nullptr;
   vectorList.VecArray[ALC_SM_THICKNESS_CONTINENTAL_CRUST] = &SmCCrustThickness  ; SmCCrustThickness   = nullptr;
   vectorList.VecArray[ALC_SM_TOP_BASALT]                  = &SmTopBasaltDepth   ; SmTopBasaltDepth    = nullptr;
   vectorList.VecArray[ALC_SM_MOHO]                        = &SmBottomBasaltDepth; SmBottomBasaltDepth = nullptr;
}

//------------------------------------------------------------//
const CompoundLithology* CrustFormation::getBasaltLithology(const int iPosition, const int jPosition) const {

   return m_basaltLithology(iPosition, jPosition);
}

//------------------------------------------------------------//
const CompoundLithology* CrustFormation::getLithology(const int iPosition, const int jPosition, const int kPosition ) const {
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

const CompoundLithology* CrustFormation::getLithology( const double aTime, const int iPosition, const int jPosition,  const double aOffset ) {
   // crustThickness shows a ContinentalCrustalThickness. If offset from the top of the Crust (aOffset) goes below it, than we are in Basalt

   const AppCtx* aBasinModel =  FastcauldronSimulator::getInstance ().getCauldron ();
   if( aBasinModel ->isALC() ) {
      // aOffset is a relative depth of the middle point of element - so could be inside Crust or Mantle
      const double basThickness = dynamic_cast<GeoPhysics::ProjectHandle*>(GeoPhysics::Formation::m_projectHandle)->getBasaltThickness(iPosition, jPosition, aTime);

      if(basThickness != IbsNoDataValue && basThickness != 0.0) {
         if( aOffset >= dynamic_cast<GeoPhysics::ProjectHandle*>(GeoPhysics::Formation::m_projectHandle)->getContCrustThickness( iPosition, jPosition, aTime )) {
            isBasaltLayer = true;
            return m_basaltLithology(iPosition, jPosition);
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
      return LayerProps::getLithology(iPosition, jPosition);
   }
}

//------------------------------------------------------------//

bool CrustFormation::setLithologiesFromStratTable () {

   bool createdLithologies = true;

   if( not GeoPhysicsCrustFormation::setLithologiesFromStratTable () ) {
      return false;
   }
   if(dynamic_cast<GeoPhysics::ProjectHandle*>(GeoPhysics::Formation::m_projectHandle)->isALC() ) {
     
     m_basaltLithology.allocate ( GeoPhysics::Formation::m_projectHandle->getActivityOutputGrid ());
     
     CompoundLithologyComposition lc ( DataAccess::Interface::ALCBasalt,           "",  "",
                                       100.0, 0.0, 0.0,
                                       DataAccess::Interface::CrustFormation::getMixModelStr (),
                                       DataAccess::Interface::CrustFormation::getLayeringIndex());

     lc.setThermalModel( m_projectHandle->getCrustPropertyModel() );
     CompoundLithology* pMixedLitho = dynamic_cast<GeoPhysics::ProjectHandle*>(GeoPhysics::Formation::m_projectHandle)->getLithologyManager ().getCompoundLithology ( lc );
     createdLithologies = pMixedLitho != nullptr;
     m_basaltLithology.fillWithLithology ( pMixedLitho );
}

  return createdLithologies;
}

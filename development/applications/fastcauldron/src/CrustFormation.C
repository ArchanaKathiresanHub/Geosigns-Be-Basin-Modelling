#include "mpi.h"

#include "CrustFormation.h"
#include "GeoPhysicsCrustFormation.h"
#include "CompoundLithology.h"

#include "FastcauldronSimulator.h"
#include "propinterface.h"

// DAL
#include "Interface/Interface.h"
#include "Interface/PaleoFormationProperty.h"
#include "Interface/GridMap.h"
#include "Interface/Snapshot.h"
#include "Interface/Surface.h"


CrustFormation::CrustFormation ( Interface::ProjectHandle * projectHandle, database::Record * record ) : 
   DataAccess::Interface::Formation ( projectHandle, record ),
   GeoPhysics::Formation ( projectHandle, record ),
   LayerProps ( projectHandle, record ), 
   DataAccess::Interface::BasementFormation ( projectHandle, record, Interface::CrustFormationName, Interface::CrustLithologyName ),
   DataAccess::Interface::CrustFormation ( projectHandle, record ),
   GeoPhysics::GeoPhysicsCrustFormation ( projectHandle, record ) {


   fluid = 0;
   TopSurface_DepoSeq = 0;
   Layer_Depo_Seq_Nb = -1;
   Calculate_Chemical_Compaction = false;

   m_heatProductionMap = 0;

   basaltThickness = 0;
   crustalThicknessMeltOnset = 0;

   TopBasaltDepth = NULL;
   BasaltThickness = NULL;
   BottomBasaltDepth       = NULL;
   ThicknessBasaltALC = NULL;
   ThicknessCCrustALC = NULL;
   SmCCrustThickness = NULL;
   SmTopBasaltDepth = NULL;
   SmBottomBasaltDepth = NULL;

   
   setBasementVectorList();
 }

//------------------------------------------------------------//
CrustFormation::~CrustFormation () {

   if ( m_heatProductionMap != 0 and not m_heatProductionMap->retrieved ()) {
      m_heatProductionMap->restoreData ( false, true );
   }
   
   delete crustalThicknessMeltOnset;
   delete basaltThickness;

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

   layername = Interface::CrustFormation::getName ();
   depoage = Interface::CrustFormation::getTopSurface ()->getSnapshot ()->getTime ();
   lithoMixModel = Interface::CrustFormation::getMixModelStr ();
   presentDayThickness = Interface::CrustFormation::getInputThicknessMap ();
   depthGridMap = Interface::CrustFormation::getTopSurface ()->getInputDepthMap ();

   m_heatProductionMap = getCrustHeatProductionMap ();

   if ( not m_heatProductionMap->retrieved ()) {
      m_heatProductionMap->retrieveGhostedData ();
   }

}

//------------------------------------------------------------//
void CrustFormation::cleanVectors() {

   if(((GeoPhysics::ProjectHandle*)(GeoPhysics::Formation::m_projectHandle))->isALC() ) {
      setVec ( TopBasaltDepth, CAULDRONIBSNULLVALUE );
      setVec ( BottomBasaltDepth, CAULDRONIBSNULLVALUE );
      setVec ( ThicknessBasaltALC, Zero );
      setVec ( SmTopBasaltDepth, CAULDRONIBSNULLVALUE );
      setVec ( SmBottomBasaltDepth, CAULDRONIBSNULLVALUE );
      
      previousBasaltMap = BasaltMap;
      BasaltMap.fill( false );

   }
}

//------------------------------------------------------------//
void CrustFormation::allocateBasementVecs() {

   const AppCtx* basinModel =  FastcauldronSimulator::getInstance ().getCauldron ();

   if( basinModel->isALC()) {
      IBSASSERT(NULL == TopBasaltDepth);
      createCount++;
      int ierr = DMCreateGlobalVector( * basinModel->mapDA, &TopBasaltDepth );
      setVec ( TopBasaltDepth, CAULDRONIBSNULLVALUE );

      IBSASSERT(NULL == BasaltThickness);
      createCount++;
      ierr = DMCreateGlobalVector( * basinModel->mapDA, &BasaltThickness );

      IBSASSERT(NULL == BottomBasaltDepth);
      createCount++;
      ierr = DMCreateGlobalVector( * basinModel->mapDA, &BottomBasaltDepth );
      setVec ( BottomBasaltDepth, CAULDRONIBSNULLVALUE );

      IBSASSERT( NULL == ThicknessBasaltALC );
      createCount++;
      ierr = DMCreateGlobalVector( * basinModel->mapDA, &ThicknessBasaltALC );
      setVec ( ThicknessBasaltALC, Zero );
	  
      IBSASSERT( NULL == ThicknessCCrustALC );
      createCount++;
      ierr = DMCreateGlobalVector( * basinModel->mapDA, &ThicknessCCrustALC );
	  
      IBSASSERT( NULL == SmCCrustThickness );
      createCount++;
      ierr = DMCreateGlobalVector( * basinModel->mapDA, &SmCCrustThickness );

      IBSASSERT( NULL == SmTopBasaltDepth );
      createCount++;
      ierr = DMCreateGlobalVector( * basinModel->mapDA, &SmTopBasaltDepth );

      IBSASSERT( NULL == SmBottomBasaltDepth );
      createCount++;
      ierr = DMCreateGlobalVector( * basinModel->mapDA, &SmBottomBasaltDepth );

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
   TopBasaltDepth           = NULL;
   BasaltThickness          = NULL;
   BottomBasaltDepth        = NULL;
   ThicknessBasaltALC       = NULL;
   ThicknessCCrustALC       = NULL;
   SmCCrustThickness        = NULL;
   SmTopBasaltDepth         = NULL;
   SmBottomBasaltDepth      = NULL;
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

   vectorList.VecArray[TOPBASALTALC] = &TopBasaltDepth; TopBasaltDepth = NULL;
   vectorList.VecArray[BASALTTHICKNESS] = &BasaltThickness;  BasaltThickness = NULL;
   vectorList.VecArray[MOHOALC] = &BottomBasaltDepth; BottomBasaltDepth = NULL;
   vectorList.VecArray[THICKNESSCCRUSTALC] = &ThicknessCCrustALC; ThicknessCCrustALC = NULL;
   vectorList.VecArray[THICKNESSBASALTALC] = &ThicknessBasaltALC; ThicknessBasaltALC = NULL;
   vectorList.VecArray[ALCSMCRUST] = &SmCCrustThickness; SmCCrustThickness = NULL;
   vectorList.VecArray[ALCSMTOPBASALT] = &SmTopBasaltDepth; SmTopBasaltDepth = NULL;
   vectorList.VecArray[ALCSMMOHO] = &SmBottomBasaltDepth; SmBottomBasaltDepth = NULL;
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
      double basThickness = ((GeoPhysics::ProjectHandle*)(GeoPhysics::Formation::m_projectHandle))->getBasaltThickness(iPosition, jPosition, aTime);
      
      if(basThickness != IBSNULLVALUE && basThickness != 0.0) {
         if( aOffset >= ((GeoPhysics::ProjectHandle*)(GeoPhysics::Formation::m_projectHandle))->getContCrustThickness( iPosition, jPosition, aTime )) {
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

   if( GeoPhysics::GeoPhysicsCrustFormation::setLithologiesFromStratTable () == false ) {
      return false;
   }
   if(((GeoPhysics::ProjectHandle*)(GeoPhysics::Formation::m_projectHandle))->isALC() ) {
     double formationStartDepositionAge = GeoPhysics::AgeOfEarth;
     
     m_basaltLithology.allocate ( GeoPhysics::Formation::m_projectHandle->getActivityOutputGrid ());
     
     CompoundLithologyComposition lc ( DataAccess::Interface::ALCBasalt,           "",  "",
                                       100.0, 0.0, 0.0,
                                       DataAccess::Interface::CrustFormation::getMixModelStr ());

     lc.setThermalModel( m_projectHandle->getCrustPropertyModel() );
     CompoundLithology* pMixedLitho = ((GeoPhysics::ProjectHandle*)(GeoPhysics::Formation::m_projectHandle))->getLithologyManager ().getCompoundLithology ( lc );
     createdLithologies = pMixedLitho != 0;
     m_basaltLithology.fillWithLithology ( formationStartDepositionAge, pMixedLitho );
}

  return createdLithologies;
}      

#include "MantleFormation.h"

#include "FastcauldronSimulator.h"
#include "propinterface.h"

// DAL
#include "Interface/Interface.h"
#include "Interface/PaleoFormationProperty.h"
#include "Interface/GridMap.h"
#include "Interface/Surface.h"
#include "Interface/BasementSurface.h"
#include "Interface/Snapshot.h"


MantleFormation::MantleFormation ( Interface::ProjectHandle * projectHandle,
                                   database::Record *              record ) : 
   DataAccess::Interface::Formation ( projectHandle, record ),
   GeoPhysics::Formation ( projectHandle, record ),
   LayerProps ( projectHandle, record ), 
   DataAccess::Interface::BasementFormation ( projectHandle, record, Interface::MantleFormationName, Interface::MantleLithologyName ),
   DataAccess::Interface::MantleFormation ( projectHandle, record ),
   GeoPhysics::GeoPhysicsMantleFormation ( projectHandle, record ) {

   fluid = 0;

   depoage = -1;
   TopSurface_DepoSeq = -2;
   Layer_Depo_Seq_Nb = -2;
   BottSurface_DepoSeq = -3;
   Calculate_Chemical_Compaction = false;

   UpliftedOrigMantleDepth = NULL;
   LithosphereThicknessMod   = NULL;

   setBasementVectorList();
}

MantleFormation::~MantleFormation () {
   Destroy_Petsc_Vector ( LithosphereThicknessMod );
   Destroy_Petsc_Vector ( UpliftedOrigMantleDepth );
}

void MantleFormation::cleanVectors() {

   if(((GeoPhysics::ProjectHandle*)(GeoPhysics::Formation::m_projectHandle))->isALC() ) {
      previousBasaltMap = BasaltMap;
      BasaltMap.fill( false );
   }
}

void MantleFormation::initialise () {

   layername = Interface::MantleFormation::getName ();
   lithoMixModel = Interface::MantleFormation::getMixModelStr ();
   m_presentDayThickness = Interface::MantleFormation::getInputThicknessMap ();
   depthGridMap = Interface::MantleFormation::getTopSurface ()->getInputDepthMap ();

}

void MantleFormation::setMaximumThicknessValue ( const double newThickness ) {
   m_maximumDepositedThickness = newThickness;
}

const CompoundLithology* MantleFormation::getBasaltLithology(const int iPosition, const int jPosition) const {

   return m_basaltLithology(iPosition, jPosition);
}

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


bool MantleFormation::setLithologiesFromStratTable () {
   
   bool createdLithologies = true;
   
   if( GeoPhysics::GeoPhysicsMantleFormation::setLithologiesFromStratTable () == false ) {
      return false;
   }
   if(((GeoPhysics::ProjectHandle*)(GeoPhysics::Formation::m_projectHandle))->isALC() ) {
      double formationStartDepositionAge = GeoPhysics::AgeOfEarth;

      m_basaltLithology.allocate ( GeoPhysics::Formation::m_projectHandle->getActivityOutputGrid ());
                
      std::string lithoName1 = "ALC Basalt";
       
      CompoundLithologyComposition lc ( lithoName1, "",  "",
                                        100.0, 0.0, 0.0,
                                        DataAccess::Interface::MantleFormation::getMixModelStr () );

      lc.setThermalModel( m_projectHandle->getMantlePropertyModel() );
      
      CompoundLithology* pMixedLitho = ((GeoPhysics::ProjectHandle*)(GeoPhysics::Formation::m_projectHandle))->getLithologyManager ().getCompoundLithology ( lc );
      createdLithologies = pMixedLitho != 0;
      m_basaltLithology.fillWithLithology ( formationStartDepositionAge, pMixedLitho );
      // if( dynamic_cast<GeoPhysics::ProjectHandle*>(m_projectHandle)->isALC() && m_projectHandle->getRank() == 0 ) {
      //    cout << "Mantle basalt property model = " << pMixedLitho->getThermalModel() << endl;
      // }
   }
   return createdLithologies;
   
}

const CompoundLithology* MantleFormation::getLithology( const double aTime, const int iPosition, const int jPosition, const double aOffset ) {
   // If offset from the top of the Mantle (aOffset) goes above the bottom of Basalt, then we are in Basalt
   const AppCtx* aBasinModel =  FastcauldronSimulator::getInstance ().getCauldron ();

   if( aBasinModel->isALC() ) { 
      double partOfBasaltInMantle = ((GeoPhysics::ProjectHandle*)(GeoPhysics::Formation::m_projectHandle))->getBasaltThickness(iPosition, jPosition, aTime) +
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

void MantleFormation::allocateBasementVecs( ) {
 
   const AppCtx* basinModel =  FastcauldronSimulator::getInstance ().getCauldron ();

   if( basinModel->isALC()) {
      IBSASSERT(NULL == UpliftedOrigMantleDepth);
      createCount++;    
      DACreateGlobalVector( * basinModel->mapDA, &UpliftedOrigMantleDepth );
       
      IBSASSERT( NULL == LithosphereThicknessMod );
      createCount++;
      DACreateGlobalVector( * basinModel->mapDA, &LithosphereThicknessMod );

      if ( BasaltMap.isNull ()) {
         BasaltMap.create ( layerDA );
      }
      if ( previousBasaltMap.isNull ()) {
         previousBasaltMap.create ( layerDA );
         previousBasaltMap.fill( false );
      }
  }
}

void MantleFormation::initialiseBasementVecs() {
   UpliftedOrigMantleDepth = NULL;
   LithosphereThicknessMod = NULL;
}

void MantleFormation::reInitialiseBasementVecs() {
   Destroy_Petsc_Vector ( UpliftedOrigMantleDepth );
   Destroy_Petsc_Vector ( LithosphereThicknessMod ); 
}

void MantleFormation::setBasementVectorList() {
   vectorList.VecArray[ALCORIGMANTLE] = &UpliftedOrigMantleDepth; UpliftedOrigMantleDepth = NULL;
   vectorList.VecArray[HLMOD]         = &LithosphereThicknessMod; LithosphereThicknessMod = NULL;
}

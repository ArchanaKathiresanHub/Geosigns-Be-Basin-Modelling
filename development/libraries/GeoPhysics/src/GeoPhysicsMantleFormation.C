#include "GeoPhysicsMantleFormation.h"

#include <cmath>

#include "Interface/Interface.h"
#include "Interface/GridMap.h"
#include "Interface/PaleoFormationProperty.h"
#include "Interface/Snapshot.h"
#include "Interface/RunParameters.h"

#include "GeoPhysicsProjectHandle.h"
#include "CompoundLithology.h"
#include "CompoundLithologyComposition.h"
#include "LithologyManager.h"
#include "GeoPhysicsCrustFormation.h"

#include "NumericFunctions.h"


using namespace DataAccess;

GeoPhysics::GeoPhysicsMantleFormation::GeoPhysicsMantleFormation ( DataAccess::Interface::ProjectHandle* projectHandle,
                                                                   database::Record*                          record ) :
   DataAccess::Interface::Formation ( projectHandle, record ),
   GeoPhysics::Formation ( projectHandle, record ),
   DataAccess::Interface::BasementFormation ( projectHandle, record, Interface::MantleFormationName, Interface::MantleLithologyName ),
   DataAccess::Interface::MantleFormation ( projectHandle, record ) 
{
   // Nothing to do here!
}



bool GeoPhysics::GeoPhysicsMantleFormation::setLithologiesFromStratTable () {

   double formationStartDepositionAge;

   m_compoundLithologies.allocate ( DataAccess::Interface::MantleFormation::m_projectHandle->getActivityOutputGrid ());

   formationStartDepositionAge = GeoPhysics::AgeOfEarth;

   string lithoName1;

   bool noDefinedLithologyValue;
   bool createdLithologies = true;

   CompoundLithology*  pMixedLitho;

   lithoName1 = DataAccess::Interface::MantleFormation::getLithoType1 ()->getName ();
   noDefinedLithologyValue = true;

   CompoundLithologyComposition lc ( lithoName1, "",  "",
                                     100.0, 0.0, 0.0,
                                     DataAccess::Interface::MantleFormation::getMixModelStr () );

   if( dynamic_cast<GeoPhysics::ProjectHandle*>(m_projectHandle)->isALC() ) {
      lc.setThermalModel( m_projectHandle->getMantlePropertyModel());
   }

   pMixedLitho = ((GeoPhysics::ProjectHandle*)(GeoPhysics::Formation::m_projectHandle))->getLithologyManager ().getCompoundLithology ( lc );
   createdLithologies = pMixedLitho != 0;
   m_compoundLithologies.fillWithLithology ( formationStartDepositionAge, pMixedLitho );

   if( dynamic_cast<GeoPhysics::ProjectHandle*>(m_projectHandle)->isALC() && m_projectHandle->getRank() == 0 ) {
      cout << "Mantle property model = " << pMixedLitho->getThermalModel() << endl;
   }
   return createdLithologies;
}

void GeoPhysics::GeoPhysicsMantleFormation::determineMinMaxThickness () {

   double gridMapMaximum;
   double gridMapMinimum;

   const GeoPhysics::GeoPhysicsCrustFormation* crust = dynamic_cast<const GeoPhysics::GeoPhysicsCrustFormation*>( m_projectHandle->getCrustFormation ());


   if ( GeoPhysics::Formation::m_projectHandle->getBottomBoundaryConditions () == Interface::MANTLE_HEAT_FLOW ) {

      const Interface::GridMap* thicknessMap = dynamic_cast<const Interface::GridMap*>(Interface::MantleFormation::getInputThicknessMap ());

      thicknessMap->retrieveData ();
      thicknessMap->getMinMaxValue ( m_minimumDepositedThickness, m_maximumDepositedThickness );

      thicknessMap->restoreData ( false );

   } else {
      Interface::PaleoFormationPropertyList* mantleThicknesses = Interface::MantleFormation::getPaleoThicknessHistory ();
      Interface::PaleoFormationPropertyList::const_iterator thicknessIter;

      m_minimumDepositedThickness =  1.0e10;
      m_maximumDepositedThickness = -1.0e10;

      for ( thicknessIter = mantleThicknesses->begin (); thicknessIter != mantleThicknesses->end (); ++thicknessIter ) {
         const Interface::GridMap* thicknessMap = dynamic_cast<const Interface::GridMap*>((*thicknessIter)->getMap (Interface::MantleThicknessHistoryInstanceThicknessMap));

         thicknessMap->retrieveData ();
         thicknessMap->getMinMaxValue ( gridMapMinimum, gridMapMaximum );

         m_maximumDepositedThickness = NumericFunctions::Maximum ( m_maximumDepositedThickness, gridMapMaximum );
         m_minimumDepositedThickness = NumericFunctions::Minimum ( m_minimumDepositedThickness, gridMapMinimum );

         thicknessMap->restoreData ( false );
      }

      delete mantleThicknesses;
   }

   const GeoPhysics::ProjectHandle* project = dynamic_cast<GeoPhysics::ProjectHandle*>(m_projectHandle);
   double maximumPresentDayBasementThickness = ( project->isALC() ? 
                                                 getInitialLithosphericMantleThickness() + crust->getInitialCrustalThickness() :
                                                 getPresentDayThickness () + crust->getCrustMaximumThicknessHistory ( 0.0 ));

#if 0
   cout << " MeSsAgE WARNING Check out the maximum mantle thickness.  "
        << getPresentDayThickness () << "  "
        << crust->getCrustMaximumThicknessHistory ( 0.0 ) << "  "
        << crust->getMinimumThickness () << "  "
        << maximumPresentDayBasementThickness << "  "
        << endl;
#endif

   m_maximumDepositedThickness = maximumPresentDayBasementThickness - crust->getMinimumThickness ();

}

//------------------------------------------------------------//

unsigned int GeoPhysics::GeoPhysicsMantleFormation::setMaximumNumberOfElements ( const bool readSizeFromVolumeData ) {

   const GeoPhysics::GeoPhysicsCrustFormation* crust = dynamic_cast<const GeoPhysics::GeoPhysicsCrustFormation*>( m_projectHandle->getCrustFormation ());
   const GeoPhysics::ProjectHandle* project = dynamic_cast<GeoPhysics::ProjectHandle*>(m_projectHandle);
   double layerMaximumThickness = getMaximumThickness ();

  if( project->isALC() ){
     int curNumberOfUniformElements0 = static_cast<int>( floor (  project->getMaximumNumberOfMantleElements() / crust->getCrustThinningRatio () ));
     // numberOfUniformElements0 = PetscMin( numberOfUniformElements0, 10.0 );
     int numberOfUniformElements0 = ( curNumberOfUniformElements0 > 10 ? 10 : ( curNumberOfUniformElements0 < 4 ? 4 : curNumberOfUniformElements0 ));
     double initLithoThickness = getInitialLithosphericMantleThickness () + crust->getInitialCrustalThickness();
     m_mantleElementHeight0 = getInitialLithosphericMantleThickness () / numberOfUniformElements0; 
     m_maximumNumberOfElements = static_cast<int>(ceil (( initLithoThickness - crust->getMinimumThickness()  ) /  ( m_mantleElementHeight0 / crust->getCrustThinningRatio () )));

     if( project->getRank() == 0 ) {
        cout << endl << "heightMantleElement0 = " << m_mantleElementHeight0 << ", numberOfMantleElements0 = " <<  numberOfUniformElements0 << ", numberOfElements = " << m_maximumNumberOfElements << endl;
     }
  } else {
     m_maximumNumberOfElements = (unsigned int)( m_zRefinementFactor * std::ceil ( layerMaximumThickness /
                                                                                   m_projectHandle->getRunParameters ()->getBrickHeightMantle () *
                                                                                   crust->getCrustThinningRatio ()));
     m_maximumNumberOfElements = NumericFunctions::Maximum<unsigned int> ( 1, m_maximumNumberOfElements );
  }
  return m_maximumNumberOfElements;
}
 
//------------------------------------------------------------//

void GeoPhysics::GeoPhysicsMantleFormation::retrieveAllThicknessMaps () {

   dynamic_cast<const Interface::GridMap*>(getInputThicknessMap ())->retrieveGhostedData ();

   if ( m_projectHandle->getBottomBoundaryConditions () == Interface::FIXED_BASEMENT_TEMPERATURE ) {
      Interface::PaleoFormationPropertyList* mantleThicknesses = getPaleoThicknessHistory ();
      Interface::PaleoFormationPropertyList::iterator mantleThicknessIter;

      for ( mantleThicknessIter = mantleThicknesses->begin (); mantleThicknessIter != mantleThicknesses->end (); ++mantleThicknessIter ) {
         dynamic_cast<const Interface::GridMap*>((*mantleThicknessIter)->getMap (Interface::MantleThicknessHistoryInstanceThicknessMap))->retrieveGhostedData ();
      }

      delete mantleThicknesses;
   }

}

//------------------------------------------------------------//

void GeoPhysics::GeoPhysicsMantleFormation::restoreAllThicknessMaps () {

   dynamic_cast<const Interface::GridMap*>(getInputThicknessMap ())->restoreData ( false, true );

   if ( m_projectHandle->getBottomBoundaryConditions () == Interface::FIXED_BASEMENT_TEMPERATURE ) {

      Interface::PaleoFormationPropertyList* mantleThicknesses = getPaleoThicknessHistory ();
      Interface::PaleoFormationPropertyList::iterator mantleThicknessIter;

      for ( mantleThicknessIter = mantleThicknesses->begin (); mantleThicknessIter != mantleThicknesses->end (); ++mantleThicknessIter ) {
         dynamic_cast<const Interface::GridMap*>((*mantleThicknessIter)->getMap (Interface::MantleThicknessHistoryInstanceThicknessMap))->restoreData ( false, true );
      }

      delete mantleThicknesses;
   }

}


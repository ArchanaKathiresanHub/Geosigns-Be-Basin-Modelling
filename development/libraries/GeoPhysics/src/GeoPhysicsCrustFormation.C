#include "GeoPhysicsCrustFormation.h"

#include <cmath>

#include "Interface/Interface.h"
#include "Interface/GridMap.h"
#include "Interface/PaleoFormationProperty.h"
#include "Interface/RunParameters.h"
#include "Interface/Snapshot.h"
#include "Interface/ObjectFactory.h"
#include "Interface/BasementFormation.h"

#include "GeoPhysicsProjectHandle.h"
#include "CompoundLithology.h"
#include "LithologyManager.h"

#include "NumericFunctions.h"

#if 0
#ifdef DISTRIBUTED 
#include "MpiFunctions.h"
#endif
#endif

using namespace DataAccess;

GeoPhysics::GeoPhysicsCrustFormation::GeoPhysicsCrustFormation ( DataAccess::Interface::ProjectHandle* projectHandle,
                                                                 database::Record*                          record ) :
   DataAccess::Interface::Formation ( projectHandle, record ),
   GeoPhysics::Formation ( projectHandle, record ),
   DataAccess::Interface::BasementFormation ( projectHandle, record, Interface::CrustFormationName, Interface::CrustLithologyName ),
   DataAccess::Interface::CrustFormation ( projectHandle, record ) 
{
   m_crustThinningRatio = Interface::DefaultUndefinedScalarValue;
   m_crustMaximumThickness = 0;
}

GeoPhysics::GeoPhysicsCrustFormation::~GeoPhysicsCrustFormation () {
   delete m_crustMaximumThickness;
}


bool GeoPhysics::GeoPhysicsCrustFormation::setLithologiesFromStratTable () {

   m_compoundLithologies.allocate ( GeoPhysics::Formation::m_projectHandle->getActivityOutputGrid ());

   std::string lithoName1;

   bool noDefinedLithologyValue;
   bool createdLithologies = true;

   CompoundLithology*  pMixedLitho;

   lithoName1 = DataAccess::Interface::CrustFormation::getLithoType1 ()->getName ();
   noDefinedLithologyValue = true;

   CompoundLithologyComposition lc ( lithoName1,           "",  "",
                                     100.0, 0.0, 0.0,
                                     DataAccess::Interface::CrustFormation::getMixModelStr () );

   if( dynamic_cast<GeoPhysics::ProjectHandle*>(m_projectHandle)->isALC() ) {
      lc.setThermalModel( m_projectHandle->getCrustPropertyModel() );
   }
   pMixedLitho = ((GeoPhysics::ProjectHandle*)(GeoPhysics::Formation::m_projectHandle))->getLithologyManager ().getCompoundLithology ( lc );
   createdLithologies = pMixedLitho != 0;
   m_compoundLithologies.fillWithLithology ( pMixedLitho );

   if( dynamic_cast<GeoPhysics::ProjectHandle*>(m_projectHandle)->isALC() && m_projectHandle->getRank() == 0 ) {
      cout << "Crust property model = " << pMixedLitho->getThermalModel() << endl;
   }


   return createdLithologies;
}

void GeoPhysics::GeoPhysicsCrustFormation::determineMinMaxThickness () {

   double gridMapMaximum = 0.0;
   double gridMapMinimum = 0.0;

   if ( GeoPhysics::Formation::m_projectHandle->getBottomBoundaryConditions () == DataAccess::Interface::MANTLE_HEAT_FLOW ) {

      const Interface::GridMap* thicknessMap = dynamic_cast<const Interface::GridMap*>(Interface::CrustFormation::getInputThicknessMap ());

      thicknessMap->retrieveData ();
      thicknessMap->getMinMaxValue ( m_minimumDepositedThickness, m_maximumDepositedThickness );

      m_crustThickessHistory.AddPoint ( 0.0, m_maximumDepositedThickness );

      thicknessMap->restoreData ( false );

   } else if ( GeoPhysics::Formation::m_projectHandle->getBottomBoundaryConditions () == DataAccess::Interface::FIXED_BASEMENT_TEMPERATURE ) {
      Interface::PaleoFormationPropertyList* crustThicknesses = Interface::CrustFormation::getPaleoThicknessHistory ();
      Interface::PaleoFormationPropertyList::const_iterator thicknessIter;

      m_minimumDepositedThickness =  1.0e10;
      m_maximumDepositedThickness = -1.0e10;

      for ( thicknessIter = crustThicknesses->begin (); thicknessIter != crustThicknesses->end (); ++thicknessIter ) {
         const Interface::GridMap* thicknessMap = dynamic_cast<const Interface::GridMap*>((*thicknessIter)->getMap (Interface::CrustThinningHistoryInstanceThicknessMap));

         double age = (*thicknessIter)->getSnapshot ()->getTime ();

         thicknessMap->retrieveData ();
         thicknessMap->getMinMaxValue ( gridMapMinimum, gridMapMaximum );

         m_crustThickessHistory.AddPoint( age, gridMapMaximum );

         m_maximumDepositedThickness = NumericFunctions::Maximum ( m_maximumDepositedThickness, gridMapMaximum );
         m_minimumDepositedThickness = NumericFunctions::Minimum ( m_minimumDepositedThickness, gridMapMinimum );

         thicknessMap->restoreData ( false );
      }
      delete crustThicknesses;
   } else {
      Interface::PaleoFormationPropertyList* crustThicknesses = Interface::CrustFormation::getPaleoThicknessHistory ();
      Interface::PaleoFormationPropertyList::const_iterator thicknessIter;

      const GeoPhysics::ProjectHandle* project = dynamic_cast<GeoPhysics::ProjectHandle*>(m_projectHandle);

      m_minimumDepositedThickness =  1.0e10;
      m_maximumDepositedThickness = -1.0e10;
      //   cout << "min = " <<  m_minimumDepositedThickness << "; max= " << m_maximumDepositedThickness << endl;

      double ectValue;
      unsigned int i;
      unsigned int j;
      double gridMapMaximumLocal =  -1.0e10;;
      double gridMapMinimumLocal =   1.0e10;;
      
      for ( thicknessIter = crustThicknesses->begin (); thicknessIter != crustThicknesses->end (); ++thicknessIter ) {
         const Interface::GridMap* thicknessMap = dynamic_cast<const Interface::GridMap*>((*thicknessIter)->getMap (Interface::CrustThinningHistoryInstanceThicknessMap));
         
         double age = (*thicknessIter)->getSnapshot ()->getTime ();
         
         thicknessMap->retrieveData ();
         
         for ( i = thicknessMap->firstI (); i <= thicknessMap->lastI (); ++i ) {
            for ( j = thicknessMap->firstJ (); j <= thicknessMap->lastJ (); ++j ) {
               if ( project->getNodeIsValid ( i, j )) {
                  ectValue = project->getCrustThickness ( i, j, age ); //effective crustal thickness in this case
                  // gridMapMaximumLocal = PetscMax ( gridMapMaximumLocal, ectValue );
                  // gridMapMinimumLocal = PetscMin ( gridMapMinimumLocal, ectValue );
                  gridMapMaximumLocal = NumericFunctions::Maximum ( gridMapMaximumLocal, ectValue );
                  gridMapMinimumLocal = NumericFunctions::Minimum ( gridMapMinimumLocal, ectValue );
               }
            }
         }
         // MPI_Allreduce (&gridMapMaximumLocal, &gridMapMaximum, 1, MPI_DOUBLE, MPI_MAX, PETSC_COMM_WORLD);
         // MPI_Allreduce (&gridMapMinimumLocal, &gridMapMinimum, 1, MPI_DOUBLE, MPI_MIN, PETSC_COMM_WORLD);
          m_projectHandle->getMinValue ( &gridMapMinimumLocal, &gridMapMinimum );
          m_projectHandle->getMaxValue ( &gridMapMaximumLocal, &gridMapMaximum );

         m_crustThickessHistory.AddPoint( age, gridMapMaximum );
         
         m_maximumDepositedThickness = NumericFunctions::Maximum ( m_maximumDepositedThickness, gridMapMaximum );
         m_minimumDepositedThickness = NumericFunctions::Minimum ( m_minimumDepositedThickness, gridMapMinimum );

         thicknessMap->restoreData ( false );
      }
      //   cout << "min = " <<  m_minimumDepositedThickness << "; max= " << m_maximumDepositedThickness << endl;
      if( getInitialCrustalThickness() < 0 ) {
         setInitialCrustalThickness( m_maximumDepositedThickness );
         if( m_projectHandle->getRank() == 0 ) {
            cout << " Set initialCrustalThickness to maximum Crustal thickness = " << m_maximumDepositedThickness << endl;
         }         
      }
      delete crustThicknesses;
  }

}

double GeoPhysics::GeoPhysicsCrustFormation::getCrustMaximumThicknessHistory ( const double age ) const {
   return m_crustThickessHistory.F ( age );
}


bool GeoPhysics::GeoPhysicsCrustFormation::determineCrustThinningRatio () {

   bool status = true;

   if ( GeoPhysics::Formation::m_projectHandle->getBottomBoundaryConditions () == DataAccess::Interface::MANTLE_HEAT_FLOW ) {
      DataAccess::Interface::IdentityFunctor identity;

      m_crustThinningRatio = 1.0;
      m_crustMaximumThickness = dynamic_cast<Interface::GridMap*>(m_projectHandle->getFactory ()->produceGridMap ( 0, 0,
                                                                                                                   dynamic_cast<const Interface::GridMap*>(getInputThicknessMap ()),
                                                                                                                   identity ));
      m_crustMaximumThickness->retrieveData ();
   } else {

      unsigned int i;
      unsigned int j;
      double maximumCrustThickness;
      double minimumCrustThickness;
      double value, age;

      Interface::PaleoFormationPropertyList* crustThicknesses = Interface::CrustFormation::getPaleoThicknessHistory ();
      Interface::PaleoFormationPropertyList::const_iterator thicknessIter;

      m_crustMaximumThickness = dynamic_cast<Interface::GridMap*>(m_projectHandle->getFactory ()->produceGridMap ( 0, 0,
                                                                                                                   GeoPhysics::Formation::m_projectHandle->getActivityOutputGrid (),
                                                                                                                   Interface::DefaultUndefinedMapValue ));

      m_crustMaximumThickness->retrieveData ();

      m_crustThinningRatio = -1.0e10;

      for ( thicknessIter = crustThicknesses->begin (); thicknessIter != crustThicknesses->end (); ++thicknessIter ) {
         const Interface::GridMap* thicknessMap = dynamic_cast<const Interface::GridMap*>((*thicknessIter)->getMap (Interface::CrustThinningHistoryInstanceThicknessMap));
         thicknessMap->retrieveData ();
      }

      const GeoPhysics::ProjectHandle* project = dynamic_cast<GeoPhysics::ProjectHandle*>(m_projectHandle);
      const bool alcMode = project->isALC();

      for ( i = project->firstI (); i <= project->lastI (); ++i ) {

         for ( j = project->firstJ (); j <= project->lastJ (); ++j ) {
            minimumCrustThickness =  1.0e10;
            maximumCrustThickness = -1.0e10;

            if ( project->getNodeIsValid ( i, j )) {

               for ( thicknessIter = crustThicknesses->begin (); thicknessIter != crustThicknesses->end (); ++thicknessIter ) {
                  const Interface::GridMap* thicknessMap = dynamic_cast<const Interface::GridMap*>((*thicknessIter)->getMap (Interface::CrustThinningHistoryInstanceThicknessMap));

                  age = (*thicknessIter)->getSnapshot()->getTime();
                  value = ( alcMode ? project->getCrustThickness( i, j, age ) : thicknessMap->getValue ( i, j ) );
                  maximumCrustThickness = NumericFunctions::Maximum ( maximumCrustThickness, value );
                  minimumCrustThickness = NumericFunctions::Minimum ( minimumCrustThickness, value );

               }

               if ( minimumCrustThickness < 0.0 ) {
                  status = false;
               }

               m_crustThinningRatio = NumericFunctions::Maximum ( m_crustThinningRatio, maximumCrustThickness / minimumCrustThickness );
               m_crustMaximumThickness->setValue ( i, j, maximumCrustThickness );
            }

         }

      }

      for ( thicknessIter = crustThicknesses->begin (); thicknessIter != crustThicknesses->end (); ++thicknessIter ) {
         const Interface::GridMap* thicknessMap = dynamic_cast<const Interface::GridMap*>((*thicknessIter)->getMap (Interface::CrustThinningHistoryInstanceThicknessMap));
         thicknessMap->restoreData ( false );
      }


      delete crustThicknesses;

      m_crustThinningRatio = m_projectHandle->getGlobalOperations ().maximum ( m_crustThinningRatio );


#if 0
#ifdef DISTRIBUTED 
      // Now find the maximum of all crust-thinning ratios on all processes.
      m_crustThinningRatio = MpiFunctions::Maximum ( PETSC_COMM_WORLD, m_crustThinningRatio );
#endif
#endif

  }

   return status;
}

double GeoPhysics::GeoPhysicsCrustFormation::getCrustMaximumThickness ( const unsigned int i,
                                                                        const unsigned int j ) const {
   return m_crustMaximumThickness->getValue ( i, j );
}

//------------------------------------------------------------//

unsigned int GeoPhysics::GeoPhysicsCrustFormation::setMaximumNumberOfElements ( const bool readSizeFromVolumeData ) {

   double layerMaximumThickness = getMaximumThickness ();

   m_maximumNumberOfElements = (unsigned int)( m_zRefinementFactor * std::ceil ( layerMaximumThickness / m_projectHandle->getRunParameters ()->getBrickHeightCrust ()));
   m_maximumNumberOfElements = NumericFunctions::Maximum<unsigned int> ( 1, m_maximumNumberOfElements );

   return m_maximumNumberOfElements;
}

//------------------------------------------------------------//

void GeoPhysics::GeoPhysicsCrustFormation::retrieveAllThicknessMaps () {

   dynamic_cast<const Interface::GridMap*>(getInputThicknessMap ())->retrieveGhostedData ();

   if ( m_projectHandle->getBottomBoundaryConditions () == Interface::FIXED_BASEMENT_TEMPERATURE ||
        m_projectHandle->getBottomBoundaryConditions () == Interface::ADVANCED_LITHOSPHERE_CALCULATOR ) {
      Interface::PaleoFormationPropertyList* crustThicknesses = getPaleoThicknessHistory ();
      Interface::PaleoFormationPropertyList::iterator crustThicknessIter;

      for ( crustThicknessIter = crustThicknesses->begin (); crustThicknessIter != crustThicknesses->end (); ++crustThicknessIter ) {
         dynamic_cast<const Interface::GridMap*>((*crustThicknessIter)->getMap (Interface::CrustThinningHistoryInstanceThicknessMap))->retrieveGhostedData ();
      }

      delete crustThicknesses;
   }

}

//------------------------------------------------------------//

void GeoPhysics::GeoPhysicsCrustFormation::restoreAllThicknessMaps () {

   dynamic_cast<const Interface::GridMap*>(getInputThicknessMap ())->restoreData ( false, true );

   if ( m_projectHandle->getBottomBoundaryConditions () == Interface::FIXED_BASEMENT_TEMPERATURE ||
        m_projectHandle->getBottomBoundaryConditions () == Interface::ADVANCED_LITHOSPHERE_CALCULATOR  ) {

      Interface::PaleoFormationPropertyList* crustThicknesses = getPaleoThicknessHistory ();
      Interface::PaleoFormationPropertyList::iterator crustThicknessIter;


      for ( crustThicknessIter = crustThicknesses->begin (); crustThicknessIter != crustThicknesses->end (); ++crustThicknessIter ) {
         dynamic_cast<const Interface::GridMap*>((*crustThicknessIter)->getMap (Interface::CrustThinningHistoryInstanceThicknessMap))->restoreData ( false, true );
      }

      delete crustThicknesses;
   }

}

//------------------------------------------------------------//

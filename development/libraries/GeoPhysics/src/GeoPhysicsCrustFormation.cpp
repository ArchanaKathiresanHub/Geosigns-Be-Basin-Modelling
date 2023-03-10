//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include "GeoPhysicsCrustFormation.h"

// std library
#include <cmath>

// DataAccess library
#include "Interface.h"
#include "GridMap.h"
#include "PaleoFormationProperty.h"
#include "RunParameters.h"
#include "Snapshot.h"
#include "ObjectFactory.h"
#include "BasementFormation.h"
using namespace DataAccess;

// Geophysics library
#include "GeoPhysicsProjectHandle.h"
#include "CompoundLithology.h"
#include "LithologyManager.h"

// utilities library
#include "NumericFunctions.h"

using namespace std;

GeoPhysics::GeoPhysicsCrustFormation::GeoPhysicsCrustFormation ( DataAccess::Interface::ProjectHandle& projectHandle,
                                                                 database::Record*                          record ) :
   DataAccess::Interface::Formation ( projectHandle, record ),
   GeoPhysics::GeoPhysicsFormation ( projectHandle, record ),
   DataAccess::Interface::BasementFormation ( projectHandle, record, Interface::CrustFormationName, getProjectHandle().getCrustLithoName() ),
   DataAccess::Interface::CrustFormation ( projectHandle, record )
{
   m_crustThinningRatio = Interface::DefaultUndefinedScalarValue;
   m_crustMaximumThickness = 0;
   m_crustMinimumThickness = 0;
}

//------------------------------------------------------------//

GeoPhysics::GeoPhysicsCrustFormation::~GeoPhysicsCrustFormation () {
   delete m_crustMaximumThickness;
   delete m_crustMinimumThickness;
}

//------------------------------------------------------------//


GeoPhysics::CompoundLithology* GeoPhysics::GeoPhysicsCrustFormation::getLithologyFromStratTable( bool& /*undefinedMapValue*/, bool /*useMaps*/,
                                                                                                 unsigned int /*i*/,  unsigned int /*j*/,
                                                             const GridMap* /*lithoMap1*/, const GridMap* /*lithoMap2*/, const GridMap* /*lithoMap3*/,
                                                             const string& /*lithoName1*/, const string& /*lithoName2*/, const string& /*lithoName3*/ ) const
{
  const std::string lithoName1 = DataAccess::Interface::CrustFormation::getLithoType1 ()->getName ();
  CompoundLithologyComposition lc ( lithoName1,           "",  "",
                                    100.0, 0.0, 0.0,
                                    DataAccess::Interface::CrustFormation::getMixModelStr (),
                                    DataAccess::Interface::CrustFormation::getLayeringIndex());

  if( getProjectHandle().isALC() )
  {
     lc.setThermalModel( getProjectHandle().getCrustPropertyModel() );
  }
  return dynamic_cast<GeoPhysics::ProjectHandle&>(getProjectHandle()).getLithologyManager ().getCompoundLithology ( lc );
}

bool GeoPhysics::GeoPhysicsCrustFormation::setLithologiesFromStratTable ()
{
   m_compoundLithologies.allocate ( GeoPhysics::GeoPhysicsFormation::getProjectHandle().getActivityOutputGrid ());

   bool undefinedMapValue = false;
   CompoundLithology* pMixedLitho = getLithologyFromStratTable( undefinedMapValue );
   const bool createdLithologies = pMixedLitho != nullptr;
   m_compoundLithologies.fillWithLithology ( pMixedLitho );

   if( getProjectHandle().isALC() && getProjectHandle().getRank() == 0 ) {
      cout << "Crust property model = " << pMixedLitho->getThermalModel() << endl;
   }
   return createdLithologies;
}

//------------------------------------------------------------//

void GeoPhysics::GeoPhysicsCrustFormation::determineMinMaxThickness () {

   double gridMapMaximum = 0.0;
   double gridMapMinimum = 0.0;

   if ( getProjectHandle().getBottomBoundaryConditions () == DataAccess::Interface::MANTLE_HEAT_FLOW ) {

      const Interface::GridMap* thicknessMap = dynamic_cast<const Interface::GridMap*>(Interface::CrustFormation::getInputThicknessMap ());

      thicknessMap->retrieveData ();
      thicknessMap->getMinMaxValue ( m_minimumDepositedThickness, m_maximumDepositedThickness );

      m_crustThickessHistory.AddPoint ( 0.0, m_maximumDepositedThickness );

      thicknessMap->restoreData ( false );

   } else if ( getProjectHandle().getBottomBoundaryConditions () == DataAccess::Interface::FIXED_BASEMENT_TEMPERATURE ) {
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

      const GeoPhysics::ProjectHandle& project = dynamic_cast<GeoPhysics::ProjectHandle&>(getProjectHandle());

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
               if ( project.getNodeIsValid ( i, j )) {
                  ectValue = project.getCrustThickness ( i, j, age ); //effective crustal thickness in this case
                  gridMapMaximumLocal = NumericFunctions::Maximum ( gridMapMaximumLocal, ectValue );
                  gridMapMinimumLocal = NumericFunctions::Minimum ( gridMapMinimumLocal, ectValue );
               }
            }
         }
          getProjectHandle().getMinValue ( &gridMapMinimumLocal, &gridMapMinimum );
          getProjectHandle().getMaxValue ( &gridMapMaximumLocal, &gridMapMaximum );

         m_crustThickessHistory.AddPoint( age, gridMapMaximum );

         m_maximumDepositedThickness = NumericFunctions::Maximum ( m_maximumDepositedThickness, gridMapMaximum );
         m_minimumDepositedThickness = NumericFunctions::Minimum ( m_minimumDepositedThickness, gridMapMinimum );

         thicknessMap->restoreData ( false );
      }

      if( getInitialCrustalThickness() < 0 ) {
         setInitialCrustalThickness( m_maximumDepositedThickness );
         if( getProjectHandle().getRank() == 0 ) {
            cout << "Basin_Warning: Set initialCrustalThickness to maximum Crustal thickness = " << m_maximumDepositedThickness << endl;
         }
      }
      delete crustThicknesses;
  }

}

//------------------------------------------------------------//

double GeoPhysics::GeoPhysicsCrustFormation::getCrustMaximumThicknessHistory ( const double age ) const {
   return m_crustThickessHistory.F ( age );
}

//------------------------------------------------------------//

bool GeoPhysics::GeoPhysicsCrustFormation::determineCrustThinningRatio () {

   bool status = true;

   if ( GeoPhysics::GeoPhysicsFormation::getProjectHandle().getBottomBoundaryConditions () == DataAccess::Interface::MANTLE_HEAT_FLOW ) {
      DataAccess::Interface::IdentityFunctor identity;

      m_crustThinningRatio = 1.0;
      m_crustMaximumThickness = dynamic_cast<Interface::GridMap*>(getProjectHandle().getFactory ()->produceGridMap ( 0, 0,
                                                                                                                   dynamic_cast<const Interface::GridMap*>(getInputThicknessMap ()),
                                                                                                                   identity ));
      m_crustMaximumThickness->retrieveData ();
      m_crustMinimumThickness = dynamic_cast<Interface::GridMap*>(getProjectHandle().getFactory ()->produceGridMap ( 0, 0,
                                                                                                                   dynamic_cast<const Interface::GridMap*>(getInputThicknessMap ()),
                                                                                                                   identity ));
      m_crustMinimumThickness->retrieveData ();
   } else {

      unsigned int i;
      unsigned int j;
      double maximumCrustThickness;
      double minimumCrustThickness;
      double value, age;

      Interface::PaleoFormationPropertyList* crustThicknesses = Interface::CrustFormation::getPaleoThicknessHistory ();
      Interface::PaleoFormationPropertyList::const_iterator thicknessIter;

      m_crustMaximumThickness = dynamic_cast<Interface::GridMap*>(getProjectHandle().getFactory ()->produceGridMap ( 0, 0,
                                                                                                                   GeoPhysics::GeoPhysicsFormation::getProjectHandle().getActivityOutputGrid (),
                                                                                                                   Interface::DefaultUndefinedMapValue ));

      m_crustMaximumThickness->retrieveData ();

      m_crustMinimumThickness = dynamic_cast<Interface::GridMap*>(getProjectHandle().getFactory ()->produceGridMap ( 0, 0,
                                                                                                                   GeoPhysics::GeoPhysicsFormation::getProjectHandle().getActivityOutputGrid (),
                                                                                                                   Interface::DefaultUndefinedMapValue ));

      m_crustMinimumThickness->retrieveData ();

      m_crustThinningRatio = -1.0e10;

      for ( thicknessIter = crustThicknesses->begin (); thicknessIter != crustThicknesses->end (); ++thicknessIter ) {
         const Interface::GridMap* thicknessMap = dynamic_cast<const Interface::GridMap*>((*thicknessIter)->getMap (Interface::CrustThinningHistoryInstanceThicknessMap));
         thicknessMap->retrieveData ();
      }

      const GeoPhysics::ProjectHandle& project = dynamic_cast<GeoPhysics::ProjectHandle&>(getProjectHandle());
      const bool alcMode = project.isALC();

      for ( i = project.firstI (); i <= project.lastI (); ++i ) {

         for ( j = project.firstJ (); j <= project.lastJ (); ++j ) {
            minimumCrustThickness =  1.0e10;
            maximumCrustThickness = -1.0e10;

            if ( project.getNodeIsValid ( i, j )) {

               for ( thicknessIter = crustThicknesses->begin (); thicknessIter != crustThicknesses->end (); ++thicknessIter ) {
                  const Interface::GridMap* thicknessMap = dynamic_cast<const Interface::GridMap*>((*thicknessIter)->getMap (Interface::CrustThinningHistoryInstanceThicknessMap));

                  age = (*thicknessIter)->getSnapshot()->getTime();
                  value = ( alcMode ? project.getCrustThickness( i, j, age ) : thicknessMap->getValue ( i, j ) );
                  maximumCrustThickness = NumericFunctions::Maximum ( maximumCrustThickness, value );
                  minimumCrustThickness = NumericFunctions::Minimum ( minimumCrustThickness, value );

               }

               if ( minimumCrustThickness < 0.0 ) {
                  status = false;
               }

               m_crustThinningRatio = NumericFunctions::Maximum ( m_crustThinningRatio, maximumCrustThickness / minimumCrustThickness );
               m_crustMaximumThickness->setValue ( i, j, maximumCrustThickness );
               m_crustMinimumThickness->setValue ( i, j, minimumCrustThickness );
            }

         }

      }

      for ( thicknessIter = crustThicknesses->begin (); thicknessIter != crustThicknesses->end (); ++thicknessIter ) {
         const Interface::GridMap* thicknessMap = dynamic_cast<const Interface::GridMap*>((*thicknessIter)->getMap (Interface::CrustThinningHistoryInstanceThicknessMap));
         thicknessMap->restoreData ( false );
      }


      delete crustThicknesses;

      m_crustThinningRatio = getProjectHandle().getGlobalOperations ().maximum ( m_crustThinningRatio );
  }

   return status;
}

double GeoPhysics::GeoPhysicsCrustFormation::getCrustMaximumThickness ( const unsigned int i,
                                                                        const unsigned int j ) const {
   return m_crustMaximumThickness->getValue ( i, j );
}


double GeoPhysics::GeoPhysicsCrustFormation::getCrustMinimumThickness ( const unsigned int i,
                                                                        const unsigned int j ) const {
   return m_crustMinimumThickness->getValue ( i, j );
}

//------------------------------------------------------------//

unsigned int GeoPhysics::GeoPhysicsCrustFormation::setMaximumNumberOfElements ( const bool readSizeFromVolumeData ) {
   (void) readSizeFromVolumeData;
   double layerMaximumThickness = getMaximumThickness ();

   m_maximumNumberOfElements = (unsigned int)( m_zRefinementFactor * std::ceil ( layerMaximumThickness / getProjectHandle().getRunParameters ()->getBrickHeightCrust ()));
   m_maximumNumberOfElements = NumericFunctions::Maximum<unsigned int> ( 1, m_maximumNumberOfElements );

   return m_maximumNumberOfElements;
}

//------------------------------------------------------------//

void GeoPhysics::GeoPhysicsCrustFormation::retrieveAllThicknessMaps () {

   dynamic_cast<const Interface::GridMap*>(getInputThicknessMap ())->retrieveGhostedData ();

   if ( getProjectHandle().getBottomBoundaryConditions () == Interface::FIXED_BASEMENT_TEMPERATURE ||
        getProjectHandle().isALC() ) {
      Interface::PaleoFormationPropertyList* crustThicknesses = getPaleoThicknessHistory ();
      Interface::PaleoFormationPropertyList* oceacrustThicknesses = getOceaPaleoThicknessHistory();

      if (crustThicknesses) {
          for (const auto& crustThicknessIter : *crustThicknesses) {
              dynamic_cast<const Interface::GridMap*>((crustThicknessIter)->getMap(Interface::CrustThinningHistoryInstanceThicknessMap))->retrieveGhostedData();
          }
          delete crustThicknesses; crustThicknesses = nullptr;
      }

      if (oceacrustThicknesses) {
          for (const auto& crustThicknessIter : *oceacrustThicknesses) {
              dynamic_cast<const Interface::GridMap*>((crustThicknessIter)->getMap(Interface::OceaCrustThinningHistoryInstanceThicknessMap))
                  ->retrieveGhostedData();
          }
          delete oceacrustThicknesses; oceacrustThicknesses = nullptr;
      }

   }

}

//------------------------------------------------------------//

void GeoPhysics::GeoPhysicsCrustFormation::restoreAllThicknessMaps () {

   dynamic_cast<const Interface::GridMap*>(getInputThicknessMap ())->restoreData ( false, true );

   if ( getProjectHandle().getBottomBoundaryConditions () == Interface::FIXED_BASEMENT_TEMPERATURE ||
        getProjectHandle().isALC()  ) {

      Interface::PaleoFormationPropertyList* crustThicknesses = getPaleoThicknessHistory ();
      Interface::PaleoFormationPropertyList::iterator crustThicknessIter;
    Interface::PaleoFormationPropertyList* oceacrustThicknesses = getOceaPaleoThicknessHistory();

      if ( crustThicknesses) {
          for (crustThicknessIter = crustThicknesses->begin(); crustThicknessIter != crustThicknesses->end(); ++crustThicknessIter) {
              dynamic_cast<const Interface::GridMap*>((*crustThicknessIter)->getMap(Interface::CrustThinningHistoryInstanceThicknessMap))
                  ->restoreData(false, true);
          }
          delete crustThicknesses; crustThicknesses = nullptr;
      }

      if (oceacrustThicknesses) {
          for (crustThicknessIter = oceacrustThicknesses->begin(); crustThicknessIter != oceacrustThicknesses->end(); ++crustThicknessIter) {
              dynamic_cast<const Interface::GridMap*>((*crustThicknessIter)->getMap(Interface::OceaCrustThinningHistoryInstanceThicknessMap))->restoreData(false, true);
          }
          delete oceacrustThicknesses; oceacrustThicknesses = nullptr;
      }

   }

}


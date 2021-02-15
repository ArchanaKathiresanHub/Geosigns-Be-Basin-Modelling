//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "GeoPhysicsMantleFormation.h"

#include <cmath>

#include "Interface.h"
#include "GridMap.h"
#include "PaleoFormationProperty.h"
#include "Snapshot.h"
#include "RunParameters.h"

#include "GeoPhysicsProjectHandle.h"
#include "CompoundLithology.h"
#include "CompoundLithologyComposition.h"
#include "LithologyManager.h"
#include "GeoPhysicsCrustFormation.h"

#include "NumericFunctions.h"

#include <cassert>

using namespace DataAccess;

GeoPhysics::GeoPhysicsMantleFormation::GeoPhysicsMantleFormation ( DataAccess::Interface::ProjectHandle& projectHandle,
                                                                   database::Record*                          record ) :
   DataAccess::Interface::Formation ( projectHandle, record ),
   GeoPhysics::GeoPhysicsFormation ( projectHandle, record ),
   DataAccess::Interface::BasementFormation ( projectHandle, record, Interface::MantleFormationName, getProjectHandle().getMantleLithoName() ),
   DataAccess::Interface::MantleFormation ( projectHandle, record )
{
   // Nothing to do here!
}

GeoPhysics::CompoundLithology* GeoPhysics::GeoPhysicsMantleFormation::getLithologyFromStratTable( bool& /*undefinedMapValue*/, bool /*useMaps*/, unsigned int /*i*/,  unsigned int /*j*/,
                                                             const GridMap* /*lithoMap1*/, const GridMap* /*lithoMap2*/, const GridMap* /*lithoMap3*/,
                                                             const string& /*lithoName1*/, const string& /*lithoName2*/, const string& /*lithoName3*/ ) const
{
  const std::string lithoName1 = DataAccess::Interface::MantleFormation::getLithoType1 ()->getName ();
  CompoundLithologyComposition lc ( lithoName1, "",  "",
                                    100.0, 0.0, 0.0,
                                    DataAccess::Interface::MantleFormation::getMixModelStr (),
                                    DataAccess::Interface::MantleFormation::getLayeringIndex());

  if( getProjectHandle().isALC() )
  {
     lc.setThermalModel( getProjectHandle().getMantlePropertyModel());
  }
  return dynamic_cast<GeoPhysics::ProjectHandle&>(getProjectHandle()).getLithologyManager ().getCompoundLithology ( lc );
}

bool GeoPhysics::GeoPhysicsMantleFormation::setLithologiesFromStratTable ()
{
   m_compoundLithologies.allocate ( getProjectHandle().getActivityOutputGrid ());

   bool undefinedMapValue = false;
   CompoundLithology* pMixedLitho = getLithologyFromStratTable( undefinedMapValue );
   bool createdLithologies = pMixedLitho != nullptr;
   m_compoundLithologies.fillWithLithology ( pMixedLitho );

   if( getProjectHandle().isALC() && getProjectHandle().getRank() == 0 ) {
      cout << "Mantle property model = " << pMixedLitho->getThermalModel() << endl;
   }
   return createdLithologies;
}

void GeoPhysics::GeoPhysicsMantleFormation::determineMinMaxThickness () {

   double gridMapMaximum;
   double gridMapMinimum;

   const GeoPhysics::GeoPhysicsCrustFormation* crust = dynamic_cast<const GeoPhysics::GeoPhysicsCrustFormation*>( getProjectHandle().getCrustFormation ());


   if ( GeoPhysics::GeoPhysicsFormation::getProjectHandle().getBottomBoundaryConditions () == Interface::MANTLE_HEAT_FLOW ) {

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

   const GeoPhysics::ProjectHandle& project = dynamic_cast<GeoPhysics::ProjectHandle&>(getProjectHandle());
   double maximumPresentDayBasementThickness = ( project.isALC() ?
                                                 getInitialLithosphericMantleThickness() + crust->getInitialCrustalThickness() :
                                                 getPresentDayThickness () + crust->getCrustMaximumThicknessHistory ( 0.0 ));

#if 0
   cout << " Basin_Warning: Check out the maximum mantle thickness.  "
        << getPresentDayThickness () << "  "
        << crust->getCrustMaximumThicknessHistory ( 0.0 ) << "  "
        << crust->getMinimumThickness () << "  "
        << maximumPresentDayBasementThickness << "  "
        << endl;
#endif

   m_maximumDepositedThickness = maximumPresentDayBasementThickness - crust->getMinimumThickness ();

}

//------------------------------------------------------------//

unsigned int GeoPhysics::GeoPhysicsMantleFormation::setMaximumNumberOfElements ( const bool /*readSizeFromVolumeData*/ ) {

   const GeoPhysics::GeoPhysicsCrustFormation* crust = dynamic_cast<const GeoPhysics::GeoPhysicsCrustFormation*>( getProjectHandle().getCrustFormation ());
   const GeoPhysics::ProjectHandle& project = dynamic_cast<GeoPhysics::ProjectHandle&>(getProjectHandle());
   const double layerMaximumThickness = getMaximumThickness ();

  if( project.isALC() )
  {
    m_maximumNumberOfElements = project.getMaximumNumberOfMantleElements();
    const double maxLithoThickness = getInitialLithosphericMantleThickness () + crust->getInitialCrustalThickness() - crust->getMinimumThickness() ;

    if ( project.getBottomBoundaryConditions() == Interface::IMPROVED_LITHOSPHERE_CALCULATOR_LINEAR_ELEMENT_MODE )
    {
      m_mantleElementHeight0 = maxLithoThickness / m_maximumNumberOfElements;
    }
    else
    {
      m_globalMaxCrustThinningRatio = crust->getCrustThinningRatio();
      const int curNumberOfUniformElements0 = static_cast<int>( floor ( m_maximumNumberOfElements / m_globalMaxCrustThinningRatio ));
      const int numberOfUniformElements0 = ( curNumberOfUniformElements0 > 10 ? 10 : ( curNumberOfUniformElements0 < 4 ? 4 : curNumberOfUniformElements0 ));
      m_mantleElementHeight0 = getInitialLithosphericMantleThickness () / numberOfUniformElements0 / m_globalMaxCrustThinningRatio;
      m_maximumNumberOfElements = static_cast<int>(ceil( maxLithoThickness / m_mantleElementHeight0));
    }
  }
  else
  {
     m_maximumNumberOfElements = (unsigned int)( m_zRefinementFactor * std::ceil ( layerMaximumThickness / getMantleElementHeight0() * crust->getCrustThinningRatio()));
     m_maximumNumberOfElements = NumericFunctions::Maximum<unsigned int> ( 1, m_maximumNumberOfElements );
  }
  return m_maximumNumberOfElements;
}

//------------------------------------------------------------//

void GeoPhysics::GeoPhysicsMantleFormation::retrieveAllThicknessMaps () {

   dynamic_cast<const Interface::GridMap*>(getInputThicknessMap ())->retrieveGhostedData ();

   if ( getProjectHandle().getBottomBoundaryConditions () == Interface::FIXED_BASEMENT_TEMPERATURE ) {
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

   if ( getProjectHandle().getBottomBoundaryConditions () == Interface::FIXED_BASEMENT_TEMPERATURE ) {

      Interface::PaleoFormationPropertyList* mantleThicknesses = getPaleoThicknessHistory ();
      Interface::PaleoFormationPropertyList::iterator mantleThicknessIter;

      for ( mantleThicknessIter = mantleThicknesses->begin (); mantleThicknessIter != mantleThicknesses->end (); ++mantleThicknessIter ) {
         dynamic_cast<const Interface::GridMap*>((*mantleThicknessIter)->getMap (Interface::MantleThicknessHistoryInstanceThicknessMap))->restoreData ( false, true );
      }

      delete mantleThicknesses;
   }
}

double GeoPhysics::GeoPhysicsMantleFormation::getMantleElementHeight0(double localMaxCrustThicknessRatio, int k, int kMax) const
{
  if ( getProjectHandle().getBottomBoundaryConditions() == Interface::IMPROVED_LITHOSPHERE_CALCULATOR_LINEAR_ELEMENT_MODE )
  {
    assert( kMax + 1 == m_maximumNumberOfElements );
    // Elements increasing linear in size as function of depth
    // Local maximum for crust thickness ratio, instead of global maximum in ALC method
    const double a = (localMaxCrustThicknessRatio - 1) * 2.0 / kMax;
    return m_mantleElementHeight0 * ( 1.0 + a * (kMax - k) );
  }
  else if ( getProjectHandle().getBottomBoundaryConditions() == Interface::ADVANCED_LITHOSPHERE_CALCULATOR )
  {
    // Original ALC method
    return m_mantleElementHeight0 * m_globalMaxCrustThinningRatio;
  }
  else
  {
    // Non ALC methods
    assert( !getProjectHandle().isALC() );
    return getProjectHandle().getRunParameters()->getBrickHeightMantle();
  }
}


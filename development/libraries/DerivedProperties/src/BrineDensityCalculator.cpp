//
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "BrineDensityCalculator.h"

#include "RunParameters.h"
#include "Surface.h"
#include "Snapshot.h"
#include "SimulationDetails.h"

#include "GeoPhysicsFormation.h"
#include "GeoPhysicsFluidType.h"
#include "GeoPhysicalConstants.h"
#include "GeoPhysicalFunctions.h"

#include "DerivedFormationProperty.h"
#include "PropertyRetriever.h"

using namespace AbstractDerivedProperties;

DerivedProperties::BrineDensityCalculator::BrineDensityCalculator ( const GeoPhysics::ProjectHandle& projectHandle ) :
   m_projectHandle ( projectHandle )
{
   addPropertyName ( "BrineDensity" );

   const DataAccess::Interface::SimulationDetails* lastFastcauldronRun = m_projectHandle.getDetailsOfLastFastcauldron();

   m_hydrostaticMode = false;

   if ( lastFastcauldronRun != 0 ) {
      // Even though
      m_hydrostaticMode = lastFastcauldronRun->getSimulatorMode () == "HydrostaticTemperature";
   }

   if ( not m_hydrostaticMode ) {
      addDependentPropertyName ( "Temperature" );
      addDependentPropertyName ( "Pressure" );
   }

}

double DerivedProperties::BrineDensityCalculator::calculateBrineDensity(const GeoPhysics::FluidType* fluid, const double temperature, const double porePressure) const
{
  return fluid->density ( temperature, porePressure);
}

double DerivedProperties::BrineDensityCalculator::calculateBrineDensityHydroStatic(const GeoPhysics::FluidType* fluid) const
{
  const double temperatureGradient = 0.001 * m_projectHandle.getRunParameters ()->getTemperatureGradient ();

  return fluid->getCorrectedSimpleDensity ( GeoPhysics::FluidType::DefaultStandardDepth,
                                            GeoPhysics::FluidType::DefaultHydrostaticPressureGradient,
                                            GeoPhysics::FluidType::StandardSurfaceTemperature,
                                            temperatureGradient );
}

void DerivedProperties::BrineDensityCalculator::calculate ( AbstractPropertyManager&            propertyManager,
                                                            const DataModel::AbstractSnapshot*  snapshot,
                                                            const DataModel::AbstractFormation* formation,
                                                                  FormationPropertyList&        derivedProperties ) const {

   const GeoPhysics::GeoPhysicsFormation* currentFormation = dynamic_cast<const GeoPhysics::GeoPhysicsFormation*>( formation );
   const GeoPhysics::FluidType* fluid = dynamic_cast<const GeoPhysics::FluidType*>(currentFormation->getFluidType ());

   derivedProperties.clear();

   if ( fluid == nullptr or currentFormation->getBottomSurface()->getSnapshot()->getTime() <= snapshot->getTime() ) {
      return;
   }

   const DataModel::AbstractProperty* brineDensityProperty = propertyManager.getProperty ( getPropertyNames ()[ 0 ]);
   DerivedFormationPropertyPtr brineDensity = DerivedFormationPropertyPtr ( new DerivedProperties::DerivedFormationProperty ( brineDensityProperty, snapshot, formation,
                                                                                                                              propertyManager.getMapGrid (),
                                                                                                                              currentFormation->getMaximumNumberOfElements() + 1 ));
   const double undefinedValue = brineDensity->getUndefinedValue ();

   if ( m_hydrostaticMode ) {
      // The temperature gradient in the project file is in C/Km and not C/m and so needs to be converted.
      double fluidDensity  = calculateBrineDensityHydroStatic(fluid);

      for ( unsigned int i = brineDensity->firstI ( true ); i <= brineDensity->lastI ( true ); ++i ) {

         for ( unsigned int j = brineDensity->firstJ ( true ); j <= brineDensity->lastJ ( true ); ++j ) {

            if ( m_projectHandle.getNodeIsValid ( i, j )) {

               for ( unsigned int k = brineDensity->firstK (); k <= brineDensity->lastK (); ++k ) {
                  brineDensity->set ( i, j, k, fluidDensity );
               }
            } else {
               for ( unsigned int k = brineDensity->firstK (); k <= brineDensity->lastK (); ++k ) {
                  brineDensity->set ( i, j, k, undefinedValue );
               }
            }
         }

      }

   } else {
      const DataModel::AbstractProperty* temperatureProperty = propertyManager.getProperty ( "Temperature" );
      FormationPropertyPtr temperature = propertyManager.getFormationProperty ( temperatureProperty, snapshot, formation );

      const DataModel::AbstractProperty* pressureProperty = propertyManager.getProperty ( "Pressure" );
      FormationPropertyPtr porePressure = propertyManager.getFormationProperty ( pressureProperty, snapshot, formation );

      PropertyRetriever temperatureRetriever ( temperature );
      PropertyRetriever pressureRetriever ( porePressure );

      for ( unsigned int i = brineDensity->firstI ( true ); i <= brineDensity->lastI ( true ); ++i ) {

         for ( unsigned int j = brineDensity->firstJ ( true ); j <= brineDensity->lastJ ( true ); ++j ) {

            if ( m_projectHandle.getNodeIsValid ( i, j )) {

               for ( unsigned int k = brineDensity->firstK (); k <= brineDensity->lastK (); ++k ) {
                  brineDensity->set ( i, j, k, calculateBrineDensity(fluid, temperature->get(i,j,k), porePressure->get(i,j,k)));
               }

            } else {
               for ( unsigned int k = brineDensity->firstK (); k <= brineDensity->lastK (); ++k ) {
                  brineDensity->set ( i, j, k, undefinedValue );
               }
            }

         }

      }

   }

   derivedProperties.push_back ( brineDensity );
}

double DerivedProperties::BrineDensityCalculator::calculateAtPosition( const GeoPhysics::GeoPhysicsFormation* formation,
                                                                       const GeoPhysics::CompoundLithology* /*lithology*/,
                                                                       const std::map<string, double>& dependentProperties ) const
{
  const GeoPhysics::FluidType* fluid = dynamic_cast<const GeoPhysics::FluidType*>( formation->getFluidType() );
  if ( !fluid )
  {
    return DataAccess::Interface::DefaultUndefinedScalarValue;
  }

  if (m_hydrostaticMode)
  {
    return calculateBrineDensityHydroStatic(fluid);
  }
  return calculateBrineDensity(fluid, dependentProperties.at("Temperature"), dependentProperties.at("Pressure"));
}

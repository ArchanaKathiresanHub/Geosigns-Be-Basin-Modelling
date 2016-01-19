#include "BrineDensityCalculator.h"

#include "Interface/RunParameters.h"
#include "Interface/Surface.h"
#include "Interface/Snapshot.h"
#include "Interface/SimulationDetails.h"

#include "GeoPhysicsFormation.h"
#include "GeoPhysicsFluidType.h"
#include "GeoPhysicalConstants.h"
#include "GeoPhysicalFunctions.h"

#include "DerivedFormationProperty.h"
#include "PropertyRetriever.h"

DerivedProperties::BrineDensityCalculator::BrineDensityCalculator ( const GeoPhysics::ProjectHandle* projectHandle ) :
   m_projectHandle ( projectHandle )
{
   addPropertyName ( "BrineDensity" );

   const DataAccess::Interface::SimulationDetails* lastFastcauldronRun = m_projectHandle->getDetailsOfLastSimulation ( "fastcauldron" );

   m_hydrostaticMode = false;

   if ( lastFastcauldronRun != 0 ) {
      // Even though
      m_hydrostaticMode = lastFastcauldronRun->getSimulatorMode () == "HydrostaticTemperature" or
                          lastFastcauldronRun->getSimulatorMode () == "HydrostaticHighResDecompaction";
   }

   if ( not m_hydrostaticMode ) {
      addDependentPropertyName ( "Temperature" );
      addDependentPropertyName ( "Pressure" );
   }

}


void DerivedProperties::BrineDensityCalculator::calculate ( AbstractPropertyManager&            propertyManager,
                                                            const DataModel::AbstractSnapshot*  snapshot,
                                                            const DataModel::AbstractFormation* formation,
                                                                  FormationPropertyList&        derivedProperties ) const {

   const GeoPhysics::Formation* currentFormation = dynamic_cast<const GeoPhysics::Formation*>( formation );
   const GeoPhysics::FluidType* fluid = dynamic_cast<const GeoPhysics::FluidType*>(currentFormation->getFluidType ());

   if ( fluid == 0 ) {
      return;
   }

   const DataModel::AbstractProperty* brineDensityProperty = propertyManager.getProperty ( getPropertyNames ()[ 0 ]);
   DerivedFormationPropertyPtr brineDensity = DerivedFormationPropertyPtr ( new DerivedProperties::DerivedFormationProperty ( brineDensityProperty, snapshot, formation, 
                                                                                                                              propertyManager.getMapGrid (),
                                                                                                                              currentFormation->getMaximumNumberOfElements() + 1 ));

   if ( m_hydrostaticMode ) {
      // The temperature gradient in the project file is in C/Km and not C/m and so needs to be converted.
      const double temperatureGradient = 0.001 * m_projectHandle->getRunParameters ()->getTemperatureGradient ();
      double fluidDensity = fluidDensity = fluid->getCorrectedSimpleDensity ( GeoPhysics::FluidType::DefaultStandardDepth,
                                                                              GeoPhysics::FluidType::DefaultHydrostaticPressureGradient,
                                                                              GeoPhysics::FluidType::StandardSurfaceTemperature,
                                                                              temperatureGradient );

      for ( unsigned int i = brineDensity->firstI ( true ); i <= brineDensity->lastI ( true ); ++i ) {

         for ( unsigned int j = brineDensity->firstJ ( true ); j <= brineDensity->lastJ ( true ); ++j ) {

            for ( unsigned int k = brineDensity->firstK (); k <= brineDensity->lastK (); ++k ) {
               brineDensity->set ( i, j, k, fluidDensity );
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

            for ( unsigned int k = brineDensity->firstK (); k <= brineDensity->lastK (); ++k ) {
               brineDensity->set ( i, j, k, fluid->density ( temperature->getA ( i, j, k ), porePressure->getA ( i, j, k )));
            }

         }

      }

   }

   derivedProperties.push_back ( brineDensity );
}

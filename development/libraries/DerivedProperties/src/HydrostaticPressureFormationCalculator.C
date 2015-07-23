#include "HydrostaticPressureFormationCalculator.h"

#include "Interface/Surface.h"
#include "Interface/Snapshot.h"
#include "Interface/RunParameters.h"
#include "Interface/SimulationDetails.h"

#include "GeoPhysicsFormation.h"
#include "GeoPhysicalConstants.h"
#include "GeoPhysicalFunctions.h"
#include "PropertyRetriever.h"

DerivedProperties::HydrostaticPressureFormationCalculator::HydrostaticPressureFormationCalculator ( const GeoPhysics::ProjectHandle* projectHandle ) :
   m_projectHandle ( projectHandle )
{
   addPropertyName ( "HydroStaticPressure" );

   const DataAccess::Interface::SimulationDetails* lastFastcauldronRun = m_projectHandle->getDetailsOfLastSimulation ( "fastcauldron" );

   m_hydrostaticDecompactionMode = false;
   m_hydrostaticMode             = false;

   if ( lastFastcauldronRun != 0 ) {
      m_hydrostaticDecompactionMode = lastFastcauldronRun->getSimulatorMode () == "HydrostaticDecompaction";
      m_hydrostaticMode = lastFastcauldronRun->getSimulatorMode () == "HydrostaticTemperature" or
                          lastFastcauldronRun->getSimulatorMode () == "HydrostaticHighResDecompaction";
   }

   if ( m_hydrostaticDecompactionMode ) {
      addDependentPropertyName ( "Depth" );
   } else if ( m_hydrostaticMode ) {
      addDependentPropertyName ( "Pressure" );
   } else {
      addDependentPropertyName ( "Depth" );
      addDependentPropertyName ( "Temperature" );
      addDependentPropertyName ( "Pressure" );
   }
}
 
void DerivedProperties::HydrostaticPressureFormationCalculator::calculate ( AbstractPropertyManager&            propertyManager,
                                                                            const DataModel::AbstractSnapshot*  snapshot,
                                                                            const DataModel::AbstractFormation* formation,
                                                                                  FormationPropertyList&        derivedProperties ) const {

   const GeoPhysics::Formation* currentFormation = dynamic_cast<const GeoPhysics::Formation*>( formation );
   
   const DataModel::AbstractProperty* hydrostaticPressureProperty = propertyManager.getProperty ( getPropertyNames ()[ 0 ]);
   DerivedFormationPropertyPtr hydrostaticPressure = DerivedFormationPropertyPtr ( new DerivedProperties::DerivedFormationProperty ( hydrostaticPressureProperty, snapshot, formation, 
                                                                                                                                     propertyManager.getMapGrid (),
                                                                                                                                     currentFormation->getMaximumNumberOfElements() + 1 ));

   if ( m_hydrostaticMode ) {
      const DataModel::AbstractProperty* porePressureProperty = propertyManager.getProperty ( "Pressure" );
      
      FormationPropertyPtr porePressure = propertyManager.getFormationProperty ( porePressureProperty, snapshot, formation );
      PropertyRetriever ppRetriever ( porePressure );
      
      derivedProperties.clear ();
      
      // now copy the pore pressure to the hydrostatic pressure
      
      for ( unsigned int i = hydrostaticPressure->firstI ( true ); i <= hydrostaticPressure->lastI ( true ); ++i ) {
         
         for ( unsigned int j = hydrostaticPressure->firstJ ( true ); j <= hydrostaticPressure->lastJ ( true ); ++j ) {
            
            for ( unsigned int k = hydrostaticPressure->firstK (); k <= hydrostaticPressure->lastK (); ++k ) {
               hydrostaticPressure->set ( i, j, k, porePressure->get ( i, j, k ));
            }
         }
      }
      
      derivedProperties.push_back ( hydrostaticPressure );
      return;
   }
   
   FormationPropertyPtr temperature;
   FormationPropertyPtr porePressure;

   if( ! m_hydrostaticDecompactionMode ) {
      const DataModel::AbstractProperty* temperatureProperty = propertyManager.getProperty ( "Temperature" );
      temperature = propertyManager.getFormationProperty ( temperatureProperty, snapshot, formation );

      const DataModel::AbstractProperty* porePressureProperty = propertyManager.getProperty ( "Pressure" );
      porePressure = propertyManager.getFormationProperty ( porePressureProperty, snapshot, formation );
    }
      
   const DataModel::AbstractProperty* depthProperty = propertyManager.getProperty ( "Depth" );
   FormationPropertyPtr depth = propertyManager.getFormationProperty ( depthProperty, snapshot, formation );

   if( ! m_hydrostaticDecompactionMode ) {
     PropertyRetriever temperatureRetriever ( temperature );
     PropertyRetriever ppRetriever ( porePressure );
   }

   PropertyRetriever depthRetriever ( depth );

   const GeoPhysics::Formation* formationAbove = 0;

   const GeoPhysics::FluidType* fluid = dynamic_cast<const GeoPhysics::FluidType*>(currentFormation->getFluidType ());

   double fluidDensityTop;
   double fluidDensityBottom;
   double fluidDensity = 0;

   double thickness;
   double segmentPressure;
   double pressure;
   unsigned int topNodeIndex = hydrostaticPressure->lastK ();
   double undefinedValue = hydrostaticPressure->getUndefinedValue ();

   derivedProperties.clear ();

   if ( currentFormation->getTopSurface ()->getSnapshot () == 0 ||
        currentFormation->getTopSurface ()->getSnapshot ()->getTime () > snapshot->getTime ()) {
      formationAbove = dynamic_cast<const GeoPhysics::Formation*>( currentFormation->getTopSurface ()->getTopFormation ());
   }

   if ( fluid == 0 ) {
      fluidDensity = 0.0;
   } else if ( m_hydrostaticDecompactionMode ) {
      double temperatureGradient = 0.001 * m_projectHandle->getRunParameters ()->getTemperatureGradient ();
      fluidDensity = fluid->getCorrectedSimpleDensity ( GeoPhysics::FluidType::DefaultStandardDepth,
                                                        GeoPhysics::FluidType::DefaultHydrostaticPressureGradient,
                                                        GeoPhysics::FluidType::StandardSurfaceTemperature,
                                                        temperatureGradient );
   }

   // Initialise the top set of nodes for the hydrostatic pressure.
   if ( formationAbove == 0 ) {
      computeHydrostaticPressureAtSeaBottom ( propertyManager, snapshot->getTime (), fluid, hydrostaticPressure );
   } else {
      copyHydrostaticPressureFromLayerAbove ( propertyManager, hydrostaticPressureProperty, snapshot, formationAbove, hydrostaticPressure );
   }

   // now that the top of the set of nodes of the property has been initialised 
   // the hydrostatic pressure for the remaining nodes below them can be computed.
   for ( unsigned int i = hydrostaticPressure->firstI ( true ); i <= hydrostaticPressure->lastI ( true ); ++i ) {

      for ( unsigned int j = hydrostaticPressure->firstJ ( true ); j <= hydrostaticPressure->lastJ ( true ); ++j ) {

         if ( m_projectHandle->getNodeIsValid ( i, j )) {
            fluidDensityTop = ( m_hydrostaticDecompactionMode ? fluidDensity : fluid->density ( temperature->get ( i, j, topNodeIndex ), porePressure->get ( i, j, topNodeIndex )));

            // Loop index is shifted up by 1.
            for ( unsigned int k = hydrostaticPressure->lastK (); k > hydrostaticPressure->firstK (); --k ) {
               // index k     is top node of segment
               // index k - 1 is bottom node of segment

               thickness = depth->get ( i, j, k - 1 ) - depth->get ( i, j, k );
               fluidDensityBottom = ( m_hydrostaticDecompactionMode ? fluidDensity : fluid->density ( temperature->get ( i, j, k - 1 ), porePressure->get ( i, j, k - 1 )));
               segmentPressure = 0.5 * thickness * ( fluidDensityTop + fluidDensityBottom ) * GeoPhysics::AccelerationDueToGravity * GeoPhysics::PascalsToMegaPascals;
               pressure = hydrostaticPressure->get ( i, j, k ) + segmentPressure;
               hydrostaticPressure->set ( i, j, k - 1, pressure );

               // now copy the density at bottom of segment to use for top of segment below.
               // This saves on a density calculation.
               fluidDensityTop = fluidDensityBottom;
            }

         } else {

            for ( unsigned int k = hydrostaticPressure->firstK (); k <= hydrostaticPressure->lastK (); ++k ) {
               hydrostaticPressure->set ( i, j, k, undefinedValue );
            }

         }

      }

   }

   derivedProperties.push_back ( hydrostaticPressure );
}

void DerivedProperties::HydrostaticPressureFormationCalculator::computeHydrostaticPressureAtSeaBottom ( const AbstractPropertyManager&     propertyManager,
                                                                                                        const double                       snapshotAge,
                                                                                                        const GeoPhysics::FluidType*       fluid,
                                                                                                              DerivedFormationPropertyPtr& hydrostaticPressure ) const {

   (void) propertyManager;
   double pressure;
   unsigned int topNodeIndex = hydrostaticPressure->lastK ();

   for ( unsigned int i = hydrostaticPressure->firstI ( true ); i <= hydrostaticPressure->lastI ( true ); ++i ) {

      for ( unsigned int j = hydrostaticPressure->firstJ ( true ); j <= hydrostaticPressure->lastJ ( true ); ++j ) {

         if ( m_projectHandle->getNodeIsValid ( i, j )) {
            GeoPhysics::computeHydrostaticPressure ( fluid,
                                                     m_projectHandle->getSeaBottomTemperature ( i, j, snapshotAge ),
                                                     m_projectHandle->getSeaBottomDepth ( i, j, snapshotAge ),
                                                     pressure );
            hydrostaticPressure->set ( i, j, topNodeIndex, pressure );
         } else {
            hydrostaticPressure->set ( i, j, topNodeIndex, hydrostaticPressure->getUndefinedValue ());
         }

      }

   }

}

void DerivedProperties::HydrostaticPressureFormationCalculator::copyHydrostaticPressureFromLayerAbove ( AbstractPropertyManager&            propertyManager,
                                                                                                        const DataModel::AbstractProperty*  hydrostaticPressureProperty,
                                                                                                        const DataModel::AbstractSnapshot*  snapshot,
                                                                                                        const DataModel::AbstractFormation* formationAbove,
                                                                                                              DerivedFormationPropertyPtr&  hydrostaticPressure ) const {

   const FormationPropertyPtr hydrostaticPressureAbove = propertyManager.getFormationProperty ( hydrostaticPressureProperty, snapshot, formationAbove );
   double undefinedValue = hydrostaticPressureAbove->getUndefinedValue ();
   unsigned int topNodeIndex = hydrostaticPressure->lastK ();

   for ( unsigned int i = hydrostaticPressureAbove->firstI ( true ); i <= hydrostaticPressureAbove->lastI ( true ); ++i ) {

      for ( unsigned int j = hydrostaticPressureAbove->firstJ ( true ); j <= hydrostaticPressureAbove->lastJ ( true ); ++j ) {

         if ( m_projectHandle->getNodeIsValid ( i, j )) {
            hydrostaticPressure->set ( i, j, topNodeIndex, hydrostaticPressureAbove->get ( i, j, 0 ));
         } else {
            hydrostaticPressure->set ( i, j, topNodeIndex, undefinedValue );
         }

      }

   }

}

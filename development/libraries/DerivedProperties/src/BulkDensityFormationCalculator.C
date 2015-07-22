#include "AbstractPropertyManager.h"
#include "DerivedFormationProperty.h"
#include "DerivedPropertyManager.h"

#include "Interface/Interface.h"
#include "Interface/RunParameters.h"
#include "Interface/SimulationDetails.h"

#include "GeoPhysicsFormation.h"
#include "CompoundLithologyArray.h"

#include "PropertyRetriever.h"
#include "BulkDensityFormationCalculator.h"


DerivedProperties::BulkDensityFormationCalculator::BulkDensityFormationCalculator ( const GeoPhysics::ProjectHandle* projectHandle ) : m_projectHandle ( projectHandle ) {
   addPropertyName ( "BulkDensity" );

   const DataAccess::Interface::SimulationDetails* lastFastcauldronRun = m_projectHandle->getDetailsOfLastSimulation ( "fastcauldron" );

   if ( lastFastcauldronRun != 0 ) {
      m_coupledModeEnabled = lastFastcauldronRun->getSimulatorMode () == "LooselyCoupledTemperature" or
                             lastFastcauldronRun->getSimulatorMode () == "CoupledHighResDecompaction" or
                             lastFastcauldronRun->getSimulatorMode () == "CoupledPressureAndTemperature" or
                             lastFastcauldronRun->getSimulatorMode () == "CoupledDarcy";
   } else {
      m_coupledModeEnabled = false;
   }

   m_alcModeEnabled = m_projectHandle->isALC ();

   addDependentPropertyName ( "Porosity" );

   // Here we determine the maximum set of dependencies based on the simulation modes.
   if ( m_coupledModeEnabled ) {
      addDependentPropertyName ( "Pressure" );
      addDependentPropertyName ( "Temperature" );

      if ( m_alcModeEnabled ) {
         addDependentPropertyName ( "LithoStaticPressure" );
      }

   } else {

      if ( m_alcModeEnabled ) {
         addDependentPropertyName ( "Temperature" );
         addDependentPropertyName ( "LithoStaticPressure" );
      }

   }

}


void DerivedProperties::BulkDensityFormationCalculator::calculate ( DerivedProperties::AbstractPropertyManager& propertyManager,
                                                                    const DataModel::AbstractSnapshot*          snapshot,
                                                                    const DataModel::AbstractFormation*         formation,
                                                                          FormationPropertyList&                derivedProperties ) const {

   const GeoPhysics::Formation* geoFormation = dynamic_cast<const GeoPhysics::Formation*>( formation );

   if ( geoFormation != 0 ) {

      if ( geoFormation->kind () == DataAccess::Interface::BASEMENT_FORMATION ) {

         if ( m_alcModeEnabled ) {
            computeBulkDensityBasementNonAlc ( propertyManager, snapshot, geoFormation, derivedProperties );
         } else {
            computeBulkDensityBasementAlc ( propertyManager, snapshot, geoFormation, derivedProperties );
         }

      } else {

         // The sediment calculation does not depend on the alc mode.
         if ( m_coupledModeEnabled ) {
            computeBulkDensitySedimentsCoupled ( propertyManager, snapshot, geoFormation, derivedProperties );
         } else {
            computeBulkDensitySedimentsHydrostatic ( propertyManager, snapshot, geoFormation, derivedProperties );
         }

      }

   }

}

void DerivedProperties::BulkDensityFormationCalculator::computeBulkDensitySedimentsHydrostatic ( DerivedProperties::AbstractPropertyManager& propertyManager,
                                                                                                 const DataModel::AbstractSnapshot*          snapshot,
                                                                                                 const GeoPhysics::Formation*                formation,
                                                                                                       FormationPropertyList&                derivedProperties ) const {

   const DataModel::AbstractProperty* bulkDensityProperty  = propertyManager.getProperty ( "BulkDensity" );
   const DataModel::AbstractProperty* porosityProperty     = propertyManager.getProperty ( "Porosity" );

   const FormationPropertyPtr layerPorosity = propertyManager.getFormationProperty ( porosityProperty, snapshot, formation );

   if ( layerPorosity != 0 ) {

      DerivedFormationPropertyPtr bulkDensity = DerivedFormationPropertyPtr ( new DerivedProperties::DerivedFormationProperty ( bulkDensityProperty,
                                                                                                                                snapshot,
                                                                                                                                formation, 
                                                                                                                                propertyManager.getMapGrid (),
                                                                                                                                formation->getMaximumNumberOfElements() + 1 ));

      const GeoPhysics::FluidType* fluid = dynamic_cast<const GeoPhysics::FluidType*>(formation->getFluidType ());

      const double temperatureGradient = 0.001 * m_projectHandle->getRunParameters ()->getTemperatureGradient ();
      double fluidDensity = fluidDensity = fluid->getCorrectedSimpleDensity ( GeoPhysics::FluidType::DefaultStandardDepth,
                                                                              GeoPhysics::FluidType::DefaultHydrostaticPressureGradient,
                                                                              GeoPhysics::FluidType::StandardSurfaceTemperature,
                                                                              temperatureGradient );
      double undefinedValue = bulkDensity->getUndefinedValue ();
      double solidDensity;
      double currentTime = snapshot->getTime ();
      const GeoPhysics::CompoundLithologyArray& lithologies = formation->getCompoundLithologyArray ();

      for ( unsigned int i = bulkDensity->firstI ( true ); i <= bulkDensity->lastI ( true ); ++i ) {
            
         for ( unsigned int j = bulkDensity->firstJ ( true ); j <= bulkDensity->lastJ ( true ); ++j ) {

            if ( m_projectHandle->getNodeIsValid ( i, j )) {
               solidDensity = lithologies ( i, j, currentTime )->density ();

               for ( unsigned int k = bulkDensity->firstK (); k <= bulkDensity->lastK (); ++k ) {
                  double porosity = 0.01 * layerPorosity->get ( i, j, k );
                  bulkDensity->set ( i, j, k,  ( 1.0 - porosity ) * solidDensity + porosity * fluidDensity );
               }

            } else {

               for ( unsigned int k = bulkDensity->firstK (); k <= bulkDensity->lastK (); ++k ) {
                  bulkDensity->set ( i, j, k, undefinedValue );
               }

            }

         }

      }

      derivedProperties.push_back ( bulkDensity );
   }

}


void DerivedProperties::BulkDensityFormationCalculator::computeBulkDensitySedimentsCoupled ( DerivedProperties::AbstractPropertyManager& propertyManager,
                                                                                             const DataModel::AbstractSnapshot*          snapshot,
                                                                                             const GeoPhysics::Formation*                formation,
                                                                                                   FormationPropertyList&                derivedProperties ) const {

   const DataModel::AbstractProperty* bulkDensityProperty  = propertyManager.getProperty ( "BulkDensity" );
   const DataModel::AbstractProperty* temperatureProperty  = propertyManager.getProperty ( "Temperature" );
   const DataModel::AbstractProperty* porePressureProperty = propertyManager.getProperty ( "Pressure" );
   const DataModel::AbstractProperty* porosityProperty     = propertyManager.getProperty ( "Porosity" );
   
   const FormationPropertyPtr temperature         = propertyManager.getFormationProperty ( temperatureProperty,  snapshot, formation );
   const FormationPropertyPtr porePressure        = propertyManager.getFormationProperty ( porePressureProperty, snapshot, formation );
   const FormationPropertyPtr layerPorosity       = propertyManager.getFormationProperty ( porosityProperty,     snapshot, formation );

   if ( temperature != 0 and porePressure != 0 and layerPorosity != 0 ) {

      DerivedFormationPropertyPtr bulkDensity = DerivedFormationPropertyPtr ( new DerivedProperties::DerivedFormationProperty ( bulkDensityProperty,
                                                                                                                                snapshot,
                                                                                                                                formation, 
                                                                                                                                propertyManager.getMapGrid (),
                                                                                                                                formation->getMaximumNumberOfElements() + 1 ));

      const GeoPhysics::FluidType* fluid = dynamic_cast<const GeoPhysics::FluidType*>(formation->getFluidType ());
      const GeoPhysics::CompoundLithologyArray& lithologies = formation->getCompoundLithologyArray ();
      double currentTime = snapshot->getTime ();
      double undefinedValue = bulkDensity->getUndefinedValue ();

      for ( unsigned int i = bulkDensity->firstI ( true ); i <= bulkDensity->lastI ( true ); ++i ) {

         for ( unsigned int j = bulkDensity->firstJ ( true ); j <= bulkDensity->lastJ ( true ); ++j ) {

            if ( m_projectHandle->getNodeIsValid ( i, j )) {
               double solidDensity = lithologies ( i, j, currentTime )->density ();

               for ( unsigned int k = bulkDensity->firstK (); k <= bulkDensity->lastK (); ++k ) {
                  double porosity = 0.01 * layerPorosity->get ( i, j, k );
                  double fluidDensity = fluid->density ( temperature->get ( i, j, k ), porePressure->get ( i, j, k ));
                  bulkDensity->set ( i, j, k, ( 1.0 - porosity ) * solidDensity + porosity * fluidDensity );
               }

            } else {

               for ( unsigned int k = bulkDensity->firstK (); k <= bulkDensity->lastK (); ++k ) {
                  bulkDensity->set ( i, j, k, undefinedValue );
               }

            }

         }

      }

      derivedProperties.push_back ( bulkDensity );
   }

}

void DerivedProperties::BulkDensityFormationCalculator::computeBulkDensityBasementNonAlc ( DerivedProperties::AbstractPropertyManager& propertyManager,
                                                                                           const DataModel::AbstractSnapshot*          snapshot,
                                                                                           const GeoPhysics::Formation*                formation,
                                                                                                 FormationPropertyList&                derivedProperties ) const {


   const DataModel::AbstractProperty* bulkDensityProperty  = propertyManager.getProperty ( "BulkDensity" );

   DerivedFormationPropertyPtr bulkDensity = DerivedFormationPropertyPtr ( new DerivedProperties::DerivedFormationProperty ( bulkDensityProperty,
                                                                                                                             snapshot,
                                                                                                                             formation, 
                                                                                                                             propertyManager.getMapGrid (),
                                                                                                                             formation->getMaximumNumberOfElements() + 1 ));

   const GeoPhysics::CompoundLithologyArray& lithologies = formation->getCompoundLithologyArray ();
   // In non alc mode the crust and mantle have the same lithology in all locations, take the first lithology.
   double solidDensity = lithologies ( bulkDensity->firstI ( false ), bulkDensity->firstJ ( false ))->density ();
   double undefinedValue = bulkDensity->getUndefinedValue ();

   for ( unsigned int i = bulkDensity->firstI ( true ); i <= bulkDensity->lastI ( true ); ++i ) {

      for ( unsigned int j = bulkDensity->firstJ ( true ); j <= bulkDensity->lastJ ( true ); ++j ) {

         if ( m_projectHandle->getNodeIsValid ( i, j )) {

            for ( unsigned int k = bulkDensity->firstK (); k <= bulkDensity->lastK (); ++k ) {
               bulkDensity->set ( i, j, k, solidDensity );
            }

         } else {

            for ( unsigned int k = bulkDensity->firstK (); k <= bulkDensity->lastK (); ++k ) {
               bulkDensity->set ( i, j, k, undefinedValue );
            }

         }

      }

   }

   derivedProperties.push_back ( bulkDensity );
}

void DerivedProperties::BulkDensityFormationCalculator::computeBulkDensityBasementAlc ( DerivedProperties::AbstractPropertyManager& propertyManager,
                                                                                        const DataModel::AbstractSnapshot*          snapshot,
                                                                                        const GeoPhysics::Formation*                formation,
                                                                                              FormationPropertyList&                derivedProperties ) const {

   const DataModel::AbstractProperty* bulkDensityProperty  = propertyManager.getProperty ( "BulkDensity" );
   const DataModel::AbstractProperty* temperatureProperty  = propertyManager.getProperty ( "Temperature" );
   const DataModel::AbstractProperty* lithostaticProperty  = propertyManager.getProperty ( "LithoStaticPressure" );
   
   const FormationPropertyPtr temperature         = propertyManager.getFormationProperty ( temperatureProperty,  snapshot, formation );
   const FormationPropertyPtr lithostaticPressure = propertyManager.getFormationProperty ( lithostaticProperty,  snapshot, formation );

   if ( temperature != 0 and lithostaticPressure != 0 ) {
      DerivedFormationPropertyPtr bulkDensity = DerivedFormationPropertyPtr ( new DerivedProperties::DerivedFormationProperty ( bulkDensityProperty,
                                                                                                                                snapshot,
                                                                                                                                formation, 
                                                                                                                                propertyManager.getMapGrid (),
                                                                                                                                formation->getMaximumNumberOfElements() + 1 ));
      const GeoPhysics::CompoundLithologyArray& lithologies = formation->getCompoundLithologyArray ();
      double undefinedValue = bulkDensity->getUndefinedValue ();

      for ( unsigned int i = bulkDensity->firstI ( true ); i <= bulkDensity->lastI ( true ); ++i ) {

         for ( unsigned int j = bulkDensity->firstJ ( true ); j <= bulkDensity->lastJ ( true ); ++j ) {

            if ( m_projectHandle->getNodeIsValid ( i, j )) {

               for ( unsigned int k = bulkDensity->firstK (); k <= bulkDensity->lastK (); ++k ) {
                  double solidDensity = lithologies ( i, j )->computeDensity ( temperature->get ( i, j, k ), lithostaticPressure->get ( i, j, k ));
                  bulkDensity->set ( i, j, k, solidDensity );
               }

            } else {

               for ( unsigned int k = bulkDensity->firstK (); k <= bulkDensity->lastK (); ++k ) {
                  bulkDensity->set ( i, j, k, undefinedValue );
               }

            }

         }

      }

      derivedProperties.push_back ( bulkDensity );
   }

}

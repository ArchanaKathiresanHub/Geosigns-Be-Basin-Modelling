//
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "AbstractPropertyManager.h"
#include "DerivedFormationProperty.h"
#include "DerivedPropertyManager.h"

#include "Interface.h"
#include "RunParameters.h"
#include "SimulationDetails.h"
#include "Surface.h"

#include "GeoPhysicsFormation.h"
#include "CompoundLithologyArray.h"

#include "PropertyRetriever.h"
#include "BulkDensityFormationCalculator.h"

using namespace AbstractDerivedProperties;

DerivedProperties::BulkDensityFormationCalculator::BulkDensityFormationCalculator ( const GeoPhysics::ProjectHandle& projectHandle ) : m_projectHandle ( projectHandle ) {
   addPropertyName ( "BulkDensity" );

   const DataAccess::Interface::SimulationDetails* lastFastcauldronRun = m_projectHandle.getDetailsOfLastFastcauldron ();

   if ( lastFastcauldronRun != 0 ) {
      m_coupledModeEnabled = lastFastcauldronRun->getSimulatorMode () == "LooselyCoupledTemperature" or
                             lastFastcauldronRun->getSimulatorMode () == "CoupledHighResDecompaction" or
                             lastFastcauldronRun->getSimulatorMode () == "CoupledPressureAndTemperature" or
                             lastFastcauldronRun->getSimulatorMode () == "CoupledDarcy";
   } else {
      m_coupledModeEnabled = false;
   }

   m_alcModeEnabled = m_projectHandle.isALC ();

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
   if ( m_alcModeEnabled ) {
      addDependentPropertyName( "ALCStepTopBasaltDepth");
      addDependentPropertyName( "ALCStepBasaltThickness");
      addDependentPropertyName( "Depth" );
   }
}


void DerivedProperties::BulkDensityFormationCalculator::calculate ( AbstractPropertyManager&            propertyManager,
                                                                    const DataModel::AbstractSnapshot*  snapshot,
                                                                    const DataModel::AbstractFormation* formation,
                                                                    FormationPropertyList&              derivedProperties ) const {

   const GeoPhysics::GeoPhysicsFormation* geoFormation = dynamic_cast<const GeoPhysics::GeoPhysicsFormation*>( formation );

   if ( geoFormation != 0 ) {

      if ( geoFormation->kind () == DataAccess::Interface::BASEMENT_FORMATION ) {

         if ( m_alcModeEnabled ) {
            computeBulkDensityBasementAlc ( propertyManager, snapshot, geoFormation, derivedProperties );
         } else {
            computeBulkDensityBasementNonAlc ( propertyManager, snapshot, geoFormation, derivedProperties );
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

void DerivedProperties::BulkDensityFormationCalculator::computeBulkDensitySedimentsHydrostatic ( AbstractPropertyManager&           propertyManager,
                                                                                                 const DataModel::AbstractSnapshot* snapshot,
                                                                                                 const GeoPhysics::GeoPhysicsFormation*       formation,
                                                                                                 FormationPropertyList&             derivedProperties ) const {

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

      const GeoPhysics::CompoundLithologyArray& lithologies = formation->getCompoundLithologyArray ();
      const double temperatureGradient = 0.001 * m_projectHandle.getRunParameters ()->getTemperatureGradient ();
      const double fluidDensity = fluid->getCorrectedSimpleDensity ( GeoPhysics::FluidType::DefaultStandardDepth,
                                                                     GeoPhysics::FluidType::DefaultHydrostaticPressureGradient,
                                                                     GeoPhysics::FluidType::StandardSurfaceTemperature,
                                                                     temperatureGradient );
      const double undefinedValue = bulkDensity->getUndefinedValue ();
      const double currentTime = snapshot->getTime ();
      double solidDensity;

      for ( unsigned int i = bulkDensity->firstI ( true ); i <= bulkDensity->lastI ( true ); ++i ) {

         for ( unsigned int j = bulkDensity->firstJ ( true ); j <= bulkDensity->lastJ ( true ); ++j ) {

            if ( m_projectHandle.getNodeIsValid ( i, j )) {
               solidDensity = lithologies ( i, j, currentTime )->density ();

               for ( unsigned int k = bulkDensity->firstK (); k <= bulkDensity->lastK (); ++k ) {
                  const double porosity = 0.01 * layerPorosity->get ( i, j, k );
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


void DerivedProperties::BulkDensityFormationCalculator::computeBulkDensitySedimentsCoupled ( AbstractPropertyManager&           propertyManager,
                                                                                             const DataModel::AbstractSnapshot* snapshot,
                                                                                             const GeoPhysics::GeoPhysicsFormation*       formation,
                                                                                             FormationPropertyList&             derivedProperties ) const {

   const DataModel::AbstractProperty* bulkDensityProperty  = propertyManager.getProperty ( "BulkDensity" );
   const DataModel::AbstractProperty* temperatureProperty  = propertyManager.getProperty ( "Temperature" );
   const DataModel::AbstractProperty* porePressureProperty = propertyManager.getProperty ( "Pressure" );
   const DataModel::AbstractProperty* porosityProperty     = propertyManager.getProperty ( "Porosity" );

   const FormationPropertyPtr temperature         = propertyManager.getFormationProperty ( temperatureProperty,  snapshot, formation );
   const FormationPropertyPtr porePressure        = propertyManager.getFormationProperty ( porePressureProperty, snapshot, formation );
   const FormationPropertyPtr layerPorosity       = propertyManager.getFormationProperty ( porosityProperty,     snapshot, formation );

   if ( temperature != 0 and porePressure != 0 and layerPorosity != 0 ) {

      PropertyRetriever temperatureRetriever ( temperature );
      PropertyRetriever porePressureRetriever ( porePressure );

      DerivedFormationPropertyPtr bulkDensity = DerivedFormationPropertyPtr ( new DerivedProperties::DerivedFormationProperty ( bulkDensityProperty,
                                                                                                                                snapshot,
                                                                                                                                formation,
                                                                                                                                propertyManager.getMapGrid (),
                                                                                                                                formation->getMaximumNumberOfElements() + 1 ));

      const GeoPhysics::FluidType* fluid = dynamic_cast<const GeoPhysics::FluidType*>(formation->getFluidType ());
      const GeoPhysics::CompoundLithologyArray& lithologies = formation->getCompoundLithologyArray ();
      const double currentTime = snapshot->getTime ();
      const double undefinedValue = bulkDensity->getUndefinedValue ();

      for ( unsigned int i = bulkDensity->firstI ( true ); i <= bulkDensity->lastI ( true ); ++i ) {

         for ( unsigned int j = bulkDensity->firstJ ( true ); j <= bulkDensity->lastJ ( true ); ++j ) {

            if ( m_projectHandle.getNodeIsValid ( i, j )) {
               double solidDensity = lithologies ( i, j, currentTime )->density ();

               for ( unsigned int k = bulkDensity->firstK (); k <= bulkDensity->lastK (); ++k ) {
                  const double porosity = 0.01 * layerPorosity->get ( i, j, k );
                  const double fluidDensity = fluid->density ( temperature->get ( i, j, k ), porePressure->get ( i, j, k ));
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

void DerivedProperties::BulkDensityFormationCalculator::computeBulkDensityBasementNonAlc ( AbstractPropertyManager&           propertyManager,
                                                                                           const DataModel::AbstractSnapshot* snapshot,
                                                                                           const GeoPhysics::GeoPhysicsFormation*       formation,
                                                                                           FormationPropertyList&             derivedProperties ) const {


   const DataModel::AbstractProperty* bulkDensityProperty  = propertyManager.getProperty ( "BulkDensity" );

   DerivedFormationPropertyPtr bulkDensity = DerivedFormationPropertyPtr ( new DerivedProperties::DerivedFormationProperty ( bulkDensityProperty,
                                                                                                                             snapshot,
                                                                                                                             formation,
                                                                                                                             propertyManager.getMapGrid (),
                                                                                                                             formation->getMaximumNumberOfElements() + 1 ));

   const GeoPhysics::CompoundLithologyArray& lithologies = formation->getCompoundLithologyArray ();
   // In non alc mode the crust and mantle have the same lithology in all locations, take the first lithology.
   const double solidDensity = lithologies ( bulkDensity->firstI ( false ), bulkDensity->firstJ ( false ))->density ();
   const double undefinedValue = bulkDensity->getUndefinedValue ();

   for ( unsigned int i = bulkDensity->firstI ( true ); i <= bulkDensity->lastI ( true ); ++i ) {

      for ( unsigned int j = bulkDensity->firstJ ( true ); j <= bulkDensity->lastJ ( true ); ++j ) {

         if ( m_projectHandle.getNodeIsValid ( i, j )) {

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

void DerivedProperties::BulkDensityFormationCalculator::computeBulkDensityBasementAlc ( AbstractPropertyManager&           propertyManager,
                                                                                        const DataModel::AbstractSnapshot* snapshot,
                                                                                        const GeoPhysics::GeoPhysicsFormation*       formation,
                                                                                        FormationPropertyList&             derivedProperties ) const {

   const DataModel::AbstractProperty* bulkDensityProperty  = propertyManager.getProperty ( "BulkDensity" );
   const DataModel::AbstractProperty* temperatureProperty  = propertyManager.getProperty ( "Temperature" );
   const DataModel::AbstractProperty* lithostaticProperty  = propertyManager.getProperty ( "LithoStaticPressure" );
   const DataModel::AbstractProperty* depthProperty        = propertyManager.getProperty ( "Depth" );

   const FormationPropertyPtr temperature         = propertyManager.getFormationProperty ( temperatureProperty,  snapshot, formation );
   const FormationPropertyPtr lithostaticPressure = propertyManager.getFormationProperty ( lithostaticProperty,  snapshot, formation );
   const FormationPropertyPtr depth               = propertyManager.getFormationProperty ( depthProperty, snapshot, formation );

   FormationMapPropertyPtr basaltDepth;
   FormationMapPropertyPtr basaltThickness;

   const DataModel::AbstractProperty* basaltDepthProperty = propertyManager.getProperty ( "ALCStepTopBasaltDepth" );
   const DataModel::AbstractProperty* basaltThicknessProperty = propertyManager.getProperty ( "ALCStepBasaltThickness" );

   if( formation->getName() != "Crust" ) {
      const GeoPhysics::GeoPhysicsFormation *mantleFormation = dynamic_cast<const GeoPhysics::GeoPhysicsFormation*>( formation );
      const DataModel::AbstractFormation * crustFormation = ( mantleFormation->getTopSurface()->getTopFormation() );

      basaltDepth = propertyManager.getFormationMapProperty ( basaltDepthProperty, snapshot, crustFormation );
      basaltThickness = propertyManager.getFormationMapProperty ( basaltThicknessProperty, snapshot, crustFormation );
   } else {
      basaltDepth = propertyManager.getFormationMapProperty ( basaltDepthProperty, snapshot, formation );
      basaltThickness = propertyManager.getFormationMapProperty ( basaltThicknessProperty, snapshot, formation );
   }

  if ( temperature != 0 and lithostaticPressure != 0 and depth != 0 and basaltDepth != 0 and basaltThickness != 0 ) {

    PropertyRetriever basaltDepthRetriever(basaltDepth);
    PropertyRetriever basaltThicknessRetriever(basaltThickness);
    PropertyRetriever temperatureRetriever(temperature);
    PropertyRetriever lithostaticPressureRetriever(lithostaticPressure);
    PropertyRetriever depthRetriever(depth);

      DerivedFormationPropertyPtr bulkDensity = DerivedFormationPropertyPtr ( new DerivedProperties::DerivedFormationProperty ( bulkDensityProperty,
                                                                                                                                snapshot,
                                                                                                                                formation,
                                                                                                                                propertyManager.getMapGrid (),
                                                                                                                                formation->getMaximumNumberOfElements() + 1 ));
      const GeoPhysics::CompoundLithologyArray& lithologies = formation->getCompoundLithologyArray ();
      const double undefinedValue = bulkDensity->getUndefinedValue ();

      for ( unsigned int i = bulkDensity->firstI ( true ); i <= bulkDensity->lastI ( true ); ++i ) {

         for ( unsigned int j = bulkDensity->firstJ ( true ); j <= bulkDensity->lastJ ( true ); ++j ) {

            if ( m_projectHandle.getNodeIsValid ( i, j )) {

               for ( unsigned int k = bulkDensity->firstK (); k <= bulkDensity->lastK (); ++k ) {
                  const GeoPhysics::CompoundLithology* lithology = lithologies ( i, j, snapshot->getTime () );

                  const double topBasaltDepth = basaltDepth->get( i, j );
                  const double botBasaltDepth = topBasaltDepth + 1 + basaltThickness->get( i, j );
                  double solidDensity;

                  if( basaltThickness->get( i, j ) != 0 and ( topBasaltDepth <= depth->get ( i, j, k ) and botBasaltDepth > depth->get ( i, j, k  ))) {

                     solidDensity = lithology->getSimpleLithology()->getBasaltDensity ( temperature->get ( i, j, k ), lithostaticPressure->get ( i, j, k ));
                  } else {
                     solidDensity = lithology->computeDensity ( temperature->get ( i, j, k ), lithostaticPressure->get ( i, j, k ));
                  }
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

bool DerivedProperties::BulkDensityFormationCalculator::isComputable ( const AbstractPropertyManager&      propManager,
                                                                       const DataModel::AbstractSnapshot*  snapshot,
                                                                       const DataModel::AbstractFormation* formation ) const {

   bool basementFormation = ( dynamic_cast<const GeoPhysics::GeoPhysicsFormation*>( formation ) != 0 and dynamic_cast<const GeoPhysics::GeoPhysicsFormation*>( formation )->kind () == DataAccess::Interface::BASEMENT_FORMATION );

   if( basementFormation ) {
      return isComputableForBasement ( propManager, snapshot, formation ) ;
   }

   const std::vector<std::string>& dependentProperties = getDependentPropertyNames ();

   bool propertyIsComputable = true;

   // Determine if the required properties are computable.
   for ( size_t i = 0; i < dependentProperties.size () and propertyIsComputable; ++i ) {

      if( dependentProperties [ i ] == "ALCStepTopBasaltDepth" or dependentProperties [ i ] == "ALCStepBasaltThickness" or
          dependentProperties [ i ] == "Depth" or dependentProperties [ i ] == "LithoStaticPressure" ) {
         propertyIsComputable = true;
      } else {

         const DataModel::AbstractProperty* property = propManager.getProperty ( dependentProperties [ i ]);

         if ( property == 0 ) {
            propertyIsComputable = false;
         } else {
            propertyIsComputable = propertyIsComputable and propManager.formationPropertyIsComputable ( property, snapshot, formation );
         }
      }
   }

   return propertyIsComputable;
}

bool DerivedProperties::BulkDensityFormationCalculator::isComputableForBasement ( const AbstractPropertyManager&      propManager,
                                                                                  const DataModel::AbstractSnapshot*  snapshot,
                                                                                  const DataModel::AbstractFormation* formation ) const {

   const std::vector<std::string>& dependentProperties = getDependentPropertyNames ();

   bool propertyIsComputable = true;

   // Determine if the required properties are computable.
   for ( size_t i = 0; i < dependentProperties.size () and propertyIsComputable; ++i ) {

      if( not m_alcModeEnabled ) {

         return true;
      }
      if( dependentProperties [ i ] == "Porosity" or dependentProperties [ i ] == "Pressure" ) {
         propertyIsComputable = true;
      } else {
         const DataModel::AbstractProperty* property = propManager.getProperty ( dependentProperties [ i ]);

         if ( property == 0 ) {
            propertyIsComputable = false;
         } else {
            if( dependentProperties [ i ] == "ALCStepTopBasaltDepth" or dependentProperties [ i ] == "ALCStepBasaltThickness" ) {
               if( formation->getName() != "Crust" ) {
                  const GeoPhysics::GeoPhysicsFormation *mantleFormation = dynamic_cast<const GeoPhysics::GeoPhysicsFormation*>( formation );

                  const DataModel::AbstractFormation * crustFormation = (mantleFormation->getTopSurface()->getTopFormation() );
                  propertyIsComputable = propertyIsComputable and propManager.formationMapPropertyIsComputable ( property, snapshot, crustFormation );
               } else {
                  propertyIsComputable = propertyIsComputable and propManager.formationMapPropertyIsComputable ( property, snapshot, formation );
               }
            } else {
               propertyIsComputable = propertyIsComputable and propManager.formationPropertyIsComputable ( property, snapshot, formation );
            }
         }
      }
   }
   return propertyIsComputable;
}

//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 
#include "HydrostaticPressureFormationCalculator.h"

#include "Interface/Surface.h"
#include "Interface/Snapshot.h"
#include "Interface/RunParameters.h"
#include "Interface/SimulationDetails.h"

#include "GeoPhysicsFormation.h"
#include "GeoPhysicalConstants.h"
#include "GeoPhysicalFunctions.h"
#include "PropertyRetriever.h"

// utilitites library
#include "ConstantsPhysics.h"
#include "ConstantsMathematics.h"

DerivedProperties::HydrostaticPressureFormationCalculator::HydrostaticPressureFormationCalculator ( const GeoPhysics::ProjectHandle* projectHandle ) :
   m_projectHandle ( projectHandle )
{
   addPropertyName ( "HydroStaticPressure" );

   const DataAccess::Interface::SimulationDetails* lastFastcauldronRun = m_projectHandle->getDetailsOfLastSimulation ( "fastcauldron" );

   m_hydrostaticDecompactionMode = false;
   m_hydrostaticMode             = false;
   m_opMode                      = false;

   if ( lastFastcauldronRun != 0 ) {
      m_hydrostaticDecompactionMode = lastFastcauldronRun->getSimulatorMode () == "HydrostaticDecompaction" ;
      
      m_hydrostaticMode = lastFastcauldronRun->getSimulatorMode () == "HydrostaticTemperature";

      m_opMode = lastFastcauldronRun->getSimulatorMode () == "Overpressure";
   }

   if ( m_hydrostaticDecompactionMode ) {
      addDependentPropertyName ( "Depth" );
   } else if ( m_hydrostaticMode ) {
      addDependentPropertyName ( "Pressure" );
   } else if( m_opMode ) {
      addDependentPropertyName ( "Depth" );
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

   if ( m_hydrostaticDecompactionMode ) {
      computeHydrostaticPressureForDecomapctionMode ( propertyManager, snapshot, formation, derivedProperties );
   } else if ( m_hydrostaticMode ) {
      computeHydrostaticPressureForHydrostaticMode ( propertyManager, snapshot, formation, derivedProperties );
   } else {
      computeHydrostaticPressureForCoupledMode ( propertyManager, snapshot, formation, derivedProperties );
   }

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
void DerivedProperties::HydrostaticPressureFormationCalculator::computeHydrostaticPressureAtSeaBottomForHydrostatic ( const AbstractPropertyManager&     propertyManager,
                                                                                                                      const double                       snapshotAge,
                                                                                                                      const GeoPhysics::FluidType*       fluid,
                                                                                                                      DerivedFormationPropertyPtr& hydrostaticPressure ) const {

   (void) propertyManager;
   double pressure;
   unsigned int topNodeIndex = hydrostaticPressure->lastK ();

   double fluidDensity;

   double temperatureGradient = 0.001 * m_projectHandle->getRunParameters ()->getTemperatureGradient ();
   if( fluid == 0 ) {
      fluidDensity = 0.0;
   } else {
      fluidDensity = fluid->getCorrectedSimpleDensity ( GeoPhysics::FluidType::DefaultStandardDepth,
                                                        GeoPhysics::FluidType::DefaultHydrostaticPressureGradient,
                                                        GeoPhysics::FluidType::StandardSurfaceTemperature,
                                                        temperatureGradient );
   }
   for ( unsigned int i = hydrostaticPressure->firstI ( true ); i <= hydrostaticPressure->lastI ( true ); ++i ) {

      for ( unsigned int j = hydrostaticPressure->firstJ ( true ); j <= hydrostaticPressure->lastJ ( true ); ++j ) {

         if ( m_projectHandle->getNodeIsValid ( i, j )) {
            GeoPhysics::computeHydrostaticPressureSimpleDensity ( fluid,
                                                                  fluidDensity,
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
            hydrostaticPressure->set ( i, j, topNodeIndex, hydrostaticPressureAbove->getA ( i, j, 0 ));
         } else {
            hydrostaticPressure->set ( i, j, topNodeIndex, undefinedValue );
         }

      }

   }

}

void DerivedProperties::HydrostaticPressureFormationCalculator::computeHydrostaticPressureForDecomapctionMode ( AbstractPropertyManager&            propertyManager,
                                                                                                                const DataModel::AbstractSnapshot*  snapshot,
                                                                                                                const DataModel::AbstractFormation* formation,
                                                                                                                FormationPropertyList&              derivedProperties ) const {

   const GeoPhysics::Formation* currentFormation = dynamic_cast<const GeoPhysics::Formation*>( formation );

   const bool basementFormation = ( currentFormation->kind () == DataAccess::Interface::BASEMENT_FORMATION );
 
   if ( basementFormation ) {
      return computeForBasement( propertyManager, snapshot, formation, derivedProperties );
   }  
   
   const DataModel::AbstractProperty* hydrostaticPressureProperty = propertyManager.getProperty ( getPropertyNames ()[ 0 ]);
   DerivedFormationPropertyPtr hydrostaticPressure = DerivedFormationPropertyPtr ( new DerivedProperties::DerivedFormationProperty ( hydrostaticPressureProperty, snapshot, formation, 
                                                                                                                                     propertyManager.getMapGrid (),
                                                                                                                                     currentFormation->getMaximumNumberOfElements() + 1 ));

      
   const DataModel::AbstractProperty* depthProperty = propertyManager.getProperty ( "Depth" );
   FormationPropertyPtr depth = propertyManager.getFormationProperty ( depthProperty, snapshot, formation );

   if( depth != 0 ) {
      PropertyRetriever depthRetriever ( depth );
      
      const GeoPhysics::Formation* formationAbove = 0;
      
      const GeoPhysics::FluidType* fluid = dynamic_cast<const GeoPhysics::FluidType*>(currentFormation->getFluidType ());
      
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
      } else {
         double temperatureGradient = 0.001 * m_projectHandle->getRunParameters ()->getTemperatureGradient ();
         fluidDensity = fluid->getCorrectedSimpleDensity ( GeoPhysics::FluidType::DefaultStandardDepth,
                                                           GeoPhysics::FluidType::DefaultHydrostaticPressureGradient,
                                                           GeoPhysics::FluidType::StandardSurfaceTemperature,
                                                           temperatureGradient );
      }
      
      // Initialise the top set of nodes for the hydrostatic pressure.
      if ( formationAbove == 0 ) {
         computeHydrostaticPressureAtSeaBottomForHydrostatic ( propertyManager, snapshot->getTime (), fluid, hydrostaticPressure );
      } else {
         copyHydrostaticPressureFromLayerAbove ( propertyManager, hydrostaticPressureProperty, snapshot, formationAbove, hydrostaticPressure );
      }
      
      // now that the top of the set of nodes of the property has been initialised 
      // the hydrostatic pressure for the remaining nodes below them can be computed.
      for ( unsigned int i = hydrostaticPressure->firstI ( true ); i <= hydrostaticPressure->lastI ( true ); ++i ) {
         
         for ( unsigned int j = hydrostaticPressure->firstJ ( true ); j <= hydrostaticPressure->lastJ ( true ); ++j ) {
            
            if ( m_projectHandle->getNodeIsValid ( i, j )) {
               
               // Loop index is shifted up by 1.
               for ( unsigned int k = hydrostaticPressure->lastK (); k > hydrostaticPressure->firstK (); --k ) {
                  // index k     is top node of segment
                  // index k - 1 is bottom node of segment
                  
                  thickness = depth->getA ( i, j, k - 1 ) - depth->getA ( i, j, k );
                  
                  segmentPressure = thickness * fluidDensity * Utilities::Physics::AccelerationDueToGravity * Utilities::Maths::PaToMegaPa;
                  pressure = hydrostaticPressure->getA ( i, j, k ) + segmentPressure;
                  hydrostaticPressure->set ( i, j, k - 1, pressure );
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
}


void DerivedProperties::HydrostaticPressureFormationCalculator::computeHydrostaticPressureForHydrostaticMode ( AbstractPropertyManager&            propertyManager,
                                                                                                               const DataModel::AbstractSnapshot*  snapshot,
                                                                                                               const DataModel::AbstractFormation* formation,
                                                                                                               FormationPropertyList&              derivedProperties ) const {

   const GeoPhysics::Formation* currentFormation = dynamic_cast<const GeoPhysics::Formation*>( formation );
 
   const bool basementFormation = ( currentFormation->kind () == DataAccess::Interface::BASEMENT_FORMATION );
 
   if ( basementFormation ) {
      return computeForBasement( propertyManager, snapshot, formation, derivedProperties );
   }  

   const DataModel::AbstractProperty* hydrostaticPressureProperty = propertyManager.getProperty ( getPropertyNames ()[ 0 ]);
   DerivedFormationPropertyPtr hydrostaticPressure = DerivedFormationPropertyPtr ( new DerivedProperties::DerivedFormationProperty ( hydrostaticPressureProperty, snapshot, formation, 
                                                                                                                                     propertyManager.getMapGrid (),
                                                                                                                                     currentFormation->getMaximumNumberOfElements() + 1 ));

   const DataModel::AbstractProperty* porePressureProperty = propertyManager.getProperty ( "Pressure" );
      
   FormationPropertyPtr porePressure = propertyManager.getFormationProperty ( porePressureProperty, snapshot, formation );

      
   derivedProperties.clear ();

   if( porePressure != 0 ) {
      PropertyRetriever ppRetriever ( porePressure );
      
      // now copy the pore pressure to the hydrostatic pressure
      
      for ( unsigned int i = hydrostaticPressure->firstI ( true ); i <= hydrostaticPressure->lastI ( true ); ++i ) {
         
         for ( unsigned int j = hydrostaticPressure->firstJ ( true ); j <= hydrostaticPressure->lastJ ( true ); ++j ) {
            
            for ( unsigned int k = hydrostaticPressure->firstK (); k <= hydrostaticPressure->lastK (); ++k ) {
               hydrostaticPressure->set ( i, j, k, porePressure->getA ( i, j, k ));
            }
            
         }
      }
      
      derivedProperties.push_back ( hydrostaticPressure );
   }
}

void DerivedProperties::HydrostaticPressureFormationCalculator::computeHydrostaticPressureForCoupledMode ( AbstractPropertyManager&            propertyManager,
                                                                                                           const DataModel::AbstractSnapshot*  snapshot,
                                                                                                           const DataModel::AbstractFormation* formation,
                                                                                                           FormationPropertyList&              derivedProperties ) const {

   const GeoPhysics::Formation* currentFormation = dynamic_cast<const GeoPhysics::Formation*>( formation );
 
   const bool basementFormation = ( currentFormation->kind () == DataAccess::Interface::BASEMENT_FORMATION );
 
   if ( basementFormation ) {
      return computeForBasement( propertyManager, snapshot, formation, derivedProperties );
   }  
  
   const DataModel::AbstractProperty* hydrostaticPressureProperty = propertyManager.getProperty ( getPropertyNames ()[ 0 ]);
   DerivedFormationPropertyPtr hydrostaticPressure = DerivedFormationPropertyPtr ( new DerivedProperties::DerivedFormationProperty ( hydrostaticPressureProperty, snapshot, formation, 
                                                                                                                                     propertyManager.getMapGrid (),
                                                                                                                                     currentFormation->getMaximumNumberOfElements() + 1 ));

   
   FormationPropertyPtr temperature;
   FormationPropertyPtr porePressure;

 
   const DataModel::AbstractProperty* porePressureProperty = propertyManager.getProperty ( "Pressure" );
   porePressure = propertyManager.getFormationProperty ( porePressureProperty, snapshot, formation );
      
   const DataModel::AbstractProperty* depthProperty = propertyManager.getProperty ( "Depth" );
   FormationPropertyPtr depth = propertyManager.getFormationProperty ( depthProperty, snapshot, formation );

   const DataModel::AbstractProperty* temperatureProperty = propertyManager.getProperty ( "Temperature" );
   temperature = propertyManager.getFormationProperty ( temperatureProperty, snapshot, formation );
   
   if( m_opMode and temperature == 0 and depth != 0 ) {
      DerivedFormationPropertyPtr temp = DerivedFormationPropertyPtr ( new DerivedProperties::DerivedFormationProperty ( temperatureProperty, snapshot, formation, 
                                                                                                    propertyManager.getMapGrid (),
                                                                                                    currentFormation->getMaximumNumberOfElements() + 1 ));

      computeEstimatedTemperature( snapshot->getTime(), depth, temp );
      
      temperature = temp;
   }
   
   PropertyRetriever temperatureRetriever;
   if( temperature != 0 ) {
      temperatureRetriever.reset( temperature );
   }

   PropertyRetriever ppRetriever ( porePressure );
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

   // Initialise the top set of nodes for the hydrostatic pressure.
   if ( formationAbove == 0 ) {
      computeHydrostaticPressureAtSeaBottom ( propertyManager, snapshot->getTime (), fluid, hydrostaticPressure );
   } else {
      copyHydrostaticPressureFromLayerAbove ( propertyManager, hydrostaticPressureProperty, snapshot, formationAbove, hydrostaticPressure );
   }

   
   if( temperature != 0 and porePressure != 0 and depth != 0 ) {
      
      
      // now that the top of the set of nodes of the property has been initialised 
      // the hydrostatic pressure for the remaining nodes below them can be computed.
      for ( unsigned int i = hydrostaticPressure->firstI ( true ); i <= hydrostaticPressure->lastI ( true ); ++i ) {
         
         for ( unsigned int j = hydrostaticPressure->firstJ ( true ); j <= hydrostaticPressure->lastJ ( true ); ++j ) {
            
            if ( m_projectHandle->getNodeIsValid ( i, j )) {
               
               fluidDensityTop = ( fluid == 0 ? 0.0 : fluid->density ( temperature->getA ( i, j, topNodeIndex ), porePressure->getA ( i, j, topNodeIndex )));
               
               // Loop index is shifted up by 1.
               for ( unsigned int k = hydrostaticPressure->lastK (); k > hydrostaticPressure->firstK (); --k ) {
                  // index k     is top node of segment
                  // index k - 1 is bottom node of segment
                  
                  thickness = depth->getA ( i, j, k - 1 ) - depth->getA ( i, j, k );
                  fluidDensityBottom = ( fluid == 0 ? 0.0 : 
                                         fluid->density ( temperature->getA ( i, j, k - 1 ), porePressure->getA ( i, j, k - 1 )));
                  segmentPressure = 0.5 * thickness * ( fluidDensityTop + fluidDensityBottom ) * Utilities::Physics::AccelerationDueToGravity * Utilities::Maths::PaToMegaPa;
                  
                  pressure = hydrostaticPressure->getA ( i, j, k ) + segmentPressure;
                  
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

}

void DerivedProperties::HydrostaticPressureFormationCalculator::computeForBasement ( AbstractPropertyManager&            propertyManager,
                                                                                     const DataModel::AbstractSnapshot*  snapshot,
                                                                                     const DataModel::AbstractFormation* formation,
                                                                                     FormationPropertyList&              derivedProperties ) const {
   
   const GeoPhysics::Formation* currentFormation = dynamic_cast<const GeoPhysics::Formation*>( formation );
 
   const bool basementFormation = ( currentFormation->kind () == DataAccess::Interface::BASEMENT_FORMATION );
 
   if ( not basementFormation ) {
      return;
   }  

   const DataModel::AbstractProperty* hydrostaticPressureProperty = propertyManager.getProperty ( getPropertyNames ()[ 0 ]);
   DerivedFormationPropertyPtr hydrostaticPressure = DerivedFormationPropertyPtr ( new DerivedProperties::DerivedFormationProperty ( hydrostaticPressureProperty, snapshot, formation, 
                                                                                                                                     propertyManager.getMapGrid (),
                                                                                                                                     currentFormation->getMaximumNumberOfElements() + 1 ));

    derivedProperties.clear ();

    double undefinedValue = hydrostaticPressure->getUndefinedValue ();
 
    for ( unsigned int i = hydrostaticPressure->firstI ( true ); i <= hydrostaticPressure->lastI ( true ); ++i ) {
       
       for ( unsigned int j = hydrostaticPressure->firstJ ( true ); j <= hydrostaticPressure->lastJ ( true ); ++j ) {

          if ( m_projectHandle->getNodeIsValid ( i, j )) {
        
             for ( unsigned int k = hydrostaticPressure->firstK (); k <= hydrostaticPressure->lastK (); ++k ) {
                hydrostaticPressure->set ( i, j, k, 0.0 );
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

void DerivedProperties::HydrostaticPressureFormationCalculator::computeEstimatedTemperature ( const double currentTime,
                                                                                              const FormationPropertyPtr& depth,
                                                                                              DerivedFormationPropertyPtr& temperature ) const {

   unsigned int topNodeIndex  = temperature->lastK ();
   double temperatureGradient = 0.001 * m_projectHandle->getRunParameters ()->getTemperatureGradient ();
   double surfaceDepth, surfaceTemperature;
   
   double estimatedTemperature;
   double realDepth;
 
   for ( unsigned int i = temperature->firstI ( true ); i <= temperature->lastI ( true ); ++i ) {

      for ( unsigned int j = temperature->firstJ ( true ); j <= temperature->lastJ ( true ); ++j ) {

         if ( m_projectHandle->getNodeIsValid ( i, j )) {
            surfaceDepth        = m_projectHandle->getSeaBottomTemperature ( i, j, currentTime );
            surfaceTemperature  = m_projectHandle->getSeaBottomDepth ( i, j, currentTime );

            for ( unsigned int k = temperature->lastK (); k > temperature->firstK (); --k ) {

               realDepth = depth->getA( i, j, k ) - surfaceDepth;
         
               if ( realDepth <= 0.0 ) 
               {
                  estimatedTemperature = surfaceTemperature;
               } 
               else 
               {
                  estimatedTemperature = surfaceTemperature + realDepth * temperatureGradient;
               }
               
               temperature->set ( i, j, k, estimatedTemperature );
            }
         } else {
            for ( unsigned int k = temperature->lastK (); k > temperature->firstK (); --k ) {
               temperature->set ( i, j, k, temperature->getUndefinedValue ());
            }
         }
      }
   }
}

bool DerivedProperties::HydrostaticPressureFormationCalculator::isComputable ( const DerivedProperties::AbstractPropertyManager& propManager,
                                                                               const DataModel::AbstractSnapshot*  snapshot,
                                                                               const DataModel::AbstractFormation* formation ) const {
   
   bool basementFormation = ( dynamic_cast<const GeoPhysics::Formation*>( formation ) != 0 and dynamic_cast<const GeoPhysics::Formation*>( formation )->kind () == DataAccess::Interface::BASEMENT_FORMATION );

   const std::vector<std::string>& dependentProperties = getDependentPropertyNames ();

   bool propertyIsComputable = true;
   
   // Determine if the required properties are computable.
   for ( size_t i = 0; i < dependentProperties.size () and propertyIsComputable; ++i ) {

      if( basementFormation ) {
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

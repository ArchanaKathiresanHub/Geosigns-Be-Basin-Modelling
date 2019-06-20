//
// Copyright (C) 2016-2018 Shell International Exploration & Production.
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

#include "AbstractProperty.h"

#include "Interface.h"
#include "Surface.h"
#include "SimulationDetails.h"

#include "GeoPhysicsFormation.h"
#include "GeoPhysicalConstants.h"
#include "CompoundLithologyArray.h"

#include "ThermalConductivityFormationCalculator.h"
#include "PropertyRetriever.h"

using namespace AbstractDerivedProperties;

DerivedProperties::ThermalConductivityFormationCalculator::ThermalConductivityFormationCalculator ( const GeoPhysics::ProjectHandle* projectHandle )
   : m_projectHandle ( projectHandle )
{
   addPropertyName ( "ThCondVec2" );

   addDependentPropertyName ( "Temperature" );
   addDependentPropertyName ( "Pressure" );
   addDependentPropertyName ( "LithoStaticPressure" );
   addDependentPropertyName ( "Porosity" );

   // basement ALC dependency
    if( m_projectHandle->isALC() ) {
       addDependentPropertyName ( "Depth" );
       addDependentPropertyName ( "ALCStepTopBasaltDepth" );
       addDependentPropertyName ( "ALCStepBasaltThickness" );
    }
}

void DerivedProperties::ThermalConductivityFormationCalculator::calculate (       AbstractPropertyManager&      propertyManager,
                                                                            const DataModel::AbstractSnapshot*  snapshot,
                                                                            const DataModel::AbstractFormation* formation,
                                                                                  FormationPropertyList&        derivedProperties ) const {

   const GeoPhysics::GeoPhysicsFormation* geoFormation = dynamic_cast<const GeoPhysics::GeoPhysicsFormation*>( formation );
  
   if( geoFormation != 0 and geoFormation->kind() == DataAccess::Interface::BASEMENT_FORMATION ) {
      return calculateForBasement ( propertyManager, snapshot, formation, derivedProperties );
   }

   const DataModel::AbstractProperty* thermalConductivityProperty = propertyManager.getProperty ( "ThCondVec2" );

   const DataModel::AbstractProperty* temperatureProperty         = propertyManager.getProperty ( "Temperature" );
   const DataModel::AbstractProperty* porePressureProperty        = propertyManager.getProperty ( "Pressure" );
   const DataModel::AbstractProperty* porosityProperty            = propertyManager.getProperty ( "Porosity" );

   const FormationPropertyPtr temperature = propertyManager.getFormationProperty ( temperatureProperty,  snapshot, formation );
   const FormationPropertyPtr porosity    = propertyManager.getFormationProperty ( porosityProperty,     snapshot, formation );

   const DataAccess::Interface::SimulationDetails* lastFastcauldronRun =  m_projectHandle->getDetailsOfLastFastcauldron();
   bool hydrostaticMode = ( lastFastcauldronRun != 0 and
                            ( lastFastcauldronRun->getSimulatorMode () == "HydrostaticDecompaction" or
                              lastFastcauldronRun->getSimulatorMode () == "HydrostaticTemperature" ));

   FormationPropertyPtr porePressure = propertyManager.getFormationProperty ( porePressureProperty, snapshot, formation );

   if ( temperature != 0 and porePressure != 0 and porosity != 0 and geoFormation != 0 ) {
      DerivedFormationPropertyPtr thermalConductivity = DerivedFormationPropertyPtr ( new DerivedProperties::DerivedFormationProperty ( thermalConductivityProperty,
                                                                                                                                        snapshot,
                                                                                                                                        formation,
                                                                                                                                        propertyManager.getMapGrid (),
                                                                                                                                        geoFormation->getMaximumNumberOfElements() + 1 ));

      PropertyRetriever temperatureRetriever ( temperature );
      PropertyRetriever porePressureRetriever ( porePressure );
      PropertyRetriever porosityRetriever ( porosity );

      const GeoPhysics::CompoundLithologyArray& lithologies = geoFormation->getCompoundLithologyArray ();
      const GeoPhysics::FluidType* fluid = dynamic_cast<const GeoPhysics::FluidType*>(geoFormation->getFluidType ());

      double currentTime = snapshot->getTime();
      
      if( hydrostaticMode ) {
         (( GeoPhysics::FluidType *) fluid )->setDensityToConstant ();
      }

      // We could use any of the formation-properties here to get the undefined value.
      double undefinedValue = thermalConductivity->getUndefinedValue ();
      double thermalConductivityNormal;
      double thermalConductivityPlane;

      for ( unsigned int i = thermalConductivity->firstI ( true ); i <= thermalConductivity->lastI ( true ); ++i ) {
            
         for ( unsigned int j = thermalConductivity->firstJ ( true ); j <= thermalConductivity->lastJ ( true ); ++j ) {
               
            if ( m_projectHandle->getNodeIsValid ( i, j )) {
               const GeoPhysics::CompoundLithology* lithology = lithologies ( i, j, currentTime );

               for ( unsigned int k = thermalConductivity->firstK (); k <= thermalConductivity->lastK (); ++k ) {

                  lithology->calcBulkThermCondNP ( fluid,
                                                   0.01 * porosity->get ( i, j, k ),
                                                   temperature->get ( i, j, k ),
                                                   porePressure->get ( i, j, k ),
                                                   thermalConductivityNormal,
                                                   thermalConductivityPlane );
                  
                  thermalConductivity->set ( i, j, k, thermalConductivityNormal );
               }

            } else {

               for ( unsigned int k = thermalConductivity->firstK (); k <= thermalConductivity->lastK (); ++k ) {
                  thermalConductivity->set ( i, j, k, undefinedValue );
               }

            }

         }

      }

      derivedProperties.push_back ( thermalConductivity );
   }

}
void DerivedProperties::ThermalConductivityFormationCalculator::calculateForBasement ( AbstractPropertyManager&            propertyManager,
                                                                                       const DataModel::AbstractSnapshot*  snapshot,
                                                                                       const DataModel::AbstractFormation* formation,
                                                                                       FormationPropertyList&              derivedProperties ) const {
   
   const DataModel::AbstractProperty* thermalConductivityProperty = propertyManager.getProperty ( "ThCondVec2" );

   const DataModel::AbstractProperty* temperatureProperty         = propertyManager.getProperty ( "Temperature" );
   const FormationPropertyPtr temperature = propertyManager.getFormationProperty ( temperatureProperty,  snapshot, formation );

   const GeoPhysics::GeoPhysicsFormation* geoFormation = dynamic_cast<const GeoPhysics::GeoPhysicsFormation*>( formation );

   bool basementFormationAndAlcMode = ( geoFormation != 0 and m_projectHandle->isALC ());

   FormationPropertyPtr    lithostaticPressure;
   FormationPropertyPtr    depth;
   FormationMapPropertyPtr basaltDepth;
   FormationMapPropertyPtr basaltThickness;

   if ( basementFormationAndAlcMode ) {
      const DataModel::AbstractProperty* depthProperty               = propertyManager.getProperty ( "Depth" );
      const DataModel::AbstractProperty* lithostaticPressureProperty = propertyManager.getProperty ( "LithoStaticPressure" );
      const DataModel::AbstractProperty* basaltDepthProperty         = propertyManager.getProperty ( "ALCStepTopBasaltDepth" );
      const DataModel::AbstractProperty* basaltThicknessProperty     = propertyManager.getProperty ( "ALCStepBasaltThickness" );

      lithostaticPressure = propertyManager.getFormationProperty ( lithostaticPressureProperty, snapshot, formation );
      depth = propertyManager.getFormationProperty ( depthProperty, snapshot, formation );

      
      if( formation->getName() != "Crust" ) {
         const GeoPhysics::GeoPhysicsFormation *mantleFormation = dynamic_cast<const GeoPhysics::GeoPhysicsFormation*>( formation );
         const DataModel::AbstractFormation * crustFormation = ( mantleFormation->getTopSurface()->getTopFormation() );
         
         basaltDepth = propertyManager.getFormationMapProperty ( basaltDepthProperty, snapshot, crustFormation );
         basaltThickness = propertyManager.getFormationMapProperty ( basaltThicknessProperty, snapshot, crustFormation );
      } else {
         basaltDepth = propertyManager.getFormationMapProperty ( basaltDepthProperty, snapshot, formation );
         basaltThickness = propertyManager.getFormationMapProperty ( basaltThicknessProperty, snapshot, formation );
      }      
   }
   
   if ( temperature != 0 and geoFormation != 0 and
        ( not basementFormationAndAlcMode or 
          ( basementFormationAndAlcMode and lithostaticPressure != 0 and depth != 0 and basaltThickness != 0 and basaltDepth != 0 ))) {
      DerivedFormationPropertyPtr thermalConductivity = DerivedFormationPropertyPtr ( new DerivedProperties::DerivedFormationProperty ( thermalConductivityProperty,
                                                                                                                                        snapshot,
                                                                                                                                        formation,
                                                                                                                                        propertyManager.getMapGrid (),
                                                                                                                                        geoFormation->getMaximumNumberOfElements() + 1 ));

      PropertyRetriever temperatureRetriever ( temperature );
      PropertyRetriever pressureRetriever;
      PropertyRetriever depthRetriever;
      PropertyRetriever basaltDepthRetriever;
      PropertyRetriever basaltThicknessRetriever;
      if ( basementFormationAndAlcMode ) {
         pressureRetriever.reset ( lithostaticPressure );
         depthRetriever.reset ( depth );
         basaltDepthRetriever.reset ( basaltDepth );
         basaltThicknessRetriever.reset ( basaltThickness );
      } 

      const GeoPhysics::CompoundLithologyArray& lithologies = geoFormation->getCompoundLithologyArray ();
      double currentTime = snapshot->getTime();
      
      // We could use any of the formation-properties here to get the undefined value.
      double undefinedValue = thermalConductivity->getUndefinedValue ();
      double thermalConductivityNormal;
      double thermalConductivityPlane;

      for ( unsigned int i = thermalConductivity->firstI ( true ); i <= thermalConductivity->lastI ( true ); ++i ) {
            
         for ( unsigned int j = thermalConductivity->firstJ ( true ); j <= thermalConductivity->lastJ ( true ); ++j ) {
               
            if ( m_projectHandle->getNodeIsValid ( i, j )) {
               const GeoPhysics::CompoundLithology* lithology = lithologies ( i, j, currentTime );

               for ( unsigned int k = thermalConductivity->firstK (); k <= thermalConductivity->lastK (); ++k ) {

                  if ( basementFormationAndAlcMode ) {
  
                     const double topBasaltDepth = basaltDepth->get( i, j );
                     const double botBasaltDepth = topBasaltDepth + basaltThickness->get( i, j ) + 1;

                     if( basaltThickness->get( i, j ) != 0 and ( topBasaltDepth <= depth->get ( i, j, k ) and botBasaltDepth >= depth->get ( i, j, k  ))) {
                        lithology->calcBulkThermCondNPBasalt ( temperature->get ( i, j, k ),
                                                               lithostaticPressure->get ( i, j, k ),
                                                               thermalConductivityNormal,
                                                               thermalConductivityPlane );
                     } else {
                        lithology->calcBulkThermCondNPBasement ( 0, 0,
                                                                 temperature->get ( i, j, k ),
                                                                 lithostaticPressure->get ( i, j, k ),
                                                                 thermalConductivityNormal,
                                                                 thermalConductivityPlane );
                     }
                  } else {
                     lithology->calcBulkThermCondNP ( 0, 0,
                                                      temperature->get ( i, j, k ),
                                                      0.0,
                                                      thermalConductivityNormal,
                                                      thermalConductivityPlane );
                  }

                  thermalConductivity->set ( i, j, k, thermalConductivityNormal );
               }

            } else {

               for ( unsigned int k = thermalConductivity->firstK (); k <= thermalConductivity->lastK (); ++k ) {
                  thermalConductivity->set ( i, j, k, undefinedValue );
               }

            }

         }

      }

      derivedProperties.push_back ( thermalConductivity );
   }

}

bool DerivedProperties::ThermalConductivityFormationCalculator::isComputable ( const AbstractPropertyManager&      propManager,
                                                                               const DataModel::AbstractSnapshot*  snapshot,
                                                                               const DataModel::AbstractFormation* formation ) const {
   
   bool basementFormation = ( dynamic_cast<const GeoPhysics::GeoPhysicsFormation*>( formation ) != 0 and dynamic_cast<const GeoPhysics::GeoPhysicsFormation*>( formation )->kind () == DataAccess::Interface::BASEMENT_FORMATION );
   if( basementFormation ) {
      return isComputableForBasement ( propManager, snapshot, formation );
   }

   const std::vector<std::string>& dependentProperties = getDependentPropertyNames ();

   bool propertyIsComputable = true;
   
   // Determine if the required properties are computable.
   for ( size_t i = 0; i < dependentProperties.size () and propertyIsComputable; ++i ) {

      if( dependentProperties [ i ] == "ALCStepTopBasaltDepth" or dependentProperties [ i ] == "ALCStepBasaltThickness" or 
          dependentProperties [ i ] == "LithoStaticPressure" or  dependentProperties [ i ] == "Depth" ) {
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

bool DerivedProperties::ThermalConductivityFormationCalculator::isComputableForBasement ( const AbstractPropertyManager&      propManager,
                                                                                          const DataModel::AbstractSnapshot*  snapshot,
                                                                                          const DataModel::AbstractFormation* formation ) const {
   
   const std::vector<std::string>& dependentProperties = getDependentPropertyNames ();

   bool propertyIsComputable = true;

   // Determine if the required properties are computable.
   for ( size_t i = 0; i < dependentProperties.size () and propertyIsComputable; ++i ) {

      if(( not m_projectHandle->isALC () and dependentProperties[i] == "LithoStaticPressure") or 
         dependentProperties [ i ] == "Porosity" or dependentProperties [ i ] == "Pressure" ) {

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

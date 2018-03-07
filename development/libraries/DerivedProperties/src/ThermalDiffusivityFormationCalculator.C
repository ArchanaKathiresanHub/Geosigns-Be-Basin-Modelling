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

#include "Interface/Interface.h"
#include "Interface/SimulationDetails.h"

#include "GeoPhysicsFormation.h"
#include "GeoPhysicalConstants.h"
#include "CompoundLithologyArray.h"

#include "ThermalDiffusivityFormationCalculator.h"
#include "PropertyRetriever.h"

using namespace AbstractDerivedProperties;

DerivedProperties::ThermalDiffusivityFormationCalculator::ThermalDiffusivityFormationCalculator ( const GeoPhysics::ProjectHandle* projectHandle ) : m_projectHandle ( projectHandle ) {
   addPropertyName ( "DiffusivityVec2" );

   addDependentPropertyName ( "Temperature" );
   addDependentPropertyName ( "Pressure" );
   addDependentPropertyName ( "LithoStaticPressure" );
   addDependentPropertyName ( "Porosity" );
}

void DerivedProperties::ThermalDiffusivityFormationCalculator::calculate (        AbstractPropertyManager&      propertyManager,
                                                                            const DataModel::AbstractSnapshot*  snapshot,
                                                                            const DataModel::AbstractFormation* formation,
                                                                                  FormationPropertyList&        derivedProperties ) const {
   const GeoPhysics::Formation* geoFormation = dynamic_cast<const GeoPhysics::Formation*>( formation );
   
   if( geoFormation != 0 and geoFormation->kind() == DataAccess::Interface::BASEMENT_FORMATION ) {
      return calculateForBasement ( propertyManager, snapshot, formation, derivedProperties );
   }

   const DataModel::AbstractProperty* thermalDiffusivityProperty  = propertyManager.getProperty ( "DiffusivityVec2" );

   const DataModel::AbstractProperty* temperatureProperty         = propertyManager.getProperty ( "Temperature" );
   const DataModel::AbstractProperty* porePressureProperty        = propertyManager.getProperty ( "Pressure" );
   const DataModel::AbstractProperty* porosityProperty            = propertyManager.getProperty ( "Porosity" );

   const FormationPropertyPtr temperature  = propertyManager.getFormationProperty ( temperatureProperty,  snapshot, formation );
   const FormationPropertyPtr porePressure = propertyManager.getFormationProperty ( porePressureProperty, snapshot, formation );
   const FormationPropertyPtr porosity     = propertyManager.getFormationProperty ( porosityProperty,     snapshot, formation );

   const DataAccess::Interface::SimulationDetails* lastFastcauldronRun = m_projectHandle->getDetailsOfLastFastcauldron();
   bool hydrostaticMode = ( lastFastcauldronRun != 0 and
                            ( lastFastcauldronRun->getSimulatorMode () == "HydrostaticDecompaction" or
                              lastFastcauldronRun->getSimulatorMode () == "HydrostaticTemperature" ));

   if ( temperature != 0 and porePressure != 0 and porosity != 0 and geoFormation != 0 ) {
      DerivedFormationPropertyPtr thermalDiffusivity = DerivedFormationPropertyPtr ( new DerivedProperties::DerivedFormationProperty ( thermalDiffusivityProperty,
                                                                                                                                       snapshot,
                                                                                                                                       formation,
                                                                                                                                       propertyManager.getMapGrid (),
                                                                                                                                       geoFormation->getMaximumNumberOfElements() + 1 ));

      PropertyRetriever temperatureRetriever ( temperature );
      PropertyRetriever porePressureRetriever ( porePressure );
      PropertyRetriever porosityRetriever ( porosity );

      const GeoPhysics::CompoundLithologyArray& lithologies = geoFormation->getCompoundLithologyArray ();
      const GeoPhysics::FluidType* fluid = dynamic_cast<const GeoPhysics::FluidType*>(geoFormation->getFluidType ());

      // We could use any of the formation-properties here to get the undefined value.
      double undefinedValue = thermalDiffusivity->getUndefinedValue ();
      double thermalConductivityNormal;
      double thermalConductivityPlane;
      double bulkDensityXHeatCapacity;

      double currentTime = snapshot->getTime();

      if( hydrostaticMode ) {
         (( GeoPhysics::FluidType *) fluid )->setDensityToConstant ();
      }

      for ( unsigned int i = thermalDiffusivity->firstI ( true ); i <= thermalDiffusivity->lastI ( true ); ++i ) {
            
         for ( unsigned int j = thermalDiffusivity->firstJ ( true ); j <= thermalDiffusivity->lastJ ( true ); ++j ) {
               
            if ( m_projectHandle->getNodeIsValid ( i, j )) {
               const GeoPhysics::CompoundLithology* lithology = lithologies ( i, j, currentTime );

               for ( unsigned int k = thermalDiffusivity->firstK (); k <= thermalDiffusivity->lastK (); ++k ) {
                  
                     lithology->calcBulkDensXHeatCapacity ( fluid,
                                                            0.01 * porosity->get ( i, j, k ),
                                                            porePressure->get ( i, j, k ),
                                                            temperature->get ( i, j, k ),
                                                            0.0,
                                                            bulkDensityXHeatCapacity );

                     lithology->calcBulkThermCondNP ( fluid,
                                                      0.01 * porosity->get ( i, j, k ),
                                                      temperature->get ( i, j, k ),
                                                      porePressure->get ( i, j, k ),
                                                      thermalConductivityNormal,
                                                      thermalConductivityPlane );

                  if ( thermalConductivityNormal != undefinedValue and bulkDensityXHeatCapacity != undefinedValue ) {
                     thermalDiffusivity->set ( i, j, k, thermalConductivityNormal / bulkDensityXHeatCapacity );
                  } else {
                     thermalDiffusivity->set ( i, j, k, undefinedValue );
                  }

               }

            } else {

               for ( unsigned int k = thermalDiffusivity->firstK (); k <= thermalDiffusivity->lastK (); ++k ) {
                  thermalDiffusivity->set ( i, j, k, undefinedValue );
               }

            }

         }

      }

      derivedProperties.push_back ( thermalDiffusivity );
   }

}

void DerivedProperties::ThermalDiffusivityFormationCalculator::calculateForBasement (       AbstractPropertyManager&      propertyManager,
                                                                                      const DataModel::AbstractSnapshot*  snapshot,
                                                                                      const DataModel::AbstractFormation* formation,
                                                                                            FormationPropertyList&        derivedProperties ) const {
   const GeoPhysics::Formation* geoFormation = dynamic_cast<const GeoPhysics::Formation*>( formation );

   const DataModel::AbstractProperty* thermalDiffusivityProperty  = propertyManager.getProperty ( "DiffusivityVec2" );

   const DataModel::AbstractProperty* temperatureProperty         = propertyManager.getProperty ( "Temperature" );
   const DataModel::AbstractProperty* lithostaticPressureProperty = propertyManager.getProperty ( "LithoStaticPressure" );

   const FormationPropertyPtr temperature  = propertyManager.getFormationProperty ( temperatureProperty,  snapshot, formation );

   FormationPropertyPtr lithostaticPressure;

   bool basementFormationAndAlcMode = ( geoFormation != 0 and m_projectHandle->isALC ());

   const DataAccess::Interface::SimulationDetails* lastFastcauldronRun = m_projectHandle->getDetailsOfLastFastcauldron();

   bool hydrostaticMode = ( lastFastcauldronRun != 0 and
                            ( lastFastcauldronRun->getSimulatorMode () == "HydrostaticDecompaction" or
                              lastFastcauldronRun->getSimulatorMode () == "HydrostaticTemperature" ));
   
   if ( basementFormationAndAlcMode ) {
      lithostaticPressure = propertyManager.getFormationProperty ( lithostaticPressureProperty, snapshot, formation );
   }
   
   if ( temperature != 0 and geoFormation != 0 and ( not basementFormationAndAlcMode or ( basementFormationAndAlcMode and lithostaticPressure != 0 ))) {
      DerivedFormationPropertyPtr thermalDiffusivity = DerivedFormationPropertyPtr ( new DerivedProperties::DerivedFormationProperty ( thermalDiffusivityProperty,
                                                                                                                                       snapshot,
                                                                                                                                       formation,
                                                                                                                                       propertyManager.getMapGrid (),
                                                                                                                                       geoFormation->getMaximumNumberOfElements() + 1 ));

      PropertyRetriever temperatureRetriever ( temperature );

      const GeoPhysics::CompoundLithologyArray& lithologies = geoFormation->getCompoundLithologyArray ();

      // We could use any of the formation-properties here to get the undefined value.
      double undefinedValue = thermalDiffusivity->getUndefinedValue ();
      double thermalConductivityNormal;
      double thermalConductivityPlane;
      double bulkDensityXHeatCapacity;

      double currentTime = snapshot->getTime();

      for ( unsigned int i = thermalDiffusivity->firstI ( true ); i <= thermalDiffusivity->lastI ( true ); ++i ) {
            
         for ( unsigned int j = thermalDiffusivity->firstJ ( true ); j <= thermalDiffusivity->lastJ ( true ); ++j ) {
               
            if ( m_projectHandle->getNodeIsValid ( i, j )) {
               const GeoPhysics::CompoundLithology* lithology = lithologies ( i, j, currentTime );

               for ( unsigned int k = thermalDiffusivity->firstK (); k <= thermalDiffusivity->lastK (); ++k ) {
                  
                  if ( basementFormationAndAlcMode ) {
                     lithology->calcBulkDensXHeatCapacity ( 0, 0.0, 0.0,
                                                            temperature->get ( i, j, k ),
                                                            lithostaticPressure->get ( i, j, k ),
                                                            bulkDensityXHeatCapacity );
                     lithology->calcBulkThermCondNPBasement ( 0, 0.0,
                                                              temperature->get ( i, j, k ),
                                                              lithostaticPressure->get ( i, j, k ),
                                                              thermalConductivityNormal,
                                                              thermalConductivityPlane );

                  } else {
                     lithology->calcBulkDensXHeatCapacity ( 0, 0.0, 0.0,
                                                            temperature->get ( i, j, k ),
                                                            0.0,
                                                            bulkDensityXHeatCapacity );

                     lithology->calcBulkThermCondNP ( 0, 0.0,
                                                      temperature->get ( i, j, k ),
                                                      0.0,
                                                      thermalConductivityNormal,
                                                      thermalConductivityPlane );
                  }

                  if ( thermalConductivityNormal != undefinedValue and bulkDensityXHeatCapacity != undefinedValue ) {
                     thermalDiffusivity->set ( i, j, k, thermalConductivityNormal / bulkDensityXHeatCapacity );
                  } else {
                     thermalDiffusivity->set ( i, j, k, undefinedValue );
                  }

               }

            } else {

               for ( unsigned int k = thermalDiffusivity->firstK (); k <= thermalDiffusivity->lastK (); ++k ) {
                  thermalDiffusivity->set ( i, j, k, undefinedValue );
               }

            }

         }

      }

      derivedProperties.push_back ( thermalDiffusivity );
   }

}

bool DerivedProperties::ThermalDiffusivityFormationCalculator::isComputable ( const AbstractPropertyManager&      propManager,
                                                                              const DataModel::AbstractSnapshot*  snapshot,
                                                                              const DataModel::AbstractFormation* formation ) const {
   
   bool basementFormation = ( dynamic_cast<const GeoPhysics::Formation*>( formation ) != 0 and dynamic_cast<const GeoPhysics::Formation*>( formation )->kind () == DataAccess::Interface::BASEMENT_FORMATION );

   const std::vector<std::string>& dependentProperties = getDependentPropertyNames ();

   bool propertyIsComputable = true;
   
   // Determine if the required properties are computable.
   for ( size_t i = 0; i < dependentProperties.size () and propertyIsComputable; ++i ) {

      if( basementFormation and ( dependentProperties [ i ] == "Pressure" or dependentProperties [ i ] == "Porosity" )) {
         propertyIsComputable = true;
      } else {
         if( not basementFormation and ( dependentProperties [ i ] == "LithoStaticPressure" )) {
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
   }

   return propertyIsComputable;
}

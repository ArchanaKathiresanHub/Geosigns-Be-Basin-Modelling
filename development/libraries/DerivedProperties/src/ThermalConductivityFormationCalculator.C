#include "AbstractPropertyManager.h"
#include "DerivedFormationProperty.h"
#include "DerivedPropertyManager.h"

#include "AbstractProperty.h"

#include "Interface/Interface.h"
#include "Interface/SimulationDetails.h"

#include "GeoPhysicsFormation.h"
#include "GeoPhysicalConstants.h"
#include "CompoundLithologyArray.h"

#include "ThermalConductivityFormationCalculator.h"
#include "PropertyRetriever.h"

DerivedProperties::ThermalConductivityFormationCalculator::ThermalConductivityFormationCalculator ( const GeoPhysics::ProjectHandle* projectHandle ) : m_projectHandle ( projectHandle ) {
   addPropertyName ( "ThCondVec2" );

   addDependentPropertyName ( "Temperature" );
   addDependentPropertyName ( "Pressure" );
   addDependentPropertyName ( "LithoStaticPressure" );
   addDependentPropertyName ( "Porosity" );
}

void DerivedProperties::ThermalConductivityFormationCalculator::calculate ( DerivedProperties::AbstractPropertyManager& propertyManager,
                                                                            const DataModel::AbstractSnapshot*          snapshot,
                                                                            const DataModel::AbstractFormation*         formation,
                                                                                  FormationPropertyList&                derivedProperties ) const {

   const GeoPhysics::Formation* geoFormation = dynamic_cast<const GeoPhysics::Formation*>( formation );
  
   if( geoFormation != 0 and geoFormation->kind() == DataAccess::Interface::BASEMENT_FORMATION ) {
      return calculateForBasement ( propertyManager, snapshot, formation, derivedProperties );
   }

  const DataModel::AbstractProperty* thermalConductivityProperty = propertyManager.getProperty ( "ThCondVec2" );

   const DataModel::AbstractProperty* temperatureProperty         = propertyManager.getProperty ( "Temperature" );
   const DataModel::AbstractProperty* porePressureProperty        = propertyManager.getProperty ( "Pressure" );
   const DataModel::AbstractProperty* porosityProperty            = propertyManager.getProperty ( "Porosity" );

   const FormationPropertyPtr temperature = propertyManager.getFormationProperty ( temperatureProperty,  snapshot, formation );
   const FormationPropertyPtr porosity    = propertyManager.getFormationProperty ( porosityProperty,     snapshot, formation );

   bool hydrostaticMode = ( m_projectHandle->getDetailsOfLastSimulation ( "fastcauldron" ) != 0 and
                            ( m_projectHandle->getDetailsOfLastSimulation ( "fastcauldron" )->getSimulatorMode () == "HydrostaticDecompaction" or
                              m_projectHandle->getDetailsOfLastSimulation ( "fastcauldron" )->getSimulatorMode () == "HydrostaticTemperature" or
                              m_projectHandle->getDetailsOfLastSimulation ( "fastcauldron" )->getSimulatorMode () == "HydrostaticHighResDecompaction" ));

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
                                                   0.01 * porosity->getA ( i, j, k ),
                                                   temperature->getA ( i, j, k ),
                                                   porePressure->getA ( i, j, k ),
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
void DerivedProperties::ThermalConductivityFormationCalculator::calculateForBasement ( DerivedProperties::AbstractPropertyManager& propertyManager,
                                                                                       const DataModel::AbstractSnapshot*          snapshot,
                                                                                       const DataModel::AbstractFormation*         formation,
                                                                                       FormationPropertyList&                derivedProperties ) const {
   
   const DataModel::AbstractProperty* thermalConductivityProperty = propertyManager.getProperty ( "ThCondVec2" );

   const DataModel::AbstractProperty* temperatureProperty         = propertyManager.getProperty ( "Temperature" );
   const DataModel::AbstractProperty* lithostaticPressureProperty = propertyManager.getProperty ( "LithoStaticPressure" );

   const FormationPropertyPtr temperature = propertyManager.getFormationProperty ( temperatureProperty,  snapshot, formation );

   const GeoPhysics::Formation* geoFormation = dynamic_cast<const GeoPhysics::Formation*>( formation );

   bool basementFormationAndAlcMode = ( geoFormation != 0 and m_projectHandle->isALC ());

   FormationPropertyPtr lithostaticPressure;
   if ( basementFormationAndAlcMode ) {
      lithostaticPressure = propertyManager.getFormationProperty ( lithostaticPressureProperty, snapshot, formation );
   }
   
   if ( temperature != 0 and ( not basementFormationAndAlcMode or ( basementFormationAndAlcMode and lithostaticPressure != 0 )) and geoFormation != 0 ) {
      DerivedFormationPropertyPtr thermalConductivity = DerivedFormationPropertyPtr ( new DerivedProperties::DerivedFormationProperty ( thermalConductivityProperty,
                                                                                                                                        snapshot,
                                                                                                                                        formation,
                                                                                                                                        propertyManager.getMapGrid (),
                                                                                                                                        geoFormation->getMaximumNumberOfElements() + 1 ));

      PropertyRetriever temperatureRetriever ( temperature );
      PropertyRetriever pressureRetriever;
      if ( basementFormationAndAlcMode ) {
         pressureRetriever.reset ( lithostaticPressure );
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
                     lithology->calcBulkThermCondNPBasement ( 0, 0,
                                                              temperature->getA ( i, j, k ),
                                                              lithostaticPressure->getA ( i, j, k ),
                                                              thermalConductivityNormal,
                                                              thermalConductivityPlane );
                  } else {
                     lithology->calcBulkThermCondNP ( 0, 0,
                                                      temperature->getA ( i, j, k ),
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

bool DerivedProperties::ThermalConductivityFormationCalculator::isComputable ( const DerivedProperties::AbstractPropertyManager& propManager,
                                                                               const DataModel::AbstractSnapshot*  snapshot,
                                                                               const DataModel::AbstractFormation* formation ) const {
   
   bool basementFormation = ( dynamic_cast<const GeoPhysics::Formation*>( formation ) != 0 and dynamic_cast<const GeoPhysics::Formation*>( formation )->kind () == DataAccess::Interface::BASEMENT_FORMATION );

   const std::vector<std::string>& dependentProperties = getDependentPropertyNames ();

   bool propertyIsComputable = true;
   
   // Determine if the required properties are computable.
   for ( size_t i = 0; i < dependentProperties.size () and propertyIsComputable; ++i ) {

      if( basementFormation and ( dependentProperties [ i ] == "Pressure" or  dependentProperties [ i ] == "Ves" or  dependentProperties [ i ] == "Porosity" )) {
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

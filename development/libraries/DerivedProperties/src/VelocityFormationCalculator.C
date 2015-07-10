#include "AbstractPropertyManager.h"
#include "DerivedFormationProperty.h"
#include "DerivedPropertyManager.h"

#include "Interface/RunParameters.h"

#include "GeoPhysicsFormation.h"
#include "GeoPhysicalConstants.h"
#include "GeoPhysicsFluidType.h"
#include "CompoundLithologyArray.h"
#include "GeoPhysicsProjectHandle.h"

#include "PropertyRetriever.h"
#include "VelocityFormationCalculator.h"


DerivedProperties::VelocityFormationCalculator::VelocityFormationCalculator () {
   addPropertyName ( "Velocity" );

   addDependentPropertyName ( "Porosity" );
   addDependentPropertyName ( "BulkDensity" );
   addDependentPropertyName ( "Pressure" );
   addDependentPropertyName ( "Temperature" );
}

void DerivedProperties::VelocityFormationCalculator::calculate ( DerivedProperties::AbstractPropertyManager& propertyManager,
                                                                 const DataModel::AbstractSnapshot*  snapshot,
                                                                 const DataModel::AbstractFormation* formation,
                                                                       FormationPropertyList&        derivedProperties ) const {

   const DataModel::AbstractProperty* porosityProperty = propertyManager.getProperty ( "Porosity" );
   const DataModel::AbstractProperty* bulkDensityProperty = propertyManager.getProperty ( "BulkDensity" );
   const DataModel::AbstractProperty* pressureProperty = propertyManager.getProperty ( "Pressure" );
   const DataModel::AbstractProperty* temperatureProperty = propertyManager.getProperty ( "Temperature" );

   const DataModel::AbstractProperty* velocityProperty = propertyManager.getProperty ( "Velocity" );
   
   const FormationPropertyPtr porosity    = propertyManager.getFormationProperty ( porosityProperty, snapshot, formation );
   const FormationPropertyPtr bulkDensity = propertyManager.getFormationProperty ( bulkDensityProperty, snapshot, formation );
   const FormationPropertyPtr pressure = propertyManager.getFormationProperty ( pressureProperty, snapshot, formation );
   const FormationPropertyPtr temperature = propertyManager.getFormationProperty ( temperatureProperty, snapshot, formation );
   
   const GeoPhysics::Formation* geophysicsFormation = dynamic_cast<const GeoPhysics::Formation*>( formation );

   const GeoPhysics::ProjectHandle* projectHandle = dynamic_cast<const GeoPhysics::ProjectHandle*>( geophysicsFormation->getProjectHandle ());
   const std::string& projectVelocityName = geophysicsFormation->getProjectHandle ()->getRunParameters ()->getVelAlgorithm ();
   string velocityAlgorithmName = ( projectVelocityName == "" ? GeoPhysics::DefaultVelocityAlgorithm : projectVelocityName );
   GeoPhysics::VelocityAlgorithm velocityAlgorithm = GeoPhysics::velocityAlgorithmValue ( velocityAlgorithmName );

   derivedProperties.clear ();
   
   if ( porosity != 0 and bulkDensity != 0 and pressure != 0 and temperature != 0 and geophysicsFormation != 0 ) {
      const GeoPhysics::FluidType* geophysicsFluid = dynamic_cast<const GeoPhysics::FluidType*>( geophysicsFormation->getFluidType ());

      const GeoPhysics::CompoundLithologyArray * lithologies = &geophysicsFormation->getCompoundLithologyArray ();
      
      if ( lithologies != 0 ) {

         PropertyRetriever porosityRetriever ( porosity );
         PropertyRetriever bulkDensityRetriever ( bulkDensity );
         PropertyRetriever pressureRetriever ( pressure );
         PropertyRetriever temperatureRetriever ( temperature );

         DerivedFormationPropertyPtr velocity = DerivedFormationPropertyPtr ( new DerivedProperties::DerivedFormationProperty ( velocityProperty,
                                                                                                                                snapshot,
                                                                                                                                formation, 
                                                                                                                                propertyManager.getMapGrid (),
                                                                                                                                geophysicsFormation->getMaximumNumberOfElements() + 1 ));

         double currentTime = snapshot->getTime();
         double undefinedValue = velocity->getUndefinedValue ();
         double velocityValue;
         
         for ( unsigned int i = velocity->firstI ( true ); i <= velocity->lastI ( true ); ++i ) {
            
            for ( unsigned int j = velocity->firstJ ( true ); j <= velocity->lastJ ( true ); ++j ) {
               
               if ( projectHandle->getNodeIsValid ( i, j )) {
                  
                  for ( unsigned int k = velocity->firstK (); k <= velocity->lastK (); ++k ) {
                     (*lithologies)( i, j, currentTime )->calcVelocity ( geophysicsFluid,
                                                            velocityAlgorithm,
                                                            porosity->get ( i, j, k ),
                                                            bulkDensity->get ( i, j, k ),
                                                            pressure->get ( i, j, k ),
                                                            temperature->get ( i, j, k ),
                                                            velocityValue );
                     velocity->set ( i, j, k, velocityValue );
                  }

               } else {

                  for ( unsigned int k = velocity->firstK (); k <= velocity->lastK (); ++k ) {
                     velocity->set ( i, j, k, undefinedValue );
                  }

               }

            }

         }

         derivedProperties.push_back ( velocity );
      }

   }

}

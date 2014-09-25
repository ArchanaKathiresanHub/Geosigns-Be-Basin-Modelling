#include "AbstractPropertyManager.h"
#include "DerivedFormationProperty.h"
#include "DerivedPropertyManager.h"

#include "GeoPhysicsFormation.h"
#include "GeoPhysicalConstants.h"
#include "CompoundLithologyArray.h"

#include "VesFormationCalculator.h"

DerivedProperties::VesFormationCalculator::VesFormationCalculator () {
   m_propertyNames.push_back ( "Ves" );
}

const std::vector<std::string>& DerivedProperties::VesFormationCalculator::getPropertyNames () const {
   return m_propertyNames;
}

void DerivedProperties::VesFormationCalculator::calculate ( DerivedProperties::AbstractPropertyManager& propertyManager,
                                                            const DataModel::AbstractSnapshot*  snapshot,
                                                            const DataModel::AbstractFormation* formation,
                                                                  FormationPropertyList&        derivedProperties ) const {

   const DataModel::AbstractProperty* aLithostaticPressureProperty = propertyManager.getProperty ( "LithoStaticPressure" );
   const DataModel::AbstractProperty* aPorePressureProperty = propertyManager.getProperty ( "Pressure" );

   const DataModel::AbstractProperty* aVesProperty = propertyManager.getProperty ( "Ves" );
   
   const FormationPropertyPtr lithostaticPressure = propertyManager.getFormationProperty ( aLithostaticPressureProperty, snapshot, formation );
   const FormationPropertyPtr porePressure = propertyManager.getFormationProperty ( aPorePressureProperty, snapshot, formation );
   
   derivedProperties.clear ();
   
   const GeoPhysics::Formation* geoFormation = dynamic_cast<const GeoPhysics::Formation*>( formation );

   if( lithostaticPressure != 0 and porePressure != 0 and geoFormation != 0 ) {
              
      lithostaticPressure->retrieveData ();
      porePressure->retrieveData ();
      
      DerivedFormationPropertyPtr ves = DerivedFormationPropertyPtr ( new DerivedProperties::DerivedFormationProperty ( aVesProperty, snapshot, formation, 
                                                                                                                        propertyManager.getMapGrid (),
                                                                                                                        geoFormation->getMaximumNumberOfElements() + 1 ));
      double undefinedValue = lithostaticPressure->getUndefinedValue ();
      
      for ( unsigned int i = lithostaticPressure->firstI ( true ); i <= lithostaticPressure->lastI ( true ); ++i ) {
         
         for ( unsigned int j = lithostaticPressure->firstJ ( true ); j <= lithostaticPressure->lastJ ( true ); ++j ) {
            
            if ( propertyManager.getNodeIsValid ( i , j ) ) { //FastcauldronSimulator::getInstance ().nodeIsDefined ( i, j )) {
               
               for ( unsigned int k = lithostaticPressure->firstK (); k <= lithostaticPressure->lastK (); ++k ) {
                  
                  ves->set ( i, j, k, ( lithostaticPressure->get ( i, j, k ) - porePressure->get ( i, j, k )) * GeoPhysics::MPa_To_Pa );
               }
            } else {
               for ( unsigned int k = lithostaticPressure->firstK (); k <= lithostaticPressure->lastK (); ++k ) {
                   ves->set ( i, j, k, undefinedValue );
               }
            }
         }
      }
      derivedProperties.push_back ( ves );
      
      lithostaticPressure->restoreData ();
      porePressure->restoreData ();
   }
}

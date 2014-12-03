#include "AbstractPropertyManager.h"
#include "DerivedFormationProperty.h"
#include "DerivedPropertyManager.h"

#include "GeoPhysicalConstants.h"
#include "VesFormationCalculator.h"
#include "PropertyRetriever.h"

DerivedProperties::VesFormationCalculator::VesFormationCalculator () {
   addPropertyName ( "Ves" );
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
   
   PropertyRetriever lpRetriever ( lithostaticPressure );
   PropertyRetriever ppRetriever ( porePressure );

   derivedProperties.clear ();
   
   if( lithostaticPressure != 0 and porePressure != 0 ) {
              
      DerivedFormationPropertyPtr ves = DerivedFormationPropertyPtr ( new DerivedProperties::DerivedFormationProperty ( aVesProperty, snapshot, formation, 
                                                                                                                        propertyManager.getMapGrid (),
                                                                                                                        porePressure->lengthK () ));
      double undefinedValue = lithostaticPressure->getUndefinedValue ();
      
      for ( unsigned int i = lithostaticPressure->firstI ( true ); i <= lithostaticPressure->lastI ( true ); ++i ) {
         
         for ( unsigned int j = lithostaticPressure->firstJ ( true ); j <= lithostaticPressure->lastJ ( true ); ++j ) {
            
            if ( propertyManager.getNodeIsValid ( i, j )) {
                
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
   }
}

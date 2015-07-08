#include "AbstractPropertyManager.h"
#include "DerivedFormationProperty.h"
#include "DerivedPropertyManager.h"

#include "GeoPhysicalConstants.h"

#include "LithostaticPressureFormationCalculator.h"
#include "PropertyRetriever.h"

DerivedProperties::LithostaticPressureFormationCalculator::LithostaticPressureFormationCalculator ( const GeoPhysics::ProjectHandle* projectHandle ) : m_projectHandle ( projectHandle ) {
   addPropertyName ( "LithoStaticPressure" );
}

void DerivedProperties::LithostaticPressureFormationCalculator::calculate ( DerivedProperties::AbstractPropertyManager& propertyManager,
                                                                            const DataModel::AbstractSnapshot*  snapshot,
                                                                            const DataModel::AbstractFormation* formation,
                                                                                  FormationPropertyList&        derivedProperties ) const {

   const DataModel::AbstractProperty* aVesProperty = propertyManager.getProperty ( "Ves" );
   const DataModel::AbstractProperty* aPorePressureProperty = propertyManager.getProperty ( "Pressure" );

   const DataModel::AbstractProperty* aLithostaticPressureProperty = propertyManager.getProperty ( "LithoStaticPressure" );
   
   const FormationPropertyPtr ves          = propertyManager.getFormationProperty ( aVesProperty, snapshot, formation );
   const FormationPropertyPtr porePressure = propertyManager.getFormationProperty ( aPorePressureProperty, snapshot, formation );

   PropertyRetriever vesRetriever ( ves );
   PropertyRetriever ppRetriever ( porePressure );
   
   derivedProperties.clear ();
   
   if( ves != 0 and porePressure != 0 ) {
              
      DerivedFormationPropertyPtr lithostaticPressure = 
         DerivedFormationPropertyPtr ( new DerivedProperties::DerivedFormationProperty ( aLithostaticPressureProperty, snapshot, formation, 
                                                                                         propertyManager.getMapGrid (),
                                                                                         ves->lengthK () ));
      double undefinedValue = ves->getUndefinedValue ();
      
      for ( unsigned int i = ves->firstI ( true ); i <= ves->lastI ( true ); ++i ) {
         
         for ( unsigned int j = ves->firstJ ( true ); j <= ves->lastJ ( true ); ++j ) {
            
            if ( m_projectHandle->getNodeIsValid ( i, j )) { 
                
               for ( unsigned int k = ves->firstK (); k <= ves->lastK (); ++k ) {
                 
                  lithostaticPressure->set ( i, j, k, ( ves->get ( i, j, k ) * GeoPhysics::PascalsToMegaPascals + porePressure->get ( i, j, k )));
               }
            } else {
               for ( unsigned int k = ves->firstK (); k <= ves->lastK (); ++k ) {
                   lithostaticPressure->set ( i, j, k, undefinedValue );
               }
            }
         }
      }

      derivedProperties.push_back ( lithostaticPressure );
   }
}

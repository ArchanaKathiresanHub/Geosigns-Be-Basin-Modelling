#include "AbstractPropertyManager.h"
#include "DerivedFormationProperty.h"
#include "DerivedPropertyManager.h"

#include "GeoPhysicalConstants.h"

#include "LithostaticPressureFormationCalculator.h"

DerivedProperties::LithostaticPressureFormationCalculator::LithostaticPressureFormationCalculator () {
   m_propertyNames.push_back ( "LithoStaticPressure" );
}

const std::vector<std::string>& DerivedProperties::LithostaticPressureFormationCalculator::getPropertyNames () const {
   return m_propertyNames;
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
   
   derivedProperties.clear ();
   
   if( ves != 0 and porePressure != 0 ) {
              
      DerivedFormationPropertyPtr lithostaticPressure = 
         DerivedFormationPropertyPtr ( new DerivedProperties::DerivedFormationProperty ( aLithostaticPressureProperty, snapshot, formation, 
                                                                                         propertyManager.getMapGrid (),
                                                                                         ves->lengthK () ));
      double undefinedValue = ves->getUndefinedValue ();
      
      for ( unsigned int i = ves->firstI ( true ); i <= ves->lastI ( true ); ++i ) {
         
         for ( unsigned int j = ves->firstJ ( true ); j <= ves->lastJ ( true ); ++j ) {
            
            if ( propertyManager.getNodeIsValid ( i , j ) ) { 
                
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
      derivedProperties.push_back ( ves );
   }
}

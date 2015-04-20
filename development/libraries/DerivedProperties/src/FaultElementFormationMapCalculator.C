#include "FaultElementFormationMapCalculator.h"

#include "DerivedFormationMapProperty.h"
#include "GeoPhysicsFormation.h"

DerivedProperties::FaultElementFormationMapCalculator::FaultElementFormationMapCalculator () {
   addPropertyName ( "FaultElements" );
}

void DerivedProperties::FaultElementFormationMapCalculator::calculate ( AbstractPropertyManager&            propertyManager,
                                                                        const DataModel::AbstractSnapshot*  snapshot,
                                                                        const DataModel::AbstractFormation* formation,
                                                                              FormationMapPropertyList&     derivedProperties ) const {

   const DataModel::AbstractProperty* faultElementProperty = propertyManager.getProperty ( "FaultElements" );
   const GeoPhysics::Formation* geophysicsFormation = dynamic_cast<const GeoPhysics::Formation*>( formation );

   derivedProperties.clear ();

   DerivedFormationMapPropertyPtr faultElements = DerivedFormationMapPropertyPtr ( new DerivedProperties::DerivedFormationMapProperty ( faultElementProperty,
                                                                                                                                        snapshot,
                                                                                                                                        formation,
                                                                                                                                        propertyManager.getMapGrid ()));
   double undefinedValue = faultElements->getUndefinedValue ();

   for ( unsigned int i = faultElements->firstI ( true ); i <= faultElements->lastI ( true ); ++i ) {

      for ( unsigned int j = faultElements->firstJ ( true ); j <= faultElements->lastJ ( true ); ++j ) {

         if ( propertyManager.getNodeIsValid ( i, j )) {

            if ( geophysicsFormation->getCompoundLithology ( i, j )->isFault ()) {
               faultElements->set ( i, j, 1.0 );
            } else {
               faultElements->set ( i, j, 0.0 );
            }

         } else {
            faultElements->set ( i, j, undefinedValue );
         }

      }

   }

   derivedProperties.push_back ( faultElements );
}

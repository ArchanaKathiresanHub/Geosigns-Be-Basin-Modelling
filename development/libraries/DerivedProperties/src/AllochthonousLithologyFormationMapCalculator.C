#include "AllochthonousLithologyFormationMapCalculator.h"

#include "AbstractProperty.h"

#include "AbstractPropertyManager.h"
#include "DerivedFormationProperty.h"
#include "DerivedFormationMapProperty.h"
#include "DerivedPropertyManager.h"

#include "GeoPhysicsFormation.h"

#include "PropertyRetriever.h"

DerivedProperties::AllochthonousLithologyFormationMapCalculator::AllochthonousLithologyFormationMapCalculator () {
   addPropertyName ( "AllochthonousLithology" );
}

void DerivedProperties::AllochthonousLithologyFormationMapCalculator::calculate ( AbstractPropertyManager&            propertyManager,
                                                                                  const DataModel::AbstractSnapshot*  snapshot,
                                                                                  const DataModel::AbstractFormation* formation,
                                                                                        FormationMapPropertyList&     derivedProperties ) const {

   const DataModel::AbstractProperty* allochLithoProperty = propertyManager.getProperty ( "AllochthonousLithology" );
   const GeoPhysics::Formation* geoFormation = dynamic_cast<const GeoPhysics::Formation*>( formation );

   derivedProperties.clear ();

   if ( geoFormation != 0  ) {
      const double age = snapshot->getTime ();
      const GeoPhysics::CompoundLithologyArray& lithologies = geoFormation->getCompoundLithologyArray ();

      DerivedFormationMapPropertyPtr allochthonousLithology = DerivedFormationMapPropertyPtr ( new DerivedProperties::DerivedFormationMapProperty ( allochLithoProperty,
                                                                                                                                                    snapshot,
                                                                                                                                                    formation,
                                                                                                                                                    propertyManager.getMapGrid ()));
      double undefinedValue = allochthonousLithology->getUndefinedValue ();
      double value;

      for ( unsigned int i = allochthonousLithology->firstI ( true ); i <= allochthonousLithology->lastI ( true ); ++i ) {

         for ( unsigned int j = allochthonousLithology->firstJ ( true ); j <= allochthonousLithology->lastJ ( true ); ++j ) {

            if ( propertyManager.getNodeIsValid ( i, j )) {

               if ( lithologies.isAllochthonousAtAge ( i, j, age )) {
                  value = 1.0;
               } else {
                  value = 0.0;
               }

            } else {
               value = undefinedValue;
            }

            allochthonousLithology->set ( i, j, value );
         }

      }

      derivedProperties.push_back ( allochthonousLithology );
   }

}

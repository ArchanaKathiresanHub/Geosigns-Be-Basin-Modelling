#include "ThicknessFormationMapCalculator.h"

#include "AbstractPropertyManager.h"
#include "DerivedFormationProperty.h"
#include "DerivedFormationMapProperty.h"
#include "DerivedPropertyManager.h"

#include "GeoPhysicsFormation.h"

#include "PropertyRetriever.h"

DerivedProperties::ThicknessFormationMapCalculator::ThicknessFormationMapCalculator () {
   addPropertyName ( "Thickness" );
}

void DerivedProperties::ThicknessFormationMapCalculator::calculate ( AbstractPropertyManager&            propertyManager,
                                                                     const DataModel::AbstractSnapshot*  snapshot,
                                                                     const DataModel::AbstractFormation* formation,
                                                                           FormationMapPropertyList&     derivedProperties ) const {

   const DataModel::AbstractProperty* thicknessProperty = propertyManager.getProperty ( "Thickness" );
   const DataModel::AbstractProperty* depthProperty = propertyManager.getProperty ( "Depth" );
   const FormationPropertyPtr depth = propertyManager.getFormationProperty ( depthProperty, snapshot, formation );

   PropertyRetriever maxVesRetriever ( depth );

   derivedProperties.clear ();

   if( depth != 0  ) {
      DerivedFormationMapPropertyPtr thickness = DerivedFormationMapPropertyPtr ( new DerivedProperties::DerivedFormationMapProperty ( thicknessProperty,
                                                                                                                                       snapshot,
                                                                                                                                       formation,
                                                                                                                                       propertyManager.getMapGrid ()));
      unsigned int nodeCount = depth->lastK ();
      double undefinedValue = depth->getUndefinedValue ();

      for ( unsigned int i = depth->firstI ( true ); i <= depth->lastI ( true ); ++i ) {

         for ( unsigned int j = depth->firstJ ( true ); j <= depth->lastJ ( true ); ++j ) {

            if ( propertyManager.getNodeIsValid ( i, j )) {
               thickness->set ( i, j, depth->get ( i, j, 0 ) - depth->get ( i, j, nodeCount ));
            } else {
               thickness->set ( i, j, undefinedValue );
            }

         }

      }

      derivedProperties.push_back ( thickness );
   }

}

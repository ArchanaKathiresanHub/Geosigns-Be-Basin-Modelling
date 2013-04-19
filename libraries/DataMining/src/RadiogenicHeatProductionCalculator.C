#include "RadiogenicHeatProductionCalculator.h"

#include "GeoPhysicsFormation.h"
#include "CompoundLithology.h"

DataAccess::Mining::RadiogenicHeatProductionCalculator::RadiogenicHeatProductionCalculator ( const DomainPropertyCollection*  collection,
                                                                                             const Interface::Snapshot* snapshot,
                                                                                             const Interface::Property* property ) :
   DomainFormationProperty ( collection, snapshot, property )
{
}

double DataAccess::Mining::RadiogenicHeatProductionCalculator::compute ( const ElementPosition& position ) const {

   if ( not position.isValidElement () or position.getFormation () == 0 ) {
      return Interface::DefaultUndefinedMapValue;
   }

   const Interface::Formation* formation = position.getFormation ();

   const GeoPhysics::Formation* geoForm = dynamic_cast<const GeoPhysics::Formation*>( formation );



   const double xi  = position.getReferencePoint ().x ();
   const double eta = position.getReferencePoint ().y ();

   double heatProduction;

   if ( geoForm != 0 and xi != DataAccess::Interface::DefaultUndefinedMapValue and eta != DataAccess::Interface::DefaultUndefinedMapValue ) {

      double heatProductionWeights [ 4 ];
      double basisFunctions [ 4 ];

      const unsigned int i = position.getI ();
      const unsigned int j = position.getJ ();

      int k;


      basisFunctions [ 0 ] = 0.25 * ( 1.0 - xi ) * ( 1.0 - eta );
      basisFunctions [ 1 ] = 0.25 * ( 1.0 + xi ) * ( 1.0 - eta );
      basisFunctions [ 2 ] = 0.25 * ( 1.0 + xi ) * ( 1.0 + eta );
      basisFunctions [ 3 ] = 0.25 * ( 1.0 - xi ) * ( 1.0 + eta );

      if ( geoForm->getCompoundLithology ( i, j ) != 0 ) {
         heatProductionWeights [ 0 ] = geoForm->getCompoundLithology ( i, j )->heatproduction ();
      } else {
         heatProductionWeights [ 0 ] = Interface::DefaultUndefinedMapValue;
      }

      if ( geoForm->getCompoundLithology ( i + 1, j ) != 0 ) {
         heatProductionWeights [ 1 ] = geoForm->getCompoundLithology ( i + 1, j )->heatproduction ();
      } else {
         heatProductionWeights [ 1 ] = Interface::DefaultUndefinedMapValue;
      }

      if ( geoForm->getCompoundLithology ( i + 1, j + 1 ) != 0 ) {
         heatProductionWeights [ 2 ] = geoForm->getCompoundLithology ( i + 1, j + 1 )->heatproduction ();
      } else {
         heatProductionWeights [ 2 ] = Interface::DefaultUndefinedMapValue;
      }

      if ( geoForm->getCompoundLithology ( i, j + 1 ) != 0 ) {
         heatProductionWeights [ 3 ] = geoForm->getCompoundLithology ( i, j + 1 )->heatproduction ();
      } else {
         heatProductionWeights [ 3 ] = Interface::DefaultUndefinedMapValue;
      }

      for ( k = 0; k < 4; ++k ) {

         // The value may be null for various reasons.
         if ( heatProductionWeights [ k ] == Interface::DefaultUndefinedMapValue ) {
            return Interface::DefaultUndefinedMapValue;
         }

      }

      heatProduction = 0.0;

      for ( k = 0; k < 4; ++k ) {
         heatProduction += basisFunctions [ k ] * heatProductionWeights [ k ];
      }

   } else {
      heatProduction = Interface::DefaultUndefinedMapValue;
   }

   return heatProduction;
}

DataAccess::Mining::DomainProperty* DataAccess::Mining::RadiogenicHeatProductionCalculatorAllocator::allocate ( const DomainPropertyCollection*  collection,
                                                                                                                const Interface::Snapshot* snapshot,
                                                                                                                const Interface::Property* property ) const {
   return new RadiogenicHeatProductionCalculator ( collection, snapshot, property );
}

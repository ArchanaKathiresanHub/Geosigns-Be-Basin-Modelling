#include "PropertyInterpolator2D.h"

#include "Point.h"

double DataAccess::Mining::PropertyInterpolator2D::operator ()( const ElementPosition&                element,
                                                                const DataAccess::Interface::GridMap* property ) const {

   return operator ()( element, property, element.getLocalK ());
}

double DataAccess::Mining::PropertyInterpolator2D::operator ()( const ElementPosition&                element,
                                                                const DataAccess::Interface::GridMap* property,
                                                                const unsigned int                    k ) const {

   int l;
   double weights [ 4 ];
   double phi [ 4 ];
   double result;

   const unsigned int i = element.getI ();
   const unsigned int j = element.getJ ();

   const double xi  = element.getReferencePoint ().x ();
   const double eta = element.getReferencePoint ().y ();

   if ( xi  == DataAccess::Interface::DefaultUndefinedMapValue or
        eta == DataAccess::Interface::DefaultUndefinedMapValue ) {
      return DataAccess::Interface::DefaultUndefinedMapValue;
   }

   weights [ 0 ] = property->getValue ( i, j, k );
   weights [ 1 ] = property->getValue ( i + 1, j, k );
   weights [ 2 ] = property->getValue ( i + 1, j + 1, k );
   weights [ 3 ] = property->getValue ( i, j + 1, k );

   phi [ 0 ] = 0.25 * ( 1.0 - xi ) * ( 1.0 - eta );
   phi [ 1 ] = 0.25 * ( 1.0 + xi ) * ( 1.0 - eta );
   phi [ 2 ] = 0.25 * ( 1.0 + xi ) * ( 1.0 + eta );
   phi [ 3 ] = 0.25 * ( 1.0 - xi ) * ( 1.0 + eta );

   for ( l = 0; l < 4; ++l ) {

      if ( weights [ l ] == DataAccess::Interface::DefaultUndefinedMapValue ) {
         return DataAccess::Interface::DefaultUndefinedMapValue;
      }

   }

   result = 0.0;

   for ( l = 0; l < 4; ++l ) {
      result += weights [ l ] * phi [ l ];
   }

   return result;
}

double DataAccess::Mining::PropertyInterpolator2D::operator ()( const ElementPosition&                element,
                                                                DerivedProperties::SurfacePropertyPtr property ) const {

   int l;
   double weights [ 4 ];
   double phi [ 4 ];
   double result;

   const unsigned int i = element.getI ();
   const unsigned int j = element.getJ ();

   const double xi  = element.getReferencePoint ().x ();
   const double eta = element.getReferencePoint ().y ();

   if ( xi  == DataAccess::Interface::DefaultUndefinedMapValue or
        eta == DataAccess::Interface::DefaultUndefinedMapValue ) {
      return DataAccess::Interface::DefaultUndefinedMapValue;
   }

   weights [ 0 ] = property->get ( i,     j );
   weights [ 1 ] = property->get ( i + 1, j );
   weights [ 2 ] = property->get ( i + 1, j + 1 );
   weights [ 3 ] = property->get ( i,     j + 1 );

   phi [ 0 ] = 0.25 * ( 1.0 - xi ) * ( 1.0 - eta );
   phi [ 1 ] = 0.25 * ( 1.0 + xi ) * ( 1.0 - eta );
   phi [ 2 ] = 0.25 * ( 1.0 + xi ) * ( 1.0 + eta );
   phi [ 3 ] = 0.25 * ( 1.0 - xi ) * ( 1.0 + eta );

   for ( l = 0; l < 4; ++l ) {

      if ( weights [ l ] == DataAccess::Interface::DefaultUndefinedMapValue ) {
         return DataAccess::Interface::DefaultUndefinedMapValue;
      }

   }

   result = 0.0;

   for ( l = 0; l < 4; ++l ) {
      result += weights [ l ] * phi [ l ];
   }

   return result;
}

double DataAccess::Mining::PropertyInterpolator2D::operator ()( const ElementPosition&                     element,
                                                                DerivedProperties::FormationMapPropertyPtr property ) const {

   int l;
   double weights [ 4 ];
   double phi [ 4 ];
   double result;

   const unsigned int i = element.getI ();
   const unsigned int j = element.getJ ();

   const double xi  = element.getReferencePoint ().x ();
   const double eta = element.getReferencePoint ().y ();

   if ( xi  == DataAccess::Interface::DefaultUndefinedMapValue or
        eta == DataAccess::Interface::DefaultUndefinedMapValue ) {
      return DataAccess::Interface::DefaultUndefinedMapValue;
   }

   weights [ 0 ] = property->get ( i,     j );
   weights [ 1 ] = property->get ( i + 1, j );
   weights [ 2 ] = property->get ( i + 1, j + 1 );
   weights [ 3 ] = property->get ( i,     j + 1 );

   phi [ 0 ] = 0.25 * ( 1.0 - xi ) * ( 1.0 - eta );
   phi [ 1 ] = 0.25 * ( 1.0 + xi ) * ( 1.0 - eta );
   phi [ 2 ] = 0.25 * ( 1.0 + xi ) * ( 1.0 + eta );
   phi [ 3 ] = 0.25 * ( 1.0 - xi ) * ( 1.0 + eta );

   for ( l = 0; l < 4; ++l ) {

      if ( weights [ l ] == DataAccess::Interface::DefaultUndefinedMapValue ) {
         return DataAccess::Interface::DefaultUndefinedMapValue;
      }

   }

   result = 0.0;

   for ( l = 0; l < 4; ++l ) {
      result += weights [ l ] * phi [ l ];
   }

   return result;
}


double DataAccess::Mining::PropertyInterpolator2D::operator ()( const ElementPosition&                  element,
                                                                DerivedProperties::FormationPropertyPtr property ) const {
   return operator ()( element, property, element.getLocalK ());
}

double DataAccess::Mining::PropertyInterpolator2D::operator ()( const ElementPosition&                  element,
                                                                DerivedProperties::FormationPropertyPtr property,
                                                                const unsigned int                      k ) const {

   int l;
   double weights [ 4 ];
   double phi [ 4 ];
   double result;

   const unsigned int i = element.getI ();
   const unsigned int j = element.getJ ();

   const double xi  = element.getReferencePoint ().x ();
   const double eta = element.getReferencePoint ().y ();

   if ( xi  == DataAccess::Interface::DefaultUndefinedMapValue or
        eta == DataAccess::Interface::DefaultUndefinedMapValue ) {
      return DataAccess::Interface::DefaultUndefinedMapValue;
   }

   weights [ 0 ] = property->get ( i, j, k );
   weights [ 1 ] = property->get ( i + 1, j, k );
   weights [ 2 ] = property->get ( i + 1, j + 1, k );
   weights [ 3 ] = property->get ( i, j + 1, k );

   phi [ 0 ] = 0.25 * ( 1.0 - xi ) * ( 1.0 - eta );
   phi [ 1 ] = 0.25 * ( 1.0 + xi ) * ( 1.0 - eta );
   phi [ 2 ] = 0.25 * ( 1.0 + xi ) * ( 1.0 + eta );
   phi [ 3 ] = 0.25 * ( 1.0 - xi ) * ( 1.0 + eta );

   for ( l = 0; l < 4; ++l ) {

      if ( weights [ l ] == DataAccess::Interface::DefaultUndefinedMapValue ) {
         return DataAccess::Interface::DefaultUndefinedMapValue;
      }

   }

   result = 0.0;

   for ( l = 0; l < 4; ++l ) {
      result += weights [ l ] * phi [ l ];
   }

   return result;
}

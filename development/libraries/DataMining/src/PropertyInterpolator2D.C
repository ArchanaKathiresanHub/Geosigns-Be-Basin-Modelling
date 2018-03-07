//
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "PropertyInterpolator2D.h"

#include "Point.h"

using namespace AbstractDerivedProperties;

double DataAccess::Mining::PropertyInterpolator2D::doInterpolation ( const double  xi,
                                                                     const double  eta,
                                                                     const double* weights) const {

   double phi [ 4 ];
   int l;

   phi [ 0 ] = 0.25 * ( 1.0 - xi ) * ( 1.0 - eta );
   phi [ 1 ] = 0.25 * ( 1.0 + xi ) * ( 1.0 - eta );
   phi [ 2 ] = 0.25 * ( 1.0 + xi ) * ( 1.0 + eta );
   phi [ 3 ] = 0.25 * ( 1.0 - xi ) * ( 1.0 + eta );

   for ( l = 0; l < 4; ++l ) {

      if ( weights [ l ] == DataAccess::Interface::DefaultUndefinedMapValue ) {
         return DataAccess::Interface::DefaultUndefinedMapValue;
      }

   }

   double result = 0.0;

   for ( l = 0; l < 4; ++l ) {
      result += weights [ l ] * phi [ l ];
   }

   return result;

}


double DataAccess::Mining::PropertyInterpolator2D::operator ()( const ElementPosition&   element,
                                                                      SurfacePropertyPtr property ) const {

   double weights [ 4 ];

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

   return doInterpolation ( xi, eta, weights );
}

double DataAccess::Mining::PropertyInterpolator2D::operator ()( const ElementPosition&        element,
                                                                      FormationMapPropertyPtr property ) const {

   double weights [ 4 ];

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

   return doInterpolation ( xi, eta, weights );
}


double DataAccess::Mining::PropertyInterpolator2D::operator ()( const ElementPosition&     element,
                                                                      FormationPropertyPtr property ) const {
   return operator ()( element, property, element.getLocalK ());
}

double DataAccess::Mining::PropertyInterpolator2D::operator ()( const ElementPosition&     element,
                                                                      FormationPropertyPtr property,
                                                                const unsigned int         k ) const {

   double weights [ 4 ];

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

   return doInterpolation ( xi, eta, weights );
}

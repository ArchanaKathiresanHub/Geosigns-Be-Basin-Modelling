//
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "PropertyInterpolator3D.h"

using namespace AbstractDerivedProperties;

double DataAccess::Mining::PropertyInterpolator3D::operator ()( const ElementPosition&     element,
                                                                      FormationPropertyPtr property ) const {

   // Check for element vlidity.

   int l;
   double weights [ 8 ];
   double phi [ 8 ];
   double result;

   const unsigned int i = element.getI ();
   const unsigned int j = element.getJ ();
   const unsigned int k = element.getLocalK ();

   const double xi   = element.getReferencePoint ().x ();
   const double eta  = element.getReferencePoint ().y ();
   const double zeta = element.getReferencePoint ().z ();

   if ( xi   == DataAccess::Interface::DefaultUndefinedMapValue or
        eta  == DataAccess::Interface::DefaultUndefinedMapValue or
        zeta == DataAccess::Interface::DefaultUndefinedMapValue ) {
      return DataAccess::Interface::DefaultUndefinedMapValue;
   }
   
   weights [ 0 ] = property->get ( i, j, k );
   weights [ 1 ] = property->get ( i + 1, j, k );
   weights [ 2 ] = property->get ( i + 1, j + 1, k );
   weights [ 3 ] = property->get ( i, j + 1, k );
   weights [ 4 ] = property->get ( i, j, k - 1 );
   weights [ 5 ] = property->get ( i + 1, j, k - 1 );
   weights [ 6 ] = property->get ( i + 1, j + 1, k - 1 );
   weights [ 7 ] = property->get ( i, j + 1, k - 1 );
     
   for ( l = 0; l < 8; ++l ) {

      if ( weights [ l ] == DataAccess::Interface::DefaultUndefinedMapValue ) {
         return DataAccess::Interface::DefaultUndefinedMapValue;
      }

   }

   // Set the reference element basis function.
   phi [ 0 ] = 0.125 * ( 1.0 - xi ) * ( 1.0 - eta ) * ( 1.0 - zeta );
   phi [ 1 ] = 0.125 * ( 1.0 + xi ) * ( 1.0 - eta ) * ( 1.0 - zeta );
   phi [ 2 ] = 0.125 * ( 1.0 + xi ) * ( 1.0 + eta ) * ( 1.0 - zeta );
   phi [ 3 ] = 0.125 * ( 1.0 - xi ) * ( 1.0 + eta ) * ( 1.0 - zeta );
   phi [ 4 ] = 0.125 * ( 1.0 - xi ) * ( 1.0 - eta ) * ( 1.0 + zeta );
   phi [ 5 ] = 0.125 * ( 1.0 + xi ) * ( 1.0 - eta ) * ( 1.0 + zeta );
   phi [ 6 ] = 0.125 * ( 1.0 + xi ) * ( 1.0 + eta ) * ( 1.0 + zeta );
   phi [ 7 ] = 0.125 * ( 1.0 - xi ) * ( 1.0 + eta ) * ( 1.0 + zeta );

   result = 0.0;

   // Calculate inner-product of basis-functions with property-values.
   for ( l = 0; l < 8; ++l ) {
      result += weights [ l ] * phi [ l ];
   }

   return result;
}

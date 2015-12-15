#ifndef _MINING__PROPERTY_INTERPOLATOR_2D_H_
#define _MINING__PROPERTY_INTERPOLATOR_2D_H_

#include "ElementPosition.h"

#include "Interface/GridMap.h"

#include "FormationProperty.h"
#include "FormationMapProperty.h"
#include "SurfaceProperty.h"

namespace DataAccess {

   namespace Mining {

      class PropertyInterpolator2D {

      public :


         /// Interpolate a property in a 2d map.
         double operator ()( const ElementPosition&                element,
                             DerivedProperties::SurfacePropertyPtr property ) const;

         /// Interpolate a property in a 2d map.
         double operator ()( const ElementPosition&                     element,
                             DerivedProperties::FormationMapPropertyPtr property ) const;

         /// Interpolate a property in a 3d map.
         double operator ()( const ElementPosition&                  element,
                             DerivedProperties::FormationPropertyPtr property,
                             const unsigned int                      k ) const;

         /// Interpolate a property.
         double operator ()( const ElementPosition&                  element,
                             DerivedProperties::FormationPropertyPtr property ) const;


      private :

         double doInterpolation ( const double  xi,
                                  const double  eta,
                                  const double* weights) const;

      };

   }

}

#endif // _MINING__PROPERTY_INTERPOLATOR_2D_H_

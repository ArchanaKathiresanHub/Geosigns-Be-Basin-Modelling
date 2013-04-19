#ifndef _MINING__PROPERTY_INTERPOLATOR_2D_H_
#define _MINING__PROPERTY_INTERPOLATOR_2D_H_

#include "ElementPosition.h"

#include "Interface/GridMap.h"

namespace DataAccess {

   namespace Mining {

      class PropertyInterpolator2D {

      public :

         /// Interpolate a property in a 2d map.
         double operator ()( const ElementPosition&                element,
                             const DataAccess::Interface::GridMap* property ) const;

         /// Interpolate a property in a 3d map.
         double operator ()( const ElementPosition&                element,
                             const DataAccess::Interface::GridMap* property,
                             const unsigned int                    k ) const;

      };

   }

}

#endif // _MINING__PROPERTY_INTERPOLATOR_2D_H_

#ifndef _MINING__PROPERTY_INTERPOLATOR_3D_H_
#define _MINING__PROPERTY_INTERPOLATOR_3D_H_

#include "FormationProperty.h"

#include "ElementPosition.h"

#include "Interface/GridMap.h"

namespace DataAccess {

   namespace Mining {

      class PropertyInterpolator3D {

      public :

         /// Interpolate a property in a 3d map.
         double operator ()( const ElementPosition&                  element,
                             DerivedProperties::FormationPropertyPtr property ) const;

      };

   }

}

#endif // _MINING__PROPERTY_INTERPOLATOR_3D_H_

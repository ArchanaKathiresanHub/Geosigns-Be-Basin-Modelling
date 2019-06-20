//
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _MINING__PROPERTY_INTERPOLATOR_3D_H_
#define _MINING__PROPERTY_INTERPOLATOR_3D_H_

#include "FormationProperty.h"

#include "ElementPosition.h"

#include "GridMap.h"

namespace DataAccess {

   namespace Mining {

      class PropertyInterpolator3D {

      public :

         /// Interpolate a property in a 3d map.
         double operator ()( const ElementPosition&                          element,
                             AbstractDerivedProperties::FormationPropertyPtr property ) const;

      };

   }

}

#endif // _MINING__PROPERTY_INTERPOLATOR_3D_H_

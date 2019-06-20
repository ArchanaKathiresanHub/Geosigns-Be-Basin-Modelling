//
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _MINING__PROPERTY_INTERPOLATOR_2D_H_
#define _MINING__PROPERTY_INTERPOLATOR_2D_H_

#include "ElementPosition.h"

#include "GridMap.h"

#include "FormationProperty.h"
#include "FormationMapProperty.h"
#include "SurfaceProperty.h"

namespace DataAccess {

   namespace Mining {

      class PropertyInterpolator2D {

      public :


         /// Interpolate a property in a 2d map.
         double operator ()( const ElementPosition&                        element,
                             AbstractDerivedProperties::SurfacePropertyPtr property ) const;

         /// Interpolate a property in a 2d map.
         double operator ()( const ElementPosition&                             element,
                             AbstractDerivedProperties::FormationMapPropertyPtr property ) const;

         /// Interpolate a property in a 3d map.
         double operator ()( const ElementPosition&                          element,
                             AbstractDerivedProperties::FormationPropertyPtr property,
                             const unsigned int                              k ) const;

         /// Interpolate a property.
         double operator ()( const ElementPosition&                          element,
                             AbstractDerivedProperties::FormationPropertyPtr property ) const;


      private :

         double doInterpolation ( const double  xi,
                                  const double  eta,
                                  const double* weights) const;

      };

   }

}

#endif // _MINING__PROPERTY_INTERPOLATOR_2D_H_

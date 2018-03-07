//
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef DERIVED_PROPERTIES__LITHOSTATIC_PRESSURE_SURFACE_CALCULATOR_H
#define DERIVED_PROPERTIES__LITHOSTATIC_PRESSURE_SURFACE_CALCULATOR_H

#include "SurfacePropertyCalculator.h"

namespace DerivedProperties {

   class  LithostaticPressureSurfaceCalculator : public AbstractDerivedProperties::SurfacePropertyCalculator {

   public :

      LithostaticPressureSurfaceCalculator ( const GeoPhysics::ProjectHandle* projectHandle );

      virtual ~ LithostaticPressureSurfaceCalculator () = default;
 
      virtual void calculate (       AbstractDerivedProperties::AbstractPropertyManager& propManager,
                               const DataModel::AbstractSnapshot*                        snapshot,
                               const DataModel::AbstractSurface*                         surface,
                                     AbstractDerivedProperties::SurfacePropertyList&     derivedProperties ) const;
   private :

      const GeoPhysics::ProjectHandle* m_projectHandle;

    };

}

#endif 

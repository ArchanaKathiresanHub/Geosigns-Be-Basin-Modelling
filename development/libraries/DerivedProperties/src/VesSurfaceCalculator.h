//
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef DERIVED_PROPERTIES__VES_SURFACE_CALCULATOR_H
#define DERIVED_PROPERTIES__VES_SURFACE_CALCULATOR_H

#include "SurfacePropertyCalculator.h"

namespace DerivedProperties {

   class VesSurfaceCalculator : public AbstractDerivedProperties::SurfacePropertyCalculator {

   public :

      VesSurfaceCalculator ();

      virtual ~VesSurfaceCalculator ()= default;
 
      virtual void calculate (       AbstractDerivedProperties::AbstractPropertyManager& propManager,
                               const DataModel::AbstractSnapshot*                        snapshot,
                               const DataModel::AbstractSurface*                         surface,
                                     AbstractDerivedProperties::SurfacePropertyList&     derivedProperties ) const;

    };


}

#endif 

//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "MapInterpolatorFactory.h"
#include "MapInterpolatorInverseDistanceWeighting.h"
#include "MapInterpolatorNearestNeighbor.h"

namespace casa
{

namespace MapInterpolatorFactory
{

std::unique_ptr<MapInterpolator> createMapInterpolator(const int interpolationMethod, const double idwPower)
{
   if (interpolationMethod == 1)
   {
      return std::move(std::make_unique<casa::MapInterpolatorNearestNeighbor>());
   }
   else
   {
      return std::move(std::make_unique<casa::MapInterpolatorInverseDistanceWeighting>(idwPower));
   }
}

}

}

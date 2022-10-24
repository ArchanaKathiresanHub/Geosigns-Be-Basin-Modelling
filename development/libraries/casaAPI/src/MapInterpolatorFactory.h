//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include "MapInterpolator.h"

#include <memory>

namespace casa
{
namespace MapInterpolatorFactory
{

std::unique_ptr<MapInterpolator> createMapInterpolator(const int interpolationMethod, const double idwPower);

}

}

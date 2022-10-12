//
// Copyright (C) 2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "Column.h"

#include "depthToVolume.cpp"

using migration::Column;

namespace migration { namespace depthToVolume {

template
const MonotonicIncreasingPiecewiseLinearInvertableFunction* compute(
   const double& origin, std::vector<Column*>::const_iterator begin, std::vector<Column*>::const_iterator end, double spillDepth);

template
const MonotonicIncreasingPiecewiseLinearInvertableFunction* compute(
   const double& origin, std::vector<Column*>::iterator begin, std::vector<Column*>::iterator end, double spillDepth);

} } // namespace migration::depthToVolume

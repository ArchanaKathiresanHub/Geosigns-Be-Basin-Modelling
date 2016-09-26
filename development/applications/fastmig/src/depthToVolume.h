//
// Copyright (C) 2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _MIGRATION_DEPTHTOVOLUME_H_
#define _MIGRATION_DEPTHTOVOLUME_H_

#include "MonotonicIncreasingPiecewiseLinearInvertableFunction.h"

#include <vector>

using std::vector;
using functions::MonotonicIncreasingPiecewiseLinearInvertableFunction;

namespace migration { namespace depthToVolume {

template <typename ITERATOR>
const MonotonicIncreasingPiecewiseLinearInvertableFunction* compute(
   const double& topDepth, ITERATOR begin, ITERATOR end, double spillDepth);

} }

#endif

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

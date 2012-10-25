#include "Column.h"

#include "depthToVolume.C"

using migration::Column;

namespace migration { namespace depthToVolume {

template
const MonotonicIncreasingPiecewiseLinearInvertableFunction* compute(
   const double& origin, vector<Column*>::const_iterator begin, vector<Column*>::const_iterator end, double spillDepth);

template
const MonotonicIncreasingPiecewiseLinearInvertableFunction* compute(
   const double& origin, vector<Column*>::iterator begin, vector<Column*>::iterator end, double spillDepth);

} } // namespace migration::depthToVolume

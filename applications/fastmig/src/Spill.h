#ifndef _MIGRATION_DISTRIBUTE_SPILL_H_
#define _MIGRATION_DISTRIBUTE_SPILL_H_

#include "functions/src/MonotonicIncreasingPiecewiseLinearInvertableFunction.h"
#include "functions/src/Tuple2.h"

using functions::MonotonicIncreasingPiecewiseLinearInvertableFunction;
using functions::Tuple2;

namespace migration { namespace distribute {

class Spill
{
private:

   Tuple2<double> m_capacity;

public:

   Spill(const MonotonicIncreasingPiecewiseLinearInvertableFunction* levelToVolume);

   void distribute(const double& fluidVolume, double& fluidSpilled) const;

   const double& maxLevel() const { return m_capacity[0]; }
   const double& maxVolume() const { return m_capacity[1]; }
   const Tuple2<double>& maxContent() const { return m_capacity; }
};

} } // namespace migration::distribute

#endif

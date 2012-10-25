#ifndef _MIGRATION_DISTRIBUTE_WASTE_H_
#define _MIGRATION_DISTRIBUTE_WASTE_H_

#include "functions/src/Tuple2.h"
#include "functions/src/MonotonicIncreasingPiecewiseLinearInvertableFunction.h"

using functions::MonotonicIncreasingPiecewiseLinearInvertableFunction;
using functions::Tuple2;

namespace migration { namespace distribute {

class Waste
{
private:

   Tuple2<double> m_maxContent;

public:

   Waste(const double& wasteLevel, const MonotonicIncreasingPiecewiseLinearInvertableFunction* 
      levelToVolume);

   void distribute(const double& fluidVolume, double& fluidWasted) const;

   const double& maxLevel() const { return m_maxContent[0]; }
   const double& maxVolume() const { return m_maxContent[1]; }
   const Tuple2<double>& maxContent() const { return m_maxContent; }
};

} } // namespace migration::distribute

#endif

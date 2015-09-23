#ifndef _MIGRATION_DISTRIBUTE_SPILLOILANDGAS_H_
#define _MIGRATION_DISTRIBUTE_SPILLOILANDGAS_H_

#include "Spill.h"

#include "MonotonicIncreasingPiecewiseLinearInvertableFunction.h"
#include "Tuple2.h"

using functions::MonotonicIncreasingPiecewiseLinearInvertableFunction;
using functions::Tuple2;

namespace migration { namespace distribute {

class SpillOilAndGas
{
private:

   Spill m_spill;

public:

   SpillOilAndGas(const MonotonicIncreasingPiecewiseLinearInvertableFunction* levelToVolume);

   void distribute(const double& gasVolume, const double& oilVolume, double& gasVolumeSpilled, 
      double& oilVolumeSpilled) const;

   const double& maxLevel() const { return m_spill.maxLevel(); }
   const double& maxVolume() const { return m_spill.maxVolume(); }
   const Tuple2<double>& maxContent() const { return m_spill.maxContent(); }
};

} } // namespace migration::distribute

#endif

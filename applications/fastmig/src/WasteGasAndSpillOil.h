#ifndef _MIGRATION_DISTRIBUTE_WASTEGASANDSPILLOIL_H_
#define _MIGRATION_DISTRIBUTE_WASTEGASANDSPILLOIL_H_

#include "Spill.h"
#include "Waste.h"

#include "functions/src/MonotonicIncreasingPiecewiseLinearInvertableFunction.h"
#include "functions/src/Tuple2.h"

using functions::MonotonicIncreasingPiecewiseLinearInvertableFunction;
using functions::Tuple2;

namespace migration { namespace distribute {

class WasteGasAndSpillOil
{
private:

   Spill m_spillOil;
   Waste m_wasteGas;

public:

   WasteGasAndSpillOil(const double& wasteLevel, 
      const MonotonicIncreasingPiecewiseLinearInvertableFunction* levelToVolume);

   void distribute(const double& gasVolume, const double& oilVolume, 
      double& gasVolumeWasted, double& oilVolumeSpilled) const;
};

} } // namespace migration::distribute

#endif

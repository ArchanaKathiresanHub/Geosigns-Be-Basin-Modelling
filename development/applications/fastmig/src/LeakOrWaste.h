#ifndef _MIGRATION_DISTRIBUTE_LEAKORWASTE_H_
#define _MIGRATION_DISTRIBUTE_LEAKORWASTE_H_

#include "Leak.h"
#include "Waste.h"

#include "functions/src/MonotonicIncreasingPiecewiseLinearInvertableFunction.h"
#include "functions/src/Tuple2.h"

using functions::MonotonicIncreasingPiecewiseLinearInvertableFunction;
using functions::Tuple2;

namespace migration { namespace distribute {

class LeakOrWaste
{
private:

   Leak m_leak;
   Waste m_waste;

   bool m_leaking;

public:

   LeakOrWaste(const double& fluidDensity, const double& sealFluidDensity, 
      const double& maxSealPressure, const double& wasteLevel,  
      const MonotonicIncreasingPiecewiseLinearInvertableFunction* levelToVolume);

   void distribute(const double& fluidVolume, double& fluidVolumeLeaked, 
      double& fluidVolumeWasted) const;

   const double& maxLevel() const { 
      return m_leaking ? m_leak.maxLevel() : m_waste.maxLevel(); 
   }
   const double& maxVolume() const { 
      return m_leaking ? m_leak.maxVolume() : m_waste.maxVolume();
   }
   const Tuple2<double>& maxContent() const { 
      return m_leaking ? m_leak.maxContent() : m_waste.maxContent();
   }
   bool leaking() const { return m_leaking; }
};

} } // namespace migration::distribute

#endif

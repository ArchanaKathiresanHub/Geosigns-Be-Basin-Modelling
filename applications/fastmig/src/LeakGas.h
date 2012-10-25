#ifndef _MIGRATION_DISTRIBUTE_LEAKGAS_H_
#define _MIGRATION_DISTRIBUTE_LEAKGAS_H_

#include "Leak.h"

#include "functions/src/MonotonicIncreasingPiecewiseLinearInvertableFunction.h"
#include "functions/src/Tuple2.h"

using functions::MonotonicIncreasingPiecewiseLinearInvertableFunction;
using functions::Tuple2;

namespace migration { namespace distribute {

class LeakGas
{
private:

   const MonotonicIncreasingPiecewiseLinearInvertableFunction* m_levelToVolume;

   double m_oilToGasLevelRatio;

   Leak m_leakGas;
   Leak m_leakOil;

#ifdef DEBUG_LEAKGAS
   Tuple2<double> m_capacity;
#endif

   double computeOilToGasLevelRatio(const double& gasDensity, const double& oilDensity, 
      const double& sealFluidDensity) const;

   double computeFinalGasVolume(const double& oilVolume, const Tuple2<Tuple2<double> >& gasLimits, 
      const Tuple2<Tuple2<double> >& hcLimits) const;

public:

   LeakGas(const double& gasDensity, const double& oilDensity, const double& sealFluidDensity, 
      const double& fracturePressure, const double& capPressure_H2O_Gas, 
      const double& capPressure_H2O_Oil, 
      const MonotonicIncreasingPiecewiseLinearInvertableFunction* levelToVolume);

   void distribute(const double& gasVolume, const double& oilVolume, double& gasVolumeLeaked,
      double& oilVolumeLeaked) const;

   const double& gasDensity() const { return m_leakGas.fluidDensity(); }
   const double& oilDensity() const { return m_leakOil.fluidDensity(); }
   const double& sealFluidDensity() const { return m_leakGas.sealFluidDensity(); }
   const double& oilToGasLevelRatio() const { return m_oilToGasLevelRatio; }

   const MonotonicIncreasingPiecewiseLinearInvertableFunction* levelToVolume() const {
     return m_levelToVolume;
   }

   const double& maxGasLevel() const { return m_leakGas.maxLevel(); }
   const double& maxGasVolume() const { return m_leakGas.maxVolume(); }
   const Tuple2<double>& maxGasContent() const { return m_leakGas.maxContent(); }

   // FIXME
   const double& maxOilLevel() const { return m_leakOil.maxLevel(); }
   const double& maxOilVolume() const { return m_leakOil.maxVolume(); }
   const Tuple2<double>& maxOilContent() const { return m_leakOil.maxContent(); }
};

} } // namespace migration::distribute

#endif

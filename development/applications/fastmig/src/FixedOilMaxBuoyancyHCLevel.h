#ifndef _MIGRATION_FIXEDOILMAXBUOYANCYHCLEVEL_H_
#define _MIGRATION_FIXEDOILMAXBUOYANCYHCLEVEL_H_

#include "functions/src/MonotonicIncreasingPiecewiseLinearInvertableFunction.h"
#include "functions/src/Tuple2.h"

#include <assert.h>

using functions::MonotonicIncreasingPiecewiseLinearInvertableFunction;
using functions::Tuple2;

namespace migration { namespace distribute {

class FixedOilMaxBuoyancyHCLevel
{
protected:

   double m_maxGasBuoyancyLevel;
   double m_oilToGasLevelRatio;
   double m_oilVolume;

   const MonotonicIncreasingPiecewiseLinearInvertableFunction* m_levelToVolume;

   double computeGasLevel(const double& hcVolume) const
   {
      // Given fixed oil m_oilVolume, calculate the with hcVolume corresponding 
      // gas level:
      double gasVolume = hcVolume - m_oilVolume;
      double gasLevel = m_levelToVolume->invert(gasVolume);
      return gasLevel;
   }

public:

   FixedOilMaxBuoyancyHCLevel(const double& maxGasBuoyancyLevel, const double& oilToGasLevelRatio, 
      const double& oilVolume, const MonotonicIncreasingPiecewiseLinearInvertableFunction* levelToVolume):
      m_maxGasBuoyancyLevel(maxGasBuoyancyLevel),
      m_oilToGasLevelRatio(oilToGasLevelRatio),
      m_oilVolume(oilVolume),
      m_levelToVolume(levelToVolume)
   {
      // A basic assumption:
      assert(oilToGasLevelRatio < 1.0);

      // We shouldn't be here, if m_oilVolume wasn't bigger than zero:
      assert(oilVolume >= 0.0);

      // If maxGasBuoyancyLevel is zero, we shouldn't be here as well, for all gas will leak away:
      assert(maxGasBuoyancyLevel >= 0.0);
   }

   bool operator<(const Tuple2<double>& hcContent) const
   {
      // This function returns whether the buoyancy represented by hcContent combined with 
      // the with hcContent corresponding gas volume (given that the oil volume m_oilVolume 
      // is constant) is less than the buoyancy given by m_maxGasBuoyancyLevel.

      // Calculate the with oilContent corresponding gas column:
      double gasLevel = computeGasLevel(hcContent[1]);

      // If the gas level alone is bigger than m_maxGasBuoyancyLevel, the result is of course 
      // true:
      if (m_maxGasBuoyancyLevel < gasLevel)
         return true;

      // Calculate the buoyancy for the combined oil and gas column in gas units:
      double gasReprLevel = gasLevel + m_oilToGasLevelRatio * (hcContent[0] - gasLevel);

      // Make the comparison:
      return m_maxGasBuoyancyLevel < gasReprLevel;
   }

   bool operator>=(const Tuple2<double>& hcContent) const
   {
      return !operator<(hcContent);
   }
};

} } // namespace migration::distribute

#endif

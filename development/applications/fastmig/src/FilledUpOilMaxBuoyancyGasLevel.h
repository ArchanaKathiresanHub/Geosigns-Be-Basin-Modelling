#ifndef _MIGRATION_FILLEDUPOILMAXBUOYANCYGASLEVEL_H_
#define _MIGRATION_FILLEDUPOILMAXBUOYANCYGASLEVEL_H_

#include "functions/src/MonotonicIncreasingPiecewiseLinearInvertableFunction.h"
#include "functions/src/Tuple2.h"

#include <assert.h>

using functions::MonotonicIncreasingPiecewiseLinearInvertableFunction;
using functions::Tuple2;

namespace migration { namespace distribute {

class FilledUpOilMaxBuoyancyGasLevel
{
private:

   double m_maxGasBuoyancyLevel;
   double m_oilToGasLevelRatio;
   double m_capacityLevel;

   const MonotonicIncreasingPiecewiseLinearInvertableFunction* m_levelToVolume;
   
public:

   FilledUpOilMaxBuoyancyGasLevel(const double& maxGasBuoyancyLevel, const double& oilToGasLevelRatio, 
      const double& capacityLevel, const MonotonicIncreasingPiecewiseLinearInvertableFunction* levelToVolume):
      m_maxGasBuoyancyLevel(maxGasBuoyancyLevel),
      m_oilToGasLevelRatio(oilToGasLevelRatio),
      m_capacityLevel(capacityLevel),
      m_levelToVolume(levelToVolume)
   {
      // A basic assumption:
      assert(oilToGasLevelRatio < 1.0);

      // Negative or zero capacity levels are strange:
      assert(m_capacityLevel > 0.0);

      // If maxGasBuoyancyLevel is zero, we shouldn't be here as well, for all gas will leak away:
      assert(maxGasBuoyancyLevel >= 0.0);
   }

   bool operator<(const Tuple2<double>& gasContent) const
   {
      // This function returns whether the buoyancy represented by gasContent (with gasContent[0] 
      // the gasLevel and gasContent[1] the gasVolume) combined with the remaining oil volume filling 
      // up m_capacity is less than the buoyancy given by m_maxGasBuoyancyLevel.

      // If gasContent[0] alone is bigger than m_maxGasBuoyancyLevel, the result is of course true:
      if (m_maxGasBuoyancyLevel < gasContent[0])
         return true;
      
      // Calculate the buoyancy of the added gas and remaining oil volume filling up m_capacity 
      // converted into gas units:
      double gasReprLevel = gasContent[0] + m_oilToGasLevelRatio * (m_capacityLevel - gasContent[0]);

      // Make the comparison:
      return m_maxGasBuoyancyLevel < gasReprLevel;    
   }

   bool operator>=(const Tuple2<double>& gasContent) const
   {
      return !operator<(gasContent);
   }
};

} } // namespace migration::distribute

#endif

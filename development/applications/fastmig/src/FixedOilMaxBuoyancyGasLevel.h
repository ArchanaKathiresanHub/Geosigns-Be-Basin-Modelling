#ifndef _MIGRATION_DISTRIBUTE_FIXEDOILMAXBUOYANCYGASLEVEL_H_
#define _MIGRATION_DISTRIBUTE_FIXEDOILMAXBUOYANCYGASLEVEL_H_

#include "MonotonicIncreasingPiecewiseLinearInvertableFunction.h"
#include "Tuple2.h"

#include <assert.h>

using functions::MonotonicIncreasingPiecewiseLinearInvertableFunction;
using functions::Tuple2;

namespace migration { namespace distribute {

      class FixedOilMaxBuoyancyGasLevel
      {
      private:

         double m_maxGasBuoyancyLevel;
         double m_oilToGasLevelRatio;
         double m_oilVolume;

         const MonotonicIncreasingPiecewiseLinearInvertableFunction* m_levelToVolume;
   
      public:

         FixedOilMaxBuoyancyGasLevel(const double& maxGasBuoyancyLevel, const double& oilToGasLevelRatio, 
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
#ifdef WRONG
               // So what (AJH)?
               assert(maxGasBuoyancyLevel >= 0.0);
#endif
            }

            bool operator<(const Tuple2<double>& gasContent) const
            {
               // This function returns whether the buoyancy represented by gasContent (with gasContent[0] 
               // the gasLevel and gasContent[1] the gasVolume) combined with the constant oil volume 
               // m_oilVolume is less than the buoyancy given by m_maxGasBuoyancyLevel.

               // If gasContent[0] alone is bigger than m_maxGasBuoyancyLevel, the result is of course true:
               if (m_maxGasBuoyancyLevel < gasContent[0])
                  return true;
      
               // Given the fixed oil volume m_oilVolume, calculate from gasContent the HC level:
               // (Note that if the combined oil and gas volume is bigger than the capacity, m_levelToVolume 
               // will limit the result to capacity.  However, that shouldn't occur here, for this functor 
               // is only used when the added initial oil and gas volumes are within capacity.):
               double hcLevel = m_levelToVolume->invert(gasContent[1] + m_oilVolume);

               // Calculate the buoyancy for hcLevel converted to gas units:
               double gasReprLevel = gasContent[0] + m_oilToGasLevelRatio * (hcLevel - gasContent[0]);     

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

//
// Copyright (C) 2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "LeakOrWasteGasAndSpillOil.h"

#include <algorithm>
#include <assert.h>

namespace migration
{
   namespace distribute
   {

      // FIXME
      const int X = 0;
      const int Y = 1;

      LeakOrWasteGasAndSpillOil::LeakOrWasteGasAndSpillOil (const double& gasDensity, const double& oilDensity,
         const double& sealFluidDensity, const double& fracturePressure,
         const double& capPressure_H2O_Gas, const double& capPressure_H2O_Oil,
         const double& wasteLevel, const MonotonicIncreasingPiecewiseLinearInvertableFunction* levelToVolume) :
         m_leakGasAndSpillOil (gasDensity, oilDensity, sealFluidDensity, fracturePressure,
         capPressure_H2O_Gas, capPressure_H2O_Oil, levelToVolume),
         m_wasteGas (wasteLevel, levelToVolume)
      {
      }

      /// Compute the leakage of oil and gas according to the seal failure parameters.
      /// @param[in] gasVolume
      /// @param[in] oilVolume
      /// @param[out] gasVolumeLeaked
      /// @param[out] gasVolumeWasted
      /// @param[out] gasVolumeSpilled
      /// @param[out] oilVolumeLeaked
      /// @param[out] oilVolumeSpilled

      void LeakOrWasteGasAndSpillOil::distribute (const double& gasVolume, const double& oilVolume,
         double& gasVolumeLeaked, double& gasVolumeWasted, double& gasVolumeSpilled, double& oilVolumeLeaked,
         double& oilVolumeSpilled) const
      {
         gasVolumeLeaked = 0.0;
         gasVolumeSpilled = 0.0;
         gasVolumeWasted = 0.0;
         oilVolumeLeaked = 0.0;
         oilVolumeSpilled = 0.0;

         // (level, volume)
         Tuple2<double> wasteContent = m_wasteGas.maxContent ();
         assert (wasteContent[1] < m_leakGasAndSpillOil.maxCapacity ()[1]);

         // If the total volume of gas is not enough to reach to wasteContent[0], wasting doesn't
         // occur:
         if (gasVolume <= wasteContent[1])
         {
            m_leakGasAndSpillOil.distribute (gasVolume, oilVolume, gasVolumeLeaked, gasVolumeSpilled,
               oilVolumeLeaked, oilVolumeSpilled);
            return;
         }

         // But also when gasVolume exceeds wasteContent[1], dealing with the extra gas wasting level 
         // is easy.  It just restricts the maximum gas content to wasteContent[1].  
         // So call m_leakGasOrSpillOil.distribute with gasVolume restricted to wasteContent[1]:

         m_leakGasAndSpillOil.distribute (wasteContent[1], oilVolume, gasVolumeLeaked,
            gasVolumeSpilled, oilVolumeLeaked, oilVolumeSpilled);

         // If now only oil would have been spilled, the volume between gasVolume and 
         // wasteContent[1] is wasted:
         if (gasVolumeLeaked == 0.0)
         {
            gasVolumeWasted = gasVolume - wasteContent[1];
            return;
         }

         // In the other case, some gas will be leaked.  This indicates that the buoyancy 
         // of original the HC column actually exceeded the max buoyancy level.  So leaking takes 
         // precedence over wasting, and we can just return the results from m_leakGasOrSpillOil.distribute.
      }

   }
} //namespace migration::distribute

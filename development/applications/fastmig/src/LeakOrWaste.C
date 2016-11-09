//
// Copyright (C) 2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "LeakOrWaste.h"

#include <assert.h>

namespace migration
{
   namespace distribute
   {

      LeakOrWaste::LeakOrWaste (const double& fluidDensity, const double& sealFluidDensity,
         const double& maxSealPressure, const double& wasteLevel,
         const MonotonicIncreasingPiecewiseLinearInvertableFunction* levelToVolume) :
         m_leak (fluidDensity, sealFluidDensity, maxSealPressure, levelToVolume),
         m_waste (wasteLevel, levelToVolume)
      {
         m_leaking = m_leak.maxLevel () < m_waste.maxLevel ();
      }

      /// Compute the leakage of oil and gas according to the seal failure parameters.
      /// @param[in] fluidVolume
      /// @param[out] fluidVolumeLeaked
      /// @param[out] fluidVolumeSpilled

      void LeakOrWaste::distribute (const double& fluidVolume, double& fluidVolumeLeaked,
         double& fluidVolumeWasted) const
      {
         fluidVolumeLeaked = 0.0;
         fluidVolumeWasted = 0.0;

         // The following two cases apply:
         //
         // a) The buoyancy is larger tham m_waste.maxLevel().
         // b) The buoyancy is equal to or less than the capacity of the trap.
         //
         // In case a) the fluid will be wasted, in case b) leaked.

         if (m_leaking)
            m_leak.distribute (fluidVolume, fluidVolumeLeaked);
         else
            m_waste.distribute (fluidVolume, fluidVolumeWasted);
      }

   }
} //namespace migration::distribute

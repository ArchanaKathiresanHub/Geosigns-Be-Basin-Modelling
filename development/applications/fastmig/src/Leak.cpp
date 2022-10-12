//
// Copyright (C) 2016-2017 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "Leak.h"
#include "Interface.h"

// std library
#include <iostream>
#include <limits>
#include <assert.h>
#include <algorithm>
using std::numeric_limits;
using std::min;

#include "ConstantsPhysics.h"
using Utilities::Physics::AccelerationDueToGravity;

namespace migration
{
   namespace distribute
   {

      Leak::Leak (const double fluidDensity, const double sealFluidDensity, const double overPressureContrast, const double crestColumnThickness,
                  const double maxSealPressure, const MonotonicIncreasingPiecewiseLinearInvertableFunction* levelToVolume) :
         m_fluidDensity (fluidDensity),
         m_sealFluidDensity (sealFluidDensity),
         m_maxSealPressure (maxSealPressure)
      {
         assert (sealFluidDensity > 0.0);
         assert (fluidDensity > 0.0);

         // Normally the following conditions should apply:
         // assert(sealFluidDensity > fluidDensity);
         // assert(fluidDensity > 0.0);
         // However, in the first real test of leaking (Aquifer), sealFluidDensity was smaller (1073 kg/m3) 
         // than oil density (1003 kg/m3). So we must be a bit careful. If the density of oil is bigger 
         // than the one from oil or gas, we define the densities such that leaking won't take place.
         // The easiest way of doing that is by limiting the density of oil and/or gas to that of 
         // the sealFluidDensity:
         m_fluidDensity = std::min (m_sealFluidDensity, m_fluidDensity);

         double buoyancyForce = (m_sealFluidDensity - m_fluidDensity) * AccelerationDueToGravity;

         double& maxBuoyancyLevel = m_maxBuoyancy[0];
         // Infinite column height if no buoyancy. Consistent with previour implementation ignoring overpressures
         if (buoyancyForce == 0.0)
            maxBuoyancyLevel = numeric_limits<double>::max ();
         // Don't account for overpressures if crest column is zero thickness
         else if (crestColumnThickness == 0.0)
            maxBuoyancyLevel = m_maxSealPressure / buoyancyForce;
         // If both buoyancy and thickness are not zero, account for overpressure
         else
            maxBuoyancyLevel = m_maxSealPressure / (buoyancyForce + overPressureContrast / crestColumnThickness);

         // Check if the denominator of the above equation is smaller than, or euqal to, zero
         if (maxBuoyancyLevel < 0 or !std::isfinite(maxBuoyancyLevel))
         {
            maxBuoyancyLevel = numeric_limits<double>::max ();
         }

         // Calculate the corresponding volume:
         double& maxBuoyancyVolume = m_maxBuoyancy[1];
         maxBuoyancyVolume = levelToVolume->apply (maxBuoyancyLevel);

#ifdef DEBUG_LEAK
         // Calculate the maximum capacity of the trap:
         m_capacity[0] = levelToVolume->invert(numeric_limits<double>::max());
         m_capacity[1] = levelToVolume->apply(numeric_limits<double>::max());
#endif
      }

      /// Compute the leakage of oil and gas according to the seal failure parameters.
      /// @param[in] fluidVolume
      /// @param[out] fluidVolumeLeaked
      void Leak::distribute (const double& fluidVolume, double& fluidVolumeLeaked) const
      {
#ifdef DEBUG_LEAK
         // fluidVolume or maxVolume() must be smaller than capacity, or else spilling would have occurred 
         // instead of leaking:
         assert(fluidVolume < m_capacity[1] || maxVolume() < m_capacity[1]);
#endif

         // Everything beyond maxBuoyancyVolume() is leaked:
         if (fluidVolume > maxVolume ())
            fluidVolumeLeaked = fluidVolume - maxVolume ();
         else
            fluidVolumeLeaked = 0.0;
      }

   }
} // namespace migration::distribute

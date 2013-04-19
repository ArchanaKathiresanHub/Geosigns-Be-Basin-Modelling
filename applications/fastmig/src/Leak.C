#include "Leak.h"
#include "CBMGenerics/src/consts.h"

#include <iostream>
#include <limits>
#include <assert.h>
#include <algorithm>

using std::numeric_limits;
using std::min;
using CBMGenerics::Gravity;

namespace migration { namespace distribute {

Leak::Leak(const double& fluidDensity, const double& sealFluidDensity, const double& maxSealPressure,
   const MonotonicIncreasingPiecewiseLinearInvertableFunction* levelToVolume):
   m_fluidDensity(fluidDensity),
   m_sealFluidDensity(sealFluidDensity),
   m_maxSealPressure(maxSealPressure)
{
   assert(sealFluidDensity > 0.0);
   assert(fluidDensity > 0.0);

   // Normally the following conditions should apply:
   // assert(sealFluidDensity > fluidDensity);
   // assert(fluidDensity > 0.0);
   // However, in the first real test of leaking (Acquifer), sealFluidDensity was smaller (1073 kg/m3) 
   // than oil density (1003 kg/m3).  So we must be a bit careful.  If the density of oil is bigger 
   // than the one from oil or gas, we define the densities such that leaking won't take place.
   // The easiest way of doing that is by limit the density of oil and or gas equal to that of 
   // the sealFluidDensity:
   m_fluidDensity = min(m_sealFluidDensity, m_fluidDensity);

   // Calculate the maximum buoyancy level:
   double& maxBuoyancyLevel = m_maxBuoyancy[0];
   maxBuoyancyLevel = m_sealFluidDensity != m_fluidDensity ? 
      m_maxSealPressure / ((m_sealFluidDensity - m_fluidDensity) * Gravity) :
      numeric_limits<double>::max();

   if (maxBuoyancyLevel < 0)
   {
      std::cerr << "m_maxSealPressure = " << m_maxSealPressure << std::endl;
      std::cerr << "sealFluidDensity = " << m_sealFluidDensity << std::endl;
      std::cerr << "fluidDensity = " << m_fluidDensity << std::endl;
      std::cerr << "maxBuoyancyLevel = " << maxBuoyancyLevel << std::endl;
   }

   assert (maxBuoyancyLevel >= 0);

   

   // Calculate the corresponding volume:
   double& maxBuoyancyVolume = m_maxBuoyancy[1];
   maxBuoyancyVolume = levelToVolume->apply(maxBuoyancyLevel);

#ifdef DEBUG_LEAK
   // Calculate the maximum capacity of the trap:
   m_capacity[0] = levelToVolume->invert(numeric_limits<double>::max());
   m_capacity[1] = levelToVolume->apply(numeric_limits<double>::max());
#endif
}

/// Compute the leakage of oil and gas according to the seal failure parameters.
/// @param[in] fluidVolume
/// @param[out] fluidVolumeLeaked
void Leak::distribute(const double& fluidVolume, double& fluidVolumeLeaked) const
{
#ifdef DEBUG_LEAK
   // fluidVolume or maxVolume() must be smaller than capacity, or else spilling would have occurred 
   // instead of leaking:
   assert(fluidVolume < m_capacity[1] || maxVolume() < m_capacity[1]);
#endif

   // Everything beyond maxBuoyancyVolume() is leaked:
   if (fluidVolume > maxVolume())
      fluidVolumeLeaked = fluidVolume - maxVolume();
   else
      fluidVolumeLeaked = 0.0;
}

} } // namespace migration::distribute

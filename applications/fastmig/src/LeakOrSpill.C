#include "LeakOrSpill.h"

#include <assert.h>

namespace migration { namespace distribute {

LeakOrSpill::LeakOrSpill(const double& fluidDensity, const double& sealFluidDensity, 
   const double& maxSealPressure, 
   const MonotonicIncreasingPiecewiseLinearInvertableFunction* levelToVolume):
   m_leak(fluidDensity, sealFluidDensity, maxSealPressure, levelToVolume),
   m_spill(levelToVolume)
{
   m_leaking = m_leak.maxLevel() < m_spill.maxLevel();
}

/// Compute the leakage of oil and gas according to the seal failure parameters.
/// @param[in] fluidVolume
/// @param[out] fluidVolumeLeaked
/// @param[out] fluidVolumeSpilled

void LeakOrSpill::distribute(const double& fluidVolume, double& fluidVolumeLeaked, 
   double& fluidVolumeSpilled) const
{
   fluidVolumeLeaked = 0.0;
   fluidVolumeSpilled = 0.0;

   // The following two cases apply:
   //
   // a) The buoyancy is equal to the capacity of the trap.
   // (Conceptually, one could think of this situation as if the buoyancy equals or 
   // exceeds the capacity of the trap.  However, there is no capacity outside the trap.  
   // So if we would calculate the maximum fluid column for a buoyancy which conceptually 
   // exceeds the capacity of the trap, we will find it will equal the capacity.)
   // b) The buoyancy is less than the capacity of the trap.
   //
   // In case a) the fluid will be spilled, in case b) leaked.

   if (m_leaking)
      m_leak.distribute(fluidVolume, fluidVolumeLeaked);
   else
      m_spill.distribute(fluidVolume, fluidVolumeSpilled);
}

} } //namespace migration::distribute

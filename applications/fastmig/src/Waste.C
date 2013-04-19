#include "Waste.h"

namespace migration { namespace distribute {

Waste::Waste(const double& wasteLevel, const MonotonicIncreasingPiecewiseLinearInvertableFunction* 
   levelToVolume)
{
   // Calculate the maximum capacity of the trap:
   m_maxContent[0] = wasteLevel;
   m_maxContent[1] = levelToVolume->apply(wasteLevel);
}

void Waste::distribute(const double& fluidVolume, double& fluidVolumeWasted) const
{
   // The volume beyond m_maxContent[1] is spilled:
   if (fluidVolume > m_maxContent[1])
      fluidVolumeWasted = fluidVolume - m_maxContent[1];
   else
      fluidVolumeWasted = 0.0;
}

} } // namespace migration::distribute

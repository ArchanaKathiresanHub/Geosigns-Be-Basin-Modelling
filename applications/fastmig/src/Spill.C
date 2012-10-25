#include "Spill.h"

#include <limits>

using std::numeric_limits;

namespace migration { namespace distribute {

Spill::Spill(const MonotonicIncreasingPiecewiseLinearInvertableFunction* levelToVolume)
{
   // Calculate the maximum capacity of the trap:
   m_capacity[0] = levelToVolume->invert(numeric_limits<double>::max());
   m_capacity[1] = levelToVolume->apply(numeric_limits<double>::max());
}

void Spill::distribute(const double& fluidVolume, double& fluidVolumeSpilled) const
{
   // Spillage is easy.  The volume beyond m_capacity is spilled:
   if (fluidVolume > m_capacity[1])
      fluidVolumeSpilled = fluidVolume - m_capacity[1];
   else
      fluidVolumeSpilled = 0.0;
}

} } // namespace migration::distribute

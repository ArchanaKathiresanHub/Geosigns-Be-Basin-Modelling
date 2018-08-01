//
// Copyright (C) 2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "Spill.h"

#include <limits>

using std::numeric_limits;

namespace migration
{
   namespace distribute
   {

      Spill::Spill (const MonotonicIncreasingPiecewiseLinearInvertableFunction* levelToVolume)
      {
         // Calculate the maximum capacity of the trap:
         m_capacity[0] = levelToVolume->invert (numeric_limits<double>::max ());
         m_capacity[1] = levelToVolume->apply (numeric_limits<double>::max ());
      }

      void Spill::distribute (const double& fluidVolume, double& fluidVolumeSpilled) const
      {
         // Spillage is easy.  The volume beyond m_capacity is spilled:
         if (fluidVolume > m_capacity[1])
            fluidVolumeSpilled = fluidVolume - m_capacity[1];
         else
            fluidVolumeSpilled = 0.0;
      }

   }
} // namespace migration::distribute

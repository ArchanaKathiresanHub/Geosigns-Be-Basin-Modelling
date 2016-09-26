//
// Copyright (C) 2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "SpillOilAndGas.h"

#include <limits>

using std::numeric_limits;

namespace migration
{
   namespace distribute
   {

      SpillOilAndGas::SpillOilAndGas (const MonotonicIncreasingPiecewiseLinearInvertableFunction* levelToVolume) :
         m_spill (levelToVolume)
      {
      }

      void SpillOilAndGas::distribute (const double& gasVolume, const double& oilVolume, double& gasVolumeSpilled,
         double& oilVolumeSpilled) const
      {
         // Compute the spilled HC's:
         double fluidSpilled;
         m_spill.distribute (gasVolume + oilVolume, fluidSpilled);

         // Distribute the spilled fluid over oil and gas.  Because oil has always a lower 
         // density than gas, oil will always be spilled before gas.  Note that in the remainder 
         // of the method, we could also have found out that only spilling would take place. 
         // But now we are in the position to also spill gas in case the gas volume would 
         // exceed capacity[1]:
         if (fluidSpilled > oilVolume)
         {
            oilVolumeSpilled = oilVolume;
            gasVolumeSpilled = fluidSpilled - oilVolume;
         }
         else
            oilVolumeSpilled = fluidSpilled;
      }

   }
} // namespace migration::distribute

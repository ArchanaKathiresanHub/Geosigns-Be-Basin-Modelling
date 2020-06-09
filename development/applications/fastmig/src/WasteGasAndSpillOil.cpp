//
// Copyright (C) 2010-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "WasteGasAndSpillOil.h"

#include <algorithm>
#include <assert.h>

namespace migration
{
   namespace distribute
   {

      // FIXME
      const int X = 0;
      const int Y = 1;

      WasteGasAndSpillOil::WasteGasAndSpillOil (const double& wasteLevel,
         const MonotonicIncreasingPiecewiseLinearInvertableFunction* levelToVolume) :
         m_spillOil (levelToVolume),
         m_wasteGas (wasteLevel, levelToVolume)
      {
      }

      /// Compute the leakage of oil and gas according to the seal failure parameters.
      /// @param[in] gasVolume
      /// @param[in] oilVolume
      /// @param[out] gasVolumeWasted
      /// @param[out] oilVolumeSpilled

      void WasteGasAndSpillOil::distribute (const double& gasVolume, const double& oilVolume,
         double& gasVolumeWasted, double& oilVolumeSpilled) const
      {
         gasVolumeWasted = 0.0;
         oilVolumeSpilled = 0.0;

         m_wasteGas.distribute (gasVolume, gasVolumeWasted);
         m_spillOil.distribute (gasVolume - gasVolumeWasted + oilVolume, oilVolumeSpilled);
      }

   }
} //namespace migration::distribute

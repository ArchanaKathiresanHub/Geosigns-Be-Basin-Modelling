//
// Copyright (C) 2010-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _MIGRATION_DISTRIBUTE_WASTEGASANDSPILLOIL_H_
#define _MIGRATION_DISTRIBUTE_WASTEGASANDSPILLOIL_H_

#include "Spill.h"
#include "Waste.h"

#include "MonotonicIncreasingPiecewiseLinearInvertableFunction.h"
#include "Tuple2.h"

using functions::MonotonicIncreasingPiecewiseLinearInvertableFunction;
using functions::Tuple2;

namespace migration
{
   namespace distribute
   {

      class WasteGasAndSpillOil
      {
      private:

         Spill m_spillOil;
         Waste m_wasteGas;

      public:

         WasteGasAndSpillOil (const double& wasteLevel,
            const MonotonicIncreasingPiecewiseLinearInvertableFunction* levelToVolume);

         void distribute (const double& gasVolume, const double& oilVolume,
            double& gasVolumeWasted, double& oilVolumeSpilled) const;
      };

   }
} // namespace migration::distribute

#endif

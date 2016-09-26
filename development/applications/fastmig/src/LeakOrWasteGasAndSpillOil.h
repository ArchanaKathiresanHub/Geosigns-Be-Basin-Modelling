//
// Copyright (C) 2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _MIGRATION_DISTRIBUTE_LEAKORWASTEGASANDSPILLOIL_H_
#define _MIGRATION_DISTRIBUTE_LEAKORWASTEGASANDSPILLOIL_H_

#include "LeakGasAndSpillOil.h"
#include "Waste.h"

#include "MonotonicIncreasingPiecewiseLinearInvertableFunction.h"
#include "Tuple2.h"

using functions::MonotonicIncreasingPiecewiseLinearInvertableFunction;
using functions::Tuple2;

namespace migration
{
   namespace distribute
   {

      class LeakOrWasteGasAndSpillOil
      {
      private:

         LeakGasAndSpillOil m_leakGasAndSpillOil;
         Waste m_wasteGas;

      public:

         LeakOrWasteGasAndSpillOil (const double& gasDensity, const double& oilDensity, const double& sealFluidDensity,
            const double& fracturePressure, const double& capPressure_H2O_Gas, const double& capPressure_H2O_Oil,
            const double& wasteLevel,
            const MonotonicIncreasingPiecewiseLinearInvertableFunction* levelToVolume);

         void distribute (const double& gasVolume, const double& oilVolume, double& gasVolumeLeaked,
            double& gasVolumeWasted, double& gasVolumeSpilled, double& oilVolumeLeaked,
            double& oilVolumeSpilled) const;
      };

   }
} // namespace migration::distribute

#endif

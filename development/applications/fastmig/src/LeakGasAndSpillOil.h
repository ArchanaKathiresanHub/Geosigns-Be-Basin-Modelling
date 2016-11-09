//
// Copyright (C) 2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _MIGRATION_DISTRIBUTE_LEAKGASANDSPILLOIL_H_
#define _MIGRATION_DISTRIBUTE_LEAKGASANDSPILLOIL_H_

#include "LeakGas.h"
#include "SpillOilAndGas.h"
// #include "LeakOrSpill.h"

#include "MonotonicIncreasingPiecewiseLinearInvertableFunction.h"
#include "Tuple2.h"

using functions::MonotonicIncreasingPiecewiseLinearInvertableFunction;
using functions::Tuple2;

namespace migration
{
   namespace distribute
   {
      class LeakGasAndSpillOil
      {
      private:

         LeakGas m_leakGas;
         SpillOilAndGas m_spillOilAndGas;

         double computeFinalGasVolume (const double maxGasBuoyancyLevel, const Tuple2<Tuple2<double> >& gasLimits) const;

      public:

         LeakGasAndSpillOil (const double& gasDensity, const double& oilDensity, const double& sealFluidDensity,
            const double& fracturePressure, const double& capPressure_H2O_Gas, const double& capPressure_H2O_Oil,
            const MonotonicIncreasingPiecewiseLinearInvertableFunction* levelToVolume);

         void distribute (const double& gasVolume, const double& oilVolume, double& gasVolumeLeaked,
            double& gasVolumeSpilled, double& oilVolumeLeaked, double& oilVolumeSpilled) const;

         const double& maxCapacityLevel () const { return m_spillOilAndGas.maxLevel (); }
         const double& maxCapacityVolume () const { return m_spillOilAndGas.maxVolume (); }
         const Tuple2<double>& maxCapacity () const { return m_spillOilAndGas.maxContent (); }
      };
   }
} // namespace migration::distribute

#endif

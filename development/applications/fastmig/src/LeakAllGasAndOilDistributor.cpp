//
// Copyright (C) 2016-2017 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "LeakAllGasAndOilDistributor.h"

namespace migration
{

   LeakAllGasAndOilDistributor::LeakAllGasAndOilDistributor (const MonotonicIncreasingPiecewiseLinearInvertableFunction*
      levelToVolume) :
      m_levelToVolume (levelToVolume)
   {
   }

   void LeakAllGasAndOilDistributor::distribute (const Composition& gas, const Composition& oil, const double& T_K,
                                                 Composition& remainingGas, Composition& remainingOil, Composition& leakedGas,
                                                 Composition& wastedGas, Composition& spilledGas, Composition& leakedOil, Composition& spilledOil,
                                                 double& finalGasLevel, double& finalHCLevel, const double brinePressure, const bool performAdvancedMigration) const
   {
      // Copy the volumes to the output Compositions:
      remainingGas = gas; leakedGas = gas; wastedGas = gas; spilledGas = gas;
      remainingOil = oil; leakedOil = oil; spilledOil = oil;

      leakedGas.setVolume (gas.getVolume ());
      wastedGas.setVolume (0.0);
      spilledGas.setVolume (0.0);
      remainingGas.setVolume (0.0);

      leakedOil.setVolume (oil.getVolume ());
      spilledOil.setVolume (0.0);
      remainingOil.setVolume (0.0);

      finalGasLevel = 0.0;
      finalHCLevel = 0.0;
   }

} // namespace migration

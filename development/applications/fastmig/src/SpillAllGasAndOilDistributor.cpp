//
// Copyright (C) 2016-2017 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "SpillAllGasAndOilDistributor.h"

namespace migration
{

   SpillAllGasAndOilDistributor::SpillAllGasAndOilDistributor (const MonotonicIncreasingPiecewiseLinearInvertableFunction*
      levelToVolume) :
      m_levelToVolume (levelToVolume)
   {
   }

   void SpillAllGasAndOilDistributor::distribute (const Composition& gas, const Composition& oil, const double& T_K,
                                                  Composition& remainingGas, Composition& remainingOil, Composition& leakedGas,
                                                  Composition& wastedGas, Composition& spilledGas, Composition& leakedOil, Composition& spilledOil,
                                                  double& finalGasLevel, double& finalHCLevel, const double brinePressure) const
   {
      // Copy the volumes to the output Compositions:
      remainingGas = gas; leakedGas = gas; wastedGas = gas; spilledGas = gas;
      remainingOil = oil; leakedOil = oil; spilledOil = oil;

      leakedGas.setVolume (0.0);
      wastedGas.setVolume (0.0);
      spilledGas.setVolume (gas.getVolume ());
      remainingGas.setVolume (0.0);

      leakedOil.setVolume (0.0);
      spilledOil.setVolume (oil.getVolume ());
      remainingOil.setVolume (0.0);

      finalGasLevel = 0.0;
      finalHCLevel = 0.0;
   }

} // namespace migration

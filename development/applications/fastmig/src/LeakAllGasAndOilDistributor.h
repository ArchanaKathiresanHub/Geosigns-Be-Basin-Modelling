//
// Copyright (C) 2016-2017 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _MIGRATION_DISTRIBUTE_LEAKALLGASANDOILDISTRIBUTOR_H_
#define _MIGRATION_DISTRIBUTE_LEAKALLGASANDOILDISTRIBUTOR_H_

#include "Distributor.h"

#include "Composition.h"

#include "MonotonicIncreasingPiecewiseLinearInvertableFunction.h"

#include <limits>

using functions::MonotonicIncreasingPiecewiseLinearInvertableFunction;

using std::numeric_limits;

namespace migration
{
   class LeakAllGasAndOilDistributor : public Distributor
   {
   private:

      const MonotonicIncreasingPiecewiseLinearInvertableFunction* m_levelToVolume;

   public:

      LeakAllGasAndOilDistributor (const MonotonicIncreasingPiecewiseLinearInvertableFunction* levelToVolume);

      void distribute (const Composition& gas, const Composition& oil, const double& T_K,
                       Composition& remainingGas, Composition& remainingOil, Composition& leakedGas,
                       Composition& wastedGas, Composition& spilledGas, Composition& leakedOil, Composition& spilledOil,
                       double& finalGasLevel, double& finalHCLevel, const double brinePressure, const bool performAdvancedMigration) const override;

      bool leaking () const { return true; }
      bool wasting () const { return false; }

      double sealFluidDensity () const { return 0.0; }
      double fracturePressure () const { return 0.0; }
      double wasteLevel () const { return numeric_limits<double>::max (); }

      const MonotonicIncreasingPiecewiseLinearInvertableFunction* levelToVolume () const { return m_levelToVolume; }

      void setLeaking (bool leaking) {}
      void setWasting (bool wasting) {}

      void setWasteLevel (const double& wasteLevel) {}
      void setLevelToVolume (const MonotonicIncreasingPiecewiseLinearInvertableFunction* levelToVolume)
      {
         m_levelToVolume = levelToVolume;
      }
   };

} // namespace migration

#endif

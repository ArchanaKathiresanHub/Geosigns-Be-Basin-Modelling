//
// Copyright (C) 2016-2017 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _MIGRATION_DISTRIBUTE_DISTRIBUTOR_H_
#define _MIGRATION_DISTRIBUTE_DISTRIBUTOR_H_

#include "Composition.h"

#include "MonotonicIncreasingPiecewiseLinearInvertableFunction.h"

#include <limits>

using functions::MonotonicIncreasingPiecewiseLinearInvertableFunction;

using std::numeric_limits;

namespace migration { 

   class Distributor
   {
   public:

      virtual ~Distributor() {}

      virtual void distribute(const Composition& gas, const Composition& oil, const double& T_K, 
                              Composition& remainingGas, Composition& remainingOil, 
                              Composition& leakedGas, Composition& wastedGas, Composition& spilledGas, 
                              Composition& leakedOil, Composition& spilledOil,
                              double& finalGasLevel, double& finalHCLevel,
                              const double brinePressure, const bool performAdvancedMigration) const = 0;

      virtual bool leaking() const = 0;
      virtual bool wasting() const = 0;

      virtual const MonotonicIncreasingPiecewiseLinearInvertableFunction* levelToVolume() const = 0;

      virtual void setLeaking(bool leaking) = 0;
      virtual void setWasting(bool wasting) = 0;

      virtual void setWasteLevel(const double& wasteLevel) = 0;
      virtual void setLevelToVolume(const MonotonicIncreasingPiecewiseLinearInvertableFunction* levelToVolume) = 0;
   };

} // namespace migration

#endif

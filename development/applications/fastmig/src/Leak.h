//
// Copyright (C) 2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _MIGRATION_DISTRIBUTE_LEAK_H_
#define _MIGRATION_DISTRIBUTE_LEAK_H_

#include "MonotonicIncreasingPiecewiseLinearInvertableFunction.h"
#include "Tuple2.h"

using functions::MonotonicIncreasingPiecewiseLinearInvertableFunction;
using functions::Tuple2;

namespace migration
{
   namespace distribute
   {

      class Leak
      {
      private:

         double m_fluidDensity;
         double m_sealFluidDensity;
         double m_maxSealPressure;

         Tuple2<double> m_maxBuoyancy;

#ifdef DEBUG_LEAK
         Tuple2<double> m_capacity;
#endif

      public:

         Leak (const double& fluidDensity, const double& sealFluidDensity, const double& maxSealPressure,
            const MonotonicIncreasingPiecewiseLinearInvertableFunction* levelToVolume);

         void distribute (const double& fluidVolume, double& fluidVolumeLeaked) const;

         double& getFluidDensity () { return m_fluidDensity; }

         const double& fluidDensity () const { return m_fluidDensity; }
         const double& sealFluidDensity () const { return m_sealFluidDensity; }
         const double& maxSealPressure () const { return m_maxSealPressure; }

         const double& maxLevel () const { return m_maxBuoyancy[0]; }
         const double& maxVolume () const { return m_maxBuoyancy[1]; }
         const Tuple2<double>& maxContent () const { return m_maxBuoyancy; }
      };

   }
} // namespace migration::distribute

#endif

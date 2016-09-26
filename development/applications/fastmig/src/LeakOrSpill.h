//
// Copyright (C) 2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _MIGRATION_DISTRIBUTE_LEAKORSPILL_H_
#define _MIGRATION_DISTRIBUTE_LEAKORSPILL_H_

#include "Leak.h"
#include "Spill.h"

#include "MonotonicIncreasingPiecewiseLinearInvertableFunction.h"
#include "Tuple2.h"

using functions::MonotonicIncreasingPiecewiseLinearInvertableFunction;
using functions::Tuple2;

namespace migration
{
   namespace distribute
   {

      class LeakOrSpill
      {
      private:

         Leak m_leak;
         Spill m_spill;

         bool m_leaking;

      public:

         LeakOrSpill (const double& fluidDensity, const double& sealFluidDensity,
            const double& maxSealPressure,
            const MonotonicIncreasingPiecewiseLinearInvertableFunction* levelToVolume);

         void distribute (const double& fluidVolume, double& fluidVolumeLeaked,
            double& fluidVolumeSpilled) const;

         const double& maxLevel () const
         {
            return m_leaking ? m_leak.maxLevel () : m_spill.maxLevel ();
         }
         const double& maxVolume () const
         {
            return m_leaking ? m_leak.maxVolume () : m_spill.maxVolume ();
         }
         const Tuple2<double>& maxContent () const
         {
            return m_leaking ? m_leak.maxContent () : m_spill.maxContent ();
         }
         bool leaking () const { return m_leaking; }
      };

   }
} // namespace migration::distribute

#endif

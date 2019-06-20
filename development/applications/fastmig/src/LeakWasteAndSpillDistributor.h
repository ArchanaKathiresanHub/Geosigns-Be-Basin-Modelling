//
// Copyright (C) 2016-2017 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _MIGRATION_DISTRIBUTE_LEAKWASTEANDSPILLDISTRIBUTOR_H_
#define _MIGRATION_DISTRIBUTE_LEAKWASTEANDSPILLDISTRIBUTOR_H_

#include <boost/array.hpp>

#include "Distributor.h"
#include "MigrationCapillarySealStrength.h"

#include "Composition.h"
#include "MonotonicIncreasingPiecewiseLinearInvertableFunction.h"

using functions::MonotonicIncreasingPiecewiseLinearInvertableFunction;

namespace migration
{

   class LeakWasteAndSpillDistributor : public Distributor
   {
   private:

      bool m_leaking;
      bool m_wasting;

      double m_sealFluidDensity;
      double m_fractureSealStrength;
      double m_wasteLevel;
      double m_overPressureContrast;
      double m_crestColumnThickness;

      MigrationCapillarySealStrength m_capSealStrength;

      const MonotonicIncreasingPiecewiseLinearInvertableFunction* m_levelToVolume;

      // Sometimes the levelToVolume function given to LeakWasteAndSpillDistributor doesn't 
      // start at depth zero as it normally would, but at the depth below zero.  This happens
      // when the crest column (and possibly some adjacent columns) have zero capacity. 
      // Normally this is caused by a zero porosity.  But whatever the cause, LeakGas and other
      // distribution implementations rely on m_levelToVolume->piece(0) == tuple(0.0,0.0).
      // So in case m_levelToVolume->piece(0)[0] isn't 0.0, we replace m_levelToVolume with 
      // our own version where all the depths are shifted:
      double m_shift;

   public:

      LeakWasteAndSpillDistributor (const double& sealFluidDensity, const double& fractureSealStrength,
                                    const double& wasteLevel, const MigrationCapillarySealStrength& capSealStrength,
                                    const MonotonicIncreasingPiecewiseLinearInvertableFunction* levelToVolume);

      LeakWasteAndSpillDistributor (const double sealFluidDensity, const double fractureSealStrength, const double overPressureContrast,
                                    const double crestColumnThickness, const MigrationCapillarySealStrength& capSealStrength,
                                    const MonotonicIncreasingPiecewiseLinearInvertableFunction* levelToVolume);

      ~LeakWasteAndSpillDistributor ();

      void shiftToOrigin ();

      void distribute (const Composition& gas, const Composition& oil, const double& T_K,
                       Composition& remainingGas, Composition& remainingOil, Composition& leakedGas,
                       Composition& wastedGas, Composition& spilledGas, Composition& leakedOil, Composition& spilledOil,
                       double& finalGasLevel, double& finalHCLevel, const double brinePressure) const;

      bool leaking () const { return m_leaking; }
      bool wasting () const { return m_wasting; }

      double sealFluidDensity () const { return m_sealFluidDensity; }
      double fractureSealStrength () const { return m_fractureSealStrength; }
      const MigrationCapillarySealStrength& capillarySealStrength () const { return m_capSealStrength; }
      double wasteLevel () const { return m_wasteLevel; }

      const MonotonicIncreasingPiecewiseLinearInvertableFunction* levelToVolume () const { return m_levelToVolume; }

      void setLeaking (bool leaking) { m_leaking = leaking; }
      void setWasting (bool wasting) { m_wasting = wasting; }

      void setSealFluidDensity (const double& sealFluidDensity) { m_sealFluidDensity = sealFluidDensity; }
      void setFractureSealStrength (const double& fractureSealStrength) { m_fractureSealStrength = fractureSealStrength; }
      void setWasteLevel (const double& wasteLevel) { m_wasteLevel = wasteLevel; }

      void setLevelToVolume (const MonotonicIncreasingPiecewiseLinearInvertableFunction* levelToVolume);
   };

} // namespace migration

#endif

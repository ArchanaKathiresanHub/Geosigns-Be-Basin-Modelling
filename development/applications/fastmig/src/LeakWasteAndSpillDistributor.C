//
// Copyright (C) 2016-2017 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "LeakWasteAndSpillDistributor.h"

#include "Leak.h"
#include "Waste.h"
#include "Spill.h"
#include "SpillOilAndGas.h"
#include "LeakOrWaste.h"
#include "LeakOrSpill.h"
#include "WasteGasAndSpillOil.h"
#include "LeakGasAndSpillOil.h"
#include "LeakOrWasteGasAndSpillOil.h"
#include "migration.h"

using migration::distribute::Leak;
using migration::distribute::Waste;
using migration::distribute::Spill;
using migration::distribute::SpillOilAndGas;
using migration::distribute::LeakOrWaste;
using migration::distribute::LeakOrSpill;
using migration::distribute::WasteGasAndSpillOil;
using migration::distribute::LeakGasAndSpillOil;
using migration::distribute::LeakOrWasteGasAndSpillOil;

namespace migration
{

   LeakWasteAndSpillDistributor::LeakWasteAndSpillDistributor (const double& sealFluidDensity,
      const double& fractureSealStrength, const double& wasteLevel, const CapillarySealStrength& capSealStrength,
      const MonotonicIncreasingPiecewiseLinearInvertableFunction* levelToVolume) :
      m_leaking (true),
      m_wasting (true),
      m_sealFluidDensity (sealFluidDensity),
      m_fractureSealStrength (fractureSealStrength),
      m_wasteLevel (wasteLevel),
      m_capSealStrength (capSealStrength),
      m_levelToVolume (levelToVolume),
      m_shift (0.0)
   {
      shiftToOrigin ();
   }

   LeakWasteAndSpillDistributor::LeakWasteAndSpillDistributor (const double& sealFluidDensity,
      const double& fractureSealStrength, const CapillarySealStrength& capSealStrength,
      const MonotonicIncreasingPiecewiseLinearInvertableFunction* levelToVolume) :
      m_leaking (true),
      m_wasting (false),
      m_sealFluidDensity (sealFluidDensity),
      m_fractureSealStrength (fractureSealStrength),
      m_wasteLevel (numeric_limits<double>::max ()),
      m_capSealStrength (capSealStrength),
      m_levelToVolume (levelToVolume),
      m_shift (0.0)
   {
      shiftToOrigin ();
   }

   void LeakWasteAndSpillDistributor::shiftToOrigin ()
   {
      if (!m_levelToVolume) return;

      // Of course the level to volume function must start at 0:
      assert (m_levelToVolume->begin (0)[1] == 0.0);

      // Sometimes the levelToVolume function given to LeakWasteAndSpillDistributor doesn't 
      // start at depth zero as it normally would, but at the depth below zero.  This happens
      // when the crest column (and possibly some adjacent columns) have zero capacity. 
      // Normally this is caused by a zero porosity.  But whatever the cause, LeakGas and other
      // distribution implementations rely on m_levelToVolume->begin(0)[0] == tuple(0.0,0.0).
      // So in case m_levelToVolume->begin(0)[0] isn't 0.0, we replace m_levelToVolume with 
      // our own version where all the depths are shifted:
      if (m_levelToVolume->begin (0)[0] != 0.0)
      {
         m_shift = m_levelToVolume->begin (0)[0];

         MonotonicIncreasingPiecewiseLinearInvertableFunction * shiftedLevelToVolume = new MonotonicIncreasingPiecewiseLinearInvertableFunction (*m_levelToVolume);
         shiftedLevelToVolume->shiftXBy (-m_shift);
         m_levelToVolume = shiftedLevelToVolume;

         assert (m_levelToVolume->begin (0)[0] == 0.0);
      }
   }

   LeakWasteAndSpillDistributor::~LeakWasteAndSpillDistributor ()
   {
      setLevelToVolume (0);
   }

   void LeakWasteAndSpillDistributor::distribute (const Composition& gas, const Composition& oil, const double& T_K,
                                                  Composition& remainingGas, Composition& remainingOil, Composition& leakedGas,
                                                  Composition& wastedGas, Composition& spilledGas, Composition& leakedOil, Composition& spilledOil,
                                                  double& finalGasLevel, double& finalHCLevel, const double brinePressure) const
   {
      assert (gas.getWeight () >= 0.0);
      assert (oil.getWeight () >= 0.0);

      assert (m_levelToVolume);

      double gorm = computeGorm (gas, oil);

      double gasVolume = gas.getVolume ();
      double oilVolume = oil.getVolume ();

      double capSealStrength_H2O_Gas = 0.0;
      double capSealStrength_H2O_Oil = 0.0;

      std::vector<Composition>  trapComposition(2);
      trapComposition[0] = gas;
      trapComposition[1] = oil;

      if (leaking() && (gasVolume > 0.0 or oilVolume > 0.0))
         m_capSealStrength.compute(trapComposition, gorm, T_K, brinePressure, capSealStrength_H2O_Gas, capSealStrength_H2O_Oil);

      double gasVolumeLeaked = 0.0;
      double gasVolumeSpilled = 0.0;
      double gasVolumeWasted = 0.0;
      double oilVolumeLeaked = 0.0;
      double oilVolumeSpilled = 0.0;

#ifdef DEBUG_LEAKWASTEANDSPILLDISTRIBUTOR
      vector<double> oilWeights = oil.getWeights();
      vector<double> gasWeights = gas.getWeights();
      double oilWeight = oil.getWeight();   
#endif

      // If fractureSealStrength and capSealStrength_H2O_Gas or capSealStrength_H2O_Oil are infinite, 
      // leaking of course doesn't happen for gas or oil:
      bool isLeaking = leaking ();
      if (gasVolume > 0.0)
      {
         if (gas.getDensity () > m_sealFluidDensity)
         {
            isLeaking = false;
#ifdef DEBUG_LEAKWASTEANDSPILLDISTRIBUTOR
            cerr << "WARNING: No seal pressure leakage because gas density: " << gas.getDensity() << 
               " is higher than water density: " << m_sealFluidDensity << "." << endl;
#endif
         }
         else
            isLeaking = isLeaking && (min (m_fractureSealStrength, capSealStrength_H2O_Gas) < numeric_limits<double>::max ());

         if (oilVolume > 0.0 && oil.getDensity () > m_sealFluidDensity)
         {
#ifdef DEBUG_LEAKWASTEANDSPILLDISTRIBUTOR
            cerr << "WARNING: oil density: " << oil.getDensity() << 
               " is larger than the density of the seal fluid: " << m_sealFluidDensity << "." << endl;
#endif
         }
      }
      else if (oilVolume > 0.0)
      {
         if (oil.getDensity () > m_sealFluidDensity)
         {
            isLeaking = false;
#ifdef DEBUG_LEAKWASTEANDSPILLDISTRIBUTOR
            cerr << "WARNING: No seal pressure leakage because oil density: " << oil.getDensity() << 
               " is higher than water density: " << m_sealFluidDensity << "." << endl;
#endif
         }
         else
            isLeaking = isLeaking && (min (m_fractureSealStrength, capSealStrength_H2O_Oil) < numeric_limits<double>::max ());
      }

      // Call the right spill and leak algorithm:
      if (gasVolume > 0.0 && oilVolume > 0.0)
      {
         if (isLeaking && wasting ())
            LeakOrWasteGasAndSpillOil (gas.getDensity (), oil.getDensity (), m_sealFluidDensity, m_fractureSealStrength,
            capSealStrength_H2O_Gas, capSealStrength_H2O_Oil, m_wasteLevel, m_levelToVolume).distribute (
            gasVolume, oilVolume, gasVolumeLeaked, gasVolumeWasted, gasVolumeSpilled, oilVolumeLeaked,
            oilVolumeSpilled);
         else if (isLeaking)
            LeakGasAndSpillOil (gas.getDensity (), oil.getDensity (), m_sealFluidDensity, m_fractureSealStrength,
            capSealStrength_H2O_Gas, capSealStrength_H2O_Oil, m_levelToVolume).distribute (
            gasVolume, oilVolume, gasVolumeLeaked, gasVolumeSpilled, oilVolumeLeaked,
            oilVolumeSpilled);
         else if (wasting ())
            WasteGasAndSpillOil (m_wasteLevel, m_levelToVolume).distribute (
            gasVolume, oilVolume, gasVolumeWasted, oilVolumeSpilled);
         else // !isLeaking && !wasting()
            SpillOilAndGas (m_levelToVolume).distribute (
            gasVolume, oilVolume, gasVolumeSpilled, oilVolumeSpilled);
      }
      else if (gasVolume > 0.0)
      {
         if (isLeaking && wasting ())
            LeakOrWaste (gas.getDensity (), m_sealFluidDensity, min (m_fractureSealStrength, capSealStrength_H2O_Gas),
            m_wasteLevel, m_levelToVolume).distribute (
            gasVolume, gasVolumeLeaked, gasVolumeWasted);
         else if (isLeaking)
            LeakOrSpill (gas.getDensity (), m_sealFluidDensity, min (m_fractureSealStrength, capSealStrength_H2O_Gas),
            m_levelToVolume).distribute (
            gasVolume, gasVolumeLeaked, gasVolumeSpilled);
         else if (wasting ())
            Waste (m_wasteLevel, m_levelToVolume).distribute (
            gasVolume, gasVolumeWasted);
         else // !isLeaking && !wasting
            Spill (m_levelToVolume).distribute (
            gasVolume, gasVolumeSpilled);
      }
      else if (oilVolume > 0.0)
      {
         // Oil wasting is already taken care of in the m_levelToVolume:
         if (isLeaking)
            LeakOrSpill (oil.getDensity (), m_sealFluidDensity, min (m_fractureSealStrength, capSealStrength_H2O_Oil),
            m_levelToVolume).distribute (
            oilVolume, oilVolumeLeaked, oilVolumeSpilled);
         else // !isLeaking
            Spill (m_levelToVolume).distribute (
            oilVolume, oilVolumeSpilled);
      }

      assert (gasVolumeLeaked >= 0.0);
      assert (gasVolumeWasted >= 0.0);
      assert (gasVolumeSpilled >= 0.0);
      assert (oilVolumeLeaked >= 0.0);
      assert (oilVolumeSpilled >= 0.0);

      // Copy the volumes to the output Compositions:
      remainingGas = gas; leakedGas = gas; wastedGas = gas; spilledGas = gas;
      remainingOil = oil; leakedOil = oil; spilledOil = oil;

      leakedGas.setVolume (gasVolumeLeaked);
      wastedGas.setVolume (gasVolumeWasted);
      spilledGas.setVolume (gasVolumeSpilled);

      leakedOil.setVolume (oilVolumeLeaked);
      spilledOil.setVolume (oilVolumeSpilled);

      // Calculate the final gas and HC volumes and levels:
      double finalGasVolume = gasVolume - gasVolumeLeaked - gasVolumeWasted - gasVolumeSpilled;
      double finalOilVolume = oilVolume - oilVolumeLeaked - oilVolumeSpilled;

      remainingGas.setVolume (finalGasVolume);
      remainingOil.setVolume (finalOilVolume);

      finalGasLevel = m_levelToVolume->invert (finalGasVolume) + m_shift;
      finalHCLevel = m_levelToVolume->invert (finalGasVolume + finalOilVolume) + m_shift;
   }

   void LeakWasteAndSpillDistributor::setLevelToVolume (const MonotonicIncreasingPiecewiseLinearInvertableFunction* levelToVolume)
   {
      if (m_shift != 0 && m_levelToVolume)
      {
         delete m_levelToVolume;
         m_levelToVolume = 0;
         m_shift = 0;
      }

      m_levelToVolume = levelToVolume;

      shiftToOrigin ();
   }

} // namespace migration

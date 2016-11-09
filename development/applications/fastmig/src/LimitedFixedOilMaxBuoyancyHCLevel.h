//
// Copyright (C) 2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _MIGRATION_DISTRIBUTE_LIMITEDFIXEDOILMAXBUOYANCYHCLEVEL_H_
#define _MIGRATION_DISTRIBUTE_LIMITEDFIXEDOILMAXBUOYANCYHCLEVEL_H_

#include "FixedOilMaxBuoyancyHCLevel.h"
#include "Tuple2.h"

#include <limits>
#include <assert.h>

using functions::Tuple2;
using std::numeric_limits;

namespace migration
{
   namespace distribute
   {

      class LimitedFixedOilMaxBuoyancyHCLevel : public FixedOilMaxBuoyancyHCLevel
      {
      private:

         Tuple2<Tuple2<double> > m_gasContentLimits;
         double m_gasLevelGradient;

         double computeGasLevel (const double& hcVolume) const
         {
            // In case m_gasLevelGradient is equal to numeric_limits<double>::max(),
            // we shouldn't be here:
            assert (m_gasLevelGradient != numeric_limits<double>::max ());

            // Calculate the gas column corresponding to hcVolume by linearly interpolating 
            // between m_gasContentLimits[0] and m_gasContentLimits[1]:
            double gasVolume = hcVolume - m_oilVolume;

            // In principle we should do here m_levelToVolume->invert(gasVolume), but 
            // that is not necessary here, for we know that the gas column is between 
            // m_gasContentLimits:
            double gasLevel = m_gasContentLimits[0][0] + m_gasLevelGradient * (gasVolume -
               m_gasContentLimits[0][1]);

            return gasLevel;
         }

      public:

         LimitedFixedOilMaxBuoyancyHCLevel (const double& maxGasBuoyancyLevel, const double& oilToGasLevelRatio,
            const double& oilVolume, const MonotonicIncreasingPiecewiseLinearInvertableFunction* levelToVolume,
            const Tuple2<Tuple2<double> >& gasContentLimits) :
            FixedOilMaxBuoyancyHCLevel (maxGasBuoyancyLevel, oilToGasLevelRatio, oilVolume, levelToVolume),
            m_gasContentLimits (gasContentLimits)
         {
            // Of course this must apply:
            assert (m_gasContentLimits[0][0] <= m_gasContentLimits[1][0]);
            assert (m_gasContentLimits[0][1] <= m_gasContentLimits[1][1]);

            // In method gasLevel we need the following gradient.  And in case m_gasContentLimits[1][1] 
            // is equal to m_gasContentLimits[0][1], we don't need m_gasLevelGradient:
            m_gasLevelGradient = m_gasContentLimits[1][1] != m_gasContentLimits[0][1] ?
               (m_gasContentLimits[1][0] - m_gasContentLimits[0][0]) / (m_gasContentLimits[1][1] - m_gasContentLimits[0][1]) :
               numeric_limits<double>::max ();
         }

         bool operator<(const Tuple2<double>& hcContent) const
         {
            // This function returns whether the buoyancy represented by hcContent combined with 
            // the with hcContent corresponding gas volume (given that the oil volume m_oilVolume 
            // is constant) is less than the buoyancy given by m_maxGasBuoyancyLevel.

            // If hcContent[1] - m_oilVolume is smaller than m_gasContentLimits[0][1], the buoyancy is smaller 
            // than m_maxGasBuoyancyLevel for the gas level is supposed to be limited by m_gasContentLimits:
            if (hcContent[1] - m_oilVolume < m_gasContentLimits[0][1])
               return false;

            // And if hcContent[1] - m_oilVolume is larger than m_gasContentLimits[1][1], the buoyancy is larger 
            // than m_maxGasBuoyancyLevel for again the gas level is supposed to be limited by m_gasContentLimits:
            if (hcContent[1] - m_oilVolume > m_gasContentLimits[1][1])
               return true;

            // If we get here it is not obvious what to return.  (And we get here if the m_levelToVolume 
            // function has more derivative jumps around the HC level than around the gas level.) 
            // We calculate therefore first the gas column column corresponding to hcContent.  
            double gasLevel = computeGasLevel (hcContent[1]);

            // Calculate the buoyancy for the combined oil and gas volume in gas units:
            double gasReprLevel = gasLevel + m_oilToGasLevelRatio * (hcContent[0] - gasLevel);

            // Make the comparison:
            return m_maxGasBuoyancyLevel < gasReprLevel;
         }
      };

   }
} // namespace migration::distribute

#endif

#include "LeakGasAndSpillOil.h"

#include "FilledUpOilMaxBuoyancyGasLevel.h"

#ifdef DEBUG_LEAKGASANDSPILLOIL
#ifdef sgi
   #ifdef _STANDARD_C_PLUS_PLUS
      #include <iostream>
   #else // !_STANDARD_C_PLUS_PLUS
      #include<iostream.h>
   #endif // _STANDARD_C_PLUS_PLUS
#else // !sgi
   #include <iostream>
#endif // sgi
#endif // DEBUG_LEAKGASANDSPILLOIL

#include <algorithm>
#include <assert.h>

using std::min;
using functions::tuple;
#ifdef DEBUG_LEAKGASANDSPILLOIL
using std::cerr;
using std::endl;
using std::max;
#endif

namespace migration { namespace distribute {

// FIXME
const int X = 0;
const int Y = 1;

LeakGasAndSpillOil::LeakGasAndSpillOil(const double& gasDensity, const double& oilDensity,
   const double& sealFluidDensity, const double& fracturePressure, 
   const double& capPressure_H2O_Gas, const double& capPressure_H2O_Oil,  
   const MonotonicIncreasingPiecewiseLinearInvertableFunction* levelToVolume):
   m_leakGas(gasDensity, oilDensity, sealFluidDensity, fracturePressure,
      capPressure_H2O_Gas, capPressure_H2O_Oil, levelToVolume),
   m_spillOilAndGas(levelToVolume)
{}

/// Compute the leakage of oil and gas according to the seal failure parameters.
/// @param[in] gasVolume
/// @param[in] oilVolume
/// @param[out] gasVolumeLeaked
/// @param[out] oilVolumeLeaked

void LeakGasAndSpillOil::distribute(const double& gasVolume, const double& oilVolume, 
   double& gasVolumeLeaked, double& gasVolumeSpilled, double& oilVolumeLeaked,
   double& oilVolumeSpilled) const
{
   gasVolumeLeaked = 0.0;
   gasVolumeSpilled = 0.0;
   oilVolumeLeaked = 0.0;
   oilVolumeSpilled = 0.0;

   // If there is not enough HC's to fill the complete trap, only leaking occurs. 
   // Go then to m_leakGas:
   Tuple2<double> capacity = m_spillOilAndGas.maxContent();
   if (gasVolume + oilVolume < capacity[1]) {
      m_leakGas.distribute(gasVolume, oilVolume, gasVolumeLeaked, oilVolumeLeaked);
      return;
   }

   // So there is oil and there is gas, and either gas is leaked or oil is spilled, 
   // for the total volume of gas and oil exceed the capacity of the trap.
   // We must determine here what case applies:
   //
   // a) The max buoyancy of the added gas and oil volumes is equal or larger than 
   // the seal-based capacity of the trap.
   // b) The max buoyancy is less than the capacity of the trap.
   //
   // In case a) oil, and sometimes also gas, will be spilled.  In case b) gas, and 
   // sometimes also oil, will be leaked.  In case a) the combined gas and oil volumes  
   // will in the end equal the capacity of the trap.  In case b), the combined oil and 
   // gas volumes won't occupy the complete trap.  However, there is a catch.  The 
   // buoyancy is not a fixed parameter.  It depends upon how much gas there is in the 
   // combined oil and gas column.
   //
   // However, we can calculate when no leaking will happen.  For if the complete trap 
   // would be completely filled with gas, the max buoyancy level would be very small, 
   // and leaking would take precedence.  Conversely, if the complete trap would be filled 
   // with oil, the max buoyancy level would be high, and HC's will consequently only 
   // be spilled.  So there is a maximum gas level below which only spilling occurs.  
   // The following functor returns what buoyancy (filled up with oil) gas levels 
   // represent in relationship to the max buoyancy level:

   FilledUpOilMaxBuoyancyGasLevel maxGasLevelFunctor(m_leakGas.maxGasLevel(), 
      m_leakGas.oilToGasLevelRatio(), capacity[0], m_leakGas.levelToVolume());

   // The maximum gas level which can occur in the trap is determined by the initial 
   // gasVolume:
   Tuple2<double> maxGasContent;
   if (gasVolume < capacity[1]) {
      maxGasContent[1] = gasVolume;
      maxGasContent[0] = m_leakGas.levelToVolume()->invert(maxGasContent[1]);
   }
   else
      maxGasContent = capacity;

   // If the buoyancy of the column with the maximum gas level obtainable within the 
   // trap is below the max buoyancy level, we know that buoyancy will always exceed 
   // capacity:
   bool onlySpilling = maxGasLevelFunctor >= maxGasContent;

   if (onlySpilling) {

       // So only spilling occurs:
       m_spillOilAndGas.distribute(gasVolume, oilVolume, gasVolumeSpilled, oilVolumeSpilled);
       return;
   }

   // It is also easy to determine whether it is certain that HC's are leaked.  Like for 
   // the case where only spilling occurs, the level is determined by the gas level, 
   // but now the gas level is a minimum.  If the gas level (topped up with oil in order 
   // to fill the complete trap) exceeds the max buoyancy level for gas (as given by 
   // the maxGasLevelFunctor), leaking will always take precedence and all HC's will be 
   // leaked, not spilled:

   Tuple2<double> minGasContent;
   if (oilVolume < capacity[1]) {
      minGasContent[1] = capacity[1] - oilVolume;
      minGasContent[0] = m_leakGas.levelToVolume()->invert(minGasContent[1]);
   }
   else
      minGasContent = tuple(0.0,0.0);

   bool onlyLeaking = maxGasLevelFunctor < minGasContent;

   if (onlyLeaking) {
      // So there is only leakage.  Leak HC's.  Note that like for only spilling 
      // it is also possible that next to gas also oil will be leaked.  And ultimately, 
      // this is the reason why we do the case with only leaking separate from the 
      // combined case below:
      m_leakGas.distribute(gasVolume, oilVolume, gasVolumeLeaked, oilVolumeLeaked);
      return;
   }
      
   // It seems that everything is clear: either oil is spilled or gas is leaked, but it 
   // isn't.  It can happen that for the minimum gas level, the max buoyancy level is above 
   // capacity, and that for the maximum gas level, the max buoyancy is below capacity. 
   // In this case oil will be spilled and gas will be leaked in such a way that the max 
   // buoyancy level will become equal to capacity.  The search for this gas level is done 
   // here:

   int gasIndex = m_leakGas.levelToVolume()->findIndex(maxGasLevelFunctor);

#ifdef DEBUG_LEAKGASANDSPILLOIL
   cerr << endl;
   cerr << "gasVolume: " << gasVolume << endl;
   cerr << "oilVolume: " << oilVolume << endl;
   cerr << "hcVolume : " << gasVolume + oilVolume << endl;

   for (int i = max(0,gasIndex-3); i < min(gasIndex+4,m_leakGas.levelToVolume()->size()); ++i) {
      Tuple2<Tuple2<double> > limits = m_leakGas.levelToVolume()->piece(i);
      cerr << "index: " << i << " =  [" << limits[0][0] << "," << limits[0][1] << "] -> [" << 
         limits[1][0] << "," << limits[1][1] << "]." << endl;
   }

   cerr << "gasIndex: " << gasIndex << endl;

#endif

   // Assure the solution makes sense:
   assert(0 <= gasIndex);
   assert(gasIndex < m_leakGas.levelToVolume()->size());

   // The required gas level is somewhere between these limits;
   Tuple2<Tuple2<double> > gasLimits = m_leakGas.levelToVolume()->piece(gasIndex);

   assert(gasLimits[0][X] + m_leakGas.oilToGasLevelRatio() * (capacity[0] - gasLimits[0][X]) <= 
      m_leakGas.maxGasLevel());
   assert(gasLimits[1][X] + m_leakGas.oilToGasLevelRatio() * (capacity[0] - gasLimits[1][X]) >= 
      m_leakGas.maxGasLevel());

   // In order to calculate the precise gas volume between these limits, we must do
   // some algebra.  This calculation is done in computeFinalGasVolume:
   double finalGasVolume = computeFinalGasVolume(m_leakGas.maxGasLevel(), gasLimits);
   assert(finalGasVolume >= 0.0);
   assert(finalGasVolume <= gasVolume);

   // Compute the with finalGasVolume corresponding oil volume:
   double finalOilVolume = capacity[1] - finalGasVolume;
   assert(finalOilVolume >= 0.0);
   assert(finalOilVolume <= oilVolume);

   // Calculated the leakages and spillages:
   gasVolumeLeaked = gasVolume - finalGasVolume;
   oilVolumeSpilled = oilVolume - finalOilVolume;
}

double LeakGasAndSpillOil::computeFinalGasVolume(const double maxGasBuoyancyLevel, 
   const Tuple2<Tuple2<double> >& gasLimits) const
{
   double capacityLevel = m_spillOilAndGas.maxLevel();

   // Below we need the following gradient:
   double gasGradient = (gasLimits[1][Y] - gasLimits[0][Y]) / (gasLimits[1][X] - gasLimits[0][X]);
   assert(gasGradient >= 0.0);

   // We have narrowed the gas column down to a piece of m_leakGas.levelToVolume() function 
   // where the depth to volume relationship is linear. The precise gas column can now 
   // be calculated by means of some algebra. The following equations apply:
   //
   // gasLevel + ( capacityLevel - gasLevel ) * ratio = maxGasBuoyancyLevel      (i)
   //
   // gasVolume = gasLimits[0][Y] + gasGradient * ( gasLevel - gasLimits[0][X] ) (ii)
   //
   // with ratio m_leakGas.oilToGasLevelRatio().
   //
   // (i) : At the end, the buoyancy equals maxGasBuoyancyLevel.
   // (ii): When gasLevel increases, gasVolume increases linearly with gasGradient.
   //
   // The soluation is easy to obtain:
   //
   //                 maxGasBuoyancyLevel - capacityLevel * ratio
   //   gasLevel  =  ---------------------------------------------
   //                   1.0  - m_leakGas.oilToGasLevelRatio()
   //
   //   gasVolume  =  gasLimits[0][Y] + gasGradient * ( gasLevel - gasLimits[0][X] )
   //
   
   double gasLevel;
   gasLevel  = maxGasBuoyancyLevel;
   gasLevel -= capacityLevel * m_leakGas.oilToGasLevelRatio();
   gasLevel /= (1.0 - m_leakGas.oilToGasLevelRatio());

   double gasVolume = gasLimits[0][Y] + gasGradient * (gasLevel - gasLimits[0][X] );

   // Check if the solution make sense:
   assert(gasVolume >= 0.0);
   assert(0.0 <= gasLevel && gasLevel <= m_leakGas.levelToVolume()->
      invert(std::numeric_limits<double>::max()));

   assert(gasLimits[0][X] <= gasLevel && gasLevel <= gasLimits[1][X]);
   assert(gasLimits[0][Y] <= gasVolume && gasVolume <= gasLimits[1][Y]);

   return gasVolume;
}

} } // namespace migration::distribute

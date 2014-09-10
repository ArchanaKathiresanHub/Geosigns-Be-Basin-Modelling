#include "LeakGas.h"

#include "FixedOilMaxBuoyancyGasLevel.h"
#include "LimitedFixedOilMaxBuoyancyHCLevel.h"
#include "functions/src/Tuple2.h"
#include <math.h>

#ifdef sgi
   #ifdef _STANDARD_C_PLUS_PLUS
      #include <iostream>
   #else // !_STANDARD_C_PLUS_PLUS
      #include<iostream.h>
   #endif // _STANDARD_C_PLUS_PLUS
#else // !sgi
   #include <iostream>
#endif // sgi

#ifdef DEBUG_LEAKGAS
#ifdef sgi
  #ifdef _STANDARD_C_PLUS_PLUS
    #include<sstream>
    using std::ostringstream;
  #else // !_STANDARD_C_PLUS_PLUS
    #include<strstream.h>
    typedef strstream ostringstream;
  #endif // _STANDARD_C_PLUS_PLUS
#else // !sgi
  #include <sstream>
  using std::ostringstream;
#endif // sgi
#endif // DEBUG_LEAKGAS

#ifdef DEBUG_LEAKGAS_2
#include "SCmathlib.h"
#endif // DEBUG_LEAKGAS_2

#include <algorithm>
#include <assert.h>

using functions::tuple;

using namespace std;

namespace migration { namespace distribute {

#ifdef DEBUG_LEAKGAS
const double TOLERANCE = 1.0e-6;
#endif

// FIXME
const int X = 0;
const int Y = 1;

LeakGas::LeakGas(const double& gasDensity, const double& oilDensity,
   const double& sealFluidDensity, const double& fracturePressure, 
   const double& capPressure_H2O_Gas, const double& capPressure_H2O_Oil,  
   const MonotonicIncreasingPiecewiseLinearInvertableFunction* levelToVolume):
   m_levelToVolume(levelToVolume),
   m_leakGas(gasDensity, sealFluidDensity, min(fracturePressure, capPressure_H2O_Gas),
      levelToVolume),
   m_leakOil(oilDensity, sealFluidDensity, min(fracturePressure, capPressure_H2O_Oil),
      levelToVolume),
   m_oilToGasLevelRatio(0.0)
{
   // Normally the following conditions should apply:
   // assert(sealFluidDensity > oilDensity);
   // assert(oilDensity > gasDensity);
   // assert(gasDensity > 0.0);
   // However, sealFluidDensity and gas and oil density are calculated with different formulas, 
   // so on occasion, the oil density can be bigger than the fluid density. So the assertions 
   // are sometimes be violated. However, the leaking model doesn't allow for oil densities 
   // (or gas densities) bigger than the seal fluid density. Therefore, if either the density 
   // of oil, or even more strangely that of gas, is bigger than that of the seal fluid, we 
   // redefine the densities of gas and oil such that leaking won't take place, nly spilling.  
   // The best way of doing that is by limiting the density of oil and gas to that of the 
   // sealFluidDensity. As a consequence, we must use here m_leakGas.fluidDensity() and 
   // m_leakOil.fluidDensity() instead of gasDensity and oilDensity:
   m_oilToGasLevelRatio = computeOilToGasLevelRatio(m_leakGas.fluidDensity(), m_leakOil.fluidDensity(), 
      sealFluidDensity);

#ifdef DEBUG_LEAKGAS
   // Calculate the maximum capacity of the trap:
   m_capacity[0] = levelToVolume->invert(numeric_limits<double>::max());
   m_capacity[1] = levelToVolume->apply(numeric_limits<double>::max());
#endif
}

/// compute the ratio in which the oil level contributes to buoyancy and the gas level.
/// @param[in] gasDensity

double LeakGas::computeOilToGasLevelRatio(const double& gasDensity, const double& oilDensity, 
   const double& sealFluidDensity) const
{
   // The maximum gas level is given by:
   //
   //    p    =  g l        ( d  - d   )
   //     max       gas,max    s    gas
   //
   // with p_max, the maximum seal pressure, g gravity, d_s and d_gas the densities of respectively
   // the seal fluid and gas, and l_gas,max the maximum gas column. Solving for l_gas,max, this is:
   //
   //                     p
   //                      max
   //    l         =  -------------
   //     gas,max
   //                 g ( d  - d   )
   //                      s    gas
   //
   // If we have both gas and oil, the pressure is given by:
   //
   //               /                                           \
   //    p    =  g  |  l    ( d  - d   )  +  l    ( d  - d   )  |
   //               \   gas    s    gas       oil    s    oil   /
   // 
   //
   // with l_gas and l_oil the gas level and oil level (not including gas), respectively.
   // This pressure must of course be smaller than the max buoyancy pressure, hence the following 
   // condition must apply:
   //
   //                                    /                                           \
   //    g l        ( d  - d   )  >=  g  |  l    ( d  - d   )  +  l    ( d  - d   )  |
   //       gas,max    s    gas          \   gas    s    gas       oil    s    oil   /
   //
   //
   // We can rewrite this in:
   //
   //                                  d  - d
   //                                   s    oil
   //    l        >=   l     +   l     ---------
   //     gas,max       gas       oil  
   //                                  d  - d
   //                                   s    gas
   //
   // (as d_s - d_gas is larger than zero). So if we want to determine whether the buoyancy pressure 
   // of a gas and oil column exceeds max buoyancy, we can calculate a gas representative column with the 
   // following conversion parameter:
   //
   //    l       =  l    +  r * l
   //     repr       gas         oil
   // 
   // with:
   //
   //             d  - d
   //              s    oil
   //    r   =    ---------
   //        
   //             d  - d
   //              s    gas
   //
   // And r is not d_gas / d_oil, as is sometimes thought.

   double result = sealFluidDensity - oilDensity;
   assert(result >= 0.0);
   result /= (sealFluidDensity - gasDensity);
   assert(result <= 1.0);
   return result;
}

/// Compute the leakage of oil and gas according to the seal failure parameters.
/// @param[in] gasVolume
/// @param[in] oilVolume
/// @param[out] gasVolumeLeaked
/// @param[out] oilVolumeLeaked
void LeakGas::distribute(const double& gasVolume, const double& oilVolume, double& gasVolumeLeaked,
   double& oilVolumeLeaked) const
{
#ifdef DEBUG_LEAKGAS
   // gasVolume or m_gasLeak.maxVolume() must be smaller than capacity, or else spilling would have 
   // occurred instead of leaking:
   assert(gasVolume < m_capacity[1] || m_leakGas.maxVolume() < m_capacity[1]);
#endif

   gasVolumeLeaked = 0.0;
   oilVolumeLeaked = 0.0;

   // If without gas, the oil buoyancy alone exceeds maxSealPressure (calculated for gas)
   // the oil buoyancy alone is enough to leak away all gas (of course assuming that the 
   // density of gas is always less than that of oil).  In that case all gas is leaked 
   // and we only need to determine whether in addition to gas any oil is leaked as well.
   // The determination whether the oil buoyancy alone is enough to leak away all gas is done
   // with help of the following functor.  Statement maxGasLevelFunctor < tuple(0.0,0.0) 
   // below invokes FixedOilMaxBuoyancyGasLevel's method operator<(const Tuple2<double>&), 
   // and in this method the buoyancy is calculated for the given gas content (in this 
   // case a content consisting of a level and volume of 0.0) and compared to the maximum 
   // allowed buoyancy:
   FixedOilMaxBuoyancyGasLevel maxGasLevelFunctor(m_leakGas.maxLevel(), m_oilToGasLevelRatio, 
      oilVolume, m_levelToVolume);

#ifdef DEBUG_LEAKGAS
   Leak leakGasDueToOilBuoyancy(m_leakOil.fluidDensity(), m_leakGas.sealFluidDensity(), 
      m_leakGas.maxSealPressure(), m_levelToVolume);
#endif

   // If the oil buoyancy alone is bigger or equal to the maximum seal pressure, all gas 
   // will be leaked:
   if (maxGasLevelFunctor < functions::tuple(0.0,0.0) ) 
   {  
#ifdef DEBUG_LEAKGAS
      assert(oilVolume >= leakGasDueToOilBuoyancy.maxVolume());
#endif

      // Leak away all gas:
      gasVolumeLeaked = gasVolume;

      // Determine also how much oil is leaked.  
      m_leakOil.distribute(oilVolume, oilVolumeLeaked);
      return;
   }

#ifdef DEBUG_LEAKGAS
   assert(oilVolume < leakGasDueToOilBuoyancy.maxVolume());
#endif

   // In all other cases, the buoyancy is only enough to leak gas.  First test if leaking 
   // happens all.  For this determine first the with gasVolume corresponding gas level:
   Tuple2<double> gasContent; 
   gasContent[1] = gasVolume;
   gasContent[0] = m_levelToVolume->invert(gasVolume);

   // If the gas level, gasContent[0], is now smaller than the gas level which together with the 
   // fixed oil volume would yield the maximum gas buoyancy, nothing is leaked.  (Note: statement 
   // maxGasLevelFunctor < gasLevel invokes FixedOilMaxBuoyancyGasLevel's method 
   // operator<(const Tuple2<double>&).):
   bool leaking = maxGasLevelFunctor < gasContent;
   if (!leaking)
      return;

   // So some gas will be leaked.  We calculate here the resultant gas level such that the 
   // buoyancy of the combined gas and oil volume equals that of m_leakGas.maxLevel().  
   // The calculation is a bit tricky, given that when the gas level changes, the HC level 
   // changes as well (given that m_levelToVolume varies anywhere and therefore also for oil).
   // The calculation is done in three steps.  We first determine at what linear piece of 
   // the m_levelToVolume function the gas level is positioned.  Subsequently we determine 
   // at what linear piece where the HC level is positioned.  (If the top of the trap is 
   // narrow, the linear piece of m_levelToVolume of the HC level is not uniquely determined 
   // by that of the gas level.  So this extra step is necessary if we want to uniquely 
   // determine both linear pieces.)  When we have got both linear pieces, we can then 
   // calculate the final gas (and corresponding HC level) by some linear algebra.

   // The linear piece of the gas level is calculated with the maxGasLevelFunctor class:
   int gasIndex = m_levelToVolume->findIndex(maxGasLevelFunctor);

#ifdef DEBUG_LEAKGAS
   cerr << endl;
   cerr << "gasVolume: " << gasVolume << endl;
   cerr << "oilVolume: " << oilVolume << endl;
   cerr << "hcVolume : " << gasVolume + oilVolume << endl;

   for (int i = max(0,gasIndex-3); i < min(gasIndex+4,m_levelToVolume->size()); ++i) {
      Tuple2<Tuple2<double> > limits = m_levelToVolume->piece(i);
      cerr << "index: " << i << " =  [" << limits[0][0] << "," << limits[0][1] << "] -> [" << 
         limits[1][0] << "," << limits[1][1] << "]." << endl;
   }

   cerr << "gasIndex: " << gasIndex << endl;

#endif

   // gasIndex < 0 happens when the oil pressure alone is enough to break the seal. 
   // However, that case is already covered.  gasIndex > m_levelToVolume->size() occurs 
   // when the added oil and gas volumes exceed capacity. However, that shouldn't occur 
   // here either, for that is another class:
   assert(0 <= gasIndex);
   assert(gasIndex < m_levelToVolume->size());

   // Now we know that the gas level is limited by:
   Tuple2<Tuple2<double> > gasLimits = m_levelToVolume->piece(gasIndex);

   // Calculate the linear piece of the HC level:
   LimitedFixedOilMaxBuoyancyHCLevel maxHCBuoyancy(m_leakGas.maxLevel(), m_oilToGasLevelRatio, 
      oilVolume, m_levelToVolume, gasLimits);
   int hcIndex = m_levelToVolume->findIndex(maxHCBuoyancy);

   // Again hcIndex must refer to an existing linear piece:
   assert(0 <= hcIndex);
   assert(hcIndex < m_levelToVolume->size());

#ifdef DEBUG_LEAKGAS
   cerr << endl;
   cerr << "gasVolume: " << gasVolume << endl;
   cerr << "oilVolume: " << oilVolume << endl;
   cerr << "hcVolume:  " << gasVolume + oilVolume << endl;

   for (int j = max(0,hcIndex-3); j < min(hcIndex+4,m_levelToVolume->size()); ++j) {
      Tuple2<Tuple2<double> > limits = m_levelToVolume->piece(j);
      cerr << "index: " << j << " =  [" << limits[0][0] << "," << limits[0][1] << "] -> [" 
         << limits[1][0] << "," << limits[1][1] << "]." << endl;
   }

   cerr << "hcIndex: " << hcIndex << endl << endl;

#endif

   // The limits of the HC level are given by:
   Tuple2<Tuple2<double> > hcLimits = m_levelToVolume->piece(hcIndex);

   // m_leakGas.maxLevel() is contained between these limits:
   assert(gasLimits[0][X] + m_oilToGasLevelRatio * (hcLimits[0][X] - gasLimits[0][X]) <= 
      m_leakGas.maxLevel());
   assert(gasLimits[1][X] + m_oilToGasLevelRatio * (hcLimits[1][X] - gasLimits[1][X]) >= 
      m_leakGas.maxLevel());

   // Calculate the maximum supported gas volume:
   double finalGasVolume = computeFinalGasVolume(oilVolume, gasLimits, hcLimits);

   // We determined already that leaking occured:
   assert(gasVolume > finalGasVolume);

   // The difference between gasVolume and finalGasVolume has leaked away:
   gasVolumeLeaked = gasVolume - finalGasVolume;

#ifdef DEBUG_LEAKGAS

   if (gasLimits[0][Y] == gasLimits[1][Y])
      return;
   if (hcLimits[0][Y] == hcLimits[1][Y])
      return;

   // The HC limits correspond to some gas limits and vice versa, the gas limits correspond 
   // to some HC limits.  The inferred limits can be obtained from the inferred volume limits:
   Tuple2<double> hcInferredGasVolumeLimits;
   hcInferredGasVolumeLimits[0] = hcLimits[0][Y] - oilVolume;
   hcInferredGasVolumeLimits[1] = hcLimits[1][Y] - oilVolume;

   Tuple2<double> gasInferredHCVolumeLimits;
   gasInferredHCVolumeLimits[0] = gasLimits[0][Y] + oilVolume;
   gasInferredHCVolumeLimits[1] = gasLimits[1][Y] + oilVolume;

   // Below we need the following gradients:
   double invGasGradient = (gasLimits[1][X] - gasLimits[0][X]) / (gasLimits[1][Y] - gasLimits[0][Y]);
   double invHCGradient  = (hcLimits [1][X] - hcLimits [0][X]) / (hcLimits [1][Y] - hcLimits [0][Y]);

   // For the next stage of the algorithm, we must determine corresponding limits for the 
   // HC and gas levels.  We do this by choosing the narrower of each of the two limits.
   if (gasLimits[0][Y] < hcInferredGasVolumeLimits[0]) {

      // Replace gasLimits[0] with the limits implied by hcInferredGasVolumeLimits[0]:
      gasLimits[0][X] += invGasGradient * (hcInferredGasVolumeLimits[0] - gasLimits[0][Y]);
      assert(gasLimits[0][X] <= gasLimits[1][X]);
      gasLimits[0][Y] = hcInferredGasVolumeLimits[0];
   }
   else {

      // Do here the opposite.  Replace hcLimits[0] with the limits implied by
      // gasInferredHCVolumeLimits[0]:
      hcLimits[0][X] += invHCGradient * (gasInferredHCVolumeLimits[0] - hcLimits[0][Y]);
      assert(hcLimits[0][X] <= hcLimits[1][X]);
      hcLimits[0][Y] = gasInferredHCVolumeLimits[0];
   }

   // Do for the upper limit the same as what was done for the lower limit:
   if (gasLimits[1][Y] > hcInferredGasVolumeLimits[1]) {

      // Replace gasLimits[0] with the limits implied by hcInferredGasVolumeLimits[1]:
      gasLimits[1][X] += invGasGradient * (hcInferredGasVolumeLimits[1] - gasLimits[1][Y]);
      assert(gasLimits[1][X] >= gasLimits[1][X]);
      gasLimits[1][Y] = hcInferredGasVolumeLimits[1];
   }
   else {

      // Do here the opposite.  Replace hcLimits[1] with the limits implied by
      // gasInferredHCVolumeLimits[1]:
      hcLimits[1][X] += invHCGradient * (gasInferredHCVolumeLimits[1] - hcLimits[1][Y]);
      assert(hcLimits[1][X] >= hcLimits[0][X]);
      hcLimits[1][Y] = gasInferredHCVolumeLimits[1];
   }

   double finalGasVolumeCheck = computeFinalGasVolume(oilVolume, gasLimits, hcLimits);
 
   assert(fabs(finalGasVolumeCheck - finalGasVolume) < TOLERANCE);

#endif
}

double LeakGas::computeFinalGasVolume(const double& oilVolume, const Tuple2<Tuple2<double> >& gasLimits, 
   const Tuple2<Tuple2<double> >& hcLimits) const
{
   // Below we need the following gradients:
   double gasGradient = (gasLimits[1][Y] - gasLimits[0][Y]) / (gasLimits[1][X] - gasLimits[0][X]);
   double hcGradient = (hcLimits[1][Y] - hcLimits[0][Y]) / (hcLimits[1][X] - hcLimits[0][X]);

   // We have narrowed the depths down towards segments where both the HC and gas volumes 
   // are linear. The precise gas and HC levels can now be calculated by means of some 
   // linear algebra. The following equations apply:
   //
   // x[GAS] + ( x[HC] - x[GAS] ) * m_oilToGasLevelRatio = m_leakGas.maxLevel()          (i)
   //
   // y[HC] - y[GAS] = oilVolume                                                         (ii)
   //
   // y[GAS] = gasLimits[0][Y] + gasGradient * ( x[GAS] - gasLimits[0][X] )              (iii)
   //
   // y[HC]  = hcLimits[0][Y] + hcGradient * ( x[HC] - hcLimits[0][X] )                  (iv)
   //
   // (i)  : At the end, the buoyancy equals that of m_leakGas.maxLevel().
   // (ii) : All the oil remains in the trap.
   // (iii): When x[GAS] increases, y[GAS] increases linearly with gasGradient.
   // (iv) : And when x[HC] increases, y[HC] increases linearly with hcGradient.
   //
   // These equations can be written in matrix form:
   //
   //	 /                   \  /        \      /     \
   //	 | 1-r    0    r   0 |  | x[GAS] |      | g_l |
   //	 |  0    -1    0   1 |  | y[GAS] |      | o_v |
   //	 |-g_g    1    0   0 |  | x[HC]  |   =  | G   |
   //	 |  0     0  -h_g  1 |  | y[HC]  |      | H   |
   //	 \                   /  \        /      \     /
   // 
   // with: r m_oilToGasLevelRatio, g_l m_leakGas.maxLevel(), o_v oilVolume, g_g gasGradient, 
   // G gasLimits[0][Y] - g_g * gasLimits[0][X], h_g hcGradient, and 
   // H hcLimits[0][Y] - h_g * hcLimits[0][X].
   //
   // Subtracting (iv) from (ii) simplifies these equations to:
   //
   //	 /                   \  /        \      /         \
   //	 | 1-r    0    r   0 |  | x[GAS] |      | g_l     |
   //	 |  0    -1   h_g  0 |  | y[GAS] |      | o_v - H |
   //	 |-g_g    1    0   0 |  | x[HC]  |   =  | G       |
   //	 |  0     0  -h_g  1 |  | y[HC]  |      | H       |
   //	 \                   /  \        /      \         /
   //
   // Adding (iii) to (ii) yields:
   //
   //	 /                   \  /        \      /             \
   //	 | 1-r    0    r   0 |  | x[GAS] |      | g_l         |
   //	 |-g_g    0   h_g  0 |  | y[GAS] |      | o_v - H + G |
   //	 |-g_g    1    0   0 |  | x[HC]  |   =  | G           |
   //	 |  0     0  -h_g  1 |  | y[HC]  |      | H           |
   //	 \                   /  \        /      \             /
   //
   // Multiplying (ii) with r / h_g and subtracting that from (i) yields the solution:
   //
   //              h_g g_l - r (o_v - H + G)
   //   x[GAS]  =  -------------------------
   //                 (1 - r) h_g + r g_g
   //
   //                          h_g g_l - r (o_v - H + G)
   //   y[GAS]  =  G  +  g_g * -------------------------
   //                             (1 - r) h_g + r g_g
   //
   
   double G = gasLimits[0][Y] - gasGradient * gasLimits[0][X];
   double H = hcLimits[0][Y] - hcGradient * hcLimits[0][X];
   double num = m_oilToGasLevelRatio * (oilVolume - H + G);
   num = hcGradient * m_leakGas.maxLevel() - num;

   double denom = (1.0 - m_oilToGasLevelRatio) * hcGradient + m_oilToGasLevelRatio * gasGradient;
   assert(denom != 0.0);

   double xGas = num / denom;
   double yGas = G + gasGradient * xGas;

#ifdef DEBUG_LEAKGAS_2
   // Check if the linear algebra calculation is right:

   Genex5::SCMatrix A(4,4);
   A(0,0) = 1.0 - m_oilToGasLevelRatio;
   A(0,2) = m_oilToGasLevelRatio;
   A(1,1) = -1.0; A(1,3) = 1.0;
   A(2,0) = -gasGradient; A(2,1) = 1.0;
   A(3,2) = -hcGradient; A(3,3) = 1.0;

   Genex5::SCVector gh(4);
   gh(0) = m_leakGas.maxLevel(); gh(1) = oilVolume; gh(2) = G; gh(3) = H;

   Genex5::SCVector xy(4);

   Genex5::GEPP(A,gh,xy);

   // If all element of xy are zero, the linear algebra calculation failed (and that can 
   // happen, for the matrix elements can have enormous differences in size).  In that case 
   // don't do the asserts:
   if (xy(0) == 0.0 && xy(1) == 0.0)
      return yGas;

   assert(fabs(xGas - xy(0)) <= TOLERANCE );
   assert(fabs(yGas - xy(1)) <= TOLERANCE );

   assert(fabs(xy(0) + m_oilToGasLevelRatio * (xy(2) - xy(0)) - m_leakGas.maxLevel()) <= TOLERANCE);
#endif

   // Check if the solution makes sense:
   assert(yGas >= 0.0);
   assert(0.0 <= xGas && xGas <= m_levelToVolume->invert(numeric_limits<double>::max()));

   assert(gasLimits[0][X] <= xGas && xGas <= gasLimits[1][X]);
   assert(gasLimits[0][Y] <= yGas && yGas <= gasLimits[1][Y]);

   return yGas;
}

} } // namespace migration::distribute

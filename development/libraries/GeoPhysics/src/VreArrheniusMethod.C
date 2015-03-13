#include "VreArrheniusMethod.h"

#include <cassert>
#include <cmath>

#include "NumericFunctions.h"

#ifdef _MSC_VER
#include <float.h>  // for _isnan() on VC++
#define isnan(x) _isnan(x)  // VC++ uses _isnan() instead of isnan()
#endif /** _MSC_VER */

namespace GeoPhysics {

VreArrheniusMethod::VreArrheniusMethod(int numberOfReactions, double preExponentialFactorA, const double * stoichiometricFactors, const double * activationEnergies)
   : m_numberOfReactions( numberOfReactions ),
     m_preExponentialFactorA( preExponentialFactorA ),
     m_stoichiometricFactors( stoichiometricFactors, stoichiometricFactors + numberOfReactions),
     m_activationEnergies( activationEnergies, activationEnergies + numberOfReactions ),
     m_deltaI(),
     m_fractionF()
{
   assert( m_numberOfReactions == (int) m_stoichiometricFactors.size() );
   assert( m_numberOfReactions == (int) m_activationEnergies.size() );  
}


namespace {
   namespace sweeney {

   int   naes  = 20;      /** number of reactions */

   double arr  = 1.0e13;  /** pre-exponential factor A */

   double fe[] =          /** stoichiometric factors */
   { 
      0.03, 0.03, 0.04, 0.04, 0.05, 
      0.05, 0.06, 0.04, 0.04, 0.07, 
      0.06, 0.06, 0.06, 0.05, 0.05, 
      0.04, 0.03, 0.02, 0.02, 0.01 
   };
 
   double en[]   =        /** activation energies */
   { 
      34000.0, 36000.0, 38000.0, 40000.0, 42000.0, 
      44000.0, 46000.0, 48000.0, 50000.0, 52000.0, 
      54000.0, 56000.0, 58000.0, 60000.0, 62000.0, 
      64000.0, 66000.0, 68000.0, 70000.0, 72000.0 
   }; 

} } /// namespace Sweeny / namespace anonymous

const double SweeneyBurnham::s_initialVRe = 0.2;

SweeneyBurnham :: SweeneyBurnham()
   : VreArrheniusMethod( sweeney::naes, sweeney::arr, sweeney::fe, sweeney::en )
{} 



/** Same quantties as in Sweeney & Burnham */
namespace { 
   namespace larter {

   int   naes  = 13;
   double arr  = 7.4e8;
   double fe[] = 
   {
      0.0020, 0.0088, 0.0270, 0.0648, 0.1210,
      0.1760, 0.1995, 0.1760, 0.1210, 0.0648, 
      0.0270, 0.0088, 0.0020
   };
   double en[] = 
   {     
      33119., 36675., 38230., 39786., 41342.,
      42898., 44454., 46010., 47566., 49122.,
      50678., 52233., 53789.
   };
} } /// end namespace Larter / end namespace anonymous

const double Larter::s_initialVRe = 0.2;

Larter :: Larter()
   : VreArrheniusMethod( larter::naes, larter::arr, larter::fe, larter::en )
{} 

void VreArrheniusMethod :: initVectors( int gridSize )
{
   if (m_fractionF.empty())
   {
      assert( m_deltaI.empty() );
      m_deltaI.resize( gridSize * m_numberOfReactions, 0.0 );
      m_fractionF.resize( gridSize, 0.0 );
   }
}

void VreArrheniusMethod :: reset()
{
   m_deltaI.clear();
   m_fractionF.clear();
}


/// This method calculates the fraction of converted reactants (F)
void VreArrheniusMethod :: doTimestep( const InputGrid & previousGrid, const InputGrid & currentGrid )
{
   assert( previousGrid.getSize() == currentGrid.getSize() );

   /** When we call this function the first time, some members still have to be initialized */
   initVectors( previousGrid.getSize() );

   assert( previousGrid.getSize() == currentGrid.getSize() );
   assert( currentGrid.getSize() * m_numberOfReactions == m_deltaI.size() );
   assert( m_fractionF.size() == currentGrid.getSize() );
   const int gridSize = currentGrid.getSize();  
   const double * currentTemperatureGrid = currentGrid.getTemperature();
   const double * previousTemperatureGrid = previousGrid.getTemperature();

   const double timeStep = previousGrid.getTime() - currentGrid.getTime();

   assert( timeStep > 0.0 );

   const int numberOfActiveNodes = currentGrid.getNumberOfActiveNodes();
   const int * activeNodes = currentGrid.getActiveNodes();
   for (int activeNode = 0; activeNode < numberOfActiveNodes; ++activeNode)
   {
      const int node = activeNodes[ activeNode ];

      /** Quantities from Appendix 1 of Sweeney & Burnham (1990) */

      /** temperatures (in Kelvins) at previous and current timestep */
      double previousTemperature = std::max( 1.0, previousTemperatureGrid[node] + 273.15);
      double currentTemperature  = std::max( 1.0, currentTemperatureGrid[node] + 273.15);

      /** factors a and b used in eq. (10) */
      const double a1 = 2.334733;
      const double a2 = 0.250621;
      const double b1 = 3.330657;
      const double b2 = 1.681534;

      /** Constant temperature not allowed so check */
      if ( NumericFunctions::isEqual ( previousTemperature, currentTemperature, 1.0e-3 ))
      {
         if ( currentTemperature > previousTemperature ) 
         {
            currentTemperature = currentTemperature + 1.0e-3;
         }
         else 
         {
            previousTemperature = previousTemperature + 1.0e-3;
         }
      }
      const double numberOfSecondsInMillionYears = 3.15576e13;
      const double gasConstant = 1.987; /// gas constant (also known as R)

      const double ratio = timeStep / (currentTemperature - previousTemperature);    /** 1/H, see eq. (7) */
      const double delt = m_preExponentialFactorA * numberOfSecondsInMillionYears * ratio; /** A/H, see eq. (3) and (7) */

      double fractionF = 0.0;  /** F from eq. (6) */ 

      /** Loop over the number of reactions */
      for (int reaction = 0; reaction < m_numberOfReactions; ++reaction) 
      {
         /** (E/RT), see eq. (3), at previous and current timestep */
         double etCurrent = m_activationEnergies[reaction] / (currentTemperature * gasConstant);
         double etPrevious = m_activationEnergies[reaction] / (previousTemperature * gasConstant);

         /** square brackets of eq. (10) at previous and current timestep */
         double edCurrent = 1.0 - (etCurrent * etCurrent + a1 * etCurrent + a2)
                                   / (etCurrent * etCurrent + b1 * etCurrent +  b2);
         double edPrevious = 1.0 - (etPrevious * etPrevious + a1 * etPrevious + a2)
                                   / (etPrevious * etPrevious + b1 * etPrevious + b2);

     
         /** deltaI is \f$ \delta I_{ij} \f$, see eq. (9) */
         double & deltaI = m_deltaI[reaction + node * m_numberOfReactions];
         deltaI += std::max(delt * ( currentTemperature * edCurrent * std::exp(- etCurrent ) -
                                     previousTemperature * edPrevious * std::exp(- etPrevious ) ) , 0.0);
         fractionF += m_stoichiometricFactors[reaction] * (1.0 - std::exp( - deltaI ));
      } /// end for each reaction
 
      assert ( !isnan( fractionF ));
      m_fractionF[node] = fractionF;

   } /// end for each node
}

void VreArrheniusMethod::getResults( OutputGrid & outputGrid) 
{
   const int gridSize = outputGrid.getSize();
   double * vreStorageArray = outputGrid.getVRe();

   initVectors( outputGrid.getSize() );
 
   for (int node = 0; node < gridSize; ++node)
   {
      vreStorageArray[node] = convertFractionToVR( m_fractionF[node] );

      assert( !isnan( vreStorageArray[node] ) );
   }
}

} // end namespace GeoPhysics

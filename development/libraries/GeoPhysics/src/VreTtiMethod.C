#include "VreTtiMethod.h"

#include <assert.h>
#include <math.h>

#include "NumericFunctions.h"

#ifdef _MSC_VER
#include <float.h>  // for _isnan() on VC++
#define isnan(x) _isnan(x)  // VC++ uses _isnan() instead of isnan()
#endif /** _MSC_VER */

namespace GeoPhysics { 

const double VreTtiMethod::s_referenceTemperature = 105.0;
const double VreTtiMethod::s_temperatureIncrement = 10.0;
const double VreTtiMethod::s_initialVRe = 0.2;


VreTtiMethod::VreTtiMethod(double factorF, double lopatinP, double lopatinQ)
   : m_factorF( factorF ),
     m_lopatinP( lopatinP ),
     m_lopatinQ( lopatinQ )
{

}

void VreTtiMethod::initVectors(int gridSize)
{
   if (m_tti.empty())
   {
      m_tti.resize( gridSize, 0.0 );
   }
}

void VreTtiMethod::reset()
{
   m_tti.clear();
}


void VreTtiMethod::doTimestep( const InputGrid & previousGrid, const InputGrid & currentGrid )
{
   assert( previousGrid.getSize() == currentGrid.getSize() );
   
   /** When we call this function the first time, some members still have to be initialized */
   initVectors( currentGrid.getSize() );

   assert( previousGrid.getSize() == currentGrid.getSize() );
   assert( m_tti.size() == currentGrid.getSize() );
   const int gridSize = currentGrid.getSize();  
   const double * currentTemperatureGrid = currentGrid.getTemperature();
   const double * previousTemperatureGrid = previousGrid.getTemperature();
   
   const double timeStep = previousGrid.getTime() - currentGrid.getTime();
   assert( timeStep > 0.0 );

   const double a = std::log ( m_factorF) / s_temperatureIncrement; /** a=ln(f)/10, where f is the Lopatin factor */

   const int numberOfActiveNodes = currentGrid.getNumberOfActiveNodes();
   const int * activeNodes = currentGrid.getActiveNodes();
   for (int activeNode = 0; activeNode < numberOfActiveNodes; ++activeNode)
   {
      const int node = activeNodes[ activeNode ];

      double previousTemperature = previousTemperatureGrid[node];
      double currentTemperature = currentTemperatureGrid[node];
      double temperatureChange = currentTemperature - previousTemperature;  /** Temperature change */
      double temperatureRate = temperatureChange / timeStep;    /** Temperature rate */
   
      /** Reaction Rates at previous time and current time */
      double previousRate = std::exp( a * (previousTemperature - s_referenceTemperature) );
      double currentRate  = std::exp( a * (currentTemperature - s_referenceTemperature) );

      /** Determine increase in tti between t1 and t2 */
      if (temperatureRate == 0.0) 
         m_tti[node] += previousRate * timeStep;
      else 
         m_tti[node] += (currentRate - previousRate) / (temperatureRate * a);

      assert ( !isnan( m_tti[node] ) );
   }
   
}


void VreTtiMethod::getResults( OutputGrid & outputGrid) 
{
   /** TTI corresponding to the initial value of %R_o */
   const double ttiBegin = std::exp( ( std::log (s_initialVRe ) + m_lopatinQ ) / m_lopatinP );

   const int gridSize = outputGrid.getSize();
   double * vreStorageArray = outputGrid.getVRe();

   initVectors(gridSize);
 
   for( int node = 0; node < gridSize; ++node )
   {
      double ttiSum = ttiBegin + m_tti[node];
      if (ttiSum > 0.0)
      {
         vreStorageArray[node] = std::exp( m_lopatinP * std::log( ttiSum ) - m_lopatinQ );
      }
      else
      { 
         vreStorageArray[node] = s_initialVRe;
      }

      assert( !isnan( vreStorageArray[node] ) );
   }
}
 
} //end namespace GeoPhysics

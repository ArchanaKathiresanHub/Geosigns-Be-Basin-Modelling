#include "UnitTestDataCreator.h"
#include "Constants.h"

namespace Genex6
{
UnitTestDataCreator::UnitTestDataCreator(const int   in_numberOfTimesteps,
                                         const double in_TemperatureAtStart,
                                         const double in_TemperatureAtEnd,
                                         const double in_heatingRate,
                                         const double in_temperatureGradient,
                                         const double in_surfaceTemperature,
                                         const double in_overBurderDensity,
                                         const double in_maximumPeff,
                                         const double in_PfSpecGrad,
                                         const bool  in_OpenConditions):
   m_numberOfTimesteps(in_numberOfTimesteps),
   m_TemperatureAtStart(in_TemperatureAtStart),
   m_TemperatureAtEnd(in_TemperatureAtEnd),
   m_heatingRate(in_heatingRate),
   m_temperatureGradient(in_temperatureGradient),
   m_surfaceTemperature(in_surfaceTemperature),
   m_overBurderDensity(in_overBurderDensity),
   m_maximumPeff(in_maximumPeff * Genex6::Constants::convertMpa2Pa),
   m_PfSpecGrad(in_PfSpecGrad),
   m_openConditions(in_OpenConditions)
{
   m_timeStart = 0.0;
   m_timeEnd = this->ComputeTimeAtEnd();
   m_depthAtStart = ComputeDepth(m_timeStart); //derived
   m_depthAtEnd = ComputeDepth(m_timeEnd);
   m_timestepSize = ComputeTimestepSize();
   m_burialRate = ComputeBurialRate();
}
double UnitTestDataCreator::ComputeTimeAtEnd()
{
   double timeAtEnd = (m_TemperatureAtEnd - m_TemperatureAtStart) / m_heatingRate;
   return timeAtEnd;
}
UnitTestDataCreator::~UnitTestDataCreator()
{
}
double UnitTestDataCreator::ComputeBurialRate()
{
  double burialRate = (m_depthAtEnd - m_depthAtStart) / (m_timeEnd - m_timeStart);
  return burialRate; 
}
double UnitTestDataCreator::ComputeTimestepSize()
{
   double timestepsize = (m_timeEnd - m_timeStart)/ ((double)m_numberOfTimesteps);
   return timestepsize;
}
double UnitTestDataCreator::ComputeDepth(const double in_time) const //start or end,  FunDepth(TCleg(0), TCgrad, TCsurf)
{
   double temp = ComputeTemperature(in_time);
   double ret = (temp - m_surfaceTemperature) / m_temperatureGradient;
   return ret;
}
double UnitTestDataCreator::ComputeZsource(const double in_time) const
{
   double ret = m_burialRate * in_time;
   return ret;
}

double UnitTestDataCreator::ComputePressure(const double in_time) const //FunPeff(DensOB, PeffMax, Pfract, Zsource)
{
//Effective pressure (Pa) in source rock as f(depth BML).
//PeffMax is max effective pressure '''***Excel version only?
//Diffusion model assumes organic matter is load-bearing and carrying the entire lithostatic pressure
//whereas boundary condition of source rock (interface with carrier bed or crack)
//is in equilibrium with por fluid pressure.
//This routine calculates the pressure difference between the two.

   double returnPressure = 0.0;
   double depth = ComputeDepth(in_time);
   //FunPeff = Z * DensOB * Grav * Pfract // was in Genex5
   //returnPressure = depth * m_overBurderDensity * Genex6::Constants::s_grav * m_Pfract;
   const double densWater = 1000;

   returnPressure = depth * ( m_overBurderDensity  - densWater * m_PfSpecGrad) * Genex6::Constants::s_grav;
   if (returnPressure < Genex6::Constants::s_patmos) {
      returnPressure = Genex6::Constants::s_patmos;
   }
   if (returnPressure > m_maximumPeff) {
      returnPressure = m_maximumPeff;
   }
   return returnPressure;
}
double UnitTestDataCreator::ComputeTemperature(const double in_time) const
{
   double temperature =  m_heatingRate * in_time + m_TemperatureAtStart;
   return temperature;
}

}

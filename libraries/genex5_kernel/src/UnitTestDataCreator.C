#include "UnitTestDataCreator.h"
#include "Constants.h"
namespace Genex5
{
UnitTestDataCreator::UnitTestDataCreator(const int &in_numberOfTimesteps,
                                                                 const double &in_TemperatureAtStart,
                                                                 const double &in_TemperatureAtEnd,
                                                                 const double &in_heatingRate,
                                                                 const double &in_temperatureGradient,
                                                                 const double &in_surfaceTemperature,
                                                                 const double &in_overBurderDensity,
                                                                 const double &in_maximumPeff,
                                                                 const double &in_Pfract,
                                                                 const bool &in_OpenConditions
                                                                 ):
m_numberOfTimesteps(in_numberOfTimesteps),
m_TemperatureAtStart(in_TemperatureAtStart),
m_TemperatureAtEnd(in_TemperatureAtEnd),
m_heatingRate(in_heatingRate),
m_temperatureGradient(in_temperatureGradient),
m_surfaceTemperature(in_surfaceTemperature),
m_overBurderDensity(in_overBurderDensity),
m_maximumPeff(in_maximumPeff * Genex5::Constants::convertMpa2Pa),
m_Pfract(in_Pfract),
m_openConditions(in_OpenConditions)
{
   m_timeStart = 0.0;
   m_timeEnd = this->ComputeTimeAtEnd();
   m_depthAtStart = ComputeDepth(m_timeStart);                       //derived
   m_depthAtEnd = ComputeDepth(m_timeEnd);
   m_timestepSize = ComputeTimestepSize();
   m_burialRate = ComputeBurialRate();
}
double  UnitTestDataCreator::ComputeTimeAtEnd()
{
   double timeAtEnd = 0.0;
   timeAtEnd = (m_TemperatureAtEnd - m_TemperatureAtStart) / m_heatingRate;
   return  timeAtEnd;
}
UnitTestDataCreator::~UnitTestDataCreator()
{


}
double UnitTestDataCreator::ComputeBurialRate()
{
  double burialRate = 0.0;
  burialRate = (m_depthAtEnd-m_depthAtStart) / m_timestepSize;
  return burialRate; 
}
double UnitTestDataCreator::ComputeTimestepSize()
{
   double timestepsize = 0.0;
   timestepsize = (m_timeEnd- m_timeStart)/ ((double)m_numberOfTimesteps);
   //timestepsize= (m_timeEnd- m_timeStart)/ m_numberOfTimesteps;
   return timestepsize;
}
double UnitTestDataCreator::ComputeDepth(const double & in_time) const//start or end,  FunDepth(TCleg(0), TCgrad, TCsurf)
{
   double ret = 0.0;
   double temp = ComputeTemperature(in_time);
   ret = (temp - m_surfaceTemperature) / m_temperatureGradient;
   return ret;
}
double UnitTestDataCreator::ComputePressure(const double & in_time) const//FunPeff(DensOB, PeffMax, Pfract, Zsource)
{
//Effective pressure (Pa) in source rock as f(depth BML).
//PeffMax is max effective pressure '''***Excel version only?
//Diffusion model assumes organic matter is load-bearing and carrying the entire lithostatic pressure
//whereas boundary condition of source rock (interface with carrier bed or crack)
//is in equilibrium with por fluid pressure.
//This routine calculates the pressure difference between the two.

   double returnPressure=0.0;
   double depth=ComputeDepth(in_time);
   //FunPeff = Z * DensOB * Grav * Pfract
   returnPressure = depth * m_overBurderDensity * Genex5::Constants::s_grav * m_Pfract;

   if (returnPressure < Genex5::Constants::s_patmos)
   {
      returnPressure = Genex5::Constants::s_patmos;
   }
   if (returnPressure > m_maximumPeff)
   {
      returnPressure = m_maximumPeff;
   }
   return returnPressure;
}
double UnitTestDataCreator::ComputeTemperature(const double &in_time) const
{
   double temperature = 0.0;
   temperature = m_heatingRate * in_time;
   return  temperature;
}

}

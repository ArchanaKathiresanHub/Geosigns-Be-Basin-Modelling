#ifndef OTGC_INPUT_H
#define OTGC_INPUT_H

#include <iostream>
#include <fstream>
#include <iomanip>
using namespace std;

#include "Constants.h"
namespace OTGC
{
class Input 
{
public:
   Input(const double &in_currentTime,  const double &in_temperature, const double &in_pressure);
   Input(const Input &theInput);
   
   virtual ~Input();
   double GetCurrentTime() const;
   double GetTemperatureCelsius() const;
   double GetTemperatureKelvin() const;
   double GetPressure() const;
	

   void SetTemperature(const double &in_temp);

   void OutputOnFile(ofstream &outputfile) const;

private:
   //temperature Celsius, pressure in Pa, Time in Ma
   double m_currentTime;             
   double m_temperature;
   double m_pressure;
};


inline void Input::SetTemperature(const double &in_temp)
{
   m_temperature = in_temp;
}
inline double Input::GetCurrentTime() const
{
   return  m_currentTime;
}
inline double Input::GetTemperatureCelsius() const
{
   return  m_temperature;
}
inline double Input::GetPressure() const
{
   return  m_pressure;
}
inline double Input::GetTemperatureKelvin() const
{
   return  (m_temperature + OTGC::Constants::s_TCabs);
}
inline void Input::OutputOnFile(ofstream &outputfile) const
{
     outputfile<<m_currentTime<<","<<m_temperature<<","<<m_pressure<<endl;
}


}
#endif

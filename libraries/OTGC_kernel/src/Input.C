#include "Input.h"

namespace OTGC
{
Input::Input(const double &in_currentTime,  const double &in_temperature, const double &in_pressure):
m_currentTime(in_currentTime),
m_temperature(in_temperature),
m_pressure(in_pressure)
{

}
Input::~Input()
{

}
Input::Input(const Input &theInput):
m_currentTime(theInput.m_currentTime),
m_temperature(theInput.m_temperature),
m_pressure(theInput.m_pressure)
{
  
}

}

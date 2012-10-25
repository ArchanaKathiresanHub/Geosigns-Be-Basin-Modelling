#ifndef NODEINPUT_H
#define NODEINPUT_H

#include <fstream>
#include<iomanip>
using namespace std;


namespace Calibration
{
class NodeInput 
{
public:
   NodeInput(const double &in_currentTime = 0.0,  const double &in_temperature = 0.0):
	m_referenceTime(in_currentTime),
	m_temperature(in_temperature)
	{
	
	}
   ~NodeInput()
	{

	}
   double getReferenceTime() const;
   double getTemperatureCelsius() const;
   double getTemperatureKelvin() const;

	void setTemperature(const double &in_temp);

	class AscendingTimeOrderInput
	{
	public:
		bool operator()(NodeInput *left, NodeInput *right)
		{
			return (left->getReferenceTime() < right->getReferenceTime()); 
		}
	}; 
	class DescendingTimeOrderInput
	{
	public:
		bool operator()(NodeInput *left, NodeInput *right)
		{
			return (left->getReferenceTime() > right->getReferenceTime()); 
		}
	}; 
 
private:
   //temperature Celsius, Time in Ma
   double m_referenceTime;             
   double m_temperature;

	static const double s_KelvinConst = 273.15;

	friend ostream &
	operator<<(ostream & os, NodeInput &theInput);

};
ostream & operator<<(ostream & os, NodeInput &theInput)
{
	return os<<theInput.m_currentTime<<" "<<theInput.m_temperature();
}

inline void NodeInput::setTemperature(const double &in_temp)
{
   m_temperature = in_temp;
}
inline double NodeInput::getReferenceTime() const
{
   return  m_currentTime;
}
inline double NodeInput::getTemperatureCelsius() const
{
   return  m_temperature;
}
inline double NodeInput::getTemperatureKelvin() const
{
   return  (m_temperature + s_KelvinConst);
}
class LessThanByTimeNodeInputPtr
{
public:
	bool operator()(NodeInput *left, NodeInput *right)
	{
		return (left->getReferenceTime() < right->getReferenceTime()); 
	}
}; 



}
#endif

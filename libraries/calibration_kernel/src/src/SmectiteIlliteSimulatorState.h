#ifndef SIMULATORSTATE_H
#define SIMULATORSTATE_H


#include <string>
#include <map>

#include<iostream>
#include <fstream>
#include<iomanip>
using namespace std;

namespace Calibration
{

class SmectiteIlliteSimulatorState
{
public:
   SmectiteIlliteSimulatorState(const double referenceTime = 0.0);
   ~SmectiteIlliteSimulatorState();
 
	 enum StateVariableId
   { 
      STOTAL = 0,               
      TOTAL1 = 1,
      TOTAL2 = 2,
      TOTALX = 3,
    	NUMOFSTATEVARIABLES = 4
   };
	
	void getStateVariables(double theStateVariables[]) const;
 	void setStateVariables(double newStateVariables[]); 

 	void setReferenceTime(const double &inTime); 
 	double getReferenceTime() const;

	double getReferenceTemperature() const;
	void setReferenceTemperature(const double &inTemperature); 
	bool isInitialized() const;
	void setAsInitialized();
	void setAsNotInitialized();
	SmectiteIlliteSimulatorState & operator=(const SmectiteIlliteSimulatorState &inValue)
	{
		if(this != &inValue)	
		{
	   	m_referenceTime 			= inValue.m_referenceTime;
	   	m_referenceTemperature 	= inValue.m_referenceTemperature;
			m_initialized 				= inValue.m_initialized;    
			for(int i = 0; i < NUMOFSTATEVARIABLES; i++)
   		{
				m_stateVariables[ i ] = inValue.m_stateVariables[ i ];
   		}
		}
		return *this;
	}
	SmectiteIlliteSimulatorState(const SmectiteIlliteSimulatorState &inValue)
	{
	   m_referenceTime 			= inValue.m_referenceTime;
	   m_referenceTemperature 	= inValue.m_referenceTemperature;
		m_initialized 				= inValue.m_initialized;    
		for(int i = 0; i < NUMOFSTATEVARIABLES; i++)
   	{
			m_stateVariables[ i ] = inValue.m_stateVariables[ i ];
   	}
	}

private:
   double m_referenceTime;  
	double m_referenceTemperature; 
	bool m_initialized;     
   double m_stateVariables[NUMOFSTATEVARIABLES];
};
inline void SmectiteIlliteSimulatorState::setReferenceTime(const double &inTime) 
{
   m_referenceTime = inTime;
}
inline double SmectiteIlliteSimulatorState::getReferenceTime() const 
{
   return m_referenceTime;
}
inline double SmectiteIlliteSimulatorState::getReferenceTemperature() const
{
	return m_referenceTemperature;
}
inline void SmectiteIlliteSimulatorState::setReferenceTemperature(const double &inTemperature) 
{
	m_referenceTemperature = inTemperature;
}  
inline bool SmectiteIlliteSimulatorState::isInitialized() const
{
	return m_initialized;
}
inline void SmectiteIlliteSimulatorState::setAsInitialized()
{
	m_initialized = true;
}
inline void SmectiteIlliteSimulatorState::setAsNotInitialized()
{
	m_initialized = false;
}


}
#endif

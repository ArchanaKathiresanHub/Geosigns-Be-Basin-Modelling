#ifndef BIOMARKERSSIMULATORSTATE_H
#define BIOMARKERSSIMULATORSTATE_H


#include <string>
#include <map>

#include<iostream>
#include <fstream>
#include<iomanip>
using namespace std;

namespace Calibration
{

class BiomarkersSimulatorState
{
public:
	enum StateVariableId
   { 
      HOPANEISOINTEGRAL = 0,  				 
      STERANEISOINTEGRAL = 1,
      STERANEAROINTEGRAL = 2,
    	NUMOFSTATEVARIABLES = 3
   };

   BiomarkersSimulatorState():
	m_referenceTime(0.0),
	m_referenceTemperature(0.0),
	m_initialized(false)
	{
		for(int i = 0; i < NUMOFSTATEVARIABLES; ++i)
		{
			m_stateVariables[i] = 0.0;
		}
	}
   ~BiomarkersSimulatorState()
	{	

	}
 	void incrementStateVariables(double StateVariablesIncrement[])
	{
		for(int i = 0; i < NUMOFSTATEVARIABLES; ++i)
		{
			m_stateVariables[i] += StateVariablesIncrement[i];
		}
	} 
	void getStateVariables(double StateVariablesIncrement[]) const
	{
		for(int i = 0; i < NUMOFSTATEVARIABLES; ++i)
		{
			StateVariablesIncrement[i] = m_stateVariables[i];
		}
	} 
 	void setReferenceTime(const double &inTime)
	{
  		 m_referenceTime = inTime;
	}
 	double getReferenceTime() const
	{
		return m_referenceTime;
	}
	double getReferenceTemperature() const
	{
		return m_referenceTemperature;
	}
	void setReferenceTemperature(const double &inTemperature)
	{
		m_referenceTemperature = inTemperature;
	}	   
	bool isInitialized() const
	{
		return m_initialized;
	}
	void setAsInitialized()
	{
		m_initialized = true;
	}
	void setAsNotInitialized()
	{
		m_initialized = false;
	}
	BiomarkersSimulatorState & operator=(const BiomarkersSimulatorState &inValue)
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
	BiomarkersSimulatorState(const BiomarkersSimulatorState &inValue)
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

}
#endif

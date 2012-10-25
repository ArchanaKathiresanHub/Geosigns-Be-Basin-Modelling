#include "SmectiteIlliteSimulatorState.h"

namespace Calibration
{

SmectiteIlliteSimulatorState::SmectiteIlliteSimulatorState(const double referenceTime):
m_initialized(false),
m_referenceTime(referenceTime),
m_referenceTemperature(0.0)
{
   for(int i = 0 ;i < NUMOFSTATEVARIABLES; i++)
   {
		m_stateVariables[ i ] = 0.0;
   }
}
SmectiteIlliteSimulatorState::~SmectiteIlliteSimulatorState()
{
 
}
void SmectiteIlliteSimulatorState::getStateVariables(double theStateVariables[]) const
{
 	for(int i = 0 ;i < NUMOFSTATEVARIABLES; i++)
   {
		theStateVariables[ i ] = m_stateVariables[ i ];
   }
}
void SmectiteIlliteSimulatorState::setStateVariables(double newStateVariables[])
{
	for(int i = 0 ;i < NUMOFSTATEVARIABLES; i++)
   {
		m_stateVariables[ i ] = newStateVariables[ i ];
   }

}

}

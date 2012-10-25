#ifndef SMECTITEILLITE_SIMULATOR_H
#define SMECTITEILLITE_SIMULATOR_H

#include <vector>

namespace Calibration
{
class NodeInput;
class SmectiteIlliteOutput;
class SmectiteIlliteSimulatorState;

class SmectiteIlliteSimulator
{
public:
//yiannis
   SmectiteIlliteSimulator( const double &in_ActEnergy1   = 55800.0,  
									const double &in_FreqFactor1  = 3.4229e+06,
									const double &in_ActEnergy2   = 13700.0,  
									const double &in_FreqFactor2  = 1.0,
									const double in_InitIlliteFraction = 0.0,
									const double &in_timeStepSize = 0.5);
					
   ~SmectiteIlliteSimulator();

	void initializeState(const NodeInput &theInput,	SmectiteIlliteSimulatorState &NodeSimulatorState) const;
	void advanceState(const std::vector<NodeInput*> &Input, SmectiteIlliteSimulatorState &NodeSimulatorState) const;
	void advanceState(const NodeInput &theInput, SmectiteIlliteSimulatorState &NodeSimulatorState) const;
   void computeOutput(const SmectiteIlliteSimulatorState &NodeSimulatorState, SmectiteIlliteOutput &theOutput) const ;

protected:

	void computeStateVariables(const double &newReferencetemperature, const double &newReferenceTime, SmectiteIlliteSimulatorState &NodeSimulatorState) const;

private:

   const double m_ActEnergy1;  
	const double m_FreqFactor1;
	const double m_ActEnergy2;  
	const double m_FreqFactor2;
	const double m_InitIlliteFraction;	  
	const double m_timeStepSize;

	static const double s_GASCONSTANT = 8.3144;
	static const double s_SECS_IN_MA  = 3.15576e13; // sec/MA 
};
}
#endif

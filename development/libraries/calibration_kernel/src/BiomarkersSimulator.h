#ifndef BIOMARKERS_SIMULATOR_H
#define BIOMARKERS_SIMULATOR_H


#include "BiomarkersUtilities.h"

namespace Calibration
{
class NodeInput;
class BiomarkersOutput;
class BiomarkersSimulatorState;

class BiomarkersSimulator
{
public:

   BiomarkersSimulator( const double &HopIsoactenergy  = 168000.000000,
								const double &SteIsoactenergy  = 169000.000000,
								const double &SteAroactenergy  = 181400.000000,
								const double &HopIsofreqfactor = 810000000.000000,
								const double &SteIsofreqfactor = 486000000.000000,
								const double &SteArofreqfactor = 48499998720.000000,
								const double &HopIsogamma  	 = 1.564000,
								const double &SteIsogamma  	 = 1.174000,
								const double &in_timeStepSize  = 0.5):
	m_Integrator(),
	m_HopaneIsoKineticsFunction(HopIsoactenergy),
	m_SteraneIsoKineticsFunction(SteIsoactenergy),
	m_SteraneAroKineticsFunction(SteAroactenergy),
	m_SteraneAromatizationFunction(SteArofreqfactor),
	m_SteraneIsomerizationFunction(SteIsogamma, SteIsofreqfactor),
	m_HopaneIsomerizationFunction(HopIsogamma, HopIsofreqfactor),
	m_timeStepSize(in_timeStepSize)
	{


	}				
   ~BiomarkersSimulator()
	{

	}

	void initializeState(const NodeInput &theInput,	BiomarkersSimulatorState &NodeSimulatorState) const;
	void advanceState(const NodeInput &theInput, BiomarkersSimulatorState &NodeSimulatorState) const;
   void computeOutput(const BiomarkersSimulatorState &NodeSimulatorState, BiomarkersOutput &theOutput) const;

protected:

	void computeStateVariables(const double &temperatureEnd, const double &timeEnd, BiomarkersSimulatorState &NodeSimulatorState ) const;

private:
	const TrapezoidalIntegrator m_Integrator;
	const BiomarkerKineticsFunction m_HopaneIsoKineticsFunction	;
	const BiomarkerKineticsFunction m_SteraneIsoKineticsFunction;
	const BiomarkerKineticsFunction m_SteraneAroKineticsFunction;	
	const AromatizationFunction 	  m_SteraneAromatizationFunction;	
	const IsomerizationFunction 	  m_SteraneIsomerizationFunction;
	const IsomerizationFunction 	  m_HopaneIsomerizationFunction;
	const double m_timeStepSize;

	static const double s_GASCONSTANT = 8.3144;
	static const double s_SECS_IN_MA  = 3.15576e13; // sec/MA 
};

}
#endif

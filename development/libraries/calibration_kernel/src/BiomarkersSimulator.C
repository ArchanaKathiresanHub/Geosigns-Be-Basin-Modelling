//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 
#include "BiomarkersSimulator.h"
#include "NodeInput.h"
#include "BiomarkersSimulatorState.h"
#include "LinearInterpolator.h"
#include "BiomarkersOutput.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <cmath>

namespace Calibration
{
   // Declare some static variables
   const double BiomarkersSimulator::s_GASCONSTANT = 8.3144;
   const double BiomarkerKineticsFunction::GASCONSTANT = 8.3144;

   // Little silly since all these variables are constants and the same
   const double AromatizationFunction::s_MillionYearToSecond = 3.15576e13; // sec/MA 
   const double BiomarkersSimulator::s_MillionYearToSecond = 3.15576e13; // sec/MA 
   const double IsomerizationFunction::s_MillionYearToSecond = 3.15576e13; // sec/MA 

void BiomarkersSimulator::initializeState(const NodeInput &theInput, BiomarkersSimulatorState &NodeSimulatorState) const
{
   NodeSimulatorState.setReferenceTime(theInput.getReferenceTime());
   NodeSimulatorState.setReferenceTemperature(theInput.getTemperatureKelvin());
   NodeSimulatorState.setAsInitialized();
}

void BiomarkersSimulator::computeStateVariables(const double &temperatureEnd, const double &timeEnd, BiomarkersSimulatorState &NodeSimulatorState ) const
{
   static double StateVariableIncrement[BiomarkersSimulatorState::NUMOFSTATEVARIABLES];

   double timestepSize = NodeSimulatorState.getReferenceTime() - timeEnd;
   double temperatureBegin = NodeSimulatorState.getReferenceTemperature();

   StateVariableIncrement[BiomarkersSimulatorState::HOPANEISOINTEGRAL]  = m_Integrator( m_HopaneIsoKineticsFunction,  
                                                                                        timestepSize, 
                                                                                        temperatureBegin, 
                                                                                        temperatureEnd);
   StateVariableIncrement[BiomarkersSimulatorState::STERANEISOINTEGRAL] = m_Integrator( m_SteraneIsoKineticsFunction, 
                                                                                        timestepSize, 
                                                                                        temperatureBegin, 
                                                                                        temperatureEnd);
   StateVariableIncrement[BiomarkersSimulatorState::STERANEAROINTEGRAL] = m_Integrator( m_SteraneAroKineticsFunction, 
                                                                                        timestepSize, 
                                                                                        temperatureBegin, 
                                                                                        temperatureEnd);

   
   NodeSimulatorState.incrementStateVariables(StateVariableIncrement);
   NodeSimulatorState.setReferenceTime(timeEnd);
   NodeSimulatorState.setReferenceTemperature(temperatureEnd);
   
}

void BiomarkersSimulator::advanceState(const NodeInput &theInput, BiomarkersSimulatorState &NodeSimulatorState) const
{
   double newReferenceTime     = theInput.getReferenceTime();
   double currentReferenceTime = NodeSimulatorState.getReferenceTime();

   double computationInterval  = currentReferenceTime - newReferenceTime; //assuming decreasing order in time...

   double newReferencetemperature = theInput.getTemperatureKelvin();
   double currentReferencetemperature = NodeSimulatorState.getReferenceTemperature();

   if(NodeSimulatorState.isInitialized())
   {
      //This if statement might not be needed, check if feasible to decouple from the timestepsize
      if(computationInterval > m_timeStepSize)
      {
         double temperature = 0.0;

         LinearInterpolator theInterpolator(currentReferenceTime, currentReferencetemperature, newReferenceTime, newReferencetemperature);

         currentReferenceTime -= m_timeStepSize;

         while(currentReferenceTime > newReferenceTime)
         {
            temperature = theInterpolator.evaluate(currentReferenceTime);

            computeStateVariables( temperature,currentReferenceTime, NodeSimulatorState );

            currentReferenceTime -= m_timeStepSize;
         }
      }
      computeStateVariables( newReferencetemperature, newReferenceTime, NodeSimulatorState );
   }
   else
   {
      initializeState(theInput, NodeSimulatorState);
      //cout<<"Warning: Initializing state"<<endl;
   }
}
void BiomarkersSimulator::computeOutput(const BiomarkersSimulatorState &NodeSimulatorState,BiomarkersOutput &theOutput) const
{
   static double SimulatorStateVariables[BiomarkersSimulatorState::NUMOFSTATEVARIABLES];

   NodeSimulatorState.getStateVariables(SimulatorStateVariables);

   theOutput.setHopaneIsomerisation  ( m_HopaneIsomerizationFunction ( SimulatorStateVariables[BiomarkersSimulatorState::HOPANEISOINTEGRAL]  ) );
   theOutput.setSteraneIsomerisation ( m_SteraneIsomerizationFunction( SimulatorStateVariables[BiomarkersSimulatorState::STERANEISOINTEGRAL] ) );
   theOutput.setSteraneAromatisation ( m_SteraneAromatizationFunction( SimulatorStateVariables[BiomarkersSimulatorState::STERANEAROINTEGRAL] ) );
}

}




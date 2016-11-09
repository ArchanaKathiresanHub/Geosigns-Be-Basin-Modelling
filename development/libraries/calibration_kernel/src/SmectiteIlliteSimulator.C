//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 
#include "SmectiteIlliteSimulator.h"
#include "NodeInput.h"
#include "SmectiteIlliteSimulatorState.h"
#include "LinearInterpolator.h"
#include "SmectiteIlliteOutput.h"

// std library
#include <iostream>
#include <fstream>
#include <iomanip>
#include <cmath>

// utilities library
#include "ConstantsPhysics.h"
using Utilities::Physics::IdealGasConstantCalibration;


namespace Calibration
{

SmectiteIlliteSimulator:: SmectiteIlliteSimulator( const double &in_ActEnergy1,  
                                    const double &in_FreqFactor1,
                                    const double &in_ActEnergy2,  
                                    const double &in_FreqFactor2,   
                                    const double in_InitIlliteFraction,  
                                    const double &in_timeStepSize):
   m_ActEnergy1(in_ActEnergy1),  
   m_FreqFactor1(in_FreqFactor1),
   m_ActEnergy2(in_ActEnergy2),  
   m_FreqFactor2(in_FreqFactor2),   
   m_InitIlliteFraction(in_InitIlliteFraction),    
   m_timeStepSize(in_timeStepSize)
{
}
SmectiteIlliteSimulator::~SmectiteIlliteSimulator()
{

}
void SmectiteIlliteSimulator::initializeState(const NodeInput &theInput, SmectiteIlliteSimulatorState &NodeSimulatorState) const
{
   
   NodeSimulatorState.setReferenceTime(theInput.getReferenceTime());
   NodeSimulatorState.setReferenceTemperature(theInput.getTemperatureKelvin());
   NodeSimulatorState.setAsInitialized();
}
void SmectiteIlliteSimulator::computeStateVariables(const double &temperatureEnd, const double &timeEnd, SmectiteIlliteSimulatorState &NodeSimulatorState) const
{
   using std::exp;

   static double SimulatorStateVariables[SmectiteIlliteSimulatorState::NUMOFSTATEVARIABLES];

   NodeSimulatorState.getStateVariables(SimulatorStateVariables);

   double temperature = 0.5 *(temperatureEnd + NodeSimulatorState.getReferenceTemperature());
   double timestepSize = NodeSimulatorState.getReferenceTime() - timeEnd;

   double rate1 = exp (-m_ActEnergy1 / (IdealGasConstantCalibration * temperature));
   double rate2 = exp (-m_ActEnergy2 / (IdealGasConstantCalibration * temperature));

   SimulatorStateVariables[SmectiteIlliteSimulatorState::STOTAL] += timestepSize * (m_FreqFactor2 * rate2 - m_FreqFactor1 * rate1);
   SimulatorStateVariables[SmectiteIlliteSimulatorState::TOTAL1] += rate1 * timestepSize * exp (SimulatorStateVariables[SmectiteIlliteSimulatorState::STOTAL]);
   SimulatorStateVariables[SmectiteIlliteSimulatorState::TOTAL2] += rate2 * timestepSize;
   SimulatorStateVariables[SmectiteIlliteSimulatorState::TOTALX] += rate1 * timestepSize;

   NodeSimulatorState.setStateVariables(SimulatorStateVariables);
   NodeSimulatorState.setReferenceTime(timeEnd);
   NodeSimulatorState.setReferenceTemperature(temperatureEnd);
   
}
void SmectiteIlliteSimulator::advanceState(const std::vector<NodeInput*> &Input, SmectiteIlliteSimulatorState &NodeSimulatorState) const
{
   std::vector<NodeInput*>::const_iterator it = Input.begin();
   std::vector<NodeInput*>::const_iterator itEnd = Input.end();
   while(it != itEnd)
   {
      advanceState(*(*it++), NodeSimulatorState);
   }
}
void SmectiteIlliteSimulator::advanceState(const NodeInput &theInput, SmectiteIlliteSimulatorState &NodeSimulatorState) const
{
   double currentReferenceTime          = NodeSimulatorState.getReferenceTime();
   double currentReferencetemperature    = NodeSimulatorState.getReferenceTemperature();

   double newReferenceTime              = theInput.getReferenceTime();
   double newReferencetemperature       = theInput.getTemperatureKelvin();

   double computationInterval           = currentReferenceTime - newReferenceTime; //assuming decreasing order in time...
   //cout<<"Interval :"<<currentReferenceTime<<" "<<computationInterval<<"timestepSize"<<m_timeStepSize<<endl;

   if(NodeSimulatorState.isInitialized())
   {
      //Check...
      if(computationInterval > m_timeStepSize)
      {
         double temperature = 0.0;

         LinearInterpolator theInterpolator(currentReferenceTime, currentReferencetemperature, newReferenceTime, newReferencetemperature);

         currentReferenceTime -= m_timeStepSize;

         while(currentReferenceTime > newReferenceTime)
         {
            temperature = theInterpolator.evaluate(currentReferenceTime);
            //cout<<"In the interpolated :"<<currentReferenceTime<<" "<<temperature<<endl;
            computeStateVariables( temperature, currentReferenceTime, NodeSimulatorState );

            currentReferenceTime -= m_timeStepSize;
         }
      }
      //cout<<"Using Original :"<<newReferencetemperature<<" "<<newReferencetemperature<<endl;
      computeStateVariables( newReferencetemperature,newReferenceTime, NodeSimulatorState);
   }
   else
   {
      initializeState(theInput, NodeSimulatorState);
   }
}
void SmectiteIlliteSimulator::computeOutput(const SmectiteIlliteSimulatorState &NodeSimulatorState, SmectiteIlliteOutput &theOutput) const
{
   using std::exp;
   static double SimulatorStateVaribles[SmectiteIlliteSimulatorState::NUMOFSTATEVARIABLES];

   NodeSimulatorState.getStateVariables(SimulatorStateVaribles);

   double totx = exp (-m_FreqFactor1 * SimulatorStateVaribles[SmectiteIlliteSimulatorState::TOTALX]);
   double toty = m_FreqFactor1 * 
                  SimulatorStateVaribles[SmectiteIlliteSimulatorState::TOTAL1] * 
                  exp (-m_FreqFactor2 * SimulatorStateVaribles[SmectiteIlliteSimulatorState::TOTAL2]);
   double smi = m_InitIlliteFraction + (1.0 - m_InitIlliteFraction) * (1.0 - (totx + 0.4 * (toty)));

   theOutput.setIlliteTransfRatio( smi );
}

}

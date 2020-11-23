//
// Copyright (C) 2015-2017 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include "SourceRockNode.h"
#include "Input.h"
#include "SimulatorState.h"
#include "Simulator.h"
#include "ChemicalModel.h"
#include "Utilities.h"
#include "UnitTestDataCreator.h"
#include "SpeciesResult.h"
#include "Species.h"

// std library
#include <math.h>
#include <cstdlib>

// utilities library
#include "ConstantsMathematics.h"
#include "ConstantsNumerical.h"
#include "StringHandler.h"

// CBMGenerics library
#include "ComponentManager.h"

using Utilities::Maths::CelciusToKelvin;
using Utilities::Numerical::CauldronNoDataValue;
typedef CBMGenerics::ComponentManager::SpeciesNamesId ComponentId;

namespace Genex6
{

SourceRockNode::SourceRockNode(const double in_thickness, const double in_TOC,
                               const double in_InorganicDensity,
                               const double in_f1, const double in_f2,
                               const int in_I, const int in_J):
   m_thickness(in_thickness),
   m_TOCi(in_TOC),
   m_InorganicDensity(in_InorganicDensity),
   m_I(in_I),
   m_J(in_J),
   m_f1 (in_f1),
   m_f2 (in_f2),
   m_currentState(0)
{
   m_mixedSimulatorState = 0;
}
SourceRockNode::~SourceRockNode()
{
   clearInputHistory();
   ClearOutputHistory();

   ClearSimulatorStates();
   m_ConcKi.clear();

   delete m_mixedSimulatorState;
}

void SourceRockNode::initialise () {

   if ( m_currentState != 0 ) {
      delete m_currentState;
      m_currentState = 0;
   }

   m_ConcKi.clear ();

   clearInputHistory ();
   ClearOutputHistory ();
}

SimulatorState* SourceRockNode::GetSimulatorState( int id ) const
{
   return m_theSimulatorStates[id];
}
SimulatorState &SourceRockNode::GetMixedSimulatorState() const
{
   return *m_mixedSimulatorState;
}
SimulatorState &SourceRockNode::getPrincipleSimulatorState () const
{
   return ( m_mixedSimulatorState != 0 ? * m_mixedSimulatorState : * m_currentState );
}

void SourceRockNode::clearInputHistory()
{
   std::vector<Input*>::iterator itEnd  = m_theInput.end();
   for(std::vector<Input*>::iterator it = m_theInput.begin(); it != itEnd; ++ it) {
      delete (*it);
   }
   m_theInput.clear();
}
void SourceRockNode::ClearOutputHistory()
{
   std::vector<SimulatorState*>::iterator itEnd  = m_theOutput.end();
   for(std::vector<SimulatorState*>::iterator it = m_theOutput.begin(); it != itEnd; ++ it) {
      delete (*it);
   }
   m_theOutput.clear();
}
void SourceRockNode::AddOuput(SimulatorState* in_theOuput)
{
   m_theOutput.push_back(in_theOuput);
}
void SourceRockNode::AddInput(Input* in_theInput)
{
   m_theInput.push_back(in_theInput);
}
void SourceRockNode::ClearSimulatorStates()
{
   std::vector<SimulatorState*>::iterator itEnd  = m_theSimulatorStates.end();
   for(std::vector<SimulatorState*>::iterator it = m_theSimulatorStates.begin(); it != itEnd; ++ it) {
      delete (*it);
   }
   m_theSimulatorStates.clear();
}
void SourceRockNode::AddSimulatorState(SimulatorState* in_theSimulatorState)
{
   m_theSimulatorStates.push_back(in_theSimulatorState);
}

const Input *SourceRockNode::getLastInput() const
{
   Input *ret = 0;

   if(!m_theInput.empty())
   {
      ret = m_theInput[m_theInput.size() - 1];
   }

   return ret;
}

bool SourceRockNode::RequestComputation ( int numberOfSourceRock, Simulator & theSimulator )
{
   SimulatorState * theState = ( numberOfSourceRock < ( static_cast<int>( m_theSimulatorStates.size() )) ?  m_theSimulatorStates[numberOfSourceRock] : 0 );

   std::vector<Input*>::iterator itInput;
   bool isInitialTimeStep = true;

   for(itInput = m_theInput.begin(); itInput != m_theInput.end(); ++ itInput) {

      Input* theInput = *itInput;

      if(theState) {
         const double thicknessScale = theInput->GetThicknessScaleFactor() *  m_thickness;

         theState->SetConckiThickness( m_ConcKi[numberOfSourceRock], thicknessScale);
         theSimulator.SetSimulatorState( theState );
         theSimulator.advanceSimulatorState( *theInput );
         theSimulator.SetSimulatorState(0);

         theState->PostProcessTimeStepComputation ();

         isInitialTimeStep = false;
      } else {
         CreateSimulatorState(numberOfSourceRock, (*itInput)->GetTime(), theSimulator);
         theSimulator.initializeSimulatorState( *theInput );
         theSimulator.SetSimulatorState(0);
      }
   }

   return isInitialTimeStep;
}


void SourceRockNode::updateAdsorptionOutput ( const AdsorptionSimulator& adsorptionSimulator ) {
   /*
   const SpeciesManager& speciesManager = genexSimulator.getSpeciesManager ();

   using namespace CBMGenerics;

   int speciesIndex;

   for ( speciesIndex = 1; speciesIndex <= speciesManager.getNumberOfSpecies (); ++speciesIndex ) {
      ComponentManager::SpeciesNamesId species = speciesManager.mapIdToComponentManagerSpecies ( speciesIndex );

      if ( species != ComponentManager::UNKNOWN ) {

         if ( adsorptionSimulator.speciesIsSimulated ( ComponentManager::SpeciesNamesId ( species ))) {

            const Genex6::SpeciesState* speciesState = m_currentState->GetSpeciesStateById ( speciesIndex );
            Genex6::SpeciesResult& result = m_currentState->GetSpeciesResult ( speciesIndex );

            result.setAdsorpedMol ( speciesState->getAdsorpedMol ());
            result.setFreeMol ( speciesState->getFreeMol ());
            result.setExpelledMol ( speciesState->getExpelledMol ());
         }

      }

   }
   */

   SimulatorState &theSimulatorState = getPrincipleSimulatorState ();
   const SpeciesManager * speciesManager = theSimulatorState.getSpeciesManager ();
   using namespace CBMGenerics;

   int speciesIndex;

   for ( speciesIndex = 1; speciesIndex <= speciesManager->getNumberOfSpecies (); ++speciesIndex ) {
      ComponentManager::SpeciesNamesId species = speciesManager->mapIdToComponentManagerSpecies ( speciesIndex );

      if ( species != ComponentManager::UNKNOWN ) {

         if ( adsorptionSimulator.speciesIsSimulated ( ComponentManager::SpeciesNamesId ( species ))) {

            const Genex6::SpeciesState* speciesState = theSimulatorState.GetSpeciesStateById ( speciesIndex );
            Genex6::SpeciesResult& result = theSimulatorState.GetSpeciesResult ( speciesIndex );

            result.setAdsorpedMol ( speciesState->getAdsorpedMol ());
            result.setFreeMol ( speciesState->getFreeMol ());
            result.setExpelledMol ( speciesState->getExpelledMol ());
         }

      }

   }

}

int SourceRockNode::CreateSimulatorState(int numberOfSourceRock, const double currentTime, Simulator & theSimulator)
{
   SimulatorState * theState = new SimulatorState( &(theSimulator.getChemicalModel ().getSpeciesManager()), theSimulator.getChemicalModel ().GetNumberOfSpecies(),  currentTime );
   theState->setImmobileSpecies( theSimulator.getChemicalModel ().getImmobileSpecies() );

   assert( theState != 0 );

   if(( static_cast<int>( m_theSimulatorStates.size() )) != numberOfSourceRock ) {
      cout<<"Basin_Fatal: Number of Source Rock is wrong in CreateSimulatorState...Aborting...";
      return FAIL;
   }
   AddSimulatorState( theState );

   double concki1 = theSimulator.ComputeNodeInitialOrganicMatterDensity(m_TOCi, m_InorganicDensity);
   m_ConcKi.push_back( concki1 );

   theState->SetConckiThickness(concki1, m_thickness);
   theState->setInitialToc (m_TOCi);

   theSimulator.SetSimulatorState(theState);

   if( numberOfSourceRock == 0 ) {
      m_currentState = theState; // m_currentState = m_theSimulatorStates[0]
   }
   return SUCCESS;
}

bool SourceRockNode::RequestMixing( ChemicalModel * aModel ) {

   if(m_mixedSimulatorState == 0) {
      if( m_theInput.empty() || m_theInput.size() != 1 ) return false;
      // Because numberOfSpecies could be different, we need to allocate mixing SimulatorState for the biggest numberOfSpecies
      m_mixedSimulatorState = new SimulatorState( &aModel->getSpeciesManager(), aModel->GetNumberOfSpecies(), m_theInput[0]->GetTime() );
      //cout << "Number of Species in MixingManager " << aModel->GetNumberOfSpecies() << endl;
      assert ( m_mixedSimulatorState != 0 );
      m_mixedSimulatorState->setInitialToc (m_TOCi);
      m_mixedSimulatorState->setImmobileDensitiesMixed( m_currentState, m_theSimulatorStates[1], m_f1, m_f2 );
   }

   m_mixedSimulatorState->SetLumpedConcentrationsToZero();
   m_mixedSimulatorState->SetResultsToZero();
   double thicknessScale = m_theInput[0]->GetThicknessScaleFactor() *  m_thickness;
   // Mix 2 SourceRocks. Do loop through all SimulatorStates to mix more.
   m_mixedSimulatorState->SetConckiThickness(m_f1 * m_ConcKi[0] + m_f2 * m_ConcKi[1], thicknessScale);
   m_mixedSimulatorState->PostProcessTimeStepComputation( m_currentState, m_theSimulatorStates[1], m_f1, m_f2 );

   return true;
}

void SourceRockNode::RequestComputation(Simulator & theSimulator)
{
   // SR rock mixing is not supported

   std::vector<Input*>::iterator itInput;

   for(itInput = m_theInput.begin(); itInput != m_theInput.end(); ++ itInput) {

      Input* theInput = *itInput;

      if(m_currentState) {
         const double thicknessScale = theInput->GetThicknessScaleFactor() * m_thickness;

         m_currentState->SetConckiThickness(m_ConcKi[0], thicknessScale);
         theSimulator.SetSimulatorState( m_currentState );
         theSimulator.advanceSimulatorState( *theInput );
         theSimulator.SetSimulatorState(0);

         m_currentState->PostProcessTimeStepComputation();
      }
      else
      {
         CreateSimulatorState(0, (*itInput)->GetTime(), theSimulator);
         theSimulator.initializeSimulatorState ( *theInput );
         theSimulator.SetSimulatorState(0);
      }
      SimulatorState *theStateOutput = new SimulatorState(*m_currentState);
      AddOuput(theStateOutput);
   }
}

// To be deprecated in BPA/Cauldron. 1D simulation is taken over by fastgenex5 as well so the only valid
// Genex6::SourceRockNode computation interface
// will be RequestComputation::(Simulator &)
int SourceRockNode::RequestComputation1D(int numberOfSourceRock, Simulator *theSimulator, double snapshots[],
                                         const int numberOfSnapshots, const double depositionAge)
{

   SimulatorState * theState = ( numberOfSourceRock < static_cast<int>( m_theSimulatorStates.size() ) ?  m_theSimulatorStates[numberOfSourceRock] : 0 );

   cout<<"1D MODE-GENEX5KERNEL"<<endl;
   if(m_theInput.size() > static_cast<unsigned int>(numberOfSnapshots)) {
      cout<<"Basin_Fatal: Input sizes are incompatible...Aborting...";
      return FAIL;
   }
   if(theSimulator == 0) {
      cout<<"Basin_Fatal: Invalid Simulator...Aborting...";
      return FAIL;
   }

   int numberOfIterations = static_cast<int>(m_theInput.size());

   std::vector<Input*>::iterator intervalStartInput = m_theInput.begin();

   CreateSimulatorState(numberOfSourceRock, (*intervalStartInput)->GetTime(), *theSimulator);
   // Is this correct to initialise simulator with the first input node?
   theSimulator->initializeSimulatorState( *m_theInput [ 0 ]);
   theSimulator->SetSimulatorState(0);

   double timeStepSize = theSimulator->GetMaximumTimeStepSize(depositionAge);
   double t = snapshots[0] - timeStepSize;

   for(int i = 1; i < numberOfIterations; ++ i) {
      Input *intervalBeginInput = m_theInput[i - 1];
      Input *intervalEndInput   = m_theInput[i];

      while(t > snapshots[i]) {
         Input * TimeInstanceInput = new Input(t, (*intervalBeginInput), (*intervalEndInput));

         const double thicknessScale = TimeInstanceInput->GetThicknessScaleFactor() * m_thickness;

         m_currentState->SetConckiThickness( m_ConcKi[numberOfSourceRock], thicknessScale);
         theSimulator->SetSimulatorState(theState);
         theSimulator->advanceSimulatorState((*TimeInstanceInput));
         theSimulator->SetSimulatorState(0);

         m_currentState->PostProcessTimeStepComputation ();

         delete TimeInstanceInput;

         t -= timeStepSize;
      }
      m_currentState->SetConckiThickness(m_ConcKi[numberOfSourceRock], m_thickness);
      theSimulator->SetSimulatorState(theState);
      theSimulator->advanceSimulatorState((*intervalEndInput));
      theSimulator->SetSimulatorState(0);

      m_currentState->PostProcessTimeStepComputation ();
   }
   return SUCCESS;
}

void SourceRockNode::CreateInputPTHistory(const UnitTestDataCreator &theUnitTestDataCreator)
{
   int numberOfTimesteps = theUnitTestDataCreator.GetNumberOfTimesteps();
   double dT = theUnitTestDataCreator.GetTimeStepSize();

   double currentTime = 0.0;
   double currentTemperatute = 0.0;
   double currentPressure = 0.0;

   for(int i = 0; i <= numberOfTimesteps; ++ i) {   //for compatibility with VBA code
      if(i == 0) {
         currentTime = theUnitTestDataCreator.GetTimeAtStart();
      } else {
         currentTime += dT;
      }

      currentTemperatute = theUnitTestDataCreator.ComputeTemperature(currentTime);
      currentPressure = theUnitTestDataCreator.ComputePressure(currentTime);
      Input *theInput = new Input(currentTime, currentTemperatute, currentPressure);
      AddInput(theInput);
   }
}
void SourceRockNode::CreateInputPTHistory(FILE * fp)
{
   double currentTime = 0.0;
   double currentTemperatute = 0.0;
   double currentPressure = 0.0;

   while(fscanf(fp, "%lg, %lg, %lg", &currentTime, &currentTemperatute, &currentPressure) == 3) { //for compatibility with VBA code
         Input *theInput = new Input(currentTime, currentTemperatute, currentPressure);
         AddInput(theInput);
   }
}

void SourceRockNode::CreateInputPTHistory(const std::vector<double>& time, const std::vector<double>& temperature, const std::vector<double>& pressure)
{
  const int n = time.size();
  assert(temperature.size() == n);
  assert(pressure.size() == n);
  for (int i = 0; i < n; ++i)
  {
    Input *theInput = new Input(time[i],temperature[i], pressure[i]);
    AddInput(theInput);
  }
}

void SourceRockNode::PrintInputHistory(ofstream &outputfile) const
{
   PrintInputHistoryHeader(outputfile);
   for(int i = 1; i < (int)m_theInput.size(); ++i) {
      m_theInput[i]->OutputOnFile(outputfile);
   }
}

void SourceRockNode::PrintInputHistory(std::string &outputFullPathFileName) const
{
   ofstream outfile;
   outfile.open(outputFullPathFileName.c_str());
   PrintInputHistory(outfile);
   outfile.close();
}
void SourceRockNode::PrintSensitivityOutputHeader(ofstream &outputfile) const
{
   outputfile << "Time,TotalOM,ExmaOil" << endl;
}
void SourceRockNode::PrintSensitivityOutput(std::string &sensitivityOutputFullPathFileName) const
{
   cout << "Saving sensitivity results file:" << sensitivityOutputFullPathFileName << endl;
   ofstream outfile;
   outfile.open(sensitivityOutputFullPathFileName.c_str());
   PrintSensitivityOutputHeader(outfile);
   for(int i = 0; i < (int)m_theOutput.size(); ++ i) {
      m_theOutput[i]->PrintSensitivityResultsOnFile(outfile);
   }
   outfile.close();
}
void SourceRockNode::NormalizePTHistory(const double dt, bool linearTemperature)
{
   std::vector<Input*>::iterator Current;
   std::vector<Input*>::iterator Previous;
   std::vector<Input*>::iterator Next;

   Current = Previous = Next = m_theInput.begin();

   std::vector<Input*> tempBuffer;

   double timeCounter = m_theInput[0]->GetTime();
   double startTime = m_theInput[0]->GetTime();
   double endTime = m_theInput[m_theInput.size() - 1]->GetTime();

   while(!(timeCounter < fabs(endTime - dt))) {
      Input * NewInput = 0;
      if(FindTimeInstance(timeCounter, Current)) {
         NewInput = new Input((*(*Current)));
      } else {
         FindInterpolationPair(timeCounter, Previous, Next);
         NewInput = new Input(timeCounter, (*(*Previous)), (*(*Next)));
      }
      if(NewInput) {
         tempBuffer.push_back(NewInput);
      }
      timeCounter -= dt;
   }
   Input *Last = new Input( *(m_theInput[m_theInput.size() -1 ]));
   tempBuffer.push_back(Last);

   std::vector<Input*>::iterator it;
   for(it = m_theInput.begin(); it != m_theInput.end(); ++ it) {
      delete  (*it);
   }
   m_theInput.clear();

   std::vector<Input*>::size_type numberOfTimesteps = tempBuffer.size() - 1;
   double tempAtBegin = tempBuffer[0]->GetTemperatureCelsius();
   double Dtemp = tempBuffer[numberOfTimesteps]->GetTemperatureCelsius() - tempBuffer[0]->GetTemperatureCelsius();
   double TimeDifference = fabs(startTime - endTime);
   double TemperatureIncrement=  Dtemp * dt / TimeDifference;
   int i = 0;
   for(it = tempBuffer.begin(); it != tempBuffer.end(); ++ it) {
      if(linearTemperature) {
         (*it)->SetTemperature((tempAtBegin + (double)i * TemperatureIncrement)) ;
         ++ i;
      }
      m_theInput.push_back(*it);
   }
}
int SourceRockNode::GetInputHistorySize() const
{
  return (int) m_theInput.size();
}
bool SourceRockNode::FindTimeInstance(const double timeCounter, std::vector<Input*>::iterator &Current)
{
   Current = m_theInput.begin();
   bool found = false;
   while(found == false && Current != m_theInput.end()) {
      if(fabs((*Current)->GetTime() - timeCounter) < 0.001) {
         found = true;
      } else {
         ++ Current;
      }
   }
   return found;
}
void SourceRockNode::FindInterpolationPair (const double timeCounter, std::vector<Input*>::iterator &Previous,
                                            std::vector<Input*>::iterator &Next)
{
   Previous = Next = m_theInput.begin();
   double timeDiff = 99999.9;
   for(Next = m_theInput.begin(); Next != m_theInput.end(); ++ Next) {
      timeDiff = (*Next)->GetTime() - timeCounter;
      if(timeDiff < 0.0) {
         break;
      }
   }
   Previous = Next - 1;
 }

void SourceRockNode::PrintInputHistoryHeader(ofstream &outputfile) const
{
   outputfile << "Time" << "," << "Temperature" << "," << "Pressure" << endl;
}

void SourceRockNode::PrintBenchmarkModelFluxTable(const ChemicalModel& chemicalModel,
                                                  ofstream &outputfile) const
{
   PrintBenchmarkModelFluxHeader(chemicalModel, outputfile);
   for(int i = 1; i < (int)m_theInput.size(); ++i) {
      outputfile << m_theInput[i]->GetTemperatureCelsius() << ",";
      m_theOutput[i]->PrintBenchmarkModelFluxData(outputfile);
   }
}
void SourceRockNode::PrintBenchmarkModelFluxHeader( const ChemicalModel& chemicalModel,
                                                    ofstream &outputfile) const
{
   outputfile << "[Table:ModelFluxC++]" << endl;
   outputfile << "temp" << "," << "VRE" << "," << "Time" << ",";

   int numberOfSpecies = chemicalModel.GetNumberOfSpecies();
   for(int id = 1; id <= numberOfSpecies; ++id) {
      const std::string SpeciesName = chemicalModel.GetSpeciesNameById(id);
      outputfile << SpeciesName << ",";
   }
   outputfile << "APIinst" << "," << "GORinst" << "," << endl;
}

void SourceRockNode::PrintBenchmarkModelConcTableHeader(const ChemicalModel& chemicalModel,
                                                        ofstream &outputfile) const
{
   outputfile << "[Table:ModelConcentrationC++]" << endl;
   outputfile << "temp" << "," << "VRE" << "," << "Time" << ",";

   int numberOfSpecies = chemicalModel.GetNumberOfSpecies();
   for(int id = 1; id <= numberOfSpecies; ++id) {
      const std::string SpeciesName = chemicalModel.GetSpeciesNameById(id);
      outputfile << SpeciesName << ",";
   }
   outputfile << "HoCTot" << "," << "OoCTot" << "," << endl;
}
void SourceRockNode::PrintBenchmarkModelConcTable(const ChemicalModel& chemicalModel,
                                                  ofstream &outputfile) const
{
   PrintBenchmarkModelConcTableHeader(chemicalModel,outputfile);
   for(int i = 1; i < (int)m_theOutput.size(); ++i) {
      outputfile << m_theInput[ i ]->GetTemperatureCelsius() << ",";
      m_theOutput[i]->PrintBenchmarkModelConcData(outputfile);
   }
}
void SourceRockNode::PrintBenchmarkModelCumExpHeader(const ChemicalModel& chemicalModel,
                                                     ofstream &outputfile) const
{
   outputfile << "[Table:ModelCummExpC++]" << endl;
   outputfile << "temp" << "," << "VRE" << "," << "Time" << ",";

   int numberOfSpecies = chemicalModel.GetNumberOfSpecies();
   for(int id = 1; id <= numberOfSpecies; ++id) {
      const std::string SpeciesName = chemicalModel.GetSpeciesNameById(id);
      outputfile << SpeciesName << ",";
   }
   outputfile << "ConvToOil" << "," << "APIcum" << "," << "GORcum" << "," << endl;
}
void SourceRockNode::PrintBenchmarkModelCumExpTable(const ChemicalModel& chemicalModel,
                                                    ofstream &outputfile) const
{
   PrintBenchmarkModelCumExpHeader(chemicalModel, outputfile);
   for(int i = 1; i < (int)m_theOutput.size(); ++i) {
      outputfile << m_theInput[i]->GetTemperatureCelsius() << ",";
      m_theOutput[i]->PrintBenchmarkModelCumExpData(outputfile);
   }
}
void SourceRockNode::PrintBenchmarkOutput(const std::string & in_FullPathBenchmarkName, const Simulator & theSimulator) const
{
   ofstream outputTestingSetFile;
   outputTestingSetFile.open(in_FullPathBenchmarkName.c_str());

   if(!outputTestingSetFile.is_open()) {
      cerr << "Can not open " << in_FullPathBenchmarkName << ". Aborting...." << endl;
      return;
   } else {
      cout << "Benchmark is saved in file " << in_FullPathBenchmarkName << endl;
   }

   //   outputTestingSetFile.setf(std::ios_base::scientific | std::ios_base::floatfield);
   outputTestingSetFile.precision(16);

   theSimulator.PrintBenchmarkOutput(outputTestingSetFile);
   PrintInputHistory(outputTestingSetFile);

   PrintBenchmarkModelConcTable(theSimulator.getChemicalModel (), outputTestingSetFile);
   PrintBenchmarkModelFluxTable(theSimulator.getChemicalModel (), outputTestingSetFile);
   PrintBenchmarkModelCumExpTable(theSimulator.getChemicalModel (), outputTestingSetFile);

   outputTestingSetFile.close();
}
void SourceRockNode::LoadTestingPTHistory(std::string in_FullPathFileName)
{
   std::string line;
   std::vector<std::string> theTokens;
   const char delim = ',';

   ifstream PThistoryFile;
   PThistoryFile.open(in_FullPathFileName.c_str());

   if(PThistoryFile) {
    std::getline(PThistoryFile, line, '\n');//omit header line

    while(! PThistoryFile.eof()) {
       std::getline(PThistoryFile, line, '\n');//get next
       if (line.empty()) {
          break;
       }
       StringHandler::parseLine(line, delim, theTokens);

       Input *theInput = new Input(atof(theTokens[4].c_str()), atof(theTokens[6].c_str()),
                                   1000.0 * atof(theTokens[5].c_str()));
       AddInput(theInput);

       theTokens.clear();
    }
    PThistoryFile.close();
   } else {
      cout << "Basin_Error: Invalid file :" << in_FullPathFileName << endl;
   }
}

void SourceRockNode::addNodeAdsorptionHistory ( NodeAdsorptionHistory* adsorptionHistory ) {
   m_adsorptionHistoryList.push_back ( adsorptionHistory );
}

void SourceRockNode::collectHistory () {

   NodeAdsorptionHistoryList::iterator nodeIter;

   for ( nodeIter = m_adsorptionHistoryList.begin (); nodeIter != m_adsorptionHistoryList.end (); ++nodeIter ) {
      (*nodeIter)->collect ( this );
   }

}

void SourceRockNode::computeHcVolumes ( double& gasVolume,
                                        double& oilVolume,
                                        double& gasExpansionRatio,
                                        double& gor,
                                        double& cgr,
                                        double& oilApi,
                                        double& condensateApi ) const {

   // Standard conditions.
   double StandardTemperatureGenexK = 15.5555556 + CelciusToKelvin; //Kelvin
   double StandardPressure    = 101325.353; //Pa

   const Genex6::Input* lastInput = getLastInput ();

   const Genex6::SimulatorState& state = getPrincipleSimulatorState(); //GetSimulatorState ();

   Genex6::PVTComponentMasses phaseMasses;
   Genex6::PVTPhaseValues     densities;
   Genex6::PVTPhaseValues     viscosities;

   double reservoirGasVolume = 0.0;

   double freeGasVolume;
   double condensateVolume;
   double solutionGasVolume;
   double liquidOilVolume; // This is the volume of oil in the liquid phase.

   Genex6::PVTComponents masses = state.getVapourComponents ();

   if ( lastInput != 0 ) {
      // Calculate reservoir gas volume
      masses ( ComponentId::COX ) = 0.0;
      masses ( ComponentId::H2S ) = 0.0;

      Genex6::PVTCalc::getInstance ().compute ( lastInput->GetTemperatureKelvin (), lastInput->getPorePressure (), masses, phaseMasses, densities, viscosities );
      reservoirGasVolume = phaseMasses.sum ( PhaseId::VAPOUR ) / densities ( PhaseId::VAPOUR );
   }

   // Calculate surface volumes from vapour components.
   masses ( ComponentId::COX ) = 0.0;
   masses ( ComponentId::H2S ) = 0.0;

   Genex6::PVTCalc::getInstance ().compute ( StandardTemperatureGenexK, StandardPressure, masses, phaseMasses, densities, viscosities );

   freeGasVolume = phaseMasses.sum ( PhaseId::VAPOUR ) / densities ( PhaseId::VAPOUR );
   gasVolume = freeGasVolume;

   condensateVolume = phaseMasses.sum ( PhaseId::LIQUID ) / densities ( PhaseId::LIQUID );
   oilVolume = condensateVolume;

   if ( densities ( PhaseId::LIQUID ) != 1000.0 ) {
      condensateApi = 141.5 / densities ( PhaseId::LIQUID ) * 1000.0 - 131.5;

      if ( condensateApi < 1.99 ) {
         condensateApi = CauldronNoDataValue;
      }

   } else {
      condensateApi = CauldronNoDataValue;
   }

   if ( freeGasVolume > 0.0 and densities ( PhaseId::VAPOUR ) != 1000.0 ) {
      cgr = condensateVolume / freeGasVolume;
   } else {
      cgr = CauldronNoDataValue;
   }

   if ( lastInput != 0 and gasVolume > 0.0 ) {
      gasExpansionRatio = reservoirGasVolume / gasVolume;
   } else {
      gasExpansionRatio = CauldronNoDataValue;
   }

   // Calculate surface volumes from liquid components.
   masses.zero();
   masses = state.getLiquidComponents ();
   masses ( ComponentId::COX ) = 0.0;
   masses ( ComponentId::H2S ) = 0.0;

   Genex6::PVTCalc::getInstance ().compute ( StandardTemperatureGenexK, StandardPressure, masses, phaseMasses, densities, viscosities );

   solutionGasVolume = phaseMasses.sum ( PhaseId::VAPOUR ) / densities ( PhaseId::VAPOUR );
   liquidOilVolume = phaseMasses.sum ( PhaseId::LIQUID ) / densities ( PhaseId::LIQUID );

   gasVolume += solutionGasVolume;
   oilVolume += liquidOilVolume;

   if ( densities ( PhaseId::LIQUID ) != 1000.0 ) {
      oilApi = 141.5 / densities ( PhaseId::LIQUID ) * 1000.0 - 131.5;

      if ( oilApi < 1.99 ) {
         oilApi = CauldronNoDataValue;
      }

   } else {
      oilApi = CauldronNoDataValue;
   }

   if ( liquidOilVolume > 0.0 and densities ( PhaseId::LIQUID ) != 1000.0 ) {
      gor = solutionGasVolume / liquidOilVolume;
   } else {
      gor = CauldronNoDataValue;
   }

}

void SourceRockNode::computeOverChargeFactor ( double& overChargeFactor ) const {

   Genex6::SimulatorState& state = getPrincipleSimulatorState(); //GetSimulatorState ();
   const Genex6::SpeciesManager& speciesManager = *state.getSpeciesManager();

   Genex6::PVTComponentMasses phaseMasses;
   Genex6::PVTComponents      masses;
   Genex6::PVTPhaseValues     densities;
   Genex6::PVTPhaseValues     viscosities;

   const Genex6::Input& nodeInput = *getLastInput ();

   double vapourVolume;
   double liquidVolume;

   double thickness    = state.GetThickness ();

   // The computed porosity less the space taken by the immobile species.
   double porosity     = state.getEffectivePorosity ();

   // Irreducible water saturation.
   double iws          = state.getIrreducibleWaterSaturation ();
   double temperature  = nodeInput.GetTemperatureKelvin ();
   double porePressure = nodeInput.GetPressure ();

   int i;

   masses.zero ();

   for ( i = speciesManager.firstSpecies (); i <= speciesManager.lastSpecies (); ++i ) {
      ComponentId pvtId = speciesManager.mapIdToPvtComponents ( i );

      if ( pvtId != ComponentId::UNKNOWN ) {
         const SpeciesState* speciesState = state.GetSpeciesStateById ( i );

         masses ( pvtId ) = speciesState->getMassExpelledFromSourceRock () + speciesState->getRetained ();
      }

   }

   Genex6::PVTCalc::getInstance ().compute ( temperature, porePressure, masses, phaseMasses, densities, viscosities );

   vapourVolume = phaseMasses.sum ( PhaseId::VAPOUR ) / densities ( PhaseId::VAPOUR );
   liquidVolume = phaseMasses.sum ( PhaseId::LIQUID ) / densities ( PhaseId::LIQUID );

   if ( thickness > 0.0 and porosity > 0.0 and iws < 1.0 ) {
      overChargeFactor = ( vapourVolume + liquidVolume ) / ( thickness * porosity * ( 1.0 - iws ));
   } else {
      overChargeFactor = CauldronNoDataValue;
   }

}


void SourceRockNode::zeroTimeStepAccumulations () {

   size_t i;

   for ( i = 0; i < m_theSimulatorStates.size (); ++i ) {
      m_theSimulatorStates [ i ]->resetIntervalCumulative ();
   }

   if ( m_mixedSimulatorState != 0 ) {
      m_mixedSimulatorState->resetIntervalCumulative ();
   }

}

// const SimulatorState& SourceRockNode::getState () const {
//    return getPrincipleSimulatorState ();
//    // return *m_currentState;
// }

}

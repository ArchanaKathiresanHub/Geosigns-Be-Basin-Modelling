#include "SourceRockNode.h"
#include "SourceRockNodeInput.h"
#include "SourceRockNodeOutput.h"
#include "SimulatorState.h"
#include "Simulator.h"
#include "ChemicalModel.h"  
#include "Utilities.h"
#include "UnitTestDataCreator.h"
#include "ComponentManager.h"

#include "SpeciesState.h"
#include "SpeciesResult.h"


#include <math.h>


namespace Genex5
{

SourceRockNode::SourceRockNode(const double &in_thickness,
                               const double &in_TOC,
                               const double &in_InorganicDensity,
                               const double &in_sgMeanBulkDensity,
                               const int &in_I,
                               const int &in_J):
m_thickness(in_thickness),
m_TOCi(in_TOC),
m_InorganicDensity(in_InorganicDensity),
m_sgMeanBulkDensity (in_sgMeanBulkDensity),
m_I(in_I),
m_J(in_J),
m_currentState(0),
m_ConcKi(0),
m_currenToc ( in_TOC )
{
}
SourceRockNode::~SourceRockNode()
{
   ClearInputHistory();

   ClearOutputHistory();

   delete m_currentState;
}
void SourceRockNode::ClearInputHistory()
{
   std::vector<SourceRockNodeInput*>::iterator itEnd  = m_theInput.end();
   for(std::vector<SourceRockNodeInput*>::iterator it = m_theInput.begin(); it != itEnd; ++it)
   {
      delete  (*it);
   }
   m_theInput.clear();
}
void SourceRockNode::ClearOutputHistory()
{
   std::vector<SourceRockNodeOutput*>::iterator itEnd  = m_theOutput.end();
   for(std::vector<SourceRockNodeOutput*>::iterator it = m_theOutput.begin(); it != itEnd; ++it)
   {
      delete  (*it);
   }
   m_theOutput.clear();
}
void SourceRockNode::AddOuput(SourceRockNodeOutput* in_theOuput)
{
   m_theOutput.push_back(in_theOuput);
}
void SourceRockNode::AddInput(SourceRockNodeInput* in_theInput)
{
   m_theInput.push_back(in_theInput);
}


/*!
 * Yiannis
 */
void SourceRockNode::RequestComputation ( Simulator & theSimulator,
                                          const bool isSnapshot )
{
   std::vector<SourceRockNodeInput*>::iterator itInput;
   bool isInitialTimeStep = true;

   for(itInput = m_theInput.begin(); itInput != m_theInput.end(); ++itInput)
   {
      SourceRockNodeOutput *theSourceRockOutput = new SourceRockNodeOutput((*itInput)->GetCurrentTime());
      if(m_currentState)
      {
         const double thicknessScale = (*itInput)->GetThicknessScaleFactor();

         theSimulator.ComputeSourceRockNodeTimeInstance((*(*itInput)), m_currentState, (*theSourceRockOutput), (thicknessScale * m_thickness), m_ConcKi);
         isInitialTimeStep = false;
      }
      else
      {
         theSimulator.InitializeSourceRockNode((*(*itInput)), m_currentState, (*theSourceRockOutput),  m_thickness, m_TOCi, m_InorganicDensity, m_sgMeanBulkDensity, m_ConcKi);
      }

      if ( isSnapshot ) {
         updateOutput ( theSimulator, *itInput, m_currentState, theSourceRockOutput );
      }

      AddOuput(theSourceRockOutput);
   }

   if ( not isInitialTimeStep ) {
      collectHistory ();
   }

}

void SourceRockNode::updateOutput (       Simulator&            simulator,
                                    const SourceRockNodeInput*  input,
                                          SimulatorState*       state,
                                          SourceRockNodeOutput* output ) {

   if ( simulator.getAdsorptionSimulator () == 0 ) {
      return;
   }

   const AdsorptionSimulator* adsorptionSimulator = simulator.getAdsorptionSimulator ();

   using namespace CBMGenerics;

   unsigned int species;

   for ( species = 0; species < ComponentManager::NumberOfOutputSpecies; ++species ) {

      if ( adsorptionSimulator->speciesIsSimulated ( ComponentManager::SpeciesNamesId ( species ))) {
         const std::string speciesName = ComponentManager::getInstance ().GetSpeciesName ( species );
         const SpeciesState* speciesState = state->GetSpeciesStateByName ( speciesName );
         SpeciesResult* speciesResult = output->GetSpeciesResultByName ( speciesName );

         speciesResult->setAdsorpedMol ( speciesState->getAdsorpedMol ());
         speciesResult->setFreeMol ( speciesState->getFreeMol ());
         speciesResult->setExpelledMol ( speciesState->getExpelledMol ());
      }

   }

}

const SourceRockNodeOutput *SourceRockNode::GetOutputByIndex(const int &index) const
{
   SourceRockNodeOutput *ret = 0;
   if(index >= 0 && index < m_theOutput.size())
   {
      ret = m_theOutput[index];
   }

   return ret;
}
unsigned int SourceRockNode::GetOutputSize() const
{
   return m_theOutput.size();
}

unsigned int SourceRockNode::getInputSize() const
{
   return m_theInput.size();
}

//To be deprecated in BPA/Cauldron. 1D simulation is taken over by fastgenex5 as well so the only valid Genex5::SourceRockNode computation interface 
//will be RequestComputation::(Simulator &)
int SourceRockNode::RequestComputation1D(Simulator *theSimulator, double snapshots[], const int &numberOfSnapshots, const double &depositionAge)
{
	cout<<"1D MODE-GENEX5KERNEL"<<endl;
   if(m_theInput.size() > numberOfSnapshots)
   {
      cout<<"Input sizes are incompatible...Aborting...";
      return FAIL;
   }
   if(theSimulator == 0)
   {
      cout<<"Invalid Simulator...Aborting...";
      return FAIL;
   }

	int numberOfIterations = static_cast<int>(m_theInput.size());

   std::vector<SourceRockNodeInput*>::iterator intervalStartInput = m_theInput.begin();
   
   SourceRockNodeOutput *FirstSourceRockOutput = new SourceRockNodeOutput((*intervalStartInput)->GetCurrentTime());
   theSimulator->InitializeSourceRockNode((*(*intervalStartInput)), m_currentState, (*FirstSourceRockOutput), m_thickness, m_TOCi, m_InorganicDensity, m_sgMeanBulkDensity, m_ConcKi);
   AddOuput(FirstSourceRockOutput);

   collectHistory ();

   double timeStepSize = theSimulator->GetMaximumTimeStepSize(depositionAge);
   double t = snapshots[0] - timeStepSize;

   for(int i = 1; i <numberOfIterations; i++)
   {
      SourceRockNodeInput *intervalStartInput = m_theInput[i - 1];
      SourceRockNodeInput *intervalEndInput   = m_theInput[i];
      SourceRockNodeOutput *SnapShotOutput = new SourceRockNodeOutput(intervalEndInput->GetCurrentTime());

      while(t > snapshots[i])
      {
         SourceRockNodeInput  TimeInstanceInput  (t, (*intervalStartInput), (*intervalEndInput) );
         SourceRockNodeOutput TimeInstanceOutput (TimeInstanceInput.GetCurrentTime());
//          SourceRockNodeInput  *TimeInstanceInput  = new SourceRockNodeInput(t, (*intervalStartInput), (*intervalEndInput) );
//          SourceRockNodeOutput *TimeInstanceOutput = new SourceRockNodeOutput(TimeInstanceInput->GetCurrentTime());

         const double thicknessScale = TimeInstanceInput.GetThicknessScaleFactor();
//          const double thicknessScale = TimeInstanceInput->GetThicknessScaleFactor();

         theSimulator->ComputeSourceRockNodeTimeInstance( TimeInstanceInput, m_currentState, TimeInstanceOutput, thicknessScale * m_thickness, m_ConcKi);
//          theSimulator->ComputeSourceRockNodeTimeInstance((*TimeInstanceInput), m_currentState, (*TimeInstanceOutput), ( thicknessScale * m_thickness ), m_ConcKi);

//          delete TimeInstanceInput;
//          delete TimeInstanceOutput;

         t -= timeStepSize;


         collectHistory ();
      }

      theSimulator->ComputeSourceRockNodeTimeInstance((*intervalEndInput), m_currentState, (*SnapShotOutput), m_thickness, m_ConcKi);

      collectHistory ();
      
      AddOuput(SnapShotOutput);
   }
   return SUCCESS;
}
const SourceRockNodeOutput *SourceRockNode::GetTheLastOutput() const
{
   SourceRockNodeOutput *ret = 0;
   if(!m_theOutput.empty()) 
   {
      ret = m_theOutput[m_theOutput.size() - 1];
   }
   return ret;
}

const SourceRockNodeInput *SourceRockNode::getTheLastInput() const
{
   SourceRockNodeInput *ret = 0;

   if(!m_theInput.empty()) 
   {
      ret = m_theInput[m_theInput.size() - 1];
   }

   return ret;
}

const SimulatorState* SourceRockNode::getSimulatorState () const {
   return m_currentState;
}

SimulatorState* SourceRockNode::getSimulatorState () {
   return m_currentState;
}


void SourceRockNode::CreateTestingPTHistory(const UnitTestDataCreator &theUnitTestDataCreator)
{
   int numberOfTimesteps = theUnitTestDataCreator.GetNumberOfTimesteps();
   double dT = theUnitTestDataCreator.GetTimeStepSize();

   double currentTime = 0.0;
   double currentTemperatute = 0.0;
   double currentPressure = 0.0;

   for(int i = 0;i <= numberOfTimesteps; i++)   //for compatibility with VBA code
   {
     if(i == 0)
     {
        currentTime = theUnitTestDataCreator.GetTimeAtStart();
     }
     else
     {
        currentTime += dT;
     }

     currentTemperatute =theUnitTestDataCreator.ComputeTemperature(currentTime);
     currentPressure = theUnitTestDataCreator.ComputePressure(currentTime);
     SourceRockNodeInput *theInput = new SourceRockNodeInput(currentTime, currentTemperatute, currentPressure);
     AddInput(theInput);
   }
}
void SourceRockNode::PrintInputHistory(ofstream &outputfile) const
{
   PrintInputHistoryHeader(outputfile);
   for(int i = 1;i < (int)m_theInput.size();i++)
   {
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
   outputfile<<"Time,TotalOM,ExmaOil"<<endl;
}
void SourceRockNode::PrintSensitivityOutput(std::string &sensitivityOutputFullPathFileName) const
{
   cout<<"Saving sensitivity results file:"<<sensitivityOutputFullPathFileName<<endl;
   ofstream outfile;
   outfile.open(sensitivityOutputFullPathFileName.c_str());
   PrintSensitivityOutputHeader(outfile);
   for(int i=0;i<(int)m_theOutput.size();i++)
   {
      m_theOutput[i]->PrintSensitivityResultsOnFile(outfile);
   }
   outfile.close();
}
void SourceRockNode::NormalizePTHistory(const double &dt, bool linearTemperature)
{
   std::vector<SourceRockNodeInput*>::iterator Current;
   std::vector<SourceRockNodeInput*>::iterator Previous;
   std::vector<SourceRockNodeInput*>::iterator Next;

   Current = Previous = Next =m_theInput.begin();

   std::vector<SourceRockNodeInput*> tempBuffer;

   double timeCounter = m_theInput[0]->GetCurrentTime();
   double startTime = m_theInput[0 ]->GetCurrentTime();
   double endTime = m_theInput[m_theInput.size() -1 ]->GetCurrentTime();

   while(! ( timeCounter< fabs(endTime - dt) ) )
   {
      SourceRockNodeInput *NewInput=0;
      if(FindTimeInstance(timeCounter, Current))
      {
         NewInput=new SourceRockNodeInput((*(*Current)));
      }
      else
      {
         FindInterpolationPair (timeCounter, Previous, Next);
         NewInput=new SourceRockNodeInput(timeCounter, (*(*Previous)), (*(*Next)));
      }
      if(NewInput)
      {
         tempBuffer.push_back(NewInput);
      }
      timeCounter -= dt;
   }
   SourceRockNodeInput *Last=new SourceRockNodeInput( *(m_theInput[m_theInput.size() -1 ]) );
   tempBuffer.push_back(Last);

   std::vector<SourceRockNodeInput*>::iterator it;
   for(it=m_theInput.begin(); it!=m_theInput.end(); ++it)
   {
      delete  (*it);
   }
   m_theInput.clear();
   std::vector<SourceRockNodeInput*>::size_type numberOfTimesteps = tempBuffer.size() - 1;
   double tempAtBegin = tempBuffer[0]->GetTemperatureCelsius();
   double Dtemp = tempBuffer[numberOfTimesteps]->GetTemperatureCelsius() -tempBuffer[0]->GetTemperatureCelsius();
   double TimeDifference = fabs(startTime - endTime);
   double TemperatureIncrement=  Dtemp * dt / TimeDifference;
   int i = 0;
   for(it=tempBuffer.begin(); it!=tempBuffer.end(); ++it)
   {
      if(linearTemperature)
      {
         (*it)->SetTemperature(( tempAtBegin +(double)i * TemperatureIncrement) ) ;
         i++;
      }
      m_theInput.push_back(*it);
   }
}
int SourceRockNode::GetInputHistorySize() const
{
  return (int) m_theInput.size();
}
bool SourceRockNode::FindTimeInstance(const double &timeCounter, std::vector<SourceRockNodeInput*>::iterator &Current)
{
   Current=m_theInput.begin();
   bool found=false;
   while(found==false && Current!=m_theInput.end())
   {
      if(fabs((*Current)->GetCurrentTime() - timeCounter)<0.001)
      {
         found=true;
      }
      else
      {
         ++Current;
      }
   }
   return found;
}
void SourceRockNode::FindInterpolationPair (const double &timeCounter, std::vector<SourceRockNodeInput*>::iterator &Previous,std::vector<SourceRockNodeInput*>::iterator &Next)
{
   Previous = Next = m_theInput.begin();
   double timeDiff = 99999.9;
   for(Next= m_theInput.begin(); Next!=m_theInput.end();++Next)
   {
      timeDiff=(*Next)->GetCurrentTime() - timeCounter;
      if(timeDiff<0.0)
      {
         break;
      }
   }
   Previous = Next - 1;
 }
void SourceRockNode::PrintInputHistoryHeader(ofstream &outputfile) const
{
   outputfile<<"Time"<<","<<"Temperature"<<","<<"Pressure"<<endl;
}
void SourceRockNode::PrintBenchmarkModelFluxHeader(ofstream &outputfile) const
{
   outputfile<<"[Table:ModelFluxC++]"<<endl;
   outputfile<<"temp"<<","<<"VRE"<<","<<"Time"<<",";

   int numberOfSpecies=ChemicalModel::GetNumberOfSpecies();
   for(int id=1;id<=numberOfSpecies;id++)
   {
      const std::string SpeciesName=ChemicalModel::GetSpeciesNameById(id);
      outputfile<<SpeciesName<<",";
   }
   outputfile<<"APIinst"<<","<<"GORinst"<<","<<endl;
}
void SourceRockNode::PrintBenchmarkModelFluxTable(ofstream &outputfile) const
{
   PrintBenchmarkModelFluxHeader(outputfile);
   for(int i=1;i<(int)m_theOutput.size();i++)
   {
      outputfile<< m_theInput[ i ]->GetTemperatureCelsius()<<",";
      m_theOutput[i]->PrintBenchmarkModelFluxData(outputfile);
   } 
}
void SourceRockNode::PrintBenchmarkModelCumExpHeader(ofstream &outputfile) const
{
   outputfile<<"[Table:ModelCummExpC++]"<<endl;
   outputfile<<"temp"<<","<<"VRE"<<","<<"Time"<<",";

   int numberOfSpecies=ChemicalModel::GetNumberOfSpecies();
   for(int id=1;id<=numberOfSpecies;id++)
   {
      const std::string SpeciesName=ChemicalModel::GetSpeciesNameById(id);
      outputfile<<SpeciesName<<",";
   }
   outputfile<<"ConvToOil"<<","<<"APIcum"<<","<<"GORcum"<<","<<endl;
}
void SourceRockNode::PrintBenchmarkModelCumExpTable(ofstream &outputfile) const
{
   PrintBenchmarkModelCumExpHeader(outputfile);
   for(int i=1;i<(int)m_theOutput.size();i++)
   {
      outputfile<< m_theInput[ i ]->GetTemperatureCelsius()<<",";
      m_theOutput[i]->PrintBenchmarkModelCumExpData(outputfile);
   }
}
void SourceRockNode::PrintBenchmarkModelConcTableHeader(ofstream &outputfile) const
{
   outputfile<<"[Table:ModelConcentrationC++]"<<endl;
   outputfile<<"temp"<<","<<"VRE"<<","<<"Time"<<",";
   
   int numberOfSpecies=ChemicalModel::GetNumberOfSpecies();
   for(int id=1;id<=numberOfSpecies;id++)
   {
      const std::string SpeciesName=ChemicalModel::GetSpeciesNameById(id);
      outputfile<<SpeciesName<<",";
   }
   outputfile<<"HoCTot"<<","<<"OoCTot"<<","<<endl;
}
void SourceRockNode::PrintBenchmarkModelConcTable(ofstream &outputfile) const
{
   PrintBenchmarkModelConcTableHeader(outputfile);
   for(int i=1;i<(int)m_theOutput.size();i++)
   {
      outputfile<< m_theInput[ i ]->GetTemperatureCelsius()<<",";
      m_theOutput[i]->PrintBenchmarkModelConcData(outputfile);
   }
}
void SourceRockNode::PrintBenchmarkOutput(const std::string &in_FullPathBenchmarkName, Simulator &theSimulator)
{
   cout<<"Benchmark is saved in file "<<in_FullPathBenchmarkName<<endl;

   ofstream outputTestingSetFile;
   outputTestingSetFile.open(in_FullPathBenchmarkName.c_str());

   //outputTestingSetFile.setf(std::ios_base::scientific|std::ios_base::floatfield);
   //outputTestingSetFile.precision(16);

   theSimulator.PrintBenchmarkOutput(outputTestingSetFile);

   PrintInputHistory(outputTestingSetFile);

   PrintBenchmarkModelConcTable(outputTestingSetFile);
   PrintBenchmarkModelFluxTable(outputTestingSetFile);
   PrintBenchmarkModelCumExpTable(outputTestingSetFile);

   outputTestingSetFile.close();
  
}
void SourceRockNode::LoadTestingPTHistory(std::string in_FullPathFileName)
{
   std::string line;
   std::vector<std::string> theTokens;
   std::string delim=",";

   ifstream PThistoryFile;
   PThistoryFile.open(in_FullPathFileName.c_str());


   if(PThistoryFile)
   {
    std::getline(PThistoryFile,line,'\n');//omit header line

    while(! PThistoryFile.eof())
    {
       std::getline(PThistoryFile,line,'\n');//get next
       if ( line.empty() )
       {
          break;
       }
       ParseLine(line, delim, theTokens);

       SourceRockNodeInput *theInput=new SourceRockNodeInput(atof(theTokens[4].c_str()), atof(theTokens[6].c_str()), 1000.0*atof(theTokens[5].c_str()));
       AddInput(theInput);

       theTokens.clear();
    }
    PThistoryFile.close();
   }
   else
   {
      cout<<"Invalid file :" << in_FullPathFileName <<endl;
   }
}

void SourceRockNode::addNodeAdsorptionHistory ( NodeAdsorptionHistory* adsorptionHistory ) {
   m_adsorptionHistoryList.push_back ( adsorptionHistory );
}


void SourceRockNode::computeCurrentToc () {

   m_currenToc = 0.0;

}

void SourceRockNode::collectHistory () {

   NodeAdsorptionHistoryList::iterator nodeIter;

   for ( nodeIter = m_adsorptionHistoryList.begin (); nodeIter != m_adsorptionHistoryList.end (); ++nodeIter ) {
      (*nodeIter)->collect ( this );
   }

}

}



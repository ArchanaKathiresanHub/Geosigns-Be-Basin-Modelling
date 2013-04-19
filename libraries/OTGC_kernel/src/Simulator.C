#include "Simulator.h"
#include "ChemicalModel.h"

#include "Input.h"
#include "Utilities.h"
#include "SimulatorState.h"

#include <cmath>
#include <map>
#include <string>
#include <vector>
#include <algorithm>
#include <memory>
#include "Constants.h"

#include "GeneralParametersHandler.h"
#include "LinearInterpolator.h"


namespace OTGC
{
std::string Simulator::s_cfgFileExtension = ".cfg";
double Simulator::s_dT = 0.0;
double Simulator::s_Peff = 0.0;
double Simulator::s_TK = 0.0;
double Simulator::s_FrequencyFactor = 0.0;
double Simulator::s_Waso = 0.0;
double Simulator::s_maxkerogenTransformationRatio = 0.0;
double Simulator::s_DiffusionConcDependence = 0.0;
double Simulator::s_VogelFulcherTemperature = 0.0;

Simulator::Simulator(const std::string &in_fullPathToConfigurationFileDirectory,
                     const std::string &in_type,
   		            const double &in_HC,
   		            const double &in_Emean,
                     const double &in_VRE ,
                     const double &in_asphalteneDiffusionEnergy,
                     const double &in_resinDiffusionEnergy,
                     const double &in_C15AroDiffusionEnergy,
                     const double &in_C15SatDiffusionEnergy):
m_fullPathToConfigurationFileDirectory(in_fullPathToConfigurationFileDirectory + OTGC::Constants::folder_divider),
m_type(in_type),
m_HC(in_HC)
{
   m_theChemicalModel = 0;
   m_currentState = 0;

   m_preProcessSpeciesKinetics = true;
   m_preProcessSpeciesComposition = true;
   m_useDefaultGeneralParameters  = true;
   m_numberOfTimesteps = 400;
   m_maximumTimeStepSize = 1.0;
   m_openConditions = true;   

   //build chemical model, get boundary conditions
   CreateInstance();

   Preprocess(in_Emean, in_VRE , in_asphalteneDiffusionEnergy, in_resinDiffusionEnergy, in_C15AroDiffusionEnergy, in_C15SatDiffusionEnergy); 
}

//meant to work with old configuration file...
Simulator::Simulator(const std::string &in_fullPathConfigurationFileName,
   		     const double &in_HC,
   		     const double &in_Emean, 
                     bool PreprocessChemicalModel)
{ 
   m_theChemicalModel = 0;
   m_currentState = 0;

   m_preProcessSpeciesKinetics = true;
   m_preProcessSpeciesComposition = true;
   m_useDefaultGeneralParameters  = true;
   m_numberOfTimesteps = 400;
   m_maximumTimeStepSize = 1.0;
   m_openConditions = true;   

   m_theChemicalModel= new OTGC::ChemicalModel(in_fullPathConfigurationFileName);
 
   
   if(PreprocessChemicalModel)
   {
      cout<<"PREPROCESSING CHEMICAL MODEL"<<endl;
      m_theChemicalModel->UpdateSpeciesCompositionsByElementName("preasphalt","H", m_HC); 
      m_theChemicalModel->CompEarlySpecies();
      m_theChemicalModel->KineticsEarlySpecies(in_Emean);
      //OTGC
      //m_theChemicalModel->SetSpeciesReactionOrder(); no need for that here
      //OTGC
      m_theChemicalModel->UpdateSpeciesProperties();
   }                  
}
void Simulator::Preprocess(const double &in_Emean,
		           const double &in_VRE,
                           const double &in_asphalteneDiffusionEnergy,
                           const double &in_resinDiffusionEnergy,
                           const double &in_C15AroDiffusionEnergy,
                           const double &in_C15SatDiffusionEnergy)

{
   m_theChemicalModel->UpdateSpeciesCompositionsByElementName("preasphalt","H", m_HC); 
  
   if(m_preProcessSpeciesComposition)
   {
      m_theChemicalModel->CompEarlySpecies();
   } 

   if(m_preProcessSpeciesKinetics)
   {  
      m_theChemicalModel->KineticsEarlySpecies(in_Emean);
   }

   m_theChemicalModel->UpdateSpeciesProperties();

   m_theChemicalModel->UpdateSpeciesDiffusionEnergy1("asphaltenes",in_asphalteneDiffusionEnergy);
   m_theChemicalModel->UpdateSpeciesDiffusionEnergy1("resins",in_resinDiffusionEnergy);
   m_theChemicalModel->UpdateSpeciesDiffusionEnergy1("C15+Aro",in_C15AroDiffusionEnergy);
   m_theChemicalModel->UpdateSpeciesDiffusionEnergy1("C15+Sat",in_C15SatDiffusionEnergy); 
   //OTGC
   m_theChemicalModel->SetSpeciesReactionOrder();//Depends on ChemicalModel::UpdateSpeciesProperties
   //OTGC
	m_theChemicalModel->ComputeStoichiometry();
}
Simulator::~Simulator()
{
   delete m_theChemicalModel; 
}
void Simulator:: CreateInstance()
{
   
   m_theChemicalModel= new OTGC::ChemicalModel();
   LoadDataFromConfigurationFile();
}
void Simulator::LoadDataFromConfigurationFile()
{ 
   std:: string FullPathConfigurationFileName = m_fullPathToConfigurationFileDirectory + m_type + s_cfgFileExtension;
   ifstream  ConfigurationFile;
   ConfigurationFile.open(FullPathConfigurationFileName.c_str());
   
   if(!ConfigurationFile)
   {
     //throw
      cout<<"Attempting to open file :"<<FullPathConfigurationFileName<<endl;
      cout<<"No cfg file available in the $OTGCDIR directory... Aborting... "<<endl;
      return;
   }

   bool finishReadingFile=false;  
   std::string line;

   while( !ConfigurationFile.eof() && finishReadingFile==false)
   {
      //std::getline(ConfigurationFile,line,'\n');
      #ifdef sun
           static char buf[1<<14];
           ConfigurationFile.getline (buf, 1<<14);
           line = buf;
        #else
           std::getline (ConfigurationFile, line, '\n');
        #endif
     
      if(line==OTGC::CFG::TableSimulatorProperties || line.find(OTGC::CFG::TableSimulatorProperties, 0) !=std::string::npos)
      {
        LoadSimulatorProperties(ConfigurationFile);
      }
      else if(line==OTGC::CFG::TableElements || line.find(OTGC::CFG::TableElements, 0) !=std::string::npos)
      {
        m_theChemicalModel->LoadElements(ConfigurationFile);
      }
      else if(line==OTGC::CFG::TableSpecies || line.find(OTGC::CFG::TableSpecies, 0) !=std::string::npos)
      {
        m_theChemicalModel->LoadSpecies(ConfigurationFile);
      }
      else if(line==OTGC::CFG::SpeciesCompositionByName || line.find(OTGC::CFG::SpeciesCompositionByName, 0) !=std::string::npos)
      {
        m_theChemicalModel->LoadSpeciesComposition(ConfigurationFile);
      }
      else if(line==OTGC::CFG::SpeciesPropertiesByName || line.find(OTGC::CFG::SpeciesPropertiesByName, 0) !=std::string::npos)
      {
        m_theChemicalModel->LoadSpeciesProperties(ConfigurationFile);
      }
      else if(line==OTGC::CFG::ReactionsBySpeciesName || line.find(OTGC::CFG::ReactionsBySpeciesName, 0) !=std::string::npos)
      {
        m_theChemicalModel->LoadReactions(ConfigurationFile);
      }
      else if(line==OTGC::CFG::ReactionRatiosBySpeciesName || line.find(OTGC::CFG::ReactionRatiosBySpeciesName, 0) !=std::string::npos)
      {
        m_theChemicalModel->LoadReactionRatios(ConfigurationFile);
      }
      else if(line==OTGC::CFG::GeneralParameters || line.find(OTGC::CFG::GeneralParameters, 0) !=std::string::npos)
      {
        //expand it for clarity
        if(m_useDefaultGeneralParameters==true)
        {
          finishReadingFile=true;
        }
        else
        {
           LoadGeneralParameters(ConfigurationFile);
        }
      }
   }
   ConfigurationFile.close();
}
bool Simulator::Validate()const
{
   bool status = true;
   status = m_theChemicalModel->Validate();
   return status;
}
void Simulator::LoadSimulatorProperties(ifstream &ConfigurationFile)
{
   std::string line;
   std::vector<std::string> theTokens;
   std::string delim = ",";
   
   for(;;)
   {
        //std::getline(ConfigurationFile,line,'\n');
        #ifdef sun
           static char buf[1<<14];
           ConfigurationFile.getline (buf, 1<<14);
           line = buf;
        #else
           std::getline (ConfigurationFile, line, '\n');
        #endif
        
        if(line == OTGC::CFG::EndOfTable || line.size() == 0 )
        {
          break;
        }
        
        ParseLine(line, delim, theTokens);

        if(theTokens.size() == 2)
        {
           if(theTokens[0] == OTGC::CFG::PreprocessSpeciesKinetics)
           {
             //transform(theTokens[1].begin(), theTokens[1].end(), theTokens[1].begin(), toupper) ;
             TransformStringToUpper(theTokens[1]);
             if(theTokens[1] == "TRUE")
             {
                m_preProcessSpeciesKinetics = true;
             }
             else
             {
                 m_preProcessSpeciesKinetics = false;  
             } 
           }
           else if(theTokens[0] == OTGC::CFG::PreprocessSpeciesComposition)
           {
              //std::transform(theTokens[1].begin(), theTokens[1].end(), theTokens[1].begin(), toupper) ;
             TransformStringToUpper(theTokens[1]);
             if(theTokens[1] == "TRUE")
             {
                m_preProcessSpeciesComposition = true;
             }
             else
             {
                 m_preProcessSpeciesComposition = false;
             }
           }
           else if(theTokens[0] == OTGC::CFG::UseDefaultGeneralParameters)
           {
             //std::transform(theTokens[1].begin(), theTokens[1].end(), theTokens[1].begin(), toupper) ;
             TransformStringToUpper(theTokens[1]);
             if(theTokens[1]=="TRUE")
             {
                m_useDefaultGeneralParameters = true;
             }
             else
             {
                 m_useDefaultGeneralParameters = false;
             }
           }
           else if(theTokens[0] == OTGC::CFG::NumberOfTimesteps)
           {
              m_numberOfTimesteps = atoi(theTokens[1].c_str());
           }
           else if(theTokens[0] == OTGC::CFG::SimulateOpenConditions)
           {
             //std::transform(theTokens[1].begin(), theTokens[1].end(), theTokens[1].begin(), toupper) ;
             TransformStringToUpper(theTokens[1]);
             if(theTokens[1] == "TRUE")
             {
                 m_openConditions = true;
             }
             else
             {
                 m_openConditions = false;
             }
           }
           else if(theTokens[0] == OTGC::CFG::MaximumTimeStepSize)
           {
              m_maximumTimeStepSize = atof(theTokens[1].c_str());
           }
           else if(theTokens[0] == OTGC::CFG::MassBalancePercentTolerance)
           {
              m_massBalancePercentTolerance = atof(theTokens[1].c_str());
           }
        }
        else
        {
          //throw "more arguments than expected"
        }
        theTokens.clear();
   }
}
void Simulator::LoadGeneralParameters(ifstream &ConfigurationFile)
{
   //change values stored in singleton
   std::string line;
   std::vector<std::string> theTokens;
   std::string delim=",";

   GeneralParametersHandler &theHandler = GeneralParametersHandler::getInstance();
   for(;;)
   {
        //std::getline(ConfigurationFile,line,'\n');
        #ifdef sun
           static char buf[1<<14];
           ConfigurationFile.getline (buf, 1<<14);
           line = buf;
        #else
           std::getline (ConfigurationFile, line, '\n');
        #endif

        if(line == OTGC::CFG::EndOfTable || line.size() == 0 )
        {
          break;
        }
        ParseLine(line, delim, theTokens);

        //2Do, implement here the interface for parameters
        if(theTokens.size()==2)
        {
           cout<< theTokens[0]<<endl;
           theHandler.SetParameterByName( theTokens[0], atof(theTokens[1].c_str()) );
        }
        else
        {
          //throw "more arguments than expected"
        }
        theTokens.clear();
   }
}
 //FunTransKerog3 = (1! - (Conc(Lkerogen) + Conc(Lpreasphalt)))
void Simulator::ComputeKerogenTransformatioRatio()
{
   s_maxkerogenTransformationRatio = 1.0 ;
}
void Simulator::ComputePrecokeTransformatioRatio()
{
	//double precokeUltimateMass = m_theChemicalModel->getSpeciesUltimateMassByName("precoke");
   double precokeUltimateMass = m_currentState->getSpeciesUltimateMassByName("precoke");
	double tempPrecoketransRatio = 0;
	if(precokeUltimateMass > 1e-30)
	{
   	tempPrecoketransRatio = (tempPrecoketransRatio = m_currentState->GetSpeciesConcentrationByName("precoke") / precokeUltimateMass) > 1 ? 1.0 :                                                    tempPrecoketransRatio;
	}
	if(tempPrecoketransRatio > m_currentState->getMaxPrecokeTransfRatio())
	{
     m_currentState->setMaxPrecokeTransfRatio(tempPrecoketransRatio);
	}
}
void Simulator::ComputeCoke2TransformatioRatio()
{
	//double coke2UltimateMass = m_theChemicalModel->getSpeciesUltimateMassByName("coke2");
   double coke2UltimateMass = m_currentState->getSpeciesUltimateMassByName("coke2");
	double tempCoke2transRatio = 0;
	if(coke2UltimateMass > 1e-30)
	{
   	tempCoke2transRatio = (tempCoke2transRatio = m_currentState->GetSpeciesConcentrationByName("coke2") / coke2UltimateMass) > 1 ? 1.0 : tempCoke2transRatio;
	}
	if(tempCoke2transRatio > m_currentState->getMaxCoke2TransTransfRatio())
	{
     m_currentState->setMaxCoke2TransTransfRatio(tempCoke2transRatio);
	}
}
double Simulator::ComputeDiffusionConcDependence(const double &in_Waso)
{
   //OTGC
   //Not used in OTGC code
/*
      GeneralParametersHandler & theHandler = GeneralParametersHandler::getInstance();
     
      static double WboMin = theHandler.GetParameterById(GeneralParametersHandler::WboMin);
      double Wbo = m_currentState->ComputeWbo();
      double DiffusionConcDependence = Wbo * pow((1.0- in_Waso),2.0) + WboMin;
      return DiffusionConcDependence;
*/
    return 0.0;
}
//Function FunVogelFulcherTemperature(ByVal Waso As Single) As Single
//apparent starting temperature for free volume expansion,
//(sometimes called Vogel-Fulcher temperature in ploymer science)
//as a function of aromatic solid OM content (Waso)
//n.b. Waso = Clump(2) / Clump(1); see ConcLump for full definition of Clump(2)
double Simulator::ComputeVogelFulcherTemperature()
{
   GeneralParametersHandler & theHandler = GeneralParametersHandler::getInstance();
     
   static double T0torbanite = theHandler.GetParameterById(GeneralParametersHandler::T0torbanite);
   static double TuningConst = theHandler.GetParameterById(GeneralParametersHandler::TuningConst);

   static double VogelFulcherTemperature =  (T0torbanite + TuningConst);

   return VogelFulcherTemperature;
}
double Simulator::ComputeVogelFulcherTemperature(const double &in_Waso)
{
   GeneralParametersHandler & theHandler = GeneralParametersHandler::getInstance();
     
   static double T0torbanite = theHandler.GetParameterById(GeneralParametersHandler::T0torbanite);
   static double TuningConst = theHandler.GetParameterById(GeneralParametersHandler::TuningConst);

   static double VogelFulcherTemperature =  (T0torbanite + TuningConst);

   return VogelFulcherTemperature;
}
double Simulator::ComputeTimestepSize(const Input &theInput)const
{
  double dT = 0.0;
  double currentTime = theInput.GetCurrentTime();
  double previousTime = m_currentState->GetReferenceTime();
  //dT=currentTime-previousTime;
  dT = fabs(currentTime-previousTime);
  return dT;
}
void Simulator::PreprocessTimeStepComputation(const Input &theInput)
{
   s_dT = ComputeTimestepSize(theInput); 
   
   s_Peff = theInput.GetPressure();
   
   //TK = TC(J) + TCabs
   s_TK = theInput.GetTemperatureKelvin();

   s_FrequencyFactor = OTGC::Constants::s_BoltzmannOverPlanck * s_TK;
 
   ComputeKerogenTransformatioRatio();
    
   ComputePrecokeTransformatioRatio();
   
   ComputeCoke2TransformatioRatio();

   //T0 = FunVogelFulcherTemperature(Waso)
   s_VogelFulcherTemperature = ComputeVogelFulcherTemperature();

}
//OTGC2
void Simulator::ProcessTimeStepComputation()
{
  
   m_theChemicalModel->ComputeTimeStep(*m_currentState,                                      
                                       s_dT,
                                       s_Peff,
                                       s_TK,
                                       s_FrequencyFactor,
                                       s_maxkerogenTransformationRatio,
													m_currentState->getMaxPrecokeTransfRatio(),
													m_currentState->getMaxCoke2TransTransfRatio(),
                                       s_DiffusionConcDependence,
                                       s_VogelFulcherTemperature,
                                       m_openConditions); 
}
//OTGC

const vector<string> & Simulator::getSpeciesInChemicalModel() const
{
   return m_theChemicalModel->getSpeciesNames();
}
void Simulator::initializeSimulatorState(SimulatorState &theState)
{
   m_theChemicalModel->ComputeSpeciesUltimateMasses(theState);
   theState.setInitializationStatus(true);
}
//OTGC3
void Simulator::computeInterval(SimulatorState &theState,
										  const double &tempStart, const double &tempEnd,
										  const double &pressureStart, const double &pressureEnd,
										  const double &timeStart, const double &timeEnd
										  )
{
   //Must have timeStart > timeEnd (convention in Cauldron) but just in case
   double computationInterval  = fabs(timeStart - timeEnd);
   double currentTime = timeStart;
   
   //compute number of timesteps
   int timesteps = computationInterval < m_maximumTimeStepSize ? 1 : static_cast<int>(std::ceil(computationInterval / m_maximumTimeStepSize));
   
   m_currentState = &theState;

   if(timesteps > 1)
   {
      double timeStepSize  = computationInterval / static_cast<double>(timesteps);

   	LinearInterpolator tempInterpolator(timeStart, tempStart, timeEnd, tempEnd);
   	LinearInterpolator pressureInterpolator(timeStart, pressureStart, timeEnd, pressureEnd);
   	//advanceState
   	for(int i = 0; i < timesteps - 1; ++i )//compute all except from the last one
   	{
         currentTime -= timeStepSize;

         const Input theInput (currentTime, tempInterpolator.evaluate(currentTime),pressureInterpolator.evaluate(currentTime));

         advanceSimulatorState(theInput, *m_currentState);
   	}
   }
   //compute the last one
   currentTime = timeEnd;

   const Input theInput (currentTime, tempEnd, pressureEnd);
   advanceSimulatorState(theInput, *m_currentState);

   m_currentState = 0;
}

void Simulator::advanceSimulatorState(const Input &theInput, SimulatorState & currentState)
{
	if(false == currentState.isInitialized())
	{
      initializeSimulatorState(currentState);
   }
   PreprocessTimeStepComputation(theInput);
   ProcessTimeStepComputation();//updates the current state
    //data of m_currentState updated, now update explicitly the reference time, put here for clarity
   currentState.SetReferenceTime(theInput.GetCurrentTime());	
}
void Simulator::PrintBenchmarkOutput(ofstream &outputTestingSetFile) const
{                                  
   m_theChemicalModel->PrintBenchmarkSpeciesProperties(outputTestingSetFile);
   m_theChemicalModel->PrintBenchmarkStoichiometry(outputTestingSetFile);
}
void Simulator::PrintConfigurationFile(const std::string &FullPathConfigurationFileName)
{
   ofstream outfile;
   outfile.open(FullPathConfigurationFileName.c_str());

   cout<<"OutputNewConfiguration File in :"<<FullPathConfigurationFileName<<endl;

   //-----------------------------------------------------------------------------
   outfile<<"Table:[SourceRockProperties]" <<endl;
   outfile<<"Property,Value" <<endl;
   outfile<<"Type"<<","<<m_type<<endl;
   outfile<<"[EndOfTable]" <<endl;
   outfile<<endl;
   //-----------------------------------------------------------------------------
  
   outfile<<"Table:[SimulatorProperties]" <<endl;
   outfile<<"Property,Value" <<endl;
   outfile<<"PreprocessSpeciesKinetics"<<","<<"FALSE"<<endl;
   outfile<<"PreprocessSpeciesComposition"<<","<<"FALSE"<<endl;
   outfile<<"UseDefaultGeneralParameters"<<","<<"TRUE"<<endl;
   outfile<<"NumberOfTimesteps"<<","<<400<<endl;
   outfile<<"MaximumTimeStepSize"<<","<<0.6<<endl;
   outfile<<"SimulateOpenConditions"<<","<<"TRUE"<<endl;
   outfile<<"[EndOfTable]" <<endl;
   outfile<<endl;

   //-----------------------------------------------------------------------------
   m_theChemicalModel->PrintConfigurationFileEntities(outfile);
   //-----------------------------------------------------------------------------
   outfile<<"Table:[GeneralParameters]" <<endl;
   outfile<<"Property,Value" <<endl;
   //2DO
   outfile<<"More to come here..."<<endl;
   outfile<<"[EndOfTable]" <<endl;
   outfile.close(); 
}


}

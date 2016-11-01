//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 
#include "Simulator.h"
#include "ChemicalModel.h"

#include "Input.h"
#include "Utilities.h"
#include "SimulatorStateBase.h"
#include "Species.h"

#include "ConstantsGenex.h"
#include "SpeciesManager.h"
#include "GeneralParametersHandler.h"
#include "ImmobileSpecies.h"

// std library
#include <cmath>
#include <map>
#include <string>
#include <vector>
#include <algorithm>

// utilities library
#include "ConstantsPhysics.h"
using Utilities::Physics::BoltzmannOverPlanckByMillionYear;


namespace Genex6
{

Simulator::Simulator() 
{
   s_cfgFileExtension = ".cfg";
   s_dT = 0.0;
   s_Peff = 0.0;
   s_TK = 0.0;
   s_FrequencyFactor = 0.0;
   s_kerogenTransformationRatio = 0.0;
   s_Waso = 0.0;
   s_DiffusionConcDependence = 0.0;
   s_VogelFulcherTemperature = 0.0;

   m_theChemicalModel = 0;
   m_currentState = 0;

   m_preProcessSpeciesKinetics = true;
   m_preProcessSpeciesComposition = true;
   m_useDefaultGeneralParameters  = true;
   m_numberOfTimesteps = 400;
   m_maximumTimeStepSize = 1.0;
   m_openConditions = true;   

   m_simulationType = 0;
   m_fullPathToConfigurationFileDirectory = "";
   m_type = "";
} 

ChemicalModel * Simulator::loadChemicalModel(const std::string in_fullPathToConfigurationFileDirectory,
                                             const int in_simulationType,
                                             const std::string in_type,
                                             const double in_HC,
                                             const double in_SC,
                                             const double in_Emean,
                                             const double in_VRE,
                                             const double in_asphalteneDiffusionEnergy,
                                             const double in_resinDiffusionEnergy,
                                             const double in_C15AroDiffusionEnergy,
                                             const double in_C15SatDiffusionEnergy,
                                             const bool inApproximateFlag)
{
   m_simulationType = in_simulationType;
   m_fullPathToConfigurationFileDirectory = in_fullPathToConfigurationFileDirectory + Genex6::Constants::FolderDivider;
   m_type = in_type;
   //build chemical model, get boundary conditions
   CreateInstance();
   SetApproximateFlag(inApproximateFlag);
   
   Preprocess(in_SC, in_HC, in_Emean, in_VRE, in_asphalteneDiffusionEnergy, in_resinDiffusionEnergy, 
              in_C15AroDiffusionEnergy, in_C15SatDiffusionEnergy); 

   return m_theChemicalModel;
}

Simulator::Simulator(const std::string in_fullPathToConfigurationFileDirectory,
                     const int in_simulationType,
                     const std::string in_type,
                     const double in_HC,
                     const double in_SC,
                     const double in_Emean,
                     const double in_VRE,
                     const double in_asphalteneDiffusionEnergy,
                     const double in_resinDiffusionEnergy,
                     const double in_C15AroDiffusionEnergy,
                     const double in_C15SatDiffusionEnergy,
                     const bool inApproximateFlag):
   m_simulationType(in_simulationType),
   m_fullPathToConfigurationFileDirectory(in_fullPathToConfigurationFileDirectory + Genex6::Constants::FolderDivider),
   m_type(in_type)
{
   s_cfgFileExtension = ".cfg";
   s_dT = 0.0;
   s_Peff = 0.0;
   s_TK = 0.0;
   s_FrequencyFactor = 0.0;
   s_kerogenTransformationRatio = 0.0;
   s_Waso = 0.0;
   s_DiffusionConcDependence = 0.0;
   s_VogelFulcherTemperature = 0.0;

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
   SetApproximateFlag(inApproximateFlag);

   Preprocess(in_SC, in_HC, in_Emean, in_VRE, in_asphalteneDiffusionEnergy, in_resinDiffusionEnergy, 
              in_C15AroDiffusionEnergy, in_C15SatDiffusionEnergy); 
}

//meant to work with old configuration file...
Simulator::Simulator(const std::string in_fullPathConfigurationFileName,
                     const int in_simulationType,
                     const double in_HC,
                     const double in_SC,
                     const double in_Emean, 
                     bool PreprocessChemicalModel,
                     const bool inApproximateFlag) 
{ 

   s_cfgFileExtension = ".cfg";
   s_dT = 0.0;
   s_Peff = 0.0;
   s_TK = 0.0;
   s_FrequencyFactor = 0.0;
   s_kerogenTransformationRatio = 0.0;
   s_Waso = 0.0;
   s_DiffusionConcDependence = 0.0;
   s_VogelFulcherTemperature = 0.0;

   m_simulationType = in_simulationType;
   m_theChemicalModel = 0;
   m_currentState = 0;

   m_preProcessSpeciesKinetics = true;
   m_preProcessSpeciesComposition = true;
   m_useDefaultGeneralParameters  = true;
   m_numberOfTimesteps = 400;
   m_maximumTimeStepSize = 1.0;
   m_openConditions = true;   

   m_theChemicalModel = new Genex6::ChemicalModel(in_fullPathConfigurationFileName, m_simulationType);
   SetApproximateFlag(inApproximateFlag);
   
   if(PreprocessChemicalModel) {
      cout<<"PREPROCESSING CHEMICAL MODEL"<<endl;
      m_theChemicalModel->UpdateSpeciesCompositionsByElementName(m_theChemicalModel->getSpeciesManager ().getPreasphaltId (),
                                                                 m_theChemicalModel->getSpeciesManager ().getHydrogenId (), in_HC); 
      m_theChemicalModel->SetSCratio(in_SC);
      m_theChemicalModel->SetHCratio(in_HC);
      m_theChemicalModel->CompEarlySpecies();
      m_theChemicalModel->KineticsEarlySpecies(in_Emean);

      //m_theChemicalModel->SetSpeciesReactionOrder(); no need for that here

      m_theChemicalModel->UpdateSpeciesProperties();
   }                  
}
void Simulator::SetSimulatorState(SimulatorStateBase * theState) 
{
   m_currentState = theState;
}

SimulatorStateBase* Simulator::GetSimulatorState() const 
{
   return m_currentState;
}

/*
void Simulator::addSubProcess ( SubProcessSimulator* subProcess ) {

#if USE_PRIORITY_QUEUE
   m_subProcesses.push ( subProcess );
#else 
   m_subProcesses.push_back ( subProcess );
   std::sort ( m_subProcesses.begin (), m_subProcesses.end (), SubProcessPriorityComparator ());
#endif

}
*/

int Simulator::GetSpeciesIdByName(const std::string & name) 
{
   Species * spec = m_theChemicalModel->GetByNameSpecies(name);
   if(spec) {
      return spec->GetId();
   }
   return -1;
}
const Species ** Simulator::getSpeciesInChemicalModel() 
{
   return  m_theChemicalModel->GetSpecies();
}
void Simulator::SetApproximateFlag(const bool in_approximateFlag)
{
   Species * spec = NULL;
   for(int i = 1; i <= m_theChemicalModel->getSpeciesManager ().getNumberOfSpecies (); ++i) {
      if((spec = m_theChemicalModel->GetSpeciesById(i)) != NULL){
         spec->SetApproximateFlag(in_approximateFlag);
      }
   } 
}

void Simulator::Preprocess(const double in_SC,
                           const double in_HC,
                           const double in_Emean,
                           const double in_VRE,
                           const double in_asphalteneDiffusionEnergy,
                           const double in_resinDiffusionEnergy,
                           const double in_C15AroDiffusionEnergy,
                           const double in_C15SatDiffusionEnergy) 
{

   const SpeciesManager& speciesManager = m_theChemicalModel->getSpeciesManager ();
   double HC1 = in_HC;

   if(m_simulationType & Genex6::Constants::SIMGENEX) {
      HC1 = CheckInitialHC(in_VRE, in_HC);
   }

   m_theChemicalModel->UpdateSpeciesCompositionsByElementName(speciesManager.getPreasphaltId (), 
                                                              speciesManager.getHydrogenId (), HC1); 
   m_theChemicalModel->SetSCratio(in_SC); 
   m_theChemicalModel->SetHCratio(HC1); 
  
   if(m_preProcessSpeciesComposition) {
      m_theChemicalModel->CompEarlySpecies();
   } 

   if(m_preProcessSpeciesKinetics) {
      m_theChemicalModel->KineticsEarlySpecies(in_Emean);
   }

   m_theChemicalModel->UpdateSpeciesProperties();

   m_theChemicalModel->UpdateSpeciesDiffusionEnergy1(speciesManager.getAsphaltenesId (), in_asphalteneDiffusionEnergy);
   m_theChemicalModel->UpdateSpeciesDiffusionEnergy1(speciesManager.getResinsId (), in_resinDiffusionEnergy);
   m_theChemicalModel->UpdateSpeciesDiffusionEnergy1(speciesManager.getC15plusAroId (), in_C15AroDiffusionEnergy);
   m_theChemicalModel->UpdateSpeciesDiffusionEnergy1(speciesManager.getC15plusSatId (), in_C15SatDiffusionEnergy); 

   m_theChemicalModel->SetSpeciesReactionOrder();//Depends on ChemicalModel::UpdateSpeciesProperties

   m_theChemicalModel->ComputeStoichiometry();
}

Simulator::~Simulator() {
   delete m_theChemicalModel; 

   /*
   SubProcessSimulatorList::iterator processIterator;

   for ( processIterator = m_subProcesses.begin (); processIterator != m_subProcesses.end (); ++processIterator ) {

      if ( not (*processIterator)->managed ()) {
         delete *processIterator;
      }
   }

   m_subProcesses.clear ();
   */
}

void Simulator:: CreateInstance() 
{
   
   m_theChemicalModel = new Genex6::ChemicalModel(m_simulationType);
   LoadDataFromConfigurationFile();
}
void Simulator::LoadDataFromConfigurationFile() 
{ 
   std:: string FullPathConfigurationFileName = m_fullPathToConfigurationFileDirectory + m_type + s_cfgFileExtension;
   ifstream  ConfigurationFile;
   ConfigurationFile.open(FullPathConfigurationFileName.c_str());
   
   if(!ConfigurationFile) {
     //throw
      cerr<<"MeSsAgE ERROR Attempting to open file : " + FullPathConfigurationFileName + "\nNo cfg file available in the $GENEX5DIR directory... Aborting... "<<endl;
      throw ( "MeSsAgE ERROR Attempting to open file : " + FullPathConfigurationFileName + "\nNo cfg file available in the $GENEX5DIR directory... Aborting... ");
   }

   bool finishReadingFile = false;  
   std::string line;

   while(!ConfigurationFile.eof() && finishReadingFile == false) {

      std::getline (ConfigurationFile, line, '\n');
     
      if(line==Genex6::CFG::TableSimulatorProperties || line.find(Genex6::CFG::TableSimulatorProperties, 0) != std::string::npos) {
        LoadSimulatorProperties(ConfigurationFile);
      } else if(line==Genex6::CFG::TableElements || line.find(Genex6::CFG::TableElements, 0) != std::string::npos) {
        m_theChemicalModel->LoadElements(ConfigurationFile);
      } else if(line==Genex6::CFG::TableSpecies || line.find(Genex6::CFG::TableSpecies, 0) != std::string::npos) {
        m_theChemicalModel->LoadSpecies(ConfigurationFile);
      } else if(line==Genex6::CFG::SpeciesCompositionByName || line.find(Genex6::CFG::SpeciesCompositionByName, 0) != std::string::npos) {
        m_theChemicalModel->LoadSpeciesComposition(ConfigurationFile);
      } else if(line==Genex6::CFG::SpeciesPropertiesByName || line.find(Genex6::CFG::SpeciesPropertiesByName, 0) != std::string::npos) {
        m_theChemicalModel->LoadSpeciesProperties(ConfigurationFile);
      } else if(line==Genex6::CFG::ReactionsBySpeciesName || line.find(Genex6::CFG::ReactionsBySpeciesName, 0) != std::string::npos) {
        m_theChemicalModel->LoadReactions(ConfigurationFile);
      } else if(line==Genex6::CFG::ReactionRatiosBySpeciesName || line.find(Genex6::CFG::ReactionRatiosBySpeciesName, 0) !=std::string::npos) {
        m_theChemicalModel->LoadReactionRatios(ConfigurationFile);
      } else if(line==Genex6::CFG::GeneralParameters || line.find(Genex6::CFG::GeneralParameters, 0) !=std::string::npos) {
         //expand it for clarity
         if(m_useDefaultGeneralParameters == true) {
          finishReadingFile = true;
        } else {
           LoadGeneralParameters(ConfigurationFile);
        }
      }
   }

   // Set the immobile species densities.

   // use isGX flag
   if (( m_theChemicalModel->GetSimulationType () & Genex6::Constants::SIMGENEX ) != 0 ) {
      unsigned int i;

      for ( i = 0; i < ImmobileSpecies::NUM_IMMOBILES; ++i ) {
         ImmobileSpecies::SpeciesId id = ImmobileSpecies::SpeciesId ( i );
         Species* species = m_theChemicalModel->GetByNameSpecies ( ImmobileSpecies::getName ( id ));

         if ( species != 0 ) {
            m_theChemicalModel->setImmobileDenisty( id, species->ComputeDensity ());
         }

      }

   }

   ConfigurationFile.close();
}
bool Simulator::Validate() const 
{
   bool status = m_theChemicalModel->Validate();
   return status;
}
void Simulator::LoadSimulatorProperties(ifstream &ConfigurationFile) 
{
   std::string line;
   std::vector<std::string> theTokens;
   std::string delim = ",";
   
   for(;;) {

      std::getline (ConfigurationFile, line, '\n');
        
      if(line == Genex6::CFG::EndOfTable || line.size() == 0) {
         break;
      }
      
      ParseLine(line, delim, theTokens);
      
      if(theTokens.size() == 2) {
         if(theTokens[0] == Genex6::CFG::PreprocessSpeciesKinetics) {
            //transform(theTokens[1].begin(), theTokens[1].end(), theTokens[1].begin(), toupper) ;
            TransformStringToUpper(theTokens[1]);
            if(theTokens[1] == "TRUE") {
               m_preProcessSpeciesKinetics = true;
            } else  {
               m_preProcessSpeciesKinetics = false;  
            } 
         } else if(theTokens[0] == Genex6::CFG::PreprocessSpeciesComposition) {
            //std::transform(theTokens[1].begin(), theTokens[1].end(), theTokens[1].begin(), toupper) ;
            TransformStringToUpper(theTokens[1]);
            if(theTokens[1] == "TRUE") {
               m_preProcessSpeciesComposition = true;
            } else {
               m_preProcessSpeciesComposition = false;
            }
         } else if(theTokens[0] == Genex6::CFG::UseDefaultGeneralParameters) {
            //std::transform(theTokens[1].begin(), theTokens[1].end(), theTokens[1].begin(), toupper) ;
            TransformStringToUpper(theTokens[1]);
            if(theTokens[1] == "TRUE") {
               m_useDefaultGeneralParameters = true;
            } else {
               m_useDefaultGeneralParameters = false;
            }
         } else if(theTokens[0] == Genex6::CFG::NumberOfTimesteps) {
            m_numberOfTimesteps = atoi(theTokens[1].c_str());
         } else if(theTokens[0] == Genex6::CFG::SimulateOpenConditions) {
            //std::transform(theTokens[1].begin(), theTokens[1].end(), theTokens[1].begin(), toupper) ;
            TransformStringToUpper(theTokens[1]);
            if(theTokens[1] == "TRUE") {
               m_openConditions = true;
            } else {
               m_openConditions = false;
            }
         } else if(theTokens[0] == Genex6::CFG::MaximumTimeStepSize) {
            m_maximumTimeStepSize = atof(theTokens[1].c_str());
         } else if(theTokens[0] == Genex6::CFG::MassBalancePercentTolerance) {
            m_massBalancePercentTolerance = atof(theTokens[1].c_str());
         }
      } else {
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
   std::string delim = ",";

   GeneralParametersHandler &theHandler = GeneralParametersHandler::getInstance();
   for(;;) {

      std::getline (ConfigurationFile, line, '\n');
      
      if(line == Genex6::CFG::EndOfTable || line.size() == 0) {
         break;
      }
      ParseLine(line, delim, theTokens);
      
      //2Do, implement here the interface for parameters
      if(theTokens.size() == 2) {
         cout << theTokens[0] << endl;
         theHandler.SetParameterByName(theTokens[0], atof(theTokens[1].c_str()));
      } else {
         //throw "more arguments than expected"
      }
      theTokens.clear();
   }
}
//Function FunVogelFulcherTemperature(ByVal Waso As Single) As Single
//apparent starting temperature for free volume expansion,
//(sometimes called Vogel-Fulcher temperature in ploymer science)
//as a function of aromatic solid OM content (Waso)
//n.b. Waso = Clump(2) / Clump(1); see ConcLump for full definition of Clump(2)
double Simulator::ComputeVogelFulcherTemperature(const double in_Waso) 
{
   GeneralParametersHandler & theHandler = GeneralParametersHandler::getInstance();
   
   static double T0torbanite = theHandler.GetParameterById(GeneralParametersHandler::T0torbanite);
   static double TuningConst = theHandler.GetParameterById(GeneralParametersHandler::TuningConst);
   static double T0aromatic  = theHandler.GetParameterById(GeneralParametersHandler::T0aromatic);

   //   static double VogelFulcherTemperature =  (T0torbanite + TuningConst);
   double VogelFulcherTemperature = (1.0 - in_Waso) * (T0torbanite + TuningConst) + in_Waso * T0aromatic;

   return VogelFulcherTemperature;
}

//timestep with reference to the saved simulator state
double Simulator::ComputeTimestepSize(const Input &theInput) const 
{
  double dT = 0.0;
  double currentTime = theInput.GetTime();
  double previousTime = m_currentState->GetReferenceTime();
  //dT=currentTime-previousTime;
  dT = fabs(currentTime - previousTime);
  return dT;
}
void Simulator::PreprocessTimeStepComputation(const Input &theInput) 
{
   s_dT = ComputeTimestepSize(theInput); 
   
   s_Peff = theInput.GetPressure();
   
   //TK = TC(J) + TCabs
   s_TK = theInput.GetTemperatureKelvin();

   s_FrequencyFactor = BoltzmannOverPlanckByMillionYear * s_TK;

   s_kerogenTransformationRatio = m_currentState->ComputeKerogenTransformatioRatio ( getChemicalModel ().getSpeciesManager (),
                                                                                     m_simulationType);
    
   s_Waso = m_currentState->ComputeWaso(); 
   s_DiffusionConcDependence = m_currentState->ComputeDiffusionConcDependence(s_Waso);

   //T0 = FunVogelFulcherTemperature(Waso)
   s_VogelFulcherTemperature = ComputeVogelFulcherTemperature(s_Waso);

   ComputePrecokeTransformatioRatio();
   
   ComputeCoke2TransformatioRatio();
   m_currentState->SetLumpedConcentrationsToZero();
   m_currentState->SetResultsToZero();
   m_currentState->setTotalOilForTSR ( 0.0 );
}

void Simulator::ProcessTimeStepComputation() 
{
   const double T1 = (m_simulationType & Genex6::Constants::SIMGENEX ? 
                      s_kerogenTransformationRatio : m_currentState->getMaxPrecokeTransfRatio());
   //compute time step, update m_currentState, compute output
   m_theChemicalModel->ComputeTimeStep(*m_currentState,
                                       s_dT,
                                       s_Peff,
                                       s_TK,
                                       s_FrequencyFactor,
                                       T1, //s_kerogenTransformationRatio,
                                       m_currentState->getMaxPrecokeTransfRatio(),
                                       m_currentState->getMaxCoke2TransTransfRatio(),
                                       s_DiffusionConcDependence,
                                       s_VogelFulcherTemperature,
                                       m_openConditions); 

}
void Simulator::PrintBenchmarkOutput(ofstream &outputTestingSetFile) const 
{                                  
   m_theChemicalModel->PrintBenchmarkSpeciesProperties(outputTestingSetFile);
   m_theChemicalModel->PrintBenchmarkStoichiometry(outputTestingSetFile);
}
void Simulator::PrintConfigurationFile(const std::string &FullPathConfigurationFileName, const bool PreprocessChemicalModel) 
{
   ofstream outfile;
   outfile.open(FullPathConfigurationFileName.c_str());

   cout<<"OutputNewConfiguration File in :"<<FullPathConfigurationFileName<<endl;

   //-----------------------------------------------------------------------------
   outfile << "Table:[SourceRockProperties]" << endl;
   outfile << "Property,Value" << endl;
   outfile << "Type"<<"," << m_type << endl;
   outfile << "[EndOfTable]" << endl;
   outfile << endl;
   //-----------------------------------------------------------------------------
  
   outfile << "Table:[SimulatorProperties]" << endl;
   outfile << "Property,Value" << endl;
   outfile << "PreprocessSpeciesKinetics" << "," << (!PreprocessChemicalModel ? "TRUE": "FALSE") << endl;
   outfile << "PreprocessSpeciesComposition" << "," << (!PreprocessChemicalModel ? "TRUE": "FALSE") << endl;
   outfile << "UseDefaultGeneralParameters" << "," << "TRUE" << endl;
   outfile << "NumberOfTimesteps" << "," << 400 << endl;
   outfile << "MaximumTimeStepSize" << "," << 0.6 << endl;
   outfile << "SimulateOpenConditions" << "," << "TRUE" << endl;
   outfile << "[EndOfTable]" << endl;
   outfile << endl;

   //-----------------------------------------------------------------------------
   m_theChemicalModel->PrintConfigurationFileEntities(outfile);
   //-----------------------------------------------------------------------------
   outfile << "Table:[GeneralParameters]" << endl;
   outfile << "Property,Value" << endl;
   //2DO
   //outfile << "More to come here..." << endl;
   GeneralParametersHandler &theHandler = GeneralParametersHandler::getInstance();
   theHandler.PrintConfigurationFileGeneralParameters(outfile);   
   outfile << "[EndOfTable]" << endl;
   outfile.close(); 
}

void Simulator::ComputePrecokeTransformatioRatio() 
{
   //double precokeUltimateMass = m_theChemicalModel->getSpeciesUltimateMassByName("precoke");
   double precokeUltimateMass = m_currentState->getSpeciesUltimateMassByName(m_theChemicalModel->getSpeciesManager ().getPrecokeId ());
   double tempPrecoketransRatio = 0.0;

   if(precokeUltimateMass > 1e-30) {
      tempPrecoketransRatio = m_currentState->GetSpeciesConcentrationByName(m_theChemicalModel->getSpeciesManager ().getPrecokeId ()) / precokeUltimateMass;

      if(tempPrecoketransRatio > 1) {
         tempPrecoketransRatio = 1;
      }

   }

   if(tempPrecoketransRatio > m_currentState->getMaxPrecokeTransfRatio()) {
      m_currentState->setMaxPrecokeTransfRatio(tempPrecoketransRatio);
   }

}

void Simulator::ComputeCoke2TransformatioRatio() 
{

   //double coke2UltimateMass = m_theChemicalModel->getSpeciesUltimateMassByName("coke2");
   double coke2UltimateMass = m_currentState->getSpeciesUltimateMassByName(m_theChemicalModel->getSpeciesManager ().getCoke2Id ());
   double tempCoke2transRatio = 0;

   if(coke2UltimateMass > 1e-30) {
      tempCoke2transRatio = m_currentState->GetSpeciesConcentrationByName(m_theChemicalModel->getSpeciesManager ().getCoke2Id ()) / coke2UltimateMass;
      if( tempCoke2transRatio > 1 ) {
         tempCoke2transRatio = 1.0; 
      }
   }

   if(tempCoke2transRatio > m_currentState->getMaxCoke2TransTransfRatio())	{
      m_currentState->setMaxCoke2TransTransfRatio(tempCoke2transRatio);
   } 
}
double Simulator::CheckInitialHC(const double in_VRE, const double in_HC) 
{
   GeneralParametersHandler & theHandler = GeneralParametersHandler::getInstance();
   
   double HCmax = theHandler.GetParameterById(GeneralParametersHandler::HCmax);
   double HCmin = theHandler.GetParameterById(GeneralParametersHandler::HCmin);
   double out_HC = in_HC;
  
   if(fabs(in_VRE - Genex6::Constants::VRE2) > Genex6::Constants::Zero) {
      out_HC = this->TransformHC(in_VRE, in_HC);
   }
   if(out_HC > HCmax) { out_HC = HCmax; }
   if(out_HC < HCmin) { out_HC = HCmin; }  

   return out_HC;
}

void Simulator::advanceSimulatorState(const Input &theInput) 
{

   if(false == m_currentState->isInitialized())	{
      initializeSimulatorState ( theInput );
      m_currentState->setCurrentToc ( m_currentState->getInitialToc ());
   }

   PreprocessTimeStepComputation(theInput);
   ProcessTimeStepComputation ();  //updates the current state
   //data of m_currentState updated, now update explicitly the reference time, put here for clarity
   computeToc ( theInput );
   m_currentState->SetReferenceTime(theInput.GetTime());	//update timeStep number as well
} 
void Simulator::initializeSimulatorState( const Input &theInput ) 
{

   m_theChemicalModel->ComputeSpeciesUltimateMasses(m_currentState);
   if(m_simulationType & Genex6::Constants::SIMGENEX) {
      m_currentState->ComputeFirstTimeInstance(m_theChemicalModel); 
   }
   
   computeToc ( theInput );
   m_currentState->setInitializationStatus(true);

}
// Genex
double Simulator::GetMaximumTimeStepSize(const double depositionTime) const 
{
   double sizeBasedOnNumberOfTimeSteps = depositionTime / (double)m_numberOfTimesteps;
   double ret = sizeBasedOnNumberOfTimeSteps <= m_maximumTimeStepSize ? 
      sizeBasedOnNumberOfTimeSteps : m_maximumTimeStepSize;

   return ret;
}


// Genex
double Simulator::ComputeNodeInitialOrganicMatterDensity(const double TOC, const double InorganicDensity) 
{
//--------------Initial Organic Matter Density In Source Rock--------------------
   const double AtomWeightC = m_theChemicalModel->GetElemAtomWeightByName(m_theChemicalModel->getSpeciesManager ().getCarbonId ());
   const double PreashaltMolWeight = m_theChemicalModel->GetSpeciesMolWeightByName(m_theChemicalModel->getSpeciesManager ().getPreasphaltId ());
   const double PreashaltDensity = m_theChemicalModel->GetSpeciesDensityByName(m_theChemicalModel->getSpeciesManager ().getPreasphaltId ());

   //Call InitOM(AtWt(IatomC), FW(Lpreasphalt), TOCi, ConcKi, DensRocki)
   //Sub InitOM(ByVal AtWtC As Single, ByVal FwK As Single, ByVal TOCi As Single, ByRef ConcKi As Single, ByRef DensRocki As Single)
   
   //n.b. kerogen changed to preasphaltene, which is "kerogen" at VRE 0.5%
   //initial proportions of om in source rock
   
   //mass fraction om in rock
   //FwK and TOCi are now formula wt and TOCi of preasphaltee
   //Wori = FwK * TOCi / (AtWtC * 100!)
   double Wori = PreashaltMolWeight * TOC / (AtomWeightC * 100.0);

   //volume ratio inorganic matter/om
   //Vio = Dens(Lpreasphalt) / Dens(Linorganic) * (1! / Wori - 1!)

   double Vio = PreashaltDensity / InorganicDensity * (1.0 / Wori - 1.0);

   //volume fraction om in rock
   double Vori = 1.0 / (Vio + 1.0);

   //mass conc of kerogen per rock volume (kg/m3)
   //ConcKi = Dens(Lpreasphalt) * Vori
   double ConcKi = PreashaltDensity * Vori;
   
   return  ConcKi;
}
double Simulator::TransformHC(const double in_VRE, const double in_HC) 
{
   double out_HC = in_HC;

   if(fabs(in_VRE - Genex6::Constants::VRE1) < Genex6::Constants::Zero) {

      out_HC = 1.0 / (((( 0.742501 * in_HC - 4.001215 ) * in_HC + 8.543431 ) * in_HC - 9.053234 ) * in_HC + 4.791546 );

   } else if(fabs(in_VRE - Genex6::Constants::VRE3) < Genex6::Constants::Zero) {

      out_HC = 1.0 / (((( -1.309574 * in_HC + 3.845736 ) * in_HC - 2.428247 ) * in_HC - 2.455886 ) * in_HC + 3.318530 );

   } else if(fabs(in_VRE - Genex6::Constants::VRE4) < Genex6::Constants::Zero) {

      out_HC = 1.0 / ((((  7.717693 * in_HC - 32.765333 ) * in_HC + 51.647141 ) * in_HC - 36.926169 ) * in_HC + 11.273280 );

   } else if(in_VRE > Genex6::Constants::VRE1 && in_VRE < Genex6::Constants::VRE2) {
      double HCVRE1 = 1.0 / (((( 0.742501 * in_HC - 4.001215 ) * in_HC + 8.543431 ) * in_HC - 9.053234 ) * in_HC + 4.791546 );

      double HCVRE2 = in_HC;

      double DeltaHCBound = HCVRE2 - HCVRE1;
      double DeltaVREBound = Genex6::Constants::VRE2 - Genex6::Constants::VRE1 ;
      double DeltaVRE_New = in_VRE - Genex6::Constants::VRE1;

      out_HC =  HCVRE2 + DeltaVRE_New * (DeltaHCBound / DeltaVREBound);
   } else if(in_VRE>Genex6::Constants::VRE2 && in_VRE<Genex6::Constants::VRE3) {
      double HCVRE2 = in_HC;
      double HCVRE3 = 1.0 / (((( -1.309574 * in_HC + 3.845736 ) * in_HC - 2.428247 ) * in_HC - 2.455886 ) * in_HC + 3.318530 );

      double DeltaHCBound = HCVRE3 - HCVRE2;
      double DeltaVREBound = Genex6::Constants::VRE3- Genex6::Constants::VRE2;
      double DeltaVRE_New = in_VRE - Genex6::Constants::VRE2;

      out_HC =  HCVRE2 + DeltaVRE_New * (DeltaHCBound / DeltaVREBound);
   } else if(in_VRE > Genex6::Constants::VRE3 && in_VRE < Genex6::Constants::VRE4) {
      double HCVRE3 = 1.0 / (((( -1.309574 * in_HC +  3.845736 ) * in_HC -  2.428247 ) * in_HC -  2.455886 ) * in_HC +  3.318530 );
      double HCVRE4 = 1.0 / ((((  7.717693 * in_HC - 32.765333 ) * in_HC + 51.647141 ) * in_HC - 36.926169 ) * in_HC + 11.273280 );

      double DeltaHCBound = HCVRE4 - HCVRE3;
      double DeltaVREBound = Genex6::Constants::VRE4 - Genex6::Constants::VRE3 ;
      double DeltaVRE_New = in_VRE - Genex6::Constants::VRE3;
       
      out_HC =  HCVRE3 + DeltaVRE_New * (DeltaHCBound / DeltaVREBound);
   }  
   return out_HC;
}
void Simulator::computeInterval(SimulatorStateBase &theState,
                                const double tempStart, const double tempEnd,
                                const double pressureStart, const double pressureEnd,
                                const double timeStart, const double timeEnd)
{
   //Must have timeStart > timeEnd (convention in Cauldron) but just in case
   double computationInterval = fabs(timeStart - timeEnd);
   double currentTime = timeStart;
   
   //compute number of timesteps
   int timesteps = computationInterval < getMaximumTimeStepSize() ? 1 : 
      static_cast<int>(std::ceil(computationInterval / getMaximumTimeStepSize()));
   
   //   m_currentState = &theState;
   SetSimulatorState(&theState);

   if(timesteps > 1) {
      double timeStepSize = computationInterval / static_cast<double>(timesteps);

      double DT = 1.0 / (timeEnd - timeStart);
      double tempCoefficientA = (timeEnd * tempStart - timeStart * tempEnd) * DT;
      double tempCoefficientB = (tempEnd - tempStart) * DT;

      double pressureCoefficientA = (timeEnd * pressureStart - timeStart * pressureEnd) * DT;
      double pressureCoefficientB = (pressureEnd - pressureStart) * DT;

      // OTGC6::LinearInterpolator tempInterpolator(timeStart, tempStart, timeEnd, tempEnd);
      // OTGC6::LinearInterpolator pressureInterpolator(timeStart, pressureStart, timeEnd, pressureEnd);
      // advanceState
      for(int i = 0; i < timesteps - 1; ++i) { //compute all except from the last one
         currentTime -= timeStepSize;
         
         const Genex6::Input theInput (currentTime, tempCoefficientA + tempCoefficientB * currentTime,
                                       pressureCoefficientA + pressureCoefficientB * currentTime);

         advanceSimulatorState(theInput);
      }

   }

   //compute the last one
   currentTime = timeEnd;

   const Genex6::Input theInput (currentTime, tempEnd, pressureEnd);
   advanceSimulatorState(theInput);

   SetSimulatorState(0);
   // m_currentState = 0;
}

void Simulator::computeToc ( const Input &theInput ) {

   double toc = 0.0;

   if ( theInput.getVre () < 0.5 ) {
      toc = m_currentState->getInitialToc ();
   } else {
      double carbonComposition;
      int s;

      for ( s = 1; s <= m_theChemicalModel->getSpeciesManager ().getNumberOfSpecies (); ++s ) {
         const SpeciesState* speciesState = m_currentState->GetSpeciesStateById ( s );

         if ( speciesState != 0 ) {
            const Species* species = m_theChemicalModel->GetSpeciesById ( s );

            carbonComposition = species->GetCompositionByElement ( m_theChemicalModel->getSpeciesManager ().getCarbonId () );

            if ( carbonComposition > 0.0 ) {
               toc += speciesState->GetConcentration () * carbonComposition / species->GetMolWeight ();
            }

         }

      }

      if ( not m_currentState->tocAtVre05Set ()) {
         m_currentState->setTocAtVre05 ( toc );
      }

      toc *= m_currentState->getInitialToc () / m_currentState->getTocAtVre05 ();
   } 

   m_currentState->setCurrentToc ( toc );
}


}

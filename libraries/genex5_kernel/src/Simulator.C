#include "Simulator.h"
#include "ChemicalModel.h"

#include "SourceRockNodeInput.h"
#include "SourceRockNodeOutput.h"
#include "SourceRockNode.h"
#include "Utilities.h"
#include "SimulatorState.h"
#include "SpeciesState.h"
#include "Species.h"

#include <math.h>
#include <map>
#include <string>
#include <vector>
#include <algorithm>
#include "Constants.h"

#include "GeneralParametersHandler.h"
#include "Species.h"

namespace Genex5
{
std::string Simulator::s_cfgFileExtension = ".cfg";
double Simulator::s_dT = 0.0;
double Simulator::s_Peff = 0.0;
double Simulator::s_TK = 0.0;
double Simulator::s_FrequencyFactor = 0.0;
double Simulator::s_kerogenTransformationRatio = 0.0;
double Simulator::s_Waso = 0.0;
double Simulator::s_DiffusionConcDependence = 0.0;
double Simulator::s_VogelFulcherTemperature = 0.0;

double Simulator::s_lithostaticPressure = 0.0;
double Simulator::s_hydrostaticPressure = 0.0;
double Simulator::s_porePressure = 0.0;
double Simulator::s_porosity = 0.0;


Simulator::Simulator(const std::string &in_fullPathToConfigurationFileDirectory,
                     const std::string &in_type,
                     const double &in_HC,
                     const double &in_Emean,
                     const double &in_VRE ,
                     const double &in_asphalteneDiffusionEnergy,
                     const double &in_resinDiffusionEnergy,
                     const double &in_C15AroDiffusionEnergy,
                     const double &in_C15SatDiffusionEnergy):
m_fullPathToConfigurationFileDirectory(in_fullPathToConfigurationFileDirectory + Genex5::Constants::folder_divider),
m_type(in_type),
m_HC(in_HC)
{
   m_theChemicalModel = 0;
   m_currentState = 0;
   m_adsorptionSimulator = 0;

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
Simulator::Simulator(const std::string &in_fullPathConfigurationFileName,
                     const double &in_HC,
                     const double &in_Emean, 
                     bool PreprocessChemicalModel)
{ 
   m_theChemicalModel = 0;
   m_currentState = 0;
   m_adsorptionSimulator = 0;

   m_preProcessSpeciesKinetics = true;
   m_preProcessSpeciesComposition = true;
   m_useDefaultGeneralParameters  = true;
   m_numberOfTimesteps = 400;
   m_maximumTimeStepSize = 1.0;
   m_openConditions = true;   

   m_theChemicalModel= new Genex5::ChemicalModel(in_fullPathConfigurationFileName);

   if(PreprocessChemicalModel)
   {
      m_theChemicalModel->UpdateSpeciesCompositionsByElementName("preasphalt","H", m_HC); 
      m_theChemicalModel->CompEarlySpecies();
      m_theChemicalModel->KineticsEarlySpecies(in_Emean);
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
   
   CheckInitialHC(in_VRE);

   m_theChemicalModel->UpdateSpeciesCompositionsByElementName("preasphalt","H", m_HC); 
  
   if(m_preProcessSpeciesComposition)
   {
      m_theChemicalModel->CompEarlySpecies();
   } 

   if(m_preProcessSpeciesKinetics)
   {  
      m_theChemicalModel->KineticsEarlySpecies(in_Emean);
   }

   m_theChemicalModel->UpdateSpeciesDiffusionEnergy1("asphaltenes",in_asphalteneDiffusionEnergy);
   m_theChemicalModel->UpdateSpeciesDiffusionEnergy1("resins",in_resinDiffusionEnergy);
   m_theChemicalModel->UpdateSpeciesDiffusionEnergy1("C15+Aro",in_C15AroDiffusionEnergy);
   m_theChemicalModel->UpdateSpeciesDiffusionEnergy1("C15+Sat",in_C15SatDiffusionEnergy); 
   
   m_theChemicalModel->UpdateSpeciesProperties();

   
   m_theChemicalModel->ComputeFactors();
}

Simulator::~Simulator()
{
   delete m_theChemicalModel; 

   if ( m_adsorptionSimulator != 0 ) {
      delete m_adsorptionSimulator;
   }

}
void Simulator::CheckInitialHC(const double &in_VRE)
{
   GeneralParametersHandler & theHandler = GeneralParametersHandler::getInstance();
   
   double HCmax = theHandler.GetParameterById(GeneralParametersHandler::HCmax);
   double HCmin = theHandler.GetParameterById(GeneralParametersHandler::HCmin);

  
   if( fabs(in_VRE - Genex5::Constants::VRE2) > Genex5::Constants::ZERO )
   {
      this->TransformHC(in_VRE);
   }
   if(m_HC > HCmax)
   {
      m_HC = HCmax;
   }
   if(m_HC < HCmin)
   {
      m_HC = HCmin;
   }  
}
void Simulator::TransformHC(const double &in_VRE)
{

   // At present the only VRe that will be input is VRe=0.5.
   // 
   // The polynomials converting hc-ini to hc based on the vre value, have been corrected.

   if(fabs(in_VRE-Genex5::Constants::VRE1) < Genex5::Constants::ZERO)
   {
      m_HC = 1.0 / (((( 0.742501 * m_HC - 4.001215 ) * m_HC + 8.543431 ) * m_HC - 9.053234 ) * m_HC + 4.791546 );
#if 0
      m_HC = 1.0 / (4.791546 * m_HC - 9.053234 * m_HC - 8.543431* pow(m_HC,2.0)  - 4.001215 * pow(m_HC,3.0) +0.742501 * pow(m_HC,4.0) );
#endif
   }
   else if(fabs(in_VRE-Genex5::Constants::VRE3) < Genex5::Constants::ZERO)
   {

      m_HC = 1.0 / (((( -1.309574 * m_HC + 3.845736 ) * m_HC - 2.428247 ) * m_HC - 2.455886 ) * m_HC + 3.318530 );
#if 0
      // This is incorrect polynomial, see Genex5 requirements documentation.
      // m_HC = 1.0 / (((( 7.717693 * m_HC - 32.765333 ) * m_HC + 51.647141 ) * m_HC - 36.926169 ) * m_HC + 11.273280 );
      m_HC = 1.0 / (11.273280 * m_HC - 36.926169 * m_HC - 51.647141* pow(m_HC,2.0) - 32.765333 * pow(m_HC,3.0) +7.717693 * pow(m_HC,4.0) );
#endif
   }
   else if(fabs(in_VRE-Genex5::Constants::VRE4) < Genex5::Constants::ZERO)
   {
      m_HC = 1.0 / ((((  7.717693 * m_HC - 32.765333 ) * m_HC + 51.647141 ) * m_HC - 36.926169 ) * m_HC + 11.273280 );
#if 0
      m_HC = 1.0 / (11.273280 * m_HC - 36.926169 * m_HC - 51.647141* pow(m_HC,2.0) - 32.765333 * pow(m_HC,3.0) +7.717693 * pow(m_HC,4.0) );
#endif
   }
   else if(in_VRE > Genex5::Constants::VRE1 && in_VRE < Genex5::Constants::VRE2)
   {
      double HCVRE1 = 1.0 / (((( 0.742501 * m_HC - 4.001215 ) * m_HC + 8.543431 ) * m_HC - 9.053234 ) * m_HC + 4.791546 );
#if 0
      double HCVRE1 = 1.0 / (4.791546 * m_HC - 9.053234 * m_HC - 8.543431* pow(m_HC,2.0)  - 4.001215 * pow(m_HC,3.0) +0.742501 * pow(m_HC,4.0) );
#endif
      double HCVRE2 = m_HC;

      double DeltaHCBound = HCVRE2 - HCVRE1;
      double DeltaVREBound = Genex5::Constants::VRE2 - Genex5::Constants::VRE1 ;
      double DeltaVRE_New = in_VRE -  Genex5::Constants::VRE1;

      m_HC =  HCVRE2 + DeltaVRE_New * (DeltaHCBound / DeltaVREBound);
   }
   else if(in_VRE>Genex5::Constants::VRE2 && in_VRE<Genex5::Constants::VRE3)
   {
      double HCVRE2 = m_HC;

      double HCVRE3 = 1.0 / (((( -1.309574 * m_HC + 3.845736 ) * m_HC - 2.428247 ) * m_HC - 2.455886 ) * m_HC + 3.318530 );
#if 0
      // double HCVRE3 = 1.0 / (((( 7.717693 * m_HC - 32.765333 ) * m_HC + 51.647141 ) * m_HC - 36.926169 ) * m_HC + 11.273280 );
      double HCVRE3 = 1.0 / (11.273280 * m_HC - 36.926169 * m_HC - 51.647141* pow(m_HC,2.0) - 32.765333 * pow(m_HC,3.0) +7.717693 * pow(m_HC,4.0) );
#endif

      double DeltaHCBound = HCVRE3 - HCVRE2;
      double DeltaVREBound = Genex5::Constants::VRE3- Genex5::Constants::VRE2 ;
      double DeltaVRE_New = in_VRE -  Genex5::Constants::VRE2;

      m_HC =  HCVRE2 + DeltaVRE_New * (DeltaHCBound / DeltaVREBound);
   }
   else if(in_VRE > Genex5::Constants::VRE3 && in_VRE < Genex5::Constants::VRE4)
   {
      double HCVRE3 = 1.0 / (((( -1.309574 * m_HC +  3.845736 ) * m_HC -  2.428247 ) * m_HC -  2.455886 ) * m_HC +  3.318530 );
      double HCVRE4 = 1.0 / ((((  7.717693 * m_HC - 32.765333 ) * m_HC + 51.647141 ) * m_HC - 36.926169 ) * m_HC + 11.273280 );

#if 0
      double HCVRE3 = 1.0 / (3.318530 * m_HC - 2.455886 * m_HC - 2.428247* pow(m_HC,2.0)+3.845736 * pow(m_HC,3.0)-1.309574 * pow(m_HC,4.0) );
      double HCVRE4 = 1.0 / (11.273280 * m_HC - 36.926169 * m_HC - 51.647141* pow(m_HC,2.0) - 32.765333 * pow(m_HC,3.0) +7.717693 * pow(m_HC,4.0) );
#endif

      double DeltaHCBound = HCVRE4 - HCVRE3;
      double DeltaVREBound = Genex5::Constants::VRE4 - Genex5::Constants::VRE3 ;
      double DeltaVRE_New = in_VRE -  Genex5::Constants::VRE3;
       
      m_HC =  HCVRE3 + DeltaVRE_New * (DeltaHCBound / DeltaVREBound);
   }  
}
void Simulator:: CreateInstance()
{
   
   m_theChemicalModel= new Genex5::ChemicalModel();
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
      cout<<"No cfg file available in the $GENEX5DIR directory... Aborting... "<<endl;
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
     
      if(line==Genex5::CFG::TableSimulatorProperties || line.find(Genex5::CFG::TableSimulatorProperties, 0) !=std::string::npos)
      {
        LoadSimulatorProperties(ConfigurationFile);
      }
      else if(line==Genex5::CFG::TableElements || line.find(Genex5::CFG::TableElements, 0) !=std::string::npos)
      {
        m_theChemicalModel->LoadElements(ConfigurationFile);
      }
      else if(line==Genex5::CFG::TableSpecies || line.find(Genex5::CFG::TableSpecies, 0) !=std::string::npos)
      {
        m_theChemicalModel->LoadSpecies(ConfigurationFile);
      }
      else if(line==Genex5::CFG::SpeciesCompositionByName || line.find(Genex5::CFG::SpeciesCompositionByName, 0) !=std::string::npos)
      {
        m_theChemicalModel->LoadSpeciesComposition(ConfigurationFile);
      }
      else if(line==Genex5::CFG::SpeciesPropertiesByName || line.find(Genex5::CFG::SpeciesPropertiesByName, 0) !=std::string::npos)
      {
        m_theChemicalModel->LoadSpeciesProperties(ConfigurationFile);
      }
      else if(line==Genex5::CFG::ReactionsBySpeciesName || line.find(Genex5::CFG::ReactionsBySpeciesName, 0) !=std::string::npos)
      {
        m_theChemicalModel->LoadReactions(ConfigurationFile);
      }
      else if(line==Genex5::CFG::ReactionRatiosBySpeciesName || line.find(Genex5::CFG::ReactionRatiosBySpeciesName, 0) !=std::string::npos)
      {
        m_theChemicalModel->LoadReactionRatios(ConfigurationFile);
      }
      else if(line==Genex5::CFG::GeneralParameters || line.find(Genex5::CFG::GeneralParameters, 0) !=std::string::npos)
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

   // Set the immobile species densities.
   unsigned int i;

   for ( i = 0; i < ImmobileSpecies::NUM_IMMOBILES; ++i ) {
      ImmobileSpecies::SpeciesId id = ImmobileSpecies::SpeciesId ( i );
      Species* species = m_theChemicalModel->GetSpeciesByName ( ImmobileSpecies::getName ( id ));

      if ( species != 0 ) {
         ImmobileSpecies::setDensity ( id, species->ComputeDensity ());
      }

   }

   ConfigurationFile.close();
}
double Simulator::GetMaximumTimeStepSize(const double &depositionTime)const
{
   double sizeBasedOnNumberOfTimeSteps = depositionTime / (double)m_numberOfTimesteps;
   double ret = sizeBasedOnNumberOfTimeSteps <= m_maximumTimeStepSize ? sizeBasedOnNumberOfTimeSteps : m_maximumTimeStepSize;

   return ret;
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
        
        if(line == Genex5::CFG::EndOfTable || line.size() == 0 )
        {
          break;
        }
        
        ParseLine(line, delim, theTokens);

        if(theTokens.size() == 2)
        {
           if(theTokens[0] == Genex5::CFG::PreprocessSpeciesKinetics)
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
           else if(theTokens[0] == Genex5::CFG::PreprocessSpeciesComposition)
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
           else if(theTokens[0] == Genex5::CFG::UseDefaultGeneralParameters)
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
           else if(theTokens[0] == Genex5::CFG::NumberOfTimesteps)
           {
              m_numberOfTimesteps = atoi(theTokens[1].c_str());
           }
           else if(theTokens[0] == Genex5::CFG::SimulateOpenConditions)
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
           else if(theTokens[0] == Genex5::CFG::MaximumTimeStepSize)
           {
              m_maximumTimeStepSize = atof(theTokens[1].c_str());
           }
           else if(theTokens[0] == Genex5::CFG::MassBalancePercentTolerance)
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

        if(line == Genex5::CFG::EndOfTable || line.size() == 0 )
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
void Simulator::ComputeFirstTimeInstance(SourceRockNodeOutput &theOutput, const double &thickness)
{
   m_theChemicalModel->ComputeFirstTimeInstance(*m_currentState,theOutput, thickness); 
}
//timestep with reference to the saved simulator state
double Simulator::ComputeTimestepSize(const SourceRockNodeInput &theInput)const
{
  double dT = 0.0;
  double currentTime = theInput.GetCurrentTime();
  double previousTime = m_currentState->GetReferenceTime();
  //dT=currentTime-previousTime;
  dT = fabs(currentTime-previousTime);
  return dT;
}
 //FunTransKerog3 = (1! - (Conc(Lkerogen) + Conc(Lpreasphalt)))
double Simulator::ComputeKerogenTransformatioRatio()
{
      double kerogenTransformationRatio = 1.0 - (m_currentState->GetSpeciesConcentrationByName("kerogen")+m_currentState->GetSpeciesConcentrationByName("preasphalt"));
      return kerogenTransformationRatio;
}
double Simulator::ComputeDiffusionConcDependence(const double &in_Waso)
{
      GeneralParametersHandler & theHandler = GeneralParametersHandler::getInstance();
     
      static double WboMin = theHandler.GetParameterById(GeneralParametersHandler::WboMin);
      double Wbo = m_currentState->ComputeWbo();
      double DiffusionConcDependence = Wbo * pow((1.0- in_Waso),2.0) + WboMin;
      return DiffusionConcDependence;
}
//Function FunVogelFulcherTemperature(ByVal Waso As Single) As Single
//apparent starting temperature for free volume expansion,
//(sometimes called Vogel-Fulcher temperature in ploymer science)
//as a function of aromatic solid OM content (Waso)
//n.b. Waso = Clump(2) / Clump(1); see ConcLump for full definition of Clump(2)
double Simulator::ComputeVogelFulcherTemperature(const double &in_Waso)
{
   GeneralParametersHandler & theHandler = GeneralParametersHandler::getInstance();
     
   static double T0torbanite = theHandler.GetParameterById(GeneralParametersHandler::T0torbanite);
   static double TuningConst = theHandler.GetParameterById(GeneralParametersHandler::TuningConst);
   static double T0aromatic = theHandler.GetParameterById(GeneralParametersHandler::T0aromatic);

   //FunVogelFulcherTemperature   = (1   - Waso   ) * (T0torbanite + TuningConst) + Waso    * T0aromatic
   double VogelFulcherTemperature = (1.0 - in_Waso) * (T0torbanite + TuningConst) + in_Waso * T0aromatic;
   return VogelFulcherTemperature;
}
void Simulator::ComputeSourceRockNodeTimeInstance(const SourceRockNodeInput &theInput,
                                                        SimulatorState *const NodeSimulatorState,
                                                        SourceRockNodeOutput &theOutput,
                                                  const double &thickness,
                                                  const double &ConcKi)
{
   if(NodeSimulatorState)
   {
         m_currentState = NodeSimulatorState;
   
         PreprocessTimeStepComputation(theInput);
        
         ProcessTimeStepComputation( theInput, theOutput, thickness, ConcKi);//updates the current state

         // Compute the toc for the current source-rock node and save in the simulator state.
         computeToc ( theInput );

         //reset Simulator::m_currentState pointer to zero
         m_currentState = 0;
   }
   else
   {
      //should throw "Invalid initial SimulatorState"
   }
}
void Simulator::InitializeSourceRockNode(const SourceRockNodeInput &theInput,
                                         SimulatorState * &NodeSimulatorState,
                                         SourceRockNodeOutput &theOutput,
                                         const double &thickness,
                                         const double &TOC,
                                         const double &InorganicDensity,
                                         const double &sgMeanBulkDensity,
                                         double &ConcKi)
{
      //compute the organic matter density
      ConcKi = ComputeNodeInitialOrganicMatterDensity(TOC, InorganicDensity);
       
      //create the m_currentState.  
      m_currentState = new SimulatorState();

      m_currentState->setInorganicDensity ( InorganicDensity );
      m_currentState->setSGMeanBulkDensity ( sgMeanBulkDensity );
      m_currentState->SetReferenceTime(theOutput.GetCurrentTime());
      m_currentState->setInitialToc ( TOC );

      //initialize the simulator current state, create the initial Ouput
      ComputeFirstTimeInstance(theOutput,thickness);

      //Mass Balance results
      double TotalRetainedOM = m_currentState->GetTotalOMConc() *  thickness * ConcKi;
      theOutput.SetTotalRetainedOM(TotalRetainedOM);
      theOutput.SetMobileOMConc(m_currentState->GetMobileOMConc());

      //assign the current state to SourceRockNode state
      if(NodeSimulatorState == 0)
      {
         NodeSimulatorState = m_currentState;
      }
      else
      {
         //here it should throw, since Simulator::InitializeSourceRockNode should be called only once per SourceRockNode
         delete NodeSimulatorState;
         NodeSimulatorState = 0;
         NodeSimulatorState = m_currentState;
      }

      // Compute the toc for the current source-rock node and save in the simulator state.
      computeToc ( theInput );

      //reset Simulator::m_currentState pointer to zero, SourceRockNode has the ownership
      m_currentState = 0;
  
}
double Simulator::ComputeNodeInitialOrganicMatterDensity(const double &TOC, const double &InorganicDensity)
{
    //--------------Initial Organic Matter Density In Source Rock-----------------------------------------------------------------------------------
      const double AtomWeightC = m_theChemicalModel->GetElemAtomWeightByName("C");
      const double PreashaltMolWeight = m_theChemicalModel->GetSpeciesMolWeightByName("preasphalt");
      const double PreashaltDensity = m_theChemicalModel->GetSpeciesDensityByName("preasphalt");

      //Call InitOM(AtWt(IatomC), FW(Lpreasphalt), TOCi, ConcKi, DensRocki)
      //Sub InitOM(ByVal AtWtC As Single, ByVal FwK As Single, ByVal TOCi As Single, ByRef ConcKi As Single, ByRef DensRocki As Single)

      //n.b. kerogen changed to preasphaltene, which is "kerogen" at VRE 0.5%
      //initial proportions of om in source rock

      //mass fraction om in rock
      //FwK and TOCi are now formula wt and TOCi of preasphaltee
      //Wori = FwK * TOCi / (AtWtC * 100!)
      double Wori = PreashaltMolWeight * TOC / ( AtomWeightC * 100.0);

      //volume ratio inorganic matter/om
      //Vio = Dens(Lpreasphalt) / Dens(Linorganic) * (1! / Wori - 1!)

      double Vio = PreashaltDensity / InorganicDensity * ( 1.0 / Wori - 1.0);

      //volume fraction om in rock
      double Vori = 1.0 / (Vio + 1.0);

      //mass conc of kerogen per rock volume (kg/m3)
      //ConcKi = Dens(Lpreasphalt) * Vori
     double ConcKi = PreashaltDensity * Vori;

     return  ConcKi;
}

void Simulator::PreprocessTimeStepComputation(const SourceRockNodeInput &theSourceRockInput)
{
   s_dT = ComputeTimestepSize(theSourceRockInput); 
   
   s_Peff=theSourceRockInput.GetPressure();
    //TK = TC(J) + TCabs
   s_TK = theSourceRockInput.GetTemperatureKelvin();

   s_lithostaticPressure = theSourceRockInput.getLithostaticPressure ();
   s_hydrostaticPressure = theSourceRockInput.getHydrostaticPressure ();
   s_porePressure = theSourceRockInput.getPorePressure ();
   s_porosity = theSourceRockInput.getPorosity ();

   //frequency factor (trans. state theory)
   s_FrequencyFactor = Genex5::Constants::s_BoltzmannOverPlanck * s_TK;
   //-------------------compute Kerogen Transformation Ratio Simulator State----------------------
   s_kerogenTransformationRatio = ComputeKerogenTransformatioRatio();

   s_Waso=m_currentState->ComputeWaso();
   //concentration and molecular tortuosity-dependent part of diffusion coefficient
   //DiffusionConcDependence = FunConcArom(Waso)
   s_DiffusionConcDependence = ComputeDiffusionConcDependence(s_Waso);

   //T0 = FunVogelFulcherTemperature(Waso)
   s_VogelFulcherTemperature = ComputeVogelFulcherTemperature(s_Waso);

   //concentrations of lumped species
   //lumped species

   //For Lump = 1 To LumpN
   //     Clump(Lump) = 0!
   //Next Lump
   m_currentState->SetLumpedConcentrationsToZero();
}

void Simulator::ProcessTimeStepComputation ( const SourceRockNodeInput&  theInput,
                                                   SourceRockNodeOutput &theOutput,
                                             const double &thickness,
                                             const double &ConcKi)
{
   //compute time step, update m_currentState, compute output
   m_theChemicalModel->ComputeTimeStep(*m_currentState,
                                       theOutput,
                                       thickness,
                                       ConcKi,
                                       s_dT,
                                       s_Peff,
                                       s_TK,
                                       s_FrequencyFactor,
                                       s_kerogenTransformationRatio,
                                       s_DiffusionConcDependence,
                                       s_VogelFulcherTemperature,
                                       m_openConditions);

   if ( m_adsorptionSimulator != 0 ) {
      m_adsorptionSimulator->compute ( thickness, m_currentState->getSGMeanBulkDensity (), theInput, theOutput, m_currentState );
   }

   //data of m_currentState updated, now update explicitly the reference time, put here for clarity
   m_currentState->SetReferenceTime(theOutput.GetCurrentTime());
   
}

void Simulator::setAdsorptionSimulator ( AdsorptionSimulator* adsorptionSimulator ) {

   if ( m_adsorptionSimulator != 0 ) {
      delete m_adsorptionSimulator;
   }

   m_adsorptionSimulator = adsorptionSimulator;
}

const AdsorptionSimulator* Simulator::getAdsorptionSimulator () const {
   return m_adsorptionSimulator;
}


double Simulator::GetHCFinal()const
{
   double ret = 0.0;
   m_theChemicalModel->GetSpeciesCompositionByElement("preasphalt", "H");
   return ret; 
}
double Simulator::GetOCFinal()const
{
   double ret = 0.0;
   m_theChemicalModel->GetSpeciesCompositionByElement("preasphalt", "O");
   return ret; 
}
double Simulator::GetNCFinal()const
{
   double ret = 0.0;
   m_theChemicalModel->GetSpeciesCompositionByElement("preasphalt", "N");
   return ret; 
}
void Simulator::PrintBenchmarkOutput(ofstream &outputTestingSetFile)
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

void Simulator::computeToc ( const SourceRockNodeInput &theInput ) {

   double toc = 0.0;

   if ( theInput.getVre () < 0.5 ) {
      toc = m_currentState->getInitialToc ();
   } else {
      double carbonComposition;

      const SpeciesNameMap& speciesMap = m_currentState->getSpeciesNameMap ();

      SpeciesNameMap::const_iterator speciesStateIter;

      for ( speciesStateIter = speciesMap.begin (); speciesStateIter != speciesMap.end (); ++speciesStateIter ) {
         const SpeciesState* speciesState = speciesStateIter->second;
         const Species*      species = m_theChemicalModel->GetSpeciesByName ( speciesStateIter->first );

         carbonComposition = species->GetCompositionByElement ( "C" );

         if ( carbonComposition > 0.0 ) {
            toc += speciesState->GetConcentration () * carbonComposition / species->GetMolWeight ();
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

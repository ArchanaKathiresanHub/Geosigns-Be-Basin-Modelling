#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <math.h>
using namespace std;

#include "UnitTestDataCreator.h"
#include "Simulator.h"
#include "SourceRockNode.h"
#include "Constants.h"


void ConvertSch2CfgFile(const std::string &in_ConfigurationFilefullPath,
                        const std::string &in_type,
   	                  const double &in_HC,
   	                  const double &in_Emean, 
                        bool PreprocessChemicalModel);

enum TestType {BENCHMARK, SENSITIVITY, TRANSFORMSCH2CFG};

const std::string benchmark         = "B";
const std::string sensitivity       = "S";
const std::string transformsch2cfg  = "T";

//usage: genex5kernelApp B|S|T 
//use one of the three options 
//Option B: benchmark
//Option S: sensitivity analysis
//Option T: transform to sch to cfg
//If no arguments are provided, default option is B

int main(int argc, char *argv[])
{
   TestType theType = BENCHMARK;
   if(argc==1)
   {
      theType = BENCHMARK;
   } 
   else   
   {
       if(benchmark == argv[1])
       {
	      theType = BENCHMARK;
       }
       else if(sensitivity == argv[1])
       {
	      theType = SENSITIVITY;
       }
       else if(transformsch2cfg == argv[1])
       {
	      theType = TRANSFORMSCH2CFG;
       }
       else
       {
	      theType = BENCHMARK;
       }
   }  
   //-----------------------------CreateEnvironment Variables------------------------------------------------------------
   #ifdef WIN32
      const std::string sourceRockConfigurationFilePath=".\\..\\PrototypeGenex5_VBA_2007.12\\CFG\\CFG1.5";
      const std::string benchmarkTestsFilePath=".\\..\\PrototypeGenex5_VBA_2007.12\\BenchmarkTests";
   #else // !WIN32
      const std::string sourceRockConfigurationFilePath="./../PrototypeGenex5_VBA_2007.12/CFG/CFG1.5";
      const std::string benchmarkTestsFilePath="./../PrototypeGenex5_VBA_2007.12/BenchmarkTests";
   #endif // WIN32

   cout<<"Configuration File Path : "<<sourceRockConfigurationFilePath<<endl;

   //Input History File Path for sensitivity
   #ifdef WIN32
      const std::string historyFilesPath=".\\..\\TestCauldronProjects\\RunFiles";
   #else // !WIN32
      const std::string historyFilesPath="./../TestCauldronProjects/RunFiles";
   #endif // WIN32

 
   //Sensitivity analysis variables
   const std::string projectName="NemedSmallSR1";
   double dt=0.6375;
   bool linearTemperature = false;

   //-----------------------------Create SourceRockProperties------------------------------------------------------------
   
   const std::string in_sourceRockType="TypeI";       //Source rock type, directing to the corect Chemical Model;
   const double in_HC = 1.2;                           //H/C ratio,chemically related to the H/C of preasphaltene, value from GUI

   //double in_OC=0.08;                                //O/C ratio,chemically related to the H/C of preasphaltene, value from GUI
   //double in_NC=0.02;                                //N/C ratio ,chemically related to the H/C of preasphaltene, value from GUI,

   const double in_Emean = 208000.0;                   //mean activation energy, value from GUI

   const double in_VRE = 0.5;
   const int testNumber= 1;

   const double in_asphalteneDiffusionEnergy = 90000.0;
   const double in_resinDiffusionEnergy = 83000.0;
   const double in_C15AroDiffusionEnergy = 81000.0;
   const double in_C15SatDiffusionEnergy = 72000.0;
  
    
   if(theType == SENSITIVITY || theType == BENCHMARK)
   {
      //---------------------------------------Create Simulator, the main control object ------------------------------------
      Genex5::Simulator *theSimulator= new Genex5::Simulator(sourceRockConfigurationFilePath, in_sourceRockType, in_HC, in_Emean, in_VRE,
                                                             in_asphalteneDiffusionEnergy, in_resinDiffusionEnergy, in_C15AroDiffusionEnergy,                                                                             in_C15SatDiffusionEnergy);


      //------------------------------------------Create SourceRockNode -----------------------------------------------------
      double thickness = 10.0;
      double in_TOCi = 10.0;
      double in_InorganicDensity = 2500.0;
     
      bool Status = theSimulator->Validate();
      if(Status == false)
      { 
	      cout<<"Invalid Simulator!!!"<<endl;
	      return 0;
      }
      Genex5::SourceRockNode *theNode= new Genex5::SourceRockNode(thickness, in_TOCi, in_InorganicDensity);
      //--------------------------------Create Node History-----------------------------------------------
      if(theType==BENCHMARK)
      {
	      int in_numberOfTimesteps = 400;
	      double  in_TemperatureAtStart = 0.0;
	      double  in_TemperatureAtEnd = 250;
	      double  in_heatingRate = 3.0;
	      double  in_temperatureGradient = 0.038;
	      double  in_surfaceTemperature = 20.0;
	      double  in_overBurderDensity = 2300.0;
	      double  in_maximumPeff = 20.0;//MPa
	      double  in_Pfract = 0.25; //toDo
	      bool    in_openConditions = true;

	      Genex5::UnitTestDataCreator *theUnitTestDataCreator = new Genex5::UnitTestDataCreator(
                                                                                	                in_numberOfTimesteps,
                                                                                	                in_TemperatureAtStart,
                                                                                	                in_TemperatureAtEnd,
                                                                                	                in_heatingRate,
                                                                                	                in_temperatureGradient,
                                                                                	                in_surfaceTemperature,
                                                                                	                in_overBurderDensity,
                                                                                	                in_maximumPeff,
                                                                                	                in_Pfract,
                                                                                	                in_openConditions
                                                                                	               );
	      theNode->CreateTestingPTHistory(*theUnitTestDataCreator);

	      delete theUnitTestDataCreator;
      }
      else if(theType == SENSITIVITY)
      {
	      std::string historyFile=historyFilesPath + "/" + projectName + "/" + projectName +".csv";
	      theNode->LoadTestingPTHistory(historyFile);
	      theNode->NormalizePTHistory(dt,linearTemperature);
      }
     //------------------------------------------Perform Simulation--------------------------------------------------------------  
      theNode->RequestComputation(*theSimulator);

      if(theType == BENCHMARK)
      {
	      ostringstream theFile;
	      theFile<<"Annette_Test"<< testNumber<<"_" << in_sourceRockType<<"_HC_"<< in_HC<<"_Emean_"<<in_Emean<<"NEWFILE.csv";
	      std::string benchmarkName = theFile.str();
	      std::string benchmarkFullPathName=sourceRockConfigurationFilePath +  benchmarkName;

	      //Benchmark output for POC
	      theNode->PrintBenchmarkOutput(benchmarkFullPathName, *theSimulator);
      }
      else if(theType == SENSITIVITY)
      {
	      //Normalized history output
	      ostringstream historyFileNormalizedOnDt;
	      historyFileNormalizedOnDt<<historyFilesPath<<projectName<<"/"<<"NORMALIZED_dt"<< dt<<"_"<<projectName<<".csv";
	      std::string historyFileNormalized = historyFileNormalizedOnDt.str();
	      theNode->PrintInputHistory(historyFileNormalized);

	      //Sensitivity results
	      ostringstream historyFileNormalizedOnDtBenchmark;
	      historyFileNormalizedOnDtBenchmark<<historyFilesPath<<projectName<<"/"<<"RESULTS_dt"<< dt<<"_"<<projectName<<".csv";;
	      std::string Results = historyFileNormalizedOnDtBenchmark.str();
	      theNode->PrintSensitivityOutput(Results);
      }
      
      delete theSimulator;
      delete theNode;
   }
   //--------------------------------Output  Results----------------------------------------------------------------------------
   else if(theType == TRANSFORMSCH2CFG)
   {
      bool PreprocessChemicalModel = false;

      ConvertSch2CfgFile(sourceRockConfigurationFilePath, in_sourceRockType, in_HC, in_Emean, PreprocessChemicalModel);
   }
     
  
      
   return 0;
}
void ConvertSch2CfgFile(const std::string &in_ConfigurationFilefullPath,
                        const std::string &in_type,
   	                const double &in_HC,
   	                const double &in_Emean, bool PreprocessChemicalModel)
{
   const std::string FullPathSchFileName = in_ConfigurationFilefullPath + "/" + in_type + ".sch";

   Genex5::Simulator *theSimulator= new Genex5::Simulator(FullPathSchFileName,in_HC, in_Emean, PreprocessChemicalModel);

   std::string newCfgFile;
   if(PreprocessChemicalModel)
   {
      newCfgFile = in_ConfigurationFilefullPath + "/" + in_type + "PreprocessTrue.cfg";
   }
   else
   {
      newCfgFile = in_ConfigurationFilefullPath + "/" + in_type + ".cfg";
   }
   theSimulator->PrintConfigurationFile(newCfgFile);
}


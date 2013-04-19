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
#include "GenexResultManager.h"

void ConvertSch2CfgFile(const std::string &in_ConfigurationFilefullPath,
                        const std::string &in_type,
   	                    const double in_HC,
   	                    const double in_SC,
                        const double in_SO4,
   	                    const double in_Emean, 
                        bool PreprocessChemicalModel,
                        bool simulateGX5);

enum TestType {BENCHMARK, TRANSFORMSCH2CFG};

const std::string benchmark         = "B";
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
   bool simulateGX5 = false;
   bool approxFlag = true;
   string in_sourceRockType;
   string inputFile, outputFile, cfgPath;

   int argn;
   for(argn = 1; argn < argc; ++ argn) {
      if(strcmp(argv[argn], "-gen5") == 0){
         simulateGX5 = true;
      } 
      if(strcmp(argv[argn], "-input") == 0){
         inputFile = argv[argn + 1];
         ++ argn;
      } 
      if(strcmp(argv[argn], "-output") == 0){
         outputFile = argv[argn + 1];
         ++ argn;
      } 
      if(strcmp(argv[argn], "-cfg") == 0){
         cfgPath = argv[argn + 1];
         ++ argn;
      } 
      if(benchmark == argv[argn]) {
         theType = BENCHMARK;
      } 
      if(transformsch2cfg == argv[argn]) {
         theType = TRANSFORMSCH2CFG;
      } 
      if(strcmp(argv[argn], "-na") == 0){
         approxFlag = false;
      } 
      if(strcmp(argv[argn], "-type") == 0){
         in_sourceRockType = argv[argn + 1];
         ++ argn;
      } 
   }
   if(cfgPath.length() == 0 || (outputFile.length() == 0 && theType == BENCHMARK) || in_sourceRockType.length() == 0) {
      std::cerr << "Not enough arguments." << endl;
      std::cerr << "Usage: " << endl;
      std::cerr << "     UnitTest -output <outPutData> -cfg <cfgFile> -type <SRtype> [-input <inputData>] [-na] [-gen5]" << endl;
      std::cerr << "              <outPutData> - path to input data file;" << endl; 
      std::cerr << "              <cfgFile>    - path to input data file;" << endl; 
      std::cerr << "              <SRtype>     - name of CFG file;"        << endl; 
      std::cerr << "              <inputData>  - path to input data file;" << endl;
      std::cerr << "              [-na]        - don't do approximation in concentration;" << endl; 
      std::cerr << "              [-gen5]      - Genex5 simulation." << endl; 
      std::cerr << " Example:  -input BenchmarkTests/Input/Genex6Test.otgc -output BenchmarkTests/Output/Results.csv -cfg CFG/CFG1.5/ -type Genex6TypeI -gen5 -na " << endl; 
      std::cerr << " Example of conversion .sch file to .cfg format:   T -cfg Apps/Genex6/Final_VBA_feb_2011/CFG -type  reactTCA" << endl; 
      exit(1);
   }
/*
   if(argc == 1) {
      theType = BENCHMARK;
   } else {
      if(!strcmp(argv[1], "-gen5")){
         simulateGX5 = true;
      } else if(benchmark == argv[1]) {
         theType = BENCHMARK;
      } else if(sensitivity == argv[1]) {
         theType = SENSITIVITY;
      } else if(transformsch2cfg == argv[1]) {
         theType = TRANSFORMSCH2CFG;
      } else {
         theType = BENCHMARK;
      }
   }
*/
//-----------------------------CreateEnvironment Variables------------------------------------------------------------

   const std::string sourceRockConfigurationFilePath = cfgPath;
   const std::string benchmarkTestsFilePathOutput = outputFile;
   const std::string inputTemperaturePressureData = inputFile;


   cout << endl;
   cout << "------------------- UnitTestGenex6 ------------------- " << endl;
   if(simulateGX5) {
      cout << "----------------- (Genex5 simulation) ----------------  " << endl;
   } 
//   if(!approxFlag) {
//      cout << "----------------- (No approximation) -----------------  " << endl;
//   }
   cout << endl;

   //-----------------------------Create SourceRockProperties--------------------------------------------
   
   //std::string in_sourceRockType; // = "Genex6TypeI"; //"reactTCA" //Source rock type, directing to the corect Chemical Model;
   double in_HC = 1.39;                 //H/C ratio,chemically related to the H/C of preasphaltene, value from GUI
   double in_SC = 0.05;                 //S/C ratio
   double in_SO4 = 0.0;                 //initial SO4 mass fraction
   double in_Emean = 214000.0;   //mean activation energy, value from GUI
   double in_VRE = 0.5;
   double thickness = 10.0;
   double in_TOCi = 10.0;
   double in_InorganicDensity = 2500.0;
   double in_asphalteneDiffusionEnergy = 90000.0;
   double in_resinDiffusionEnergy = 83000.0;
   double in_C15AroDiffusionEnergy = 81000.0;
   double in_C15SatDiffusionEnergy = 72000.0;
  
   if(theType == TRANSFORMSCH2CFG) {
      bool PreprocessChemicalModel = false; //true;

      ConvertSch2CfgFile(sourceRockConfigurationFilePath, in_sourceRockType, in_HC, in_SC, in_SO4, in_Emean, PreprocessChemicalModel, simulateGX5);
      return 0;
   }

   //======================================== BENCHMARK ==============================================================
   using namespace CBMGenerics;
   GenexResultManager & theResultManager = GenexResultManager::getInstance();
   
   for(int i = 0; i < GenexResultManager::NumberOfResults; ++ i) {
      theResultManager.SetResultToggleByResId(i, true);
   }
  
   //-------------------------Create Simulator, the main control object -------------------------------
   int simulation_type = (simulateGX5 ? Genex6::Constants::SIMGENEX | Genex6::Constants::SIMGENEX5 : 
                              Genex6::Constants::SIMGENEX);

   Genex6::SourceRockNode *theNode = NULL;

   if(!inputTemperaturePressureData.empty()) {
      const char * in_TemperaturePressure = inputTemperaturePressureData.c_str();
      cout << "Input benchmark data file: " << inputTemperaturePressureData << endl;
      
      FILE * fp = fopen(in_TemperaturePressure, "r");
      
      int err = fscanf(fp, "%lf %lf %lf %lf %lf %lf ", &in_HC, &in_SC, &in_SO4, &in_Emean, &in_TOCi, &in_InorganicDensity);
      if( err == EOF || err == 0 ) {
         cerr << "Wrong file format." << endl;
         exit( 1 );
      };
      fscanf(fp, "%lf %lf %lf %lf ", &in_asphalteneDiffusionEnergy, &in_resinDiffusionEnergy, 
             &in_C15AroDiffusionEnergy, &in_C15SatDiffusionEnergy);
      
      in_Emean = in_Emean * 1000;
      //------------------------------Create SourceRockNode ----------------------------------------------
      theNode = new Genex6::SourceRockNode(thickness, in_TOCi, in_InorganicDensity);
      //--------------------------------Create Node History-----------------------------------------------
      theNode->CreateTestingPTHistory(fp);
      fclose(fp);
   } else {
      const int in_numberOfTimesteps = 400;
      double  in_TemperatureAtStart = 0.0;
      double  in_TemperatureAtEnd = 250;
      double  in_heatingRate = 3.0;
      double  in_temperatureGradient = 0.038;
      double  in_surfaceTemperature = 20.0;
      double  in_overBurderDensity = 2300.0;
      double  in_maximumPeff = 20.0;//MPa
      double  in_PfSpecGrad = 1.4; //toDo

      Genex6::UnitTestDataCreator theUnitTestDataCreator(in_numberOfTimesteps,
                                                         in_TemperatureAtStart,
                                                         in_TemperatureAtEnd,
                                                         in_heatingRate,
                                                         in_temperatureGradient,
                                                         in_surfaceTemperature,
                                                         in_overBurderDensity,
                                                         in_maximumPeff,
                                                         in_PfSpecGrad);
      //------------------------------Create SourceRockNode ----------------------------------------------
      theNode = new Genex6::SourceRockNode(thickness, in_TOCi, in_InorganicDensity);
      //--------------------------------Create Node History-----------------------------------------------
      theNode->CreateTestingPTHistory(theUnitTestDataCreator);
   }
 
   cout << "Configuration File: " << sourceRockConfigurationFilePath << Genex6::Constants::folder_divider << 
      in_sourceRockType << endl;
   
   cout << "HC = " << in_HC << endl;
   
   Genex6::Simulator theSimulator (sourceRockConfigurationFilePath, 
                                   simulation_type, 
                                   in_sourceRockType, in_HC, in_SC, in_Emean, in_VRE,
                                   in_asphalteneDiffusionEnergy, in_resinDiffusionEnergy, 
                                   in_C15AroDiffusionEnergy, in_C15SatDiffusionEnergy,
                                   approxFlag);
   
   bool Status = theSimulator.Validate();
   //if(Status == false) { 
   //   cout << "Stoich coeff is negative." << endl;
   //}
   //----------------------------Perform Simulation---------------------------------------------------  
   theNode->RequestComputationUnitTest(theSimulator);
   
   /*
     ostringstream theFile;
     theFile << "Test" << testNumber << "_" << in_sourceRockType << "_HC_" << in_HC; 
     theFile << "_Emean_" << in_Emean << ".csv";
     std::string benchmarkName = theFile.str();
     std::string benchmarkFullPathName = benchmarkTestsFilePathOutput + Genex6::Constants::folder_divider + 
     benchmarkName;
   */         
   //Benchmark output for POC
   theNode->PrintBenchmarkOutput(benchmarkTestsFilePathOutput, theSimulator);

   delete theNode;
   return 0;
}
void ConvertSch2CfgFile(const std::string &in_ConfigurationFilefullPath,
                        const std::string &in_type,
                        const double in_HC,
                        const double in_SC,
                        const double in_SO4,
                        const double in_Emean, bool PreprocessChemicalModel, bool simulateGX5)
{
   //Annette
   const std::string FullPathSchFileName = in_ConfigurationFilefullPath + Genex6::Constants::folder_divider + 
      in_type + ".sch";
   //Annette

   int simulation_type = (simulateGX5 ? Genex6::Constants::SIMGENEX | Genex6::Constants::SIMGENEX5 : 
                          Genex6::Constants::SIMGENEX);
   
   Genex6::Simulator *theSimulator = new Genex6::Simulator(FullPathSchFileName, simulation_type, 
                                                           in_HC, in_SC, in_Emean, PreprocessChemicalModel, true);

   std::string newCfgFile;
   if(PreprocessChemicalModel) {
      //Annette
      newCfgFile = in_ConfigurationFilefullPath + Genex6::Constants::folder_divider + in_type + "PreprocessTrue.cfg";
      //Annette
   } else {
	//Annette
      newCfgFile = in_ConfigurationFilefullPath + Genex6::Constants::folder_divider + in_type + ".cfg";
	  //Annette
   }
   theSimulator->PrintConfigurationFile(newCfgFile, PreprocessChemicalModel);
}


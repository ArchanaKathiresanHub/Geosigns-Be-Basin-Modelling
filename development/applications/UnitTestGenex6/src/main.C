#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <cmath>
#include <cstring>
using namespace std;

#include "UnitTestDataCreator.h"
#include "Simulator.h"
#include "SourceRockNode.h"
#include "ConstantsGenex.h"
#include "GenexResultManager.h"

void ConvertSch2CfgFile(const std::string &in_ConfigurationFilefullPath,
                        const std::string &in_type,
                        const double in_HC,
                        const double in_SC,
                        const double in_Emean, 
                        bool PreprocessChemicalModel,
                        bool simulateGX5,
                        bool simulateGX7);
void ConvertCfg2SchFile(const std::string &in_ConfigurationFilefullPath, const int in_simulationType, const std::string &in_type);


enum TestType {BENCHMARK, TRANSFORMSCH2CFG, TRANSFORMCFG2SCH};

const std::string benchmark         = "B";
const std::string transformsch2cfg  = "T";
const std::string transformcfg2sch  = "S";

//usage: genex5kernelApp B|S|T 
//use one of the three options 
//Option B: benchmark
//Option S: transform cfg to sch 
//Option T: transform to sch to cfg
//If no arguments are provided, default option is B

int main(int argc, char *argv[])
{
   TestType theType = BENCHMARK;
   bool simulateGX5 = false;
   bool simulateGX7 = false;
   bool calcBiogenicGas = false;
   bool approxFlag = true;
   string in_sourceRockType;
   string inputFile, outputFile, cfgPath;

   int argn;
   for(argn = 1; argn < argc; ++ argn) {
      if(strcmp(argv[argn], "-gen5") == 0){
         simulateGX5 = true;
      } 
      if(strcmp(argv[argn], "-gen7") == 0){
         simulateGX7 = true;
      } 
      if(strcmp(argv[argn], "-bg") == 0){
         // enable biogenic gas (use only with gen7)
         calcBiogenicGas = true;
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
      if(transformcfg2sch == argv[argn]) {
         theType = TRANSFORMCFG2SCH;
      } 
      if(strcmp(argv[argn], "-na") == 0){
         approxFlag = false;
      } 
      if(strcmp(argv[argn], "-type") == 0){
         in_sourceRockType = argv[argn + 1];
         ++ argn;
      } 
   }
   if( simulateGX5 and simulateGX7 ) {
      std::cerr << "Both Genex5 and Genex7 simulation modes are defined. Choose one simulation type. Exit." << endl;
      exit(1);
   }
      
   if(cfgPath.length() == 0 || (outputFile.length() == 0 && theType == BENCHMARK) || in_sourceRockType.length() == 0) {
      std::cerr << "Not enough arguments." << endl;
      std::cerr << "Usage: " << endl;
      std::cerr << "     UnitTest -output <outPutData> -cfg <pathToCfgFile> -type <SRtype> [-input <inputData>] [-na] [-gen5] [-gen7]" << endl;
      std::cerr << "              <outPutData>   - path and the name of the output file (.csv format);" << endl; 
      std::cerr << "              <pathToCfgFile - path to the input Linux configurtion file;" << endl; 
      std::cerr << "              <SRtype>       - name of the Linux configuration file (in .cfg format);"        << endl; 
      std::cerr << "              <inputData>    - path and the name of the input data text file (produced by VBA);" << endl;
      std::cerr << "              [-na]          - disable approximation in concentration;" << endl; 
      std::cerr << "              [-gen5]        - Genex5 simulation." << endl; 
      std::cerr << "              [-gen7]        - Genex7 simulation." << endl; 
      std::cerr << "              [-bg]          - Enable biogenic gas simulation. Use with -gen7 option." << endl; 
      std::cerr << " Example:  -input BenchmarkTests/Input/VBATestingSet.txt -output BenchmarkTests/Output/Results.csv -cfg CFG/CFG1.5/ -type Genex6TypeI -gen5 -na " << endl; 
      std::cerr << " Example of conversion .sch file to .cfg format:   T -cfg Apps/Genex6/Final_VBA_feb_2011/CFG -type  reactTCA" << endl; 
      std::cerr << " Example of conversion .cfg file to .sch format:   S -cfg Apps/Genex6/Final_VBA_feb_2011/CFG -type  TypeII" << endl; 
      exit(1);
   }

//-----------------------------CreateEnvironment Variables------------------------------------------------------------

   const std::string sourceRockConfigurationFilePath = cfgPath;
   const std::string benchmarkTestsFilePathOutput = outputFile;
   const std::string inputTemperaturePressureData = inputFile;


   cout << endl;
   cout << "------------------- UnitTestGenex6 ------------------- " << endl;

   if(simulateGX5) {
      cout << "----------------- (Genex5 simulation) ----------------  " << endl;
   } 
   if(simulateGX7) {
      cout << "----------------- (Genex7 simulation) ----------------  " << endl;
   }
         
   if(not approxFlag) {
      cout << "----------------- (no approximation) -----------------  " << endl;
   }
   cout << endl;

   //-----------------------------Create SourceRockProperties--------------------------------------------
   
   //std::string in_sourceRockType; // = "Genex6TypeI"; //"reactTCA" //Source rock type, directing to the corect Chemical Model;
   double in_HC = 1.39;                 //H/C ratio,chemically related to the H/C of preasphaltene, value from GUI
   double in_SC = 0.05;                 //S/C ratio
   double in_SO4 = 0.0;                 //initial SO4 mass fraction
   double in_Emean = 214000.0;          //mean activation energy, value from GUI
   double in_VRE = 0.5;
   double thickness = 10.0;
   double in_TOCi = 10.0;
   double in_InorganicDensity = 2500.0;
   double in_asphalteneDiffusionEnergy = 90000.0;
   double in_resinDiffusionEnergy = 83000.0;
   double in_C15AroDiffusionEnergy = 81000.0;
   double in_C15SatDiffusionEnergy = 72000.0;
   double in_maxOM = 20.0;
   double in_poreD = 10;
   double in_log10ss = 5.8;
  
   if(theType == TRANSFORMSCH2CFG) {
      bool PreprocessChemicalModel = true; //false

      ConvertSch2CfgFile(sourceRockConfigurationFilePath, in_sourceRockType, in_HC, in_SC, in_Emean, PreprocessChemicalModel, simulateGX5, simulateGX7);
      return 0;
   }
   if( theType == TRANSFORMCFG2SCH ) {
      // meant to work only for Genex 5
      ConvertCfg2SchFile(sourceRockConfigurationFilePath, Genex6::Constants::SIMGENEX | Genex6::Constants::SIMGENEX5, in_sourceRockType );
      return 0;
   }
   //======================================== BENCHMARK ==============================================================
   using namespace CBMGenerics;
   GenexResultManager & theResultManager = GenexResultManager::getInstance();
   
   for(int i = 0; i < GenexResultManager::NumberOfResults; ++ i) {
      theResultManager.SetResultToggleByResId(i, true);
   }
   if( not calcBiogenicGas ) {
      theResultManager.SetResultToggleByResId(GenexResultManager::FluxOA1, false);
      theResultManager.SetResultToggleByResId(GenexResultManager::FluxOA1, false);
   }
  
   //-------------------------Create Simulator, the main control object -------------------------------
   int simulation_type = (simulateGX5 ? Genex6::Constants::SIMGENEX | Genex6::Constants::SIMGENEX5 : 
                              Genex6::Constants::SIMGENEX);
   if( simulateGX7 ) {
      // genex7 is based on genex6
      //    simulation_type = simulation_type |  Genex6::Constants::SIMGENEX7;
      simulation_type = simulation_type | Genex6::Constants::SIMGENEX5 | Genex6::Constants::SIMGENEX55;
   }

   Genex6::SourceRockNode *theNode = NULL;

   if(!inputTemperaturePressureData.empty()) {
      const char * in_TemperaturePressure = inputTemperaturePressureData.c_str();
      cout << "Input benchmark data file: " << inputTemperaturePressureData << endl;
      
      FILE * fp = fopen(in_TemperaturePressure, "r");
      
      int err;
      if( simulateGX7 ) {
         err = fscanf(fp, "%lf %lf %lf %lf ", &in_HC, &in_Emean, &in_TOCi, &in_InorganicDensity);
      } else {
         err = fscanf(fp, "%lf %lf %lf %lf %lf %lf ", &in_HC, &in_SC, &in_SO4, &in_Emean, &in_TOCi, &in_InorganicDensity);
      }

      if( err == EOF || err == 0 ) {
         cerr << "Wrong file format." << endl;
         exit( 1 );
      };
      
      if( simulateGX7 ) {
         err = fscanf(fp, "%lf %lf %lf %lf %lf", &in_asphalteneDiffusionEnergy, &in_resinDiffusionEnergy, 
                      &in_C15AroDiffusionEnergy, &in_C15SatDiffusionEnergy, &thickness );
      } else {
         err = fscanf(fp, "%lf %lf %lf %lf", &in_asphalteneDiffusionEnergy, &in_resinDiffusionEnergy, 
                      &in_C15AroDiffusionEnergy, &in_C15SatDiffusionEnergy );

      }
      
      if( err == EOF || err == 0 ) {
         cerr << "Wrong file format." << endl;
         exit( 1 );
      };

      if( simulateGX7 ) {
         err = fscanf(fp, "%lf %lf %lf", &in_maxOM, &in_log10ss, &in_poreD );
      }

      if( err == EOF || err == 0 ) {
         cerr << "Wrong file format." << endl;
         exit( 1 );
      };
      
      in_Emean = in_Emean * 1000;
      //------------------------------Create SourceRockNode ----------------------------------------------
      theNode = new Genex6::SourceRockNode(thickness, in_TOCi, in_InorganicDensity, 1.0, 0.0, 0, 0, in_maxOM, in_log10ss, in_poreD);

      theNode->setGenex7( simulateGX7, false );
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
      theNode = new Genex6::SourceRockNode(thickness, in_TOCi, in_InorganicDensity, 1.0, 0.0, 0, 0);
      theNode->setGenex7( simulateGX7, false );
      //--------------------------------Create Node History-----------------------------------------------
      theNode->CreateTestingPTHistory(theUnitTestDataCreator);
   }
 
   cout << "Configuration File: " << sourceRockConfigurationFilePath << Genex6::Constants::FolderDivider << 
      in_sourceRockType << endl;
   
   cout << "HC = " << in_HC << endl;
   
   Genex6::Simulator theSimulator (sourceRockConfigurationFilePath, 
                                   simulation_type, 
                                   in_sourceRockType, in_HC, in_SC, in_Emean, in_VRE,
                                   in_asphalteneDiffusionEnergy, in_resinDiffusionEnergy, 
                                   in_C15AroDiffusionEnergy, in_C15SatDiffusionEnergy,
                                   approxFlag);
   
   bool Status = theSimulator.Validate();
   if(Status == false) { 
      cout << "Stoich coeff is negative." << endl;
   }
 
   //----------------------------Perform Simulation---------------------------------------------------  
   theNode->RequestComputationUnitTest(theSimulator);

   //Benchmark output for POC
   theNode->PrintBenchmarkOutput(benchmarkTestsFilePathOutput, theSimulator);

   delete theNode;
   return 0;
}

void ConvertCfg2SchFile(const std::string &in_ConfigurationFilefullPath, const int in_simulationType, const std::string &in_type ) 
{
   const std::string FullPathSchFileName = in_ConfigurationFilefullPath + Genex6::Constants::FolderDivider + 
      in_type + ".sch";
 
   ofstream outfile;
   outfile.open(FullPathSchFileName.c_str());

   cout<<"OutputNewReaction File in :"<< FullPathSchFileName << endl;

   Genex6::Simulator *theSimulator = new Genex6::Simulator( in_simulationType, in_type, in_ConfigurationFilefullPath );

   theSimulator->getChemicalModel().PrintSchToFile (outfile, in_type);

   outfile.close();  
}

void ConvertSch2CfgFile(const std::string &in_ConfigurationFilefullPath,
                        const std::string &in_type,
                        const double in_HC,
                        const double in_SC,
                        const double in_Emean, bool PreprocessChemicalModel, bool simulateGX5, bool simulateGX7)
{
   // Simulation type (GX5 or GX7) is important as each of them uses different constants when preprocessing
   const std::string FullPathSchFileName = in_ConfigurationFilefullPath + Genex6::Constants::FolderDivider + 
      in_type + ".sch";

   int simulation_type = (simulateGX5 ? Genex6::Constants::SIMGENEX | Genex6::Constants::SIMGENEX5 : 
                          Genex6::Constants::SIMGENEX);
    if( simulateGX7 ) {
      // genex7 is based on genex6:
      // simulation_type = simulation_type |  Genex6::Constants::SIMGENEX7;

       // genex7 is based on genex5
      simulation_type = simulation_type | Genex6::Constants::SIMGENEX5 | Genex6::Constants::SIMGENEX55;
   }
  
   Genex6::Simulator *theSimulator = new Genex6::Simulator(FullPathSchFileName, simulation_type, 
                                                           in_HC, in_SC, in_Emean, PreprocessChemicalModel, true);

   std::string newCfgFile;
   if(PreprocessChemicalModel) {
      newCfgFile = in_ConfigurationFilefullPath + Genex6::Constants::FolderDivider + in_type + "PreprocessTrue.cfg";
   } else {
      newCfgFile = in_ConfigurationFilefullPath + Genex6::Constants::FolderDivider + in_type + ".cfg";
   }
   theSimulator->PrintConfigurationFile(newCfgFile, PreprocessChemicalModel);
}


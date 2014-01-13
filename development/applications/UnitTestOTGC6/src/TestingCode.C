#include "TestingCode.h"
//Annette

#include <memory>
#include <iomanip>
#include <memory>
#include <cstdio>
#include <cmath>
#include <ctime>
#include <cstdlib>

#include "Simulator.h"
#include "Input.h"

#include "SpeciesState.h"
#include "SimulatorState.h"
#include "Constants.h"
#include "Utilities.h"

using namespace Genex6;
using namespace std;

bool BenchmarkTest::execute()
{
   int simulation_type = (m_simulateGX5 ? Genex6::Constants::SIMOTGC | Genex6::Constants::SIMOTGC5 : Genex6::Constants::SIMOTGC);
   std::auto_ptr<Genex6::Simulator> theSimulator (new Genex6::Simulator(m_theData.m_configurationFilePath, 
                                                                        simulation_type,
                                                                        m_theData.m_sourceRockType, 
                                                                        m_theData.m_HC,
                                                                        m_theData.m_SC,
                                                                        m_theData.m_Emean, 
                                                                        m_theData.m_VRE,
                                                                        m_theData.m_asphalteneDiffusionEnergy, 
                                                                        m_theData.m_resinDiffusionEnergy, 
                                                                        m_theData.m_C15AroDiffusionEnergy,
                                                                        m_theData.m_C15SatDiffusionEnergy,
                                                                        m_approximateFlag));
   //theSimulator->SetOpenConditions(m_dataCreationValues.m_openConditions);
   //cout << "Compute OTGC with " << (m_dataCreationValues.m_openConditions? "Open " : "Close ") << "Conditions." << endl;

   typedef std::vector<Genex6::Input*>::const_iterator historyConstIt;
   typedef std::vector<Genex6::Input*>::iterator historyIt;

   //inverse the time labels
   if(m_PTHistory.empty()) {
      cout << "No input history available. Aborting...." << endl;
      return false;
   }
   std::string benchmarkFullPathName;
#ifdef WIN32
   benchmarkFullPathName = m_theData.m_TestResultsFullPath + "\\" + m_theData.m_TestResultsName;
#else
   benchmarkFullPathName = m_theData.m_TestResultsFullPath + "/" + m_theData.m_TestResultsName;
#endif
   
   ofstream outputTestingSetFile(benchmarkFullPathName.c_str());
   if(!outputTestingSetFile.is_open()) {
      cerr << "Can not open " << benchmarkFullPathName << ". Aborting...." << endl;
      return false;
   } else {
      cout << "Benchmark results are saved in file " << benchmarkFullPathName << endl;
   }
   clock_t timeStart = clock();
   
   std::vector<TestingOutputRecord *> TestingResults;

   std::map<std::string, double> concentrationsByName = m_initialSpeciesConcentrations;

   TestingOutputRecord * firstTestingOuputRecord = new TestingOutputRecord;

   historyIt itFirst = m_PTHistory.begin();
   firstTestingOuputRecord->m_time = (*itFirst)->GetTime();
   firstTestingOuputRecord->m_temperature = (*itFirst)->GetTemperatureCelsius();
   firstTestingOuputRecord->m_pressure = (*itFirst)->GetPressure();
   firstTestingOuputRecord->m_concentrationsByName = concentrationsByName;
   TestingResults.push_back(firstTestingOuputRecord);

   OTGC6::SimulatorState theState(firstTestingOuputRecord->m_time, theSimulator->getSpeciesInChemicalModel(), concentrationsByName);
   //and go...

   for(historyConstIt it = m_PTHistory.begin(), itPlusOne = it + 1, itEnd = m_PTHistory.end(); itPlusOne != itEnd; ++itPlusOne) {
      
      Genex6::Input *inputStart = (*it);
      Genex6::Input *inputEnd = *(itPlusOne);
      
      theSimulator->computeInterval(theState,
                                    inputStart->GetTemperatureCelsius(), inputEnd->GetTemperatureCelsius(),
                                    inputStart->GetPressure(), inputEnd->GetPressure(), 
                                    inputStart->GetTime(), inputEnd->GetTime());
      
      TestingOutputRecord * theTestingOuputRecord = new TestingOutputRecord;
      
      theTestingOuputRecord->m_time = inputEnd->GetTime();
      theTestingOuputRecord->m_temperature = inputEnd->GetTemperatureCelsius();
      theTestingOuputRecord->m_pressure = inputEnd->GetPressure();
      
      theState.GetSpeciesStateConcentrations(&theSimulator->getChemicalModel(), theTestingOuputRecord->m_concentrationsByName);
      
      //       const std::string SpeciesOutputOrder1[] = {"kerogen","preasphalt","asphaltenes","resins",
      //                                                 "precoke","coke1","C15+Aro","C15+Sat","C6-14Sat","C6-14Aro",
      //                                                 "Hetero1","C5","C4","C3","C2","C1",
      //                                                 "H2O", "COx","N2", "NH4",
      //                                                 "coke2", "orgacid","H2",""};
      //       printf("%g %g %g\n", theTestingOuputRecord->m_time, theTestingOuputRecord->m_temperature,
      //              theTestingOuputRecord->m_pressure);
      
      
      TestingResults.push_back(theTestingOuputRecord);
      // 	   for(int i = 0; SpeciesOutputOrder1[i] != ""; ++i)
      //    	{
      //         double conc = theTestingOuputRecord->m_concentrationsByName[SpeciesOutputOrder1[i]];
      //         printf("%g\n", conc);
      //    	}		
      
      it = itPlusOne;
   }
   
   cout << "Executing 3:" << endl;
   cout << "Elapsed time: " << clock() - timeStart << " seconds" << endl;
   
   theSimulator->PrintBenchmarkOutput(outputTestingSetFile);
   
   typedef std::vector<TestingOutputRecord *>::iterator itRes;

   outputTestingSetFile << "[Table:ModelConcentrationC++]" << endl;
   outputTestingSetFile << "temp" << "," << "VRE" << "," << "Time" << ",";
   
   int i, numberOfSpecies = theSimulator->getChemicalModel().GetNumberOfSpecies();
   for( i = 1; i <= numberOfSpecies; ++i) {
      const std::string SpeciesName = theSimulator->getChemicalModel().GetSpeciesNameById(i);
      outputTestingSetFile << SpeciesName << ",";
   }
   
   outputTestingSetFile << endl;
   std::vector<TestingOutputRecord*>::iterator itBegin = TestingResults.begin();
   ++ itBegin; // skip 0 
   
   for(itRes it = itBegin, itEnd = TestingResults.end(); it != itEnd; ++ it) {
      TestingOutputRecord * theTestingOuputRecord = (*it);
      outputTestingSetFile << theTestingOuputRecord->m_temperature << ",0.0," << theTestingOuputRecord->m_time << ",";
      for(i = 1; i <= numberOfSpecies; ++i) {
         double conc = theTestingOuputRecord->m_concentrationsByName[ theSimulator->getChemicalModel().GetSpeciesNameById(i)];
         outputTestingSetFile << scientific << conc << ",";
      }		
      outputTestingSetFile << endl;				
   } 
   
   outputTestingSetFile << "Time" << "," << "Temperature" << "," << "Pressure" << endl;
   for(itRes it = TestingResults.begin() + 1, itEnd = TestingResults.end(); it != itEnd; ++it) {
      TestingOutputRecord * theTestingOuputRecord = (*it);
      outputTestingSetFile << theTestingOuputRecord->m_time << "," << theTestingOuputRecord->m_temperature << ","
                           << theTestingOuputRecord->m_pressure << endl;
   } 
   
   
   for(itRes it = TestingResults.begin(), itEnd = TestingResults.end(); it != itEnd; ++ it) {
      delete(*it);
   }
	return true;
}

bool TransformSch::execute()
{
   cout << "Executing file format transformation based on :" << m_testFileFullPathName << endl;
   const std::string InputFullPathSchFileName = m_theData.m_InputFullPath + "/" + m_theData.m_InputSchName;
   
   cout << "About to use file: " << InputFullPathSchFileName << endl;
   
   int simulation_type = (m_simulateGX5 ? Genex6::Constants::SIMOTGC | Genex6::Constants::SIMGENEX5 : Genex6::Constants::SIMOTGC);
   auto_ptr<Genex6::Simulator> theSimulator (new Genex6::Simulator(InputFullPathSchFileName,
                                                                   simulation_type, 
                                                                   m_theData.m_HC, m_theData.m_SC, m_theData.m_Emean, 
                                                                   m_theData.m_preprocessData,
                                                                   m_approximateFlag));
   
   const std::string newCfgFile = m_theData.m_OuputFullPath + "/" + m_theData.m_OuputCfgName;
   
   cout << "About to save in file: " << newCfgFile << endl;
   theSimulator->PrintConfigurationFile(newCfgFile, m_theData.m_preprocessData);
   
   return true;
}
void BenchmarkTest::setStateFromTestFile(const std::string &testFileFullPathName)
{
   cout << endl;
   cout << "-------------------- OTGC Library Benchmark";
   if(m_simulateGX5) { 
      cout << " (genex5 simulation) ------------------" << endl;
   } else {
      cout << " --------------------------------------" << endl;
   }
   cout << endl;
   cout << "Executing benchmark: " << m_testFileFullPathName << endl;
   std::ifstream testFile(testFileFullPathName.c_str());
   

   if(testFile == false) {
      cout << "Benchmark Test Input File :" << testFileFullPathName << " not found. Aborting..." << endl;
      throw FILE_NOT_FOUND;
   }
   
   while(!testFile.eof() && !testFile.fail()) {
      std::string line;
      getline (testFile, line, '\n');
      if("[Table:BenchmarkTestSimulatorData]" == line) {
         LoadBenchmarkTestSimulatorData(testFile);
      } 
      if("[Table:BenchmarkTestDataCreation]" == line) {
         LoadBenchmarkTestDataCreationProperties(testFile);
      } 
      if("[Table:BenchmarkPTHistory]" == line) {
         LoadBenchmarkPTHistory(testFile);
      } 
      if("[Table:BenchmarkInitialState]" == line) {
         LoadBenchmarkInitialSimulatorState(testFile);
      }        
   }
}
void TransformSch::setStateFromTestFile(const std::string &testFileFullPathName)
{
   
   std::ifstream testFile(testFileFullPathName.c_str());
   
   while(!testFile.eof() && !testFile.fail()) {
      std::string line;
      getline (testFile, line, '\n');
      if("[Table:TransformSch2CfgData]" == line) {
         LoadTransformScf2CfgData(testFile);
      }  
   }
}
void TransformSch::LoadTransformScf2CfgData(std::ifstream &testFile)
{
   std::string line;
   std::vector<std::string> theTokens;
   std::string delim = ",";
   int i = 0;
   for(;;) {
      std::getline (testFile, line, '\n');
      if(line == "[EndOfTable]" || line.size() == 0) {
         break;
      }
      
      Genex6::ParseLine(line, delim, theTokens);
      
      switch(i) {
      case TransformSch2CfgData::INPUTFULLPATH :
         m_theData.m_InputFullPath = theTokens[1];
         break;
      case TransformSch2CfgData::INPUTSCHNAME:
         m_theData.m_InputSchName = theTokens[1];
         break;
      case TransformSch2CfgData::OUPUTFULLPATH:
         m_theData.m_OuputFullPath = theTokens[1];
         break;
      case TransformSch2CfgData::OUPUTCFGNAME:
         m_theData.m_OuputCfgName = theTokens[1];
         break;
      case TransformSch2CfgData::HC:
         m_theData.m_HC = atof(theTokens[1].c_str());
         break;
      case TransformSch2CfgData::SC:
         m_theData.m_SC = atof(theTokens[1].c_str());
         break;
      case TransformSch2CfgData::SO4:
         m_theData.m_SO4 = atof(theTokens[1].c_str());
         break;
      case TransformSch2CfgData::EMEAN:
         m_theData.m_Emean = atof(theTokens[1].c_str());
         break;
      case TransformSch2CfgData::PREPROCESSDATA:
         m_theData.m_preprocessData = theTokens[1] == "True" ? true : false;
         break;
      default:
         break;
      }
      ++ i;
      theTokens.clear();
   } 
}
void BenchmarkTest::LoadBenchmarkInitialSimulatorState(std::ifstream &testFile)
{
   std::vector<std::string> theTokens;
   std::string delim = ",";

   std::string line;
   for(;;) {
      std::getline (testFile, line, '\n');
      
      if(line == "[EndOfTable]" || line.size() == 0) {
         break;
      }
      Genex6::ParseLine(line, delim, theTokens);

      /*wrong
        cout<<line<<endl;
        tokenizer tokens(line, sep);
        tokenizer::iterator it = tokens.begin();
        const std::string & name = *it;
        ++it;
        const std::string & value = *it;
        cout<<name<<","<<value<<endl;
        m_initialSpeciesConcentrations[name] = atof(value.c_str());
      */
      //correct
      //tokenizer tokens(line, sep);
      //tokenizer::iterator it = tokens.begin();
      //std::string name = *it;
      // ++ it;
      //std::string value = *it;
      //cout<<name<<","<<value<<endl;
      m_initialSpeciesConcentrations[theTokens[0]] = atof(theTokens[1].c_str());
      theTokens.clear();
   }
}
void BenchmarkTest::LoadBenchmarkPTHistory(std::ifstream &testFile)
{
   std::string line;
   std::vector<std::string> theTokens;
   std::string delim = ",";
    
   enum HistoryDataIndex{TIME, TEMPERATURE, PRESSURE, NUMBEROFDATA};
   double historyData[NUMBEROFDATA];
   
   for(;;) {
      std::getline (testFile, line, '\n');

      if(line == "[EndOfTable]" || line.size() == 0) {
         break;
      }
      Genex6::ParseLine(line, delim, theTokens);
 
      historyData[TIME] = atof(theTokens[0].c_str());
      historyData[TEMPERATURE] = atof(theTokens[1].c_str());
      historyData[PRESSURE] = atof(theTokens[2].c_str());
      
      m_PTHistory.push_back(new Genex6::Input(historyData[TIME],historyData[TEMPERATURE],historyData[PRESSURE]));
      theTokens.clear();
   }
}

BenchmarkTest::~BenchmarkTest()
{
   typedef std::vector<Genex6::Input*>::iterator historyIt;
   for(historyIt it = m_PTHistory.begin(), itEnd = m_PTHistory.end(); it != itEnd; ++ it) {
      delete (*it);
   }
}
void BenchmarkTest::LoadBenchmarkTestDataCreationProperties(std::ifstream &testFile)
{
   std::string line;
   std::vector<std::string> theTokens;
   std::string delim = ",";
   int i = 0;
   for(;;) {
      std::getline (testFile, line, '\n');
      if(line == "[EndOfTable]" || line.size() == 0) {
         break;
      }
      
      Genex6::ParseLine(line, delim, theTokens);
      
      switch(i) {
      case BenchmarkTestDataCreation::NUMBEROFTIMESTEPS :
         m_dataCreationValues.m_numberOfTimesteps = atoi(theTokens[1].c_str());
         break;
      case BenchmarkTestDataCreation::TEMPERATUREATSTART:
         m_dataCreationValues.m_TemperatureAtStart = atof(theTokens[1].c_str());
         break;
      case BenchmarkTestDataCreation::TEMPERATUREATEND:
         m_dataCreationValues.m_TemperatureAtEnd = atof(theTokens[1].c_str());
         break;
      case BenchmarkTestDataCreation::HEATINGRATE:
         m_dataCreationValues.m_heatingRate = atof(theTokens[1].c_str());
         break;
      case BenchmarkTestDataCreation::TEMPERATUREGRADIENT:
         m_dataCreationValues.m_temperatureGradient = atof(theTokens[1].c_str());
         break;
      case BenchmarkTestDataCreation::SURFACETEMPERATURE:
         m_dataCreationValues.m_surfaceTemperature = atof(theTokens[1].c_str());
         break;
      case BenchmarkTestDataCreation::OVERBURDERDENSITY:
         m_dataCreationValues.m_overBurderDensity = atof(theTokens[1].c_str());
         break;
      case BenchmarkTestDataCreation::MAXIMUMPEFF:
         m_dataCreationValues.m_maximumPeff = atof(theTokens[1].c_str());
         break;
      case BenchmarkTestDataCreation::PFRACT:
         m_dataCreationValues.m_Pfract = atof(theTokens[1].c_str());
         break;
      case BenchmarkTestDataCreation::PSPECGRAD:
         m_dataCreationValues.m_PSpecGrad = atof(theTokens[1].c_str());
         break;
      case BenchmarkTestDataCreation::OPENCONDITIONS:
         m_dataCreationValues.m_openConditions = theTokens[1] == "True" ? true : false;
         break;
      case BenchmarkTestDataCreation::THICKNESS:
         m_dataCreationValues.m_thickness = atof(theTokens[1].c_str());
         break;
      case BenchmarkTestDataCreation::TOCI:
         m_dataCreationValues.m_TOCi = atof(theTokens[1].c_str());
         break;
      case BenchmarkTestDataCreation::INORGANICDENSITY:
         m_dataCreationValues.m_InorganicDensity = atof(theTokens[1].c_str());
         break;
      default:
         break;
      }
      ++ i;
      theTokens.clear();
    } 
}
void BenchmarkTest::LoadBenchmarkTestSimulatorData(std::ifstream &testFile)
{
   std::string line;
   std::vector<std::string> theTokens;
   std::string delim = ",";
   int i = 0;
   for(;;) {
      std::getline (testFile, line, '\n');
      if(line == "[EndOfTable]" || line.size() == 0) {
         break;
      }
      Genex6::ParseLine(line, delim, theTokens);
      switch(i) {
      case BenchmarkTestSimulatorData::SOURCEROCKTYPE :
         m_theData.m_sourceRockType = theTokens[1];
         break;
      case BenchmarkTestSimulatorData::CONFIGURATIONFILEPATH:
         m_theData.m_configurationFilePath = theTokens[1];
         break;
      case BenchmarkTestSimulatorData::HC:
         m_theData.m_HC = atof(theTokens[1].c_str());
         break;
      case BenchmarkTestSimulatorData::SC:
         m_theData.m_SC = atof(theTokens[1].c_str());
         break;
      case BenchmarkTestSimulatorData::SO4:
         m_theData.m_SO4 = atof(theTokens[1].c_str());
         break;
      case BenchmarkTestSimulatorData::EMEAN:
         m_theData.m_Emean = atof(theTokens[1].c_str());
         break;
      case BenchmarkTestSimulatorData::VRE:
         m_theData.m_VRE = atof(theTokens[1].c_str());
         break;
      case BenchmarkTestSimulatorData::ASPHALTENEDIFFUSIONENERGY:
         m_theData.m_asphalteneDiffusionEnergy = atof(theTokens[1].c_str());
         break;
      case BenchmarkTestSimulatorData::RESINDIFFUSIONENERGY:
         m_theData.m_resinDiffusionEnergy = atof(theTokens[1].c_str());
         break;
      case BenchmarkTestSimulatorData::C15ARODIFFUSIONENERGY:
         m_theData.m_C15AroDiffusionEnergy = atof(theTokens[1].c_str());
         break;
      case BenchmarkTestSimulatorData::C15SATDIFFUSIONENERGY:
         m_theData.m_C15SatDiffusionEnergy = atof(theTokens[1].c_str());
         break;
      case BenchmarkTestSimulatorData::TESTRESULTSNAME:
         m_theData.m_TestResultsName = theTokens[1];
         break;
      case BenchmarkTestSimulatorData::TESTRESULTSFULLPATH:
         m_theData.m_TestResultsFullPath = theTokens[1];
         break;
      default:
         break;
      }
      ++ i;
      
      theTokens.clear();
   }
}
UnitTest *createTransformSchTest(const std::string &testFileFullPathName)
{
   return new TransformSch(testFileFullPathName);
}
UnitTest *createBenchmarkTest(const std::string &testFileFullPathName)
{
   return new BenchmarkTest(testFileFullPathName);
} 
UnitTestFactory::UnitTestFactory()
{
   registerFactoryMethod(BENCHMARK, createBenchmarkTest);
   registerFactoryMethod(TRANSFORMSCH2CFG, createTransformSchTest);
}
void UnitTestFactory::registerFactoryMethod(int factoryMethodKey, factoryMethod theMethod)
{
   m_registeredFactoryMethods.insert(make_pair(factoryMethodKey,theMethod));
}
UnitTest *UnitTestFactory::createTest (int factoryMethodKey, const std::string &testFileFullPathName, 
                                       bool flagGX5, bool flagApprox)
{
   std::map<int,factoryMethod>::iterator it = m_registeredFactoryMethods.find(factoryMethodKey);
   if(it != m_registeredFactoryMethods.end())  {
      UnitTest *ret = (*(it->second))(testFileFullPathName);
      ret->setGX5(flagGX5);
      ret->setApprox(flagApprox);
      try {
         ret->setStateFromTestFile(testFileFullPathName);
      } catch(int &ex) { 
         throw;
      }
      return ret;
   } else {
      return 0;
   }
}
UnitTestType UnitTestFactory::determineTestType(const std::string &testFullPathName)
{
   UnitTestType ret = BENCHMARK;
   std::ifstream testFile(testFullPathName.c_str());
   
   if(testFile.fail()) {
      cout << "File " << testFullPathName << " not found" << endl;
   }
   std::string delim = ",";
   
   while(!testFile.eof() && !testFile.fail()) {
      std::string line;
      getline (testFile, line, '\n');
      std::vector<std::string> theTokens;
      
      if("[Table:OTGCTest]" == line) {
         for(getline (testFile, line, '\n'); line!="[EndOfTable]"; getline (testFile, line, '\n')) {
            
            Genex6::ParseLine(line, delim, theTokens);
            
            if("TestType" == theTokens[0]) {
               if("BENCHMARK" == theTokens[1]) {
                  ret = BENCHMARK;
                  break;
               }
               if("TRANSFORMSCH2CFG" == theTokens[1]) {
                  ret = TRANSFORMSCH2CFG;
                  break;
               }
               if("SENSITIVITY" == theTokens[1]) {
                  ret = SENSITIVITY;
                  break;
               } else {
                  std::cout << "Invalid Test type option: " << theTokens[1] << ", setting it to BENCHMARK" << endl;
                  ret = BENCHMARK;
                  break;
               }
            }
         }
         break;
      }  
   }
   return ret;
}

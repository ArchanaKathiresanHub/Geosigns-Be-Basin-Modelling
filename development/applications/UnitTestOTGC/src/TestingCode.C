#include "TestingCode.h"
//Annette
#ifdef WIN32
#include <boost\tokenizer.hpp>
#include <boost\timer.hpp>
#include<boost\shared_ptr.hpp>
#else
#include <boost/tokenizer.hpp>
#include <boost/timer.hpp>
#include<boost/shared_ptr.hpp>
#endif
#include <memory>

#include "Simulator.h"
#include "Input.h"

#include "SpeciesState.h"
#include "SimulatorState.h"
#include "Input.h"

typedef boost::shared_ptr<TestingOutputRecord> OutputRecordPtr;

bool BenchmarkTest::execute()
{


   std::auto_ptr<OTGC::Simulator> theSimulator (new OTGC::Simulator(m_theData.m_configurationFilePath, 
                                                                    m_theData.m_sourceRockType, 
                                                                    m_theData.m_HC, 
                                                                    m_theData.m_Emean, 
                                                                    m_theData.m_VRE,
                                                                    m_theData.m_asphalteneDiffusionEnergy, 
                                                                    m_theData.m_resinDiffusionEnergy, 
                                                                    m_theData.m_C15AroDiffusionEnergy,
                                                                    m_theData.m_C15SatDiffusionEnergy));
   



   typedef std::vector<OTGC::Input*>::const_iterator historyConstIt;
   typedef std::vector<OTGC::Input*>::iterator historyIt;

   //inverse the time labels
   if(m_PTHistory.empty())
   {
      cout<<"No input history available. Aborting...."<<endl;
      return false;
   }

   boost::timer theTimer;

   std::vector<OutputRecordPtr> TestingResults;

   std::map<std::string, double> concentrationsByName = m_initialSpeciesConcentrations;

   OutputRecordPtr firstTestingOuputRecord (new TestingOutputRecord);
   historyIt itFirst = m_PTHistory.begin();
   firstTestingOuputRecord->m_time = (*itFirst)->GetCurrentTime();
   firstTestingOuputRecord->m_temperature = (*itFirst)->GetTemperatureCelsius();
   firstTestingOuputRecord->m_pressure = (*itFirst)->GetPressure();
   firstTestingOuputRecord->m_concentrationsByName = concentrationsByName;
   TestingResults.push_back(firstTestingOuputRecord);

   OTGC::SimulatorState theState(firstTestingOuputRecord->m_time, theSimulator->getSpeciesInChemicalModel(), concentrationsByName);
   //and go...
   
   
   for(historyConstIt it = m_PTHistory.begin(), itPlusOne = it + 1, itEnd = m_PTHistory.end(); itPlusOne != itEnd; ++itPlusOne)
   {
      OTGC::Input *inputStart = (*it);
      OTGC::Input *inputEnd = *(itPlusOne);
      
      theSimulator->computeInterval( theState,
                                     inputStart->GetTemperatureCelsius(), inputEnd->GetTemperatureCelsius(),
                                     inputStart->GetPressure(), inputEnd->GetPressure(), 
                                     inputStart->GetCurrentTime(), inputEnd->GetCurrentTime()
                                     );
      
      OutputRecordPtr theTestingOuputRecord (new TestingOutputRecord);
      
      theTestingOuputRecord->m_time = inputEnd->GetCurrentTime();
      theTestingOuputRecord->m_temperature = inputEnd->GetTemperatureCelsius();
      theTestingOuputRecord->m_pressure = inputEnd->GetPressure();
      theState.GetSpeciesStateConcentrations(theTestingOuputRecord->m_concentrationsByName);
      
      TestingResults.push_back(theTestingOuputRecord);
      
      it = itPlusOne;
   }
   
   cout<<"Executing 3:"<<endl;
   cout<<"Elapsed time: "<<theTimer.elapsed()<<" seconds"<<endl;
   
   std::string benchmarkFullPathName = m_theData.m_TestResultsFullPath + "/" + m_theData.m_TestResultsName;
   
   cout<<"Benchmark results are saved in file "<<benchmarkFullPathName<<endl;
   
   ofstream outputTestingSetFile(benchmarkFullPathName.c_str());
   
   theSimulator->PrintBenchmarkOutput(outputTestingSetFile);
   
   typedef std::vector<OutputRecordPtr>::iterator itRes;
   
   outputTestingSetFile<<"Time"<<","<<"Temperature"<<","<<"Pressure"<<endl;
   for(itRes it = TestingResults.begin(),  itEnd = TestingResults.end(); it != itEnd; ++it)
   {
      OutputRecordPtr theTestingOuputRecord = (*it);
      outputTestingSetFile << theTestingOuputRecord->m_time << "," <<theTestingOuputRecord->m_temperature << "," << theTestingOuputRecord->m_pressure << endl;
   } 
   
   const std::string SpeciesOutputOrder[] = {"kerogen","preasphalt","asphaltenes","resins",
                                             "precoke","coke1","C15+Aro","C15+Sat","C6-14Sat","C6-14Aro",
                                             "Hetero1","C5","C4","C3","C2","C1",
                                             "H2O", "COx","N2", "NH4",
                                             "coke2", "orgacid","H2",""};
   
   outputTestingSetFile<<"[Table:ModelConcentrationC++]"<<endl;
   outputTestingSetFile<<"temp"<<","<<"VRE"<<","<<"Time"<<",";
   
   for(int i = 0; SpeciesOutputOrder[i]!=""; ++i)
   {
      outputTestingSetFile<<SpeciesOutputOrder[i]<<",";
   }
   outputTestingSetFile<<endl;
   for(itRes it = TestingResults.begin(),  itEnd = TestingResults.end(); it != itEnd; ++it)
   {
      OutputRecordPtr theTestingOuputRecord = (*it);
      outputTestingSetFile << theTestingOuputRecord->m_temperature << ",0.0," << theTestingOuputRecord->m_time << ",";
      for(int i = 0; SpeciesOutputOrder[i]!=""; ++i)
      {
         double conc = theTestingOuputRecord->m_concentrationsByName[SpeciesOutputOrder[i]];
         outputTestingSetFile << conc << ",";
      }		
      outputTestingSetFile << endl;				
   } 
   
   return true;
}

bool TransformSch::execute()
{
   cout<<"Executing file format transformation based on :"<<m_testFileFullPathName<<endl;
   const std::string InputFullPathSchFileName = m_theData.m_InputFullPath + "/" + m_theData.m_InputSchName;

   cout<<"About to use file: "<<InputFullPathSchFileName<<endl;

   auto_ptr<OTGC::Simulator> theSimulator (new OTGC::Simulator(InputFullPathSchFileName, m_theData.m_HC, m_theData.m_Emean, m_theData.m_preprocessData));

   const std::string newCfgFile = m_theData.m_OuputFullPath + "/" + m_theData.m_OuputCfgName;

   cout<<"About to save in file: "<<newCfgFile<<endl;
   theSimulator->PrintConfigurationFile(newCfgFile);

   return true;
}
void BenchmarkTest::setStateFromTestFile(const std::string &testFileFullPathName)
{
   cout<<"-----------------------------------------------------------------------------------------------------"<<endl;
   cout<<"----------------------------------OTGC Library Benchamrk---------------------------------------------"<<endl;
   cout<<"----------------------------------------------------------------------------------------------------"<<endl;
   cout<<endl;
   cout<<"Executing benchmark :"<<m_testFileFullPathName<<endl;
   std::ifstream testFile(testFileFullPathName.c_str());
   if(testFile == false)
   {
      cout<<"Benchmark Test Input File :"<<testFileFullPathName<<" not found. Aborting..."<<endl;
   }

   while(!testFile.eof() && !testFile.fail())
   {
      std::string line;
      getline (testFile, line, '\n');
      if("[Table:BenchmarkTestSimulatorData]" == line)
      {
         LoadBenchmarkTestSimulatorData(testFile);
      } 
      if("[Table:BenchmarkTestDataCreation]" == line)
      {
         LoadBenchmarkTestDataCreationProperties(testFile);
      } 
      if("[Table:BenchmarkPTHistory]" == line)
      {
         LoadBenchmarkPTHistory(testFile);
      } 
      if("[Table:BenchmarkInitialState]" == line)
      {
         LoadBenchmarkInitialSimulatorState(testFile);
      }        
   }
}
void TransformSch::setStateFromTestFile(const std::string &testFileFullPathName)
{
   std::ifstream testFile(testFileFullPathName.c_str());
   
   while(!testFile.eof() && !testFile.fail())
   {
      std::string line;
      getline (testFile, line, '\n');
      if("[Table:TransformSch2CfgData]" == line)
      {
         LoadTransformScf2CfgData(testFile);
      }  
   }
}
void TransformSch::LoadTransformScf2CfgData(std::ifstream &testFile)
{
   typedef boost::tokenizer<boost::char_separator<char> >  tokenizer;
   boost::char_separator<char> sep(",");

   int i = 0;
   std::string line;
   for( getline (testFile, line, '\n'); line!="[EndOfTable]"; getline (testFile, line, '\n') )
   {
      tokenizer tokens(line, sep);
      tokenizer::iterator it = tokens.begin();
      ++it;
      switch(i)
      {
      case TransformSch2CfgData::INPUTFULLPATH :
         m_theData.m_InputFullPath = (*(it));
         break;
      case TransformSch2CfgData::INPUTSCHNAME:
         m_theData.m_InputSchName = (*(it ));
         break;
      case TransformSch2CfgData::OUPUTFULLPATH:
         m_theData.m_OuputFullPath = (*(it ));
         break;
      case TransformSch2CfgData::OUPUTCFGNAME:
         m_theData.m_OuputCfgName = (*(it ));
         break;
      case TransformSch2CfgData::HC:
         m_theData.m_HC = atof((*(it)).c_str());
         break;
      case TransformSch2CfgData::EMEAN:
         m_theData.m_Emean = atof((*(it)).c_str());
         break;
      case TransformSch2CfgData::PREPROCESSDATA:
         m_theData.m_preprocessData = (*(it)) == "True" ? true : false;
         break;
      default:
         break;
      }
      ++i;
   } 
}
void BenchmarkTest::LoadBenchmarkInitialSimulatorState(std::ifstream &testFile)
{
   typedef boost::tokenizer<boost::char_separator<char> >  tokenizer;
   boost::char_separator<char> sep(",");

   std::string line;
   for( getline (testFile, line, '\n'); line!="[EndOfTable]"; getline (testFile, line, '\n') )
   {
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
      tokenizer tokens(line, sep);
      tokenizer::iterator it = tokens.begin();
      std::string name = *it;
      ++it;
      std::string value = *it;
      //cout<<name<<","<<value<<endl;
      m_initialSpeciesConcentrations[name] = atof(value.c_str());
   }
}
void BenchmarkTest::LoadBenchmarkPTHistory(std::ifstream &testFile)
{
   typedef boost::tokenizer<boost::char_separator<char> >  tokenizer;
   boost::char_separator<char> sep(",");
   
   enum HistoryDataIndex{TIME, TEMPERATURE, PRESSURE, NUMBEROFDATA};
   double historyData[NUMBEROFDATA];
   
   std::string line;
   for( getline (testFile, line, '\n'); line!="[EndOfTable]"; getline (testFile, line, '\n') )
   {
      tokenizer tokens(line, sep);
      tokenizer::iterator it = tokens.begin();
      
      historyData[TIME] = atof((*it).c_str());
      historyData[TEMPERATURE] = atof((*(++it)).c_str());
      historyData[PRESSURE] = atof((*(++it)).c_str());
      
      m_PTHistory.push_back(new OTGC::Input(historyData[TIME],historyData[TEMPERATURE],historyData[PRESSURE]));
   }
}

BenchmarkTest::~BenchmarkTest()
{
   typedef std::vector<OTGC::Input*>::iterator historyIt;
   for(historyIt it = m_PTHistory.begin(), itEnd = m_PTHistory.end(); it != itEnd; ++it)
   {
      delete (*it);
   }
}
void BenchmarkTest::LoadBenchmarkTestDataCreationProperties(std::ifstream &testFile)
{
   typedef boost::tokenizer<boost::char_separator<char> >  tokenizer;
   boost::char_separator<char> sep(",");
   
   int i = 0;
   std::string line;
   for( getline (testFile, line, '\n'); line!="[EndOfTable]"; getline (testFile, line, '\n') )
   {
      tokenizer tokens(line, sep);
      tokenizer::iterator it = tokens.begin();
      ++it;
      switch(i)
      {
      case BenchmarkTestDataCreation::NUMBEROFTIMESTEPS :
         m_dataCreationValues.m_numberOfTimesteps = atoi((*(it)).c_str());
         break;
      case BenchmarkTestDataCreation::TEMPERATUREATSTART:
         m_dataCreationValues.m_TemperatureAtStart = atof((*(it)).c_str());
         break;
      case BenchmarkTestDataCreation::TEMPERATUREATEND:
         m_dataCreationValues.m_TemperatureAtEnd = atof((*(it)).c_str());
         break;
      case BenchmarkTestDataCreation::HEATINGRATE:
         m_dataCreationValues.m_heatingRate = atof((*(it)).c_str());
         break;
      case BenchmarkTestDataCreation::TEMPERATUREGRADIENT:
         m_dataCreationValues.m_temperatureGradient = atof((*(it)).c_str());
         break;
      case BenchmarkTestDataCreation::SURFACETEMPERATURE:
         m_dataCreationValues.m_surfaceTemperature = atof((*(it)).c_str());
         break;
      case BenchmarkTestDataCreation::OVERBURDERDENSITY:
         m_dataCreationValues.m_overBurderDensity = atof((*(it)).c_str());
         break;
      case BenchmarkTestDataCreation::MAXIMUMPEFF:
         m_dataCreationValues.m_maximumPeff = atof((*(it)).c_str());
         break;
      case BenchmarkTestDataCreation::PFRACT:
         m_dataCreationValues.m_Pfract = atof((*(it)).c_str());
         break;
      case BenchmarkTestDataCreation::PSPECGRAD:
         m_dataCreationValues.m_PSpecGrad = atof((*(it)).c_str());
         break;
      case BenchmarkTestDataCreation::OPENCONDITIONS:
         m_dataCreationValues.m_openConditions = (*(it)) == "True" ? true : false;
         break;
      case BenchmarkTestDataCreation::THICKNESS:
         m_dataCreationValues.m_thickness = atof((*(it)).c_str());
         break;
      case BenchmarkTestDataCreation::TOCI:
         m_dataCreationValues.m_TOCi = atof((*(it)).c_str());
         break;
      case BenchmarkTestDataCreation::INORGANICDENSITY:
         m_dataCreationValues.m_InorganicDensity = atof((*(it)).c_str());
         break;
      default:
         break;
      }
      ++i;
   } 
}
void BenchmarkTest::LoadBenchmarkTestSimulatorData(std::ifstream &testFile)
{
   typedef boost::tokenizer<boost::char_separator<char> >  tokenizer;
   boost::char_separator<char> sep(",");
   int i = 0;
   std::string line;
   for( getline (testFile, line, '\n'); line!="[EndOfTable]"; getline (testFile, line, '\n') )
   {
      tokenizer tokens(line, sep);
      tokenizer::iterator it = tokens.begin();
      ++it;
      switch(i)
      {
      case BenchmarkTestSimulatorData::SOURCEROCKTYPE :
         m_theData.m_sourceRockType = (*(it));
         break;
      case BenchmarkTestSimulatorData::CONFIGURATIONFILEPATH:
         m_theData.m_configurationFilePath = (*(it ));
         break;
      case BenchmarkTestSimulatorData::HC:
         m_theData.m_HC = atof((*(it )).c_str());
         break;
      case BenchmarkTestSimulatorData::EMEAN:
         m_theData.m_Emean = atof((*(it )).c_str());
         break;
      case BenchmarkTestSimulatorData::VRE:
         m_theData.m_VRE = atof((*(it)).c_str());
         break;
      case BenchmarkTestSimulatorData::ASPHALTENEDIFFUSIONENERGY:
         m_theData.m_asphalteneDiffusionEnergy = atof((*(it)).c_str());
         break;
      case BenchmarkTestSimulatorData::RESINDIFFUSIONENERGY:
         m_theData.m_resinDiffusionEnergy = atof((*(it)).c_str());
         break;
      case BenchmarkTestSimulatorData::C15ARODIFFUSIONENERGY:
         m_theData.m_C15AroDiffusionEnergy = atof((*(it)).c_str());
         break;
      case BenchmarkTestSimulatorData::C15SATDIFFUSIONENERGY:
         m_theData.m_C15SatDiffusionEnergy = atof((*(it)).c_str());
         break;
      case BenchmarkTestSimulatorData::TESTRESULTSNAME:
         m_theData.m_TestResultsName = (*(it ));
         break;
      case BenchmarkTestSimulatorData::TESTRESULTSFULLPATH:
         m_theData.m_TestResultsFullPath = (*(it ));
         break;
      default:
         break;
      }
      ++i;
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
UnitTest *UnitTestFactory::createTest (int factoryMethodKey, const std::string &testFileFullPathName)
{
   std::map<int,factoryMethod>::iterator it = m_registeredFactoryMethods.find(factoryMethodKey);
   if(it != m_registeredFactoryMethods.end())
   {
      UnitTest *ret =  (*(it->second))(testFileFullPathName);
      ret->setStateFromTestFile(testFileFullPathName);
      return ret;
   }
   else
   {
      return 0;
   }
}
UnitTestType UnitTestFactory::determineTestType(const std::string &testFullPathName)
{
   UnitTestType ret = BENCHMARK;
   std::ifstream testFile(testFullPathName.c_str());

   if(testFile.fail())
   {
      cout<<"File "<<testFullPathName<<" not found"<<endl;
   }
   typedef boost::tokenizer<boost::char_separator<char> >  tokenizer;
   boost::char_separator<char> sep(",");
   
   while(!testFile.eof() && !testFile.fail())
   {
      std::string line;
      getline (testFile, line, '\n');
      
      if("[Table:OTGCTest]" == line)
      {
         for( getline (testFile, line, '\n'); line!="[EndOfTable]"; getline (testFile, line, '\n') )
         {
            tokenizer tokens(line, sep);
            tokenizer::iterator it = tokens.begin();
            if("TestType" == (*it))
            {
               ++it;
               if("BENCHMARK" == (*it))
               {
                  ret = BENCHMARK;
                  break;
               }
               if("TRANSFORMSCH2CFG" == (*it))
               {
                  ret = TRANSFORMSCH2CFG;
                  break;
               }
               if("SENSITIVITY" == (*it))
               {
                  ret = SENSITIVITY;
                  break;
               }
               else
               {
                  std::cout<<"Invalid Test type option: "<<(*it)<<", setting it to BENCHMARK"<<endl;
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

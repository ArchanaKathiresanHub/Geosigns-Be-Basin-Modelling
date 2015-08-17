#ifndef TESTING_CODE_H
#define TESTING_CODE_H

#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <memory>
#include <map>
#include<vector>

#define FILE_NOT_FOUND 1

struct TestingOutputRecord
{
   double m_time;
   double m_temperature;
   double m_pressure ;
   std::map<std::string, double> m_concentrationsByName;
};
namespace Genex6
{
   class Input;
}

class UnitTest;
UnitTest *createTransformSchTest(const std::string &testFileFullPathName);
UnitTest *createBenchmarkTest(const std::string &testFileFullPathName);

enum UnitTestType{BENCHMARK, TSRBENCHMARK, TRANSFORMSCH2CFG, SENSITIVITY, numOfTestTypes};

class UnitTest
{
public:
   virtual ~UnitTest(){ m_simulateGX5 = false;  m_approximateFlag = true; }
   virtual bool execute() = 0;
   virtual void setStateFromTestFile(const std::string &testFileFullPathName) = 0;
   
   void setGX5(bool in_flag){ m_simulateGX5 = in_flag; }
   void setApprox(bool in_flag){ m_approximateFlag = in_flag; }
   void setSimulationType( UnitTestType in_simType ) { m_simType = in_simType; }
protected:
   bool m_simulateGX5;
   bool m_approximateFlag;
   UnitTestType m_simType;
};

struct BenchmarkTestSimulatorData
{
   enum SimDataIndex{ SOURCEROCKTYPE,
                      CONFIGURATIONFILEPATH,
                      HC,					  
                      SC,	
                      SO4,
                      EMEAN, 
                      VRE,
                      ASPHALTENEDIFFUSIONENERGY,
                      RESINDIFFUSIONENERGY,
                      C15ARODIFFUSIONENERGY,
                      C15SATDIFFUSIONENERGY, TESTRESULTSNAME,					  
                      TESTRESULTSFULLPATH };
   std::string m_sourceRockType;
   std::string m_configurationFilePath;
   double m_HC;				  
   double m_SC;			
   double m_SO4;
   double m_Emean; 
   double m_VRE;
   double m_asphalteneDiffusionEnergy;
   double m_resinDiffusionEnergy;
   double m_C15AroDiffusionEnergy;
   double m_C15SatDiffusionEnergy;  
   std::string m_TestResultsName;
   std::string m_TestResultsFullPath; 
};
struct OTGCTestData
{
   enum SimDataIndex{ TESTNAME, TESTTYPE };
   std::string m_TestName;
   std::string m_TestType;
};
struct BenchmarkTestDataCreation
{
   enum SimDataIndex{ NUMBEROFTIMESTEPS,
                      TEMPERATUREATSTART,
                      TEMPERATUREATEND,
                      HEATINGRATE,
                      TEMPERATUREGRADIENT,
                      SURFACETEMPERATURE,
                      OVERBURDERDENSITY,
                      MAXIMUMPEFF,
                      PFRACT,
                      PSPECGRAD, 
                      OPENCONDITIONS,
                      THICKNESS,
                      TOCI,
                      INORGANICDENSITY };
   int m_numberOfTimesteps;
   double m_TemperatureAtStart;
   double m_TemperatureAtEnd;
   double m_heatingRate;
   double m_temperatureGradient;
   double m_surfaceTemperature;
   double m_overBurderDensity;
   double m_maximumPeff;
   double m_Pfract; 
   double m_PSpecGrad;
   bool m_openConditions;
   double m_thickness;
   double m_TOCi;
   double m_InorganicDensity;   
};
class BenchmarkTest: public UnitTest
{
public:  
   BenchmarkTest(const std::string &testFileFullPathName):
      UnitTest(),
      m_testFileFullPathName(testFileFullPathName){}
   
   virtual ~BenchmarkTest();
   virtual bool execute();
   virtual void setStateFromTestFile(const std::string &testFileFullPathName);
   
private:
   std::string m_testFileFullPathName;
   BenchmarkTestSimulatorData m_theData;
   BenchmarkTestDataCreation m_dataCreationValues;
   std::vector<Genex6::Input*> m_PTHistory;
   std::map<std::string, double> m_initialSpeciesConcentrations;

   void LoadBenchmarkTestSimulatorData(std::ifstream &testFile); 
   void LoadBenchmarkTestDataCreationProperties(std::ifstream &testFile); 
   void LoadBenchmarkPTHistory(std::ifstream &testFile); 
   void LoadBenchmarkInitialSimulatorState(std::ifstream &testFile); 
};

struct TransformSch2CfgData
{
   enum SimDataIndex{ INPUTFULLPATH,
                      INPUTSCHNAME,
                      OUPUTFULLPATH,
                      OUPUTCFGNAME,
                      HC,  				 
                      SC,  
                      SO4,
                      EMEAN, 
                      PREPROCESSDATA };
   std::string m_InputFullPath;
   std::string m_InputSchName;
   std::string m_OuputFullPath;
   std::string m_OuputCfgName;
   double m_HC;				  
   double m_SC;		
   double m_SO4;
   double m_Emean; 
   bool m_preprocessData;
};

class TransformSch: public UnitTest
{
public:
   TransformSch(const std::string &testFileFullPathName):
      UnitTest(),
      m_testFileFullPathName(testFileFullPathName) { }
   virtual ~TransformSch(){}
   virtual bool execute();
   virtual void setStateFromTestFile(const std::string &testFileFullPathName);
private:
   std::string m_testFileFullPathName;
   void LoadTransformScf2CfgData(std::ifstream &testFile);
   TransformSch2CfgData m_theData;
};

class UnitTestFactory
{
public:
   typedef UnitTest*(*factoryMethod)(const std::string &);
   //use the joshi technique instead
   UnitTestFactory();
   static UnitTestType determineTestType(const std::string &testFullPathName);
   void registerFactoryMethod(int factoryMethodKey, factoryMethod theMethod);
   UnitTest *createTest (int factoryMethodKey, const std::string &testFileFullPathName, bool &flagGX5, bool &flagApprox);
private:
   std::map<int,factoryMethod> m_registeredFactoryMethods;
};

template<typename T>
class Singleton
{
public:
   static T & instance()
   {
      static T ret;
      return ret;
   }
private:
   Singleton(const Singleton<T> &inSingleton);
   Singleton<T> & operator =(const Singleton<T> &inSingleton);
   
};
/*
  template<typename Calculator>
  class DataRep
  {
  public:
  typedef Calculator theCalculator;
  friend class theCalculator;
  };
*/
#endif




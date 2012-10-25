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

struct TestingOutputRecord
{
double m_time ;
double m_temperature;
double m_pressure ;
std::map<std::string, double> m_concentrationsByName;
};
namespace OTGC
{
   class Input;
}

class UnitTest;
UnitTest *createTransformSchTest(const std::string &testFileFullPathName);
UnitTest *createBenchmarkTest(const std::string &testFileFullPathName);

enum UnitTestType{BENCHMARK, TRANSFORMSCH2CFG, SENSITIVITY, numOfTestTypes};

class UnitTest
{
public:
   virtual ~UnitTest(){}
	virtual bool execute() = 0;
   virtual void setStateFromTestFile(const std::string &testFileFullPathName) = 0;
};

struct BenchmarkTestSimulatorData
{
   enum SimDataIndex{ SOURCEROCKTYPE,
							 CONFIGURATIONFILEPATH,
							 HC,					  
							 EMEAN, 
							 VRE,
							 ASPHALTENEDIFFUSIONENERGY,
							 RESINDIFFUSIONENERGY,
							 C15ARODIFFUSIONENERGY,
							 C15SATDIFFUSIONENERGY, TESTRESULTSNAME,					  
							 TESTRESULTSFULLPATH    };
	std::string m_sourceRockType;
	std::string m_configurationFilePath;
	double m_HC;				  
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
   enum SimDataIndex{ TESTNAME,
							 TESTTYPE
							};
	std::string m_TestName;
	std::string m_TestType;
};
struct BenchmarkTestDataCreation
{
   enum SimDataIndex{
								NUMBEROFTIMESTEPS,
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
								INORGANICDENSITY								   
							};
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
class BenchmarkTest:public UnitTest
{
public:  
   BenchmarkTest(const std::string &testFileFullPathName):
   UnitTest(),
   m_testFileFullPathName(testFileFullPathName)
   {
     
   }
   virtual ~BenchmarkTest();
	virtual bool execute();
   virtual void setStateFromTestFile(const std::string &testFileFullPathName);

private:
std::string m_testFileFullPathName;
BenchmarkTestSimulatorData m_theData;
BenchmarkTestDataCreation m_dataCreationValues;
std::vector<OTGC::Input*> m_PTHistory;
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
							 EMEAN, 
							 PREPROCESSDATA
							 };
	std::string m_InputFullPath;
	std::string m_InputSchName;
   std::string m_OuputFullPath;
	std::string m_OuputCfgName;
	double m_HC;				  
	double m_Emean; 
   bool m_preprocessData;
};
class TransformSch:public UnitTest
{
public:
   TransformSch(const std::string &testFileFullPathName):
   UnitTest(),
   m_testFileFullPathName(testFileFullPathName)
   {

   }
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
   UnitTest *createTest (int factoryMethodKey, const std::string &testFileFullPathName);
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




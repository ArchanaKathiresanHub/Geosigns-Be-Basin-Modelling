#include "TestingCode.h"

//Calibration engine testing procedure. Steps I to IV below should be performed in order to run a test

//I)

   //Run Cauldron 1d with SmectiteIllite and Biomarkers computation.

   //Remark: 
   //Choose a fine snapshot time resolution so that the computed intervals within cauldron1d coincide with
   //the snapshot intervals. To create a fine snapshot time resolution in cauldron -v2007.09 adjust the snapshotInterval through 
   //Edit Run Parameters ->Snapshots


   //***********WARNING********//

   //The testing application assumes that there are no spaces in the formation name. 
   //For example for a formation with a name "Base Namurian",
   //the name in the temperature history should be replaced with "BaseNamurian" before running the test

   //**************************//

//II)

   //For a particular formation save the following properties: 
   //1.Temperature
   //2.IlliteFraction
   //3.HopaneIsomerization
   //4.SteraneAromatization
   //5.SteraneIsomerization
   //In order to store the above properties for a particular formation from Cauldron GUI:
   //Plot Property vs Time--> Select Property-->Select formation-->PrintTable

//III) 

   //If the temperature file has been saved under the name BENCHMARK and the to path is DIRECTORYNAME
   //use calibrionUnitTest as follows:
   //calibrationUnitTest -benchmark BENCHMARKNAME -dir DIRECTORYNAME -timestep TIMESTEPSIZE

   //calibrationUnitTest prompts the fullpath name of the result files for Biomarkers and SmectiteIllite

//IV)

   // Compare the results

int main(int argc, char *argv[])
{
	try
	{
   //Initialize timestepSize to be used if not given in command line
   double timestepSize = 100.0;

   if(argc < 5)
   {
      throw std::runtime_error("Invalid input parameters. Aborting...\nPlease use calibrationUnitTest -benchmark BENCHMARKNAME -dir DIRECTORYNAME -timestep TIMESTEPSIZE\n[-timestep] switch is optional. Default is 0.5");
       
   }
   std::string benchmarkName = argv[2];
   std::string inputHistoryFilePath = argv[4];
   
   if(argc == 7)
   {
      timestepSize = atof(argv[6]);
   }
   
   //Environment Variables

  

  
	CalibrationTestingUtilities::ProcessInput(argc, argv, benchmarkName, inputHistoryFilePath, timestepSize);

   //Create Smectite Illite Simulator
	const double in_ActEnergy1 = 55800.0;		//ActEnergy1	J/mol, Phase 1 activation energy 
	const double in_FreqFactor1 = 3.4229e+06;	//FreqFactor1	Phase 1  frequency factor
	const double in_ActEnergy2 = 13700.0;		//ActEnergy2  J/mol, Phase 2 activation energy
	const double in_FreqFactor2 = 1.0; 			//FreqFactor2	Phase 2 frequency  factor

	//Create SmectiteIlliteSimulator
	auto_ptr<Calibration::SmectiteIlliteSimulator> theSmectiteIlliteSimulator(new Calibration::SmectiteIlliteSimulator(in_ActEnergy1,
																																							 in_FreqFactor1,
																																							 in_ActEnergy2, 
																																							 in_FreqFactor2, 
																																							 timestepSize));
	 //Create Biomarkers Simulator
  	double HOPISOACTENERGY	  = 168000.000000;
   double STEISOACTENERGY	  = 169000.000000;
   double STEAROACTENERGY	  = 181400.000000;
   double HOPISOFREQFACTOR   = 810000000.000000;
   double STEISOFREQFACTOR   = 486000000.000000;
   double STEAROFREQFACTOR   = 48499998720.000000;
   double HOPISOGAMMA 		  = 1.564000;
   double STEISOGAMMA 		  = 1.174000;
	
	auto_ptr<Calibration::BiomarkersSimulator> theBiomarkersSimulator(new Calibration::BiomarkersSimulator(HOPISOACTENERGY,
																																			 STEISOACTENERGY,
																																			 STEAROACTENERGY,
																																			 HOPISOFREQFACTOR,
																																			 STEISOFREQFACTOR,
																																			 STEAROFREQFACTOR,
																																			 HOPISOGAMMA,   
																																			 STEISOGAMMA , 
																																			 timestepSize));
   //Create Calibration Node
   const double in_InitIlliteFraction = 0.0; 
   auto_ptr<Calibration::CalibrationNode> theNode(new Calibration::CalibrationNode(in_InitIlliteFraction));

   //Load Node Temperature History
	CalibrationTestingUtilities::LoadTestingTemperatureHistory( inputHistoryFilePath + benchmarkName, *theNode);

	//Compute
	theNode->requestComputation(*theSmectiteIlliteSimulator);
	theNode->requestComputation(*theBiomarkersSimulator);

	//Print results
	TestingSmectiteIllite::PrintBenchmarkResults(inputHistoryFilePath + "UnitTestResultsSmectiteIllite" + benchmarkName, *theNode);
	TestingBiomarkers::PrintBenchmarkResults(inputHistoryFilePath + "UnitTestResultsBiomarkers" + benchmarkName, *theNode);

	cout<<"Finished"<<endl;
	}
	catch(std::exception & ex)
	{
		cout<<ex.what()<<endl;
	}
   catch(const std::string &msg)
   {
      cout<<msg<<endl;
   }
	catch(...)
	{
		cout<<"Unknown error"<<endl;
	}
   return 0;
}



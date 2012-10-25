#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <exception>

#include "TestingCode.h"

namespace TestingSmectiteIllite
{

void LoadTestingTemperatureHistory(const std::string &in_FullPathFileName, std::vector<double> &time, std::vector<double> &temperature)
{
  	std::string line;
  	std::vector<std::string> theTokens;
  	std::string delim=" ";

  	ifstream PThistoryFile;
  	PThistoryFile.open(in_FullPathFileName.c_str());

  	if(PThistoryFile)
  	{
   	while(! PThistoryFile.eof())
   	{
      	std::getline(PThistoryFile,line,'\n');//get next
			cout<<line<<endl;
      	if ( line.empty() )
      	{
      		break;
      	}
			CalibrationTestingUtilities::ParseLine(line, delim, theTokens);

			time.push_back( atof( theTokens[0].c_str() ) );
			temperature.push_back( atof( theTokens[1].c_str() ) );

  	  		theTokens.clear();
    	}
    	PThistoryFile.close();
  	}
  	else
  	{
      std::string msg = "Invalid file :" + in_FullPathFileName;
      throw std::runtime_error(msg);
      
  	}
}
void ComputeIlliteTransformationHistory(const std::vector<double> &time, const std::vector<double> &temperature, std::vector<double> &Illite)
{
	const double GASCONSTANT = 8.3144;

	double stotal = 0.0; 
   double total1 = 0.0;
   double total2 = 0.0;
   double totalx = 0.0; 

	const double in_ActEnergy1 = 55800.0;		//ActEnergy1	J/mol, Phase 1 activation energy 
	const double in_FreqFactor1 = 3.4229e+06;	//FreqFactor1	Phase 1  frequency factor
	const double in_ActEnergy2 = 13700.0;		//ActEnergy2  J/mol, Phase 2 activation energy
	const double in_FreqFactor2 = 1.0; 			//FreqFactor2	Phase 2 frequency  factor
	const double in_InitIlliteFraction = 0.0; 

	Illite.push_back(0.0);
	double currentTime = time[0];
	static std::ofstream ResultFile;
	
	//ResultFile.open("PrototypeIntegralResultsNode90");
	//ResultFile<<"currentTime"<<" "<<"timestep"<<" "<<"rate1"<<" "<<"rate2"<<" "<<"stotal"<<" "<<"total1"<<" "<<"total2"<<" "<<"totalx"<<" "<<"totx"<<" "<<"toty"<<" "<<"smi"<<endl;
	for(size_t i = 1; i < time.size(); i++)
	{
		double timestep = time[i-1] - time[i];
		currentTime -= timestep;
		double temp = 0.5 * (temperature[i-1] + temperature[i]) + 273.15;

   	double rate1 = exp (-in_ActEnergy1 / (GASCONSTANT * temp));
   	double rate2 = exp (-in_ActEnergy2 / (GASCONSTANT * temp));

   	stotal += timestep * (in_FreqFactor2 * rate2 - in_FreqFactor1 * rate1);
   	total1 += rate1 * timestep * exp (stotal);
   	total2 += rate2 * timestep;
   	totalx += rate1 * timestep;

		double totx = exp (-in_FreqFactor1 * totalx);
   	double toty = in_FreqFactor1 * total1 * exp (-in_FreqFactor2 * total2);
   	double smi = in_InitIlliteFraction + (1.0 - in_InitIlliteFraction) * (1.0 - (totx + 0.4 * (toty)));
		
		//ResultFile<<currentTime<<" "<<timestep<<" "<<rate1<<" "<<rate2<<" "<<stotal<<" "<<total1<<" "<<total2<<" "<<totalx<<" "<<totx<<" "<<toty<<" "<<smi<<endl;
		
		Illite.push_back(smi);
	}
	//ResultFile.close();	
}
void PrintBenchmarkResults(const std::string &in_FullPathBenchmarkName, Calibration::CalibrationNode &Node)
{
	size_t size = Node.getSmectiteIlliteOutputSize();

   ofstream outputTestingSetFile;
   outputTestingSetFile.open(in_FullPathBenchmarkName.c_str());

	for(size_t i = size - 1; i > 0; --i)
	{
		const Calibration::SmectiteIlliteOutput *theOutput = Node.getSmectiteIlliteOutputByIndex(i); 
		const Calibration::NodeInput *theInput = Node.getInputByIndex(i); 
		
		outputTestingSetFile<<theInput->getReferenceTime()<<" "<<theInput->getTemperatureCelsius()<<" "<<theOutput->getIlliteTransfRatio()<<endl;
	}
	
	 cout<<"\nBenchmark is saved in file "<<in_FullPathBenchmarkName<<endl;
	//2do: write benchmark code
   outputTestingSetFile.close();
  
}
//*/


}
namespace CalibrationTestingUtilities
{
//calibrationUnitTest -benchmark BENCHMARKNAME -dir DIRECTORYNAME
void ProcessInput(int argc, char *argv[], std::string &benchmarkName, std::string &inputHistoryFilePath, double &temperature)
{
	if(7 <= argc)
	{
		if(5 <= argc)
		{
			benchmarkName 			= argv[2];
			inputHistoryFilePath = argv[4];
			inputHistoryFilePath += "/";
		}
		if(7 <= argc)
		{
			temperature = atof(argv[6]);
		}
	}
}

void ParseLine(const std::string &theString, const std::string &theDelimiter, std::vector<std::string> &theTokens)
{

   std::string::size_type startPos=0;
   std::string::size_type endPos=0;

   std::string::size_type increment=0;
   std::string Token;

   if(theString.empty() || theDelimiter.empty())
   {
      return;
   }
   while(endPos!=std::string::npos)
   {
      endPos=theString.find_first_of(theDelimiter,startPos);
      increment=endPos-startPos;

      Token=theString.substr(startPos,increment);
      if(Token.size()!=0)
      {
         theTokens.push_back(Token);
      }
      startPos+=increment+1;
   }
}
void LoadTestingTemperatureHistory(const std::string &in_FullPathFileName, Calibration::CalibrationNode &theNode)
{

  	std::string line;
  	std::vector<std::string> theTokens;
  	std::string delim=" ";

  	ifstream PThistoryFile;
  	PThistoryFile.open(in_FullPathFileName.c_str());

  	if(PThistoryFile)
  	{
		for(int i = 0; i < 4; i++)//omit header line
		{
   		std::getline(PThistoryFile,line,'\n');
		}
    
   	while(! PThistoryFile.eof())
   	{
      	std::getline(PThistoryFile,line,'\n');//get next
      	if ( line.empty() || "[End]" == line)
      	{
      		break;
      	}
			CalibrationTestingUtilities::ParseLine(line, delim, theTokens);

			Calibration::NodeInput *theInput = new Calibration::NodeInput(atof(theTokens[1].c_str()), atof(theTokens[4].c_str()));
         //cout<<"Adding :"<<theTokens[1]<<" "<<theTokens[4]<<endl;
      	theNode.addInput(theInput);
    
  	  		theTokens.clear();
    	}
    	PThistoryFile.close();
  	}
  	else
  	{
   	std::string msg = "Invalid file :" + in_FullPathFileName + " Aborting....";
      throw std::runtime_error(msg);
  	}
}

}
namespace TestingBiomarkers
{
class AromatizationFunction
{
public:
	AromatizationFunction(const double &frequencyFactor):
	m_frequencyFactor(frequencyFactor)
	{

	}
	~AromatizationFunction(){}
	double operator()(const double &integral) const
	{
		return (1. - exp (-m_frequencyFactor * integral)); 
	}
private:
	double m_frequencyFactor;
};
class IsomerizationFunction
{
public:
	IsomerizationFunction(const double &gamma, const double &frequencyFactor):
	m_gamma(gamma),
	m_frequencyFactor(frequencyFactor)
	{

	}
	~IsomerizationFunction(){}
	double operator()(const double &integral)
	{
   	return (m_gamma / (1. + m_gamma) * (1. - exp ((-1. - m_gamma) * m_frequencyFactor * integral)));
	}
private:
	double m_gamma;
	double m_frequencyFactor;
};
class BiomarkerKineticsFunction
{
public:
	BiomarkerKineticsFunction(const double &inActivationEnergy):
	m_activationEnergy(inActivationEnergy)
	{

	}
	~BiomarkerKineticsFunction(){}
	double operator()(const double &temperature) const
	{
		return exp (-m_activationEnergy / (GASCONSTANT * temperature));
	}
private:
	static const double GASCONSTANT = 8.3144;
	double m_activationEnergy;
};
class TrapezoidalIntegrator
{
public:
	double operator()(const BiomarkerKineticsFunction &theFunction, const double &timestepSize, const double &startTemp, const double &endTemp) const
	{
		return (timestepSize * 0.5 * (theFunction(startTemp) + theFunction(endTemp)));
	}

};
void ComputeBiomarkers(const std::vector<double> &time, const std::vector<double> &temperature, std::vector<TestingBiomarkers::BiomarkerOutput> &BiomarkerOutput)
{
	double HOPISOACTENERGY		= 168000.000000;
	double STEISOACTENERGY		= 169000.000000;
	double STEAROACTENERGY		= 181400.000000;
	double HOPISOFREQFACTOR		= 810000000.000000;
	double STEISOFREQFACTOR		= 486000000.000000;
	double STEAROFREQFACTOR		= 48499998720.000000;
	double HOPISOGAMMA			= 1.564000;
	double STEISOGAMMA			= 1.174000;

	TrapezoidalIntegrator theIntegrator;
	BiomarkerKineticsFunction theHopaneIsoKineticsFunction(HOPISOACTENERGY);
	BiomarkerKineticsFunction theSteraneIsoKineticsFunction(STEISOACTENERGY);
	BiomarkerKineticsFunction theSteraneAroKineticsFunction(STEAROACTENERGY);
	AromatizationFunction theSteraneAromatizationFunction(STEAROFREQFACTOR);
	IsomerizationFunction theSteraneIsomerizationFunction(STEISOGAMMA, STEISOFREQFACTOR);
	IsomerizationFunction theHopaneIsomerizationFunction(HOPISOGAMMA, HOPISOFREQFACTOR);

	double HopaneIsoIntegral = 0.0;
	double SteraneIsoIntegral = 0.0;
	double SteraneAroIntegral = 0.0;

	for(size_t i = 1; i < time.size(); i++)
	{
		double timestepSize = time[i-1] - time[i];
		
		double tempStart = temperature[i-1] + 273.5;
		double tempEnd   = temperature[i] + 273.5;

		TestingBiomarkers::BiomarkerOutput theOutput; 

		HopaneIsoIntegral  += theIntegrator(theHopaneIsoKineticsFunction,  timestepSize, tempStart, tempEnd);
		SteraneIsoIntegral += theIntegrator(theSteraneIsoKineticsFunction, timestepSize, tempStart, tempEnd);
		SteraneAroIntegral += theIntegrator(theSteraneAroKineticsFunction, timestepSize, tempStart, tempEnd);

		theOutput.HopaneIsomerisation  = theHopaneIsomerizationFunction ( HopaneIsoIntegral  );
		theOutput.SteraneIsomerisation = theSteraneIsomerizationFunction( SteraneIsoIntegral );
		theOutput.SteraneAromatisation = theSteraneAromatizationFunction( SteraneAroIntegral );
																												  
		BiomarkerOutput.push_back(theOutput);
	}
}
void PrintBenchmarkResults(const std::string &in_FullPathBenchmarkName, Calibration::CalibrationNode &Node)
{
	size_t size = Node.getBiomarkersOutputSize();

   ofstream outputTestingSetFile;
   outputTestingSetFile.open(in_FullPathBenchmarkName.c_str());

	outputTestingSetFile	<<"time"<<" "<<"temperature"<<" "<<"HopaneIsomerisation"<<" "<<"SteraneAromatisation"<<" "<<"SteraneIsomerisation"<<endl;
	for(size_t i = size - 1; i != 0; --i)
	{
		const Calibration::BiomarkersOutput *theOutput = Node.getBiomarkersOutputByIndex(i); 
		const Calibration::NodeInput *theInput = Node.getInputByIndex(i); 
		
		outputTestingSetFile	<<theInput->getReferenceTime()<<" "<<theInput->getTemperatureCelsius()<<" "
									<<theOutput->getHopaneIsomerisation()<<" "
									<<theOutput->getSteraneAromatisation()<<" "
									<<theOutput->getSteraneIsomerisation()
									<<endl;
	}
	
	 cout<<"\nBenchmark is saved in file "<<in_FullPathBenchmarkName<<endl;
	//2do: write benchmark code
   outputTestingSetFile.close();
}

}



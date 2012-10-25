#ifndef TESTING_CODE_H
#define TESTING_CODE_H

#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <exception>
#include <stdexcept>
#include "SmectiteIlliteSimulator.h"
#include "BiomarkersSimulator.h"
#include "CalibrationNode.h"
#include "NodeInput.h"
#include "SmectiteIlliteOutput.h" 
#include "BiomarkersOutput.h" 

namespace CalibrationTestingUtilities
{
	void ProcessInput(int argc, char *argv[], std::string &benchmarkName, std::string &inputHistoryFilePath, double &timestepSize);
	void ParseLine(const std::string &theString, const std::string &theDelimiter, std::vector<std::string> &theTokens);
	void LoadTestingTemperatureHistory(const std::string &in_FullPathFileName, Calibration::CalibrationNode &Node);
}
namespace TestingSmectiteIllite
{
	void PrintBenchmarkResults(const std::string &in_FullPathBenchmarkName, Calibration::CalibrationNode &Node);
	void LoadTestingTemperatureHistory(const std::string &in_FullPathFileName, std::vector<double> &time, std::vector<double> &temperature);
	void ComputeIlliteTransformationHistory(const std::vector<double> &time, const std::vector<double> &temperature, std::vector<double> &Illite);
}
namespace TestingBiomarkers
{
	struct BiomarkerOutput
	{
		double SteraneAromatisation;
		double SteraneIsomerisation;
		double HopaneIsomerisation;
	};

	void ComputeBiomarkers(const std::vector<double> &time, const std::vector<double> &temperature, std::vector<BiomarkerOutput> &BiomarkerOutput);
	void PrintBenchmarkResults(const std::string &in_FullPathBenchmarkName, Calibration::CalibrationNode &Node);
}

#endif

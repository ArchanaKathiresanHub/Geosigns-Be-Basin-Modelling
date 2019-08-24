//
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by CGI.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "RunOptionsModelConverter.h"

//cmbAPI
#include "RunOptionsManager.h"

//utilities
#include "LogHandler.h"

using namespace mbapi;



std::string Prograde::RunOptionsModelConverter::upgradeVelAlgorithm(std::string velAlgorithm) {
	if (velAlgorithm.compare("Gardner's Velocity-Density") == 0 || velAlgorithm.compare("Wyllie's Time-Average") == 0) {
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "VelAlgorithm upgraded from " << velAlgorithm << " to Kennan's Velocity-Porosity";
		return "Kennan's Velocity-Porosity";
	}
	else {
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Velocity model specified is as per the BPA2 model. No upgrade is required";
		return velAlgorithm;
	}
}

std::string Prograde::RunOptionsModelConverter::upgradeVreAlgorithm(std::string vreAlgorithm) {
	if (vreAlgorithm.compare("Burnham & Sweeney") != 0) {
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "VreAlgorithm upgraded from " << vreAlgorithm << " to Burnham & Sweeney";
	}
	else {
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "VRE model specified is as per the BPA2 model. No upgrade is required";
	}
	return "Burnham & Sweeney";
}

void Prograde::RunOptionsModelConverter::upgradePTCouplingMode(std::string& PTCouplingMode, int& coupledMode) {
	if (PTCouplingMode.compare("LooselyCoupled") == 0) {

		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "PTCoupling Mode upgraded from " << PTCouplingMode << " to ItCoupled";
		PTCouplingMode = "ItCoupled";
		coupledMode = 1;
	}
	else {
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "PT Coupling specified is as per the BPA2 model. No upgrade is required";
	}
}

std::string Prograde::RunOptionsModelConverter::upgradeChemicalCompactionAlgorithm(std::string chemCompactionAlgo) {
	std::string chemAlgo = "Walderhaug";
	if (chemCompactionAlgo.compare("Schneider") == 0) {
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Chemical compaction Algorithm upgraded from " << chemCompactionAlgo << " to Walderhaug";
	}
	else {
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Chemical Compaction Algorithm specified is as per the BPA2 model.No upgrade is required";
	}
	return chemAlgo;
}

std::string Prograde::RunOptionsModelConverter::upgradeOptimisationLevel(std::string optimisationLevel) {

	if (optimisationLevel.compare("Level 1") == 0 ||
		optimisationLevel.compare("Level 2") == 0 ||
		optimisationLevel.compare("Level 3") == 0) {
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Optimisation level upgraded from " << optimisationLevel << " to Level 4";
		return "Level 4";
	}
	else {
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Optimisation level specified is as per the BPA2 model. No upgrade is required";
		return optimisationLevel;
	}
}

double Prograde::RunOptionsModelConverter::upgradeTemperatureRange(double temperature, std::string fieldName) {
	if (temperature < 0.) {
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << fieldName + " upgraded from " << temperature << " to 0.0";
		return 0.;
	}
	if (temperature > 1e5) {
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << fieldName + " upgraded from " << temperature << " to 100000.0";
		return 1e5;
	}
	LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << fieldName + " values specified are within the allowed range for BPA2. No upgrade is required";
	return temperature;
}

double Prograde::RunOptionsModelConverter::upgradePressureRange(double pressure, std::string fieldName) {

	if (pressure < 0.) {
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << fieldName + " upgraded from " << pressure << " to 0.0";
		return 0.;
	}
	if (pressure > 1e5) {
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << fieldName + " upgraded from " << pressure << " to 100000.0";
		return 1e5;
	}
	LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << fieldName + " values specified are within the allowed range for BPA2 model. No upgrade is required";
	return pressure;
}
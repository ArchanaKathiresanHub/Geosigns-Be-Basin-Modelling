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
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Warning> VelAlgorithm upgraded from " << velAlgorithm << " to Kennan's Velocity-Porosity";
		return "Kennan's Velocity-Porosity";
	}
	else 
		return velAlgorithm;
}

std::string Prograde::RunOptionsModelConverter::upgradeVreAlgorithm(std::string vreAlgorithm) {
	if (vreAlgorithm.compare("Burnham & Sweeney") != 0) {
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Warning> VreAlgorithm upgraded from " << vreAlgorithm << " to Burnham & Sweeney";
	}
	return "Burnham & Sweeney";
}

void Prograde::RunOptionsModelConverter::upgradePTCouplingMode(std::string& PTCouplingMode, int& coupledMode) {
	if (PTCouplingMode.compare("LooselyCoupled") == 0) {

		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Warning> PTCoupling Mode upgraded from " << PTCouplingMode << " to ItCoupled";
		PTCouplingMode = "ItCoupled";
		coupledMode = 1;
	}
}

std::string Prograde::RunOptionsModelConverter::upgradeChemicalCompactionAlgorithm(std::string chemCompactionAlgo) {
	std::string chemAlgo = "Walderhaug";
	if (chemCompactionAlgo.compare("Schneider") == 0) {
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Warning> Chemical compaction Algorithm upgraded from " << chemCompactionAlgo << " to Walderhaug";
	}
	return chemAlgo;
}

std::string Prograde::RunOptionsModelConverter::upgradeOptimisationLevel(std::string optimisationLevel) {


	if (optimisationLevel.compare("Level 1") == 0 ||
		optimisationLevel.compare("Level 2") == 0 ||
		optimisationLevel.compare("Level 3") == 0) {
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Warning> Optimization level upgraded from " << optimisationLevel << " to Level 4";
		return "Level 4";
	}
	else 
		return optimisationLevel;
}

double Prograde::RunOptionsModelConverter::upgradeTemperatureRange(double temperature, std::string fieldName) {
	if (temperature < 0.) {
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Warning> "<< fieldName + " upgraded from " << temperature << " to 0.0";
		return 0.;
	}
	if (temperature > 1e5) {
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Warning> "<< fieldName + " upgraded from " << temperature << " to 100000.0";
		return 1e5;
	}
	return temperature;
}

double Prograde::RunOptionsModelConverter::upgradePressureRange(double pressure, std::string fieldName) {

	if (pressure < 0.) {
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) <<"<Basin-Warning> "<< fieldName + " upgraded from " << pressure << " to 0.0";
		return 0.;
	}
	if (pressure > 1e5) {
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) <<"<Basin-Warning> "<< fieldName + " upgraded from " << pressure << " to 100000.0";
		return 1e5;
	}
	return pressure;
}

int Prograde::RunOptionsModelConverter::upgradeLegacyFlag(int legacy)
{
	if (legacy == 1) legacy = 0;
	else throw ErrorHandler::Exception(ErrorHandler::ValidationError) << "Not a legacy scenario; Legacy flag is  "<< legacy <<" in RunOptionsIoTbl";
	return legacy;
}
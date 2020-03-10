//
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by CGI
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include "StratigraphyModelConverter.h"

//utilities
#include "LogHandler.h"

//cmbAPI
#include "StratigraphyManager.h"

using namespace mbapi;

std::string Prograde::StratigraphyModelConverter::upgradeName(const std::string &originalName)
{
	std::string updatedName;
	for (unsigned i = 0; i < originalName.size(); i++) { //ASCII characters for the allowed characters : (0-9) -> (48,57) ; (space) -> 32 ; (A-Z) -> (65-90) ; (a-z) -> (97-122) ; ( _ ) -> 95
		if ((originalName[i] >= 48 && originalName[i] <= 57) || (originalName[i] == 32) || (originalName[i] >= 65 && originalName[i] <= 90) || (originalName[i] >= 97 && originalName[i] <= 122) || (originalName[i] == 0) || (originalName[i] == 95)) {
			updatedName.push_back(originalName[i]);
		}
	}
	if (originalName.compare(updatedName) != 0) {
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << originalName << " is changed to " << updatedName;
	}
	else {
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "No change required";
	}
	return updatedName;
}

double Prograde::StratigraphyModelConverter::upgradeDepthThickness(const double & originalVal)
{
	double updatedVal = originalVal;
	if (updatedVal>6380000) {
		updatedVal = 6380000;
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "The value exceeds the upper limit of 6380000. Hence, trimmed to 6380000";
	}
	else if(updatedVal < -6380000){
		updatedVal = -6380000;
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "The value subceeds the lower limit of -6380000. Hence, trimmed to -6380000";
	}
	
	else {
		return updatedVal;
	}
		return updatedVal;
	
}

double Prograde::StratigraphyModelConverter::upgradeLayeringIndex(const std::string &mixModel, const double &originalLayeringIndex)
{
	double updatedLayeringIndex = originalLayeringIndex;
	if (!mixModel.compare("Homogeneous")){
		updatedLayeringIndex = -9999;
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Layering Index is changed from " << originalLayeringIndex << " to " << updatedLayeringIndex;
	}
	return updatedLayeringIndex;
}

int Prograde::StratigraphyModelConverter::checkChemicalCompaction(const std::string &ptCoupling, const int &originalChemCompaction_run)
{
	int	updatedChemCompaction_run = originalChemCompaction_run;
	if(!ptCoupling.compare("Hydrostatic")&&(updatedChemCompaction_run !=0)){
		updatedChemCompaction_run = 0;
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Hydrostatic mode detected! ChemicalCompaction flag in RunOptionsIoTbl is corrected from " << originalChemCompaction_run << " to " << updatedChemCompaction_run;
	}
	return updatedChemCompaction_run;
}

int Prograde::StratigraphyModelConverter::upgradeChemicalCompaction(const int &originalChemicalCompaction_run, const int &originalChemicalCompaction_layer)
{
	int updatedChemCompaction_layer = originalChemicalCompaction_layer;
	if (updatedChemCompaction_layer != originalChemicalCompaction_run) {
		updatedChemCompaction_layer = originalChemicalCompaction_run;
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "ChemicalCompaction is changed from " << originalChemicalCompaction_layer << " to " << updatedChemCompaction_layer;
	}
	return updatedChemCompaction_layer;
}
//
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by CGI
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include "SgsConverter.h"

//utilities
#include "LogHandler.h"

//cmbAPI
#include "SourceRockManager.h"

using namespace mbapi;

int Prograde::SgsConverter::upgradeAdsorptionTOCDependent(int legacyAdsorptionTOCDependent)
{
	if (legacyAdsorptionTOCDependent)
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Since TOC dependent adsoption model is deprecated in BPA2, AdsorptionTOCDependent is prograded from '1' to '0' in SourceRockLithoIoTbl";

	return 0;
}
std::string Prograde::SgsConverter::upgradeAdsorptionCapacityFunctionName(int legacyAdsorptionTOCDependent, const std::string & legacyAdsorptionCapacityFunctionName)
{
	std::string bpa2AdsorptionCapacityFunctionName = legacyAdsorptionCapacityFunctionName;
	if (legacyAdsorptionTOCDependent)
	{
		bpa2AdsorptionCapacityFunctionName = "Low Langmuir Isotherm";
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Since TOC dependent adsoption model is deprecated in BPA2, AdsorptionCapacityFunctionName is prograded from " <<"'"<< legacyAdsorptionCapacityFunctionName<<"'" << " to 'Low Langmuir Isotherm' in SourceRockLithoIoTbl";
	}
	else if (legacyAdsorptionCapacityFunctionName == "Langmuir Isotherm")
	{
		bpa2AdsorptionCapacityFunctionName = "Default Langmuir Isotherm";
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "AdsorptionCapacityFunctionName is prograded from " << "'" << legacyAdsorptionCapacityFunctionName << "'" << " to 'Default Langmuir Isotherm' in SourceRockLithoIoTbl";

	}
	
	return bpa2AdsorptionCapacityFunctionName;
}

std::string Prograde::SgsConverter::upgradeWhichAdsorptionSimulator(int legacyComputeOTGC)
{
	std::string bpa2WhichAdsorptionSimulator = "OTGCC1AdsorptionSimulator";
	if (!legacyComputeOTGC)	
	{
		bpa2WhichAdsorptionSimulator = "C1AdsorptionSimulator";
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "WhichAdsorptionSimulator is prograded from 'OTGCC1AdsorptionSimulator' to 'C1AdsorptionSimulator' in SourceRockLithoIoTbl ";
	}
		
	return bpa2WhichAdsorptionSimulator;
}
	

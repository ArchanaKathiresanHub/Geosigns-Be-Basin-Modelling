//
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "BrineModelConverter.h"

//utilities
#include "LogHandler.h"

//cmbAPI
#include "FluidManager.h"

using namespace mbapi;

FluidManager::FluidDensityModel Prograde::BrineModelConverter::upgradeDensityModel(const FluidManager::FluidDensityModel densModel, const std::string & fluidName) const {
	FluidManager::FluidDensityModel upgradedModel = densModel;
	switch (densModel) {
	case FluidManager::Constant:
	{
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "legacy density model detected for " << fluidName;
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "upgrading project to modified B&W density model for " << fluidName;
		upgradedModel = FluidManager::Calculated;
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "density model upgrade done for " << fluidName;
		break;
	}
	case FluidManager::Calculated:
	{
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "latest density model detected for " << fluidName;
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "no density model upgrade needed for " << fluidName;
		break;
	}
	default:
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "unknown density model detected for " << fluidName;
		upgradedModel = FluidManager::Calculated;
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "density model upgrade done for " << fluidName;
		break;
	}
	return upgradedModel;
}

FluidManager::CalculationModel Prograde::BrineModelConverter::upgradeSeismicVelocityModel(const FluidManager::CalculationModel seisVelModel, const std::string & fluidName) const {
	FluidManager::CalculationModel upgradedModel = seisVelModel;
	switch (seisVelModel) {
	case FluidManager::ConstantModel:
	{
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "legacy seismic velocity model detected for " << fluidName;
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "upgrading project to modified B&W seismic velocity model for " << fluidName;
		upgradedModel = FluidManager::CalculatedModel;
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "seismic velocity model upgrade done for " << fluidName;
		break;
	}
	case FluidManager::CalculatedModel:
	{
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "latest seismic velocity model detected for " << fluidName;
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "no seismic velocity model upgrade needed for " << fluidName;
		break;
	}
	default:
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "unknown seismic velocity model detected for " << fluidName;
		upgradedModel = FluidManager::CalculatedModel;
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "seismic velocity model upgrade done for " << fluidName;
		break;
	}
	return upgradedModel;
}
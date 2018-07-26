//
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by CGI
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

int Prograde::BrineModelConverter::upgradeUserDefined
(const std::string & fluidName, const int originalUserDefined)
{
   int upgradedUserDefined = originalUserDefined;
   if(fluidName == "Std. Marine Water")
   {
      if (upgradedUserDefined != 1)
      {
         LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "System defined " << fluidName << " detected";
         upgradedUserDefined = 1;
         LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << fluidName << " upgraded to user defined";
      }
      else
      {
         LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "User defined " << fluidName << " detected";
         LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << " no upgrade required";
      }
   }
   return upgradedUserDefined;
}

std::string Prograde::BrineModelConverter::upgradeDescription
(const std::string & fluidName, const std::string & originalDescription)
{
   std::string upgradedDescription = originalDescription;
   if (fluidName == "Std. Marine Water")
   {
      if (upgradedDescription != "KSEPL's Deprecated Marine Water")
      {
         LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Latest description not found for " << fluidName;
         upgradedDescription = std::string("KSEPL's Deprecated Marine Water");
         LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Description upgraded for " << fluidName;
      }
      else
      {
         LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Latest description found for " << fluidName;
         LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << " no upgrade required";
      }


   }
   return upgradedDescription;
}

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
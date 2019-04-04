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
   else if (fluidName != "Std. Sea Water" && fluidName != "Std. Water" && fluidName != "Std. Hyper Saline Water" )
   {
      
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "User defined brine is detected for" << fluidName ;
      upgradedUserDefined = 1;
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << fluidName << " updated to user defined flag value to 1";
      
   }
   
   
   return upgradedUserDefined;
}

std::string Prograde::BrineModelConverter::upgradeDescription
(const std::string & fluidName, const std::string & originalDescription)
{
   std::string upgradedDescription = originalDescription;
  
   if (upgradedDescription == "KSEPL's Standard Marine Water")
   {
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Latest description not found for " << fluidName;
      upgradedDescription = std::string("Deprecated Marine Water");
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Description upgraded for " << fluidName;
   }
   
   if (upgradedDescription == "KSEPL's Standard Water")
   {
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Latest description not found for " << fluidName;
      upgradedDescription = std::string("Standard Water");
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Description is upgraded by removing KSEPL's prefix for " << fluidName;
   }
  
  
   if (upgradedDescription == "KSEPL's Standard Ultra Marine Water")
   {
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Latest description not found for " << fluidName;
      upgradedDescription = std::string("Standard Ultra Marine Water");
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Description is upgraded by removing KSEPL's prefix for " << fluidName;
   }
     
   if (upgradedDescription == "KSEPL's Standard Sea Water")
   {
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Latest description not found for " << fluidName;
      upgradedDescription = std::string("Standard Sea Water");
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Description is upgraded by removing KSEPL's prefix for " << fluidName;
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

std::string Prograde::BrineModelConverter::upgradeHeatCapType
(const std::string & fluidName, const std::string & fluidDescription, const std::string & originalHeatCapType)
{
   std::string upgradedHeatCapType = originalHeatCapType;

   if(fluidDescription == "KSEPL's Standard Ultra Marine Water")
   {
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Latest HeatCapType is not found for " << fluidName;
      upgradedHeatCapType = "Std. Hyper Saline Water";
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "HeatCapType is upgraded for " << fluidName;
   }
   else if (fluidDescription == "KSEPL's Standard Sea Water")
   {
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Latest HeatCapType is not found for " << fluidName;
      upgradedHeatCapType = "Std. Sea Water";
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "HeatCapType is upgraded for " << fluidName;
   }
   else
   {
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Latest HeatCapType is found for " << fluidName;
      upgradedHeatCapType = "Std. Water";
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "No upgradation is required for " << fluidName;
   }
   return upgradedHeatCapType;
}

std::string Prograde::BrineModelConverter::upgradeThermCondType
(const std::string & fluidName, const std::string & fluidDescription, const std::string & originalThermCondType)
{
   std::string upgradedThermCondType = originalThermCondType;

   if (fluidDescription == "KSEPL's Standard Ultra Marine Water")
   {
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Latest HeatCapType is not found for " << fluidName;
      upgradedThermCondType = "Std. Hyper Saline Water";
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "HeatCapType is upgraded for " << fluidName;
   }
   else if (fluidDescription == "KSEPL's Standard Sea Water")
   {
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Latest HeatCapType is not found for " << fluidName;
      upgradedThermCondType = "Std. Sea Water";
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "HeatCapType is upgraded for " << fluidName;
   }
   else
   {
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Latest HeatCapType is found for " << fluidName;
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "No upgradation is required for " << fluidName;
   }
   return upgradedThermCondType;
}
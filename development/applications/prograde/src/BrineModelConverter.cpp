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
         upgradedUserDefined = 1;
         LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "UserDefined: "<< fluidName << " is no longer a system defined brine in BPA2. The UserDefined flag is updated to " << upgradedUserDefined;
      }
      else
      {
         LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "User defined " << fluidName << " detected";
         LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << " no upgrade required";
      }
   }
   else if (fluidName != "Std. Sea Water" && fluidName != "Std. Water" && fluidName != "Std. Hyper Saline Water" )
   {
      upgradedUserDefined = 1;
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "UserDefined: " << fluidName << " is a user defined brine. The UserDefined flag is updated to " << upgradedUserDefined;
      
   }
   else
   {
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "UserDefined: " << fluidName << " is a system defined brine. No upgradation is required";
   }
   
   
   return upgradedUserDefined;
}

std::string Prograde::BrineModelConverter::upgradeDescription
(const std::string & fluidName, const std::string & originalDescription)
{
   std::string upgradedDescription = originalDescription;
   if (upgradedDescription == "KSEPL's Standard Marine Water")
   {
      upgradedDescription = std::string("Deprecated Marine Water");
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Description: Upgraded by removing KSEPL's prefix for " << fluidName;
   }
   
   if (upgradedDescription == "KSEPL's Standard Water")
   {
      upgradedDescription = std::string("Standard Water");
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Description: Upgraded by removing KSEPL's prefix for " << fluidName;
   }
  
  
   if (upgradedDescription == "KSEPL's Standard Ultra Marine Water")
   {
      upgradedDescription = std::string("Standard Ultra Marine Water");
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Description: Upgraded by removing KSEPL's prefix for " << fluidName;
   }
     
   if (upgradedDescription == "KSEPL's Standard Sea Water")
   {
      upgradedDescription = std::string("Standard Sea Water");
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Description: Upgraded by removing KSEPL's prefix for " << fluidName;
   }
 
   return upgradedDescription;
}

FluidManager::FluidDensityModel Prograde::BrineModelConverter::upgradeDensityModel(const FluidManager::FluidDensityModel densModel, const std::string & fluidName) const {
   FluidManager::FluidDensityModel upgradedModel = densModel;
   switch (densModel) {
   case FluidManager::Constant:
   {
      upgradedModel = FluidManager::Calculated;
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Density model: Legacy model is detected for "<< fluidName << ". Upgrading it to modified B&W density model ";
		break;
   }
	case FluidManager::Calculated:
   {
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Density model: Latest density model is detected for " << fluidName << ". No upgrade is needed ";
      break;
   }
   default:
      upgradedModel = FluidManager::Calculated;
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Density model: Unknown density model is detected for " << fluidName << ". Upgrading it to modified B&W density model";
      break;
   }
   return upgradedModel;
}

FluidManager::CalculationModel Prograde::BrineModelConverter::upgradeSeismicVelocityModel(const FluidManager::CalculationModel seisVelModel, const std::string & fluidName) const {
   FluidManager::CalculationModel upgradedModel = seisVelModel;
   switch (seisVelModel) {
   case FluidManager::ConstantModel:
   {
      upgradedModel = FluidManager::CalculatedModel;
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Seismic velocity model: Legacy model is detected for " << fluidName << ". Upgrading it to modified B&W seismic velocity model ";
      break;
   }
   case FluidManager::CalculatedModel:
   {
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Seismic velocity model: Latest model is detected for " << fluidName << ". No upgrade is needed "; 
      break;
   }
   default:
      upgradedModel = FluidManager::CalculatedModel;
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Seismic velocity model: Unknown seismic velocity model is detected for " << fluidName << ". Upgrading it to modified B&W seismic velocity model"; 
      break;
   }
   return upgradedModel;
}

std::string Prograde::BrineModelConverter::upgradeHeatCapType
(const std::string & fluidName, const std::string & fluidDescription, const std::string & originalHeatCapType)
{
   std::string upgradedHeatCapType;

   if(fluidDescription == "KSEPL's Standard Ultra Marine Water")
   {
      upgradedHeatCapType = "Std. Hyper Saline Water";
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "HeatCapType: Legacy moddel is found for this brine. It is upgraded from "<< originalHeatCapType<< " to " << upgradedHeatCapType;
   }
   else if (fluidDescription == "KSEPL's Standard Sea Water")
   {
      upgradedHeatCapType = "Std. Sea Water";
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "HeatCapType: Legacy moddel is found for this brine. It is upgraded from " << originalHeatCapType << " to " << upgradedHeatCapType;
   }
   else
   {
      upgradedHeatCapType = "Std. Water";
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "HeatCapType: Latest model is found for this brine. No upgradation is required";
   }
   return upgradedHeatCapType;
}

std::string Prograde::BrineModelConverter::upgradeThermCondType
(const std::string & fluidName, const std::string & fluidDescription, const std::string & originalThermCondType)
{
   std::string upgradedThermCondType;
   
   if (fluidDescription == "KSEPL's Standard Ultra Marine Water")
   {
      upgradedThermCondType = "Std. Hyper Saline Water";
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "ThermCondType: Legacy moddel is found for this brine. It is upgraded from  "<< originalThermCondType<<" to " << upgradedThermCondType;
   }
   else if (fluidDescription == "KSEPL's Standard Sea Water")
   {
      upgradedThermCondType = "Std. Sea Water";
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "ThermCondType: Legacy moddel is not found for this brine. It is upgraded from  " << originalThermCondType << " to " << upgradedThermCondType;
   }
   else
   {
      upgradedThermCondType = originalThermCondType;
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "ThermCondType: Latest model is found for this brine. No upgradation is required";
   }
   return upgradedThermCondType;
}
std::string Prograde::BrineModelConverter::upgradeDefinedBy(const std::string & fluidName, const std::string & OriginalDefinedBy)
{
   std::string upgradedDefinedBy;
   if (fluidName == "Std. Water" || fluidName == "Std. Sea Water" || fluidName == "Std. Hyper Saline Water")
   {
      upgradedDefinedBy = "System";
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "DefinedBy: "<< fluidName << " is a system defined brine. The DefinedBy field is updated from "<< OriginalDefinedBy<<" to " << upgradedDefinedBy ;
   }
   else
   {
      upgradedDefinedBy = "BPA user";
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "DefinedBy: " << fluidName << " is a user defined brine. The DefinedBy field is updated from " << OriginalDefinedBy << " to " << upgradedDefinedBy;
   }

   return upgradedDefinedBy;
}

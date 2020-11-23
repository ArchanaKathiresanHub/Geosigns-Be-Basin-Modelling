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
		 LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) <<"<Basin-Info> " <<fluidName << ", is no longer a system defined brine in BPA2; UserDefined flag is updated from " << originalUserDefined << " to " << upgradedUserDefined;
      }
   }
   else if (fluidName != "Std. Sea Water" && fluidName != "Std. Water" && fluidName != "Std. Hyper Saline Water" )
   {
      upgradedUserDefined = 1;
	  LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Info> " << fluidName << ", is a user defined brine; UserDefined flag is updated from " << originalUserDefined << " to " << upgradedUserDefined;
      
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
	  LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Info> " << fluidName << ", Description is upgraded from '" << originalDescription << "' to '" << upgradedDescription << "' by removing KSEPL's prefix";
   }
   
   if (upgradedDescription == "KSEPL's Standard Water")
   {
      upgradedDescription = std::string("Standard Water");
	  LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Info> " << fluidName << ", Description is upgraded from '" << originalDescription << "' to '" << upgradedDescription << "' by removing KSEPL's prefix";
   }
  
  
   if (upgradedDescription == "KSEPL's Standard Ultra Marine Water")
   {
      upgradedDescription = std::string("Standard Ultra Marine Water");
	  LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Info> " << fluidName << ", Description is upgraded from '" << originalDescription << "' to '" << upgradedDescription << "' by removing KSEPL's prefix";
   }
     
   if (upgradedDescription == "KSEPL's Standard Sea Water")
   {
      upgradedDescription = std::string("Standard Sea Water");
	  LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Info> " << fluidName << ", Description is upgraded from '" << originalDescription << "' to '" << upgradedDescription << "' by removing KSEPL's prefix";
   }
 
   return upgradedDescription;
}

FluidManager::FluidDensityModel Prograde::BrineModelConverter::upgradeDensityModel(const FluidManager::FluidDensityModel densModel, const std::string & fluidName) const {
   FluidManager::FluidDensityModel upgradedModel = densModel;
   switch (densModel) {
   case FluidManager::Constant:
   {
      upgradedModel = FluidManager::Calculated;
	  LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Warning> " << fluidName << ", density model is upgraded from 'User Defined' to 'Modified B&W density model' as the model is deprecated in BPA2";
	  break;
   }
	case FluidManager::Calculated:
   {
      break;
   }
   default:
      upgradedModel = FluidManager::Calculated;
	  LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Warning> " << fluidName << ", an unknown density model is detected; upgraded to 'Modified B&W density model' as the model is deprecated in BPA2";
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
	  LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Warning> " << fluidName << ", seismic velocity model is upgraded from 'User Defined' to 'Modified B&W seismic velocity model' as the model is deprecated in BPA2";
      break;
   }
   case FluidManager::CalculatedModel:
   {
      break;
   }
   default:
      upgradedModel = FluidManager::CalculatedModel;
	  LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Warning> " << fluidName << ", an unknown seismic velocity model is detected ; upgraded to 'Modified B&W seismic velocity model' as the model is deprecated in BPA2";
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
	  LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Info> " << fluidName << ", HeatCapType is upgraded from " << originalHeatCapType << " to " << upgradedHeatCapType;
   }
   else if (fluidDescription == "KSEPL's Standard Sea Water")
   {
      upgradedHeatCapType = "Std. Sea Water";
	  LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Info> " << fluidName << ", HeatCapType is upgraded from " << originalHeatCapType << " to " << upgradedHeatCapType;
   }
   else
   {
      upgradedHeatCapType = "Std. Water";
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
	  LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Info> " << fluidName << ", ThermCondType is upgraded from " << originalThermCondType << " to " << upgradedThermCondType;
   }
   else if (fluidDescription == "KSEPL's Standard Sea Water")
   {
      upgradedThermCondType = "Std. Sea Water";
	  LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Info> " << fluidName << ", ThermCondType is upgraded from " << originalThermCondType << " to " << upgradedThermCondType;
   }
   else
   {
      upgradedThermCondType = originalThermCondType;
   }
   return upgradedThermCondType;
}
std::string Prograde::BrineModelConverter::upgradeDefinedBy(const std::string & fluidName, const std::string & OriginalDefinedBy)
{
   std::string upgradedDefinedBy;
   if (fluidName == "Std. Water" || fluidName == "Std. Sea Water" || fluidName == "Std. Hyper Saline Water")
   {
      upgradedDefinedBy = "System";
	  LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Info> " << fluidName << ", is a system defined brine; The DefinedBy field is updated from " << OriginalDefinedBy << " to " << upgradedDefinedBy;
   }
   else
   {
      upgradedDefinedBy = "BPA user";
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Info> " << fluidName << ", is a user defined brine. The DefinedBy field is updated from " << OriginalDefinedBy << " to " << upgradedDefinedBy;
   }

   return upgradedDefinedBy;
}

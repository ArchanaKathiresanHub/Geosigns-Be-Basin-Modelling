//
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by CGI
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include "AlcModelConverter.h"

//utilities
#include "LogHandler.h"

//cmbAPI
#include "BottomBoundaryManager.h"

using namespace mbapi;


BottomBoundaryManager::CrustPropertyModel Prograde::AlcModelConverter::upgradeAlcCrustPropModel(const BottomBoundaryManager::CrustPropertyModel CrPropModel) const {
   BottomBoundaryManager::CrustPropertyModel myCrPropModel = CrPropModel;
   switch (CrPropModel) {
   case BottomBoundaryManager::LegacyCrust:
   {
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "legacy crust model detected ";
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "upgrading crust property model to standard conductivity model";
      myCrPropModel = BottomBoundaryManager::StandardCondModel;
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "crust property model upgrade done";
      break;
   }
   case BottomBoundaryManager::LowCondModel:
   {
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "low conductivity crust model detected ";
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "upgrading crust property model to standard conductivity model";
      myCrPropModel = BottomBoundaryManager::StandardCondModel;
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "crust property model upgrade done";
      break;
   }
   case BottomBoundaryManager::StandardCondModel:
   {
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "latest crust property model detected";
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "no upgrade needed ";
      break;
   }
   default:
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "unknown crust property model detected";
      myCrPropModel = BottomBoundaryManager::StandardCondModel;
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "crust property model upgrade done";
      break;
   }
   return myCrPropModel;

}

BottomBoundaryManager::MantlePropertyModel Prograde::AlcModelConverter::upgradeAlcMantlePropModel(const BottomBoundaryManager::MantlePropertyModel MnPropModel) const {
   BottomBoundaryManager::MantlePropertyModel myMnPropModel = MnPropModel;
   switch (MnPropModel) {
   case BottomBoundaryManager::LegacyMantle:
   {
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "legacy mantle model detected ";
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "upgrading mantle property model to High Conductivity Mantle model";
      myMnPropModel = BottomBoundaryManager::HighCondMnModel;
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "mantle property model upgrade done";
      break;
   }
   case BottomBoundaryManager::LowCondMnModel:
   {
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "low conductivity mantle model detected ";
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "upgrading mantle property model to High Conductivity Mantle model";
      myMnPropModel = BottomBoundaryManager::HighCondMnModel;
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "mantle property model upgrade done";
      break;
   }
   case BottomBoundaryManager::StandardCondMnModel:
   {
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "standard conductivity mantle model detected ";
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "upgrading mantle property model to High Conductivity Mantle model";
      myMnPropModel = BottomBoundaryManager::HighCondMnModel;
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "mantle property model upgrade done";
      break;
   }
   case BottomBoundaryManager::HighCondMnModel:
   {
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "latest mantle property model detected";
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "no upgrade needed ";
      break;
   }
   default:
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "unknown mantle property model detected";
      myMnPropModel = BottomBoundaryManager::HighCondMnModel;
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "mantle property model upgrade done";
      break;
   }
   return myMnPropModel;

}

std::string Prograde::AlcModelConverter::updateBottomBoundaryModel(std::string & BottomBoundaryModel_original)
{
	std::string BottomBoundaryModel_updated = BottomBoundaryModel_original;
	if (!BottomBoundaryModel_updated.compare("Advanced Lithosphere Calculator")) {
		BottomBoundaryModel_updated = "Improved Lithosphere Calculator Linear Element Mode";
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "BottomBoundaryModel changed from 'Advanced Lithosphere Calculator' to 'Improved Lithosphere Calculator Linear Element Mode'";
	}
	return BottomBoundaryModel_updated;
}

double Prograde::AlcModelConverter::updateTopCrustHeatProd(double TopCrustHeatProd)
{
	if (TopCrustHeatProd < 0) {
		TopCrustHeatProd = 0;
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "TopCrustHeatProd subceeds the lower limit : 0. Hence, trimmed to 0";
	}
	else if (TopCrustHeatProd > 1000) {
		TopCrustHeatProd = 1000;
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "TopCrustHeatProd exceeds the upper limit : 1000. Hence, trimmed to 1000";
	}

	return TopCrustHeatProd;
}

double Prograde::AlcModelConverter::updateInitialLithosphericMantleThickness(double InitialLithosphericMantleThickness)
{
	if (InitialLithosphericMantleThickness < 0) {
		InitialLithosphericMantleThickness = 0;
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "InitialLithosphericMantleThickness subceeds the lower limit : 0. Hence, trimmed to 0";
	}
	else if (InitialLithosphericMantleThickness > 6300000) {
		InitialLithosphericMantleThickness = 6300000;
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "InitialLithosphericMantleThickness exceeds the upper limit : 1000. Hence, trimmed to 1000";
	}
		
	return InitialLithosphericMantleThickness;
}




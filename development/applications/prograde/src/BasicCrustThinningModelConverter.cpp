//
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by CGI
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include "BasicCrustThinningModelConverter.h"

//utilities
#include "LogHandler.h"

//cmbAPI
#include "BottomBoundaryManager.h"

using namespace mbapi;

BottomBoundaryManager::BottomBoundaryModel Prograde::BasicCrustThinningModelConverter::upgradeBotBoundModel(const BottomBoundaryManager::BottomBoundaryModel BBCModel)
{
   BottomBoundaryManager::BottomBoundaryModel myBBCModel = BBCModel;
   if (myBBCModel == BottomBoundaryManager::BasicCrustThinning)
   {
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) <<"Deprecated basic crust thinning model detected";
      myBBCModel = BottomBoundaryManager::AdvancedCrustThinning;
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) <<" upgraded to new Crust thinning model ";
   }
   
   return myBBCModel;
}

BottomBoundaryManager::CrustPropertyModel Prograde::BasicCrustThinningModelConverter::upgradeCrustPropModel(const BottomBoundaryManager::CrustPropertyModel CrPropModel) const {
   BottomBoundaryManager::CrustPropertyModel myCrPropModel = CrPropModel;
   switch(CrPropModel){
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

BottomBoundaryManager::MantlePropertyModel Prograde::BasicCrustThinningModelConverter::upgradeMantlePropModel(const BottomBoundaryManager::MantlePropertyModel MnPropModel) const {
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

std::string Prograde::BasicCrustThinningModelConverter::upgradeGridMapTable(const std::string & OriginalName)
{
   std::string upgradedName= OriginalName;
   if (OriginalName == "CrustIoTbl")
   {
      upgradedName = "ContCrustalThicknessIoTbl";

      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "CrustIoTbl is detected in GridMapIoTbl";
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "upgraded to ContCrustalThicknessIoTbl";
   }
      
   return upgradedName;
}
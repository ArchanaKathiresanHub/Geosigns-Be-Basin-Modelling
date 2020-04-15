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
   if (myBBCModel == BottomBoundaryManager::BottomBoundaryModel::BasicCrustThinning)
   {
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) <<"Deprecated basic crust thinning model detected";
      myBBCModel = BottomBoundaryManager::BottomBoundaryModel::AdvancedCrustThinning;
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) <<"upgraded to new Crust thinning model: Advanced Lithosphere Calculator";
   }
   
   return myBBCModel;
}

BottomBoundaryManager::CrustPropertyModel Prograde::BasicCrustThinningModelConverter::upgradeCrustPropModel(const BottomBoundaryManager::CrustPropertyModel CrPropModel) const {
   BottomBoundaryManager::CrustPropertyModel myCrPropModel = CrPropModel;
   switch(CrPropModel){
   case BottomBoundaryManager::CrustPropertyModel::LegacyCrust:
   {
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "legacy crust model detected, upgrading crust property model to standard conductivity model";
      myCrPropModel = BottomBoundaryManager::CrustPropertyModel::StandardCondModel;
      break;
   }
   case BottomBoundaryManager::CrustPropertyModel::LowCondModel:
   {
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "low conductivity crust model detected, upgrading crust property model to standard conductivity model";
      myCrPropModel = BottomBoundaryManager::CrustPropertyModel::StandardCondModel;
      break;
   }
   case BottomBoundaryManager::CrustPropertyModel::StandardCondModel:
   {
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "latest crust property model detected, no upgrade needed ";
      break;
   }
   default:
   myCrPropModel = BottomBoundaryManager::CrustPropertyModel::StandardCondModel;
   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "unknown crust property model detected, crust property model upgrade done";
   break;
   }
   return myCrPropModel;

}

BottomBoundaryManager::MantlePropertyModel Prograde::BasicCrustThinningModelConverter::upgradeMantlePropModel(const BottomBoundaryManager::MantlePropertyModel MnPropModel) const {
   BottomBoundaryManager::MantlePropertyModel myMnPropModel = MnPropModel;
   switch (MnPropModel) {
   case BottomBoundaryManager::MantlePropertyModel::LegacyMantle:
   {
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "legacy mantle model detected, upgrading mantle property model to High Conductivity Mantle model";
      myMnPropModel = BottomBoundaryManager::MantlePropertyModel::HighCondMnModel;
      break;
   }
   case BottomBoundaryManager::MantlePropertyModel::LowCondMnModel:
   {
       LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "low conductivity mantle model detected, upgrading mantle property model to High Conductivity Mantle model";
       myMnPropModel = BottomBoundaryManager::MantlePropertyModel::HighCondMnModel;
       break;
   }
   case BottomBoundaryManager::MantlePropertyModel::StandardCondMnModel:
   {
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "standard conductivity mantle model detected, upgrading mantle property model to High Conductivity Mantle model";
      myMnPropModel = BottomBoundaryManager::MantlePropertyModel::HighCondMnModel;
      break;
   }
   case BottomBoundaryManager::MantlePropertyModel::HighCondMnModel:
   {
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "latest mantle property model detected, no upgrade needed ";
      break;
   }
   default:
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "unknown mantle property model detected";
      myMnPropModel = BottomBoundaryManager::MantlePropertyModel::HighCondMnModel;
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
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "CrustIoTbl is detected in GridMapIoTbl, upgraded to ContCrustalThicknessIoTbl";
   }
   if (OriginalName == "BasaltThicknessIoTbl")
   {
       upgradedName = "";
       LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "BasaltThicknessIoTbl is detected in GridMapIoTbl, cleared BasaltThicknessIoTbl";
   }
   if (OriginalName == "MntlHeatFlowIoTbl")
   {
       upgradedName = "";
       LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "MntlHeatFlowIoTbl is detected in GridMapIoTbl, cleared MntlHeatFlowIoTbl";
   }
      
   return upgradedName;
}
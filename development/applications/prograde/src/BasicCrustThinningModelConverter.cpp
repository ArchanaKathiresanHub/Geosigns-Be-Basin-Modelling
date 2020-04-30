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
      myBBCModel = BottomBoundaryManager::BottomBoundaryModel::ImprovedCrustThinningLinear;
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) <<"Upgraded to new Crust thinning model: 'Improved Lithosphere Calculator Linear Element Mode'";
   }
   
   return myBBCModel;
}

BottomBoundaryManager::CrustPropertyModel Prograde::BasicCrustThinningModelConverter::upgradeCrustPropModel(const BottomBoundaryManager::CrustPropertyModel CrPropModel) const {
   BottomBoundaryManager::CrustPropertyModel myCrPropModel = CrPropModel;
   switch(CrPropModel){
   case BottomBoundaryManager::CrustPropertyModel::LegacyCrust:
   {
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Legacy crust model detected, upgrading crust property model to 'Standard Conductivity Crust' model";
      myCrPropModel = BottomBoundaryManager::CrustPropertyModel::StandardCondModel;
      break;
   }
   case BottomBoundaryManager::CrustPropertyModel::LowCondModel:
   {
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Low conductivity crust model detected, upgrading crust property model to 'Standard Conductivity Crust' model";
      myCrPropModel = BottomBoundaryManager::CrustPropertyModel::StandardCondModel;
      break;
   }
   case BottomBoundaryManager::CrustPropertyModel::StandardCondModel:
   {
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Latest crust property model detected, no upgrade needed ";
      break;
   }
   default:
   myCrPropModel = BottomBoundaryManager::CrustPropertyModel::StandardCondModel;
   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Unknown crust property model detected, crust property model is upgraded to 'Standard Conductivity Crust' model.";
   break;
   }
   return myCrPropModel;

}

BottomBoundaryManager::MantlePropertyModel Prograde::BasicCrustThinningModelConverter::upgradeMantlePropModel(const BottomBoundaryManager::MantlePropertyModel MnPropModel) const {
   BottomBoundaryManager::MantlePropertyModel myMnPropModel = MnPropModel;
   switch (MnPropModel) {
   case BottomBoundaryManager::MantlePropertyModel::LegacyMantle:
   {
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Legacy mantle model detected, upgrading mantle property model to 'High Conductivity Mantle' model";
      myMnPropModel = BottomBoundaryManager::MantlePropertyModel::HighCondMnModel;
      break;
   }
   case BottomBoundaryManager::MantlePropertyModel::LowCondMnModel:
   {
       LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Low conductivity mantle model detected, upgrading mantle property model to 'High Conductivity Mantle' model";
       myMnPropModel = BottomBoundaryManager::MantlePropertyModel::HighCondMnModel;
       break;
   }
   case BottomBoundaryManager::MantlePropertyModel::StandardCondMnModel:
   {
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Standard conductivity mantle model detected, upgrading mantle property model to 'High Conductivity Mantle' model";
      myMnPropModel = BottomBoundaryManager::MantlePropertyModel::HighCondMnModel;
      break;
   }
   case BottomBoundaryManager::MantlePropertyModel::HighCondMnModel:
   {
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Latest mantle property model detected, no upgrade needed ";
      break;
   }
   default:
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Unknown mantle property model detected, mantle property model is upgraded to 'High Conductivity Mantle' model";
      myMnPropModel = BottomBoundaryManager::MantlePropertyModel::HighCondMnModel;
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
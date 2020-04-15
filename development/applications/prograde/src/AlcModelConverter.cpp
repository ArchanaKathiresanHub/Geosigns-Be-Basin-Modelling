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
   case BottomBoundaryManager::CrustPropertyModel::LegacyCrust:
   {
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "legacy crust model detected ";
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "upgrading crust property model to standard conductivity model";
      myCrPropModel = BottomBoundaryManager::CrustPropertyModel::StandardCondModel;
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "crust property model upgrade done";
      break;
   }
   case BottomBoundaryManager::CrustPropertyModel::LowCondModel:
   {
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "low conductivity crust model detected ";
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "upgrading crust property model to standard conductivity model";
      myCrPropModel = BottomBoundaryManager::CrustPropertyModel::StandardCondModel;
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "crust property model upgrade done";
      break;
   }
   case BottomBoundaryManager::CrustPropertyModel::StandardCondModel:
   {
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "latest crust property model detected";
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "no upgrade needed ";
      break;
   }
   default:
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "unknown crust property model detected";
      myCrPropModel = BottomBoundaryManager::CrustPropertyModel::StandardCondModel;
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "crust property model upgrade done";
      break;
   }
   return myCrPropModel;

}

BottomBoundaryManager::MantlePropertyModel Prograde::AlcModelConverter::upgradeAlcMantlePropModel(const BottomBoundaryManager::MantlePropertyModel MnPropModel) const {
   BottomBoundaryManager::MantlePropertyModel myMnPropModel = MnPropModel;
   switch (MnPropModel) {
   case BottomBoundaryManager::MantlePropertyModel::LegacyMantle:
   {
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "legacy mantle model detected ";
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "upgrading mantle property model to High Conductivity Mantle model";
      myMnPropModel = BottomBoundaryManager::MantlePropertyModel::HighCondMnModel;
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "mantle property model upgrade done";
      break;
   }
   case BottomBoundaryManager::MantlePropertyModel::LowCondMnModel:
   {
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "low conductivity mantle model detected ";
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "upgrading mantle property model to High Conductivity Mantle model";
      myMnPropModel = BottomBoundaryManager::MantlePropertyModel::HighCondMnModel;
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "mantle property model upgrade done";
      break;
   }
   case BottomBoundaryManager::MantlePropertyModel::StandardCondMnModel:
   {
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "standard conductivity mantle model detected ";
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "upgrading mantle property model to High Conductivity Mantle model";
      myMnPropModel = BottomBoundaryManager::MantlePropertyModel::HighCondMnModel;
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "mantle property model upgrade done";
      break;
   }
   case BottomBoundaryManager::MantlePropertyModel::HighCondMnModel:
   {
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "latest mantle property model detected";
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "no upgrade needed ";
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


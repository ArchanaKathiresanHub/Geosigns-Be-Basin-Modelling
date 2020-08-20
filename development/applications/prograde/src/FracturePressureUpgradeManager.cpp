//
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by CGI.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "FracturePressureUpgradeManager.h"

//std
#include <vector>

//utilities
#include "LogHandler.h"

//Prograde
#include "FracturePressureModelConverter.h"

//cmbAPI
#include "cmbAPI.h"
#include "FracturePressureManager.h"

//DataAccess
#include "ProjectHandle.h"
#include "RunParameters.h"

using namespace mbapi;

//------------------------------------------------------------//

Prograde::FracturePressureUpgradeManager::FracturePressureUpgradeManager(mbapi::Model& model) :
   IUpgradeManager("fracture pressure model upgrade manager"), m_model(model)
{
   const auto ph = m_model.projectHandle();
   if (ph == nullptr) {
      throw std::invalid_argument(getName() + " cannot retrieve the project handle from Cauldron data model");
   }
   m_ph = ph;
}

//------------------------------------------------------------//
void Prograde::FracturePressureUpgradeManager::upgrade() {
   Prograde::FracturePressureModelConverter modelConverter;

   std::string FPName, PressFuncIoTblModel;
   m_model.fracturePressureManager().getFracturePressureFunctionName(FPName);
   int TableSize = m_model.tableSize("PressureFuncIoTbl");
   
   if (FPName == "None")
   {
      m_model.fracturePressureManager().setFracturePressureFunctionName(modelConverter.upgradeFracturePressureModel());
      m_model.clearTable("PressureFuncIoTbl");

	  LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Info> Deprecated 'NONE' model is detected for fracture pressure modelling; Upgraded the model name by blank quotes";
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Info> PressureFuncIoTbl cleared";
   }
   else if( TableSize >1)
   {
      auto FracPressId = m_model.fracturePressureManager().getpressFuncTblID();
     
      std::vector<int> countV; int count = 0;
      for (auto tsId : FracPressId)
      {
         m_model.fracturePressureManager().getPresFuncName(tsId, PressFuncIoTblModel);
         if (FPName != PressFuncIoTblModel)
         {
            countV.push_back(count);
            continue;
         } 
         count++;
      }

      for (auto i = 0; i < countV.size();i++) {
         m_model.removeRecordFromTable("PressureFuncIoTbl", countV[i]);
      }
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Info> All models are referred in the PressureFuncIoTbl; model being referred in RunOptionsIoTbl is retained and others deleted";
  } 
   std::string FractureType;
   m_model.fracturePressureManager().getFracturePressureType(FractureType);
   if(FractureType=="None"){
      m_model.fracturePressureManager().setFractureConstraintMethod(-9999);
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Info> FractureType is found to be None, Resetting FractureConstraintMethod to -9999";
   }
   auto FracPressId = m_model.fracturePressureManager().getpressFuncTblID();
   for (auto tsId : FracPressId) {
      m_model.fracturePressureManager().getPresFuncName(tsId, PressFuncIoTblModel);
      if (PressFuncIoTblModel != "None" && PressFuncIoTblModel != "C-NorthSea" && PressFuncIoTblModel != "Nigeria Deep" && PressFuncIoTblModel != "Terschelling" && PressFuncIoTblModel != "Nigeria East" && PressFuncIoTblModel != "Nigeria West" && PressFuncIoTblModel != "Brunei Deep" && PressFuncIoTblModel != "Linear 70%" && PressFuncIoTblModel != "Linear 75%" && PressFuncIoTblModel != "Linear 80%" && PressFuncIoTblModel != "Linear 85%" && PressFuncIoTblModel != "Linear 90%" && PressFuncIoTblModel != "Linear 95%")
      {
         m_model.fracturePressureManager().setUserDefinedFlag(tsId, 1);
         LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Info> UserDefined fracture pressure model is found, updating the UserDefined flag to 1";
      }
   }
//TODO: "Selected" field of PressureFuncIoTbl needs to be removed as a part of the project3D cleaning work as it is a redundant field having value always equal to 1. 
}
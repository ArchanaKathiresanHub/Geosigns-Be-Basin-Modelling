//
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by CGI.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "BasicCrustThinningUpgradeManager.h"

//std
#include <vector>

//utilities
#include "LogHandler.h"

//Prograde
#include "BasicCrustThinningModelConverter.h"

//cmbAPI
#include "cmbAPI.h"
#include "BottomBoundaryManager.h"

//DataAccess
#include "Interface/ProjectHandle.h"
#include "Interface/RunParameters.h"

using namespace mbapi;

//------------------------------------------------------------//

Prograde::BasicCrustThinningUpgradeManager::BasicCrustThinningUpgradeManager(mbapi::Model& model) :
   IUpgradeManager("basic crustal thinning model upgrade manager"), m_model(model)
{
   const auto ph = m_model.projectHandle();
   if (ph == nullptr) {
      throw std::invalid_argument(getName() + " cannot retreive the project handle from Cauldron data model");
   }
   m_ph = ph;
}

//------------------------------------------------------------//
void Prograde::BasicCrustThinningUpgradeManager::upgrade() {
   Prograde::BasicCrustThinningModelConverter modelConverter;
  
   BottomBoundaryManager::BottomBoundaryModel botBoundModel;

   m_model.bottomBoundaryManager().getBottomBoundaryModel(botBoundModel);

   if(botBoundModel== mbapi::BottomBoundaryManager::BottomBoundaryModel::BasicCrustThinning)
   {
      //upgrading the bottom boundary model
      m_model.bottomBoundaryManager().setBottomBoundaryModel(modelConverter.upgradeBotBoundModel(botBoundModel));
      
      //upgrading the crust property model to standard conductivity crust
      BottomBoundaryManager::CrustPropertyModel crstPropModel;
      m_model.bottomBoundaryManager().getCrustPropertyModel(crstPropModel);
      m_model.bottomBoundaryManager().setCrustPropertyModel(modelConverter.upgradeCrustPropModel(crstPropModel));

      //upgrading the mantle property model to high conductivity mantle model
      BottomBoundaryManager::MantlePropertyModel mntlPropModel;
      m_model.bottomBoundaryManager().getMantlePropertyModel(mntlPropModel);
      m_model.bottomBoundaryManager().setMantlePropertyModel(modelConverter.upgradeMantlePropModel(mntlPropModel));

      //setting initial lithospheric mantle thickness in BasementIoTbl, to default value, 115000
      double InitLithMnThickness = 115000.0;
      m_model.bottomBoundaryManager().setInitialLithoMantleThicknessValue(InitLithMnThickness);
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Basic crust thinning model is detected";
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Initial lithospheric mantle thickness value is set to the default value of 115000";

      cleanContCrustIoTbl();

      auto timesteps = m_model.bottomBoundaryManager().getTimeStepsID();
      for (auto tsId : timesteps) 
      {
         double age, thickness;
         std::string thicknessGrid, TableName;

         m_model.addRowToTable("ContCrustalThicknessIoTbl");
         m_model.addRowToTable("OceaCrustalThicknessIoTbl");
         //copying Age,thickness and thickness maps corresponding to a particular time step ID from CrustIoTbl to ContCrustalThicknessIoTbl
         m_model.bottomBoundaryManager().getAge(tsId, age);
         m_model.bottomBoundaryManager().setContCrustAge(tsId, age);

         m_model.bottomBoundaryManager().getThickness(tsId, thickness);
         m_model.bottomBoundaryManager().setContCrustThickness(tsId, thickness);

         m_model.bottomBoundaryManager().getCrustThicknessGrid(tsId, thicknessGrid);
         m_model.bottomBoundaryManager().setContCrustThicknessGrid(tsId, thicknessGrid);

         //setting thickness of OceaCrustalThicknessIoTbl to 0 for all time step ID
         m_model.bottomBoundaryManager().setOceaCrustAge(tsId, age);
         m_model.bottomBoundaryManager().setOceaCrustThickness(tsId, 0.0);

      }
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "CrustIoTbl table is detected ";
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "CrustIoTbl is copied to the ContCrustalThicknessIoTbl";
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "OceaCrustalThicknessIoTbl is set";

      cleanCrustIoTbl();
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "CrustIoTbl is cleaned";

      timesteps = m_model.bottomBoundaryManager().getGridMapTimeStepsID();
      for (auto tsId : timesteps)
      {
         std::string TableName;
         m_model.bottomBoundaryManager().getReferredBy(tsId, TableName);
         
         m_model.bottomBoundaryManager().setReferredBy(tsId, modelConverter.upgradeGridMapTable(TableName));

      }
   }
   else {
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Deprecated basic crust thinning model is not found";
      LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "no upgrade needed";
   }

   
}

//------------------------------------------------------------//

void Prograde::BasicCrustThinningUpgradeManager::cleanCrustIoTbl() const {
   m_model.clearTable("CrustIoTbl");
}
//------------------------------------------------------------//
void Prograde::BasicCrustThinningUpgradeManager::cleanContCrustIoTbl() const {
   m_model.clearTable("ContCrustalThicknessIoTbl");
}
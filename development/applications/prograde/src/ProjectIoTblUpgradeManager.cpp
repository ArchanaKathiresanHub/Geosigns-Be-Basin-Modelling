//
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by CGI.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "ProjectIoTblUpgradeManager.h"
#include "ProjectIoModelConverter.h"

//utilities
#include "LogHandler.h"

//cmbAPI
#include "cmbAPI.h"
#include "ProjectDataManager.h"

//DataAccess
#include "ProjectHandle.h"
//#include "RunParameters.h"

using namespace mbapi;

//------------------------------------------------------------//

Prograde::ProjectIoTblUpgradeManager::ProjectIoTblUpgradeManager(mbapi::Model& model) :
   IUpgradeManager("projectIoTbl upgrade manager"), m_model(model)
{
   const auto ph = m_model.projectHandle();
   if (ph == nullptr) {
      throw std::invalid_argument(getName() + " cannot retreive the project handle from Cauldron data model");
   }
   m_ph = ph;
}

//------------------------------------------------------------//
void Prograde::ProjectIoTblUpgradeManager::upgrade() {
   Prograde::ProjectIoModelConverter modelConverter;

   std::string modellingMode, description;
   double deltaX, deltaY;
   int xnodes, ynodes, windowXMax,windowYMax, NewWindowXMax, NewWindowYMax;

   m_model.projectDataManager().getModellingMode(modellingMode);
   m_model.projectDataManager().setModellingMode(modelConverter.upgradeModellingMode(modellingMode));
   
   //upgradation of node count to the default values for out of range values, if any
   m_model.projectDataManager().getNumberOfNodesX(xnodes);
   m_model.projectDataManager().getSimulationWindowMax(windowXMax, windowYMax);
   NewWindowXMax = windowXMax;
   NewWindowYMax = windowYMax; 
   m_model.projectDataManager().setNumberOfNodesX(modelConverter.upgradeNodeX(modellingMode, xnodes, windowXMax, NewWindowXMax));

   m_model.projectDataManager().getNumberOfNodesY(ynodes);
   m_model.projectDataManager().setNumberOfNodesY(modelConverter.upgradeNodeY(modellingMode, ynodes, windowYMax, NewWindowYMax));
   m_model.projectDataManager().setSimulationWindowMax(NewWindowXMax, NewWindowYMax);

   m_model.projectDataManager().getDeltaX(deltaX);
   m_model.projectDataManager().setDeltaX(modelConverter.upgradeDeltaX(modellingMode, deltaX));

   m_model.projectDataManager().getDeltaY(deltaY);
   m_model.projectDataManager().setDeltaY(modelConverter.upgradeDeltaY(modellingMode, deltaY));

   m_model.projectDataManager().getProjectDescription(description);
   m_model.projectDataManager().setProjectDescription(modelConverter.upgradeDescription(modellingMode, description));
}


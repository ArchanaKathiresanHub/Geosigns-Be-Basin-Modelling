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
   double xcoord_center, ycoord_center, deltaX, deltaY;
   int xnodes, ynodes;

   m_model.projectDataManager().getModellingMode(modellingMode);
   m_model.projectDataManager().setModellingMode(modelConverter.upgradeModellingModeFor1D(modellingMode));

   //upgradation of AOI origin coordinates for 3d scenarios
   m_model.projectDataManager().getOriginXCoord(xcoord_center);
   m_model.projectDataManager().getDeltaX(deltaX);
   m_model.projectDataManager().setOriginXCoord(modelConverter.upgradeProjectOriginX(modellingMode, xcoord_center, deltaX));

   m_model.projectDataManager().getOriginYCoord(ycoord_center);
   m_model.projectDataManager().getDeltaY(deltaY);
   m_model.projectDataManager().setOriginYCoord(modelConverter.upgradeProjectOriginY(modellingMode, ycoord_center, deltaY));
   
   //upgradation of node count to the default values for out of range values, if any
   m_model.projectDataManager().getNumberOfNodesX(xnodes);
   m_model.projectDataManager().setNumberOfNodesX(modelConverter.upgradeNodeX(modellingMode, xnodes));

   m_model.projectDataManager().getNumberOfNodesY(ynodes);
   m_model.projectDataManager().setNumberOfNodesY(modelConverter.upgradeNodeY(modellingMode, ynodes));

   m_model.projectDataManager().getDeltaX(deltaX);
   m_model.projectDataManager().setDeltaX(modelConverter.upgradeDeltaX(modellingMode, deltaX));

   m_model.projectDataManager().getDeltaY(deltaY);
   m_model.projectDataManager().setDeltaY(modelConverter.upgradeDeltaY(modellingMode, deltaY));

   m_model.projectDataManager().getProjectDescription(description);
   m_model.projectDataManager().setProjectDescription(modelConverter.upgradeDescription(description));
}


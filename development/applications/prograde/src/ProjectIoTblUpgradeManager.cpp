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
      throw std::invalid_argument(getName() + " cannot retrieve the project handle from Cauldron data model");
   }
   m_ph = ph;
}

//------------------------------------------------------------//
void Prograde::ProjectIoTblUpgradeManager::upgrade() {
   Prograde::ProjectIoModelConverter modelConverter;

   std::string description, originalModellingMode;
   double deltaX, deltaY;
   int xnodes, ynodes, legacyWindowXMin, legacyWindowXMax, legacyWindowYMin, legacyWindowYMax, stepX, stepY;

   //upgradation of node count to the default values for out of range values, if any
   m_model.projectDataManager().getNumberOfNodesX(xnodes);
   m_model.projectDataManager().getNumberOfNodesY(ynodes);

   originalModellingMode = (xnodes == 2 && ynodes == 2) ? "1d" : "3d";

   // Upgrading the simulation window if the legacy inputs doesn't satisfy BPA2 GUI validations
   m_model.projectDataManager().getSimulationWindowDetails(legacyWindowXMin, legacyWindowXMax, stepX, legacyWindowYMin, legacyWindowYMax, stepY);
   
   modelConverter.preProcessSimulationWindow(legacyWindowXMin, legacyWindowXMax, xnodes);
   modelConverter.preProcessSimulationWindow(legacyWindowYMin, legacyWindowYMax, ynodes);
   
   //Upgrade the simulation window if needed
   modelConverter.upgradeSimulationWindow(originalModellingMode, legacyWindowXMin, legacyWindowXMax, xnodes, stepX);
   m_model.projectDataManager().setSimulationWindowX(legacyWindowXMin, legacyWindowXMax, stepX);

   modelConverter.upgradeSimulationWindow(originalModellingMode, legacyWindowYMin, legacyWindowYMax, ynodes, stepY);
   m_model.projectDataManager().setSimulationWindowY(legacyWindowYMin, legacyWindowYMax, stepY);

	 if ((xnodes < 2) or (ynodes < 2))
	 {
		 LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Warning> Legacy 1d scenario with [" << xnodes << "x" << ynodes << "] nodes; not having the default value for 1d mode. These scenarios will fail in import";
	 }

   m_model.projectDataManager().getDeltaX(deltaX);
   m_model.projectDataManager().setDeltaX(modelConverter.upgradeDeltaX(originalModellingMode, deltaX, xnodes));

   m_model.projectDataManager().getDeltaY(deltaY);
   m_model.projectDataManager().setDeltaY(modelConverter.upgradeDeltaY(originalModellingMode, deltaY, ynodes));

   m_model.projectDataManager().getProjectDescription(description);
   m_model.projectDataManager().setProjectDescription(modelConverter.upgradeDescription(originalModellingMode, description));
}


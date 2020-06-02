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

   std::string description, originalModellingMode;
   double deltaX, deltaY;
   int xnodes, ynodes;

   m_model.projectDataManager().getModellingMode(originalModellingMode);
   m_model.projectDataManager().setModellingMode(modelConverter.upgradeModellingMode(originalModellingMode));
   
   //upgradation of node count to the default values for out of range values, if any
   m_model.projectDataManager().getNumberOfNodesX(xnodes);
   m_model.projectDataManager().getNumberOfNodesY(ynodes);

   /*Note: As per the earlier mapping sheet, depending on the legacy modellingMode, Prograde has to upgrade the nodeX/nodeY (and hence simulation window area), if the legacy input is not valid as per BPA2 standard.
   The old strategy is now changed and @Matthijs has confirmed that no upgrade of the number of nodes will needed to be done inPrograde even though it is outside the valid ranges of BPA2...
   This type of scenarios will automatically get failed in import if the validations are not met. (for setails refer to the Work item #379379)*/
   /*m_model.projectDataManager().getSimulationWindowMax(windowXMax, windowYMax);
   NewWindowXMax = windowXMax;
   NewWindowYMax = windowYMax; 
   m_model.projectDataManager().setNumberOfNodesX(modelConverter.upgradeNodeX(originalModellingMode, xnodes, windowXMax, NewWindowXMax));
   m_model.projectDataManager().setNumberOfNodesY(modelConverter.upgradeNodeY(modellingMode, ynodes, windowYMax, NewWindowYMax));
   m_model.projectDataManager().setSimulationWindowMax(NewWindowXMax, NewWindowYMax);*/

   //If any invalid case is encountered then Prograde log is updated just to have this extra information.
   if (originalModellingMode == "3d" and (xnodes < 3 or ynodes < 3))
	   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Warning> Legacy 3d scenario with [" << xnodes << "x" << ynodes <<"] nodes doesn't fall within the acceptable limit of BPA2";
   else if (originalModellingMode == "1d")
   {
	   if ((xnodes > 2) or (ynodes > 2))
		   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Warning> Legacy 1d scenario with [" << xnodes << "x" << ynodes << "] nodes; not having the default value for 1d mode. These scenarios will become actual 3d in BPA2";
	   else if ((xnodes < 2) or (ynodes < 2))
		   LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Warning> Legacy 1d scenario with [" << xnodes << "x" << ynodes << "] nodes; not having the default value for 1d mode. These scenarios will fail in import";
   }
	   

   m_model.projectDataManager().getDeltaX(deltaX);
   m_model.projectDataManager().setDeltaX(modelConverter.upgradeDeltaX(originalModellingMode, deltaX, xnodes));

   m_model.projectDataManager().getDeltaY(deltaY);
   m_model.projectDataManager().setDeltaY(modelConverter.upgradeDeltaY(originalModellingMode, deltaY, ynodes));

   m_model.projectDataManager().getProjectDescription(description);
   m_model.projectDataManager().setProjectDescription(modelConverter.upgradeDescription(originalModellingMode, description));
}


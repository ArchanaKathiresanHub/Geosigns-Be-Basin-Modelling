//
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by CGI
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "GridMapIoTblUpgradeManager.h"

//utilities
#include "LogHandler.h"

//cmbAPI
#include "cmbAPI.h"

using namespace mbapi;

//------------------------------------------------------------//

Prograde::GridMapIoTblUpgradeManager::GridMapIoTblUpgradeManager(Model& model) :
	IUpgradeManager("gridmapiotbl upgrade manager"), m_model(model)
{
	const auto ph = m_model.projectHandle();
	if (ph == nullptr) {
		throw std::invalid_argument(getName() + " cannot retreive the project handle from Cauldron data model");
	}
	m_ph = ph;
}

//------------------------------------------------------------//

void Prograde::GridMapIoTblUpgradeManager::upgrade()
{
	LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Updating GridMapIoTbl";
	LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "(Removing the references from GridMapIoTbl for the GridMaps which are removed from iotables in Project3d file during prograde)";
	database::Table * gridMapIo_Table = m_ph->getTable("GridMapIoTbl");
	
	/**Removing the unnecessary maps from the GridMapIoTbl
	** There are two situations in which need to clear the references from the GridMapIoTbl -->
	** Case 1. When the table is cleared that is after prograde it is empty so all the maps that are referenced by that table need to be removed from GridMapIoTbl
	** Case 2. When the grid map from a specific table is no longer required and the entry in the column or the complete row has been removed
	*/
	///Running through the list of the maps to be cleared from GridMapIoTbl
	for (auto itr = Prograde::GridMapIoTblUpgradeManager::tblNameMapName.begin(); itr != Prograde::GridMapIoTblUpgradeManager::tblNameMapName.end(); itr++)
	{
		size_t size_gridMapIoTbl = gridMapIo_Table->size();
		///Running through GridMapIoTbl
		for (size_t id = 0; id < size_gridMapIoTbl; ++id)
		{
			database::Record * rec = gridMapIo_Table->getRecord(static_cast<int> (id));
			std::string tblName = rec->getValue<std::string>("ReferredBy");
			std::string mapName = rec->getValue<std::string>("MapName");
			// Case 1. When a table is cleared
			if ((itr->first == tblName) && (itr->second == "All"))
			{
				m_model.removeRecordFromTable("GridMapIoTbl", id--);
				LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Record with the MapName " << mapName << " and ReferredBy " << tblName << " is removed as the table is cleared";
				size_gridMapIoTbl = gridMapIo_Table->size();
			}
			// Case 2. When a grid map from a specific table is removed
			else if ((itr->first == tblName) && (itr->second == mapName))
			{
				 m_model.removeRecordFromTable("GridMapIoTbl", id);
				 LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Record with the MapName " << mapName << " and ReferredBy " << tblName << " is removed";
				 break;
			}
		}
	}
}

///Defining the static variable tblNameMapName
std::vector<std::pair<std::string, std::string>> Prograde::GridMapIoTblUpgradeManager::tblNameMapName;

///Defining the static method clearMapNameTblNamepReferenceGridMap
void Prograde::GridMapIoTblUpgradeManager::clearTblNameMapNamepReferenceGridMap(const std::string& tblName, const std::string& mapName)
{
	Prograde::GridMapIoTblUpgradeManager::tblNameMapName.push_back(std::pair<std::string, std::string>(tblName, mapName));
}
///Defining the overload of the  static method clearMapNameTblNamepReferenceGridMap
void Prograde::GridMapIoTblUpgradeManager::clearTblNameMapNamepReferenceGridMap(const std::string & tblName)
{
	Prograde::GridMapIoTblUpgradeManager::tblNameMapName.push_back(std::pair<std::string, std::string>(tblName,"All"));
}
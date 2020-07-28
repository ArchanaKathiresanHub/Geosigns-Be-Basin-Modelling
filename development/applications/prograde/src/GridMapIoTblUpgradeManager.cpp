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
	IUpgradeManager("GridMapIoTbl upgrade manager"), m_model(model)
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
	LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Info> Updating GridMapIoTbl - Removing the references from GridMapIoTbl for the GridMaps which are removed from iotables in Project3d file during prograde";
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
		for (int id = 0; id < size_gridMapIoTbl; ++id)
		{
			database::Record * rec = gridMapIo_Table->getRecord(static_cast<int> (id));
			std::string tblName = rec->getValue<std::string>("ReferredBy");
			std::string mapName = rec->getValue<std::string>("MapName");
			// Case 1. When a table is cleared
			if ((itr->first == tblName) && (itr->second == "All"))
			{
				m_model.removeRecordFromTable("GridMapIoTbl", id--);
				LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Info> Record with the MapName " << mapName << " and ReferredBy " << tblName << " is removed as the table is cleared";
				size_gridMapIoTbl = gridMapIo_Table->size();
			}
			// Case 2. When a grid map from a specific table is removed
			else if ((itr->first == tblName) && (itr->second == mapName))
			{
				m_model.removeRecordFromTable("GridMapIoTbl", id);
				LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Info> Record with the MapName " << mapName << " and ReferredBy " << tblName << " is removed";
				break;
			}
		}
	}

	/** The code below changes the map sequence of FLT maps
	* The MapSeqNbr of maps in each map file starts from 0
	* In case when we have maps imported from multiple HDF/FLT files then we have multiple maps in GridMapIoTbl starting from 0
	* and hence the MapSeqNbr is repeated which is perfectly fine as such maps belong to different files. However, maps with 
	* same MapSeqNbr though belonging to different files is not handled by scenario import which results in import failure
	* [TEMPORARY SOLUTION] Hence, in order to successfully import the scenarios to bpa2, for the time being, 
	* the MapSeqNbr of FLT maps are changed to have unique values by assigning them the values next to the largest value 
	* of MapSeqNbr in the GridMapIoTbl
	*/
	size_t maxMapSeqNbr = 0; // Max. value of map sequence number
	std::vector<size_t> idsFLT; // Vector containing the list of all the ids in GridMapIoTbl with FLT maps
	size_t size_gridMapIoTbl = gridMapIo_Table->size();
	// Finding the value of maxMapSeqNbr and idsFLT from the GridMapIoTbl
	for (int id = 0; id < size_gridMapIoTbl; ++id)
	{
		database::Record* rec = gridMapIo_Table->getRecord(static_cast<int> (id));
		size_t mapSeqNbr = rec->getValue<int>("MapSeqNbr");
		if (mapSeqNbr > maxMapSeqNbr) maxMapSeqNbr = mapSeqNbr;
		std::string mapType = rec->getValue<std::string>("MapType");
		if (mapType == "FLT") idsFLT.push_back(id);
	}
	// Updating the value of MapSeqNbr for FLT maps
	for (auto i : idsFLT)
	{
		database::Record* rec = gridMapIo_Table->getRecord(static_cast<int> (i));
		size_t mapSeqNbr = rec->getValue<int>("MapSeqNbr");
		std::string mapName = rec->getValue<std::string>("MapName");
		std::string referredBy = rec->getValue<std::string>("ReferredBy");
		rec->setValue<int>("MapSeqNbr", (int) ++maxMapSeqNbr);
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Info> MapSeqNbr for the map : " << mapName << " ReferredBy : " << referredBy << " is changed from "<< mapSeqNbr <<" to "<< maxMapSeqNbr;
	}	
}

///Defining the static variable tblNameMapName
std::vector<std::pair<std::string, std::string>> Prograde::GridMapIoTblUpgradeManager::tblNameMapName;

///Defining the static method clearMapNameTblNamepReferenceGridMap
void Prograde::GridMapIoTblUpgradeManager::clearTblNameMapNamepReferenceGridMap(const std::string& tblName, const std::string& mapName)
{
	Prograde::GridMapIoTblUpgradeManager::tblNameMapName.push_back(std::pair<std::string, std::string>(tblName, mapName));
}
///defining the overload of the  static method clearMapNameTblNamepReferenceGridMap
void Prograde::GridMapIoTblUpgradeManager::clearTblNameMapNamepReferenceGridMap(const std::string & tblName)
{
	Prograde::GridMapIoTblUpgradeManager::tblNameMapName.push_back(std::pair<std::string, std::string>(tblName, "All"));
}

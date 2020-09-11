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
		throw std::invalid_argument(getName() + " cannot retrieve the project handle from Cauldron data model");
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


	/*The code below adds reference of DepthGrid of SurfaceDepthIoTbl at age = 0 Ma 
	* which has been updated in the TopBoundaryUpgradeManager. 
	* This needs to be done to make the legacy p3d file consistent with the BPA2 format 
	* as BPA1 p3d files doesn't have this input by default. 
	* Otherwise, imported legacy scenarios will not reflect the depthGrid input @0Ma 
	* in the TopBoundary condition page as import doesn't cater for map calculations.
	*/

	// Fetching the age and DepthGrid for the first row from SurfaceDepthIoTbl 
	database::Table* surfaceDepthIo_Tbl = m_ph->getTable("SurfaceDepthIoTbl");
	database::Record* rec = surfaceDepthIo_Tbl->getRecord(static_cast<int> (0));
	if (rec == nullptr)
		throw ErrorHandler::Exception(ErrorHandler::IoError) << "In the prject3d file, the SurfaceDepthIoTbl is empty";
	double age = rec->getValue<double>("Age");
	std::string depthMap = rec->getValue<std::string>("DepthGrid");
	// Updating the GridMapIoTbl if the first row contains the age = 0 and DepthMap is not empty
	if ((age == 0.0) && depthMap.compare(""))
	{
		std::string mapRefStratIoTbl;
		std::string mapType;
		std::string mapFileName;
		int mapSeqNbr = 0;
		size_t sizeGridMapIoTbl = gridMapIo_Table->size();
		/// Get MapSeqNbr, MapType, MapFileName for the DepthGrid present in SurfaceDepthIoTbl (referred by StratIoTbl) in GridMapIoTbl
		for (int it = 0; it < sizeGridMapIoTbl; ++it)
		{
			database::Record* record = gridMapIo_Table->getRecord(static_cast<int> (it));
			mapRefStratIoTbl = record->getValue<std::string>("MapName");
			if (!mapRefStratIoTbl.compare(depthMap))
			{
				mapSeqNbr = record->getValue<int>("MapSeqNbr");
				mapType = record->getValue<std::string>("MapType");
				mapFileName = record->getValue<std::string>("MapFileName");
				break;
			}
		}
		/// Adding another row and setting the corresponding values for the DepthGrid
		ErrorHandler::ReturnCode err = m_model.addRowToTable("GridMapIoTbl");
		if (err != ErrorHandler::ReturnCode::NoError)
			throw ErrorHandler::Exception(err) << "Can't add a new record to GridMapIoTbl";
		database::Record* record = gridMapIo_Table->getRecord(static_cast<int> (sizeGridMapIoTbl));
		record->setValue<std::string>("ReferredBy", "SurfaceDepthIoTbl");
		record->setValue<std::string>("MapName", depthMap);
		record->setValue<std::string>("MapType", mapType);
		record->setValue<std::string>("MapFileName", mapFileName);
		record->setValue<int>("MapSeqNbr", mapSeqNbr);
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Info> Reference of DepthGrid of SurfaceDepthIoTbl at age = 0 is added in GridMapIoTbl";
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

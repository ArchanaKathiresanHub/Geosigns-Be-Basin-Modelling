//
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by CGI.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "HeatFlowModeUpgradeManager.h"
//Prograde class to update the GridMapIoTbl if any GridMap is removed from any table
#include "GridMapIoTblUpgradeManager.h"
/**Static function named 'Prograde::GridMapIoTblUpgradeManager::clearTblNameMapNameReferenceGridMap()' is defined for the operation
* Overload 1: Prograde::GridMapIoTblUpgradeManager::clearTblNameMapNameReferenceGridMap("tableName"); //clears all the map references ReferredBy the table "tableName" from GridMapIoTbl
* Overload 2: Prograde::GridMapIoTblUpgradeManager::clearTblNameMapNameReferenceGridMap("tableName","mapName"); //clears the map reference of the "mapName" ReferredBy "tableName" from GridMapIoTbl
*/

//utilities
#include "LogHandler.h"
#include "NumericFunctions.h"

//cmbAPI
#include "cmbAPI.h"
#include "BottomBoundaryManager.h"
#include "MapsManager.h"
//DataAccess
#include "ProjectHandle.h"
#include "CrustFormation.h"
#include "Surface.h"
#include "Snapshot.h"
#include "Interface.h"

using namespace mbapi;

//------------------------------------------------------------//

Prograde::HeatFlowModeUpgradeManager::HeatFlowModeUpgradeManager(mbapi::Model& model) :
	BottomBoundaryModelUpgradeManager("Fixed heat flow model upgrade manager", model )
{
	
}

//------------------------------------------------------------//
void Prograde::HeatFlowModeUpgradeManager::upgrade() {
	BottomBoundaryManager::BottomBoundaryModel botBoundModel;

	m_model.bottomBoundaryManager().getBottomBoundaryModel(botBoundModel);
	if (botBoundModel == mbapi::BottomBoundaryManager::BottomBoundaryModel::BaseSedimentHeatFlow)
	{
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Updating the BasementIoTbl";
		this->upgradeBasementIoTbl();
		this->upgradeMantleHeatFlowIoTbl();
		bool tableCleared = this->cleartables("CrustIoTbl");
		if (tableCleared)
		{
			Prograde::GridMapIoTblUpgradeManager::clearTblNameMapNameReferenceGridMap("CrustIoTbl");
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Info> GridMaps ReferredBy CrustIoTbl (if any) in GridMapIoTbl will be cleared by GridMapIoTbl Upgrade Manager";
		}
		tableCleared = this->cleartables("ContCrustalThicknessIoTbl");
		if (tableCleared)
		{
			Prograde::GridMapIoTblUpgradeManager::clearTblNameMapNameReferenceGridMap("ContCrustalThicknessIoTbl");
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Info> GridMaps ReferredBy ContCrustalThicknessIoTbl (if any) in GridMapIoTbl will be cleared by GridMapIoTbl Upgrade Manager";
		}
		tableCleared = this->cleartables("BasaltThicknessIoTbl");
		if (tableCleared)
		{
			Prograde::GridMapIoTblUpgradeManager::clearTblNameMapNameReferenceGridMap("BasaltThicknessIoTbl");
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Info> GridMaps ReferredBy BasaltThicknessIoTbl (if any) in GridMapIoTbl will be cleared by GridMapIoTbl Upgrade Manager";
		}	
	}
}

void Prograde::HeatFlowModeUpgradeManager::upgradeBasementRelatedProperty(const std::string & tableName, const std::string & propName, double value)
{
	double legacyValue = m_model.tableValueAsDouble(tableName, 0, propName);
	m_model.setTableValue(tableName, 0, propName, value);
	LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Warning> " << propName<< " is upgraded from " << legacyValue << " to "<< value<<" m";
}

void Prograde::HeatFlowModeUpgradeManager::upgradeMantlePropertyModel(const std::string & bottomBoundaryModelName, const std::string & tableName, const std::string & propName)
{
	std::string legacyValue = m_model.tableValueAsString(tableName, 0, propName);
	std::string updatedmodel{};
	if (bottomBoundaryModelName == "Fixed HeatFlow") {
		updatedmodel = DataAccess::Interface::NullString;
		m_model.setTableValue(tableName, 0, propName, updatedmodel);
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Warning> " << propName << " is upgraded from '" << legacyValue << "' to null-string";
	}
	else if( (bottomBoundaryModelName == "Fixed Temperature" || bottomBoundaryModelName == "Advanced Lithosphere Calculator") ){
		if (legacyValue != "High Conductivity Mantle")
		{
			updatedmodel = "High Conductivity Mantle";
			m_model.setTableValue(tableName, 0, propName, updatedmodel);
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Warning> " << propName << " is upgraded from '" << legacyValue << "' to '" << updatedmodel<<"'";
		}
	}
	
}
void Prograde::HeatFlowModeUpgradeManager::upgradeCrustPropertyModel(const std::string & bottomBoundaryModelName, const std::string & tableName, const std::string & propName)
{
	std::string legacyValue = m_model.tableValueAsString(tableName, 0, propName);
	std::string updatedmodel{};
	if (bottomBoundaryModelName == "Fixed HeatFlow") {
		updatedmodel = DataAccess::Interface::NullString;
		m_model.setTableValue(tableName, 0, propName, updatedmodel);
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Warning> " << propName << " is upgraded from '" << legacyValue << "' to null-string";
	}
	else if ((bottomBoundaryModelName == "Fixed Temperature" || bottomBoundaryModelName == "Advanced Lithosphere Calculator")) 
	{
		if (legacyValue != "Standard Conductivity Crust")
		{
			updatedmodel = "Standard Conductivity Crust";
			m_model.setTableValue(tableName, 0, propName, updatedmodel);
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Warning> " << propName << " is upgraded from '" << legacyValue << "' to '" << updatedmodel<<"'";
		}
	}
}

/*void Prograde::HeatFlowModeUpgradeManager::updateTableValue(const std::string & tableName, const std::string & fieldName, const double bpa2DefaultValue)
{
	auto propertyValue = m_model.tableValueAsDouble(tableName, 0, fieldName);
	if (!NumericFunctions::isEqual(propertyValue, bpa2DefaultValue, 1e-6))
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Note: Legacy " << fieldName << " value:" << propertyValue << ", which is not equal to the default value of BPA2:" << bpa2DefaultValue;
}*/

void Prograde::HeatFlowModeUpgradeManager::upgradeBasementIoTbl()
{
	std::string tableName{ "BasementIoTbl" };

	this->CheckDefaultValues(tableName, "TopAsthenoTemp", 1333);
	this->CheckDefaultValues(tableName, "TopCrustHeatProd", 0);
	this->CheckDefaultValues(tableName, "CrustHeatPDecayConst", 10000);
	this->CheckDefaultValues(tableName, "FixedCrustThickness", 1);
	this->CheckDefaultValues(tableName, "LithoMantleThickness", 1);

	this->upgradeBasementRelatedProperty(tableName, "InitialLithosphericMantleThickness", 0.0);
	this->upgradeBasementRelatedProperty(tableName, "InitialLthMntThickns", 0.0);

	this->checkRangeForMaps(tableName,"TopCrustHeatProd", 0.0, 1000.0);

	std::string updatedModelName = "";
	std::string bottomBoundaryModel = m_model.tableValueAsString(tableName, 0, "BottomBoundaryModel");

	this->upgradeMantlePropertyModel(bottomBoundaryModel, tableName, "MantlePropertyModel");
	this->upgradeCrustPropertyModel(bottomBoundaryModel, tableName, "CrustPropertyModel");

}

void Prograde::HeatFlowModeUpgradeManager::upgradeMantleHeatFlowIoTbl()
{
	std::string tableName{ "MntlHeatFlowIoTbl" };
	std::string propertyName{ "HeatFlow" };

	LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Info> Updating the MntlHeatFlowIoTbl";
	auto BasinAge = m_ph->getCrustFormation()->getTopSurface()->getSnapshot()->getTime();
	LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "The basement Age is: " << BasinAge << "Ma";

	// This function below checks if the MntlHeatFlowIoTbl has only records greater than the basement age
	// In this case, the record at the basement age is obtained by constant interpolation from the first record
	Prograde::BottomBoundaryModelUpgradeManager::onlyAgesOlderThanBasinPresent(tableName, BasinAge);

	database::Table * mantleHeatFlowHistory_Table = m_ph->getTable(tableName);

	size_t heatFlowHistoryTableSize = mantleHeatFlowHistory_Table->size();

	//Checking the original inputs for allowed ranges of the property
	double allowedMinV = 0.0, allowedMaxV = 1000.0;
	this->preProcessingInput(tableName, propertyName, allowedMinV, allowedMaxV);

	//Checking whether interpolation is needed or not
	double age1, age2;
	bool doInterpolate1= this->findInterpolatingAges(tableName, propertyName, BasinAge, age1, age2);
	if (doInterpolate1)
	{
		DataAccess::Interface::GridMap* gridMap = this->generateInterpolatedMapAtAge("Fixed HeatFlow", doInterpolate1, BasinAge, age1, age2);
		//Saves the map in the Inputs.hdf, create the map references in the GridMapIoTbl and the respective bottom boundary table. In the bottomboundaryTable, the new map is appended at the end of the table  
		this->saveInterpolatedMap(gridMap, tableName, heatFlowHistoryTableSize, propertyName, BasinAge, allowedMinV, allowedMaxV);
	}	
	std::vector<std::pair<std::string, std::string>> removedRecords = this->removeRecordsOlderThanBasinAge(tableName, BasinAge, propertyName);
	for (int i = 0; i < removedRecords.size(); i++)
	{
		Prograde::GridMapIoTblUpgradeManager::clearTblNameMapNameReferenceGridMap(removedRecords[i].first, removedRecords[i].second);
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "* <Basin-Info> GridMap " << removedRecords[i].second << " ReferredBy " << removedRecords[i].first <<" will be cleared by GridMapIoTbl Upgrade Manager";
	}
}
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
		this->removeRecordsFromGridMapIoTbl(tableCleared, "CrustIoTbl");
		tableCleared = this->cleartables("ContCrustalThicknessIoTbl");
		this->removeRecordsFromGridMapIoTbl(tableCleared, "ContCrustalThicknessIoTbl");
		tableCleared = this->cleartables("BasaltThicknessIoTbl");
		this->removeRecordsFromGridMapIoTbl(tableCleared, "BasaltThicknessIoTbl");
	}

}

void Prograde::HeatFlowModeUpgradeManager::upgradeBasementRelatedProperty(const std::string & tableName, const std::string & propName, double value)
{
	double legacyValue = m_model.tableValueAsDouble(tableName, 0, propName);
	m_model.setTableValue(tableName, 0, propName, value);
	LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << propName<< " is upgraded from " << legacyValue << " to "<< value<<" m";
}

void Prograde::HeatFlowModeUpgradeManager::upgradeMantlePropertyModel(const std::string & bottomBoundaryModelName, const std::string & tableName, const std::string & propName)
{
	std::string legacyValue = m_model.tableValueAsString(tableName, 0, propName);
	std::string updatedmodel{};
	if (bottomBoundaryModelName == "Fixed HeatFlow") {
		updatedmodel = DataAccess::Interface::NullString;
		m_model.setTableValue(tableName, 0, propName, updatedmodel);
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << propName << " is upgraded from '" << legacyValue << "' to null-string";
	}
	else if( (bottomBoundaryModelName == "Fixed Temperature" || bottomBoundaryModelName == "Advanced Lithosphere Calculator") ){
		if (legacyValue != "High Conductivity Mantle")
		{
			updatedmodel = "High Conductivity Mantle";
			m_model.setTableValue(tableName, 0, propName, updatedmodel);
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << propName << " is upgraded from '" << legacyValue << "' to '" << updatedmodel<<"'";
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
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << propName << " is upgraded from '" << legacyValue << "' to null-string";
	}
	else if ((bottomBoundaryModelName == "Fixed Temperature" || bottomBoundaryModelName == "Advanced Lithosphere Calculator")) 
	{
		if (legacyValue != "Standard Conductivity Crust")
		{
			updatedmodel = "Standard Conductivity Crust";
			m_model.setTableValue(tableName, 0, propName, updatedmodel);
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << propName << " is upgraded from '" << legacyValue << "' to '" << updatedmodel<<"'";
		}
		else
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Latest model is found for "<< propName<<", no update is needed";
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

	LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Updating the MntlHeatFlowIoTbl";
	auto BasinAge = m_ph->getCrustFormation()->getTopSurface()->getSnapshot()->getTime();
	LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "The basement Age is: " << BasinAge << "Ma";

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
		this->saveInterpolatedMap(gridMap, tableName, heatFlowHistoryTableSize, propertyName, BasinAge);
	}	
	this->removeRecordsOlderThanBasinAge(tableName, BasinAge);
}







//
// Copyright (C) 2018-2020 Shell International Exploration & Production.
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
#include <algorithm> // for copy

//utilities
#include "LogHandler.h"
#include "NumericFunctions.h"
//Prograde
#include "BasicCrustThinningModelConverter.h"
//Prograde class to update the GridMapIoTbl if any GridMap is removed from any table
#include "GridMapIoTblUpgradeManager.h"
/**Static function named 'Prograde::GridMapIoTblUpgradeManager::clearTblNameMapNameReferenceGridMap()' is defined for the operation
* Overload 1: Prograde::GridMapIoTblUpgradeManager::clearTblNameMapNameReferenceGridMap("tableName"); //clears all the map references ReferredBy the table "tableName" from GridMapIoTbl
* Overload 2: Prograde::GridMapIoTblUpgradeManager::clearTblNameMapNameReferenceGridMap("tableName","mapName"); //clears the map reference of the "mapName" ReferredBy "tableName" from GridMapIoTbl
*/

//cmbAPI
#include "cmbAPI.h"
#include "BottomBoundaryManager.h"
#include "MapsManagerImpl.h"
#include "CtcManager.h"

//DataAccess
#include "ProjectHandle.h"
#include "CrustFormation.h"
#include "Surface.h"
#include "Snapshot.h"
#include "ContinentalCrustHistoryGenerator.h"

#include "PaleoFormationProperty.h"

// TableIO library
#include "cauldronschemafuncs.h"
#include "database.h"
#include "cauldronschema.h"

using namespace mbapi;

//------------------------------------------------------------//

Prograde::BasicCrustThinningUpgradeManager::BasicCrustThinningUpgradeManager(mbapi::Model& model) :
	BottomBoundaryModelUpgradeManager("Basic crustal thinning model upgrade manager", model)
{
}

//------------------------------------------------------------//
void Prograde::BasicCrustThinningUpgradeManager::upgrade() {
	Prograde::BasicCrustThinningModelConverter modelConverter;

	BottomBoundaryManager::BottomBoundaryModel botBoundModel;

	m_model.bottomBoundaryManager().getBottomBoundaryModel(botBoundModel);

	if (botBoundModel == mbapi::BottomBoundaryManager::BottomBoundaryModel::BasicCrustThinning)
	{
		//upgrading the bottom boundary model
		m_model.bottomBoundaryManager().setBottomBoundaryModel(modelConverter.upgradeBotBoundModel(botBoundModel));
		(LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Info> Updating  BasementIoTbl");
		//upgrading the crust property model to standard conductivity crust
		BottomBoundaryManager::CrustPropertyModel crstPropModel;
		m_model.bottomBoundaryManager().getCrustPropertyModel(crstPropModel);
		m_model.bottomBoundaryManager().setCrustPropertyModel(modelConverter.upgradeCrustPropModel(crstPropModel));

		//upgrading the mantle property model to high conductivity mantle model
		BottomBoundaryManager::MantlePropertyModel mntlPropModel;
		m_model.bottomBoundaryManager().getMantlePropertyModel(mntlPropModel);
		m_model.bottomBoundaryManager().setMantlePropertyModel(modelConverter.upgradeMantlePropModel(mntlPropModel));
		mbapi::MapsManager& mapsMgrLocal = m_model.mapsManager();

		//setting initial lithospheric mantle thickness in BasementIoTbl, to default value, 115000
		double InitLithMnThickness_original = 0.0;
		double InitLithMnThickness = 115000.0;
		m_model.bottomBoundaryManager().getInitialLithoMantleThicknessValue(InitLithMnThickness_original);
		if (InitLithMnThickness_original != InitLithMnThickness)
		{
			m_model.bottomBoundaryManager().setInitialLithoMantleThicknessValue(InitLithMnThickness);
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Warning> Initial lithospheric mantle thickness value is changed from " << InitLithMnThickness_original << " to the default value of " << InitLithMnThickness;
		}

		//setting initial LithoMantleThickness,InitialLthMntThickns & FixedCrustThickness in BasementIoTbl, to default value 0.0
		double legayVal = m_model.tableValueAsDouble("BasementIoTbl", 0, "LithoMantleThickness");
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS)
			<< "setting LithoMantleThickness from, " << legayVal << ", to 0.0";
		auto err = m_model.setTableValue("BasementIoTbl", 0, "LithoMantleThickness", 0.0);

		legayVal = m_model.tableValueAsDouble("BasementIoTbl", 0, "InitialLthMntThickns");
		err = m_model.setTableValue("BasementIoTbl", 0, "InitialLthMntThickns", 0.0);
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS)
			<< "setting InitialLthMntThickns from, " << legayVal << ", to 0.0";

		legayVal = m_model.tableValueAsDouble("BasementIoTbl", 0, "FixedCrustThickness");
		err = m_model.setTableValue("BasementIoTbl", 0, "FixedCrustThickness", 0.0);
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS)
			<< "setting FixedCrustThickness from, " << legayVal << ", to 0.0";
		if (ErrorHandler::NoError != err)
			throw ErrorHandler::Exception(mapsMgrLocal.errorCode()) << mapsMgrLocal.errorMessage();

		// checking CrustHeatPDecayConst in BasementIoTbl for its default value of BPA2=10,000
		double CrustHeatPDecayConst = m_model.tableValueAsDouble("BasementIoTbl", 0, "CrustHeatPDecayConst");
		if (!NumericFunctions::isEqual(CrustHeatPDecayConst, 10000., 1e-4))
			(LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Info> CrustHeatPDecayConst-" << CrustHeatPDecayConst << " is not equal the default value of BPA2: 10000");

		// checking TopAsthenoTemp in BasementIoTbl for its default value of BPA2=1333 oC
		auto TopAsthenoTemp = m_model.tableValueAsDouble("BasementIoTbl", 0, "TopAsthenoTemp");
		if (!NumericFunctions::isEqual(TopAsthenoTemp, 1333.0, 1e-4))
			(LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Info> TopAsthenoTemp-" << TopAsthenoTemp << " is not equal the default value of BPA2: 1333 degrees centegrade ");
		/*Copy the value as is if the value lies within[0, 1000].Else reset its value to the nearest limiting value.If map is defined
		then copy the map name as is but put a check if the map is having out - of - range value or not.If the map
		contains out - of - range values then update the prograde log file.*/

		/* check for the acceptable limits of[0, 1000] for TopCrustHeatProd values*/
		auto TopCrustHeatProd = m_model.tableValueAsDouble("BasementIoTbl", 0, "TopCrustHeatProd");
		if (!NumericFunctions::inRange(TopCrustHeatProd, 0.0, 1000.0)) {
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Warning> TopCrustHeatProd-" << TopCrustHeatProd << " out of range [0, 1000], resetting to nearest limit! ";
			TopCrustHeatProd = NumericFunctions::clipValueToRange(TopCrustHeatProd, 0.0, 1000.0);
			m_model.setTableValue("BasementIoTbl", 0, "TopCrustHeatProd", TopCrustHeatProd);
		}

		auto TopCrustHeatProdGrid = m_model.tableValueAsString("BasementIoTbl", 0, "TopCrustHeatProdGrid");


		if (TopCrustHeatProdGrid.compare(DataAccess::Interface::NullString)) {
			auto mi = mapsMgrLocal.findID(TopCrustHeatProdGrid);
			double oldMin;
			double oldMax;
			if (ErrorHandler::NoError != mapsMgrLocal.mapValuesRange(mi, oldMin, oldMax))
				throw ErrorHandler::Exception(mapsMgrLocal.errorCode()) << mapsMgrLocal.errorMessage();
			if (!(NumericFunctions::inRange(oldMin, 0.0, 1000.0) && NumericFunctions::inRange(oldMax, 0.0, 1000.0)))
				LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) <<"<Basin-Info> TopCrustHeatProdGrid : " <<TopCrustHeatProdGrid + " map OUT of Range [0, 1000] ";


			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Warning> GridMap found in TopCrustHeatProdGrid, " <<
				"resetting the 'TopCrustHeatProd': " << TopCrustHeatProd << " value to -9999";
			m_model.setTableValue("BasementIoTbl", 0, "TopCrustHeatProd", DataAccess::Interface::DefaultUndefinedScalarValue);
		}


		cleanContCrustIoTbl();

		auto BasinAge = m_ph->getCrustFormation()->getTopSurface()->getSnapshot()->getTime();
		BasinAge = BasinAge > 999.0 ? 999.0 : BasinAge;
		/*
		\\ For OceaCrustalThicknessIoTbl
		1.  Must have to be specified for all the major system defined snapsot ages.
		This is needed because fastcauldron simulator does not interpolate OceaCrustal maps for major system defined snapshots
		from the given inputs, unlike with Cont. CrustalThickness maps
		2.  The Thickness is always 0.
		\\
		*/
		auto SnapshotList = m_ph->getSnapshots();
		std::vector<double> agesForOceaTbl(0);
		for (const auto snapshot : *SnapshotList)
		{
			auto SnpAge = snapshot->getTime();
			if ((SnpAge < BasinAge || abs(SnpAge - BasinAge) < 1e-6) && (snapshot->getKind() == "System Generated"))
			{
				agesForOceaTbl.push_back(SnpAge);
			}
		}
		delete SnapshotList;

		double age = 0.0;
		double thickness{};
		std::string thicknessGrid{};

		auto timesteps = m_model.bottomBoundaryManager().getTimeStepsID();

		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Info> Updating CrustIoTbl";
		std::string tableName{ "CrustIoTbl" };
		std::string propertyName{ "Thickness" };
		double minValueAllowedForCrustalThickness = 0.0;
		double maxValueAllowedForCrustalThickness = 6300000.0;
		database::Table * crustHistory_Table = m_ph->getTable(tableName);

		// This function below checks if the CrustIoTbl has only records greater than the basement age
		// In this case, the record at the basement age is obtained by constant interpolation from the first record
		Prograde::BottomBoundaryModelUpgradeManager::onlyAgesOlderThanBasinPresent(tableName, BasinAge);

		size_t crustHistoryTableSize = crustHistory_Table->size();
		this->preProcessingInput(tableName, propertyName, minValueAllowedForCrustalThickness, maxValueAllowedForCrustalThickness);
		//Checking whether interpolation is needed or not
		double age1, age2;
		bool doInterpolate1 = this->findInterpolatingAges(tableName, propertyName, BasinAge, age1, age2);
		if (doInterpolate1)
		{
			DataAccess::Interface::GridMap* gridMap = this->generateInterpolatedMapAtAge("Fixed Temperature", doInterpolate1, BasinAge, age1, age2);
			if (gridMap != nullptr)
				this->saveInterpolatedMap(gridMap, tableName, crustHistoryTableSize, propertyName, BasinAge, minValueAllowedForCrustalThickness, maxValueAllowedForCrustalThickness);
			else
				throw ErrorHandler::Exception(ErrorHandler::ReturnCode::UnknownError) << "Could not save the interpolated maps";
		}

		auto updatedTimesteps = m_model.bottomBoundaryManager().getTimeStepsID();
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Info> Updating ContCrustalThicknessIoTbl";
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Warning> Updated CrustIoTbl is copied to the ContCrustalThicknessIoTbl";
		for (auto tsId : updatedTimesteps)
		{
			// copying Age,thickness and thickness maps of crustIoTbl to ContCrustalThicknessIoTbl
			m_model.bottomBoundaryManager().getAge(tsId, age);
			m_model.bottomBoundaryManager().getThickness(tsId, thickness);
			m_model.bottomBoundaryManager().getCrustThicknessGrid(tsId, thicknessGrid);

			m_model.addRowToTable("ContCrustalThicknessIoTbl");
			m_model.bottomBoundaryManager().setContCrustAge(tsId, age);
			m_model.bottomBoundaryManager().setContCrustThickness(tsId, thickness);
			m_model.bottomBoundaryManager().setContCrustThicknessGrid(tsId, thicknessGrid);

			// Checking if an age from ContCrustalThicknessIoTbl is missing from OceaCrustalThicknessIoTbl's list
			auto aCTage = std::find(agesForOceaTbl.begin(), agesForOceaTbl.end(), age);
			// if missing put in this age
			if (aCTage == std::end(agesForOceaTbl) && (age < BasinAge || abs(age - BasinAge) < 1e-6))
				agesForOceaTbl.push_back(age);
		}

		std::vector<std::pair<std::string, std::string>> removedRecords = this->removeRecordsOlderThanBasinAge("ContCrustalThicknessIoTbl", BasinAge, propertyName);

		/* Updating OceaCrustalThicknessIoTblwith all the major, system defined snapshot ages*/
		size_t index = 0;
		std::sort(agesForOceaTbl.begin(), agesForOceaTbl.end());

		(LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Info> Updating  Ocea. CrustalThickness");
		(LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Warning> Setting oceanic thickness to 0 m for all the system generated snapshot ages");
		for (const auto OceaAge : agesForOceaTbl) {
			m_model.addRowToTable("OceaCrustalThicknessIoTbl");
			//setting thickness of OceaCrustalThicknessIoTbl to 0.0 for scalers
			m_model.bottomBoundaryManager().setOceaCrustAge(index, OceaAge);
			m_model.bottomBoundaryManager().setOceaCrustThickness(index, 0.0);
			// "" for grids for all these time step ID is set automatically at initialization of the Record in addRowToTable
			++index;
		}

		cleanCrustIoTbl();
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Info> CrustIoTbl is cleared; GridMapIoTbl records for CrustIoTbl are updated to ContCrustalThicknessIoTbl as CrustIoTbl is copied to ContCrustalThicknessIoTbl";
		cleanBasaltThicknessIoTbl();
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Info> BasaltThicknessIoTbl is cleared; GridMaps ReferredBy BasaltThicknessIoTbl (if any) in GridMapIoTbl will be cleared by GridMapIoTbl Upgrade Manager";
		cleanMntlHeatFlowIoTbl();
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Info> MntlHeatFlowIoTbl is cleared; GridMaps ReferredBy MntlHeatFlowIoTbl (if any) in GridMapIoTbl will be cleared by GridMapIoTbl Upgrade Manager";

		timesteps = m_model.bottomBoundaryManager().getGridMapTimeStepsID();
		if (timesteps.size() != 0)
		{
			// cleaning ReferredBy field from GridMapIoTbl for all deleted entries in BasementIoTbl
			for (int tsId = 0; tsId < timesteps.size(); tsId++)
			{
				std::string TableName, mapName;
				m_model.bottomBoundaryManager().getReferredBy(tsId, TableName);
				m_model.ctcManager().getGridMapIoTblMapName(tsId, mapName);
				auto newName = modelConverter.upgradeGridMapTable(TableName);
				if (newName == "ContCrustalThicknessIoTbl")
				{
					m_model.bottomBoundaryManager().setReferredBy(tsId, newName);
				}
			}
			for (int i = 0; i < removedRecords.size(); i++)
			{
				Prograde::GridMapIoTblUpgradeManager::clearTblNameMapNameReferenceGridMap(removedRecords[i].first, removedRecords[i].second);
				LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Info> GridMapIoTbl: GridMap " << removedRecords[i].second << " ReferredBy " << removedRecords[i].first << " will be cleared by GridMapIoTbl Upgrade Manager";
			}
			Prograde::GridMapIoTblUpgradeManager::clearTblNameMapNameReferenceGridMap("BasaltThicknessIoTbl");
			Prograde::GridMapIoTblUpgradeManager::clearTblNameMapNameReferenceGridMap("MntlHeatFlowIoTbl");
		}
	}
	else
	{
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Info> Deprecated basic crust thinning model is not found, no upgrade needed";
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

void Prograde::BasicCrustThinningUpgradeManager::cleanBasaltThicknessIoTbl() const
{
	m_model.clearTable("BasaltThicknessIoTbl");
}

void Prograde::BasicCrustThinningUpgradeManager::cleanMntlHeatFlowIoTbl() const
{
	m_model.clearTable("MntlHeatFlowIoTbl");
}

DataAccess::Interface::GridMap * Prograde::BasicCrustThinningUpgradeManager::generateInterpolatedMapAtAge(std::string bottomBoundaryModel, bool needInterpolation, const double basinAge, double & interpolatingLowerAge, double & interpolatingHigherAge)
{
	DataAccess::Interface::GridMap* interpolatedMap = nullptr;
	LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Info> BasinAge is younger than the oldest property age. Genearating the property history at basinAge(" << basinAge << " Ma) by Linear Interpolation using the properties at " << interpolatingLowerAge << " Ma and " << interpolatingHigherAge << " Ma";
	m_ph->loadCrustThinningHistory();
	const DataAccess::Interface::PaleoFormationPropertyList* crustalMaps = m_ph->getCrustPaleoThicknessHistory();
	DataAccess::Interface::PaleoFormationPropertyList::const_iterator CrustalThicknessIter;

	// value 5000.0	is selected randomly based on the condition that ages never exceeds or equals to this value
	double age1 = 5000.0, age2 = 5000.0;
	const DataAccess::Interface::GridMap* crustalMap1, *crustalMap2, *crustalMap;

	bool foundInterpolatingLowerAge = false;
	bool foundInterpolatingHigherAge = false;

	for (CrustalThicknessIter = crustalMaps->begin(); CrustalThicknessIter != crustalMaps->end(); ++CrustalThicknessIter)
	{
		const DataAccess::Interface::PaleoFormationProperty* crustalHistoryInstance = dynamic_cast<const DataAccess::Interface::PaleoFormationProperty*>(*CrustalThicknessIter);
		const double age = crustalHistoryInstance->getSnapshot()->getTime();
		crustalMap = crustalHistoryInstance->getMap(DataAccess::Interface::CrustThinningHistoryInstanceThicknessMap);
		if (age == interpolatingLowerAge)
		{
			foundInterpolatingLowerAge = true;
			crustalMap1 = crustalMap;
			age1 = age;

		}
		if (age == interpolatingHigherAge)
		{
			foundInterpolatingHigherAge = true;
			crustalMap2 = crustalMap;
			age2 = age;

		}
		if (foundInterpolatingLowerAge && foundInterpolatingHigherAge)
		{
			database::Record record(*(*CrustalThicknessIter)->getRecord());
			database::setAge(&record, basinAge);

			DataAccess::Interface::PaleoFormationProperty* interpolatedCrustalThicknessMap = m_ph->getFactory()->producePaleoFormationProperty(*m_ph.get(), &record, m_ph->getCrustFormation());
			DataAccess::Interface::InterpolateFunctor functor(age1, age2, basinAge);
			interpolatedMap = interpolatedCrustalThicknessMap->computeMap(DataAccess::Interface::CrustThinningHistoryInstanceThicknessMap, crustalMap1, crustalMap2, functor);

			break;
		}
	}
	if (age1 == 5000.0) {
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Error> Crustal thickness map at age " << interpolatingLowerAge << " Ma is not found";
	}
	if (age2 == 5000.0) {
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Error> Crustal thickness map at age " << interpolatingHigherAge << " Ma is not found";
	}

	return interpolatedMap;
}

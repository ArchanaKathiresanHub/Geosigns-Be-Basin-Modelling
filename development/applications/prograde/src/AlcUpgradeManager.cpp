//
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "AlcUpgradeManager.h"
//utilities
#include "LogHandler.h"
#include "NumericFunctions.h"

//DataAccess
#include "GridMap.h"
#include "CrustFormation.h"
#include "MantleFormation.h"
#include "ProjectHandle.h"
#include "ContinentalCrustHistoryGenerator.h"
#include "PaleoFormationProperty.h"
#include "Snapshot.h"
#include "Interface.h"

//Prograde
#include "LegacyBasaltThicknessCalculator.h"
#include "AlcModelConverter.h"
//Prograde class to update the GridMapIoTbl if any GridMap is removed from any table
#include "GridMapIoTblUpgradeManager.h"
/**Static function named 'Prograde::GridMapIoTblUpgradeManager::clearTblNameMapNameReferenceGridMap()' is defined for the operation
* Overload 1: Prograde::GridMapIoTblUpgradeManager::clearTblNameMapNameReferenceGridMap("tableName"); //clears all the map references ReferredBy the table "tableName" from GridMapIoTbl
* Overload 2: Prograde::GridMapIoTblUpgradeManager::clearTblNameMapNameReferenceGridMap("tableName","mapName"); //clears the map reference of the "mapName" ReferredBy "tableName" from GridMapIoTbl
*/

//cmbAPI
#include "MapsManager.h"
#include "cmbAPI.h"
#include "BottomBoundaryManager.h"

#include <algorithm>

using namespace mbapi;

typedef std::pair<const std::shared_ptr<DataModel::AbstractSnapshot const>,
	const std::shared_ptr<DataAccess::Interface::GridMap const>> SmartAbstractSnapshotSmartGridMapPair;

typedef std::map<const DataModel::AbstractSnapshot* const,
	const DataAccess::Interface::GridMap* const,
	DataModel::AbstractSnapshot::ComparePointers<const DataModel::AbstractSnapshot* const>> AbstractSnapshotVsGridMap;

typedef std::pair<const DataModel::AbstractSnapshot* const,
	const DataAccess::Interface::GridMap* const > AbstractSnapshotVsGridMapPair;

Prograde::AlcUpgradeManager::AlcUpgradeManager(mbapi::Model& model) :
	IUpgradeManager("ALC upgrade manager"), m_model(model), m_initialCrustalThickness(0)
{
	const auto ph = m_model.projectHandle();
	if (ph == nullptr) {
		throw std::invalid_argument(getName() + " cannot retrieve the project handle from Cauldron data model");
	}
	const auto crust = ph->getCrustFormation();
	if (crust == nullptr) {
		throw std::invalid_argument(getName() + " cannot retrieve the crust from project handle");
	}
	m_ph = ph;
	m_crust = crust;
}

//------------------------------------------------------------//

void Prograde::AlcUpgradeManager::upgrade()
{

	if (isLegacyAlc())
	{
		Prograde::AlcModelConverter modelConverter;

		// Get the DepoAge of basement layer
		database::Table * stratIo_Table = m_ph->getTable("StratIoTbl");
		size_t id = stratIo_Table->size() - 1;
		database::Record * rec = stratIo_Table->getRecord(static_cast<int>(id));
		double basementAge = rec->getValue<double>("DepoAge");

		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Warning> legacy ALC detected (v2016.11); Upgrading project to latest ALC";

		/////////////////////////////////////////////////////////////////////////////////// updating BasementIoTbl //////////////////////////////////////////////////////////////////////////////////////////////////////////

		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Info> Updating BasementIoTbl";

		database::Table * BasementIo_Table = m_ph->getTable("BasementIoTbl");
		database::Record * Rec = BasementIo_Table->getRecord(0);

		// Below code converts the BottomBoundaryModel name "Advanced Lithosphere Calculator" to "Improved Lithosphere Calculator Linear Element Mode"
		std::string BottomBoundaryModel = Rec->getValue<std::string>("BottomBoundaryModel");
		Rec->setValue<std::string>("BottomBoundaryModel", modelConverter.updateBottomBoundaryModel(BottomBoundaryModel));

		// log the value of TopAsthenoTemp if it is not the default value
		double TopAsthenoTemp = Rec->getValue<double>("TopAsthenoTemp");
		if (TopAsthenoTemp != 1333)
		{
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Info> The value of TopAsthenoTemp is : " << TopAsthenoTemp << " which is not equal to BPA2 default value of 1333";
		}

		// log the value of CrustHeatPDecayConst if it is not the default value
		double CrustHeatPDecayConst = Rec->getValue<double>("CrustHeatPDecayConst");
		if (CrustHeatPDecayConst != 10000)
		{
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Info> The value of CrustHeatPDecayConst is : " << CrustHeatPDecayConst << " which is not equal to BPA2 default value of 10000";
		}

		//checking and logging the values in TopCrustHeatProd and TopCrustHeatProdGrid to be in the acceptable limits[0,1000]
		std::string TopCrustHeatProdGrid = Rec->getValue<std::string>("TopCrustHeatProdGrid");
		double TopCrustHeatProd = Rec->getValue<double>("TopCrustHeatProd");
		if (!TopCrustHeatProdGrid.compare("")) //if TopCrustHeatProdGrid is empty; that is TopCrustHeatProd is populated
		{
			if (!NumericFunctions::inRange(TopCrustHeatProd, 0.0, 1000.0))
			{
				double updatedTopCrustHeatProd = NumericFunctions::clipValueToRange(TopCrustHeatProd, 0.0, 1000.0);
				Rec->setValue<double>("TopCrustHeatProd", updatedTopCrustHeatProd);
				LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Warning> The value of TopCrustHeatProd is changed from " << TopCrustHeatProd << " to " << updatedTopCrustHeatProd << " as the value is not in acceptable limits [0,1000]";
			}
		}
		else ///TopCrustHeatProdGrid is populated
		{
			///setting the scalar value in TopCrustHeatProd to -9999 if both grid as well as scalar is present; grid will be used
			if (TopCrustHeatProd != DataAccess::Interface::DefaultUndefinedScalarValue)
			{
				Rec->setValue<double>("TopCrustHeatProd", DataAccess::Interface::DefaultUndefinedScalarValue);
				LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Info> The value of TopCrustHeatProd is changed from " << TopCrustHeatProd << " to -9999 as both grid and scalar value were present; Grid is used";
			}

			//checking and logging the out of range maps
			double valInMap = 0;
			size_t mapId = m_model.mapsManager().findID(TopCrustHeatProdGrid);
			double minV = 0, maxV = 0;
			m_model.mapsManager().mapValuesRange(mapId, minV, maxV);
			if ((!NumericFunctions::inRange(minV, 0.0, 1000.0)) || (!NumericFunctions::inRange(maxV, 0.0, 1000.0)))
			{
				LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Info> The values in the grid TopCrustHeatProdGrid are not in the acceptable limits (0-1000)";
			}
		}

		//setting LithoMantleThickness, InitialLthMntThickns and FixedCrustThickness to 0; as ALC mode is detected
		double LithoMantleThickness = Rec->getValue<double>("LithoMantleThickness");
		double InitialLthMntThickns = Rec->getValue<double>("InitialLthMntThickns");
		double FixedCrustThickness = Rec->getValue<double>("FixedCrustThickness");
		Rec->setValue<double>("LithoMantleThickness", 0);
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Info> LithoMantleThickness is changed from " << LithoMantleThickness << " to 0";
		Rec->setValue<double>("InitialLthMntThickns", 0);
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Info> InitialLthMntThickns is changed from " << InitialLthMntThickns << " to 0";
		Rec->setValue<double>("FixedCrustThickness", 0);
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Info> FixedCrustThickness is changed from " << FixedCrustThickness << " to 0";

		double InitialLithosphericMantleThickness = Rec->getValue<double>("InitialLithosphericMantleThickness");
		if (!NumericFunctions::inRange(InitialLithosphericMantleThickness, 0.0, 6300000.0))
		{
			double updatedInitialLithosphericMantleThickness = NumericFunctions::clipValueToRange(InitialLithosphericMantleThickness, 0.0, 6300000.0);
			Rec->setValue<double>("InitialLithosphericMantleThickness", updatedInitialLithosphericMantleThickness);
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Warning> The value of InitialLithosphericMantleThickness is changed from " << InitialLithosphericMantleThickness << " to " << updatedInitialLithosphericMantleThickness << " as the value is not in acceptable limits [0,6300000.0]";
		}

		// BasementIoTbl updated

		/////////////////////////////////////////////////////////////////////////////////// updating ContCrustalThicknessIoTbl //////////////////////////////////////////////////////////////////////////////////////////////////////////

		///In ContCrustalThicknessIoTbl, if the last record's age is less than the DepoAge of basement, then add a new row with age = basement age copying data from the last record
		///This condition has been taken care separately because the ContCrustalThickness is required at the age >=basement age for calculations of OceanicCrustalThickness values
		database::Table * contCrustalThicknessIo_Table = m_ph->getTable("ContCrustalThicknessIoTbl");
		database::Record * record = contCrustalThicknessIo_Table->getRecord(static_cast<int>(contCrustalThicknessIo_Table->size() - 1));
		double age_lastRecord = record->getValue<double>("Age");
		if (age_lastRecord < basementAge)
		{
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Info> Updating ContCrustalThicknessIoTbl";
			double thickness_lastRecord = record->getValue<double>("Thickness");
			std::string thickGrid_lastRecord = record->getValue<std::string>("ThicknessGrid");
			m_model.addRowToTable("ContCrustalThicknessIoTbl");
			m_model.setTableValue("ContCrustalThicknessIoTbl", contCrustalThicknessIo_Table->size() - 1, "Age", basementAge);
			m_model.setTableValue("ContCrustalThicknessIoTbl", contCrustalThicknessIo_Table->size() - 1, "Thickness", thickness_lastRecord);
			m_model.setTableValue("ContCrustalThicknessIoTbl", contCrustalThicknessIo_Table->size() - 1, "ThicknessGrid", thickGrid_lastRecord);
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Info> BasinAge : " << basementAge << " is older than the oldest crust age : " << age_lastRecord << ", Generating the crustal history at BasinAge by constant extrapolation";
		}
		else
		{
			/// Condition to check if all the records present are of age older than the basement age,
			/// then in that case the first record is labelled as the record for the basement age that is constant interpolation
			record = contCrustalThicknessIo_Table->getRecord(0);
			double ageFirst = record->getValue<double>("Age");
			if (ageFirst > basementAge)
			{
				record->setValue<double>("Age", basementAge);
				LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Info> ContCrustalThicknessIoTbl contains records only for ages greater than the basement age : " << basementAge
					<< "; Hence, the record at the basement age is obtained by constant interpolation from the record at age : " << ageFirst;
				///  The age_lastRecord is updated for the case if the ContCrustalThicknessIoTbl had only one record that was greater than the basement age but now that record is updated as the
				///  record for the basement age, so new age_lastRecord becomes the basement age; Now we can avoid the next condition in line.
				record = contCrustalThicknessIo_Table->getRecord(static_cast<int>(contCrustalThicknessIo_Table->size() - 1));
				age_lastRecord = record->getValue<double>("Age");
			}
		}

		createCrustThickness(); // creates crustal thicknesses
		computeBasaltThickness(); // computes basalt thicknesses

		/// The condition below could not be merged with the previous condition because of calculations for the crustal thicknesses; The calculations need to be at this place only.
		/// we need to update the ContCrustalThicknessIoTbl only in case if age_lastRecord > basementAge as the case age_lastRecord < basementAge has already been taken care and in case if age_lastRecord = basementAge then we donot have to do anything
		if (age_lastRecord > basementAge)
		{
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Info> Updating ContCrustalThicknessIoTbl";
			// Deleting all the rows for which age > basement age
			std::vector<std::string> mapNames;
			int size_contCrustalThicknessIoTbl = contCrustalThicknessIo_Table->size();
			for (int id = 0; id < size_contCrustalThicknessIoTbl; id++)
			{
				database::Record * rec = contCrustalThicknessIo_Table->getRecord(static_cast<int>(id));
				double Age = rec->getValue<double>("Age");
				std::string thicknessGrid = rec->getValue<std::string>("ThicknessGrid");

				if (Age <= basementAge)
				{
					mapNames.push_back(thicknessGrid);
				}
				else
				{
					m_model.removeRecordFromTable("ContCrustalThicknessIoTbl", id--);
					LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Info> ContCrustal thickness at age : " << Age << " Ma is removed as it is greater than the basement age : " << basementAge << " Ma";
					size_contCrustalThicknessIoTbl = contCrustalThicknessIo_Table->size();
					// Checking for each of the GridMap which is cleared as the row is removed if it is referred in any other row of the same table
					int flag = 0;
					for (flag = 0; flag < mapNames.size(); flag++)
					{
						if (thicknessGrid == mapNames[flag])
							break;
					}
					/// if the map is not referred in any other row of the same table then we can remove it from GridMapIoTbl
					if (flag == mapNames.size() && (thicknessGrid != ""))
					{
						Prograde::GridMapIoTblUpgradeManager::clearTblNameMapNameReferenceGridMap("ContCrustalThicknessIoTbl", thicknessGrid);
					}
				}
			}

			database::Record * updatedRecord = contCrustalThicknessIo_Table->getRecord(static_cast<int>(contCrustalThicknessIo_Table->size() - 1));
			double age_updatedLastRecord = updatedRecord->getValue<double>("Age");
			if (age_updatedLastRecord != basementAge) ///if a record with age=basement age is present then we donot have to interpolate
			{
				size_t rowNumber = contCrustalThicknessIo_Table->size();
				const DataAccess::Interface::PaleoFormationPropertyList* paleoFormations = m_crust->getPaleoThicknessHistory();
				AbstractSnapshotVsGridMap crustThicknesses;
				std::for_each(paleoFormations->begin(), paleoFormations->end(), [&crustThicknesses, basementAge, this, &rowNumber](const DataAccess::Interface::PaleoFormationProperty *const it)
				{
					const DataModel::AbstractSnapshot* const snapshot = it->getSnapshot();
					auto age = snapshot->getTime();
					if (age == basementAge)
					{
						const auto gridMap = const_cast<DataAccess::Interface::GridMap*>(it->getMap(DataAccess::Interface::CrustThinningHistoryInstanceThicknessMap));
						generateCrustalMaps("ContCrustalThicknessIoTbl", age, gridMap, "ContCrustThicknessInterpolated_", rowNumber);
						rowNumber++;
					}
				});
				//clean memory
				delete paleoFormations;
			}
		}
		// ContCrustalThicknessIoTbl updated

		/////////////////////////////////////////////////////////////////////////////////// updating OceaCrustalThicknessIoTbl //////////////////////////////////////////////////////////////////////////////////////////////////////////
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Info> Creating OceaCrustalThicknessIoTbl";
		/// In OceaCrustalThicknessIoTbl, times with only system generated snapshots are required.
		database::Table * snapshotIo_Table = m_ph->getTable("SnapshotIoTbl");
		std::vector<double>timeWithSystemSnapshots;
		for (size_t id = 0; id < snapshotIo_Table->size(); ++id)
		{
			std::string snapshotType;
			double time = 0;
			database::Record * rec = snapshotIo_Table->getRecord(static_cast<int>(id));
			snapshotType = rec->getValue<std::string>("TypeOfSnapshot");
			time = rec->getValue<double>("Time");
			///collecting the times of system generated snapshots in a vector
			if (!snapshotType.compare("System Generated"))
			{
				timeWithSystemSnapshots.push_back(time);
			}
		}
		std::size_t rowNumber = 0;
		std::for_each(m_basaltThicknessHistory.begin(), m_basaltThicknessHistory.end(), [this, &rowNumber, basementAge, timeWithSystemSnapshots](const SmartAbstractSnapshotSmartGridMapPair& pair)
		{
			const auto age = pair.first->getTime();
			if (age > basementAge) return; /// return from the function when age reaches basement age to avoid writing the maps for time values greater than the basement age
			for (int i = 0; i < timeWithSystemSnapshots.size(); i++)
			{
				if (age == timeWithSystemSnapshots[i])
				{
					const auto gridMap = const_cast<DataAccess::Interface::GridMap*>(pair.second.get());
					generateCrustalMaps("OceaCrustalThicknessIoTbl", age, gridMap, "OceanicCrustThicknessFromLegacyALC_", rowNumber);
					rowNumber++;
					break; ///break out of the for loop when the age matches one of the values in timeWithSystemSnapshots
				}
			}
		});
		// OceaCrustalThicknessIoTbl updated

		//clearing BasaltThicknessIoTbl, CrustIoTbl and MntlHeatFlowIoTbl as ALC mode is detected
		m_model.clearTable("BasaltThicknessIoTbl");
		m_model.clearTable("CrustIoTbl");
		m_model.clearTable("MntlHeatFlowIoTbl");
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Info> BasaltThicknessIoTbl is cleared; map references in GridMapIoTbl will be removed by GridMapIoTbl Upgrade manager";
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Info> CrustIoTbl is cleared; map references in GridMapIoTbl will be removed by GridMapIoTbl Upgrade manager";
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Info> MntlHeatFlowIoTbl is cleared; map references in GridMapIoTbl will be removed by GridMapIoTbl Upgrade manager";

		//updating GridMapIoTbl for deleting the references of the maps in the tables cleared
		Prograde::GridMapIoTblUpgradeManager::clearTblNameMapNameReferenceGridMap("BasaltThicknessIoTbl");
		Prograde::GridMapIoTblUpgradeManager::clearTblNameMapNameReferenceGridMap("CrustIoTbl");
		Prograde::GridMapIoTblUpgradeManager::clearTblNameMapNameReferenceGridMap("MntlHeatFlowIoTbl");

		//upgrading the crust property model to standard conductivity crust
		BottomBoundaryManager::CrustPropertyModel crstPropModel;
		m_model.bottomBoundaryManager().getCrustPropertyModel(crstPropModel);
		m_model.bottomBoundaryManager().setCrustPropertyModel(modelConverter.upgradeAlcCrustPropModel(crstPropModel));

		//upgrading the mantle property model to high conductivity mantle model
		BottomBoundaryManager::MantlePropertyModel mntlPropModel;
		m_model.bottomBoundaryManager().getMantlePropertyModel(mntlPropModel);
		m_model.bottomBoundaryManager().setMantlePropertyModel(modelConverter.upgradeAlcMantlePropModel(mntlPropModel));

	}
	else
	{
		if (m_ph->isFixedTempBasement()) LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Info> Basic Crust Thinning mode detected";
		else if (m_ph->isFixedHeatFlow())  LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Info> Fixed Heat Flow mode detected";
		else  LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Info> Bottom Boundary Condition : Unknown";
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Info> no ALC upgrade needed!";
	}
}

//------------------------------------------------------------//

bool Prograde::AlcUpgradeManager::isLegacyAlc() const
{
	if (m_ph->isALC()) {
		const DataAccess::Interface::GridMap* presentDayBasaltThickness = m_crust->getBasaltThicknessMap();
		const DataAccess::Interface::GridMap* crustMeltOnsetMap = m_crust->getCrustThicknessMeltOnsetMap();
		return (presentDayBasaltThickness != nullptr or crustMeltOnsetMap != nullptr);
	}
	else {
		return false;
	}
}

//------------------------------------------------------------//

void Prograde::AlcUpgradeManager::createCrustThickness()
{
	m_ph->setActivityOutputGrid(m_ph->getInputGrid());
	m_ph->loadCrustThinningHistory();
	m_ph->addCrustThinningHistoryMaps();
	m_ph->correctCrustThicknessHistory();
	m_ph->initialiseValidNodes(false);
	DataAccess::Interface::ContinentalCrustHistoryGenerator generator(*m_ph->getActivityOutputGrid(), *m_crust, m_ph->getValidator(), m_ph->getGlobalOperations());
	generator.createCrustThickness(DataAccess::Interface::IMPROVED_LITHOSPHERE_CALCULATOR_LINEAR_ELEMENT_MODE);
	m_initialCrustalThickness = generator.getInitialCrustalThickness();
	m_crustThicknessHistory = generator.getcrustThicknessHistory();
}

//------------------------------------------------------------//

void Prograde::AlcUpgradeManager::computeBasaltThickness()
{

	const DataAccess::Interface::PaleoFormationPropertyList* paleoFormations = m_crust->getPaleoThicknessHistory();
	const DataAccess::Interface::GridMap* presentDayBasaltThickness = m_crust->getBasaltThicknessMap();
	const DataAccess::Interface::GridMap* crustMeltOnsetMap = m_crust->getCrustThicknessMeltOnsetMap();
	const DataAccess::Interface::Grid* referenceGrid = m_ph->getInputGrid();
	const DataAccess::Interface::ObjectFactory* factory = m_ph->getFactory();

	AbstractSnapshotVsGridMap crustThicknesses;
	std::for_each(paleoFormations->begin(), paleoFormations->end(), [&crustThicknesses](const DataAccess::Interface::PaleoFormationProperty *const it)
	{
		const DataAccess::Interface::GridMap* const gridMap = it->getMap(DataAccess::Interface::CrustThinningHistoryInstanceThicknessMap);
		const DataModel::AbstractSnapshot* const snapshot = it->getSnapshot();
		crustThicknesses.insert(std::pair<const DataModel::AbstractSnapshot* const, const DataAccess::Interface::GridMap* const>(snapshot, gridMap));
	});

	try {
		LegacyBasaltThicknessCalculator legacyBasaltThicknessCalculator(crustThicknesses,
			m_crustThicknessHistory,
			presentDayBasaltThickness,
			crustMeltOnsetMap,
			m_initialCrustalThickness,
			m_ph->getValidator(),
			referenceGrid,
			factory);
		legacyBasaltThicknessCalculator.compute(m_basaltThicknessHistory);
	}
	catch (std::invalid_argument& ex) {
		LogHandler(LogHandler::ERROR_SEVERITY) << "One of the Legacy Advanced Lithosphere Calculator (ALC) input is invalid (see details below)";
		LogHandler(LogHandler::ERROR_SEVERITY) << ex.what();
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Error> Migration from BPA to BPA2 Basin Aborted...";
		// there is only one kind of exception caught in this catch block that is the maps with -ve thickness which as per the business decision makes the scenario invalid for migration to bpa2
		exit(31);
	}
	catch (std::runtime_error& ex) {
		LogHandler(LogHandler::ERROR_SEVERITY) << "The Legacy Advanced Lithosphere Calculator (ALC) could not compute the Effective Crustal Thickness (see details below)";
		LogHandler(LogHandler::ERROR_SEVERITY) << ex.what();
	}
	catch (...) {
		LogHandler(LogHandler::ERROR_SEVERITY) << "The Legacy Advanced Lithosphere Calculator encounterred a fatal error (unkown details)";
	}

	//clean memory
	delete paleoFormations;
}
//------------------------------------------------------------//

void Prograde::AlcUpgradeManager::generateCrustalMaps(std::string refferedTable, double age, DataAccess::Interface::GridMap* const gridMap, std::string mapName_part, std::size_t &rowNumber)
{
	double value = 0;
	double minV = 6300000.0;///initialising with the maximum possible value
	double maxV = 0;///initialising with the minimum possible value
	for (unsigned int j = gridMap->firstJ(); j <= gridMap->lastJ(); ++j)
	{
		for (unsigned int i = gridMap->firstI(); i <= gridMap->lastI(); ++i)
		{
			value = gridMap->getValue(i, j);
			if (value != DataAccess::Interface::DefaultUndefinedMapValue) ///avoiding the undefined nodes
			{
				if (minV > value) minV = value;
				if (maxV < value) maxV = value;
			}
		}
	}
	const auto mapName = mapName_part + std::to_string(age);
	const auto ageField = "Age";
	const auto thicknessField = "Thickness";
	const auto thicknessGridField = "ThicknessGrid";
	///check if the values at all the nodes in the gridmap is a constant value; if constant then store it as a scalar instead of a map
	if (minV == maxV)
	{
		///check for limits and clip values to range
		double updatedValue = minV;

		if (!refferedTable.compare("ContCrustalThicknessIoTbl"))
		{
			if (!NumericFunctions::inRange(minV, 0.0, 6300000.0))
			{
				updatedValue = NumericFunctions::clipValueToRange(minV, 0.0, 6300000.0);
				LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Warning> Calculated ContCrustal thickness at age : " << age << " Ma : " << minV << " m  is not in the acceptable limits [0,6300000.0], hence clipped to : " << updatedValue;
			}
		}
		else if (!refferedTable.compare("OceaCrustalThicknessIoTbl"))
		{
			if (!NumericFunctions::inRange(minV, 0.0, 50000.0))
			{
				updatedValue = NumericFunctions::clipValueToRange(minV, 0.0, 50000.0);
				LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Warning> Calculated OceaCrustal thickness at age : " << age << " Ma : " << minV << " m  is not in the acceptable limits [0,50000], hence clipped to : " << updatedValue;
			}
		}

		m_model.addRowToTable(refferedTable);
		m_model.setTableValue(refferedTable, rowNumber, ageField, age);
		m_model.setTableValue(refferedTable, rowNumber, thicknessField, updatedValue);
		m_model.setTableValue(refferedTable, rowNumber, thicknessGridField, "");
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Info> A record with calculated value of thickness : " << updatedValue << " m at the age : " << age << " Ma is added";
	}
	else
	{
		const auto outputFileName = "Inputs.HDF";
		size_t mapsSequenceNbr = Utilities::Numerical::NoDataIDValue;
		m_model.mapsManager().generateMap(refferedTable, mapName, gridMap, mapsSequenceNbr, outputFileName);
		m_model.addRowToTable(refferedTable);
		m_model.setTableValue(refferedTable, rowNumber, ageField, age);
		m_model.setTableValue(refferedTable, rowNumber, thicknessGridField, mapName);
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Info> A record with calculated thickness map : " << mapName << " at the age : " << age << " Ma is added; Reference to GridMapIoTbl is added";
		//check for the values in the maps to be in range
		if (!refferedTable.compare("ContCrustalThicknessIoTbl"))
		{
			if ((!NumericFunctions::inRange(minV, 0.0, 6300000.0)) || (!NumericFunctions::inRange(maxV, 0.0, 6300000.0)))
			{
				LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Info> The values [min,max] ["<<minV<<", "<<maxV<<"] in the ContCrustal ThicknessGrid : " << mapName << " at age : " << age << " Ma are not in the acceptable limits [0,6300000]";
			}
		}
		else if (!refferedTable.compare("OceaCrustalThicknessIoTbl"))
		{
			if ((!NumericFunctions::inRange(minV, 0.0, 50000.0)) || (!NumericFunctions::inRange(maxV, 0.0, 50000.0)))
			{
				LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Info> The values [min,max] [" << minV << ", " << maxV << "] in the OceaCrustal ThicknessGrid : " << mapName << " at age : " << age << " Ma are not in the acceptable limits [0,50000]";
			}
		}
	}
}

//
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by CGI
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "BottomBoundaryModelUpgradeManager.h"
#include "NumericFunctions.h"
#include "cmbAPI.h"
#include "BottomBoundaryManager.h"
#include "MapsManager.h"
//DataAccess
#include "ProjectHandle.h"
#include "CrustFormation.h"
#include "Surface.h"
#include "Snapshot.h"
#include "Interface.h"
#include "PaleoSurfaceProperty.h"

// TableIO library
#include "database.h"
#include "cauldronschema.h"
#include "cauldronschemafuncs.h"
#include "ErrorHandler.h"
#include "LogHandler.h"

using namespace mbapi;
//using namespace std;
//#include<map>
#include <algorithm>

Prograde::BottomBoundaryModelUpgradeManager::BottomBoundaryModelUpgradeManager(std::string name, mbapi::Model & model) :
	IUpgradeManager(name), m_model(model)
{
	const auto ph = m_model.projectHandle();
	if (ph == nullptr) {
		throw std::invalid_argument(getName() + " cannot retrieve the project handle from Cauldron data model");
	}
	m_ph = ph;
}


void Prograde::BottomBoundaryModelUpgradeManager::updateTableValue(const std::string & tableName, const std::string & fieldName, const double bpa2DefaultValue)
{
	//this->CheckDefaultValues(tableName, fieldName, bpa2DefaultValue);
	
}

void Prograde::BottomBoundaryModelUpgradeManager::preProcessingInput(const std::string tableName, const std::string propertyName, double allowedMinValue, double allowedMaxValue)
{
	database::Table * bottomBoundary_Table = m_ph->getTable(tableName);
	size_t bottomBoundaryTableSize = bottomBoundary_Table->size();
	mbapi::MapsManager& mapsMgrLocal = m_model.mapsManager();
	for (int id = 0; id < bottomBoundaryTableSize; id++)
	{
		database::Record *rec = bottomBoundary_Table->getRecord(id);
		double age = rec->getValue<double>("Age");
		double propoertyValue = rec->getValue<double>(propertyName);
		std::string propertyMapName = rec->getValue<std::string>(propertyName + "Grid");

		double minV, maxV;
		//Checking property values (maps/scalar) for within the limit or not
		if (propertyMapName.compare(DataAccess::Interface::NullString))
		{
			auto mapID = mapsMgrLocal.findID(propertyMapName);
			if (ErrorHandler::NoError != mapsMgrLocal.mapValuesRange(mapID, minV, maxV))
				throw ErrorHandler::Exception(mapsMgrLocal.errorCode()) << mapsMgrLocal.errorMessage();

			if (!(NumericFunctions::inRange(minV, allowedMinValue, allowedMaxValue) && NumericFunctions::inRange(maxV, allowedMinValue, allowedMaxValue)))
				LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Info> The max-min of legacy " << propertyName << " map at " << age << " Ma is found ["<< minV<<","<< maxV<<"] which are OUT of Range [" << allowedMinValue << "," << allowedMaxValue << "]";
			if (propoertyValue != DataAccess::Interface::DefaultUndefinedScalarValue)
			{
				LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Info> Legacy " << propertyName << " value of "<< propoertyValue<<" at " << age << " Ma is reset to " << DataAccess::Interface::DefaultUndefinedScalarValue << " as map is defined at that age";
				propoertyValue = DataAccess::Interface::DefaultUndefinedScalarValue;
				m_model.setTableValue(tableName, id, propertyName, propoertyValue);
			}
		}
		else
		{
			if (!(NumericFunctions::inRange(propoertyValue, allowedMinValue, allowedMaxValue)))
			{
				LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Warning> Legacy " << propertyName << " value of " << propoertyValue << " at " << age << " Ma is " << propoertyValue << ", which is OUT of range [" << allowedMinValue << "," << allowedMaxValue << "]. Resetting to nearest limit! ";
				propoertyValue = NumericFunctions::clipValueToRange(propoertyValue, allowedMinValue, allowedMaxValue);
				m_model.setTableValue(tableName, id, propertyName, propoertyValue);
			}
		}//Checking for the limiting values are done
	}
}

bool Prograde::BottomBoundaryModelUpgradeManager::findInterpolatingAges(const std::string tableName, const std::string propertyName, const double basinAge, double& interpolatingLowerAge, double& interpolatingHigherAge)
{
	bool doInterpolate = true;
	database::Table * bottomBoundary_Table = m_ph->getTable(tableName);
	size_t bottomBoundaryTableSize = bottomBoundary_Table->size();
	database::Record *lastRec = bottomBoundary_Table->getRecord(static_cast<int>(bottomBoundaryTableSize) - 1);

	double oldestBottomBoundaryAge = lastRec->getValue<double>("Age");
	LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Info> The oldest " << propertyName << " Age is: " << oldestBottomBoundaryAge << "Ma";

	if (oldestBottomBoundaryAge < basinAge)
	{
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Warning> BasinAge is older than the oldest " << propertyName << " age. Genearating the " << propertyName << " history at basement age by constant extrapolation";
		double property_lastRecord = lastRec->getValue<double>(propertyName);
		std::string propertyGrid_lastRecord = lastRec->getValue<std::string>(propertyName + "Grid");
		m_model.addRowToTable(tableName);
		m_model.setTableValue(tableName, bottomBoundaryTableSize, "Age", basinAge);
		m_model.setTableValue(tableName, bottomBoundaryTableSize, propertyName, property_lastRecord);
		m_model.setTableValue(tableName, bottomBoundaryTableSize, (propertyName + "Grid"), propertyGrid_lastRecord);

		doInterpolate = false;
	}
	else//Linear interpolation of the maps/scalars
	{
		for (int id = 0; id < bottomBoundaryTableSize; id++)
		{
			database::Record *rec = bottomBoundary_Table->getRecord(id);
			double age = rec->getValue<double>("Age");

			if (age == basinAge)
			{
				doInterpolate = false;
				LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Info> "<<propertyName <<" at basement age is present. No extra calculations to generate the heatflow values at the BasinAge is needed.";
			}
			else if (age > basinAge)
			{
				interpolatingHigherAge = age;
				break;
			}
			else
			{
				interpolatingLowerAge = age;
			}
		}
	}
	return doInterpolate;
}

DataAccess::Interface::GridMap * Prograde::BottomBoundaryModelUpgradeManager::generateInterpolatedMapAtAge(std::string bottomBoundaryModel, bool needInterpolation, const double basinAge, double& interpolatingLowerAge, double& interpolatingHigherAge)
{
	DataAccess::Interface::GridMap* interpolatedMap = nullptr;
	if (needInterpolation)
	{
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "BasinAge is younger than the oldest property age. Genearating the property history at basinAge(" << basinAge << " Ma) by Linear Interpolating using the property at " << interpolatingLowerAge << " Ma and " << interpolatingHigherAge << " Ma";
		if (bottomBoundaryModel == "Fixed HeatFlow")
		{
			m_ph->loadHeatFlowHistory();
			const DataAccess::Interface::PaleoSurfacePropertyList* heatFlowMaps = m_ph->getHeatFlowHistory();
			DataAccess::Interface::PaleoSurfacePropertyList::const_iterator heatFlowIter;

			double age1, age2;
			const DataAccess::Interface::GridMap* heatFlowMap1, *heatFlowMap2, *heatFlowMap;

			bool foundInterpolatingLowerAge = false;
			bool foundInterpolatingHigherAge = false;

			for (heatFlowIter = heatFlowMaps->begin(); heatFlowIter != heatFlowMaps->end(); ++heatFlowIter)
			{
				const DataAccess::Interface::PaleoSurfaceProperty* heatFlowInstance = dynamic_cast<const DataAccess::Interface::PaleoSurfaceProperty*>(*heatFlowIter);

				const double age = heatFlowInstance->getSnapshot()->getTime();
				heatFlowMap = heatFlowInstance->getMap(DataAccess::Interface::HeatFlowHistoryInstanceHeatFlowMap);

				if (age == interpolatingLowerAge)
				{
					foundInterpolatingLowerAge = true;
					heatFlowMap1 = heatFlowMap;
				
					age1 = age;
				}
				if (age == interpolatingHigherAge)
				{
					foundInterpolatingHigherAge = true;
					heatFlowMap2 = heatFlowMap;
					age2 = age;
				}
				if (foundInterpolatingLowerAge && foundInterpolatingHigherAge)
				{
					database::Record record(*(*heatFlowIter)->getRecord());
					database::setAge(&record, basinAge);

					DataAccess::Interface::PaleoSurfaceProperty* interpolatedHeatFlowMap = m_ph->getFactory()->producePaleoSurfaceProperty(*m_ph.get(), &record, m_ph->getCrustFormation()->getTopSurface());
					DataAccess::Interface::InterpolateFunctor functor(age1, age2, basinAge);
					interpolatedMap = interpolatedHeatFlowMap->computeMap(DataAccess::Interface::HeatFlowHistoryInstanceHeatFlowMap, heatFlowMap1, heatFlowMap2, functor);

					break;
				}
			}
		}
	}
	return interpolatedMap;
}

//This function appended the newly generated map at BasinAge at the specified "rowIndex" to the respective table (tableName)
void Prograde::BottomBoundaryModelUpgradeManager::saveInterpolatedMap(DataAccess::Interface::GridMap * gridmap, const std::string tableName, const size_t rowIndex, const std::string propertyName, const double basinAge, const double allowedMinValue, const double allowedMaxValue)
{
	const auto outputFileName = "Inputs.HDF";
	const auto mapName = "InterpolatedMap_" + std::to_string(basinAge);
	size_t mapsSequenceNbr = Utilities::Numerical::NoDataIDValue;

	//Check whether the newly calculated map is constant or not..meaning the map is having a single value (scalar) or not mutiple
	//If it is true then save the entry as a scalar in the respective table else save it as a map and have the map in the inputs.hdf.
	bool isConstant = gridmap->isConstant();
	double allowedPropertyValueMin = allowedMinValue;
	double allowedPropertyValueMax = allowedMaxValue;
	if (isConstant)
	{
		double propoValue = gridmap->getConstantValue();
		if (!(NumericFunctions::inRange(propoValue, allowedPropertyValueMin, allowedPropertyValueMax)))
		{
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Warning> Interpolated " << propertyName << " scalar value at basinAge (" << basinAge << " Ma) is " << propoValue << ", which is OUT of range [" << allowedPropertyValueMin << "," << allowedPropertyValueMax << "]. Resetting to nearest limit! ";
			propoValue = NumericFunctions::clipValueToRange(propoValue, allowedPropertyValueMin, allowedPropertyValueMax);

		}
		m_model.addRowToTable(tableName);
		m_model.setTableValue(tableName, rowIndex, "Age", basinAge);
		m_model.setTableValue(tableName, rowIndex, propertyName, propoValue);
		m_model.setTableValue(tableName, rowIndex, (propertyName + "Grid"), DataAccess::Interface::NullString);
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Info> Interpolated " << propertyName << " of " << propoValue << " m at basinAge (" << basinAge << " Ma) is added in " << tableName;

	}
	else
	{
		size_t mapIdToSave = m_model.mapsManager().generateMap(tableName, mapName, gridmap, mapsSequenceNbr, outputFileName);
		if (IsValueUndefined(mapIdToSave))
		{
			throw ErrorHandler::Exception(ErrorHandler::ValidationError) << "Interpolated " << propertyName << " map not valid in " << tableName;
		}
		else
		{
			//Additional check to know whether the map values are within the allowed range or not 
			double minV, maxV;
			gridmap->getMinMaxValue(minV, maxV);
			if (!(NumericFunctions::inRange(minV, allowedPropertyValueMin, allowedPropertyValueMax) && NumericFunctions::inRange(maxV, allowedPropertyValueMin, allowedPropertyValueMax)))
				LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Warning> The max-min of interpolated " << propertyName + " map at " << basinAge << " Ma is [" << minV << "," << maxV << "] which are OUT of Range [" << allowedPropertyValueMin << "," << allowedPropertyValueMax << "]";

			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Warning> Interpolated " << propertyName << " map at basinAge (" << basinAge << " Ma) is added in " << tableName;
			m_model.addRowToTable(tableName);
			m_model.setTableValue(tableName, rowIndex, "Age", basinAge);
			m_model.setTableValue(tableName, rowIndex, propertyName, DataAccess::Interface::DefaultUndefinedScalarValue);
			m_model.setTableValue(tableName, rowIndex, (propertyName + "Grid"), mapName);
		}
	}
}

//void Prograde::BottomBoundaryModelUpgradeManager::removeRecordsOlderThanBasinAge(const std::string & tableName, const double basinAge, std::string propertyName)
std::vector<std::pair<std::string, std::string>> Prograde::BottomBoundaryModelUpgradeManager::removeRecordsOlderThanBasinAge(const std::string & tableName, const double basinAge, std::string propertyName)
{
	mbapi::BottomBoundaryManager& bbmMgrLocal = m_model.bottomBoundaryManager();
	database::Table * tableToCheck = m_ph->getTable(tableName);
	std::vector<std::pair<std::string, std::string>> removed_TableName_MapName;
	std::vector<std::string> mapsUsedInYoungerEvenets, mapsUsedInOlderEvents;//Younger and older are defined wrt basinAge 
	for (int index = 0; index <= (static_cast<int>(tableToCheck->size()) - 1); index++)
	{
		database::Record *rec = tableToCheck->getRecord(index);
		double ageOfTheRecord = rec->getValue<double>("Age");
		std::string mapName = rec->getValue<std::string>(propertyName+"Grid");
		if (ageOfTheRecord > basinAge)
		{
			ErrorHandler::ReturnCode err = m_model.removeRecordFromTable(tableName, index);
			if (err == ErrorHandler::ReturnCode::NoError)
			{	
				index--;
				tableToCheck = m_ph->getTable(tableName);//size of the table gets updated as removing the record will reduce the table size by 1
				//Storing the map names which are getting used in the record removed from the table. These maps needs to be deleted from the GridMapIoTbl. 
				//There will be no issue if the records removed have uniques maps. However, special care needs to be taken if the mapName gethered for the older wvents were specified in the younger events 
				mapsUsedInOlderEvents.push_back(mapName);
				LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Info> Removing the record from " << tableName << " at age (" << ageOfTheRecord << " Ma) as it is older than the basin age (" << basinAge << " Ma)";
			}
			else
				throw ErrorHandler::Exception(err) << "Can't delete the record from "<< tableName;
		}
		else
		{
			mapsUsedInYoungerEvenets.push_back(mapName);
		}
	}

	//Sort the mapsUsedInOlderEvents and mapsUsedInYoungerEvenets to have only unique maps first
	sort(mapsUsedInYoungerEvenets.begin(), mapsUsedInYoungerEvenets.end());
	sort(mapsUsedInOlderEvents.begin(), mapsUsedInOlderEvents.end());

	//remove the duplicates
	mapsUsedInYoungerEvenets.erase(unique(mapsUsedInYoungerEvenets.begin(), mapsUsedInYoungerEvenets.end()), mapsUsedInYoungerEvenets.end());	
	mapsUsedInOlderEvents.erase(unique(mapsUsedInOlderEvents.begin(), mapsUsedInOlderEvents.end()), mapsUsedInOlderEvents.end());

	//Updating mapsUsedInOlderEvents to keep only the map names that does not matches with the maps stored in mapsUsedInYoungerEvenets vector. 
	std::vector<std::string>::iterator it = mapsUsedInOlderEvents.begin();
	while (it!= mapsUsedInOlderEvents.end())
	{
		std::string mapDeleted = *it;
		std::vector<std::string>::iterator it1 = std::find(std::begin(mapsUsedInYoungerEvenets), std::end(mapsUsedInYoungerEvenets), mapDeleted);
	
		if (it1 != mapsUsedInYoungerEvenets.end())
		{
			std::vector<std::string>::iterator position = std::find(mapsUsedInOlderEvents.begin(), mapsUsedInOlderEvents.end(), *it1);
			if (position != mapsUsedInOlderEvents.end())
			{
				it=mapsUsedInOlderEvents.erase(position);
				continue;
			}
			else
				it++;		
		} 
		else
			it++;	
	}

	for (int i = 0; i < mapsUsedInOlderEvents.size(); i++)
	{
		std::string mapDeleted = mapsUsedInOlderEvents[i];
		removed_TableName_MapName.push_back(std::make_pair(tableName, mapDeleted));
	}

	return removed_TableName_MapName;
}

bool Prograde::BottomBoundaryModelUpgradeManager::cleartables(std::string tableName)
{
	database::Table * tableToClear = m_ph->getTable(tableName);
	size_t SizeOfTableToClear = tableToClear->size();

	//Clear the tables only if the legacy tables are having records..otherwise no need to perform this cleaning.
	if (static_cast<int>(SizeOfTableToClear) != 0)
	{
		ErrorHandler::ReturnCode err = m_model.clearTable(tableName);
		if (err != ErrorHandler::ReturnCode::NoError)
			throw ErrorHandler::Exception(err) << tableName << " can't be cleared";
		else {
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Info> "<< tableName << " is cleared as not needed";
			return true;
		}
	}
	return false;
}

void Prograde::BottomBoundaryModelUpgradeManager::removeRecordsFromGridMapIoTbl(bool isRemoved, std::string tableName)
{
	if (isRemoved)
	{
		std::string referredBy{}, mapName{};
		database::Table * tableToClear = m_ph->getTable("GridMapIoTbl");
		size_t tableSize= tableToClear->size();
		for (int id = 0; id < tableSize; id++)
		{
			database::Record *rec = tableToClear->getRecord(id);
			referredBy = rec->getValue<std::string>("ReferredBy");
			mapName = rec->getValue<std::string>("MapName");
			if (referredBy == tableName)
			{
				m_model.removeRecordFromTable("GridMapIoTbl", id);
				LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Info> Map record (" << mapName << ") for " << tableName << " is removed from the GridMapIoTbl since " << tableName << " is cleared";
				id--;
				tableSize = tableToClear->size();//The size of the table will also reduced by 1 if we remove a record from there. So resizing the loop count
			}

		}

	}
}

bool Prograde::BottomBoundaryModelUpgradeManager::CheckDefaultValues(const std::string & tableName, const std::string & fieldName, const double bpa2DefaultValue)
{
	auto propertyValue = m_model.tableValueAsDouble(tableName, 0, fieldName);
	if (!NumericFunctions::isEqual(propertyValue, bpa2DefaultValue, 1e-6))
	{
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Note: Legacy " << fieldName << " value:" << propertyValue << ", not equal to the default value of BPA2:" << bpa2DefaultValue << ". Not resetting the value.";
		return false;
	}

	return true;
}

void Prograde::BottomBoundaryModelUpgradeManager::checkRangeForMaps(const std::string & tableName, const std::string & propertyName, double lowerLimit, double higherLimit)
{
	std::string propNameGrid = propertyName + "Grid";
	mbapi::MapsManager& mapsMgrLocal = m_model.mapsManager();

	database::Table * tableToCheck = m_ph->getTable(tableName);
	database::Record *rec = tableToCheck->getRecord(0);

	double propoertyValue = rec->getValue<double>(propertyName);
	std::string propertyMapName = rec->getValue<std::string>(propertyName + "Grid");

	if (propertyMapName.compare(DataAccess::Interface::NullString))
	{
		auto mi = mapsMgrLocal.findID(propertyMapName);
		/* check whether the map values falls under the scceptable limit of [lowerLimit, higherLimit] or not */
		double mapMin;
		double mapMax;
		if (ErrorHandler::NoError != mapsMgrLocal.mapValuesRange(mi, mapMin, mapMax))
			throw ErrorHandler::Exception(mapsMgrLocal.errorCode()) << mapsMgrLocal.errorMessage();
		if (!(NumericFunctions::inRange(mapMin, lowerLimit, higherLimit) && NumericFunctions::inRange(mapMax, lowerLimit, higherLimit)))
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Note: " << propertyMapName + " map is OUT of Range [" << lowerLimit << "," << higherLimit << "]";

		if (propoertyValue != DataAccess::Interface::DefaultUndefinedScalarValue)
		{
			propoertyValue = DataAccess::Interface::DefaultUndefinedScalarValue;
			m_model.setTableValue(tableName, 0, propertyName, propoertyValue);

			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Note: " << propoertyValue << " is updated to " << DataAccess::Interface::DefaultUndefinedScalarValue << " as grid map is present for that property";
		}
	}
	else
	{
		if (!(NumericFunctions::inRange(propoertyValue, lowerLimit, higherLimit)))
		{
			LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "Note: " << propertyName << " scalar value : " << propoertyValue << ",  is OUT of range [" << lowerLimit << "," << higherLimit << "]. Resetting to nearest limit! ";
			propoertyValue = NumericFunctions::clipValueToRange(propoertyValue, lowerLimit, higherLimit);
			m_model.setTableValue(tableName, 0, propertyName, propoertyValue);
		}

	}
}

void Prograde::BottomBoundaryModelUpgradeManager::onlyAgesOlderThanBasinPresent(const std::string& tableName, const double basementAge)
{
	database::Table* tableToCheck = m_ph->getTable(tableName);
	database::Record* rec = tableToCheck->getRecord(0);
	double age = rec->getValue<double>("Age");
	if (age > basementAge)
	{
		rec->setValue<double>("Age", basementAge);
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Info> Table " << tableName << " contains records only for ages greater than the basement age : " << basementAge<<"; Hence, the record at the basement age is obtained by constant interpolation from the record at age : " << age;
	}
}
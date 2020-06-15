//
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by CGI
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "FaultCutUpgradeManager.h"
#include "StratigraphyModelConverter.h"
// Prograde class to update the GridMapIoTbl if any GridMap is removed from any table
#include "GridMapIoTblUpgradeManager.h"
/**Static function named 'Prograde::GridMapIoTblUpgradeManager::clearTblNameMapNamepReferenceGridMap()' is defined for the operation
* Overload 1: Prograde::GridMapIoTblUpgradeManager::clearTblNameMapNamepReferenceGridMap("tableName"); // Clears all the map references ReferredBy the table "tableName" from GridMapIoTbl
* Overload 2: Prograde::GridMapIoTblUpgradeManager::clearTblNameMapNamepReferenceGridMap("tableName","mapName"); // Clears the map reference of the "mapName" ReferredBy "tableName" from GridMapIoTbl
*/

//utilities
#include "LogHandler.h"

//cmbAPI
#include "cmbAPI.h"


using namespace mbapi;

//------------------------------------------------------------//

Prograde::FaultCutUpgradeManager::FaultCutUpgradeManager(Model& model) :
	IUpgradeManager("Faultcut upgrade manager"), m_model(model)
{
	const auto ph = m_model.projectHandle();
	if (ph == nullptr) {
		throw std::invalid_argument(getName() + " cannot retreive the project handle from Cauldron data model");
	}
	m_ph = ph;
}

//------------------------------------------------------------//

void Prograde::FaultCutUpgradeManager::upgrade()
{
	///using functions defined in the StratigraphyModelConverter for name updation
	Prograde::StratigraphyModelConverter modelConverter;

	double age = 0;
	std::string name;
	std::string updated_name;

	database::Table * stratIo_Table = m_ph->getTable("StratIoTbl");
	database::Record * rec = stratIo_Table->getRecord(static_cast<int>(stratIo_Table->size() - 1));
	double basementAge = rec->getValue<double>("DepoAge");

	// Updating PalinspasticIoTbl - remove the special characters from SurfaceName and BottomFormationName 
	database::Table * palinspasticio_table = m_ph->getTable("PalinspasticIoTbl");
	if (palinspasticio_table->size() != 0)
	{
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Info> Updating the PalinspasticIoTbl : ";
		for (size_t id = 0; id < palinspasticio_table->size(); ++id)
		{
			database::Record * rec = palinspasticio_table->getRecord(static_cast<int>(id));
			age = rec->getValue<double>("Age");
			// SurfaceName
			name = rec->getValue<std::string>("SurfaceName");
			updated_name = modelConverter.upgradeName(name);
			// LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "* At age : " << age<<",";
			if (name.compare(updated_name))
			{
				LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Info>  At age : " << age << ", SurfaceName : " << name << " is updated to " << updated_name;
				rec->setValue<std::string>("SurfaceName", updated_name);
			}
			// BottomFormationName
			name = rec->getValue<std::string>("BottomFormationName");
			updated_name = modelConverter.upgradeName(name);
			if (name.compare(updated_name))
			{
				LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Info> At age : " << age << ", BottomFormationName : " << name << " is updated to " << updated_name;
				rec->setValue<std::string>("BottomFormationName", updated_name);
			}
		}/// for loop ends here
	}

	// Updating FaultCutIoTbl - by removing the rows with Age >= basementAge
	database::Table * faultcutio_table = m_ph->getTable("FaultcutIoTbl");
	size_t sizeFaultcutIoTbl = faultcutio_table->size();
	if (sizeFaultcutIoTbl != 0)
	{
		std::string mapName;
		std::vector<std::string> mapNames;
		for (size_t id = 0; id < sizeFaultcutIoTbl; ++id)
		{
			database::Record * rec = faultcutio_table->getRecord(static_cast<int>(id));
			mapName = rec->getValue<std::string>("SurfaceName");
			age = rec->getValue<double>("Age");
			if (age < basementAge)
			{
				mapNames.push_back(mapName);
			}
		}/// for loop ends here
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "Updating the FaultcutIoTbl : ";
		for (size_t id = 0; id < sizeFaultcutIoTbl; ++id)
		{
			database::Record * rec = faultcutio_table->getRecord(static_cast<int>(id));
			age = rec->getValue<double>("Age");
			if (age >= basementAge)
			{
				mapName = rec->getValue<std::string>("SurfaceName");
				m_model.removeRecordFromTable("FaultcutIoTbl", id--);
				sizeFaultcutIoTbl = faultcutio_table->size();
				LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Warning> Record with Age : " << age << " is removed as the age is greater than or equal to the basement age : " << basementAge;
				for (int i = 0; i < mapNames.size(); i++)
				{
					if (mapName == mapNames[i]) break;
					else
					{
						Prograde::GridMapIoTblUpgradeManager::clearTblNameMapNamepReferenceGridMap("FaultcutIoTbl", mapName);
						LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Info> GridMap " << mapName << " ReferredBy FaultcutIoTbl will be cleared by GridMapIoTbl Upgrade Manager as the record containing the map is removed";
					}
				}
			}
		}/// for loop ends here
	}
}

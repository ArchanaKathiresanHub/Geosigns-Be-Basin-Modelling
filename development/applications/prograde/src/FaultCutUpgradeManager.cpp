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
/**Static function named 'Prograde::GridMapIoTblUpgradeManager::clearTblNameMapNameReferenceGridMap()' is defined for the operation
* Overload 1: Prograde::GridMapIoTblUpgradeManager::clearTblNameMapNameReferenceGridMap("tableName"); // Clears all the map references ReferredBy the table "tableName" from GridMapIoTbl
* Overload 2: Prograde::GridMapIoTblUpgradeManager::clearTblNameMapNameReferenceGridMap("tableName","mapName"); // Clears the map reference of the "mapName" ReferredBy "tableName" from GridMapIoTbl
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
		throw std::invalid_argument(getName() + " cannot retrieve the project handle from Cauldron data model");
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
	if (stratIo_Table == nullptr) throw ErrorHandler::Exception(ErrorHandler::NonexistingID) << "Could not find the StratIoTbl in the Project3d file";
	database::Record * rec = stratIo_Table->getRecord(static_cast<int>(stratIo_Table->size() - 1));
	double basementAge = rec->getValue<double>("DepoAge");

	// Updating PalinspasticIoTbl - remove the special characters from SurfaceName and BottomFormationName
	// and if the BottomFormationName is empty then populate it with the layer name for which the column SurfaceName identifies the top surface
	std::string faulcutmaps;
	std::string nameSurface;
	database::Table * palinspasticio_table = m_ph->getTable("PalinspasticIoTbl");
	if (palinspasticio_table == nullptr) throw ErrorHandler::Exception(ErrorHandler::NonexistingID) << "Could not find the PalinspasticIoTbl in the Project3d file";
	if (palinspasticio_table->size() != 0)
	{
		LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_SUBSTEP) << "<Basin-Info> Updating the PalinspasticIoTbl : ";
		for (size_t id = 0; id < palinspasticio_table->size(); ++id)
		{
			database::Record * rec = palinspasticio_table->getRecord(static_cast<int>(id));
			age = rec->getValue<double>("Age");

			// SurfaceName - update for the special characters
			name = rec->getValue<std::string>("SurfaceName");
			updated_name = modelConverter.upgradeName(name);

			nameSurface = updated_name;

			if (name.compare(updated_name))
			{
				LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Info>  At age : " << age << ", SurfaceName : " << name << " is updated to " << updated_name;
				rec->setValue<std::string>("SurfaceName", updated_name);
			}

			// BottomFormationName - update for the special characters and BottomFormationName as empty
			name = rec->getValue<std::string>("BottomFormationName");

			// if the BottomFormationName is empty
			// No need to check for the special characters in this case as the BottomFormationName is getting populated from the StratIoTbl's LayerName
			// which has already been checked for the special characters in Stratigraphy Update Manager
			if (!name.compare(""))
			{
				faulcutmaps = rec->getValue<std::string>("FaultcutsMap");
				// get the LayerName corresponding to the surface from StratIoTbl
				name = getLayerNameFromStratIoTbl(nameSurface);
				rec->setValue<std::string>("BottomFormationName", name);
				LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Info> For the FaultcutsMap : "<< faulcutmaps<<", BottomFormationName is updated from empty to : '"
					<< name << "' which is the formation name corresponding to the Surface defining the faultcut";
			}
			// if the BottomFormationName is not empty then check for the special characters
			else
			{
				updated_name = modelConverter.upgradeName(name);
				if (name.compare(updated_name))
				{
					LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Info> At age : " << age << ", BottomFormationName : " << name << " is updated to " << updated_name;
					rec->setValue<std::string>("BottomFormationName", updated_name);
				}
			}
		}/// for loop ends here
	}

	// Updating FaultCutIoTbl - by removing the rows with Age >= basementAge
	database::Table * faultcutio_table = m_ph->getTable("FaultcutIoTbl");
	if (faultcutio_table == nullptr) throw ErrorHandler::Exception(ErrorHandler::NonexistingID) << "Could not find the FaultcutIoTbl in the Project3d file";
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
						Prograde::GridMapIoTblUpgradeManager::clearTblNameMapNameReferenceGridMap("FaultcutIoTbl", mapName);
						LogHandler(LogHandler::INFO_SEVERITY, LogHandler::COMPUTATION_DETAILS) << "<Basin-Info> GridMap " << mapName << " ReferredBy FaultcutIoTbl will be cleared by GridMapIoTbl Upgrade Manager as the record containing the map is removed";
					}
				}
			}
		}/// for loop ends here
	}
}

std::string Prograde::FaultCutUpgradeManager::getLayerNameFromStratIoTbl(const std::string& surfaceNameSearch)
{
	std::string layerName;
	std::string surfaceName;
	database::Table* stratIo_Table = m_ph->getTable("StratIoTbl");
	size_t sizeStratIoTbl = stratIo_Table->size();
	for (int id = 0; id < sizeStratIoTbl; ++id)
	{
		database::Record* rec = stratIo_Table->getRecord(static_cast<int>(id));
			surfaceName = rec->getValue<std::string>("SurfaceName");
		if (!surfaceName.compare(surfaceNameSearch))
		{
			layerName = rec->getValue<std::string>("LayerName");
			break;
		}
	}
	return layerName;
}



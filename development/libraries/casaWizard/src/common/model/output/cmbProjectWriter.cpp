//
// Copyright (C) 2020 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "cmbProjectWriter.h"

#include "cmbAPI.h"
#include "OutputProperty.h"

#include <QStringList>

using namespace DataAccess;
using namespace Interface;

namespace casaWizard
{

CMBProjectWriter::CMBProjectWriter(const QString& projectFile) :
  ProjectWriter(),
  cmbModel_{new mbapi::Model()}
{
  // Load the project from file into memory
  cmbModel_->loadModelFromProjectFile(projectFile.toStdString());
}

CMBProjectWriter::~CMBProjectWriter()
{
}

void CMBProjectWriter::appendTimeStampToCalibratedLithoMaps(const QString& timeStamp)
{
  mbapi::MapsManager& mapsManager = cmbModel_->mapsManager();
  mbapi::StratigraphyManager& stratManager = cmbModel_->stratigraphyManager();
  for (mbapi::StratigraphyManager::LayerID lid : stratManager.layersIDs())
  {
    std::vector<std::string> lithoList;
    std::vector<double> lithoPercent;
    std::vector<std::string> lithoPercMap;
    stratManager.layerLithologiesList( lid, lithoList, lithoPercent, lithoPercMap );
    bool foundSACpercentMap = false;
    for ( std::string& map : lithoPercMap )
    {
      if (map.find("_percent_") != std::string::npos)
      {
        const std::string newMapName = map + "_" + timeStamp.toStdString();
        cmbModel_->setTableValue("GridMapIoTbl", mapsManager.findID(map), "MapName", newMapName);
        map = newMapName;
        foundSACpercentMap = true;
      }
    }
    if (foundSACpercentMap)
    {
      stratManager.setLayerLithologiesList( lid, lithoList, lithoPercent, lithoPercMap );
    }
  }
}

void CMBProjectWriter::appendTimeStampToT2ZMaps(const QString &timeStamp)
{
  mbapi::MapsManager& mapsManager = cmbModel_->mapsManager();

  std::vector<std::pair<std::string, std::string>> t2zReferTables;
  t2zReferTables.push_back({"TwoWayTimeIoTbl", "TwoWayTimeGrid"});
  t2zReferTables.push_back({"StratIoTbl", "DepthGrid"});

  std::vector<mbapi::MapsManager::MapID> removeMaps;
  for (mbapi::MapsManager::MapID mapId : mapsManager.mapsIDs())
  {
    if (cmbModel_->tableValueAsString("GridMapIoTbl", mapId, "MapFileName") == "CalibratedInputs.HDF")
    {
      // Copy the map to the Inputs.HDF
      // Give the map name a timestamp
      std::string mapName = cmbModel_->tableValueAsString("GridMapIoTbl", mapId, "MapName");
      std::string newMapName = mapName + "_" + timeStamp.toStdString();
      const mbapi::MapsManager::MapID newId = mapsManager.copyMap(mapId, newMapName, "Inputs.HDF");
      mapsManager.saveMapToHDF(newId, "Inputs.HDF");
      removeMaps.push_back(mapId);

      // Also use the new map name in the referred table
      std::string referredTable = cmbModel_->tableValueAsString("GridMapIoTbl", mapId, "ReferredBy");
      for ( auto& t2zReferTable : t2zReferTables )
      {
        if (referredTable == t2zReferTable.first &&
            cmbModel_->tableSize(t2zReferTable.first) != Utilities::Numerical::NoDataIntValue)
        {
          for ( unsigned int i = 0; i<cmbModel_->tableSize(t2zReferTable.first); ++i)
          {
            if (cmbModel_->tableValueAsString(t2zReferTable.first, i, t2zReferTable.second) == mapName)
            {
              cmbModel_->setTableValue(t2zReferTable.first, i, t2zReferTable.second, newMapName);
            }
          }
        }
      }
    }
  }
  // Delete the records related to the CalibratedInputs.HDF
  // Descending sort is used, as the table will get fewer rows while removing records
  std::sort(removeMaps.begin(), removeMaps.end(), greater<double>());
  for ( mbapi::MapsManager::MapID mapId : removeMaps )
  {
    cmbModel_->removeRecordFromTable("GridMapIoTbl", mapId);
  }
}

void CMBProjectWriter::deleteOutputTables()
{
  // Delete Output tables
  for (int i = 0; i < cmbModel_->tableSize("OutputTablesIoTbl"); i++)
  {
    const std::string tableName = cmbModel_->tableValueAsString("OutputTablesIoTbl", i, "TableName");
    cmbModel_->clearTable(tableName);
  }
  cmbModel_->clearTable("OutputTablesFileIoTbl");
  cmbModel_->clearTable("OutputTablesIoTbl");
}

void CMBProjectWriter::generateOutputProject(const QString& timeStamp)
{
  appendTimeStampToCalibratedLithoMaps(timeStamp);
  appendTimeStampToT2ZMaps(timeStamp);
  deleteOutputTables();

  // Finally save the model from memory to file
  cmbModel_->saveModelToProjectFile(cmbModel_->projectFileName());
}

void CMBProjectWriter::setRelevantOutputParameters(const QStringList& activeProperties)
{
  int tableSize = cmbModel_->tableSize("FilterTimeIoTbl");
  for (size_t i = 0; i < tableSize; i++)
  {
    cmbModel_->setTableValue("FilterTimeIoTbl", i, "OutputOption", "None");
  }

  for (int i = 0; i < activeProperties.size(); i++)
  {
    // Mapping to Project3d names if necessary
    if (activeProperties[i] == QString("BulkDensity"))
    {
      cmbModel_->propertyManager().requestPropertyInSnapshots("BulkDensityVec", "SedimentsOnly");
    }
    else if (activeProperties[i] == QString("SonicSlowness"))
    {
      cmbModel_->propertyManager().requestPropertyInSnapshots("SonicVec", "SedimentsOnly");
    }
    else if (activeProperties[i] == QString("Velocity"))
    {
      cmbModel_->propertyManager().requestPropertyInSnapshots("VelocityVec", "SedimentsOnly");
    }
    else if (activeProperties[i] == QString("VRe"))
    {
      cmbModel_->propertyManager().requestPropertyInSnapshots("VrVec", "SedimentsOnly");
    }
    else
    {
      cmbModel_->propertyManager().requestPropertyInSnapshots(activeProperties[i].toStdString(), "SedimentsOnly");
    }
  }

  // Always output the Depth
  cmbModel_->propertyManager().requestPropertyInSnapshots("Depth", "SedimentsPlusBasement");

  cmbModel_->saveModelToProjectFile(cmbModel_->projectFileName());
}

void CMBProjectWriter::setScaling(int scaleX, int scaleY)
{
  cmbModel_->setSubsampling(scaleX, scaleY);
  cmbModel_->saveModelToProjectFile(cmbModel_->projectFileName());
}

} // namespace casaWizard

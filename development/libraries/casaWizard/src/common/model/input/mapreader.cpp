//
// Copyright (C) 2020 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "mapreader.h"

#include "model/lithofractionmap.h"
#include "cmbAPI.h"
#include "MapsManager.h"

#include <stdexcept>

namespace casaWizard
{

MapReader::MapReader() :
  cmbModel_{new mbapi::Model()},
  loaded_{false}
{

}

MapReader::~MapReader()
{
}

LithofractionMap MapReader::getMapData(const std::string& mapName) const
{
  if (!loaded_)
  {
    return LithofractionMap({});
  }

  mbapi::MapsManager& mapsManager = cmbModel_->mapsManager();
  const std::unordered_map<std::string, mbapi::MapsManager::MapID> mapIDs = mapsManager.mapNameIDs();
  const mbapi::MapsManager::MapID mapID = mapIDs.at(mapName);

  std::vector<double> out;
  mapsManager.mapGetValues(mapID, out);

  int numI, numJ;
  mapsManager.mapGetDimensions(mapID, numI, numJ);

  return LithofractionMap(resizeData(out, numI, numJ));
}

bool MapReader::mapExists(const std::string& mapName) const
{
  if (!loaded_)
  {
    return false;
  }

  mbapi::MapsManager& mapsManager = cmbModel_->mapsManager();
  const std::unordered_map<std::string, mbapi::MapsManager::MapID> mapIDs = mapsManager.mapNameIDs();
  try
  {
    mapIDs.at(mapName);
  }
  catch (std::out_of_range)
  {
    return false;
  }

  return true;
}

void MapReader::load(const std::string& projectFile)
{
  loaded_ = (cmbModel_->loadModelFromProjectFile(projectFile.c_str()) == ErrorHandler::ReturnCode::NoError);
}

std::vector<std::vector<double>> MapReader::resizeData(const std::vector<double>& out, const int numI, const int numJ) const
{
  std::vector<std::vector<double>> returnData;
  for (int j = 0; j < numJ; j++)
  {
    std::vector<double> row;
    for (int i = j * numI; i < j * numI + numI; i++)
    {
      row.push_back(out[i]);
    }
    returnData.push_back(row);
  }

  return returnData;
}

}

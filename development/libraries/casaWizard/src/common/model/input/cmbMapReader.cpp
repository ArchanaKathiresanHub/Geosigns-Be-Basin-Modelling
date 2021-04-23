//
// Copyright (C) 2020 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "cmbMapReader.h"

#include "cmbAPI.h"
#include "MapsManager.h"
#include "model/vectorvectormap.h"

#include <stdexcept>

namespace casaWizard
{

CMBMapReader::CMBMapReader() :
  cmbModel_{new mbapi::Model()},
  loaded_{false}
{

}

CMBMapReader::~CMBMapReader()
{
}

VectorVectorMap CMBMapReader::getMapData(const std::string& mapName) const
{
  if (!loaded_)
  {
    return VectorVectorMap({});
  }

  mbapi::MapsManager& mapsManager = cmbModel_->mapsManager();
  const std::unordered_map<std::string, mbapi::MapsManager::MapID> mapIDs = mapsManager.mapNameIDs();
  const mbapi::MapsManager::MapID mapID = mapIDs.at(mapName);

  std::vector<double> out;
  mapsManager.mapGetValues(mapID, out);

  int numI, numJ;
  mapsManager.mapGetDimensions(mapID, numI, numJ);

  return VectorVectorMap(resizeData(out, numI, numJ));
}

bool CMBMapReader::mapExists(const std::string& mapName) const
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

double CMBMapReader::getValue(const double x, const double y, const string& mapName) const
{
  if (!loaded_)
  {
    return Utilities::Numerical::CauldronNoDataValue;
  }

  mbapi::MapsManager& mapsManager = cmbModel_->mapsManager();
  const std::unordered_map<std::string, mbapi::MapsManager::MapID> mapIDs = mapsManager.mapNameIDs();
  const mbapi::MapsManager::MapID mapID = mapIDs.at(mapName);

  return mapsManager.mapGetValue(mapID, x, y);
}

void CMBMapReader::load(const std::string& projectFile)
{
  loaded_ = (cmbModel_->loadModelFromProjectFile(projectFile) == ErrorHandler::ReturnCode::NoError);
}

std::vector<std::vector<double>> CMBMapReader::resizeData(const std::vector<double>& out, const int numI, const int numJ) const
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

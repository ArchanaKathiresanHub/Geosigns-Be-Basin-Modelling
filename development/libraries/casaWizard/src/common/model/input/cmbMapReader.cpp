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
#include "model/VectorVectorMap.h"

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

double CMBMapReader::getValue(const double x, const double y, const std::string& mapName) const
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

VectorVectorMap CMBMapReader::createConstantMap(const double value) const
{
  long numI = 0;
  long numJ = 0;
  cmbModel_->highResGridArealSize(numI, numJ);

  std::vector<std::vector<double>> constantData;
  for (int j = 0; j < numJ; j++)
  {
    std::vector<double> row;
    for (int i = j * numI; i < j * numI + numI; i++)
    {
      row.push_back(value);
    }
    constantData.push_back(row);
  }

  return VectorVectorMap(constantData);
}

std::vector<VectorVectorMap> CMBMapReader::getInputLithoMapsInLayer(const int layerIndex) const
{
  if (!loaded_)
  {
    return {};
  }

  std::vector<VectorVectorMap> maps;

  mbapi::StratigraphyManager& stratigraphyManager = cmbModel_->stratigraphyManager();
  std::vector<std::string> lithoNames;
  std::vector<double> lithoPercent;
  std::vector<std::string> lithoPercMap;
  stratigraphyManager.layerLithologiesList(layerIndex, lithoNames, lithoPercent, lithoPercMap);
  for (int i = 0; i<2; ++i)
  {
    if (!Utilities::isValueUndefined(lithoPercent[i]))
    {
      maps.push_back(createConstantMap(lithoPercent[i]));
    }
    else if (!lithoPercMap[i].empty())
    {
      maps.push_back(getMapData(lithoPercMap[i]));
    }
    else if (i==1)
    {
      maps.push_back(maps[0] * -1.0 + 100.0);
    }
  }
  maps.push_back(maps[0]*-1.0 + maps[1]*-1.0 + 100.0);

  return maps;
}

std::vector<VectorVectorMap> CMBMapReader::getOptimizedLithoMapsInLayer(const int layerIndex) const
{
  std::vector<VectorVectorMap> lithologyMaps;
  lithologyMaps.push_back(getMapData(std::to_string(layerIndex) + "_percent_1"));

  if (mapExists(std::to_string(layerIndex) + "_percent_2"))
  {
    lithologyMaps.push_back(getMapData(std::to_string(layerIndex) + "_percent_2"));
  }
  else
  {
    lithologyMaps.push_back(lithologyMaps[0] * -1.0 + 100);
  }

  lithologyMaps.push_back((lithologyMaps[0] + lithologyMaps[1]) * (-1.0) + 100);

  return lithologyMaps;
}

bool CMBMapReader::checkIfPointIsInLayer(const double x, const double y, const double z, const std::string& layerName) const
{
  return cmbModel_->checkPoint(x, y, z, layerName);
}

void CMBMapReader::getMapDimensions(double& xMin, double& xMax, double& yMin, double& yMax, long& numI, long& numJ)
{
  cmbModel_->highResOrigin(xMin, yMin);
  double dimX = 0.0;
  double dimY = 0.0;
  cmbModel_->arealSize(dimX, dimY);

  xMax = xMin + dimX;
  yMax = yMin + dimY;

  cmbModel_->highResGridArealSize(numI, numJ);
}
}

//
// Copyright (C) 2020 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include "model/VectorVectorMap.h"

#include <QString>
#include <string>
#include <vector>

namespace mbapi
{
  class Model;
}

namespace casaWizard
{

class MapReader
{
public:
  MapReader() = default;
  virtual ~MapReader() = default;

  virtual VectorVectorMap getMapData(const std::string& mapName) const = 0;
  virtual void load(const std::string& projectFile) = 0;
  virtual bool mapExists(const std::string& mapName) const = 0;
  virtual double getValue(const double x, const double y, const std::string& mapName) const = 0;
  virtual std::vector<VectorVectorMap> getInputLithoMapsInLayer(const int layerIndex) const = 0;
  virtual std::vector<VectorVectorMap> getOptimizedLithoMapsInLayer(const int layerIndex) const = 0;
  virtual VectorVectorMap getTopCrustHeatProductionMap() const = 0;
  virtual std::string getTopCrustHeatProductionMapName() const = 0;
  virtual bool checkIfPointIsInLayer(const double x, const double y, const double z, const std::string& layerName) const = 0;
  virtual void getHighResolutionMapDimensions(double& xMin, double& xMax, double& yMin, double& yMax, long& numI, long& numJ) = 0;
  virtual void getHighResolutionMapDimensions(double& xMin, double& xMax, double& yMin, double& yMax) = 0;
};

}

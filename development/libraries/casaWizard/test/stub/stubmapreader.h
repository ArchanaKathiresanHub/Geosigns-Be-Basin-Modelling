//
// Copyright (C) 2020 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include "model/input/mapReader.h"
#include <QVector>

namespace casaWizard
{

class StubMapReader : public MapReader
{
public:
  VectorVectorMap getMapData(const std::string& /*mapName*/) const override {return VectorVectorMap({});}
  void load(const std::string& /*projectFile*/) override {}
  bool mapExists(const std::string& /*mapName*/) const override {return true;}
  double getValue(const double /*x*/, const double /*y*/, const std::string& /*mapName*/) const override {return 5.0;}
  std::vector<VectorVectorMap> getOptimizedLithoMapsInLayer(const int /*layerIndex*/) const override
  {
    return {VectorVectorMap({{1.0, 2.0}, {3.0, 4.0}}), VectorVectorMap({{5.0, 6.0}, {7.0, 8.0}})};
  }
  std::vector<VectorVectorMap> getInputLithoMapsInLayer(const int /*layerIndex*/) const override {return {};}
  VectorVectorMap getTopCrustHeatProductionMap() const override { return VectorVectorMap({{1.0, 2.0}, {3.0, 4.0}}); }
  std::string getTopCrustHeatProductionMapName() const override { return ""; }
  bool checkIfPointIsInLayer(const double /*x*/, const double /*y*/, const double /*z*/, const std::string& /*layerName*/) const override {return false;}
  void getMapDimensions(double& xMin, double& xMax, double& yMin, double& yMax, long& numI, long& numJ) override
  {
    xMin = 0.0;
    xMax = 234.5;
    yMin = 4.0;
    yMax = 835.5;
    numI = 2;
    numJ = 2;
  }
};

} // namespace casaWizard

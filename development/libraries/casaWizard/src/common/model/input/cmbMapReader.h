//
// Copyright (C) 2012-2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include "mapReader.h"

#include <memory>

namespace mbapi
{
  class Model;
}

namespace casaWizard
{

class CMBMapReader : public MapReader
{
public:
  CMBMapReader();
  ~CMBMapReader() override;

  VectorVectorMap getMapData(const std::string& mapName) const override;
  void load(const std::string& projectFile) override;
  bool mapExists(const std::string& mapName) const override;
  double getValue(const double x, const double y, const std::string& mapName) const override;
  std::vector<VectorVectorMap> getInputLithoMapsInLayer(const int layerIndex) const override;
  std::vector<VectorVectorMap> getOptimizedLithoMapsInLayer(const int layerIndex) const override;
  VectorVectorMap getTopCrustHeatProductionMap() const final;
  std::string getTopCrustHeatProductionMapName() const final;
  bool checkIfPointIsInLayer(const double x, const double y, const double z, const std::string& layerName) const override;
  void getHighResolutionMapDimensions(double& xMin, double& xMax, double& yMin, double& yMax, long& numI, long& numJ) override;

private:
  std::vector<std::vector<double> > resizeData(const std::vector<double>& out, const int numI, const int numJ) const;
  VectorVectorMap createConstantMap(const double value) const;

  std::unique_ptr<mbapi::Model> cmbModel_;
  bool loaded_;
};

} // namespace casaWizard

//
// Copyright (C) 2020 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include "model/vectorvectormap.h"

#include <memory>
#include <string>
#include <vector>

namespace mbapi
{
  class Model;
}

namespace casaWizard
{

class CMBMapReader
{
public:
  CMBMapReader();
  ~CMBMapReader();

  VectorVectorMap getMapData(const std::string& mapName) const;
  void load(const std::string& projectFile);
  bool mapExists(const std::string& mapName) const;
  double getValue(const double x, const double y, const std::string& mapName) const;

private:
  std::vector<std::vector<double> > resizeData(const std::vector<double>& out, const int numI, const int numJ) const;

  std::unique_ptr<mbapi::Model> cmbModel_;
  bool loaded_;
};

}

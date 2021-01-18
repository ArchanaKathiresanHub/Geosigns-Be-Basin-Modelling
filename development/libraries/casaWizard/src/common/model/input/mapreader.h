//
// Copyright (C) 2020 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include <string>
#include <vector>
#include <memory>

#include "../lithofractionmap.h"
namespace mbapi
{
  class Model;
}

namespace casaWizard
{

class MapReader
{
public:
  MapReader();
  ~MapReader();

  LithofractionMap getMapData(const std::string& mapName) const;
  void load(const std::string& projectFile);
  bool mapExists(const std::string& mapName) const;

private:
  std::vector<std::vector<double> > resizeData(const std::vector<double>& out, const int numI, const int numJ) const;

  std::unique_ptr<mbapi::Model> cmbModel_;
  bool loaded_;
};

}

//
// Copyright (C) 2013-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include "Interface.h"

#include <string>

namespace modelPseudo1d
{

struct ModelPseudo1dTableProperty
{
  std::string name;
  std::string nameGridMap;
  double value;
  std::string mapName;

  ModelPseudo1dTableProperty(const std::string& name = "", const std::string& nameGridMap = "",
                             const double value = DataAccess::Interface::DefaultUndefinedScalarValue,
                             const std::string& mapName = "") :
    name{name},
    nameGridMap{nameGridMap},
    value{value},
    mapName{mapName}
  {
  }
};

} // namespace modelPseudo1d

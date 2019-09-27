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

  ModelPseudo1dTableProperty(const std::string & name = "", const std::string & nameGridMap = "",
                             const double value = DataAccess::Interface::DefaultUndefinedScalarValue) :
    name{name},
    nameGridMap{nameGridMap},
    value{value}
  {
  }
};

} // namespace modelPseudo1d

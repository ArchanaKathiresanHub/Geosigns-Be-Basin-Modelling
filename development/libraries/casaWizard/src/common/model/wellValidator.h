//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include <memory>
#include <string>

namespace casaWizard
{

class Well;
class CMBMapReader;
class WellValidator
{
public:
  WellValidator(CMBMapReader& mapReader);
  ~WellValidator();

  bool isValid(const Well& well, const std::string& depthGridName) const;

  CMBMapReader& mapReader_;
};

} // namespace casaWizard


//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include <string>

namespace casaWizard
{

enum WellState
{
  valid,
  invalidLocation,
  invalidData
};

class Well;
class CMBMapReader;
class WellValidator
{
public:
  WellValidator(CMBMapReader& mapReader);
  ~WellValidator();

  WellState wellState(const Well& well, const std::string& depthGridName) const;

  CMBMapReader& mapReader_;
};

} // namespace casaWizard


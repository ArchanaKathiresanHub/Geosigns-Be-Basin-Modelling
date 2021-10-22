//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include "lasSectionReader.h"

#include <vector>
#include <string>

namespace casaWizard
{

class LASWellInfoSectionReader : public LASSectionReader
{
public:
  explicit LASWellInfoSectionReader(const std::vector<std::string>& section, WellData& welldata, ImportOptions& importOptions);
  void readSection() final;

private:
};

} // namespace casaWizard


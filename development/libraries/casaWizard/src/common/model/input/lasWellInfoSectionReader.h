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
  explicit LASWellInfoSectionReader(const std::vector<std::string>& section, WellData& welldata, ImportOptionsLAS& importOptions);
  void readSection() final;

private:
  int getCoordinatePriority(const std::string& coordinateName) const;
};

} // namespace casaWizard


//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include "lasSectionReader.h"

namespace casaWizard
{

class LASCurveInfoSectionReader : public LASSectionReader
{
public:
  LASCurveInfoSectionReader(const std::vector<std::string>& section, WellData& welldata, ImportOptionsLAS& importOptions);

  void readSection() final;

private:
  bool curveInfoLineInvalid(const std::vector<std::string>& splitLine);
  void validateSection();
};

} // namespace casaWizard


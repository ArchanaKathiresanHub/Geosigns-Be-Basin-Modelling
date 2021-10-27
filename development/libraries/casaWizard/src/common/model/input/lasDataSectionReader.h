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

class LASDataSectionReader : public LASSectionReader
{
public:
  LASDataSectionReader(const std::vector<std::string>& section, WellData& welldata, ImportOptions& importOptions);
  void readSection() final;

private:
  void addDepth(const double depth, const int counter);
  void addValue(const double value, const int counter);
  void initializeValuesAndDepths();
  void readLine(const std::string& line);
  void readColumn(const std::vector<std::string>& splitLine, const int column, int& counter);
  void saveToWellData();
  std::vector<std::string> splitDataLine(const std::string& line) const;
  void unwrapSection();

  std::vector<std::vector<double>> values_;
  std::vector<std::vector<double>> depths_;
  std::vector<std::string> dataSection_;
};

} // namespace casaWizard



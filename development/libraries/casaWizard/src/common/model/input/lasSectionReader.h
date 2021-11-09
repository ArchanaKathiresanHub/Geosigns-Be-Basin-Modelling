//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include <vector>
#include <string>

namespace casaWizard
{

struct WellData;
struct ImportOptionsLAS;

class LASSectionReader
{
public:
  LASSectionReader(const std::vector<std::string>& section, WellData& welldata, ImportOptionsLAS& importOptions);
  virtual ~LASSectionReader() = default;
  virtual void readSection() = 0;

protected:
  std::vector<std::string> splitLASLine(const std::string& line) const;
  bool lineInvalid(const std::vector<std::string>& splitLine) const;
  const std::vector<std::string>& section_;
  WellData& welldata_;
  ImportOptionsLAS& importOptions_;

private:
  std::vector<std::string> splitDataLine(const std::string& line) const;
  std::string trim(const std::string& input) const;
  std::string trimRight(const std::string& input) const;
  std::string trimLeft(const std::string& input) const;
};

} // namespace casaWizard

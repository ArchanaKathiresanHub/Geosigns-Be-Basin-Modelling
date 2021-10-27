//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "lasVersionInfoSectionReader.h"
#include "model/input/importOptions.h"

#include <stdexcept>


namespace casaWizard
{

LASVersionInfoSectionReader::LASVersionInfoSectionReader(const std::vector<std::string>& section, WellData& welldata, ImportOptions& importOptions) :
  LASSectionReader{section, welldata, importOptions}
{

}

void LASVersionInfoSectionReader::readSection()
{
  if (section_.size() < 2)
  {
    throw std::runtime_error("Empty Version Info Section (~V).");
  }
  bool versionFound = false;
  bool wrappingFound = false;
  for (int i = 1; i < section_.size(); i++)
  {
    const std::vector<std::string> splitLine = splitLASLine(section_[i]);
    if (splitLine[0] == "VERS")
    {
      versionFound = true;
      double version = std::stod(splitLine[2]);
      if (version != 2.0)
      {
        throw std::runtime_error("Wrong LAS version, only 2.0 is supported.");
      }
    }
    if (splitLine[0] == "WRAP")
    {
      wrappingFound = true;
      importOptions_.wrapped = splitLine[2] == "YES";
    }
  }

  if (!versionFound)
  {
    throw std::runtime_error("No LAS version detected, only version 2.0 is supported.");
  }
  if (!wrappingFound)
  {
    throw std::runtime_error("No wrapping specification detected.");
  }

}

} // namespace casaWizard

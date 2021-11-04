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
  bool wrappingFound = false;
  for (int i = 1; i < section_.size(); i++)
  {
    const std::vector<std::string> splitLine = splitLASLine(section_[i]);

    if (splitLine.empty())
    {
      throw std::runtime_error("Invalid line in Version Info Section (~V).");
    }

    if (splitLine[0] == "VERS")
    {
      importOptions_.lasVersion = std::stod(splitLine[2]);
    }
    if (splitLine[0] == "WRAP")
    {
      wrappingFound = true;
      importOptions_.wrapped = splitLine[2] == "YES";
    }
  }

  if (importOptions_.lasVersion < 0)
  {
    throw std::runtime_error("No LAS version detected.");
  }
  if (!wrappingFound)
  {
    throw std::runtime_error("No wrapping specification detected.");
  }

}

} // namespace casaWizard

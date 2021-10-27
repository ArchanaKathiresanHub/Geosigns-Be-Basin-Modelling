//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "lasWellInfoSectionReader.h"

#include "model/input/importOptions.h"
#include "model/wellData.h"

namespace casaWizard
{

LASWellInfoSectionReader::LASWellInfoSectionReader(const std::vector<std::string>& section, WellData& welldata, ImportOptions& importOptions) :
  LASSectionReader{section, welldata, importOptions}
{
}

void LASWellInfoSectionReader::readSection()
{
  bool xCoordinateFound = false;
  bool yCoordinateFound = false;
  bool wellNameFound = false;

  for (unsigned int i = 1; i < section_.size(); i++)
  {
    std::vector<std::string> splitLine = splitLASLine(section_[i]);
    if (lineInvalid(splitLine))
    {
      continue;
    }

    if (splitLine[0] == "WELL")
    {
      welldata_.wellName_ = QString::fromStdString(splitLine[2]);
      wellNameFound = true;
    }
    if (splitLine[0].find("X") == 0)
    {
      welldata_.xCoord_ = std::stod(splitLine[2]) * importOptions_.userPropertyNameToUnitConversion.value(importOptions_.depthUserPropertyName, 1.0);
      xCoordinateFound = true;
    }
    if (splitLine[0].find("Y") == 0)
    {
      welldata_.yCoord_ = std::stod(splitLine[2]) * importOptions_.userPropertyNameToUnitConversion.value(importOptions_.depthUserPropertyName, 1.0);
      yCoordinateFound = true;
    }
    if (splitLine[0] == "EREF" || splitLine[0] == "ELEV")
    {
      importOptions_.elevationCorrection = std::stod(splitLine[2]);
      importOptions_.elevationCorrectionUnit = QString::fromStdString(splitLine[1]);
    }
    if (splitLine[0] == "NULL")
    {
      importOptions_.undefinedValue = std::stod(splitLine[2]);
    }
  }

  if (!wellNameFound)
  {
    throw std::runtime_error("Invalid LAS-file: No well name is provided.");
  }
  if (!xCoordinateFound)
  {
    throw std::runtime_error("Invalid LAS-file: No x coordinate is provided.");
  }
  if (!yCoordinateFound)
  {
    throw std::runtime_error("Invalid LAS-file: No y coordinate is provided.");
  }
}

} // namespace casaWizard


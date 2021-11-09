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

LASWellInfoSectionReader::LASWellInfoSectionReader(const std::vector<std::string>& section, WellData& welldata, ImportOptionsLAS& importOptions) :
  LASSectionReader{section, welldata, importOptions}
{
}

void LASWellInfoSectionReader::readSection()
{
  int xCoordinateHighestPriorityFound = INT_MAX;
  int yCoordinateHighestPriorityFound = INT_MAX;
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
    if (splitLine[0] == "XCOORD" || splitLine[0] == "XWELL" || splitLine[0] == "X")
    {
      if (getCoordinatePriority(splitLine[0]) < xCoordinateHighestPriorityFound)
      {
        welldata_.xCoord_ = std::stod(splitLine[2]);
        xCoordinateHighestPriorityFound = getCoordinatePriority(splitLine[0]);
      }
    }
    if (splitLine[0] == "YCOORD" || splitLine[0] == "YWELL" || splitLine[0] == "Y")
    {
      if (getCoordinatePriority(splitLine[0]) < yCoordinateHighestPriorityFound)
      {
        welldata_.yCoord_ = std::stod(splitLine[2]);
        yCoordinateHighestPriorityFound = getCoordinatePriority(splitLine[0]);
      }
    }
    if (splitLine[0] == "ELEV")
    {
      importOptions_.elevationCorrection = std::stod(splitLine[2]);
      importOptions_.elevationCorrectionUnit = QString::fromStdString(splitLine[1]);
    }
    if (splitLine[0] == "EREF")
    {
      importOptions_.referenceCorrection = std::stod(splitLine[2]);
      importOptions_.referenceCorrectionUnit = QString::fromStdString(splitLine[1]);
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
}

int LASWellInfoSectionReader::getCoordinatePriority(const std::string& coordinateName) const
{
  if (coordinateName.find("COORD") != std::string::npos)
  {
    return 1;
  }
  else if (coordinateName.find("WELL") != std::string::npos)
  {
    return 2;
  }
  else
  {
    return 3;
  }
}

} // namespace casaWizard


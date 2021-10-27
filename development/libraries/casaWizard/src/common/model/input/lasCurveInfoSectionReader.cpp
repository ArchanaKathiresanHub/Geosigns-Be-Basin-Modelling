//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "lasCurveInfoSectionReader.h"

#include "model/input/importOptions.h"
#include "model/wellData.h"

namespace casaWizard
{

LASCurveInfoSectionReader::LASCurveInfoSectionReader(const std::vector<std::string>& section, WellData& welldata, ImportOptions& importOptions) :
  LASSectionReader{section, welldata, importOptions}
{
}

void LASCurveInfoSectionReader::readSection()
{
  validateSection();

  for (unsigned int i = 1; i < section_.size(); i++)
  {
    // Line layout:
    // PropertyUserName          .UNIT    : description
    //        0                    1             2
    std::vector<std::string> splitLine = splitLASLine(section_[i]);
    if (curveInfoLineInvalid(splitLine))
    {
      throw std::runtime_error("Invalid line in the Curve Info Section (~C).");
    }
    if (splitLine[0] == importOptions_.depthUserPropertyName.toStdString())
    {
      importOptions_.depthColumn = i-1;
      continue;
    }

    welldata_.calibrationTargetVarsUserName_.push_back(QString::fromStdString(splitLine[0]));
    welldata_.units_.push_back(QString::fromStdString(splitLine[1]));
  }

  if (importOptions_.depthUserPropertyName == "")
  {
    importOptions_.depthUserPropertyName = welldata_.calibrationTargetVarsUserName_[0];
  }

  welldata_.nCalibrationTargetVars_ = welldata_.calibrationTargetVarsUserName_.size();
  welldata_.nDataPerTargetVar_.resize(welldata_.nCalibrationTargetVars_);
  std::fill(welldata_.nDataPerTargetVar_.begin(), welldata_.nDataPerTargetVar_.end(), 0);
}

void LASCurveInfoSectionReader::validateSection()
{
  if (section_.size() < 2)
  {
    throw std::runtime_error("No data was found in the curve information section (~C).");
  }
}

bool LASCurveInfoSectionReader::curveInfoLineInvalid(const std::vector<std::string>& splitLine)
{
  return (splitLine.size() != 4 || splitLine[0].empty());
}

} // namespace casaWizard


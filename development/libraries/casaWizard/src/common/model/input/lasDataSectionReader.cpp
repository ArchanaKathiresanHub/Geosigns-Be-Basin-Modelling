//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "lasDataSectionReader.h"
#include "model/input/importOptions.h"
#include "model/wellData.h"

#include <cmath>
#include <sstream>

namespace casaWizard
{

LASDataSectionReader::LASDataSectionReader(const std::vector<std::string>& section, WellData& welldata, ImportOptions& importOptions) :
  LASSectionReader{section, welldata, importOptions}
{

}

void LASDataSectionReader::readSection()
{
  initializeValuesAndDepths();

  for (const std::string& line : section_)
  {
    readLine(line);
  }

  saveToWellData();
}

void LASDataSectionReader::initializeValuesAndDepths()
{
  for (const QString& userName : welldata_.calibrationTargetVarsUserName_)
  {
    if (userName != importOptions_.depthUserPropertyName)
    {
      values_.push_back({});
      depths_.push_back({});
    }
  }
}

void LASDataSectionReader::readLine(const std::string& line)
{
  const std::vector<std::string> splittedLine = splitDataLine(line);

  if (line.find("~") == 0 || splittedLine.empty())
  {
    return;
  }
  if (splittedLine.size() != welldata_.calibrationTargetVarsUserName_.size() + 1)
  {
    throw std::runtime_error("Invalid line in the LAS data section (~A)");
  }

  int counter = 0;
  for (int column = 0; column < splittedLine.size(); column++)
  {
    if (column != importOptions_.depthColumn)
    {
      readColumn(splittedLine, column, counter);
    }
  }
}

void LASDataSectionReader::saveToWellData()
{
  for (unsigned int i = 0; i < values_.size(); i++)
  {
    for (unsigned int j = 0; j < values_[i].size(); j++)
    {
      welldata_.calibrationTargetValues_.push_back(values_[i][j]);
      welldata_.depth_.push_back(depths_[i][j]);
      welldata_.calibrationTargetStdDeviation_.push_back(0.0);
    }
  }
}

std::vector<std::string> LASDataSectionReader::splitDataLine(const std::string& line) const
{
  std::vector<std::string> splittedLine;
  std::stringstream ss(line);
  std::string splittedString;
  while (ss >> splittedString)
  {
    splittedLine.push_back(splittedString);
  }

  return splittedLine;
}

void LASDataSectionReader::readColumn(const std::vector<std::string>& splittedLine, const int column, int& counter)
{
  const double value = std::stod(splittedLine[column]);
  const double depth = std::stod(splittedLine[importOptions_.depthColumn]);

  if (std::fabs(value - importOptions_.undefinedValue) < 1e-5 ||
      std::fabs(depth - importOptions_.undefinedValue) < 1e-5 )
  {
    counter++;
    return;
  }

  addDepth(depth, counter);
  addValue(value, counter);

  welldata_.nDataPerTargetVar_[counter]++;
  counter++;
}

void LASDataSectionReader::addDepth(const double depth, const int counter)
{
  const double depthUnitConversion = importOptions_.userPropertyNameToUnitConversion.value(importOptions_.depthUserPropertyName, 1.0);
  if (importOptions_.correctForElevation)
  {
    depths_[counter].push_back((depth - importOptions_.elevationCorrection) * depthUnitConversion);
  }
  else
  {
    depths_[counter].push_back(depth * depthUnitConversion);
  }
}

void LASDataSectionReader::addValue(const double value, const int counter)
{
  const double unitConversion = importOptions_.userPropertyNameToUnitConversion.value(welldata_.calibrationTargetVarsUserName_[counter], 1.0);
  values_[counter].push_back(value * unitConversion);
}








} // namespace casaWizard


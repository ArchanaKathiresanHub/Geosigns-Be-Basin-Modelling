//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "lasDataSectionReader.h"

#include "defaultUnitConversions.h"
#include "model/input/importOptions.h"
#include "model/wellData.h"

#include <cmath>
#include <sstream>

namespace casaWizard
{

LASDataSectionReader::LASDataSectionReader(const std::vector<std::string>& section, WellData& welldata, ImportOptionsLAS& importOptions) :
  LASSectionReader{section, welldata, importOptions},
  dataSection_{section}
{

}

void LASDataSectionReader::readSection()
{
  initializeValuesAndDepths();

  if (importOptions_.wrapped)
  {
    unwrapSection();
  }
  for (const std::string& line : dataSection_)
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

void LASDataSectionReader::unwrapSection()
{
  std::vector<std::string> unwrappedSection = {""};
  int propertyCounter = 0;
  for (int i = 1; i < dataSection_.size(); i++)
  {
    const std::vector<std::string>& splitLine = splitDataLine(dataSection_[i]);
    if (propertyCounter == welldata_.calibrationTargetVarsUserName_.size() + 1)
    {
      propertyCounter = 0;
      unwrappedSection.push_back("");
    }

    for (const std::string& value : splitLine)
    {
      unwrappedSection.back() += " " + value;
      propertyCounter++;
    }
  }

  dataSection_ = unwrappedSection;
}

void LASDataSectionReader::readLine(const std::string& line)
{
  const std::vector<std::string> splitLine = splitDataLine(line);

  if (line.find("~") == 0 || splitLine.empty())
  {
    return;
  }
  if (splitLine.size() != welldata_.calibrationTargetVarsUserName_.size() + 1)
  {
    throw std::runtime_error("Invalid line in the LAS data section (~A)");
  }

  int counter = 0;
  for (int column = 0; column < splitLine.size(); column++)
  {
    if (column != importOptions_.depthColumn)
    {
      readColumn(splitLine, column, counter);
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
  std::vector<std::string> splitLine;
  std::stringstream ss(line);
  std::string splitString;
  while (ss >> splitString)
  {
    splitLine.push_back(splitString);
  }

  return splitLine;
}

void LASDataSectionReader::readColumn(const std::vector<std::string>& splitLine, const int column, int& counter)
{
  const double value = std::stod(splitLine[column]);
  const double depth = std::stod(splitLine[importOptions_.depthColumn]);

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
    const double elevationUnitConversion = defaultUnitConversions.value(importOptions_.elevationCorrectionUnit, depthUnitConversion);
    const double referenceUnitConversion = defaultUnitConversions.value(importOptions_.referenceCorrectionUnit, depthUnitConversion);
    depths_[counter].push_back(depth * depthUnitConversion - importOptions_.elevationCorrection * elevationUnitConversion - importOptions_.referenceCorrection * referenceUnitConversion);
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


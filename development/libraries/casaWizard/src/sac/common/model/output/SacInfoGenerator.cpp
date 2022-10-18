//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "SacInfoGenerator.h"

#include "model/input/projectReader.h"
#include "model/input/cmbMapReader.h"
#include "model/objectiveFunctionValue.h"
#include "model/SacScenario.h"
#include "model/MapsManager.h"

#include <iomanip>
#include <sstream>
namespace casaWizard
{

namespace sac
{

SacInfoGenerator::SacInfoGenerator(ProjectReader& projectReader) :
  casaWizard::InfoGenerator(),
  m_projectReader{projectReader}
{}

void SacInfoGenerator::loadProjectReader(const std::string& projectFileLocation)
{
  m_projectReader.load(QString::fromStdString(projectFileLocation) + "/" + scenario().project3dFilename());
}

void SacInfoGenerator::addGeneralSettingsSection()
{
  addSectionSeparator();
  addHeader("General Settings");
  addRunMode();
  addOption("Cluster", scenario().clusterName().toStdString());
}

void SacInfoGenerator::addRunMode()
{
  if (scenario().applicationName() == "fastcauldron \"-itcoupled\"")
  {
    addOption("Run mode", "Iteratively Coupled");
  }
  else if (scenario().applicationName() == "fastcauldron \"-temperature\"")
  {
    addOption("Run mode", "Hydrostatic");
  }
}

void SacInfoGenerator::addInputTabSection()
{
  addSectionSeparator();
  addHeader("Input Tab");
  addFormationInfo();
  addObjectiveFunction();
}

void SacInfoGenerator::addFormationInfo()
{
  const QStringList layerNames = m_projectReader.layerNames();
  const QStringList mapNames = m_projectReader.mapNames();

  int layerID = 0;
  for (const QString& layerName: layerNames)
  {
    bool includedInMapOptimization = false;
    for (const QString& mapName : mapNames)
    {

      QString test = QString::fromStdString(std::to_string(layerID)) + "_percent";
      if (mapName.startsWith(test))
      {
        includedInMapOptimization = true;
        break;
      }
    }

    addOption("Formation " + layerName.toStdString(), includedInMapOptimization ? "INCLUDED" : "EXCLUDED");

    layerID++;
  }
}

void SacInfoGenerator::addObjectiveFunction()
{
  addHeader("Objective Function");  
  QVector<ObjectiveFunctionValue> objectiveFunctionValues = scenario().objectiveFunctionManager().values();

  std::sort(objectiveFunctionValues.begin(), objectiveFunctionValues.end(), [](const ObjectiveFunctionValue& a, const ObjectiveFunctionValue& b)
  {
    return a.variableUserName().toStdString() < b.variableUserName().toStdString();
  });

  for (const ObjectiveFunctionValue& value : objectiveFunctionValues)
  {
    std::string optionValue = "Absolute error = " + doubleToFormattedString(value.absoluteError()) + ", Relative error = " + doubleToFormattedString(value.relativeError());
    if (value.enabled())
    {
      addOption(value.variableUserName().toStdString(), optionValue);
    }
  }
}

void SacInfoGenerator::addMapsGenerationSection()
{
  addSectionSeparator();
  addHeader("Map Generation");
  addInterpolation();
  addSmoothing();
}

void SacInfoGenerator::addInterpolation()
{
  std::string interpolationOption;
  if (mapsManager().interpolationMethod() == 0)
  {
    interpolationOption = "Inverse Distance Weighting, P = " + std::to_string(mapsManager().pIDW());
  }
  else if (mapsManager().interpolationMethod() == 1)
  {
    interpolationOption = "Natural Neighbor";
  }
  addOption("Interpolation", interpolationOption);
}

void SacInfoGenerator::addSmoothing()
{
  std::string smoothingOption = "";
  switch (mapsManager().smoothingOption())
  {
    case 0:
      smoothingOption = "None";
      break;
    case 1:
      smoothingOption = "Gaussian, Radius = " + std::to_string(mapsManager().radiusSmoothing());
      break;
    case 2:
      smoothingOption = "Moving Average, Radius = " + std::to_string(mapsManager().radiusSmoothing());
      break;
  }
  addOption("Smoothing", smoothingOption);
}

void SacInfoGenerator::addWellsSection()
{
  addSectionSeparator();
  addHeader("Wells");
  const casaWizard::CalibrationTargetManager& manager = scenario().calibrationTargetManager();
  for (const casaWizard::Well* well : manager.wells())
  {
    addWellInfo(well);
  }
}

void SacInfoGenerator::addWellInfo(const casaWizard::Well* well)
{
  std::string optionValue = "";
  if (well->isActive() && !well->isExcluded())
  {
    optionValue += "INCLUDED";
  }
  else
  {
    optionValue = "EXCLUDED";
  }

  addOption(well->name().toStdString() + ", " + doubleToFormattedString(well->x()) + ", " + doubleToFormattedString(well->y()), optionValue);
}

std::string SacInfoGenerator::doubleToFormattedString(const double inputDouble)
{
  std::stringstream stream;
  stream << std::fixed << std::setprecision(2) << inputDouble;
  return stream.str();
}

const ProjectReader& SacInfoGenerator::projectReader() const
{
  return m_projectReader;
}

} // namespace sac

} // namespace casaWizard

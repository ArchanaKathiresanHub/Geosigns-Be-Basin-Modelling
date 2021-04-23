//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "SACInfoGenerator.h"

#include "model/sacScenario.h"
#include "model/input/projectReader.h"
#include "model/input/cmbMapReader.h"
#include "model/objectiveFunctionValue.h"

#include <iomanip>
#include <sstream>
namespace casaWizard
{

namespace sac
{

SACInfoGenerator::SACInfoGenerator(const SACScenario& scenario, ProjectReader& projectReader) :
  casaWizard::InfoGenerator(),
  projectReader_{projectReader},
  scenario_{scenario}
{

}

void SACInfoGenerator::loadProjectReader(const std::string& projectFileLocation)
{
  projectReader_.load(QString::fromStdString(projectFileLocation) + "/" + scenario().project3dFilename());
}

void SACInfoGenerator::addGeneralSettingsSection()
{
  addSectionSeparator();
  addHeader("General Settings");
  addRunMode();
  addOption("Cluster", scenario_.clusterName().toStdString());
}

void SACInfoGenerator::addRunMode()
{
  if (scenario_.applicationName() == "fastcauldron \"-itcoupled\"")
  {
    addOption("Run mode", "Iteratively Coupled");
  }
  else if (scenario_.applicationName() == "fastcauldron \"-temperature\"")
  {
    addOption("Run mode", "Hydrostatic");
  }
}

void SACInfoGenerator::addInputTabSection()
{
  addSectionSeparator();
  addHeader("Input Tab");
  addFormationInfo();
  addObjectiveFunction();
}

void SACInfoGenerator::addFormationInfo()
{
  const QStringList layerNames = projectReader_.layerNames();
  const QStringList mapNames = projectReader_.mapNames();

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

void SACInfoGenerator::addObjectiveFunction()
{
  addHeader("Objective Function");
  const casaWizard::CalibrationTargetManager& manager = scenario_.calibrationTargetManager();
  QVector<ObjectiveFunctionValue> objectiveFunctionValues = manager.objectiveFunctionManager().values();

  std::sort(objectiveFunctionValues.begin(), objectiveFunctionValues.end(), [](const ObjectiveFunctionValue& a, const ObjectiveFunctionValue& b)
  {
    return a.variable().toStdString() < b.variable().toStdString();
  });

  for (const ObjectiveFunctionValue& value : objectiveFunctionValues)
  {
    std::string optionValue = "Absolute error = " + doubleToFormattedString(value.absoluteError()) + ", Relative error = " + doubleToFormattedString(value.relativeError());
    addOption(value.variable().toStdString(), optionValue);
  }
}

void SACInfoGenerator::addMapsGenerationSection()
{
  addSectionSeparator();
  addHeader("Map Generation");
  addInterpolation();
  addSmoothing();
}

void SACInfoGenerator::addInterpolation()
{
  std::string interpolationOption;
  if (scenario().interpolationMethod() == 0)
  {
    interpolationOption = "Inverse Distance Weighting, P = " + std::to_string(scenario().pIDW());
  }
  else if (scenario().interpolationMethod() == 1)
  {
    interpolationOption = "Natural Neighbor";
  }
  addOption("Interpolation", interpolationOption);
}

void SACInfoGenerator::addSmoothing()
{
  std::string smoothingOption = "";
  switch (scenario().smoothingOption())
  {
    case 0:
      smoothingOption = "None";
      break;
    case 1:
      smoothingOption = "Gaussian, Radius = " + std::to_string(scenario().radiusSmoothing());
      break;
    case 2:
      smoothingOption = "Moving Average, Radius = " + std::to_string(scenario().radiusSmoothing());
      break;
  }
  addOption("Smoothing", smoothingOption);
}

void SACInfoGenerator::addWellsSection()
{
  addSectionSeparator();
  addHeader("Wells");
  const casaWizard::CalibrationTargetManager& manager = scenario_.calibrationTargetManager();
  for (const casaWizard::Well* well : manager.wells())
  {
    addWellInfo(well);
  }
}

void SACInfoGenerator::addWellInfo(const casaWizard::Well* well)
{
  std::string optionValue = "";
  if (well->isActive())
  {
    optionValue += "INCLUDED for CALIBRATION";
    if (!well->isExcluded())
    {
      optionValue += " & MAP GENERATION";
    }
  }
  else
  {
    optionValue = "EXCLUDED";
  }

  addOption(well->name().toStdString() + ", " + doubleToFormattedString(well->x()) + ", " + doubleToFormattedString(well->y()), optionValue);
}


std::string SACInfoGenerator::doubleToFormattedString(const double inputDouble)
{
  std::stringstream stream;
  stream << std::fixed << std::setprecision(2) << inputDouble;
  return stream.str();
}


const SACScenario& SACInfoGenerator::scenario() const
{
  return scenario_;
}

const ProjectReader& SACInfoGenerator::projectReader() const
{
  return projectReader_;
}

} // namespace sac

} // namespace casaWizard

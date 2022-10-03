//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "SacScenario.h"

#include "model/functions/sortedByXWellIndices.h"
#include "model/input/cmbMapReader.h"
#include "model/input/projectReader.h"
#include "model/scenarioReader.h"
#include "model/scenarioWriter.h"

#include "ConstantsMathematics.h"

#include <QDateTime>
#include <QDir>
#include <QFileInfo>

namespace
{
   static int s_scenario_version = 3;
}

namespace casaWizard
{

namespace sac
{

SacScenario::SacScenario(ProjectReader* projectReader) :
  CasaScenario{projectReader},
  m_stateFileNameSAC{"casaStateSAC.txt"},
  m_calibrationDirectory{"calibration_step1"},
  m_mapsManager{},
  m_wellTrajectoryManager{},
  m_activePlots(4, true),
  m_fitRangeToData{false}
{
  m_activePlots[2] = false;
  m_activePlots[3] = false;

  calibrationTargetManager().setShowPropertiesInTable(false);
}

QString SacScenario::stateFileNameSAC() const
{
  return m_stateFileNameSAC;
}

QString SacScenario::calibrationDirectory() const
{
  return workingDirectory() + "/" + m_calibrationDirectory;
}

QString SacScenario::optimizedProjectDirectory() const
{
  return calibrationDirectory() + "/ThreeDFromOneD/";
}

bool SacScenario::fitRangeToData() const
{
  return m_fitRangeToData;
}

void SacScenario::setFitRangeToData(const bool fitRangeToData)
{
  m_fitRangeToData = fitRangeToData;
}

WellTrajectoryManager& SacScenario::wellTrajectoryManager()
{
  return m_wellTrajectoryManager;
}

const WellTrajectoryManager& SacScenario::wellTrajectoryManager() const
{
  return m_wellTrajectoryManager;
}

MapsManager& SacScenario::mapsManager()
{
  return m_mapsManager;
}

const MapsManager&SacScenario::mapsManager() const
{
  return m_mapsManager;
}

void SacScenario::writeToFile(ScenarioWriter& writer) const
{
  CasaScenario::writeToFile(writer);
  writer.writeValue("SACScenarioVersion", s_scenario_version);

  m_wellTrajectoryManager.writeToFile(writer);
  m_mapsManager.writeToFile(writer);
}

void SacScenario::readFromFile(const ScenarioReader& reader)
{
  CasaScenario::readFromFile(reader);

  m_wellTrajectoryManager.readFromFile(reader);
  m_mapsManager.readFromFile(reader);
}

void SacScenario::clear()
{
  CasaScenario::clear();
  m_wellTrajectoryManager.clear();
}

QString SacScenario::iterationDirName() const
{
  const QString iterationPath = calibrationDirectory() + "/" + runLocation();

  const QDir dir(iterationPath);
  QDateTime dateTime = QFileInfo(dir.path()).lastModified();

  QString dirName{""};
  for (const QString& entry : dir.entryList())
  {
    if (entry.toStdString().find("Iteration_") == 0)
    {
      const QFileInfo info{dir.path() + "/" + entry};
      if (info.lastModified() >= dateTime)
      {
        dateTime = info.lastModified();
        dirName = entry;
      }
    }
  }

  return dirName;
}

QVector<bool> SacScenario::activePlots() const
{
  return m_activePlots;
}

void SacScenario::setActivePlots(const QVector<bool>& activePlots)
{
  if (activePlots.size() == 4)
  {
    m_activePlots = activePlots;
  }
}

void SacScenario::wellPrepToSAC()
{
  calibrationTargetManager().appendFrom(calibrationTargetManagerWellPrep());
  updateObjectiveFunctionFromTargets();
  m_wellTrajectoryManager.updateWellTrajectories(calibrationTargetManager());

  if (!project3dPath().isEmpty())
  {
    updateWellsForProject3D();
  }
}

void SacScenario::updateWellsForProject3D()
{
  calibrationTargetManager().disableInvalidWells(project3dPath().toStdString(), projectReader().getDepthGridName(0).toStdString());
  calibrationTargetManager().setWellHasDataInLayer(project3dPath().toStdString(), projectReader().layerNames());
}

QVector<int> SacScenario::getIncludedWellIndicesFromSelectedWells(const QVector<int>& selectedWellIndices)
{
  return m_mapsManager.transformToActiveAndIncluded(selectedWellIndices, calibrationTargetManager().getExcludedWellsFromActiveWells());
}

bool SacScenario::hasOptimizedSuccessfully(const int caseIndex)
{
  const QVector<int> sortedIndices = casaWizard::functions::sortedByXYWellIndices(calibrationTargetManager().activeWells());  
  const QString caseFolderNumber = QString::number(sortedIndices.indexOf(caseIndex) + 1);
  QFile successFile(calibrationDirectory() + "/" + runLocation() + "/" + iterationDirName() + "/Case_" + caseFolderNumber + "/Stage_0.sh.success");
  return successFile.exists();
}

bool SacScenario::openMaps(MapReader& mapReader, const int layerID) const
{
  QDir threeDFromOneD = optimizedProjectDirectory();
  if (project3dFilename() == "" || !threeDFromOneD.exists())
  {
    return false;
  }

  mapReader.load((optimizedProjectDirectory() + project3dFilename()).toStdString());
  if (!mapReader.mapExists(std::to_string(layerID) + "_percent_1"))
  {
    return false;
  }

  return true;
}

} // namespace sac

} // namespace casaWizard

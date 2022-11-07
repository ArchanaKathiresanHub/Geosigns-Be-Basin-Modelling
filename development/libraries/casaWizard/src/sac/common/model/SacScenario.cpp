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
#include "model/SacMapsManager.h"
#include "model/scenarioReader.h"
#include "model/scenarioWriter.h"
#include "model/logger.h"

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
  m_wellTrajectoryManager{},
  m_activePlots(4, true),
  m_fitRangeToData{false},
  m_showSurfaceLines{true}
{
  m_activePlots[2] = false;
  m_activePlots[3] = false;

  calibrationTargetManager().setShowPropertiesInTable(false);
}

void SacScenario::writeToFile(ScenarioWriter& writer) const
{
  CasaScenario::writeToFile(writer);
  writer.writeValue("SACScenarioVersion", s_scenario_version);
  m_wellTrajectoryManager.writeToFile(writer);
}

void SacScenario::readFromFile(const ScenarioReader& reader)
{
  CasaScenario::readFromFile(reader);
  m_wellTrajectoryManager.readFromFile(reader);
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
   return mapsManager().transformToActiveAndIncluded(selectedWellIndices, calibrationTargetManager().getExcludedWellsFromActiveWells());
}

void SacScenario::setCalibrationTargetsBasedOnObjectiveFunctions()
{
   const QStringList userNames = objectiveFunctionManager().enabledVariablesUserNames();
   for (const Well* well : calibrationTargetManager().wells() )
   {
      if ( !well->isInvalid() )
      {
         for (const QString& property : calibrationTargetManager().getPropertyUserNamesForWell(well->id()) )
         {
            if (userNames.indexOf(property) != -1)
            {
               if (!well->hasActiveProperties()) //only switch state if its currently doesn't have active properties
               {
                  calibrationTargetManager().setWellHasActiveProperties(true, well->id());
               }
               goto nextWell;
            }
         }

         if (well->hasActiveProperties()) //only switch state if its currently does have active properties
         {
            calibrationTargetManager().setWellHasActiveProperties(false, well->id());
            Logger::log() << "Warning: " << well->name() << " has currently no active data series to match the current selection and is disabled" << Logger::endl();
         }
      }
      nextWell:;
   }
}

bool SacScenario::hasOptimizedSuccessfully(const int caseIndex)
{
  const QVector<int> sortedIndices = casaWizard::functions::sortedByXYWellIndices(calibrationTargetManager().activeWells());
  const QString caseFolderNumber = QString::number(sortedIndices.indexOf(caseIndex) + 1);
  QFile successFile(calibrationDirectory() + "/" + runLocation() + "/" + iterationDirName() + "/Case_" + caseFolderNumber + "/Stage_0.sh.success");
  return successFile.exists();
}

void SacScenario::exportOptimizedMapsToZycor(const QString& targetPath)
{
   CMBMapReader mapReader;
   mapReader.load((optimizedProjectDirectory() + project3dFilename()).toStdString());
   mapsManager().exportOptimizedMapsToZycor(projectReader(), mapReader, targetPath);
}

bool SacScenario::openMaps(MapReader& mapReader, const QString& mapname) const
{
   QDir threeDFromOneD = optimizedProjectDirectory();
   if (project3dFilename() == "" || !threeDFromOneD.exists())
   {
     return false;
   }

   mapReader.load((optimizedProjectDirectory() + project3dFilename()).toStdString());
   return mapReader.mapExists(mapname.toStdString());
}

void SacScenario::setActivePlots(const QVector<bool>& activePlots)
{
  if (activePlots.size() == 4)
  {
    m_activePlots = activePlots;
  }
}

void SacScenario::setFitRangeToData(const bool fitRangeToData)
{
  m_fitRangeToData = fitRangeToData;
}

void SacScenario::setShowSurfaceLines(const bool showSurfaceLines)
{
   m_showSurfaceLines = showSurfaceLines;
}

QVector<bool> SacScenario::activePlots() const
{
  return m_activePlots;
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

WellTrajectoryManager& SacScenario::wellTrajectoryManager()
{
  return m_wellTrajectoryManager;
}

const WellTrajectoryManager& SacScenario::wellTrajectoryManager() const
{
  return m_wellTrajectoryManager;
}

bool SacScenario::showSurfaceLines() const
{
   return m_showSurfaceLines;
}

} // namespace sac

} // namespace casaWizard

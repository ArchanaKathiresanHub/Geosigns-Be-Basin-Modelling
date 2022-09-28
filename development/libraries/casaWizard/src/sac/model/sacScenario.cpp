//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "sacScenario.h"

#include "model/functions/sortedByXWellIndices.h"
#include "model/input/cmbMapReader.h"
#include "model/input/projectReader.h"
#include "model/scenarioReader.h"
#include "model/scenarioWriter.h"

#include "ConstantsMathematics.h"

#include <QDateTime>
#include <QDir>
#include <QFileInfo>

namespace casaWizard
{

namespace sac
{

const int defaultReferenceSurface{0};
const int defaultt2zNumberCPUs{1};

SACScenario::SACScenario(ProjectReader* projectReader) :
  CasaScenario{projectReader},
  stateFileNameSAC_{"casaStateSAC.txt"},
  calibrationDirectory_{"calibration_step1"},
  lithofractionManager_{},
  mapsManager_{},
  wellTrajectoryManager_{},
  t2zLastSurface_{projectReader->lowestSurfaceWithTWTData()},
  t2zReferenceSurface_{defaultReferenceSurface},
  t2zSubSampling_{1},
  t2zRunOnOriginalProject_{false},
  t2zNumberCPUs_{defaultt2zNumberCPUs},
  activePlots_(4, true),
  showSurfaceLines_{true},
  fitRangeToData_{false}
{
  activePlots_[2] = false;
  activePlots_[3] = false;

  calibrationTargetManager().setShowPropertiesInTable(false);
}

QString SACScenario::stateFileNameSAC() const
{
  return stateFileNameSAC_;
}

QString SACScenario::calibrationDirectory() const
{
  return workingDirectory() + "/" + calibrationDirectory_;
}

QString SACScenario::optimizedProjectDirectory() const
{
  return calibrationDirectory() + "/ThreeDFromOneD/";
}

int SACScenario::t2zReferenceSurface() const
{
  return t2zReferenceSurface_;
}

void SACScenario::setT2zReferenceSurface(int refSurface)
{
  t2zReferenceSurface_ = refSurface;
}

int SACScenario::t2zLastSurface() const
{
  return t2zLastSurface_;
}

int SACScenario::t2zNumberCPUs() const
{
  return t2zNumberCPUs_;
}

void SACScenario::setT2zNumberCPUs(int t2zNumberCPUs)
{
  t2zNumberCPUs_ = t2zNumberCPUs;
}

int SACScenario::t2zSubSampling() const
{
  return t2zSubSampling_;
}

void SACScenario::setT2zSubSampling(int t2zSubSampling)
{
  t2zSubSampling_ = t2zSubSampling;
}

bool SACScenario::t2zRunOnOriginalProject() const
{
  return t2zRunOnOriginalProject_;
}

void SACScenario::setT2zRunOnOriginalProject(bool t2zRunOnOriginalProject)
{
  t2zRunOnOriginalProject_ = t2zRunOnOriginalProject;
}

bool SACScenario::showSurfaceLines() const
{
  return showSurfaceLines_;
}

void SACScenario::setShowSurfaceLines(const bool showSurfaceLines)
{
  showSurfaceLines_ = showSurfaceLines;
}

bool SACScenario::fitRangeToData() const
{
  return fitRangeToData_;
}

void SACScenario::setFitRangeToData(const bool fitRangeToData)
{
  fitRangeToData_ = fitRangeToData;
}

LithofractionManager& SACScenario::lithofractionManager()
{
  return lithofractionManager_;
}

const LithofractionManager& SACScenario::lithofractionManager() const
{
  return lithofractionManager_;
}

WellTrajectoryManager& SACScenario::wellTrajectoryManager()
{
  return wellTrajectoryManager_;
}

const WellTrajectoryManager& SACScenario::wellTrajectoryManager() const
{
  return wellTrajectoryManager_;
}

MapsManager& SACScenario::mapsManager()
{
  return mapsManager_;
}

const MapsManager&SACScenario::mapsManager() const
{
  return mapsManager_;
}

void SACScenario::writeToFile(ScenarioWriter& writer) const
{
  CasaScenario::writeToFile(writer);
  writer.writeValue("SACScenarioVersion", 3);

  writer.writeValue("referenceSurface", t2zReferenceSurface_);
  writer.writeValue("t2zSubSampling", t2zSubSampling_);
  writer.writeValue("t2zRunOnOriginalProject", t2zRunOnOriginalProject_);
  writer.writeValue("t2zNumberOfCPUs", t2zNumberCPUs_);

  lithofractionManager_.writeToFile(writer);
  wellTrajectoryManager_.writeToFile(writer);
  mapsManager_.writeToFile(writer);
}

void SACScenario::readFromFile(const ScenarioReader& reader)
{
  CasaScenario::readFromFile(reader);

  int sacScenarioVersion = reader.readInt("SACScenarioVersion");

  if (sacScenarioVersion > 1)
  {
    t2zSubSampling_ = reader.readInt("t2zSubSampling") > 0 ? reader.readInt("t2zSubSampling") : t2zSubSampling_;
    t2zRunOnOriginalProject_ = reader.readBool("t2zRunOnOriginalProject");
    t2zNumberCPUs_ = reader.readInt("t2zNumberOfCPUs");
  }

  t2zLastSurface_ = projectReader().lowestSurfaceWithTWTData();
  t2zReferenceSurface_ = reader.readInt("referenceSurface");

  lithofractionManager_.readFromFile(reader);
  wellTrajectoryManager_.readFromFile(reader);
  mapsManager_.readFromFile(reader);
}

void SACScenario::clear()
{
  CasaScenario::clear();

  t2zReferenceSurface_ = defaultReferenceSurface;
  t2zSubSampling_ = 1;
  t2zRunOnOriginalProject_ = false;
  t2zNumberCPUs_ = defaultt2zNumberCPUs;

  lithofractionManager_.clear();
  wellTrajectoryManager_.clear();
}

QString SACScenario::iterationDirName() const
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

QVector<bool> SACScenario::activePlots() const
{
  return activePlots_;
}

void SACScenario::setActivePlots(const QVector<bool>& activePlots)
{
  if (activePlots.size() == 4)
  {
    activePlots_ = activePlots;
  }
}

void SACScenario::updateT2zLastSurface()
{
  t2zLastSurface_ = projectReader().lowestSurfaceWithTWTData();
}

QVector<OptimizedLithofraction> SACScenario::getOptimizedLithoFractionsInLayer(const QString& layer) const
{
  const QVector<Lithofraction>& lithofractions = lithofractionManager_.lithofractions();

  QVector<OptimizedLithofraction> optimizedLithoFractionsInLayer;
  for (const Well* well : calibrationTargetManager().activeAndIncludedWells())
  {
    QVector<OptimizedLithofraction> optimizedLithoFractions = lithofractionManager_.optimizedInWell(well->id());
    for (OptimizedLithofraction lithofraction : optimizedLithoFractions)
    {
      if (lithofractions[lithofraction.lithofractionId()].layerName() == layer)
      {
        optimizedLithoFractionsInLayer.push_back(lithofraction);
      }
    }
  }

  return optimizedLithoFractionsInLayer;
}

void SACScenario::getVisualisationData(QVector<OptimizedLithofraction>& optimizedLithoFractions, QVector<const Well*>& activeAndIncludedWells, const QString& activeLayer) const
{
  optimizedLithoFractions = getOptimizedLithoFractionsInLayer(activeLayer);
  activeAndIncludedWells = calibrationTargetManager().activeAndIncludedWells();

  if (optimizedLithoFractions.size() != activeAndIncludedWells.size())
  {
    return; // In this layer the wells have not been optimized
  }

  if (mapsManager().smartGridding())
  {
    for (int i = activeAndIncludedWells.size() - 1 ; i >= 0; i--)
    {
      if (!wellHasDataInActiveLayer(activeAndIncludedWells[i], activeLayer))
      {
        optimizedLithoFractions.remove(i);
        activeAndIncludedWells.remove(i);
      }
    }
  }
}

bool SACScenario::wellHasDataInActiveLayer(const Well* well, const QString& activeLayer) const
{
    // If the well has no data (meaning the wells were imported before the changes to the calibrationTargetCreator)
    // just always show all wells
    if (well->hasDataInLayer().empty())
    {
      return true;
    }

    return well->hasDataInLayer()[projectReader().getLayerID(activeLayer.toStdString())];
}

void SACScenario::exportOptimizedLithofractionMapsToZycor(const QString& targetPath)
{
  CMBMapReader mapReader;
  mapReader.load((optimizedProjectDirectory() + project3dFilename()).toStdString());
  mapsManager_.exportOptimizedLithofractionMapsToZycor(projectReader(), mapReader, targetPath);
}

void SACScenario::wellPrepToSAC()
{
  calibrationTargetManager().appendFrom(calibrationTargetManagerWellPrep());
  updateObjectiveFunctionFromTargets();
  wellTrajectoryManager_.updateWellTrajectories(calibrationTargetManager());

  if (!project3dPath().isEmpty())
  {
    updateWellsForProject3D();
  }
}

void SACScenario::updateWellsForProject3D()
{
  calibrationTargetManager().disableInvalidWells(project3dPath().toStdString(), projectReader().getDepthGridName(0).toStdString());
  calibrationTargetManager().setWellHasDataInLayer(project3dPath().toStdString(), projectReader().layerNames());
}

QVector<int> SACScenario::getIncludedWellIndicesFromSelectedWells(const QVector<int>& selectedWellIndices)
{
  return mapsManager_.transformToActiveAndIncluded(selectedWellIndices, calibrationTargetManager().getExcludedWellsFromActiveWells());
}

bool SACScenario::hasOptimizedSuccessfully(const int caseIndex)
{
  const QVector<int> sortedIndices = casaWizard::functions::sortedByXYWellIndices(calibrationTargetManager().activeWells());
  const QString caseFolderNumber = QString::number(sortedIndices.indexOf(caseIndex) + 1);
  QFile successFile(calibrationDirectory() + "/" + runLocation() + "/" + iterationDirName() + "/Case_" + caseFolderNumber + "/Stage_0.sh.success");

  return successFile.exists();
}

QVector<int> SACScenario::getHighlightedWells(const QVector<int>& selectedWells, const QString& activeLayer)
{
  QVector<int> highlightedWells;
  if (mapsManager_.smartGridding())
  {
    // Shift the selected wells to account for wells which are not plotted, since they do
    // not have data in the active layer
    for (int wellIndex : selectedWells)
    {
      int exclusionShift = 0;
      bool excluded = false;
      int excludedWellIndex = 0;
      for (const Well* well : calibrationTargetManager().activeAndIncludedWells())
      {
        if(excludedWellIndex == wellIndex && !wellHasDataInActiveLayer(well, activeLayer))
        {
          excluded = true;
          break;
        }

        if(wellIndex > excludedWellIndex && !wellHasDataInActiveLayer(well, activeLayer))
        {
           exclusionShift++;
        }

        excludedWellIndex++;
      }

      if (!excluded)
      {
        highlightedWells.push_back(wellIndex - exclusionShift);
      }
    }
  }
  else
  {
    highlightedWells = selectedWells;
  }

  return highlightedWells;
}

bool SACScenario::getLithologyTypesAndMaps(const QString& layerName, std::vector<VectorVectorMap>& lithologyMaps, QStringList& lithologyTypes)
{
  const int layerID = projectReader().getLayerID(layerName.toStdString());

  CMBMapReader mapReader;
  const bool optimizedLithomapsAvailable = openMaps(mapReader, layerID);
  if (optimizedLithomapsAvailable)
  {
    lithologyMaps = mapReader.getOptimizedLithoMapsInLayer(layerID);
  }
  else
  {
    mapReader.load(project3dPath().toStdString());
    lithologyMaps = mapReader.getInputLithoMapsInLayer(layerID);
  }

  VectorVectorMap depthMap = mapReader.getMapData(projectReader().getDepthGridName(0).toStdString());
  for (VectorVectorMap& lithologyMap : lithologyMaps)
  {
    lithologyMap.setUndefinedValuesBasedOnReferenceMap(depthMap);
  }

  lithologyTypes = projectReader().lithologyTypesForLayer(layerID);

  return optimizedLithomapsAvailable;
}

bool SACScenario::openMaps(MapReader& mapReader, const int layerID) const
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

std::vector<double> SACScenario::getLithopercentagesOfClosestWell(const double xInKm, const double yInKm, const QString& activeLayer, int& closestWellID) const
{
  std::vector<double> lithofractionsAtPoint;

  double xMin = 0; double xMax = 1; double yMin = 0; double yMax = 1;
  projectReader().domainRange(xMin, xMax, yMin, yMax);
  double smallestDistance2 = ((xMax - xMin) / 30)*((xMax - xMin) / 30);

  int closestLithofractionIndex = -1;
  int currentLithofractionIndex = 0;

  const QVector<OptimizedLithofraction>& lithofractionsInLayer = getOptimizedLithoFractionsInLayer(activeLayer);
  for (const auto& optimizedLithofraction : lithofractionsInLayer)
  {
    const int wellId = optimizedLithofraction.wellId();
    const auto& currentWell = calibrationTargetManager().well(wellId);
    const double distance2 = (currentWell.x() - xInKm * Utilities::Maths::KilometerToMeter) * (currentWell.x() - xInKm * Utilities::Maths::KilometerToMeter) +
                             (currentWell.y() - yInKm * Utilities::Maths::KilometerToMeter) * (currentWell.y() - yInKm * Utilities::Maths::KilometerToMeter);
    const bool isCloser = distance2 < smallestDistance2;
    if (isCloser)
    {
      smallestDistance2 = distance2;
      closestLithofractionIndex = currentLithofractionIndex;
      closestWellID = wellId;
    }
    currentLithofractionIndex++;
  }

  if (closestLithofractionIndex!= -1)
  {
    lithofractionsAtPoint.push_back(lithofractionsInLayer[closestLithofractionIndex].optimizedPercentageFirstComponent());
    lithofractionsAtPoint.push_back(lithofractionsInLayer[closestLithofractionIndex].optimizedPercentageSecondComponent());
    lithofractionsAtPoint.push_back(lithofractionsInLayer[closestLithofractionIndex].optimizedPercentageThirdComponent());
  }

  return lithofractionsAtPoint;
}

} // namespace sac

} // namespace casaWizard

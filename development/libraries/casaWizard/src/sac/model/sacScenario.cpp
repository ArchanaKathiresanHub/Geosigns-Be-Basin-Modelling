#include "sacScenario.h"

#include "model/input/projectReader.h"
#include "model/scenarioReader.h"
#include "model/scenarioWriter.h"

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
  wellTrajectoryManager_{},
  interpolationMethod_{0},
  smoothingOption_{0},
  pIDW_{3},
  radiusSmoothing_{1000},
  smartGridding_{true},
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
}

QString SACScenario::stateFileNameSAC() const
{
  return stateFileNameSAC_;
}

int SACScenario::interpolationMethod() const
{
  return interpolationMethod_;
}

void SACScenario::setInterpolationMethod(int interpolationMethod)
{
  interpolationMethod_ = interpolationMethod;
}

int SACScenario::smoothingOption() const
{
  return smoothingOption_;
}

void SACScenario::setSmoothingOption(int smoothingOption)
{
  smoothingOption_ = smoothingOption;
}

int SACScenario::pIDW() const
{
  return pIDW_;
}

void SACScenario::setPIDW(int pIDW)
{
  pIDW_ = pIDW;
}

int SACScenario::radiusSmoothing() const
{
  return radiusSmoothing_;
}

void SACScenario::setRadiusSmoothing(int radiusSmoothing)
{
  radiusSmoothing_ = radiusSmoothing;
}

bool SACScenario::smartGridding() const
{
  return smartGridding_;
}

void SACScenario::setSmartGridding(bool smartGridding)
{
  smartGridding_ = smartGridding;
}

QString SACScenario::calibrationDirectory() const
{
  return workingDirectory() + "/" + calibrationDirectory_;
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

void SACScenario::writeToFile(ScenarioWriter& writer) const
{
  CasaScenario::writeToFile(writer);
  writer.writeValue("SACScenarioVersion", 3);

  writer.writeValue("interpolationMethod", interpolationMethod_);
  writer.writeValue("smoothingOption",smoothingOption_);
  writer.writeValue("pIDW",pIDW_);  
  writer.writeValue("radiusSmoothing",radiusSmoothing_);
  writer.writeValue("smartGridding", smartGridding_);

  writer.writeValue("referenceSurface", t2zReferenceSurface_);
  writer.writeValue("t2zSubSampling", t2zSubSampling_);
  writer.writeValue("t2zRunOnOriginalProject", t2zRunOnOriginalProject_);
  writer.writeValue("t2zNumberOfCPUs", t2zNumberCPUs_);

  lithofractionManager_.writeToFile(writer);
  wellTrajectoryManager_.writeToFile(writer);
}

void SACScenario::readFromFile(const ScenarioReader& reader)
{
  CasaScenario::readFromFile(reader);

  int sacScenarioVersion = reader.readInt("SACScenarioVersion");

  if (sacScenarioVersion > 0)
  {
    interpolationMethod_ = reader.readInt("interpolationMethod");
    smoothingOption_ = reader.readInt("smoothingOption");
    pIDW_ = reader.readInt("pIDW");    
    radiusSmoothing_ = reader.readInt("radiusSmoothing");
  }

  if (sacScenarioVersion > 1)
  {
    t2zSubSampling_ = reader.readInt("t2zSubSampling") > 0 ? reader.readInt("t2zSubSampling") : t2zSubSampling_;
    t2zRunOnOriginalProject_ = reader.readBool("t2zRunOnOriginalProject");
    t2zNumberCPUs_ = reader.readInt("t2zNumberOfCPUs");
  }
  if (sacScenarioVersion > 2)
  {
    smartGridding_ = reader.readBool("smartGridding");
  }

  t2zLastSurface_ = projectReader().lowestSurfaceWithTWTData();
  t2zReferenceSurface_ = reader.readInt("referenceSurface");

  lithofractionManager_.readFromFile(reader);
  wellTrajectoryManager_.readFromFile(reader);
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

void SACScenario::wellPrepToSAC()
{
  calibrationTargetManager().appendFrom(calibrationTargetManagerWellPrep());
  calibrationTargetManager().disableInvalidWells(project3dPath().toStdString(), projectReader().getDepthGridName(0).toStdString());
  calibrationTargetManager().setWellHasDataInLayer(project3dPath().toStdString(), projectReader().layerNames());

  updateObjectiveFunctionFromTargets();
  wellTrajectoryManager_.updateWellTrajectories(calibrationTargetManager());
}

} // namespace sac

} // namespace casaWizard

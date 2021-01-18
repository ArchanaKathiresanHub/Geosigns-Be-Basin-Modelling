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
const int defaultLastSurface{6};

SACScenario::SACScenario(ProjectReader* projectReader) :
  CasaScenario{projectReader},
  stateFileNameSAC_{"casaStateSAC.txt"},
  calibrationFolder_{"calibration_step1"},
  lithofractionManager_{},
  wellTrajectoryManager_{},
  interpolationMethod_{0},
  smoothingOption_{0},
  pIDW_{1},
  threadsSmoothing_{1},
  radiusSmoothing_{1000},
  referenceSurface_{defaultReferenceSurface},
  lastSurface_{defaultLastSurface},
  activePlots_(4, true)
{
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

int SACScenario::threadsSmoothing() const
{
  return threadsSmoothing_;
}

void SACScenario::setThreadsSmoothing(int threadsSmoothing)
{
  threadsSmoothing_ = threadsSmoothing;
}

int SACScenario::radiusSmoothing() const
{
  return radiusSmoothing_;
}

void SACScenario::setRadiusSmoothing(int radiusSmoothing)
{
  radiusSmoothing_ = radiusSmoothing;
}

QString SACScenario::calibrationDirectory() const
{
  return workingDirectory() + "/" + calibrationFolder_;
}

int SACScenario::referenceSurface() const
{
  return referenceSurface_;
}

void SACScenario::setReferenceSurface(int refSurface)
{
  referenceSurface_ = refSurface;
}

int SACScenario::lastSurface() const
{
  return lastSurface_;
}

void SACScenario::setLastSurface(int lastSurface)
{
  lastSurface_ = lastSurface;
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
  writer.writeValue("SACScenarioVersion", 1);

  writer.writeValue("interpolationMethod", interpolationMethod_);
  writer.writeValue("smoothingOption",smoothingOption_);
  writer.writeValue("pIDW",pIDW_);
  writer.writeValue("threadsSmoothing",threadsSmoothing_);
  writer.writeValue("radiusSmoothing",radiusSmoothing_);

  writer.writeValue("referenceSurface", referenceSurface_);
  writer.writeValue("lastSurface", lastSurface_);

  lithofractionManager_.writeToFile(writer);
  wellTrajectoryManager_.writeToFile(writer);
}

void SACScenario::readFromFile(const ScenarioReader& reader)
{
  CasaScenario::readFromFile(reader);

  int sacScenarioVersion = reader.readInt("SACScenarioVersion");

  if (sacScenarioVersion>0)
  {
    interpolationMethod_ = reader.readInt("interpolationMethod");
    smoothingOption_ = reader.readInt("smoothingOption");
    pIDW_ = reader.readInt("pIDW");
    threadsSmoothing_ = reader.readInt("threadsSmoothing");
    radiusSmoothing_ = reader.readInt("radiusSmoothing");
  }

  referenceSurface_ = reader.readInt("referenceSurface");
  lastSurface_ = reader.readInt("lastSurface");

  lithofractionManager_.readFromFile(reader);
  wellTrajectoryManager_.readFromFile(reader);
}

void SACScenario::clear()
{
  CasaScenario::clear();

  referenceSurface_ = defaultReferenceSurface;
  lastSurface_ = defaultLastSurface;

  lithofractionManager_.clear();
  wellTrajectoryManager_.clear();
}

QString SACScenario::iterationDirName() const
{
  const QString iterationPath = calibrationDirectory() + "/" + runLocation();

  const QDir dir(iterationPath);
  QDateTime dateTime = QFileInfo(dir.path()).lastModified();

  QString dirName{""};
  for (const QString entry : dir.entryList())
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

} // namespace sac

} // namespace casaWizard

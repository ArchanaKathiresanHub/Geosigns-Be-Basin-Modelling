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

SACScenario::SACScenario(std::unique_ptr<ProjectReader> projectReader) :
  CasaScenario{std::move(projectReader)},
  stateFileNameSAC_{"casaStateSAC.txt"},
  calibrationFolder_{"calibration_step1"},
  lithofractionManager_{},
  wellTrajectoryManager_{},
  referenceSurface_{defaultReferenceSurface},
  lastSurface_{defaultLastSurface},
  activePlots_(4, true)
{
}

QString SACScenario::stateFileNameSAC() const
{
  return stateFileNameSAC_;
}

void SACScenario::setStateFileNameSAC(const QString& stateFileNameSAC)
{
  stateFileNameSAC_ = stateFileNameSAC;
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
  writer.writeValue("SACScenarioVersion", 0);

  writer.writeValue("referenceSurface", referenceSurface_);
  writer.writeValue("lastSurface", lastSurface_);

  lithofractionManager_.writeToFile(writer);
  wellTrajectoryManager_.writeToFile(writer);
}

void SACScenario::readFromFile(const ScenarioReader& reader)
{
  CasaScenario::readFromFile(reader);

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

QString SACScenario::calibrationDirectory() const
{
    return workingDirectory() + "/" + calibrationFolder_;
}

} // namespace sac

} // namespace casaWizard

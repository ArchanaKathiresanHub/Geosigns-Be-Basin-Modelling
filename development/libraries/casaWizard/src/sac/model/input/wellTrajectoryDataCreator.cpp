#include "wellTrajectoryDataCreator.h"

#include "model/logger.h"
#include "model/sacScenario.h"

#include <QFile>
#include <QFileInfo>
#include <QTextStream>


namespace casaWizard
{

namespace sac
{

WellTrajectoryDataCreator::WellTrajectoryDataCreator(SACScenario& scenario, const QString& iterationPath) :
  CaseDataCreator(scenario.calibrationTargetManager(), iterationPath),
  wellTrajectoryManager_{scenario.wellTrajectoryManager()},
  projectName_{QFileInfo(scenario.project3dPath()).baseName()}
{
}

void WellTrajectoryDataCreator::readCase(const int wellIndex, const int caseIndex)
{
  const Well& well = calibrationTargetManager().well(wellIndex);
  const QString dataFolder = iterationPath() + "/Case_" + QString::number(caseIndex);

  const QVector<WellTrajectory> trajectories = wellTrajectoryManager_.trajectoriesType(TrajectoryType::Base1D);
  for (const WellTrajectory& trajectory : trajectories )
  {
    if(trajectory.wellIndex() == well.id())
    {
      WellData baseRunData = readSingleTrajectory(trajectory, dataFolder, projectName_);
      wellTrajectoryManager_.setTrajectoryData(TrajectoryType::Base1D, trajectory.trajectoryIndex(), baseRunData.depth, baseRunData.value);

      WellData bestMatchData = readSingleTrajectory(trajectory, dataFolder, "bestMatchedCase");
      wellTrajectoryManager_.setTrajectoryData(TrajectoryType::Optimized1D, trajectory.trajectoryIndex(), bestMatchData.depth, bestMatchData.value);
    }
  }
}

WellTrajectoryDataCreator::WellData WellTrajectoryDataCreator::readSingleTrajectory(const WellTrajectory& trajectory, const QString& dataFolder, const QString& projectName)
{
  const QString dataFilename = dataFolder + "/wellTrajectory-" + projectName + "-" + trajectory.property() + ".csv";
  QFile file{dataFilename};

  if( !file.exists())
  {
    Logger::log() << "File " << dataFilename << " not found" << Logger::endl();
    return WellData{{}, {}};
  }
  Logger::log() << "Reading file " << dataFilename << Logger::endl();

  QVector<double> depth;
  QVector<double> value;

  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
  {
    Logger::log() << "Opening datafile failed" << Logger::endl();
    return WellData{{}, {}};
  }

  QTextStream inputStream{&file};
  Logger::log() << "Header: " << inputStream.readLine() << Logger::endl(); // also skips header row

  while (!inputStream.atEnd())
  {
    const QString textLine{inputStream.readLine()};
    const QStringList entries{textLine.split(",")};
    depth.append(entries[8].toDouble());
    if (entries.size() > 9 ) // sometimes track1d doesn't provide last colum with values
    {
      value.append(entries[9].toDouble());
    }
    else
    {
      value.append(0.0);
    }
  }

  return WellData{depth, value};
}

} // namespace sac

} // namespace casaWizard

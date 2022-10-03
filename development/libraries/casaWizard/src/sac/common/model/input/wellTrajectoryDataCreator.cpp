//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "wellTrajectoryDataCreator.h"

#include "model/logger.h"
#include "model/SacScenario.h"

#include <QFile>
#include <QTextStream>

namespace casaWizard
{

namespace sac
{

WellTrajectoryDataCreator::WellTrajectoryDataCreator(SacScenario& scenario, const QString& projectName, const QString& iterationPath) :
  CaseDataCreator(scenario.calibrationTargetManager(), iterationPath),
  m_wellTrajectoryManager{scenario.wellTrajectoryManager()},
  m_projectName{projectName}
{
}

void WellTrajectoryDataCreator::readCase(const int wellIndex, const int caseIndex)
{
  const Well& well = calibrationTargetManager().well(wellIndex);

  const TrajectoryType type = (m_projectName == "bestMatchedCase") ? TrajectoryType::Optimized1D : TrajectoryType::Original1D;

  QString dataFolder;
  if (type == TrajectoryType::Original1D)
  {
    dataFolder = iterationPath() + "/" + well.name();
  }
  else
  {
    dataFolder = iterationPath() + "/Case_" + QString::number(caseIndex);
  }

  const QVector<WellTrajectory> trajectories = m_wellTrajectoryManager.trajectoriesType(type);
  for (const WellTrajectory& trajectory : trajectories )
  {
    if(trajectory.wellIndex() == well.id())
    {    
      const WellData data = readSingleTrajectory(trajectory, dataFolder, m_projectName);
      m_wellTrajectoryManager.setTrajectoryData(type, trajectory.trajectoryIndex(), data.depth, data.value);
    }
  }
}

WellTrajectoryDataCreator::WellData WellTrajectoryDataCreator::readSingleTrajectory(const WellTrajectory& trajectory, const QString& dataFolder, const QString& projectName)
{
  const QString dataFilename = dataFolder + "/wellTrajectory-" + projectName + "-" + trajectory.propertyUserName() + ".csv";
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

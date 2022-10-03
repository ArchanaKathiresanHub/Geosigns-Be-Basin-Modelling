//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "wellTrajectoryManager.h"

#include "model/calibrationTargetManager.h"
#include "model/scenarioReader.h"
#include "model/scenarioWriter.h"
#include "model/well.h"

#include <QStringList>

namespace casaWizard
{

namespace sac
{

WellTrajectoryManager::WellTrajectoryManager() :
  m_trajectories(4, {})
{
}

QVector<WellTrajectory> WellTrajectoryManager::trajectoriesType(const TrajectoryType type) const
{
  return m_trajectories[type];
}

QVector<QVector<WellTrajectory> > WellTrajectoryManager::trajectories() const
{
  return m_trajectories;
}

QVector<QVector<WellTrajectory>> WellTrajectoryManager::trajectoriesInWell(const QVector<int> wellIndices, const QStringList properties) const
{
  QVector<QVector<WellTrajectory>> trajectoriesInWell(4,{});
  for (int i = 0; i < 4; ++i)
  {
    trajectoriesInWell[i] = selectFromWell(m_trajectories[i], wellIndices, properties);
  }
  return trajectoriesInWell;
}

void WellTrajectoryManager::updateWellTrajectories(const CalibrationTargetManager& calibrationTargetManager)
{
  clear();
  for (const Well* well : calibrationTargetManager.wells())
  {
    QStringList propertyUserNames;
    calibrationTargetManager.extractWellTargets(propertyUserNames, well->id());
    for(const QString& propertyUserName : propertyUserNames)
    {
      addWellTrajectory(well->id(), propertyUserName);
    }
  }
}

void WellTrajectoryManager::addWellTrajectory(const int wellIndex, const QString& propertyCauldronName)
{
  for (int i = 0; i < m_trajectories.size(); ++i)
  {
    m_trajectories[i].append(WellTrajectory{m_trajectories[i].size(), wellIndex, propertyCauldronName, {}, {}});
  }
}

void WellTrajectoryManager::setTrajectoryData(const TrajectoryType type, const int trajectoryIndex, const QVector<double> depth, const QVector<double> value)
{
  if (trajectoryIndex >= m_trajectories[type].size())
  {
    return;
  }
  m_trajectories[type][trajectoryIndex].setDepth(depth);
  m_trajectories[type][trajectoryIndex].setValue(value);
}


QVector<WellTrajectory> WellTrajectoryManager::selectFromWell(const QVector<WellTrajectory>& trajectories, const QVector<int> wellIndices, const QStringList& properties) const
{
  QVector<WellTrajectory> trajectoryForWell;
  for (const WellTrajectory& trajectory : trajectories)
  {
    if (wellIndices.contains(trajectory.wellIndex()))
    {
      const int propertyIndex = properties.indexOf(trajectory.propertyUserName());
      if (propertyIndex > -1)
      {
        trajectoryForWell.append(trajectory);
      }
    }
  }
  return trajectoryForWell;
}

void WellTrajectoryManager::writeToFile(ScenarioWriter& writer) const
{
  writer.writeValue("WellTrajectoryManagerVersion", 0);
  writer.writeValue("baseRunTrajectories", m_trajectories[TrajectoryType::Original1D]);
  writer.writeValue("bestMatchTrajectories", m_trajectories[TrajectoryType::Optimized1D]);
  writer.writeValue("base3dTrajectories", m_trajectories[TrajectoryType::Original3D]);
  writer.writeValue("optimized3dTrajectories", m_trajectories[TrajectoryType::Optimized3D]);
}

void WellTrajectoryManager::readFromFile(const ScenarioReader& reader)
{
  m_trajectories[TrajectoryType::Original1D] = reader.readVector<WellTrajectory>("baseRunTrajectories");
  m_trajectories[TrajectoryType::Optimized1D] = reader.readVector<WellTrajectory>("bestMatchTrajectories");
  m_trajectories[TrajectoryType::Original3D] = reader.readVector<WellTrajectory>("base3dTrajectories");
  m_trajectories[TrajectoryType::Optimized3D] = reader.readVector<WellTrajectory>("optimized3dTrajectories");
}

void WellTrajectoryManager::clear()
{
  for (int i = 0; i < m_trajectories.size(); ++i)
  {
    m_trajectories[i].clear();
  }
}

}  // namespace sac

}  // namespace casaWizard

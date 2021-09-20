#include "wellTrajectoryManager.h"

#include "model/calibrationTargetManager.h"
#include "model/logger.h"
#include "model/scenarioReader.h"
#include "model/scenarioWriter.h"
#include "model/trajectoryType.h"
#include "model/well.h"

#include <QStringList>

namespace casaWizard
{

namespace sac
{

WellTrajectoryManager::WellTrajectoryManager() :
  trajectories_(4, {})
{
}

QVector<WellTrajectory> WellTrajectoryManager::trajectoriesType(const TrajectoryType type) const
{
  return trajectories_[type];
}

QVector<QVector<WellTrajectory> > WellTrajectoryManager::trajectories() const
{
  return trajectories_;
}

QVector<QVector<WellTrajectory>> WellTrajectoryManager::trajectoriesInWell(const QVector<int> wellIndices, const QStringList properties) const
{
  QVector<QVector<WellTrajectory>> trajectoriesInWell(4,{});
  for (int i = 0; i < 4; ++i)
  {
    trajectoriesInWell[i] = selectFromWell(trajectories_[i], wellIndices, properties);
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
  for (int i = 0; i < trajectories_.size(); ++i)
  {
    trajectories_[i].append(WellTrajectory{trajectories_[i].size(), wellIndex, propertyCauldronName, {}, {}});
  }
}

void WellTrajectoryManager::setTrajectoryData(const TrajectoryType type, const int trajectoryIndex, const QVector<double> depth, const QVector<double> value)
{
  if (trajectoryIndex >= trajectories_[type].size())
  {
    return;
  }
  trajectories_[type][trajectoryIndex].setDepth(depth);
  trajectories_[type][trajectoryIndex].setValue(value);
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
  writer.writeValue("baseRunTrajectories", trajectories_[TrajectoryType::Original1D]);
  writer.writeValue("bestMatchTrajectories", trajectories_[TrajectoryType::Optimized1D]);
  writer.writeValue("base3dTrajectories", trajectories_[TrajectoryType::Original3D]);
  writer.writeValue("optimized3dTrajectories", trajectories_[TrajectoryType::Optimized3D]);
}

void WellTrajectoryManager::readFromFile(const ScenarioReader& reader)
{
  trajectories_[TrajectoryType::Original1D] = reader.readVector<WellTrajectory>("baseRunTrajectories");
  trajectories_[TrajectoryType::Optimized1D] = reader.readVector<WellTrajectory>("bestMatchTrajectories");
  trajectories_[TrajectoryType::Original3D] = reader.readVector<WellTrajectory>("base3dTrajectories");
  trajectories_[TrajectoryType::Optimized3D] = reader.readVector<WellTrajectory>("optimized3dTrajectories");
}

void WellTrajectoryManager::clear()
{
  for (int i = 0; i < trajectories_.size(); ++i)
  {
    trajectories_[i].clear();
  }
}

}  // namespace sac

}  // namespace casaWizard

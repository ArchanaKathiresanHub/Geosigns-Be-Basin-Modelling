// Manager for the data of the well trajectories
#pragma once

#include "model/writable.h"

#include "trajectoryType.h"
#include "wellTrajectory.h"

#include <QVector>

namespace casaWizard
{

class CalibrationTargetManager;

namespace sac
{

class WellTrajectoryManager : public Writable
{
public:
  explicit WellTrajectoryManager();

  QVector<WellTrajectory> trajectoriesType(const TrajectoryType type) const;
  QVector<QVector<WellTrajectory>> trajectories() const;
  QVector<QVector<WellTrajectory>> trajectoriesInWell(const QVector<int> wellIndices, const QStringList properties) const;

  void updateWellTrajectories(const CalibrationTargetManager& calibrationTargetManager);
  void addWellTrajectory(const int wellIndex, const QString& property);
  void setTrajectoryData(const TrajectoryType type, const int trajectoryIndex, const QVector<double> depth, const QVector<double> value);

  void writeToFile(ScenarioWriter& writer) const override;
  void readFromFile(const ScenarioReader& reader) override;
  void clear() override;

private:
  WellTrajectoryManager(const WellTrajectoryManager&) = delete;
  WellTrajectoryManager& operator=(WellTrajectoryManager) = delete;

  QVector<WellTrajectory> selectFromWell(const QVector<WellTrajectory>& trajectories, const QVector<int> wellIndices, const QStringList& properties) const;

  QVector<QVector<WellTrajectory>> trajectories_;
};

}  // namespace sac

}  // namespace casaWizard

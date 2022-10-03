//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// Manager for the data of the well trajectories
#pragma once

#include "model/writable.h"

#include "model/trajectoryType.h"
#include "model/wellTrajectory.h"

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
  void setTrajectoryData(const TrajectoryType type, const int trajectoryIndex, const QVector<double> depth, const QVector<double> value);

  void writeToFile(ScenarioWriter& writer) const override;
  void readFromFile(const ScenarioReader& reader) override;
  void clear() override;

  void addWellTrajectory(const int wellIndex, const QString& propertyCauldronName);
private:
  WellTrajectoryManager(const WellTrajectoryManager&) = delete;
  WellTrajectoryManager& operator=(WellTrajectoryManager) = delete;

  QVector<WellTrajectory> selectFromWell(const QVector<WellTrajectory>& trajectories, const QVector<int> wellIndices, const QStringList& properties) const;

  QVector<QVector<WellTrajectory>> m_trajectories;
};

}  // namespace sac

}  // namespace casaWizard

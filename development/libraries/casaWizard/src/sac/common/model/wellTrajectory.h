//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// Data struct for well trajectories, written to file
#pragma once

#include <QStringList>
#include <QVector>

namespace casaWizard
{

namespace sac
{

class WellTrajectory
{
public:
  WellTrajectory() = default;
  explicit WellTrajectory(const int trajectoryIndex, const int wellIndex,
                          const QString& propertyUserName, const QVector<double>& depth, const QVector<double>& value);
  int version() const;
  static WellTrajectory read(const int version, const QStringList& p);

  QStringList write() const;

  int trajectoryIndex() const;
  int wellIndex() const;
  QString propertyUserName() const;

  QVector<double> depth() const;
  void setDepth(const QVector<double>& depth);

  QVector<double> value() const;
  void setValue(const QVector<double>& value);

private:
  int m_trajectoryIndex;
  int m_wellIndex;
  QString m_propertyUserName;
  QVector<double> m_depth;
  QVector<double> m_value;
};

} // namespace sac

} // namespace casaWizard

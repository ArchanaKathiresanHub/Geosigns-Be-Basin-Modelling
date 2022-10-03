//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "wellTrajectory.h"

#include "model/scenarioIO.h"

namespace casaWizard
{

namespace sac
{

WellTrajectory::WellTrajectory(const int trajectoryIndex,
                               const int wellIndex,
                               const QString& propertyUserName,
                               const QVector<double>& depth ,
                               const QVector<double>& value) :
  m_trajectoryIndex{trajectoryIndex},
  m_wellIndex{wellIndex},
  m_propertyUserName{propertyUserName},
  m_depth{depth},
  m_value{value}
{
}

int WellTrajectory::version() const
{
  return 0;
}

WellTrajectory WellTrajectory::read(const int /*version*/, const QStringList& p)
{
  if (p.size() != 5)
  {
    return WellTrajectory{0, 0, "UnknownPropertyFromRead", {}, {}};
  }
  return WellTrajectory
  {
    p[0].toInt(),
    p[1].toInt(),
    p[2],
    scenarioIO::vectorFromRead(p[3]),
    scenarioIO::vectorFromRead(p[4])
  };
}

QStringList WellTrajectory::write() const
{
  QStringList out;
  out << QString::number(m_trajectoryIndex)
      << QString::number(m_wellIndex)
      << m_propertyUserName
      << scenarioIO::vectorToWrite(m_depth)
      << scenarioIO::vectorToWrite(m_value);

  return out;
}
int WellTrajectory::trajectoryIndex() const
{
  return m_trajectoryIndex;
}

int WellTrajectory::wellIndex() const
{
  return m_wellIndex;
}

QString WellTrajectory::propertyUserName() const
{
  return m_propertyUserName;
}

QVector<double> WellTrajectory::depth() const
{
  return m_depth;
}

void WellTrajectory::setDepth(const QVector<double>& depth)
{
  m_depth = depth;
}
QVector<double> WellTrajectory::value() const
{
  return m_value;
}

void WellTrajectory::setValue(const QVector<double>& value)
{
  m_value = value;
}

} // namespace sac

} // namespace casaWizard

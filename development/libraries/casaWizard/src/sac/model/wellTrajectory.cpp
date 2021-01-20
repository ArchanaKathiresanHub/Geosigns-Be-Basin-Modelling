#include "wellTrajectory.h"

#include "model/scenarioIO.h"

namespace casaWizard
{

namespace sac
{

WellTrajectory::WellTrajectory(const int trajectoryIndex,
                               const int wellIndex,
                               const QString& property,
                               const QVector<double>& depth ,
                               const QVector<double>& value) :
  trajectoryIndex_{trajectoryIndex},
  wellIndex_{wellIndex},
  property_{property},
  depth_{depth},
  value_{value}
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

QString WellTrajectory::name() const
{
  return QString::number(wellIndex_) + "_" + property_;
}

QStringList WellTrajectory::write() const
{
  QStringList out;
  out << QString::number(trajectoryIndex_)
      << QString::number(wellIndex_)
      << property_
      << scenarioIO::vectorToWrite(depth_)
      << scenarioIO::vectorToWrite(value_);

  return out;
}
int WellTrajectory::trajectoryIndex() const
{
  return trajectoryIndex_;
}

int WellTrajectory::wellIndex() const
{
  return wellIndex_;
}

QString WellTrajectory::property() const
{
  return property_;
}

QVector<double> WellTrajectory::depth() const
{
  return depth_;
}

void WellTrajectory::setDepth(const QVector<double>& depth)
{
  depth_ = depth;
}
QVector<double> WellTrajectory::value() const
{
  return value_;
}

void WellTrajectory::setValue(const QVector<double>& value)
{
  value_ = value;
}

} // namespace sac

} // namespace casaWizard
#include "calibrationTarget.h"

namespace casaWizard
{

CalibrationTarget::CalibrationTarget(const QString& name, const QString& propertyUserName,
                                     const double z, const double value, const double standardDeviation, const double uaWeight) :
  name_{name},
  propertyUserName_{propertyUserName},
  z_{z},
  value_{value},
  standardDeviation_{standardDeviation},
  uaWeight_{uaWeight}
{
}

int CalibrationTarget::version() const
{
  return 0;
}

CalibrationTarget CalibrationTarget::read(const int /*version*/, const QStringList& p)
{
  if (p.size() != 6)
  {
    return CalibrationTarget{};
  }

  return CalibrationTarget
  {
    p[0],
    p[1],
    p[2].toDouble(),
    p[3].toDouble(),
    p[4].toDouble(),
    p[5].toDouble()
  };
}

QStringList CalibrationTarget::write() const
{
  QStringList out;
  out << name_
      << propertyUserName_
      << QString::number(z_)
      << QString::number(value_)
      << QString::number(standardDeviation_)
      << QString::number(uaWeight_);
  return out;
}
QString CalibrationTarget::name() const
{
  return name_;
}

QString CalibrationTarget::propertyUserName() const
{
  return propertyUserName_;
}

double CalibrationTarget::z() const
{
  return z_;
}

double CalibrationTarget::value() const
{
  return value_;
}

double CalibrationTarget::standardDeviation() const
{
  return standardDeviation_;
}

void CalibrationTarget::setStandardDeviation(double standardDeviation)
{
  standardDeviation_ = standardDeviation;
}

double CalibrationTarget::uaWeight() const
{
  return uaWeight_;
}

void CalibrationTarget::setUaWeight(double uaWeight)
{
  uaWeight_ = uaWeight;
}

}  // namespace casaWizard

#pragma once

#include <QStringList>

namespace casaWizard
{

class CalibrationTarget
{
public:
  explicit CalibrationTarget(const QString& name = "",
                             const QString& property = "",
                             const double z = 0.0,
                             const double value = 0.0,
                             const double standardDeviation = 0.0,
                             const double uaWeight = 0.0);
  static CalibrationTarget read(const int version, const QStringList& p);
  int version() const;
  QStringList write() const;

  QString name() const;
  void setName(const QString& name);

  QString property() const;
  void setProperty(const QString& property);

  double z() const;
  void setZ(double z);

  double value() const;
  void setValue(double value);

  double standardDeviation() const;
  void setStandardDeviation(double standardDeviation);

  double uaWeight() const;
  void setUaWeight(double uaWeight);

private:
  QString name_;
  QString property_;
  double z_;
  double value_;
  double standardDeviation_;
  double uaWeight_;
};

}  // namespace casaWizard

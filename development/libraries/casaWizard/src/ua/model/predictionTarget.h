// Abstract base class for prediction target
#pragma once

#include <QStringList>

namespace casaWizard
{

namespace ua
{

class PredictionTarget
{
public:
  PredictionTarget() = default;
  explicit PredictionTarget(const QString& property, const double x, const double y, const double age);
  virtual ~PredictionTarget() = default;

  virtual int version() const = 0;
  virtual QStringList write() const = 0;
  virtual QString name() const = 0;
  virtual QString nameWithoutAge() const = 0;
  virtual QString variable() const = 0;
  virtual QString typeName() const = 0;

  QString property() const;
  void setProperty(const QString& property);

  double x() const;
  void setX(double x);

  double y() const;
  void setY(double y);

  double age() const;
  void setAge(double age);

  QString unitSI() const;

private:
  QString property_;
  double x_;
  double y_;
  double age_;
};

} // namespace ua

} // namespace casaWizard

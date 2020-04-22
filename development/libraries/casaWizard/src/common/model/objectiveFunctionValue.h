// Container for objective function value
#pragma once

#include <QString>

namespace casaWizard
{

class ObjectiveFunctionValue
{
public:
  ObjectiveFunctionValue() = default;
  explicit ObjectiveFunctionValue(const QString& var, const double absErr = 0.0, const double relErr = 0.0, const double w = 0.0);
  static ObjectiveFunctionValue read(const int version, const QStringList& p);
  int version() const;
  QStringList write() const;

  QString variable() const;
  void setVariable(const QString& variable);

  double absoluteError() const;
  void setAbsoluteError(double absoluteError);

  double relativeError() const;
  void setRelativeError(double relativeError);

  double weight() const;
  void setWeight(double weight);

private:
  QString variable_;
  double absoluteError_;
  double relativeError_;
  double weight_;
};

} // casaWizard

// Container for objective function value
#pragma once

#include <QString>

namespace casaWizard
{

class ObjectiveFunctionValue
{
public:
  ObjectiveFunctionValue() = default;
  explicit ObjectiveFunctionValue(const QString& variableCauldronName, const QString& variableUserName, const double absErr = 0.0, const double relErr = 0.1, const double w = 0.0, const bool enabled = true);
  static ObjectiveFunctionValue read(const int version, const QStringList& p);
  int version() const;
  QStringList write() const;

  QString variableCauldronName() const;
  QString variableUserName() const;

  double absoluteError() const;
  void setAbsoluteError(double absoluteError);

  bool enabled() const;
  void setEnabled(const bool enabled);

  double relativeError() const;
  void setRelativeError(double relativeError);

  double weight() const;
  void setWeight(double weight);

private:
  double absoluteError_;
  double relativeError_;
  bool enabled_;
  QString variableCauldronName_;
  QString variableUserName_;
  double weight_;
};

} // casaWizard

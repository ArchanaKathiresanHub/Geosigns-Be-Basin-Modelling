// Container for objective function value
#include "objectiveFunctionValue.h"

#include "scenarioIO.h"
#include <QStringList>

namespace casaWizard
{


ObjectiveFunctionValue::ObjectiveFunctionValue(const QString& var, const double absErr, const double relErr, const double w) :
  variable_{var},
  absoluteError_{absErr},
  relativeError_{relErr},
  weight_{w}
{
}

ObjectiveFunctionValue ObjectiveFunctionValue::read(const int /*version*/, const QStringList& p)
{
  if (p.size() != 4)
  {
    return ObjectiveFunctionValue{"ReadError"};
  }

  return ObjectiveFunctionValue
  {
    p[0],
    p[1].toDouble(),
    p[2].toDouble(),
    p[3].toDouble()
  };
}

int ObjectiveFunctionValue::version() const
{
  return 0;
}


QStringList ObjectiveFunctionValue::write() const
{
  QStringList out;
  out << variable_
      << scenarioIO::doubleToQString(absoluteError_)
      << scenarioIO::doubleToQString(relativeError_)
      << scenarioIO::doubleToQString(weight_);
  return out;
}

QString ObjectiveFunctionValue::variable() const
{
  return variable_;
}

void ObjectiveFunctionValue::setVariable(const QString& variable)
{
  variable_ = variable;
}

double ObjectiveFunctionValue::absoluteError() const
{
  return absoluteError_;
}

void ObjectiveFunctionValue::setAbsoluteError(double absoluteError)
{
  absoluteError_ = absoluteError;
}

double ObjectiveFunctionValue::relativeError() const
{
  return relativeError_;
}

void ObjectiveFunctionValue::setRelativeError(double relativeError)
{
  relativeError_ = relativeError;
}

double ObjectiveFunctionValue::weight() const
{
  return weight_;
}

void ObjectiveFunctionValue::setWeight(double weight)
{
  weight_ = weight;
}

} // casaWizard

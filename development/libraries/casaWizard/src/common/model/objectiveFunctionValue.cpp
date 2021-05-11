// Container for objective function value
#include "objectiveFunctionValue.h"

#include "scenarioIO.h"
#include <QStringList>

namespace casaWizard
{


ObjectiveFunctionValue::ObjectiveFunctionValue(const QString& variableCauldronName, const QString& variableUserName, const double absErr, const double relErr, const double w, const bool enabled) :
  absoluteError_{absErr},
  relativeError_{relErr},
  enabled_{enabled},
  variableCauldronName_{variableCauldronName},
  variableUserName_{variableUserName},
  weight_{w}
{
}

ObjectiveFunctionValue ObjectiveFunctionValue::read(const int version, const QStringList& p)
{
  if (version > 1)
  {
    if (p.size() != 6)
    {
      return ObjectiveFunctionValue{"ReadError", "ReadError"};
    }

    return ObjectiveFunctionValue
    {
      p[0],
      p[1],
      p[2].toDouble(),
      p[3].toDouble(),
      p[4].toDouble(),
      p[5] == "1"
    };
  }
  else if (version > 0)
  {
    if (p.size() != 5)
    {
      return ObjectiveFunctionValue{"ReadError", "ReadError"};
    }

    return ObjectiveFunctionValue
    {
      p[0],
      p[0],
      p[1].toDouble(),
      p[2].toDouble(),
      p[3].toDouble(),
      p[4] == "1"
    };
  }
  else
  {
    if (p.size() != 4)
    {
      return ObjectiveFunctionValue{"ReadError", "ReadError"};
    }

    return ObjectiveFunctionValue
    {
      p[0],
      p[0],
      p[1].toDouble(),
      p[2].toDouble(),
      p[3].toDouble()
    };
  }
}

int ObjectiveFunctionValue::version() const
{
  return 2;
}


QStringList ObjectiveFunctionValue::write() const
{
  QStringList out;
  out << variableCauldronName_
      << variableUserName_
      << scenarioIO::doubleToQString(absoluteError_)
      << scenarioIO::doubleToQString(relativeError_)
      << scenarioIO::doubleToQString(weight_)
      << QString::number(enabled_);
  return out;
}

QString ObjectiveFunctionValue::variableCauldronName() const
{
  return variableCauldronName_;
}

QString ObjectiveFunctionValue::variableUserName() const
{
  return variableUserName_;
}

double ObjectiveFunctionValue::absoluteError() const
{
  return absoluteError_;
}

void ObjectiveFunctionValue::setAbsoluteError(double absoluteError)
{
  absoluteError_ = absoluteError;
}

bool ObjectiveFunctionValue::enabled() const
{
  return enabled_;
}

void ObjectiveFunctionValue::setEnabled(const bool enabled)
{
  enabled_ = enabled;
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

#include "predictionTarget.h"

#include "model/targetParameterMapCreator.h"

namespace casaWizard
{

namespace ua
{

PredictionTarget::PredictionTarget(const QString& property, const double x, const double y, const double age) :
  property_{property},
  x_{x},
  y_{y},
  age_{age}
{
}

QString PredictionTarget::property() const
{
  return property_;
}

void PredictionTarget::setProperty(const QString& property)
{
  property_ = property;
}

double PredictionTarget::x() const
{
  return x_;
}

void PredictionTarget::setX(double x)
{
  x_ = x;
}

double PredictionTarget::y() const
{
  return y_;
}

void PredictionTarget::setY(double y)
{
  y_ = y;
}

double PredictionTarget::age() const
{
    return age_;
}

void PredictionTarget::setAge(double age)
{
    age_ = age;
}

QString PredictionTarget::unitSI() const
{
  return targetParameterMapCreator::lookupSIUnit(property_);
}

} // namespace ua

} // namespace casaWizard

#include "predictionTargetDepth.h"

#include "model/scenarioIO.h"

namespace casaWizard
{

namespace ua
{

PredictionTargetDepth::PredictionTargetDepth(const QString& property, const double x, const double y, const double z, const double age) :
  PredictionTarget(property, x, y, age),
  z_{z}
{
}

PredictionTargetDepth PredictionTargetDepth::read(const int /*version*/, const QStringList& p)
{
  if (p.size() != 5)
  {
    return PredictionTargetDepth{"Unknown", 0, 0, 0, 0};
  }

  return PredictionTargetDepth
  {
    p[0],
    p[1].toDouble(),
    p[2].toDouble(),
    p[3].toDouble(),
    p[4].toDouble()
  };
}

int PredictionTargetDepth::version() const
{
  return 0;
}

QStringList PredictionTargetDepth::write() const
{
  QStringList out;
  out << property()
      << scenarioIO::doubleToQString(x())
      << scenarioIO::doubleToQString(y())
      << scenarioIO::doubleToQString(z_)
      << scenarioIO::doubleToQString(age());
  return out;
}

QString PredictionTargetDepth::name() const
{
  auto doubleNoDigitToQString = [](double d){return QString::number(d, 'g', 12); };

  return property() + " (" + doubleNoDigitToQString(x()) + ", " + doubleNoDigitToQString(y()) + ", " + doubleNoDigitToQString(z_) + ", " + QString::number(age(), 'f', 1) + ")";
}

QString PredictionTargetDepth::nameWithoutAge() const
{
  auto doubleNoDigitToQString = [](double d){return QString::number(d, 'g', 12); };

  return property() + " (" + doubleNoDigitToQString(x()) + ", " + doubleNoDigitToQString(y()) + ", " + doubleNoDigitToQString(z_) + ")";
}

QString PredictionTargetDepth::variable() const
{
  return QString::number(z_, 'g', 12);
}

QString PredictionTargetDepth::typeName() const
{
  return "XYZPoint";
}

double PredictionTargetDepth::z() const
{
  return z_;
}

void PredictionTargetDepth::setZ(double z)
{
  z_ = z;
}

} // namespace ua

} // namespace casaWizard

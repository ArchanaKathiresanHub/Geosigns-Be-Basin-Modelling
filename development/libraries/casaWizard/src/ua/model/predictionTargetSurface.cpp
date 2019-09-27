#include "predictionTargetSurface.h"

#include "model/scenarioIO.h"

namespace casaWizard
{

namespace ua
{

PredictionTargetSurface::PredictionTargetSurface(const QString& property, const double x, const double y, const QString& surface, const double age) :
  PredictionTarget(property, x, y, age),
  layerName_{surface}
{

}

PredictionTargetSurface PredictionTargetSurface::read(const int /*version*/, const QStringList& p)
{
  if (p.size() != 5)
  {
    return PredictionTargetSurface{"Unknown", 0, 0, "", 0};
  }

  return PredictionTargetSurface
  {
    p[0],
    p[1].toDouble(),
    p[2].toDouble(),
    p[3],
    p[4].toDouble()
  };
}

int PredictionTargetSurface::version() const
{
  return 0;
}

QStringList PredictionTargetSurface::write() const
{
  QStringList out;
  out << property()
      << scenarioIO::doubleToQString(x())
      << scenarioIO::doubleToQString(y())
      << layerName_
      << scenarioIO::doubleToQString(age());
  return out;
}

QString PredictionTargetSurface::name() const
{
  auto doubleNoDigitToQString = [](double d){return QString::number(d, 'g', 12); };

  return property() + " (" + doubleNoDigitToQString(x()) + ", " + doubleNoDigitToQString(y()) + ", " + layerName_ + ", " + QString::number(age(), 'f', 1) + ")";
}

QString PredictionTargetSurface::nameWithoutAge() const
{
  auto doubleNoDigitToQString = [](double d){return QString::number(d, 'g', 12); };

  return property() + " (" + doubleNoDigitToQString(x()) + ", " + doubleNoDigitToQString(y()) + ", " + layerName_ + ")";
}

QString PredictionTargetSurface::variable() const
{
  return "\"" + layerName_ + "\"";
}

QString PredictionTargetSurface::typeName() const
{
  return "XYPointLayerTopSurface";
}

QString PredictionTargetSurface::layerName() const
{
  return layerName_;
}

void PredictionTargetSurface::setLayerName(const QString& surface)
{
  layerName_ = surface;
}

} // namespace ua

} // namespace casaWizard

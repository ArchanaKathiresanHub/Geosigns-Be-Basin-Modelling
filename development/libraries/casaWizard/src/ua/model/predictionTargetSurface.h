// Target at a specific surface
#pragma once

#include "predictionTarget.h"

namespace casaWizard
{

class ToDepthConverter;

namespace ua
{

class PredictionTargetSurface : public PredictionTarget
{
public:
  PredictionTargetSurface() = default;
  PredictionTargetSurface(const QVector<QString>& properties, const double x, const double y, const QString& surfaceName, const double age, const ToDepthConverter* todepthConverter = nullptr, const QString& locationName = "");
  static PredictionTargetSurface read(const int version, const QStringList& parameters);

  int version() const override;
  QStringList write() const override;
  QString name(const QString& property) const override;
  QString nameWithoutAge() const override;
  QString variable() const override;
  QString typeName() const override;

  void setToDepthConverterAndCalcDepth(const ToDepthConverter* todepthConverter);

  QString surfaceName() const override;

  void setSurfaceAndLayerName(const QString& surfaceName, const QString layerName);

  void setX(double x) override;
  void setY(double y) override;

  PredictionTarget* createCopy() const override;

private:
  QString m_surface;
  QString m_layer;
  const ToDepthConverter* m_toDepthConverter;
};

} // namespace ua

} // namespace casaWizard

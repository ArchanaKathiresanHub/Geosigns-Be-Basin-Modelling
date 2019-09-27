// Target at a specific surface
#pragma once

#include "predictionTarget.h"

namespace casaWizard
{

namespace ua
{

class PredictionTargetSurface : public PredictionTarget
{
public:
  PredictionTargetSurface() = default;
  explicit PredictionTargetSurface(const QString& property, const double x, const double y, const QString& layerName, const double age);
  static PredictionTargetSurface read(const int version, const QStringList& parameters);

  int version() const override;
  QStringList write() const override;
  QString name() const override;
  QString nameWithoutAge() const override;
  QString variable() const override;
  QString typeName() const override;

  QString layerName() const;
  void setLayerName(const QString& layerName);

private:
  QString layerName_;
};

} // namespace ua

} // namespace casaWizard

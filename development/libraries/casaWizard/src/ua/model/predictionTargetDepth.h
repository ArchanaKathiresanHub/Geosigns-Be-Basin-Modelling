// Target at specific depth
#pragma once

#include "predictionTarget.h"

namespace casaWizard
{

namespace ua
{

class PredictionTargetDepth : public PredictionTarget
{
public:
  PredictionTargetDepth() = default;
  explicit PredictionTargetDepth(const QString& property, const double x, const double y, const double z, const double age);
  static PredictionTargetDepth read(const int version, const QStringList& parameters);

  int version() const override;
  QStringList write() const override;
  QString name() const override;
  QString nameWithoutAge() const override;
  QString variable() const override;
  QString typeName() const override;

  double z() const;
  void setZ(double z);

private:
  double z_;
};

} // namespace ua

} // namespace casaWizard

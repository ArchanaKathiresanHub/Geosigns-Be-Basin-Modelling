// Target at specific depth
#pragma once

#include "predictionTarget.h"
#include <memory>

namespace casaWizard
{

namespace ua
{

class PredictionTargetDepth : public PredictionTarget
{
public:
  PredictionTargetDepth() = default;
  explicit PredictionTargetDepth(const QVector<QString>& properties, const double x, const double y, const double z, const double age, const QString& locationName = "");

  static PredictionTargetDepth read(const int version, const QStringList& parameters);

  int version() const override;
  QStringList write() const override;
  QString name(const QString& property) const override;
  QString nameWithoutAge() const override;
  QString variable() const override;
  QString typeName() const override;

  PredictionTarget* createCopy() const override;
};

} // namespace ua

} // namespace casaWizard

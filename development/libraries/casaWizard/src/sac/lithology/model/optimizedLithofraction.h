// Struct for storing the optimized lithofraction values per well
#pragma once

#include <QStringList>

namespace casaWizard
{

namespace sac
{

namespace lithology
{

class OptimizedLithofraction
{
public:
  OptimizedLithofraction() = default;
  OptimizedLithofraction(const int wellId,
                         const int lithofractionId,
                         const double optimizedPercentageFirstComponent,
                         const double optimizedFractionSecondComponent);
  int version() const;
  static OptimizedLithofraction read(const int version, const QStringList& p);
  QStringList write() const;

  int wellId() const;
  int lithofractionId() const;
  double optimizedPercentageFirstComponent() const;
  double optimizedFractionSecondComponent() const;

  double optimizedPercentageSecondComponent() const;
  double optimizedPercentageThirdComponent() const;

  void setLithofractionId(int lithofractionId);

private:
  int wellId_;
  int lithofractionId_;
  double optimizedPercentageFirstComponent_;
  double optimizedFractionSecondComponent_;
};

} // namespace lithology

} // namespace sac

} // namespace casaWizard

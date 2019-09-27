// Struct for the lithofraction
#pragma once

#include <QStringList>

namespace casaWizard
{

namespace sac
{

class Lithofraction
{
public:
  Lithofraction() = default;
  explicit Lithofraction(const QString& layerName, const int firstComponent, const double minPercentageFirstComponent, const double maxPercentageFirstComponent,
                         const int secondComponent, const double minFractionSecondComponent, const double maxFractionSecondComponent);
  int version() const;
  static Lithofraction read(const int version, const QStringList& p);
  QStringList write() const;

  QString layerName() const;
  void setLayerName(const QString& value);

  int firstComponent() const;
  void setFirstComponent(int value);

  double minPercentageFirstComponent() const;
  void setMinPercentageFirstComponent(double value);

  double maxPercentageFirstComponent() const;
  void setMaxPercentageFirstComponent(double value);

  int secondComponent() const;
  void setSecondComponent(int value);

  double minFractionSecondComponent() const;
  void setMinFractionSecondComponent(double value);

  double maxFractionSecondComponent() const;
  void setMaxFractionSecondComponent(double value);

  int thirdComponent() const;

  static const QStringList percentNames;

private:
  QString layerName_;
  int firstComponent_;
  double minPercentageFirstComponent_;
  double maxPercentageFirstComponent_;
  int secondComponent_;
  double minFractionSecondComponent_;
  double maxFractionSecondComponent_;
};

} // namespace sac

} // namespace casaWizard

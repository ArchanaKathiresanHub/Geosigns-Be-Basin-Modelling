//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

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
                         const int secondComponent, const double minFractionSecondComponent, const double maxFractionSecondComponent, const bool doFirstOptimization = true,
                         const bool doSecondOptimization = true);
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

  bool doFirstOptimization() const;
  void setDoFirstOptimization(bool doFirstOptimization);

  bool doSecondOptimization() const;
  void setDoSecondOptimization(bool doSecondOptimization);

  static const QStringList percentNames;

private:
  QString layerName_;
  int firstComponent_;
  double minPercentageFirstComponent_;
  double maxPercentageFirstComponent_;
  int secondComponent_;
  double minFractionSecondComponent_;
  double maxFractionSecondComponent_;

  bool doFirstOptimization_;
  bool doSecondOptimization_;

};

} // namespace sac

} // namespace casaWizard

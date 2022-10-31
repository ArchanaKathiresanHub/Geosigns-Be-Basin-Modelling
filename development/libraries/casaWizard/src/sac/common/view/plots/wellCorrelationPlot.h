//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// Correlation plot of the simulated versus the measured data
#pragma once

#include "view/plots/plot.h"

namespace casaWizard
{

namespace sac
{

struct WellTrajectory;

class WellCorrelationPlot : public Plot
{
  Q_OBJECT

public:
  explicit WellCorrelationPlot(QWidget* parent = nullptr);

  void clear();
  void setAbsoluteErrorMargin(double absoluteErrorMargin);
  void setRelativeErrorMargin(double relativeErrorMargin);

  void setData(const QVector<QVector<double> >& measuredValueTrajectories, const QVector<QVector<double> >& simulatedValueTrajectories, const QString activeProperty, const QVector<bool> activePlots, const double minValue, const double maxValue, const QVector<int>& wellIndices);
signals:
  void selectedWell(int wellIndex);

protected:
  void paintEvent(QPaintEvent* event) override;
  void resizeEvent(QResizeEvent* event) override;
  void updateMinMaxData() override;

private slots:
  void selectedPoint(int plot, int point);

private:
  QStringList completeLegend_;
  QVector<int> wellIndices_;
  double minValue_;
  double maxValue_;
  double absoluteErrorMargin_;
  double relativeErrorMargin_;
  void drawAbsoluteErrorMargin(const double minValue, const double maxValue);
  void drawRelativeErrorMargin(const double minValue, const double maxValue);
  void drawDiagonal(const double minValue, const double maxValue);
};

}  // namespace sac

}  // namespace casaWizard

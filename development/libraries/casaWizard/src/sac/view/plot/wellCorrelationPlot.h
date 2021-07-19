//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// Correlation plot of the simulated versus the measured data
#pragma once

#include "model/trajectoryType.h"
#include "view/plot/plot.h"

namespace casaWizard
{

struct CalibrationTarget;

namespace sac
{

struct WellTrajectory;

class WellCorrelationPlot : public Plot
{
  Q_OBJECT

public:
  explicit WellCorrelationPlot(QWidget* parent = nullptr);

  void setData(const QVector<QVector<const CalibrationTarget*> >& targets,
               const QVector<QVector<WellTrajectory>>& allTrajectories,
               const QString property,
               const QVector<bool> activePlots);
  void clear();
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
};

}  // namespace sac

}  // namespace casaWizard

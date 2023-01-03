//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include "view/assets/SacMapToolTip.h"
#include <QWidget>

namespace QtCharts
{
  class QChart;
  class QChartView;
  class QPieSeries;
}

class QLabel;

namespace casaWizard
{

namespace sac
{

namespace lithology
{

class LithoMapsToolTip : public SacMapToolTip
{
  Q_OBJECT
public:
  explicit LithoMapsToolTip(QWidget *parent = nullptr);
  void setLithofractions(const std::vector<double>& lithofractions, const QString& wellName, const int activePlot);
  void setLithoNames(const QStringList& lithoNames);

private:
  std::vector<double> lithofractions_;

  QtCharts::QChart* pieChart_;
  QtCharts::QChartView* chartView_;
  QtCharts::QPieSeries* series_;
  QStringList lithoNames_;

  void setTotalLayout();
  void initializeChart();
  void initializeChartView();
  void initializePieChart();
  void updatePieSliceColors();
};

} // namespace lithology

} // namespace sac

} // namespace casaWizard

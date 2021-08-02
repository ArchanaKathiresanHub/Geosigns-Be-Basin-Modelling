//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

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

enum CornerPosition
{
  topLeft,
  topRight,
  bottomLeft,
  bottomRight
};


class LithoMapsToolTipOverlay : public QWidget
{
  Q_OBJECT
public:
  explicit LithoMapsToolTipOverlay(QWidget *parent = nullptr);

  CornerPosition cornerPosition_;
  void setCornerPosition(const CornerPosition& cornerPosition) {cornerPosition_ = cornerPosition;}
  void paintEvent(QPaintEvent *event) override;
};

class LithoMapsToolTip : public QWidget
{
  Q_OBJECT
public:
  explicit LithoMapsToolTip(QWidget *parent = nullptr);
  void setLithofractions(const std::vector<double>& lithofractions, const QString& wellName, const int activePlot);
  void setDomainPosition(const QPointF& domainPosition);
  void setCorner(const bool movedX, const bool movedY);
  void setLithoNames(const QStringList& lithoNames);

private:
  std::vector<double> lithofractions_;
  QPointF domainPosition_;

  QLabel* header_;
  QLabel* valueLabel_;
  QLabel* infoLabel_;
  LithoMapsToolTipOverlay* overlay_;
  QtCharts::QChart* pieChart_;
  QtCharts::QChartView* chartView_;
  QtCharts::QPieSeries* series_;
  QStringList lithoNames_;

  void setTotalLayout();
  void initializeChart();
  void initializeChartView();
  void initializeHeader();
  void initializePieChart();
  void initializeValueLabel();
  void initializeWellLabel();
  void updatePieSliceColors();
};

} // namespace sac

} // namespace casaWizard

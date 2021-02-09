//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// Widget to display 2D view of grids
#pragma once

#include "../common/view/plot/plot.h"
#include "wellBirdsView.h"

namespace casaWizard
{

namespace sac
{

class LithoMapsToolTip;
class ColorMap;
class OptimizedLithofraction;

class Grid2DView : public WellBirdsView
{
  Q_OBJECT

public:
  explicit Grid2DView(const ColorMap& colormap, QWidget* parent = nullptr);

  void clearData();
  void correctToolTipPositioning();

  void setFixedValueRange(const std::pair<double, double>& valueRange);
  void setOptimizedLithofractions(const QVector<OptimizedLithofraction>& optimizedLithofractions);
  void setStretch(const bool stretched);
  void setToolTipData(const std::vector<double>& lithofractionsAtPoint, const int activePlot);
  void setToolTipVisible(const bool visible);
  void setToolTipLithotypes(const QStringList& lithotypes);
  void setVariableValueRange();
  void setWellsVisible(const bool wellsVisible);

  const std::pair<double, double>& getValueRange() const;
  double getValue(const QPointF& point) const;

  void updatePlots(const QVector<QVector<double> > values);
  void updateRange(const double xMin, const double xMax, const double yMin, const double yMax);

  void drawData(QPainter &painter) override;
  void updateMinMaxData() override;
  void mousePressEvent(QMouseEvent* event) override;

private:
  const ColorMap& colorMap_;
  std::unique_ptr<std::pair<double, double>> range_;
  QVector<QVector<double>> values_;
  LithoMapsToolTip* lithoMapsToolTip_;
  QPoint mousePosition_;
  QVector<OptimizedLithofraction> optimizedLithofractions_;

  bool fixedRange_;
  bool stretched_;
  bool wellsVisible_;

  void determineRange();
  void initializeToolTip(const QPoint& mousePosition);
  void stretch();

  void drawPieChart(QPainter& painter, const int shade, const int size, const int counter, const OptimizedLithofraction& optimizedLitho);
  void drawPieChartsSelectedWells(QPainter& painter);
  void drawPieChartsWells(QPainter& painter);

signals:
  void toolTipCreated(const QPointF&);
};

} // namespace sac

} // namespace casaWizard

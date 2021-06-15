//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// Widget to display 2D view of grids
#pragma once

#include "../sac/view/plot/wellBirdsView.h"

#include "vector"

namespace casaWizard
{

namespace sac
{

class LithoMapsToolTip;
class ColorMap;
class OptimizedLithofraction;

class LithoPercent2DView : public WellBirdsView
{
  Q_OBJECT

public:
  explicit LithoPercent2DView(const ColorMap& colormap, QWidget* parent = nullptr);

  void correctToolTipPositioning();
  void setOptimizedLithofractions(const QVector<OptimizedLithofraction>& optimizedLithofractions);

  void setToolTipData(const std::vector<double>& lithofractionsAtPoint, const QString& wellName, const int activePlot);
  void setToolTipVisible(const bool visible);
  void setToolTipLithotypes(const QStringList& lithotypes);
  void setWellsVisible(const bool wellsVisible);  
  void moveTooltipToDomainLocation(const QPointF& domainLocation);
  void finalizeTooltip(const std::vector<double>& lithofractionsAtPoint, const QString& wellName, const int plotID);

  void drawData(QPainter &painter) override;  
  void mousePressEvent(QMouseEvent* event) override;

private:
  LithoMapsToolTip* lithoMapsToolTip_;
  QVector<OptimizedLithofraction> optimizedLithofractions_;

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

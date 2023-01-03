//
// Copyright (C) 2012-2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// Widget to display 2D view of grids
#pragma once

#include "view/plots/SacMap2DView.h"
#include "view/assets/lithoMapsToolTip.h"

#include "vector"

namespace casaWizard
{

class ColorMap;

namespace sac
{

class OptimizedLithofraction;

class LithoPercent2DView : public SacMap2DView
{
  Q_OBJECT

public:
  explicit LithoPercent2DView(const ColorMap& colormap, QWidget* parent = nullptr);

  void setOptimizedLithofractions(const QVector<OptimizedLithofraction>& optimizedLithofractions);

  void setToolTipData(const std::vector<double>& lithofractionsAtPoint, const QString& wellName, const int activePlot);
  void setToolTipLithotypes(const QStringList& lithotypes);
  void finalizeTooltip(const std::vector<double>& lithofractionsAtPoint, const QString& wellName, const int plotID);

  void drawData(QPainter& painter) override;

private:
  LithoMapsToolTip* mapToolTip() final;
  LithoMapsToolTip* m_lithoMapsToolTip;

  QVector<OptimizedLithofraction> m_optimizedLithofractions;

  void drawPieChart(QPainter& painter, const int shade, const int size, const int counter, const OptimizedLithofraction& optimizedLitho);
  void drawPieChartsSelectedWells(QPainter& painter);
  void drawPieChartsWells(QPainter& painter);
};

} // namespace sac

} // namespace casaWizard

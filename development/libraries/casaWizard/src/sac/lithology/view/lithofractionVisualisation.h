//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include <QWidget>
#include <memory>
#include "view/colormap.h"

class QPushButton;
class QComboBox;
class QCheckBox;
class QGridLayout;
class QLabel;

namespace casaWizard
{

class CustomCheckbox;
class Well;

namespace sac
{
class Grid2DPlot;
class OptimizedLithofraction;
class MapPlotOptions;

class LithofractionVisualisation : public QWidget
{
  Q_OBJECT
public:
  explicit LithofractionVisualisation(QWidget *parent = nullptr);

  std::vector<Grid2DPlot*> lithoFractionPlots() const;

  const QComboBox* layerSelection() const;
  QComboBox* lithotypeSelection() const;
  CustomCheckbox* singleMapLayout() const;
  QCheckBox* wellsVisible() const;

  void clearPlots();
  void updateBirdsView(const QVector<const Well*> wells, const QVector<OptimizedLithofraction>& optimizedLithoFractions);
  void updateLayerOptions(QStringList availableLayers);
  void updateSelectedWells(const QVector<int> selectedWells);
  void updateMapLayout(const bool singleMapLayout);
  void refreshCurrentPercentageRange();
  void hideAllTooltips();
  void finalizeTooltip(const std::vector<double>& lithofractionsAtPoint, const QString& wellName, const int plotID);

private:
  std::unique_ptr<ColorMap> colormap_;
  std::vector<Grid2DPlot*> lithoFractionPlots_;
  MapPlotOptions* plotOptions_;
  QGridLayout* plotsAndOptions_;
  QLabel* lithoSelectionLabel_;

  void setTotalLayout();
  void initializeLithoFractionPlots();
  void connectSignalsAndSlots();

  std::pair<double, double> getGlobalRange();
  void setThreePlotLayout();
  void setOnePlotLayout(const int activePlot);

signals:
  void updateAvailableLayers();

public slots:
  void setColorMapType(const std::string& colorMapType);
  void slotUpdatePercentageRanges(const QString& percentageRangeType);

private slots:
  void slotUpdateAspectRatio(int state);
  void slotUpdateColorMaps(const QString& colormapType);
  void slotUpdateWellsVisibility(int state);
  void slotUpdateActivePlot(int activePlot);
};

}
}

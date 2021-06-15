//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include "colormap.h"

#include <QWidget>

#include <memory>

class QPushButton;
class QComboBox;
class QCheckBox;
class QGridLayout;

namespace casaWizard
{

class CustomCheckbox;
class Well;

namespace sac
{

class ColorBar;
class Grid2DPlot;
class OptimizedLithofraction;

class LithofractionVisualisation : public QWidget
{
  Q_OBJECT
public:
  explicit LithofractionVisualisation(QWidget *parent = nullptr);

  std::vector<Grid2DPlot*> lithoFractionPlots() const;

  const QComboBox* layerSelection() const;
  CustomCheckbox* singleMapLayout() const;
  QComboBox* colorMapSelection() const;
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
  QComboBox* percentageRange_;
  std::unique_ptr<ColorMap> colormap_;
  QComboBox* colorMapSelection_;
  QComboBox* layerSelection_;
  std::vector<Grid2DPlot*> lithoFractionPlots_;
  QWidget* plotOptions_;
  CustomCheckbox* stretched_;
  CustomCheckbox* wellsVisible_;
  CustomCheckbox* singleMapLayout_;
  QGridLayout* plotsAndOptions_;

  void setPlotOptionsLayout();
  void setTotalLayout();
  void initializeLithoFractionPlots();
  void connectSignalsAndSlots();

  std::pair<double, double> getGlobalRange();
  void setThreePlotLayout();
  void setOnePlotLayout();

signals:
  void updateAvailableLayers();

public slots:
  void setColorMapType(const std::string& colorMapType);
  void slotUpdatePercentageRanges(const QString& percentageRangeType);

private slots:
  void slotUpdateAspectRatio(int state);
  void slotUpdateColorMaps(const QString& colormapType);
  void slotUpdateWellsVisibility(int state);
};

}

}

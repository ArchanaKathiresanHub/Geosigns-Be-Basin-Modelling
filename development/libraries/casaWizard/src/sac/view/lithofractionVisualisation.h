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
  QComboBox* colorMapSelection() const;
  QCheckBox* wellsVisible() const;

  void clearPlots();
  void updateBirdsView(const QVector<const Well*> wells, const QVector<OptimizedLithofraction>& optimizedLithoFractions);
  void updateLayerOptions(QStringList availableLayers);
  void updateSelectedWells(const QVector<int> selectedWells);

private:
  QComboBox* percentageRange_;
  std::unique_ptr<ColorMap> colormap_;
  QComboBox* colorMapSelection_;
  QComboBox* layerSelection_;
  std::vector<Grid2DPlot*> lithoFractionPlots_;
  QWidget* plotOptions_;
  CustomCheckbox* stretched_;
  CustomCheckbox* wellsVisible_;

  void setPlotOptionsLayout();
  void setTotalLayout();
  void initializeLithoFractionPlots();
  void connectSignalsAndSlots();

  std::pair<double, double> getGlobalRange();
signals:
  void updateAvailableLayers();

public slots:
  void setColorMapType(const std::string& colorMapType);

private slots:
  void slotUpdateAspectRatio(int state);
  void slotUpdateColorMaps(const QString& colormapType);
  void slotUpdateWellsVisibility(int state);
  void slotUpdatePercentageRanges(const QString& percentageRangeType);
};

}

}

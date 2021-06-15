//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "lithofractionVisualisation.h"

#include "colorbar.h"
#include "grid2dplot.h"
#include "plot/lithoPercent2Dview.h"
#include "../common/view/components/customcheckbox.h"
#include "../common/view/components/customtitle.h"
#include "model/well.h"
#include "model/optimizedLithofraction.h"

#include <QLayout>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>

namespace casaWizard
{

namespace sac
{

LithofractionVisualisation::LithofractionVisualisation(QWidget *parent) :
  QWidget(parent),
  percentageRange_(new QComboBox(this)),
  colormap_{new ColorMap},
  colorMapSelection_{new QComboBox(this)},
  layerSelection_{new QComboBox(this)},
  plotOptions_{new QWidget(this)},
  stretched_{new CustomCheckbox(this)},
  wellsVisible_{new CustomCheckbox(this)},
  singleMapLayout_{new CustomCheckbox(this)},
  plotsAndOptions_{new QGridLayout(this)}
{
  initializeLithoFractionPlots();
  setPlotOptionsLayout();
  setTotalLayout();

  connectSignalsAndSlots();
}

void LithofractionVisualisation::initializeLithoFractionPlots()
{
  for (int i = 0 ; i < 3; i++)
  {
    lithoFractionPlots_.push_back(new Grid2DPlot(*colormap_, this));
  }
}

void LithofractionVisualisation::setPlotOptionsLayout()
{
  colorMapSelection_->insertItems(0, {"Viridis", "Gray scale", "Rainbow"});
  wellsVisible_->setCheckState(Qt::CheckState::Checked);
  stretched_->setCheckState(Qt::CheckState::Unchecked);
  singleMapLayout_->setCheckState(Qt::CheckState::Unchecked);

  percentageRange_->insertItems(0, {"Variable", "Fixed between 0 and 100", "Fixed between global min and max"});

  QGridLayout* plotOptionsLayout = new QGridLayout(plotOptions_);

  CustomTitle* plotOptionsLabel = new CustomTitle(" Plot Options ", this);
  plotOptionsLayout->addWidget(plotOptionsLabel, 0, 0);
  plotOptionsLayout->addWidget(new QLabel(" Selected layer: ", this), 1, 0);
  plotOptionsLayout->addWidget(layerSelection_, 1, 1);
  plotOptionsLayout->addWidget(new QLabel(" Color map: ", this), 2, 0);
  plotOptionsLayout->addWidget(colorMapSelection_, 2, 1);

  plotOptionsLayout->addWidget(new QLabel(" Wells visible: ", this), 3, 0);
  plotOptionsLayout->addWidget(wellsVisible_, 3, 1);

  plotOptionsLayout->addWidget(new QLabel(" Stretched: ", this), 4, 0);
  plotOptionsLayout->addWidget(stretched_, 4, 1);

  plotOptionsLayout->addWidget(new QLabel(" Single map layout: ", this), 5, 0);
  plotOptionsLayout->addWidget(singleMapLayout_, 5, 1);

  plotOptionsLayout->addWidget(new QLabel(" Percentage range: ", this), 6, 0);
  plotOptionsLayout->addWidget(percentageRange_, 6, 1);

  plotOptions_->setMaximumWidth(400);
  plotOptions_->setMaximumHeight(165);
}

void LithofractionVisualisation::setTotalLayout()
{
  setThreePlotLayout();

  QSizePolicy retainSize = sizePolicy();
  retainSize.setRetainSizeWhenHidden(true);
  setSizePolicy(retainSize);

  QPalette pal = palette();
  pal.setColor(QPalette::Background, Qt::white);
  setAutoFillBackground(true);
  setPalette(pal);
}

void LithofractionVisualisation::setThreePlotLayout()
{
  delete plotsAndOptions_;
  plotsAndOptions_ = new QGridLayout(this);

  plotsAndOptions_->addWidget(lithoFractionPlots_[0], 0, 0);
  plotsAndOptions_->addWidget(lithoFractionPlots_[1], 0, 1);
  plotsAndOptions_->addWidget(lithoFractionPlots_[2], 1, 0);
  plotsAndOptions_->addWidget(plotOptions_, 1, 1, Qt::Alignment(Qt::AlignmentFlag::AlignTop));

  plotsAndOptions_->setMargin(0);
}

void LithofractionVisualisation::connectSignalsAndSlots()
{
  connect(colorMapSelection_, SIGNAL(currentTextChanged(const QString&)), this, SLOT(slotUpdateColorMaps(const QString&)));
  connect(wellsVisible_, SIGNAL(stateChanged(int)), this, SLOT(slotUpdateWellsVisibility(int)));
  connect(stretched_, SIGNAL(stateChanged(int)), this, SLOT(slotUpdateAspectRatio(int)));
  connect(percentageRange_, SIGNAL(currentTextChanged(const QString&)), this, SLOT(slotUpdatePercentageRanges(const QString&)));
}

void LithofractionVisualisation::slotUpdateColorMaps(const QString& colormapType)
{
  setColorMapType(colormapType.toStdString());

  for (Grid2DPlot* plot : lithoFractionPlots_)
  {
    update();
    plot->lithoPercent2DView()->update();
    plot->lithoPercent2DView()->setToolTipVisible(false);
  }
}

void LithofractionVisualisation::slotUpdateWellsVisibility(int state)
{
  for (Grid2DPlot* plot : lithoFractionPlots_)
  {
    plot->lithoPercent2DView()->setWellsVisible(state == Qt::CheckState::Checked);
    plot->lithoPercent2DView()->setToolTipVisible(false);
  }
}

void LithofractionVisualisation::slotUpdateAspectRatio(int state)
{
  for (Grid2DPlot* plot : lithoFractionPlots_)
  {
    plot->lithoPercent2DView()->setStretch(state == Qt::CheckState::Checked);
    plot->lithoPercent2DView()->setToolTipVisible(false);
  }
}

void LithofractionVisualisation::slotUpdatePercentageRanges(const QString& percentageRangeType)
{
  if (percentageRangeType == "Variable")
  {
    for (Grid2DPlot* plot : lithoFractionPlots_)
    {
      plot->lithoPercent2DView()->setVariableValueRange();
      plot->updateColorBar();
      plot->lithoPercent2DView()->setToolTipVisible(false);
    }
  }
  else if (percentageRangeType == "Fixed between 0 and 100")
  {
    for (Grid2DPlot* plot : lithoFractionPlots_)
    {
      plot->lithoPercent2DView()->setFixedValueRange({0,100});
      plot->updateColorBar();
      plot->lithoPercent2DView()->setToolTipVisible(false);
    }
  }
  else if (percentageRangeType == "Fixed between global min and max")
  {
    for (Grid2DPlot* plot : lithoFractionPlots_)
    {
      plot->lithoPercent2DView()->setVariableValueRange();
    }

    std::pair<double, double> globalRange = getGlobalRange();

    for (Grid2DPlot* plot : lithoFractionPlots_)
    {
      plot->lithoPercent2DView()->setFixedValueRange(globalRange);
      plot->updateColorBar();
      plot->lithoPercent2DView()->setToolTipVisible(false);
    }
  }
}

void LithofractionVisualisation::updateMapLayout(const bool singleMapLayout)
{
  if (singleMapLayout)
  {
    setOnePlotLayout();
  }
  else
  {
    setThreePlotLayout();

    lithoFractionPlots_[1]->setVisible(true);
    lithoFractionPlots_[2]->setVisible(true);
  }

  hideAllTooltips();
}

void LithofractionVisualisation::refreshCurrentPercentageRange()
{
  slotUpdatePercentageRanges(percentageRange_->currentText());
}

void LithofractionVisualisation::hideAllTooltips()
{
  for (const Grid2DPlot* lithoFractionPlot : lithoFractionPlots())
  {
    lithoFractionPlot->lithoPercent2DView()->setToolTipVisible(false);
  }
}

void LithofractionVisualisation::finalizeTooltip(const std::vector<double>& lithofractionsAtPoint, const QString& wellName, const int plotID)
{
  lithoFractionPlots()[plotID]->lithoPercent2DView()->finalizeTooltip(lithofractionsAtPoint, wellName, plotID);
}

void LithofractionVisualisation::setOnePlotLayout()
{
  delete plotsAndOptions_;
  plotsAndOptions_ = new QGridLayout(this);

  plotsAndOptions_->addWidget(lithoFractionPlots_[0], 0, 0);
  plotsAndOptions_->addWidget(plotOptions_, 0, 1, Qt::Alignment(Qt::AlignmentFlag::AlignTop));

  lithoFractionPlots_[1]->setVisible(false);
  lithoFractionPlots_[2]->setVisible(false);

  plotsAndOptions_->setMargin(0);
}

std::pair<double, double> LithofractionVisualisation::getGlobalRange()
{
  std::pair<double, double> globalRange = lithoFractionPlots_[0]->lithoPercent2DView()->getValueRange();

  for (Grid2DPlot* plot : lithoFractionPlots_)
  {
    std::pair<double, double> range = plot->lithoPercent2DView()->getValueRange();
    globalRange.first = range.first < globalRange.first ? range.first : globalRange.first;
    globalRange.second = range.second > globalRange.second ? range.second : globalRange.second;
  }

  return globalRange;
}

std::vector<Grid2DPlot*> LithofractionVisualisation::lithoFractionPlots() const
{
  return lithoFractionPlots_;
}

void LithofractionVisualisation::updateLayerOptions(QStringList availableLayers)
{
  QSignalBlocker blocker(this);
  layerSelection_->clear();
  for (QString availableLayer: availableLayers)
  {
    layerSelection_->addItem(availableLayer);
  }
}

const QComboBox* LithofractionVisualisation::layerSelection() const
{
  return layerSelection_;
}

CustomCheckbox* LithofractionVisualisation::singleMapLayout() const
{
  return singleMapLayout_;
}

QComboBox* LithofractionVisualisation::colorMapSelection() const
{
  return colorMapSelection_;
}

QCheckBox* LithofractionVisualisation::wellsVisible() const
{
  return wellsVisible_;
}

void LithofractionVisualisation::clearPlots()
{
  for (Grid2DPlot* plot : lithoFractionPlots_)
  {
    plot->lithoPercent2DView()->clearData();
    plot->updateColorBar();
  }
}

void LithofractionVisualisation::setColorMapType(const std::string& colorMapType)
{
  colormap_->setColorMapType(colorMapType);
}

void LithofractionVisualisation::updateBirdsView(const QVector<const Well*> wells, const QVector<OptimizedLithofraction>& optimizedLithoFractions)
{
  QVector<double> x;
  QVector<double> y;
  for (const Well *const well : wells)
  {
    x.append(well->x());
    y.append(well->y());
  }

  for (Grid2DPlot* plot: lithoFractionPlots())
  {
    plot->lithoPercent2DView()->setWellLocations(x, y);
    plot->lithoPercent2DView()->setOptimizedLithofractions(optimizedLithoFractions);
  }
}

void LithofractionVisualisation::updateSelectedWells(const QVector<int> selectedWells)
{
  for (Grid2DPlot* plot: lithoFractionPlots_)
  {
    plot->lithoPercent2DView()->setSelectedWells(selectedWells);
  }
}

} // namespace sac

} // namespace casaWizard


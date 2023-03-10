//
// Copyright (C) 2012-2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "lithofractionVisualisation.h"

#include "LithoGrid2DPlot.h"
#include "view/sharedComponents/customcheckbox.h"
#include "model/well.h"

#include <QGridLayout>
#include <QLabel>
#include <QComboBox>

namespace casaWizard
{

namespace sac
{

namespace lithology
{

LithofractionVisualisation::LithofractionVisualisation(QWidget *parent) :
   SacMapsVisualisation(parent),
   plotOptions_{new MapPlotOptionsLithofraction(this)},
   plotsAndOptions_{new QGridLayout(this)}
{
   initializeLithoFractionPlots();
   setTotalLayout();
   connectSignalsAndSlots();
}

void LithofractionVisualisation::initializeLithoFractionPlots()
{
   for (int i = 0 ; i < 3; i++)
   {
      lithoFractionPlots_.push_back(new LithoGrid2DPlot(colormap(), this));
   }
}

const MapPlotOptionsLithofraction* LithofractionVisualisation::plotOptions() const
{
   return plotOptions_;
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
   connect(plotOptions_->colorMapSelection(), SIGNAL(currentTextChanged(const QString&)), this, SLOT(slotUpdateColorMaps(const QString&)));
   connect(plotOptions_->lithotypeSelection(), SIGNAL(currentIndexChanged(int)), this, SLOT(slotUpdateActivePlot(int)));
   connect(plotOptions_->wellsVisible(), SIGNAL(stateChanged(int)), this, SLOT(slotUpdateWellsVisibility(int)));
   connect(plotOptions_->stretched(), SIGNAL(stateChanged(int)), this, SLOT(slotUpdateAspectRatio(int)));
   connect(plotOptions_->percentageRange(), SIGNAL(currentTextChanged(const QString&)), this, SLOT(slotUpdatePercentageRanges(const QString&)));
}

void LithofractionVisualisation::slotUpdateColorMaps(const QString& colormapType)
{
   setColorMapType(colormapType.toStdString());
   hideAllTooltips();

   for (LithoGrid2DPlot* plot : lithoFractionPlots_)
   {
      plot->refresh();
   }
}

void LithofractionVisualisation::slotUpdateWellsVisibility(int state)
{
   for (LithoGrid2DPlot* plot : lithoFractionPlots_)
   {
      plot->get2DView()->setWellsVisible(state == Qt::CheckState::Checked);
   }
}

void LithofractionVisualisation::slotUpdateActivePlot(int activePlot)
{
   if (plotOptions_->singleMapLayout()->checkState() == Qt::CheckState::Checked && !(activePlot<0))
   {
      setOnePlotLayout(activePlot);
      hideAllTooltips();
   }
}

void LithofractionVisualisation::slotUpdateAspectRatio(int state)
{
   for (LithoGrid2DPlot* plot : lithoFractionPlots_)
   {
      plot->get2DView()->setStretch(state == Qt::CheckState::Checked);
   }

   hideAllTooltips();
}

void LithofractionVisualisation::slotUpdatePercentageRanges(const QString& percentageRangeType)
{
   if (percentageRangeType == "Variable")
   {
      for (LithoGrid2DPlot* plot : lithoFractionPlots_)
      {
         plot->get2DView()->setVariableValueRange();
         plot->updateColorBar(2);
      }
   }
   else if (percentageRangeType == "Fixed between 0 and 100")
   {
      for (LithoGrid2DPlot* plot : lithoFractionPlots_)
      {
         plot->get2DView()->setFixedValueRange({0,100});
         plot->updateColorBar(0);
      }
   }
   else if (percentageRangeType == "Fixed between global min and max")
   {
      for (LithoGrid2DPlot* plot : lithoFractionPlots_)
      {
         plot->get2DView()->setVariableValueRange();
      }

      std::pair<double, double> globalRange = getGlobalRange();

      for (LithoGrid2DPlot* plot : lithoFractionPlots_)
      {
         plot->get2DView()->setFixedValueRange(globalRange);
         plot->updateColorBar(2);
      }
   }

   hideAllTooltips();
}

void LithofractionVisualisation::updateMapLayout(const bool singleMapLayout)
{
   if (singleMapLayout)
   {
      setOnePlotLayout(lithotypeSelection()->currentIndex());
   }
   else
   {
      setThreePlotLayout();

      for (LithoGrid2DPlot* plot : lithoFractionPlots_)
      {
         plot->setVisible(true);
      }
   }

   hideAllTooltips();
}

void LithofractionVisualisation::refreshCurrentPercentageRange()
{
   slotUpdatePercentageRanges(plotOptions_->percentageRange()->currentText());
}

void LithofractionVisualisation::hideAllTooltips()
{
   for (const LithoGrid2DPlot* lithoFractionPlot : lithoFractionPlots())
   {
      lithoFractionPlot->get2DView()->setToolTipVisible(false);
   }
}

void LithofractionVisualisation::finalizeTooltip(const std::vector<double>& lithofractionsAtPoint, const QString& wellName, const int plotID)
{
   lithoFractionPlots_[plotID]->get2DView()->finalizeTooltip(lithofractionsAtPoint, wellName, plotID);
}

void LithofractionVisualisation::setOnePlotLayout(const int activePlot)
{
   delete plotsAndOptions_;
   plotsAndOptions_ = new QGridLayout(this);

   plotsAndOptions_->addWidget(lithoFractionPlots_[activePlot], 0, 0);
   plotsAndOptions_->addWidget(plotOptions_, 0, 1, Qt::Alignment(Qt::AlignmentFlag::AlignTop));

   for (int i = 0; i < 3; i++)
   {
      lithoFractionPlots_[i]->setVisible(i == activePlot);
   }
   plotsAndOptions_->setMargin(0);
}

std::pair<double, double> LithofractionVisualisation::getGlobalRange()
{
   std::pair<double, double> globalRange = lithoFractionPlots_[0]->get2DView()->getValueRange();

   for (LithoGrid2DPlot* plot : lithoFractionPlots_)
   {
      std::pair<double, double> range = plot->get2DView()->getValueRange();
      globalRange.first = range.first < globalRange.first ? range.first : globalRange.first;
      globalRange.second = range.second > globalRange.second ? range.second : globalRange.second;
   }
   return globalRange;
}

std::vector<LithoGrid2DPlot*> LithofractionVisualisation::lithoFractionPlots() const
{
   return lithoFractionPlots_;
}

void LithofractionVisualisation::updateLayerOptions(QStringList availableLayers)
{
   QSignalBlocker blocker(this);
   plotOptions_->layerSelection()->clear();
   for (const QString& availableLayer: availableLayers)
   {
      plotOptions_->layerSelection()->addItem(availableLayer);
   }
}

const QComboBox* LithofractionVisualisation::layerSelection() const
{
   return plotOptions_->layerSelection();
}

QComboBox* LithofractionVisualisation::lithotypeSelection() const
{
   return plotOptions_->lithotypeSelection();
}

CustomCheckbox* LithofractionVisualisation::singleMapLayout() const
{
   return plotOptions_->singleMapLayout();
}

void LithofractionVisualisation::clearPlots()
{
   for (LithoGrid2DPlot* plot : lithoFractionPlots_)
   {
      plot->get2DView()->clearData();
      plot->updateColorBar();
   }
}

void LithofractionVisualisation::setColorMapType(const std::string& colorMapType)
{
   colormap().setColorMapType(colorMapType);
   for (LithoGrid2DPlot* plot : lithoFractionPlots_)
   {
      plot->setColorBarMap(colormap());
   }
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

   for (LithoGrid2DPlot* plot: lithoFractionPlots())
   {
      plot->get2DView()->setWellLocations(x, y);
      plot->get2DView()->setOptimizedLithofractions(optimizedLithoFractions);
   }
}

void LithofractionVisualisation::updateSelectedWells(const QVector<int> selectedWells)
{
   for (LithoGrid2DPlot* plot: lithoFractionPlots_)
   {
      plot->get2DView()->setSelectedWells(selectedWells);
   }
}

} // namespace lithology

} // namespace sac

} // namespace casaWizard


//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "TCHPVisualisation.h"

#include "view/plots/ThermalGrid2DPlot.h"
#include "view/sharedComponents/customcheckbox.h"
#include "model/well.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QComboBox>

namespace casaWizard
{

namespace sac
{

namespace thermal
{

TCHPVisualisation::TCHPVisualisation(QWidget *parent) :
   SacMapsVisualisation(parent),
   m_plotOptions{new ThermalMapPlotOptions(this)},
   m_plotsAndOptions{new QVBoxLayout(this)},
   m_TCHPPlot{new ThermalGrid2DPlot(colormap(), this)}
{
   connect(m_plotOptions->colorMapSelection(), SIGNAL(currentTextChanged(const QString&)), this, SLOT(slotUpdateColorMaps(const QString&)));
   connect(m_plotOptions->wellsVisible(), SIGNAL(stateChanged(int)), this, SLOT(slotUpdateWellsVisibility(int)));
   connect(m_plotOptions->stretched(), SIGNAL(stateChanged(int)), this, SLOT(slotUpdateAspectRatio(int)));

   m_plotsAndOptions->addWidget(m_plotOptions, Qt::Alignment(Qt::AlignmentFlag::AlignLeft));
   m_plotsAndOptions->addWidget(m_TCHPPlot);
   m_plotsAndOptions->setMargin(0);

   QSizePolicy retainSize = sizePolicy();
   retainSize.setRetainSizeWhenHidden(true);
   setSizePolicy(retainSize);

   QPalette pal = palette();
   pal.setColor(QPalette::Background, Qt::white);
   setAutoFillBackground(true);
   setPalette(pal);
}

const ThermalMapPlotOptions* TCHPVisualisation::plotOptions() const
{
   return m_plotOptions;
}

void TCHPVisualisation::slotUpdateColorMaps(const QString& colormapType)
{
   setColorMapType(colormapType.toStdString());
   hideAllTooltips();
   m_TCHPPlot->refresh();
}

void TCHPVisualisation::slotUpdateWellsVisibility(int state)
{
   m_TCHPPlot->get2DView()->setWellsVisible(state == Qt::CheckState::Checked);
}

void TCHPVisualisation::slotUpdateAspectRatio(int state)
{
   m_TCHPPlot->get2DView()->setStretch(state == Qt::CheckState::Checked);
   hideAllTooltips();
}

void TCHPVisualisation::hideAllTooltips()
{
      m_TCHPPlot->get2DView()->setToolTipVisible(false);

}

void TCHPVisualisation::finalizeTooltip(const double& TCHPAtPoint, const QString& wellName)
{
   m_TCHPPlot->get2DView()->finalizeTooltip(TCHPAtPoint, wellName);
}

ThermalGrid2DPlot* TCHPVisualisation::TCHPPlot() const
{
   return m_TCHPPlot;
}

void TCHPVisualisation::clearPlots()
{
   m_TCHPPlot->get2DView()->clearData();
   m_TCHPPlot->updateColorBar();
}

void TCHPVisualisation::setColorMapType(const std::string& colorMapType)
{
   colormap().setColorMapType(colorMapType);
   m_TCHPPlot->setColorBarMap(colormap());

}

void TCHPVisualisation::updateBirdsView(const QVector<const Well*> wells, const QVector<OptimizedTCHP>& optimizedTCHPs)
{
   QVector<double> x;
   QVector<double> y;
   for (const Well *const well : wells)
   {
      x.append(well->x());
      y.append(well->y());
   }

   m_TCHPPlot->get2DView()->setWellLocations(x, y);
   m_TCHPPlot->get2DView()->setOptimizedTCHPs(optimizedTCHPs);
}

void TCHPVisualisation::updateSelectedWells(const QVector<int> selectedWells)
{
   m_TCHPPlot->get2DView()->setSelectedWells(selectedWells);
}

} // namespace themal

} // namespace sac

} // namespace casaWizard


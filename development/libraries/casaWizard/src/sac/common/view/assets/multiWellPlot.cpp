//
// Copyright (C) 2012-2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "multiWellPlot.h"

#include "model/calibrationTarget.h"
#include "model/trajectoryType.h"
#include "model/wellTrajectory.h"

#include "view/plots/wellPlot.h"
#include "view/sharedComponents/customcheckbox.h"
#include "view/plots/legend.h"

#include <QGridLayout>
#include <QLabel>
#include <QMap>
#include <QString>
#include <QFrame>

namespace casaWizard
{

namespace sac
{

MultiWellPlot::MultiWellPlot(QWidget* parent) :
   QWidget(parent),
   m_plots{},
   m_expandedPlots{},
   m_isExpandedCheckboxes{},
   m_plotSelection{new QWidget(this)},
   m_legend{},
   m_showSurfaceLines{new CustomCheckbox("Show surface lines", this)},
   m_fitRangeToData{new CustomCheckbox("Fit range to well data", this)}
{  
   setLayout(new QGridLayout());

   connect(m_showSurfaceLines, SIGNAL(stateChanged(int)), this, SLOT(slotShowSurfaceLinesChanged(int)));
   connect(m_fitRangeToData, SIGNAL(stateChanged(int)), this, SLOT(slotFitRangeToDataChanged(int)));
}

void MultiWellPlot::slotShowSurfaceLinesChanged(int state)
{
   emit showSurfaceLinesChanged(state == Qt::CheckState::Checked);
}

void MultiWellPlot::slotFitRangeToDataChanged(int state)
{
   emit fitRangeToDataChanged(state == Qt::CheckState::Checked);
}

void MultiWellPlot::setActivePropertyUserNames(const QStringList& activePropertyUserNames)
{
   activePropertyUserNames_ = activePropertyUserNames;

   const int nProperties = activePropertyUserNames.size();

   // Reset the selection if the size has changed, default of 4 plots maximum
   if (m_expandedPlots.size() != nProperties)
   {
      m_expandedPlots = QVector<bool>(nProperties, true);
      for (int i = 4; i< nProperties; ++i)
      {
         m_expandedPlots[i] = false;
      }
   }

   createPlotSelectionHeader();
}

void MultiWellPlot::createPlotSelectionHeader()
{
   QSignalBlocker blocker(this);

   delete m_plotSelection;
   m_isExpandedCheckboxes.clear();

   m_plotSelection = new QWidget(this);
   QGridLayout* plotSelectionLayout = new QGridLayout(m_plotSelection);
   plotSelectionLayout->addWidget(new QLabel("Plot selection: ", m_plotSelection), 0, 0);

   const int maxPerRow = 5;
   int row = 0;
   int col = 1;
   for (int i = 0; i < activePropertyUserNames_.size(); i++)
   {
      CustomCheckbox* isExpandedCheckbox = new CustomCheckbox(activePropertyUserNames_[i]);
      isExpandedCheckbox->setChecked(m_expandedPlots[i]);
      connect(isExpandedCheckbox, &CustomCheckbox::stateChanged, [=](int state){emit isExpandedChanged(state, i);});

      plotSelectionLayout->addWidget(isExpandedCheckbox, row, col);
      if (col == maxPerRow)
      {
         col = 0;
         row++;
      }
      col++;

      m_isExpandedCheckboxes.push_back(isExpandedCheckbox);
   }
}

void MultiWellPlot::setShowSurfaceLines(const bool checked)
{
   m_showSurfaceLines->setChecked(checked);
}

void MultiWellPlot::setFitRangeToData(const bool checked)
{
   m_fitRangeToData->setChecked(checked);
}

void MultiWellPlot::setTotalLayout()
{
   QGridLayout* total = static_cast<QGridLayout*>(layout());
   total->addLayout(createPlotOptionsAndLegendLayout(), 0, 0, 1, m_plots.size());
   total->addWidget(m_plotSelection, 1, 0, 1, m_plots.size(), Qt::AlignLeft);

   for (int i = 0; i < m_plots.size(); i++)
   {
      total->addWidget(m_plots[i], 2, i, 1, 1);
      total->setMargin(0);
      total->setSpacing(0);
   }

   total->setRowStretch(0,0);
   total->setRowStretch(1,0);
   total->setRowStretch(2,1);
}

void MultiWellPlot::updatePlots(const QVector<QVector<const CalibrationTarget*>>& targets,
                                const QStringList& units,
                                const QVector<QVector<WellTrajectory>>& allTrajectories,
                                const QVector<bool>& activePlots,
                                const QMap<QString, double>& surfaceLines, const bool fitRangeToData)
{
   clearState();
   drawPlots(targets, units, allTrajectories, activePlots, surfaceLines, fitRangeToData);
   setTotalLayout();
}

void MultiWellPlot::clearState()
{
   clearPlots();
   clearLegend();
}

void MultiWellPlot::clearPlots()
{
   for (WellPlot* plot : m_plots)
   {
      delete plot;
   }
   m_plots.clear();
}

void MultiWellPlot::clearLegend()
{
   if (m_legend)
   {
      delete m_legend;
      m_legend = nullptr;
   }
}

void MultiWellPlot::drawPlots(const QVector<QVector<const CalibrationTarget*>>& targets,
                              const QStringList& units,
                              const QVector<QVector<WellTrajectory>>& allTrajectories,
                              const QVector<bool>& activePlots,
                              const QMap<QString, double>& surfaceLines, const bool fitRangeToData)
{
   const int nTypes = allTrajectories.size();

   std::pair<double, double> zTotalRange;

   for ( int iProperty = 0 ; iProperty < activePropertyUserNames_.size(); ++iProperty)
   {
      if (!m_expandedPlots[iProperty])
      {
         continue;
      }
      WellPlot* plot = new WellPlot(this);
      m_plots.push_back(plot);

      plot->setFitRangeToWellData(fitRangeToData);
      for (const QString& surfaceLine : surfaceLines.keys())
      {
         plot->drawSurfaceLine(surfaceLine, surfaceLines[surfaceLine]);
      }
      const QString activePropertyName = activePropertyUserNames_[iProperty];
      plot->setXLabel(activePropertyName);

      int iTargetProperty = 0;
      for (const QVector<const CalibrationTarget*>& target : targets)
      {
         if (target[0]->propertyUserName() == activePropertyName)
         {
            QVector<WellTrajectory> plotTrajectories(nTypes, {});
            for (int i = 0; i < nTypes; i++)
            {
               plotTrajectories[i] = allTrajectories[i][iTargetProperty];
            }

            plot->setData(target, plotTrajectories, activePlots);
            for (const QString& surfaceLine : surfaceLines.keys())
            {
               plot->drawSurfaceLine(surfaceLine, surfaceLines[surfaceLine]);
            }
            plot->setXLabel(activePropertyName + " [" + units[iTargetProperty] + "]" );

            std::pair<double, double> zDataRange = plot->zDataRange();
            if (iProperty == 0)
            {
               zTotalRange = zDataRange;
            }
            else
            {
               if (zDataRange.first < zTotalRange.first) zTotalRange.first = zDataRange.first;
               if (zDataRange.second < zTotalRange.second) zTotalRange.second = zDataRange.second;
            }
            break;
         }
         iTargetProperty++;
      }
   }

   for (WellPlot* plot : m_plots)
   {
      plot->setZDataRange(zTotalRange);
      plot->updateMinMaxData();
   }
}

QHBoxLayout* MultiWellPlot::createPlotOptionsAndLegendLayout()
{
   QVBoxLayout* plotOptions = new QVBoxLayout();
   plotOptions->addWidget(m_showSurfaceLines);
   plotOptions->addWidget(m_fitRangeToData);

   QHBoxLayout* plotOptionsAndLegendLayout = new QHBoxLayout();
   plotOptionsAndLegendLayout->addLayout(plotOptions, 0);
   if (m_plots.size() > 0)
   {
      for ( WellPlot* wellPlot : m_plots )
      {
         if (wellPlot->containsData()) // Some plots might not have any data as the property is not calibrated for this well
         {
            m_legend = new Legend(wellPlot->plotSettings(), wellPlot->plotDataForLegend(), wellPlot->legend(), this);
            plotOptionsAndLegendLayout->addWidget(m_legend);
            plotOptionsAndLegendLayout->setStretch(1,1);
            break;
         }
      }
   }
   else
   {
      plotOptionsAndLegendLayout->addSpacerItem(new QSpacerItem(0,0));
      plotOptionsAndLegendLayout->setStretch(1,1);
   }
   plotOptionsAndLegendLayout->setStretch(0,0);

   return plotOptionsAndLegendLayout;
}

void MultiWellPlot::setExpanded(const bool isExpanded, const int plotID)
{
   m_expandedPlots[plotID] = isExpanded;
}

} // namespace sac

} // namespace casaWizard

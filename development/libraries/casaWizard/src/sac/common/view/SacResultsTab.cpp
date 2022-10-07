//
// Copyright (C) 2012-2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "SacResultsTab.h"

#include "assets/multiWellPlot.h"
#include "plot/wellBirdsView.h"
#include "plot/wellCorrelationPlot.h"
#include "assets/PlotOptions.h"
#include "assets/wellCorrelationPlotLayout.h"

#include "model/calibrationTarget.h"
#include "model/trajectoryType.h"
#include "model/well.h"
#include "model/wellTrajectory.h"

#include "view/colormap.h"
#include "view/sharedComponents/customtitle.h"
#include "view/sharedComponents/customradiobutton.h"

#include <QHeaderView>
#include <QLabel>
#include <QLayout>
#include <QListWidget>
#include <QPushButton>
#include <QSignalBlocker>
#include <QStackedLayout>
#include <QTableWidget>

#include <assert.h>
#include <math.h>

namespace casaWizard
{

namespace sac
{

SacResultsTab::SacResultsTab(QWidget* parent) :
   QWidget(parent),
   m_layoutWellList{new QVBoxLayout()},
   m_layoutStackedPlots{new QStackedLayout{}},
   m_selectAll{new QPushButton("Select all", this)},
   m_selectClear{new QPushButton("Clear selection", this)},
   m_wellsList{new QListWidget(this)},
   m_multiWellPlot{new MultiWellPlot(this)},
   m_plotOptions{new PlotOptions(this)},
   m_colorMap{new ColorMap()},
   m_wellBirdsView{new WellBirdsView(*m_colorMap, this)},
   m_wellCorrelationPlotLayout{new WellCorrelationPlotLayout(this)},
   m_optimizedResultsTable{new QTableWidget(this)}
{
   m_colorMap->setColorMapType("Gray scale");

   QHBoxLayout* selectionLayout = new QHBoxLayout();
   selectionLayout->addWidget(m_selectAll);
   selectionLayout->addWidget(m_selectClear);
   m_selectAll->setVisible(false);
   m_selectClear->setVisible(false);

   // List with wells
   m_layoutWellList->addWidget(new CustomTitle("Wells"), 0);
   m_layoutWellList->addWidget(m_wellsList, 1);
   m_layoutWellList->addLayout(selectionLayout);
   m_layoutWellList->addWidget(m_wellBirdsView, 0);
   m_wellBirdsView->setMaximumSize(400, 250);
   m_layoutWellList->addWidget(m_plotOptions, 0);

   m_layoutStackedPlots->addWidget(m_multiWellPlot);
   m_layoutStackedPlots->addWidget(m_wellCorrelationPlotLayout);
   m_layoutStackedPlots->addWidget(m_optimizedResultsTable);

   QHBoxLayout* total = new QHBoxLayout();
   total->addLayout(m_layoutWellList,1);
   total->addLayout(m_layoutStackedPlots, 4);

   setLayout(total);

   connect(m_selectClear, SIGNAL(clicked()), m_wellsList, SLOT(clearSelection()));
   connect(m_selectAll,   SIGNAL(clicked()), m_wellsList, SLOT(selectAll()));
}

QTableWidgetItem* SacResultsTab::createHeaderItem(const QString& name, int align)
{
   QTableWidgetItem* item = new QTableWidgetItem(name);
   QFont font = item->font();
   font.setBold(true);
   item->setFont(font);
   item->setTextAlignment(align);
   return item;
}

void SacResultsTab::clearWellCorrelationPlot()
{
   m_wellCorrelationPlotLayout->wellCorrelationPlot()->clear();
}

void SacResultsTab::updateWellList(const QVector<const Well*> wells)
{
   QSignalBlocker blocker(m_wellsList);
   m_wellsList->clear();
   for (const Well* well : wells)
   {
      m_wellsList->addItem(well->name());
   }
   m_wellsList->setMinimumWidth(m_wellsList->sizeHintForColumn(0));
}

void SacResultsTab::updateActivePropertyUserNames(const QStringList& activePropertyUserNames)
{
   m_multiWellPlot->setActivePropertyUserNames(activePropertyUserNames);
}

void SacResultsTab::updateWellPlot(const QVector<QVector<const CalibrationTarget*> > targets, const QStringList units,
                                   const QVector<QVector<WellTrajectory> > allTrajectories, const QVector<bool>& activePlots,
                                   const QMap<QString, double>& surfaceLines, const bool fitRangeToData)
{
   assert(targets.size() ==  units.size());
   m_multiWellPlot->updatePlots(targets,
                               units,
                               allTrajectories,
                               activePlots,
                               surfaceLines,
                               fitRangeToData);
}

void SacResultsTab::updateCorrelationPlot(const QVector<QVector<double>>& measuredValueTrajectories,
                                          const QVector<QVector<double>>& simulatedValueTrajectories,
                                          const QString activeProperty,
                                          const QVector<bool> activePlots,
                                          const double minValue,
                                          const double maxValue,
                                          const QVector<int>& wellIndices)
{
   m_wellCorrelationPlotLayout->updateCorrelationPlot(measuredValueTrajectories,
                                                     simulatedValueTrajectories,
                                                     activeProperty,
                                                     activePlots,
                                                     minValue,
                                                     maxValue,
                                                     wellIndices);
}

void SacResultsTab::updateBirdsView(const QVector<const Well*> wells)
{
   QSignalBlocker blocker(m_wellBirdsView);
   QVector<double> x;
   QVector<double> y;
   for (const Well* well : wells)
   {
      x.append(well->x());
      y.append(well->y());
   }
   m_wellBirdsView->setWellLocations(x, y);
}

void SacResultsTab::setRangeBirdsView(const double xMin, const double xMax, const double yMin, const double yMax)
{
   m_wellBirdsView->updateRange(xMin, xMax, yMin, yMax);
}

void SacResultsTab::updateSelectedWells()
{
   QVector<int> selectedIndices;
   for(const QModelIndex& index : wellsList()->selectionModel()->selectedIndexes())
   {
      selectedIndices.push_back(index.row());
   }

   m_wellBirdsView->setSelectedWells(selectedIndices);
}

void SacResultsTab::setPlotType(const int currentIndex)
{
   m_selectClear->setVisible(false);
   m_selectAll->setVisible(false);
   m_wellsList->setSelectionMode(QAbstractItemView::SingleSelection);
   switch(currentIndex)
   {
      case 0:
         {
            m_layoutStackedPlots->setCurrentWidget(m_multiWellPlot);
            break;
         }
      case 1:
         {
            m_selectClear->setVisible(true);
            m_selectAll->setVisible(true);
            m_wellsList->setSelectionMode(QAbstractItemView::ExtendedSelection);
            m_layoutStackedPlots->setCurrentWidget(m_wellCorrelationPlotLayout);
            break;
         }
      case 2:
         {
            m_layoutStackedPlots->setCurrentWidget(m_optimizedResultsTable);
            break;
         }
   }
}

QListWidget* SacResultsTab::wellsList() const
{
   return m_wellsList;
}

PlotOptions* SacResultsTab::plotOptions() const
{
   return m_plotOptions;
}

WellBirdsView* SacResultsTab::wellBirdsView() const
{
   return m_wellBirdsView;
}

WellCorrelationPlot* SacResultsTab::wellCorrelationPlot() const
{
   return m_wellCorrelationPlotLayout->wellCorrelationPlot();
}

WellCorrelationPlotLayout* SacResultsTab::wellCorrelationPlotLayout() const
{
   return m_wellCorrelationPlotLayout;
}

MultiWellPlot* SacResultsTab::multiWellPlot() const
{
   return m_multiWellPlot;
}

QTableWidget* SacResultsTab::optimizedResultsTable() const
{
   return m_optimizedResultsTable;
}
QVBoxLayout* SacResultsTab::layoutWellList() const
{
   return m_layoutWellList;
}


} // namespace sac

} // namespace casaWizard

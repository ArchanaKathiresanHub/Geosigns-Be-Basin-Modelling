#include "resultsTab.h"

#include "multiWellPlot.h"
#include "plot/wellBirdsView.h"
#include "plot/wellCorrelationPlot.h"
#include "plotOptions.h"

#include "model/calibrationTarget.h"
#include "model/input/projectReader.h"
#include "model/lithofraction.h"
#include "model/optimizedLithofraction.h"
#include "model/trajectoryType.h"
#include "model/well.h"
#include "model/wellTrajectory.h"

#include "../common/view/components/customtitle.h"

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

ResultsTab::ResultsTab(QWidget* parent) :
  QWidget(parent),
  selectAll_{new QPushButton("Select all", this)},
  selectClear_{new QPushButton("Clear selection", this)},
  wellsList_{new QListWidget(this)},
  optimizedLithoTable_{new QTableWidget(this)},
  multiWellPlot_{new MultiWellPlot(this)},
  wellCorrelationPlot_{new WellCorrelationPlot(this)},
  plotOptions_{new PlotOptions(this)},
  layoutStackedPlots_{new QStackedLayout{}},  
  wellBirdsView_{new WellBirdsView(this)}
{
  QHBoxLayout* selectionLayout = new QHBoxLayout();
  selectionLayout->addWidget(selectAll_);
  selectionLayout->addWidget(selectClear_);
  selectAll_->setVisible(false);
  selectClear_->setVisible(false);

  // List with wells
  QVBoxLayout* wellList = new QVBoxLayout();
  wellList->addWidget(new CustomTitle("Wells"), 0);
  wellList->addWidget(wellsList_, 1);
  wellList->addLayout(selectionLayout);
  wellList->addWidget(wellBirdsView_, 0);
  wellBirdsView_->setMaximumSize(400, 250);
  wellList->addWidget(plotOptions_, 0);

  optimizedLithoTable_->setRowCount(0);
  optimizedLithoTable_->setColumnCount(5);

  optimizedLithoTable_->setHorizontalHeaderItem(0, createHeaderItem("Layer name", Qt::AlignLeft));
  optimizedLithoTable_->setHorizontalHeaderItem(1, createHeaderItem("Lithofraction", Qt::AlignLeft));
  optimizedLithoTable_->setHorizontalHeaderItem(2, createHeaderItem("Original value", Qt::AlignRight));
  optimizedLithoTable_->setHorizontalHeaderItem(3, createHeaderItem("Optimized value", Qt::AlignRight));
  optimizedLithoTable_->setHorizontalHeaderItem(4, createHeaderItem("Difference", Qt::AlignRight));

  optimizedLithoTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
  optimizedLithoTable_->verticalHeader()->hide();
  optimizedLithoTable_->setSelectionMode(QAbstractItemView::NoSelection);
  for (int i = 0; i<5; ++i)
  {
    optimizedLithoTable_->horizontalHeader()->setSectionResizeMode(i, QHeaderView::Stretch);
  }

  layoutStackedPlots_->addWidget(multiWellPlot_);
  layoutStackedPlots_->addWidget(wellCorrelationPlot_);
  layoutStackedPlots_->addWidget(optimizedLithoTable_);

  QHBoxLayout* total = new QHBoxLayout();
  total->addLayout(wellList,1);
  total->addLayout(layoutStackedPlots_, 4);

  setLayout(total);  

  connect(selectClear_, SIGNAL(clicked()), wellsList_, SLOT(clearSelection()));
  connect(selectAll_,   SIGNAL(clicked()), wellsList_, SLOT(selectAll()));
}

QTableWidgetItem* ResultsTab::createHeaderItem(const QString& name, int align)
{
  QTableWidgetItem* item = new QTableWidgetItem(name);
  QFont font = item->font();
  font.setBold(true);
  item->setFont(font);
  item->setTextAlignment(align);
  return item;
}

QListWidget* ResultsTab::wellsList() const
{
  return wellsList_;
}

PlotOptions* ResultsTab::plotOptions() const
{
  return plotOptions_;
}

WellBirdsView* ResultsTab::wellBirdsView() const
{
  return wellBirdsView_;
}

WellCorrelationPlot* ResultsTab::wellCorrelationPlot() const
{
  return wellCorrelationPlot_;
}

void ResultsTab::updateWellList(const QVector<const Well*> wells)
{
  QSignalBlocker blocker(wellsList_);
  wellsList_->clear();
  for (const Well* well : wells)
  {
    wellsList_->addItem(well->name());
  }
  wellsList_->setMinimumWidth(wellsList_->sizeHintForColumn(0));
}

void ResultsTab::updateWellPlot(const QVector<QVector<CalibrationTarget> > targets, const QStringList properties, const QVector<QVector<WellTrajectory> > allTrajectories, const QVector<bool>& activePlots)
{
  assert(targets.size() ==  properties.size());
  multiWellPlot_->updatePlots(targets,
                              properties,
                              allTrajectories,
                              activePlots);
}

void ResultsTab::updateCorrelationPlot(const QVector<QVector<CalibrationTarget>> targets,
                                       const QStringList properties,
                                       const QVector<QVector<WellTrajectory>> allTrajectories,
                                       const QVector<bool> activePlots,
                                       const QString activeProperty)
{
  if(properties.size()==0)
  {
    wellCorrelationPlot_->clear();
    return;
  }
  assert(targets.size() ==  allTrajectories[0].size());
  int activePropertyIndex = properties.indexOf(activeProperty);
  if (activePropertyIndex == -1)
  {
    activePropertyIndex = 0;
  }
  plotOptions_->setProperties(properties, activePropertyIndex);

  wellCorrelationPlot_->setData(targets,
                            allTrajectories,
                            properties[activePropertyIndex],
                            activePlots);
}

void ResultsTab::updateOptimizedLithoTable(const QStringList& layerNameList,
                                           const QVector<QStringList>& lithoNamesVector,
                                           const QVector<QVector<double>>& originalValuesVector,
                                           const QVector<QVector<double>>& optimizedValuesVector)
{
  optimizedLithoTable_->clearContents();
  optimizedLithoTable_->setRowCount(0);

  auto doubleToQString = [](double d){return QString::number(d, 'f',2); };

  int row = 0;
  for (int i = 0; i<layerNameList.size(); ++i)
  {
    double diff =  optimizedValuesVector[i][0] - originalValuesVector[i][0];
    addLithofractionRow(row, {new QTableWidgetItem(layerNameList[i]),
                              new QTableWidgetItem(lithoNamesVector[i][0]),
                              new QTableWidgetItem(doubleToQString(originalValuesVector[i][0])),
                              new QTableWidgetItem(doubleToQString(optimizedValuesVector[i][0])),
                              new QTableWidgetItem(doubleToQString(diff))}, diff);


    for (int j = 1; j<lithoNamesVector[i].size(); ++j)
    {
      diff = optimizedValuesVector[i][j] - originalValuesVector[i][j];
      addLithofractionRow(row, {nullptr,
                                new QTableWidgetItem(lithoNamesVector[i][j]),
                                new QTableWidgetItem(doubleToQString(originalValuesVector[i][j])),
                                new QTableWidgetItem(doubleToQString(optimizedValuesVector[i][j])),
                                new QTableWidgetItem(doubleToQString(diff))}, diff);
    }
  }
}

void ResultsTab::addLithofractionRow(int& row, QVector<QTableWidgetItem*> items, const double diff)
{
  optimizedLithoTable_->setRowCount(row+1);
  for (int i = 0; i<items.size();++i)
  {
    optimizedLithoTable_->setItem(row, i, items[i]);
    if (i>1)
    {
      items[i]->setTextAlignment(Qt::AlignRight);
    }
    if (i==4 && std::fabs(diff) > 10)
    {
      items[i]->setBackgroundColor(QColor(Qt::red));
    }
  }
  row++;
}

void ResultsTab::updateBirdsView(const QVector<const Well*> wells)
{
  QSignalBlocker blocker(wellBirdsView_);
  QVector<double> x;
  QVector<double> y;
  for (const Well* well : wells)
  {
    x.append(well->x());
    y.append(well->y());
  }
  wellBirdsView_->setWellLocations(x, y);
}

void ResultsTab::setRangeBirdsView(const double xMin, const double xMax, const double yMin, const double yMax)
{
  wellBirdsView_->updateRange(xMin, xMax, yMin, yMax);
}

void ResultsTab::updateSelectedWells()
{
  QVector<int> selectedIndices;
  for(const QModelIndex& index : wellsList()->selectionModel()->selectedIndexes())
  {
    selectedIndices.push_back(index.row());
  }

  wellBirdsView_->setSelectedWells(selectedIndices);
}

void ResultsTab::setPlotType(const int currentIndex)
{  
  selectClear_->setVisible(false);
  selectAll_->setVisible(false);
  optimizedLithoTable_->setEnabled(false);
  multiWellPlot_->setEnabled(false);
  wellCorrelationPlot_->setEnabled(false);
  wellsList_->setSelectionMode(QAbstractItemView::SingleSelection);
  switch(currentIndex)
  {
    case 0:
    {
      multiWellPlot_->setEnabled(true);
      layoutStackedPlots_->setCurrentWidget(multiWellPlot_);
      break;
    }
    case 1:
    {
      selectClear_->setVisible(true);
      selectAll_->setVisible(true);
      wellCorrelationPlot_->setEnabled(true);
      wellsList_->setSelectionMode(QAbstractItemView::ExtendedSelection);
      layoutStackedPlots_->setCurrentWidget(wellCorrelationPlot_);
      break;
    }
    case 2:
    {
      optimizedLithoTable_->setEnabled(true);
      layoutStackedPlots_->setCurrentWidget(optimizedLithoTable_);
      break;
    }
  }
}

} // namespace sac

} // namespace casaWizard

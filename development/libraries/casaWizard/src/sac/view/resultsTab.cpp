#include "resultsTab.h"

#include "multiWellPlot.h"
#include "plot/wellBirdsView.h"
#include "plot/wellScatterPlot.h"
#include "plotOptions.h"

#include "model/calibrationTarget.h"
#include "model/input/projectReader.h"
#include "model/lithofraction.h"
#include "model/optimizedLithofraction.h"
#include "model/trajectoryType.h"
#include "model/well.h"
#include "model/wellTrajectory.h"

#include <QHeaderView>
#include <QLabel>
#include <QLayout>
#include <QListWidget>
#include <QPushButton>
#include <QStackedLayout>
#include <QTableWidget>

#include <assert.h>
namespace casaWizard
{

namespace sac
{

ResultsTab::ResultsTab(QWidget* parent) :
  QWidget(parent),
  wellsList_{new QListWidget(this)},
  optimizedLithoTable_{new QTableWidget(this)},
  multiWellPlot_{new MultiWellPlot(this)},
  wellScatterPlot_{new WellScatterPlot(this)},
  buttonSaveOptimized_{new QPushButton("Save optimized", this)},
  buttonRunOptimized_{new QPushButton("Run optimized", this)},
  buttonBaseCase_{new QPushButton("Run and import base case", this)},
  plotOptions_{new PlotOptions(this)},
  layoutStackedPlots_{new QStackedLayout{}},
  tableLable_{new QLabel("Optimized lithofractions",this)},
  wellBirdsView_{new WellBirdsView(this)}
{
  // List with wells
  QVBoxLayout* wellList = new QVBoxLayout();

  wellList->addWidget(new QLabel("Wells"), 0);
  wellList->addWidget(wellsList_, 1);
  wellList->addWidget(wellBirdsView_, 0);
  wellList->addWidget(plotOptions_, 0);

  QVBoxLayout* optimizedLitho = new QVBoxLayout();
  optimizedLithoTable_->setRowCount(0);
  optimizedLithoTable_->setColumnCount(3);

  optimizedLithoTable_->setHorizontalHeaderItem(0, new QTableWidgetItem("Layer name"));
  optimizedLithoTable_->setHorizontalHeaderItem(1, new QTableWidgetItem("Lithofraction"));
  optimizedLithoTable_->setHorizontalHeaderItem(2, new QTableWidgetItem("Value"));

  optimizedLithoTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
  optimizedLithoTable_->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);

  optimizedLitho->addWidget(tableLable_, 0);

  QHBoxLayout* optimizedButtons = new QHBoxLayout();
  optimizedButtons->addWidget(buttonSaveOptimized_);
  optimizedButtons->addWidget(buttonRunOptimized_);

  optimizedLitho->addLayout(optimizedButtons);
  optimizedLitho->addWidget(buttonBaseCase_);
  optimizedLitho->addWidget(optimizedLithoTable_);

  layoutStackedPlots_->addWidget(multiWellPlot_);
  layoutStackedPlots_->addWidget(wellScatterPlot_);

  QHBoxLayout* total = new QHBoxLayout();
  total->addLayout(wellList,0);
  total->addLayout(optimizedLitho, 2);
  total->addLayout(layoutStackedPlots_, 0);

  setLayout(total);
}

QListWidget* ResultsTab::wellsList() const
{
  return wellsList_;
}

QPushButton* ResultsTab::buttonSaveOptimized() const
{
  return buttonSaveOptimized_;
}

QPushButton* ResultsTab::buttonRunOptimized() const
{
  return buttonRunOptimized_;
}

QPushButton* ResultsTab::buttonBaseCase() const
{
  return buttonBaseCase_;
}

PlotOptions* ResultsTab::plotOptions() const
{
  return plotOptions_;
}

WellBirdsView* ResultsTab::wellBirdsView() const
{
  return wellBirdsView_;
}

WellScatterPlot* ResultsTab::wellScatterPlot() const
{
  return wellScatterPlot_;
}

void ResultsTab::updateWellList(const QVector<Well>& wells)
{
  const int row = wellsList()->currentRow();

  wellsList_->clear();
  for (const Well& well : wells)
  {
    wellsList_->addItem(well.name());
  }
  wellsList_->setMinimumWidth(wellsList_->sizeHintForColumn(0));

  if (row >= 0 && row < wellsList_->count())
  {
    wellsList_->setCurrentRow(row);
  }
}

void ResultsTab::updateWellPlot(const QVector<QVector<CalibrationTarget> > targets, const QStringList properties, const QVector<QVector<WellTrajectory> > allTrajectories, const QVector<bool>& activePlots)
{
  assert(targets.size() ==  properties.size());
  multiWellPlot_->updatePlots(targets,
                              properties,
                              allTrajectories,
                              activePlots);
}

void ResultsTab::updateScatterPlot(const QVector<QVector<CalibrationTarget>> targets,
                                   const QStringList properties,
                                   const QVector<QVector<WellTrajectory>> allTrajectories,
                                   const QVector<bool> activePlots,
                                   const QString activeProperty)
{
  if(properties.size()==0)
  {
    wellScatterPlot_->clear();
    return;
  }
  assert(targets.size() ==  allTrajectories[0].size());
  int activePropertyIndex = properties.indexOf(activeProperty);
  if (activePropertyIndex == -1)
  {
    activePropertyIndex = 0;
  }
  plotOptions_->setProperties(properties, activePropertyIndex);

  wellScatterPlot_->setData(targets,
                            allTrajectories,
                            properties[activePropertyIndex],
                            activePlots);
}

void ResultsTab::updateOptimizedLithoTable(const QVector<OptimizedLithofraction>& optimizedLithofractions, const QVector<Lithofraction>& lithofractions, const ProjectReader& projectReader)
{
  optimizedLithoTable_->clearContents();
  optimizedLithoTable_->setRowCount(0);
  int row = 0;

  auto doubleToQString = [](double d){return QString::number(d, 'f',2); };

  const QStringList layerNames = projectReader.layerNames();

  for (const OptimizedLithofraction& optimized : optimizedLithofractions)
  {
    const Lithofraction& litho = lithofractions[optimized.lithofractionId()];

    const int layerIndex = layerNames.indexOf(litho.layerName());
    const QStringList lithoNames = projectReader.lithologyTypesForLayer(layerIndex);

    optimizedLithoTable_->setRowCount(row+1);

    optimizedLithoTable_->setItem(row, 0, new QTableWidgetItem(litho.layerName()));
    optimizedLithoTable_->setItem(row, 1, new QTableWidgetItem(lithoNames[litho.firstComponent()]));
    optimizedLithoTable_->setItem(row, 2, new QTableWidgetItem(doubleToQString(optimized.optimizedPercentageFirstComponent())));
    row++;

    if (litho.secondComponent() < 3) // not None selected
    {
      optimizedLithoTable_->setRowCount(row+1);
      optimizedLithoTable_->setItem(row, 1, new QTableWidgetItem(lithoNames[litho.secondComponent()]));
      optimizedLithoTable_->setItem(row, 2, new QTableWidgetItem(doubleToQString(optimized.optimizedPercentageSecondComponent())));
      row++;

      optimizedLithoTable_->setRowCount(row+1);
      optimizedLithoTable_->setItem(row, 1, new QTableWidgetItem(lithoNames[litho.thirdComponent()]));
      optimizedLithoTable_->setItem(row, 2, new QTableWidgetItem(doubleToQString(optimized.optimizedPercentageThirdComponent())));
      row++;

    }
  }
}

void ResultsTab::updateBirdsView(const QVector<const Well*> wells)
{
  QVector<double> x;
  QVector<double> y;
  for (const Well *const well : wells)
  {
    x.append(well->x());
    y.append(well->y());
  }
  wellBirdsView_->setWellLocations(x, y);
}

void ResultsTab::updateActiveWells(const QVector<int> activeWells)
{
  wellBirdsView_->setActiveWells(activeWells);
}

void ResultsTab::setPlotType(const int currentIndex)
{
  multiWellPlot_->setEnabled(false);
  wellScatterPlot_->setEnabled(false);
  setVisibleLithofractionColumn(true);
  wellsList_->setSelectionMode(QAbstractItemView::SingleSelection);
  switch(currentIndex)
  {
    case 0:
    {
      multiWellPlot_->setEnabled(true);
      layoutStackedPlots_->setCurrentWidget(multiWellPlot_);
    }
      break;
    case 1:
    {
      wellScatterPlot_->setEnabled(true);
      setVisibleLithofractionColumn(false);
      wellsList_->setSelectionMode(QAbstractItemView::ExtendedSelection);
      layoutStackedPlots_->setCurrentWidget(wellScatterPlot_);
    }
  }
}

void ResultsTab::setVisibleLithofractionColumn(const bool visible)
{
  buttonBaseCase_->setVisible(visible);
  buttonRunOptimized_->setVisible(visible);
  buttonSaveOptimized_->setVisible(visible);
  optimizedLithoTable_->setVisible(visible);
  tableLable_->setVisible(visible);
}

} // namespace sac

} // namespace casaWizard

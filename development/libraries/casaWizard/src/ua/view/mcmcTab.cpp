#include "mcmcTab.h"

#include "model/predictionTarget.h"
#include "view/plot/histogram.h"
#include "view/plot/quartilePlot.h"

#include <QCheckBox>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QSlider>
#include <QStackedLayout>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>

namespace casaWizard
{

namespace ua
{

MCMCTab::MCMCTab(QWidget* parent) :
  QWidget(parent),
  pushButtonUArunCASA_{new QPushButton("Run MCMC", this)},
  pushButtonExportMcmcResults_{new QPushButton("Export MCMC results", this)},
  pushButtonExportOptimalCase_{new QPushButton("Export optimal settings", this)},
  pushButtonRunOptimalCase_{new QPushButton("Run optimal settings", this)},
  pushButtonAddOptimalDesignPoint_{new QPushButton("Add optimal sample as design point", this)},
  lineEditL2normRS_{new QLineEdit("", this)},
  lineEditL2norm_{new QLineEdit("", this)},
  tablePredictionTargets_{new QTableWidget(this)},
  histogramPredictionTarget_{new Histogram(this)},
  sliderHistograms_{new QSlider{Qt::Horizontal, this}},
  lineEditTimeSeries_{new QLineEdit{"0", this}},
  checkBoxHistoryPlotsMode_{new QCheckBox{"Plot time series", this}},
  quartilePredictionTargetTimeSeries_{new QuartilePlot{this}},
  layoutStackedPlots_{new QStackedLayout{}}
{
  pushButtonUArunCASA_->setFixedHeight(30);
  pushButtonExportMcmcResults_->setFixedHeight(30);
  pushButtonExportOptimalCase_->setFixedHeight(30);
  pushButtonRunOptimalCase_->setFixedHeight(30);
  lineEditTimeSeries_->setReadOnly(true);
  checkBoxHistoryPlotsMode_->setChecked(false);

  tablePredictionTargets_->setRowCount(0);
  tablePredictionTargets_->setColumnCount(1);
  tablePredictionTargets_->setHorizontalHeaderItem(0, new QTableWidgetItem("Prediction target"));
  tablePredictionTargets_->setSortingEnabled(false);
  tablePredictionTargets_->horizontalHeader()->setCascadingSectionResizes(true);
  tablePredictionTargets_->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
  tablePredictionTargets_->horizontalHeader()->setStretchLastSection(true);

  QGridLayout* gridLayout = new QGridLayout();
  gridLayout->addWidget(pushButtonUArunCASA_, 0, 0);
  gridLayout->addWidget(new QLabel("L2 norm", this), 0, 1);
  gridLayout->addWidget(lineEditL2normRS_, 0, 2);
  gridLayout->addWidget(pushButtonExportMcmcResults_, 1, 0);
  gridLayout->addWidget(pushButtonExportOptimalCase_, 2, 0);

  gridLayout->addWidget(pushButtonRunOptimalCase_, 3, 0);
  gridLayout->addWidget(new QLabel("L2 norm", this), 3, 1);
  gridLayout->addWidget(lineEditL2norm_, 3, 2);
  gridLayout->addWidget(checkBoxHistoryPlotsMode_, 4, 0);

  gridLayout->addWidget(pushButtonAddOptimalDesignPoint_);

  QVBoxLayout* layoutTableAndRun = new QVBoxLayout();
  layoutTableAndRun->addLayout(gridLayout);
  layoutTableAndRun->addWidget(tablePredictionTargets_);
  layoutTableAndRun->setStretch(1,1);

  QGridLayout* sliderGridLayout = new QGridLayout();
  sliderGridLayout->addWidget(new QLabel("Time series [Ma]", this), 0,0);
  sliderGridLayout->addWidget(lineEditTimeSeries_, 0, 1);
  sliderGridLayout->addWidget(sliderHistograms_, 0, 2);
  sliderGridLayout->setColumnStretch(2,5);

  layoutStackedPlots_->addWidget(histogramPredictionTarget_);
  layoutStackedPlots_->addWidget(quartilePredictionTargetTimeSeries_);

  QVBoxLayout* layoutPlotAndSlider = new QVBoxLayout();
  layoutPlotAndSlider->addLayout(layoutStackedPlots_);
  layoutPlotAndSlider->addLayout(sliderGridLayout);
  layoutPlotAndSlider->setStretch(0,1);

  QHBoxLayout* tabLayout = new QHBoxLayout();
  tabLayout->addLayout(layoutTableAndRun);
  tabLayout->addLayout(layoutPlotAndSlider);
  tabLayout->setStretch(1, 1);

  setLayout(tabLayout);
}

const QPushButton* MCMCTab::pushButtonUArunCASA() const
{
  return pushButtonUArunCASA_;
}

const QPushButton* MCMCTab::pushButtonExportMcmcResults() const
{
  return pushButtonExportMcmcResults_;
}

const QPushButton* MCMCTab::pushButtonExportOptimalCase() const
{
  return pushButtonExportOptimalCase_;
}

const QPushButton* MCMCTab::pushButtonRunOptimalCase() const
{
  return pushButtonRunOptimalCase_;
}

const QPushButton*MCMCTab::pushButtonAddOptimalDesignPoint() const
{
  return pushButtonAddOptimalDesignPoint_;
}

const QTableWidget* MCMCTab::tablePredictionTargets() const
{
  return tablePredictionTargets_;
}

void MCMCTab::setL2normRS(const double l2norm) const
{
  lineEditL2normRS_->setText(QString::number(l2norm));
}

void MCMCTab::setL2norm(const double l2norm) const
{
  lineEditL2norm_->setText(QString::number(l2norm));
}

void MCMCTab::fillPredictionTargetTable(const QVector<const PredictionTarget*> predictionTargets)
{
  tablePredictionTargets_->clearContents();
  tablePredictionTargets_->setRowCount(predictionTargets.size());

  int row = 0;
  for (const PredictionTarget* const predictionTarget : predictionTargets)
  {
    const QStringList list = QStringList() << predictionTarget->nameWithoutAge();
    int col = 0;
    for (const QString& data : list)
    {
      QTableWidgetItem* item = new QTableWidgetItem(data);
      item->setFlags(item->flags() & ~Qt::ItemIsEditable);
      tablePredictionTargets_->setItem(row, col, item);
      ++col;
    }
    ++row;
  }
  if (predictionTargets.size() > 0)
  {
    tablePredictionTargets_->setCurrentCell(0,0);
  }
}

void MCMCTab::updateSliderHistograms(const int timeSeriesSize)
{
  sliderHistograms_->setEnabled(true);
  lineEditTimeSeries_->setEnabled(true);
  sliderHistograms_->setSliderPosition(0);
  sliderHistograms_->setRange(0, timeSeriesSize-1);
}

QCheckBox* MCMCTab::checkBoxHistoryPlotsMode() const
{
  return checkBoxHistoryPlotsMode_;
}

QSlider* MCMCTab::sliderHistograms() const
{
  return sliderHistograms_;
}

void MCMCTab::updateHistogram(const PredictionTarget* const predictionTarget, const QVector<double>& values)
{
  layoutStackedPlots_->setCurrentWidget(histogramPredictionTarget_);


  histogramPredictionTarget_->setData(values);
  histogramPredictionTarget_->update();

  if (predictionTarget)
  {
    lineEditTimeSeries_->setText(QString::number(predictionTarget->age(), 'f', 0));
    histogramPredictionTarget_->setXLabel(predictionTarget->property() + " " + predictionTarget->unitSI());
  }
  histogramPredictionTarget_->setYLabel("Number of counts");
  histogramPredictionTarget_->setFontStyle(FontStyle::small);
}

void MCMCTab::updateTimeSeriesPlot(const PredictionTarget& predictionTarget, const QVector<double>& timeSeries, const QVector<QVector<double>>& valuesMatrix, const QVector<double>& sampleCoordinates)
{
  layoutStackedPlots_->setCurrentWidget(quartilePredictionTargetTimeSeries_);

  quartilePredictionTargetTimeSeries_->setData(timeSeries, valuesMatrix, sampleCoordinates);
  quartilePredictionTargetTimeSeries_->update();

  quartilePredictionTargetTimeSeries_->setXLabel("age [Ma]");
  quartilePredictionTargetTimeSeries_->setYLabel(predictionTarget.property() + " " + predictionTarget.unitSI());
}


void MCMCTab::setEnableTimeSeries(const bool checkState)
{
  if(checkState)
  {
    sliderHistograms_->setDisabled(true);
    lineEditTimeSeries_->setDisabled(true);
    layoutStackedPlots_->setCurrentWidget(quartilePredictionTargetTimeSeries_);
  }
  else
  {
    sliderHistograms_->setEnabled(true);
    lineEditTimeSeries_->setEnabled(true);
    layoutStackedPlots_->setCurrentWidget(histogramPredictionTarget_);
  }
}

} // namespace ua

} // namespace casaWizard

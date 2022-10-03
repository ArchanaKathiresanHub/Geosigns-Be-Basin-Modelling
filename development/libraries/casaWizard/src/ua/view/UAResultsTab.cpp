//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "UAResultsTab.h"

#include "UAResultsTargetTable.h"
#include "NamesValuesList.h"

#include "model/predictionTarget.h"
#include "model/predictionTargetManager.h"
#include "model/targetParameterMapCreator.h"
#include "model/UAResultsTargetsData.h"

#include "view/sharedComponents/helpLabel.h"
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

UAResultsTab::UAResultsTab(QWidget* parent) :
   QWidget(parent),
   pushButtonUArunCASA_{new QPushButton("Run MCMC", this)},
   pushButtonExportMcmcResults_{new QPushButton("Export uncertainty results", this)},
   pushButtonExportOptimalCase_{new QPushButton("Export optimal settings", this)},
   pushButtonRunOptimalCase_{new QPushButton("Run optimal settings", this)},
   pushButtonRecalcResponseSurfaces_{new QPushButton("Recalculate response surfaces including optimal settings run", this)},
   rmseList_{new NamesValuesList("Optimal case quality:")},
   optimalValuesList_{new NamesValuesList("Optimal values for calibration data:")},
   tablePredictionTargets_{new UAResultsTargetTable(this)},
   histogramsPredictionTarget_{new QWidget(this)},
   sliderHistograms_{new QSlider{Qt::Horizontal, this}},
   lineEditTimeSeries_{new QLineEdit{"0", this}},
   checkBoxHistoryPlotsMode_{new QCheckBox{"Plot time series", this}},
   quartilePredictionTargetTimeSeries_{new QuartilePlot{this}},
   layoutStackedPlots_{new QStackedLayout{}},
   timeSeriesPlots_{nullptr}
{
   rmseList_->addToolTipToHeader("Normalized root mean squared error of model results at calibration targets with all calibration target values.\n"
                                 "Represents the average ratio between the model error at data points and the stated data standard deviations.");

   pushButtonUArunCASA_->setFixedHeight(30);
   pushButtonExportMcmcResults_->setFixedHeight(30);
   pushButtonExportOptimalCase_->setFixedHeight(30);
   pushButtonRunOptimalCase_->setFixedHeight(30);
   lineEditTimeSeries_->setReadOnly(true);
   checkBoxHistoryPlotsMode_->setChecked(false);

   QGridLayout* gridLayout = new QGridLayout();
   gridLayout->addWidget(pushButtonUArunCASA_, 0, 0);
   gridLayout->addWidget(pushButtonExportMcmcResults_, 1, 0);
   gridLayout->addWidget(pushButtonExportOptimalCase_, 0, 1);

   HelpLabel* helpLabelZ = new HelpLabel(this, "Optimal setting refers to the MCMC point that fits all data points best given the applied weighting.");
   gridLayout->addWidget(helpLabelZ, 0, 2);
   gridLayout->addWidget(pushButtonRunOptimalCase_, 1, 1);

   QHBoxLayout* hButtonLayout = new QHBoxLayout();
   hButtonLayout->addWidget(checkBoxHistoryPlotsMode_);
   hButtonLayout->addWidget(pushButtonRecalcResponseSurfaces_);

   gridLayout->addLayout(hButtonLayout,2,0,1,2);

   QVBoxLayout* layoutTableAndRun = new QVBoxLayout();
   layoutTableAndRun->addLayout(gridLayout);
   layoutTableAndRun->addLayout(rmseList_);
   layoutTableAndRun->addLayout(optimalValuesList_);
   layoutTableAndRun->addWidget(tablePredictionTargets_);
   layoutTableAndRun->setStretch(3,1);

   QGridLayout* sliderGridLayout = new QGridLayout();
   timeSeriesLabel_ = new QLabel("Time series [Ma]", this);

   sliderGridLayout->addWidget(timeSeriesLabel_, 0,0);
   sliderGridLayout->addWidget(lineEditTimeSeries_, 0, 1);
   sliderGridLayout->addWidget(sliderHistograms_, 0, 2);
   sliderGridLayout->setColumnStretch(2,5);

   layoutStackedPlots_->addWidget(histogramsPredictionTarget_);
   layoutStackedPlots_->addWidget(quartilePredictionTargetTimeSeries_);

   QVBoxLayout* layoutPlotAndSlider = new QVBoxLayout();
   layoutPlotAndSlider->addLayout(layoutStackedPlots_);
   layoutPlotAndSlider->addLayout(sliderGridLayout);
   layoutPlotAndSlider->setStretch(0,1);

   QHBoxLayout* tabLayout = new QHBoxLayout();
   tabLayout->addLayout(layoutTableAndRun);
   tabLayout->addLayout(layoutPlotAndSlider);
   tabLayout->setStretch(0, 1);
   tabLayout->setStretch(1, 2);

   setLayout(tabLayout);
}

const QPushButton* UAResultsTab::pushButtonUArunCASA() const
{
   return pushButtonUArunCASA_;
}

const QPushButton* UAResultsTab::pushButtonExportMcmcResults() const
{
   return pushButtonExportMcmcResults_;
}

const QPushButton* UAResultsTab::pushButtonExportOptimalCase() const
{
   return pushButtonExportOptimalCase_;
}

const QPushButton* UAResultsTab::pushButtonRunOptimalCase() const
{
   return pushButtonRunOptimalCase_;
}

const QPushButton* UAResultsTab::pushButtonRecalcResponseSurfaces() const
{
   return pushButtonRecalcResponseSurfaces_;
}

const QTableWidget* UAResultsTab::tablePredictionTargets() const
{
   return tablePredictionTargets_;
}

UAResultsTargetTable* UAResultsTab::targetTable() const
{
   return tablePredictionTargets_;
}

void UAResultsTab::disableAll(bool disabled)
{
   pushButtonUArunCASA_->setDisabled(disabled);
   pushButtonExportMcmcResults_->setDisabled(disabled);
   pushButtonExportOptimalCase_->setDisabled(disabled);
   pushButtonRunOptimalCase_->setDisabled(disabled);
   pushButtonRecalcResponseSurfaces_->setDisabled(disabled);
   tablePredictionTargets_->setDisabled(disabled);
   histogramsPredictionTarget_->setDisabled(disabled);
   sliderHistograms_->setDisabled(disabled);
   lineEditTimeSeries_->setDisabled(disabled);
   checkBoxHistoryPlotsMode_->setDisabled(disabled);
   quartilePredictionTargetTimeSeries_->setDisabled(disabled);
   timeSeriesLabel_->setDisabled(disabled);
}

void UAResultsTab::displayOptimalCaseQuality(const QVector<double>& optimalValues, const QStringList& parameterNames)
{
   rmseList_->displayList(optimalValues,parameterNames);
}

void UAResultsTab::clearOptimalCaseQuality()
{
   rmseList_->clearList();
}

void UAResultsTab::displayOptimalParameterValues(const QVector<double>& optimalValues, const QStringList& parameterNames)
{
   optimalValuesList_->clearList();
   optimalValuesList_->displayList(optimalValues,parameterNames);
}

void UAResultsTab::clearOptimalResultsLayout()
{
   optimalValuesList_->clearList();
}

void UAResultsTab::setUArunCASAButtonEnabled(bool enabled)
{
   pushButtonUArunCASA_->setEnabled(enabled);
}

void UAResultsTab::setRecalcResponseSurfacesButtonEnabled(bool enabled)
{
   pushButtonRecalcResponseSurfaces_->setEnabled(enabled);
}

void UAResultsTab::setOptimalCaseButtonsDisabled()
{
   pushButtonExportOptimalCase_->setDisabled(true);
   pushButtonRunOptimalCase_->setDisabled(true);
   pushButtonRecalcResponseSurfaces_->setDisabled(true);
}

void UAResultsTab::setOptimalCaseButtonsEnabled()
{
   pushButtonExportOptimalCase_->setDisabled(false);
   pushButtonRunOptimalCase_->setDisabled(false);
}

void UAResultsTab::updateSliderHistograms(const int timeSeriesSize)
{
   sliderHistograms_->setSliderPosition(0);
   if (timeSeriesSize == 0)
   {
      sliderHistograms_->setEnabled(false);
      lineEditTimeSeries_->setEnabled(false);
      return;
   }
   sliderHistograms_->setEnabled(true);
   lineEditTimeSeries_->setEnabled(true);
   sliderHistograms_->setRange(0, timeSeriesSize-1);
}

QCheckBox* UAResultsTab::checkBoxHistoryPlotsMode() const
{
   return checkBoxHistoryPlotsMode_;
}

void UAResultsTab::clearTimeSeriesPlots()
{
   delete timeSeriesPlots_;
   timeSeriesPlots_ = new QWidget(this);
   layoutStackedPlots_->addWidget(timeSeriesPlots_);
   layoutStackedPlots_->setCurrentWidget(timeSeriesPlots_);
}

QSlider* UAResultsTab::sliderHistograms() const
{
   return sliderHistograms_;
}

void UAResultsTab::updateHistogram(const PredictionTarget* const predictionTarget, const QVector<QVector<double>>& values, const QVector<QString>& predictionProperties)
{
   delete histogramsPredictionTarget_;
   histogramsPredictionTarget_ = new QWidget(this);
   QHBoxLayout* layout = new QHBoxLayout(histogramsPredictionTarget_);
   layoutStackedPlots_->addWidget(histogramsPredictionTarget_);
   layoutStackedPlots_->setCurrentWidget(histogramsPredictionTarget_);

   if (!predictionTarget)
   {
      return;
   }

   int propertyCounter = 0;
   for (const QString& predictionProperty : predictionProperties)
   {
      Histogram* histogram = new Histogram(histogramsPredictionTarget_);
      histogram->setData(values[propertyCounter]);
      histogram->update();

      lineEditTimeSeries_->setText(QString::number(predictionTarget->age(), 'f', 0));
      histogram->setXLabel(predictionProperty + " " + targetParameterMapCreator::lookupSIUnit(predictionProperty));
      histogram->setYLabel("Number of counts");
      histogram->setFontStyle(FontStyle::small);

      layout->addWidget(histogram);

      propertyCounter++;
   }
}

void UAResultsTab::updateTimeSeriesPlot(const QVector<double>& timeSeries, const QMap<QString,QVector<QVector<double>>>& valuesMatrices,
                                        const QMap<QString, QVector<double> >& sampleCoordinatesPerProperty, const QVector<QString>& predictionProperties)
{
   delete timeSeriesPlots_;
   timeSeriesPlots_ = new QWidget(this);
   QVBoxLayout* layout = new QVBoxLayout(timeSeriesPlots_);
   layoutStackedPlots_->addWidget(timeSeriesPlots_);
   layoutStackedPlots_->setCurrentWidget(timeSeriesPlots_);

   int propertyCounter = 0;
   for (const QString& predictionProperty : predictionProperties)
   {
      QuartilePlot* timePlot = new QuartilePlot(timeSeriesPlots_);
      timePlot->setData(timeSeries, valuesMatrices[predictionProperty], sampleCoordinatesPerProperty[predictionProperty]);
      timePlot->update();
      timePlot->setXLabel("age [Ma]");
      timePlot->setYLabel(predictionProperty + " " + targetParameterMapCreator::lookupSIUnit(predictionProperty));
      layout->addWidget(timePlot);
      propertyCounter++;
   }
}


void UAResultsTab::setEnableTimeSeries(const bool checkState)
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
      layoutStackedPlots_->setCurrentWidget(histogramsPredictionTarget_);
   }
}

} // namespace ua

} // namespace casaWizard

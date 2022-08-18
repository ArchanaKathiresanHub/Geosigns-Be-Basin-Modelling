//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// View for the Uncertainty Analysis tab
#pragma once

#include <QWidget>

class QCheckBox;
class QGridLayout;
class QLabel;
class QLineEdit;
class QPushButton;
class QSlider;
class QStackedLayout;
class QTableWidget;
class QVBoxLayout;

namespace casaWizard
{

class Histogram;
class QuartilePlot;

namespace ua
{

class NamesValuesList;
class UAResultsTargetTable;
class PredictionTarget;

class UAResultsTab : public QWidget
{
   Q_OBJECT

public:
   explicit UAResultsTab(QWidget *parent = nullptr);

   const QPushButton* pushButtonUArunCASA() const;
   const QPushButton* pushButtonExportMcmcResults() const;
   const QPushButton* pushButtonExportOptimalCase() const;
   const QPushButton* pushButtonRunOptimalCase() const;
   const QPushButton* pushButtonRecalcResponseSurfaces() const;
   const QTableWidget* tablePredictionTargets() const;
   UAResultsTargetTable* targetTable() const;

   void updateHistogram(const PredictionTarget* const predictionTarget, const QVector<QVector<double> >& values, const QVector<QString>& predictionProperties);
   void updateTimeSeriesPlot(const QVector<double>& timeSeries, const QMap<QString, QVector<QVector<double>>>& valuesMatrices, const QMap<QString,QVector<double>>& sampleCoordinatesPerProperty, const QVector<QString>& predictionProperties);
   void setEnableTimeSeries(const bool checkState);

   void disableAll(bool disabled = true);

   void setUArunCASAButtonEnabled(bool enabled = true);
   void setRecalcResponseSurfacesButtonEnabled(bool enabled = true);

   void setOptimalCaseButtonsDisabled();
   void setOptimalCaseButtonsEnabled();

   QSlider* sliderHistograms() const;
   void updateSliderHistograms(const int timeSeriesSize);
   QCheckBox* checkBoxHistoryPlotsMode() const;

   void displayOptimalParameterValues(const QVector<double>& optimalValues, const QStringList& parameterNames);
   void clearOptimalResultsLayout();

   void displayOptimalCaseQuality(const QVector<double>& optimalValues, const QStringList& parameterNames);
   void clearOptimalCaseQuality();

   void clearTimeSeriesPlots();

private:
   QPushButton* pushButtonUArunCASA_;
   QPushButton* pushButtonExportMcmcResults_;
   QPushButton* pushButtonExportOptimalCase_;
   QPushButton* pushButtonRunOptimalCase_;
   QPushButton* pushButtonRecalcResponseSurfaces_;
   QLabel*      timeSeriesLabel_;
   NamesValuesList* rmseList_;
   NamesValuesList* optimalValuesList_;
   UAResultsTargetTable* tablePredictionTargets_;
   QWidget* histogramsPredictionTarget_;
   QWidget* timeSeriesPlots_;
   QSlider* sliderHistograms_;
   QLineEdit* lineEditTimeSeries_;
   QCheckBox* checkBoxHistoryPlotsMode_;
   QuartilePlot* quartilePredictionTargetTimeSeries_;
   QStackedLayout* layoutStackedPlots_;
};

} // namespace ua

} // namespace casaWizard

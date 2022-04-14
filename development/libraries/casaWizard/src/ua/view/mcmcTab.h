// View for the Uncertainty Analysis tab
#pragma once

#include <QWidget>

class QCheckBox;
class QLineEdit;
class QPushButton;
class QSlider;
class QStackedLayout;
class QTableWidget;

namespace casaWizard
{

class Histogram;
class QuartilePlot;

namespace ua
{

struct PredictionTarget;

class MCMCTab : public QWidget
{
   Q_OBJECT

public:
   explicit MCMCTab(QWidget *parent = nullptr);

   const QPushButton* pushButtonUArunCASA() const;
   const QPushButton* pushButtonExportMcmcResults() const;
   const QPushButton* pushButtonExportOptimalCase() const;
   const QPushButton* pushButtonRunOptimalCase() const;
   const QPushButton* pushButtonAddOptimalDesignPoint() const;
   const QTableWidget* tablePredictionTargets() const;

   void setL2normRS(const double l2norm) const;
   void setL2norm(const double l2norm) const;
   void updateHistogram(const PredictionTarget* const predictionTarget, const QVector<double>& values);
   void fillPredictionTargetTable(const QVector<const PredictionTarget*> predictionTargets);
   void setEnableTimeSeries(const bool checkState);
   void updateTimeSeriesPlot(const PredictionTarget& predictionTarget, const QVector<double>& timeSeries, const QVector<QVector<double>>& valuesMatrix, const QVector<double>& sampleCoordinates);

   QSlider* sliderHistograms() const;
   void updateSliderHistograms(const int timeSeriesSize);
   QCheckBox* checkBoxHistoryPlotsMode() const;

private:
   QPushButton* pushButtonUArunCASA_;
   QPushButton* pushButtonExportMcmcResults_;
   QPushButton* pushButtonExportOptimalCase_;
   QPushButton* pushButtonRunOptimalCase_;
   QPushButton* pushButtonAddOptimalDesignPoint_;
   QLineEdit*   lineEditL2normRS_;
   QLineEdit*   lineEditL2norm_;
   QTableWidget* tablePredictionTargets_;
   Histogram* histogramPredictionTarget_;
   QSlider* sliderHistograms_;
   QLineEdit* lineEditTimeSeries_;
   QCheckBox* checkBoxHistoryPlotsMode_;
   QuartilePlot* quartilePredictionTargetTimeSeries_;
   QStackedLayout* layoutStackedPlots_;
};

} // namespace ua

} // namespace casaWizard

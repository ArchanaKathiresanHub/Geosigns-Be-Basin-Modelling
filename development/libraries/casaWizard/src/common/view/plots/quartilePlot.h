// Quartile plot in time series widget
#pragma once

#include "plotBase.h"

#include <QVector>

class QPointF;

namespace casaWizard
{

class QuartilePlot : public PlotBase
{
  Q_OBJECT

  struct Quartile
  {
    double P10;
    double P90;
    double Q1;
    double median;
    double Q3;
    double xValue;
    double sampleValue;

    Quartile(const double P10 = 0.0, const double P90 = 0.0, const double Q1 = 0.0, const double median = 0.0, const double Q3 = 0.0, const double xValue = 0.0, const double sampleValue = 0.0) :
      P10{P10},
      P90{P90},
      Q1{Q1},
      median{median},
      Q3{Q3},
      xValue{xValue},
      sampleValue{sampleValue}
    {
    }
  };

public:
  explicit QuartilePlot(QWidget* parent = 0);
  void setData(const QVector<double>& xAxisValuesSorted, const QVector<QVector<double>>& valuesMatrix, const QVector<double> pointCoords);

private:
  void clearData() final;
  void updateMinMaxData() final;
  void drawData(QPainter& painter) final;
  void drawMinMaxBar(QPainter& painter, const double xValue, const double y0, const double y1);
  void drawLegend(QPainter& painter, const QPen& penPoint, const QStringList& items);

  QVector<Quartile> quartiles_;
  double quartileSpacing_;
};

} // namespace casaWizard

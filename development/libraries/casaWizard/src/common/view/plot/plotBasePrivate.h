// Class containing private members of plotBase class
#pragma once

#include <QVector>

class QPainter;
class QPointF;
class QFont;
class QString;

namespace casaWizard
{

class PlotBase;

class PlotBase::PlotBasePrivate
{
public:
  explicit PlotBasePrivate();
  PlotBasePrivate& operator = (const PlotBasePrivate&) = delete;
  PlotBasePrivate(const PlotBasePrivate&) = delete;

  void drawBorder(QPainter& painter, const int width, const int height);
  void drawAxes(QPainter& painter);
  void drawTicks(QPainter& painter);
  void drawLabels(QPainter& painter, const int height);
  void updatePlotRange(const int width, const int height);

  QPointF valToPoint(double x, double y);

  double xAxisMinValue() const;
  void setXAxisMinValue(double xAxisMinValue);

  double xAxisMaxValue() const;
  void setXAxisMaxValue(double xAxisMaxValue);

  double yAxisMinValue() const;
  void setYAxisMinValue(double yAxisMinValue);

  double yAxisMaxValue() const;
  void setYAxisMaxValue(double yAxisMaxValue);

  void setXLabel(const QString& xLabel);
  void setYLabel(const QString& yLabel);

  QPointF plotRangeTopLeft() const;
  QPointF plotRangeBottomRight() const;

  QFont font() const;
  void setFont(const QFont& font);

private:
  void tickCalculator(QVector<double>& majorXTicks, QVector<double>& majorYTicks, const double xAxisMinValue,
                      const double xAxisMaxValue, const double yAxisMinValue, const double yAxisMaxValue);
  void simpleTickCalculator(QVector<double>& majorTicks, const double axisMinValue, const double axisMaxValue);

  double xAxisMinValue_;
  double xAxisMaxValue_;
  double yAxisMinValue_;
  double yAxisMaxValue_;

  QVector<double> majorXticks_;
  QVector<double> majorYticks_;
  double maxYtickWidth_;

  QString xLabel_;
  QString yLabel_;

  QPointF plotRangeTopLeft_;
  QPointF plotRangeBottomRight_;

  QFont font_;
};

} // namespace casaWizard

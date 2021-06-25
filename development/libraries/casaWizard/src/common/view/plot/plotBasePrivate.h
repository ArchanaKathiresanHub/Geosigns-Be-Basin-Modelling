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

  QPointF valToPoint(double x, double y) const;
  QPointF pointToVal(double px, double py) const;
  bool validPosition(double px, double py) const;

  double xAxisMinValue() const;
  void setXAxisMinValue(double xAxisMinValue);

  double xAxisMaxValue() const;
  void setXAxisMaxValue(double xAxisMaxValue);

  double yAxisMinValue() const;
  void setYAxisMinValue(double yAxisMinValue);

  double yAxisMaxValue() const;
  void setYAxisMaxValue(double yAxisMaxValue);

  bool invertYAxis() const;
  void setInvertYAxis(bool invertYAxis);

  void setXLabel(const QString& xLabel);
  void setYLabel(const QString& yLabel);
  void setAspectRatio(const double aspectRatio);

  QPointF plotRangeTopLeft() const;
  QPointF plotRangeBottomRight() const;

  QFont font() const;
  void setFont(const QFont& font);

private:
  void calculateTicks();
  void simpleTickCalculator(QVector<double>& majorTicks, const double axisMinValue, const double axisMaxValue);

  double xAxisMinValue_;
  double xAxisMaxValue_;
  double yAxisMinValue_;
  double yAxisMaxValue_;

  QVector<double> majorXticks_;
  QVector<double> majorYticks_;
  double maxYtickWidth_;

  double aspectRatioPlotArea_;

  QString xLabel_;
  QString yLabel_;

  QPointF plotRangeTopLeft_;
  QPointF plotRangeBottomRight_;
  bool invertYAxisLabel_;

  QFont font_;
  void addBaseTicks(const double xBase, const double yBase);
  void addExtraTicks(QVector<double>& ticks, const double axisMinValue, const double axisMaxValue, const double tickSeparation);
};

} // namespace casaWizard

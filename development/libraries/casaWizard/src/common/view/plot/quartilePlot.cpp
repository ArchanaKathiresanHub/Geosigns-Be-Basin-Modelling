#include "quartilePlot.h"

#include "model/percentileGenerator.h"
#include "model/input/cmbDataAccess.h"

#include <QPainter>
#include <QPointF>
#include <QRectF>

#include <algorithm>
#include <cmath>
#include <functional>

namespace casaWizard
{

namespace
{

const double epsilon{1e-12};
const QVector<double> quartileTargets{0.1, 0.25, 0.5, 0.75, 0.9};
QPointF edgeBar(2.0, 0.0);

} // namespace

QuartilePlot::QuartilePlot(QWidget* parent) :
  PlotBase(parent),
  quartiles_{},
  quartileSpacing_{}
{
}

void QuartilePlot::clearData()
{
  quartiles_.clear();
}

void QuartilePlot::setData(const QVector<double>& xAxisValuesSorted, const QVector<QVector<double>>& valuesMatrix, const QVector<double> sampleCoords)
{
  clearData();
  quartileSpacing_ = (xAxisValuesSorted.last() - xAxisValuesSorted[0])/(xAxisValuesSorted.size() - 1)*0.3;

  int i = 0;
  for (QVector<double> values : valuesMatrix)
  {
    Quartile quartileI;

    // Remove undefined values
    values.erase(std::remove_if(values.begin(), values.end(), [](double a)
    {
      return std::fabs(a - CMBDataAccess::DefaultUndefinedMapValue) < epsilon || std::fabs(a - CMBDataAccess::DefaultUndefinedScalarValue) < epsilon;
    }), values.end());

    if (!values.isEmpty())
    {
      QVector<double> currentPercentiles = percentileGenerator::getPercentileValues(values, quartileTargets);

      quartileI.P10 = currentPercentiles[0];
      quartileI.Q1 = currentPercentiles[1];
      quartileI.median = currentPercentiles[2];
      quartileI.Q3 = currentPercentiles[3];
      quartileI.P90 = currentPercentiles[4];
      quartileI.xValue = xAxisValuesSorted[i];
      quartileI.sampleValue = sampleCoords[i];

      quartiles_.push_back(quartileI);
    }
    ++i;
  }

  dataChanged();
}

void QuartilePlot::updateMinMaxData()
{
  QVector<Quartile> tmpQuartiles = quartiles_;
  std::sort(tmpQuartiles.begin(), tmpQuartiles.end(), [](const Quartile& a, const Quartile& b)
  {
    return a.P10 < b.P10;
  });
  double minValueAxis = tmpQuartiles[0].P10;

  std::sort(tmpQuartiles.begin(), tmpQuartiles.end(), [](const Quartile& a, const Quartile& b)
  {
    return a.P90 > b.P90;
  });
  double maxValueAxis = tmpQuartiles[0].P90;

  const double diffYAxis = maxValueAxis - minValueAxis;
  const double minYAxis = minValueAxis - diffYAxis*0.01;
  const double maxYAxis = maxValueAxis + diffYAxis*0.05;

  const double minXAxis = quartiles_[0].xValue - 2*quartileSpacing_;
  const double maxXAxis = quartiles_.last().xValue + 2*quartileSpacing_;

  setMinMaxValues(minXAxis, maxXAxis, minYAxis, maxYAxis);
}

void QuartilePlot::drawData(QPainter& painter)
{
  painter.save();
  painter.drawRect(QRectF(0, 0, width() - 1, height() - 1));

  QPen penPoints = painter.pen();
  penPoints.setWidth(6);
  penPoints.setCapStyle(Qt::RoundCap);
  penPoints.setColor(Qt::red);

  if (!quartiles_.isEmpty())
  {
    int i = 0;
    for (const Quartile& quartile : quartiles_)
    {
      const double left = quartile.xValue - 0.5*quartileSpacing_;
      QPointF topLeft = valToPoint(left, quartile.Q3);
      QPointF bottomRight = valToPoint(left + quartileSpacing_, quartile.Q1);
      painter.drawRect(QRectF(topLeft, bottomRight));

      drawMinMaxBar(painter, quartile.xValue, quartile.Q3, quartile.P90);
      drawMinMaxBar(painter, quartile.xValue, quartile.Q1, quartile.P10);

      const QPointF medianLinep0 = valToPoint(left, quartile.median);
      const QPointF medianLinep1 = valToPoint(left + quartileSpacing_, quartile.median);
      painter.drawLine(medianLinep0, medianLinep1);

      painter.save();
      painter.setRenderHint(QPainter::Antialiasing,true);
      painter.setPen(penPoints);
      const QPointF samplePoint = valToPoint(quartile.xValue, quartile.sampleValue);
      painter.drawPoint(samplePoint);
      painter.restore();

      ++i;
    }
  }

  drawLegend(painter, penPoints);
  painter.restore();
}

void QuartilePlot::drawMinMaxBar(QPainter& painter, const double xValue, const double y0, const double y1)
{
  QPointF point0 = valToPoint(xValue, y0);
  QPointF point1 = valToPoint(xValue, y1);
  painter.drawLine(point1 + edgeBar, point1 - edgeBar);
  painter.drawLine(point0, point1);
}

void QuartilePlot::drawLegend(QPainter& painter, const QPen& penPoint)
{
  const int legendLineWidth = 3;
  const QString legend = "Best matched case";
  const double verticalSpacing = 20;
  const double horizontalSpacing = 40;

  painter.save();
  QPen pen = painter.pen();
  pen.setWidth(legendLineWidth);

  QFontMetrics fm(font());
  const double legendWidth = fm.width(legend);
  const double legendHeight = fm.height();

  const QPointF v(0,verticalSpacing);
  const QPointF h(horizontalSpacing, 0);
  const QPointF textTopLeft = valToPoint(xAxisMaxValue(), yAxisMaxValue()) - QPoint(legendWidth, 0) - h;

  pen.setColor(Qt::black);
  painter.setPen(pen);
  painter.drawText(textTopLeft + h, legend);

  painter.save();
  painter.setPen(penPoint);
  painter.setRenderHint(QPainter::Antialiasing,true);
  painter.drawPoint(textTopLeft + h*0.5);
  painter.restore();

  const QPointF topLeft = textTopLeft + QPoint(0, legendHeight);
  const QPointF bottomRight = topLeft + h*1.2 + QPoint(legendWidth, 0) - QPoint(0, 2*legendHeight);
  painter.drawRect(QRectF(topLeft, bottomRight));

  painter.restore();
}

} // namespace casaWizard

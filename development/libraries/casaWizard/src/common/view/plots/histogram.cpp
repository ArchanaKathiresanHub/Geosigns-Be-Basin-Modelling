#include "histogram.h"

#include "model/histogramGenerator.h"
#include "model/percentileGenerator.h"

#include <QPainter>
#include <QPointF>
#include <QRectF>

#include <algorithm>

namespace casaWizard
{

const int binSpacing{2};
const double epsilon{1e-12};
const QVector<double> percentileTargets{0.1,0.5,0.9};

Histogram::Histogram(QWidget* parent) :
  PlotBase(parent),
  minValue_{0},
  maxValue_{1},
  dataBins_{1},
  percentileValues_{}
{
}

void Histogram::setData(const QVector<double>& values, int bins)
{
  clearData();
  if (values.empty())
  {
     return;
  }
  minValue_ = *std::min_element(values.constBegin(), values.constEnd());
  maxValue_ = *std::max_element(values.constBegin(), values.constEnd());


  // If amount of bins is not provided, set them between 20 and 100, depending on the amount of data points
  if (bins<0)
  {
    const int size = values.size();
    bins = size/50;
    bins = (bins<20)?20:bins;
    bins = (bins>100)?100:bins;
  }

  if (maxValue_ - minValue_ < epsilon)
  {
    bins = 1;
  }

  dataBins_ = histogramGenerator::generateBins(values, bins);
  percentileValues_ = percentileGenerator::getPercentileValues(values, percentileTargets);

  dataChanged();
}

void Histogram::clearData()
{
  dataBins_.clear();
  percentileValues_.clear();
}

void Histogram::updateMinMaxData()
{
  double xAxisMinValue = minValue_;
  double xAxisMaxValue = maxValue_;
  if (maxValue_ - minValue_ < epsilon)
  {
    xAxisMinValue *= 0.9;
    xAxisMaxValue *= 1.1;
  }

  const double yAxisMinValue = 0.0;
  const double yAxisMaxValue = *std::max_element(dataBins_.constBegin(), dataBins_.constEnd());
  setMinMaxValues(xAxisMinValue, xAxisMaxValue, yAxisMinValue, yAxisMaxValue*1.1);
}

void Histogram::drawData(QPainter& painter)
{
  painter.save();
  painter.drawRect(QRectF(0, 0, width() - 1,height() - 1));
  painter.setBrush(Qt::red);

  QPointF offset{0.5*binSpacing, 0.0};
  double left = minValue_;
  const double binSize = (maxValue_ - left)/dataBins_.size();
  double right = left + binSize;
  for (const int& data : dataBins_)
  {
    QPointF topLeft = valToPoint(left, data) + offset;
    QPointF bottomRight = valToPoint(right, 0.0) - offset;
    painter.drawRect(QRectF(topLeft, bottomRight));
    left += binSize;
    right += binSize;
  }

  for ( int i = 0; i<percentileValues_.size(); ++i)
  {
    const QPointF bottom = valToPoint(percentileValues_[i], 0.0);
    const QPointF top = valToPoint(percentileValues_[i], yAxisMaxValue());
    painter.drawLine(bottom, top);

    const QFontMetrics fontMetrics(painter.font());
    const int fontHeight = fontMetrics.height();
    painter.drawText(top + QPointF(-22, fontHeight),
                     "P" + QString::number(percentileTargets[i]*100,'f',0) +
                     " (" + QString::number(percentileValues_[i],'g',4) + ")");
  }

  painter.restore();
}

} // namespace casaWizard


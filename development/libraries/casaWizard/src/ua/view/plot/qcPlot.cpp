#include "qcPlot.h"

#include "model/targetQC.h"

#include <QPainter>
#include <QPointF>
#include <QRectF>

namespace casaWizard
{

namespace ua
{

const int symbolSize = 5;

QCPlot::QCPlot(QWidget* parent) :
  Plot(parent),
  targetValue_{0.0},
  targetStandardDeviation_{0.0},
  hasCalibrationValue_{false},
  hasData_{false}
{
}

void QCPlot::resizeEvent(QResizeEvent* event)
{
  const QSize currentSize = size();
  const int min = std::min(currentSize.width(), currentSize.height());
  resize(min, min);
  Plot::resizeEvent(event);
}

void QCPlot::setTargetQC(const TargetQC& targetQC)
{
  clearData();

  if (targetQC.y().isEmpty())
  {
    hasData_ = false;
    return;
  }

  addXYscatter(targetQC.y(), targetQC.yProxy());
  hasCalibrationValue_ = targetQC.calibration();
  if (hasCalibrationValue_)
  {
    targetValue_ = targetQC.value();
    targetStandardDeviation_ = targetQC.standardDeviation();
  }  
  hasData_ = true;
}

void QCPlot::reset()
{
   clearData();
   targetValue_ = 0.0;
   targetStandardDeviation_ = 0.0;
   hasCalibrationValue_ = false;
   hasData_ = false;
   update();
}

void QCPlot::paintEvent(QPaintEvent* event)
{
  QPainter painter(this);
  if (!hasData_)
  {
    painter.drawText(rect(), QString("No data found, please run casa"), QTextOption(Qt::AlignCenter));
    return;
  }

  const double minV = yAxisMinValue();
  const double maxV = yAxisMaxValue();

  if (hasCalibrationValue_)
  {
    // Draw green box bottom to top, 2x standard deviation wide
    painter.setPen(Qt::green);
    painter.setBrush(QColor(int(0.8*255), 255, int(0.8*255)));
    QPointF p1 = valToPoint(targetValue_ - targetStandardDeviation_, minV);
    QPointF p2 = valToPoint(targetValue_ + targetStandardDeviation_, maxV);
    painter.drawRect(QRectF(p1, p2));

    // Draw green box 2x standard deviation wide and heigh
    painter.setBrush(QColor(int(0.5*255), 255, int(0.5*255)));
    p1 = valToPoint(targetValue_ - targetStandardDeviation_, targetValue_ - targetStandardDeviation_);
    p2 = valToPoint(targetValue_ + targetStandardDeviation_, targetValue_ + targetStandardDeviation_);
    painter.drawRect(QRectF(p1, p2));
  }

  // 90%, 100%, 110% lines
  painter.setPen(Qt::red);
  painter.drawLine(valToPoint(minV, minV*0.9), valToPoint(maxV, maxV*0.9));
  painter.drawLine(valToPoint(minV, minV), valToPoint(maxV, maxV));
  painter.drawLine(valToPoint(minV, minV*1.1), valToPoint(maxV, maxV*1.1));

  // 90%, 100%, 110% text
  painter.setPen(Qt::black);
  double dx = (maxV-minV)*0.13;
  painter.drawText(valToPoint(maxV - dx,       maxV - dx),       QString("100%"));
  painter.drawText(valToPoint(maxV - dx,       (maxV - dx)*0.9), QString("90%"));
  painter.drawText(valToPoint((maxV - dx)*0.9, maxV - dx),       QString("110%"));

  if (hasCalibrationValue_)
  {
    // Target value
    painter.setPen(Qt::red);
    painter.setBrush(Qt::white);
    painter.drawEllipse(valToPoint(targetValue_, targetValue_), symbolSize, symbolSize);
  }

  // Also execute the parent paint event
  Plot::paintEvent(event);
}

} // namespace ua

} // namespace casaWizard

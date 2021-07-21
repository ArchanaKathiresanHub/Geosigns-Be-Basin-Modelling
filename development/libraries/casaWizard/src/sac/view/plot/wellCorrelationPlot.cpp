//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "wellCorrelationPlot.h"

#include "model/wellTrajectory.h"

#include <QPainter>

#include <assert.h>

namespace casaWizard
{

namespace sac
{

WellCorrelationPlot::WellCorrelationPlot(QWidget* parent) :
  Plot(parent),
  completeLegend_{},
  wellIndices_{},
  minValue_{0.0},
  maxValue_{1.0},
  absoluteErrorMargin_{0.0},
  relativeErrorMargin_{0.1}
{
  QVector<QString> legend(4, "");
  legend[TrajectoryType::Original1D] = "Original 1d case";
  legend[TrajectoryType::Optimized1D] = "Optimized 1d case";
  legend[TrajectoryType::Original3D] = "Original 3d case";
  legend[TrajectoryType::Optimized3D] = "Optimized 3d case";
  completeLegend_ = legend.toList();
  setLegend(completeLegend_);
  setXLabel("Measurement");
  setYLabel("Simulation");
  setSeparateLegend(true);

  connect(this, SIGNAL(pointSelectEvent(int,int)), this, SLOT(selectedPoint(int,int)));
}

void WellCorrelationPlot::setData( const QVector<QVector<double>>& measuredValueTrajectories,
                                   const QVector<QVector<double>>& simulatedValueTrajectories,
                                   const QString activeProperty,
                                   const QVector<bool> activePlots,
                                   const double minValue,
                                   const double maxValue,
                                   const QVector<int>& wellIndices)
{
  wellIndices_.clear();
  clearData();
  QStringList activeLegend;
  wellIndices_ = wellIndices;

  int counter = 0;
  for (int i = 0; i < activePlots.size(); i++)
  {
    if (activePlots[i])
    {
      activeLegend.append(completeLegend_[i]);
      addXYscatter(measuredValueTrajectories[counter], simulatedValueTrajectories[counter], i);
      counter++;
    }
  }

  minValue_ = minValue;
  maxValue_ = maxValue;
  setXLabel("Measured " + activeProperty);
  setYLabel("Simulated " + activeProperty);
  setLegend(activeLegend);
  updateMinMaxData();
  update();
}

void WellCorrelationPlot::clear()
{
  clearData();
  update();
}

void WellCorrelationPlot::setAbsoluteErrorMargin(double absoluteErrorMargin)
{
  absoluteErrorMargin_ = absoluteErrorMargin;
}

void WellCorrelationPlot::setRelativeErrorMargin(double relativeErrorMargin)
{
  relativeErrorMargin_ = relativeErrorMargin;
}

void WellCorrelationPlot::updateMinMaxData()
{
  setMinMaxValues(minValue_, maxValue_, minValue_, maxValue_);
}

void WellCorrelationPlot::resizeEvent(QResizeEvent* event)
{
  const QSize currentSize = size();
  const int min = std::min(currentSize.width(), currentSize.height());
  resize(min, min);
  Plot::resizeEvent(event);
}

void WellCorrelationPlot::selectedPoint(int /*plot*/, int point)
{
  if (point >= wellIndices_.size())
  {
    return;
  }
  emit selectedWell(wellIndices_[point]);
}

void WellCorrelationPlot::paintEvent(QPaintEvent* event)
{
  Plot::paintEvent(event);

  const double minValue = yAxisMinValue();
  const double maxValue = yAxisMaxValue();

  drawDiagonal(minValue, maxValue);
  drawAbsoluteErrorMargin(minValue, maxValue);
  drawRelativeErrorMargin(minValue, maxValue);
}

void WellCorrelationPlot::drawDiagonal(const double minValue, const double maxValue)
{
  QPainter painter(this);
  QPen pen(Qt::black);
  pen.setStyle(Qt::DashDotLine);
  pen.setWidth(1);
  painter.setPen(pen);
  painter.drawLine(valToPoint(minValue, minValue), valToPoint(maxValue, maxValue));
}

void WellCorrelationPlot::drawAbsoluteErrorMargin(const double minValue, const double maxValue)
{
  if (absoluteErrorMargin_ > 0)
  {
    QPainter painter(this);
    QPen pen = painter.pen();
    pen.setStyle(Qt::DashDotLine);
    pen.setWidth(2);
    pen.setColor(QColor(255, 213, 0));
    painter.setPen(pen);

    painter.drawLine(valToPoint(minValue, minValue + absoluteErrorMargin_),
                     valToPoint(maxValue - absoluteErrorMargin_, maxValue));
    painter.drawLine(valToPoint(minValue + absoluteErrorMargin_, minValue ),
                     valToPoint(maxValue, maxValue - absoluteErrorMargin_));
  }
}

void WellCorrelationPlot::drawRelativeErrorMargin(const double minValue, const double maxValue)
{
  if (relativeErrorMargin_ > 0)
  {
    QPainter painter(this);
    QPen pen = painter.pen();
    pen.setStyle(Qt::DashDotLine);
    pen.setWidth(2);
    pen.setColor(QColor(255,165,0));
    painter.setPen(pen);

    painter.drawLine(valToPoint(minValue, minValue + relativeErrorMargin_*minValue),
                     valToPoint(maxValue, maxValue + relativeErrorMargin_*maxValue));
    painter.drawLine(valToPoint(minValue / (1-relativeErrorMargin_) , minValue),
                     valToPoint(maxValue, maxValue - relativeErrorMargin_*maxValue));
  }
}

}  // namespace sac

}  // namespace casaWizard

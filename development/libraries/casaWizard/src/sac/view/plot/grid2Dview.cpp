//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "grid2Dview.h"

#include "../colormap.h"
#include "../lithoMapsToolTip.h"
#include "model/optimizedLithofraction.h"

// QT
#include <QGridLayout>
#include <QLabel>
#include <QPainter>

#include <cmath>

namespace casaWizard
{

namespace sac
{

Grid2DView::Grid2DView(const ColorMap& colormap, QWidget* parent) :
  WellBirdsView(parent),
  colorMap_{colormap},
  range_{new std::pair<double, double>(0, 100)},
  values_{},
  lithoMapsToolTip_{new LithoMapsToolTip(this)},
  fixedRange_{false},
  stretched_{false},
  wellsVisible_{true}
{
  lithoMapsToolTip_->setVisible(false);
}

void Grid2DView::updatePlots(const QVector<QVector<double>> values)
{
  values_ = values;
  determineRange();

  update();
}


void Grid2DView::updateRange(const double xMin, const double xMax, const double yMin, const double yMax)
{
  setMinMaxValues(xMin, xMax, yMin, yMax);

  stretch();
}

void Grid2DView::stretch()
{
  if (stretched_)
  {
    setAspectRatio(0);
  }
  else
  {
    setAspectRatio((xAxisMaxValue() - xAxisMinValue()) / (yAxisMaxValue() - yAxisMinValue()));
  }
}

void Grid2DView::setWellsVisible(const bool wellsVisible)
{
  wellsVisible_ = wellsVisible;
  update();
}

void Grid2DView::setStretch(const bool stretched)
{
  stretched_ = stretched;
  stretch();
  update();
}

void Grid2DView::clearData()
{
  values_.clear();
  range_.reset(new std::pair<double, double>(0, 100));

  update();
}

const std::pair<double, double>& Grid2DView::getValueRange() const
{
  return *range_;
}

void Grid2DView::setFixedValueRange(const std::pair<double, double>& valueRange)
{
  range_.reset(new std::pair<double,double>(valueRange));
  fixedRange_ = true;
  update();
}

void Grid2DView::setVariableValueRange()
{
  fixedRange_ = false;
  determineRange();
  update();
}

void Grid2DView::drawData(QPainter& painter)
{
  if (values_.empty())
  {
    return;
  }

  const int valuesX = values_[0].size();
  const int valuesY = values_.size();

  const double deltaX = (xAxisMaxValue() - xAxisMinValue()) * 1.0 / valuesX;
  const double deltaY = (yAxisMaxValue() - yAxisMinValue()) * 1.0 / valuesY;

  const double w = 1.0*width() / valuesX;
  const double h = 1.0*height() / valuesY;

  determineRange();
  double yd = yAxisMinValue();
  QColor color;
  for ( const QVector<double>& rows : values_)
  {
    double xd = xAxisMinValue();
    for ( const double& value : rows )
    {
      color = colorMap_.getColor(value, range_->first, range_->second);
      painter.fillRect(QRectF(valToPoint(xd, yd) + QPointF(0, -h), QSizeF(w, h)), QBrush(color));
      xd = xd + deltaX;
    }
    yd += deltaY;
  }

  if (wellsVisible_)
  {
    painter.save();
    drawPieChartsWells(painter);
    drawPieChartsSelectedWells(painter);
    painter.restore();
  }
}

void Grid2DView::drawPieChartsWells(QPainter& painter)
{
  const int shade = activeWells().empty() ? 255 : 150;
  painter.setPen(Qt::black);
  painter.setRenderHint(QPainter::Antialiasing, true);
  int counter = 0;
  for (const OptimizedLithofraction& optimizedLitho : optimizedLithofractions_)
  {
    drawPieChart(painter, shade, 10, counter, optimizedLitho);
    counter++;
  }
}

void Grid2DView::drawPieChart(QPainter& painter, const int shade, const int size, const int counter, const OptimizedLithofraction& optimizedLitho)
{
  QPoint position = valToPoint(x()[counter], y()[counter]).toPoint();

  double firstAngle = - optimizedLitho.optimizedPercentageFirstComponent()/100*360*16;
  double secondAngle = - optimizedLitho.optimizedPercentageSecondComponent()/100*360*16;
  double thirdAngle = - optimizedLitho.optimizedPercentageThirdComponent()/100*360*16;

  painter.setBrush(QColor(shade, 0, 0));
  painter.drawPie(QRect(position - QPoint(size,size), position + QPoint(size,size)), 90 * 16, firstAngle);

  painter.setBrush(QColor(0, shade, 0));
  painter.drawPie(QRect(position - QPoint(size,size), position + QPoint(size,size)), 90 * 16+ firstAngle, secondAngle);

  painter.setBrush(QColor(0, 0, shade));
  painter.drawPie(QRect(position - QPoint(size,size), position + QPoint(size,size)), 90 * 16+secondAngle + firstAngle, thirdAngle);
}


void Grid2DView::drawPieChartsSelectedWells(QPainter& painter)
{
  QPen border(Qt::black);
  border.setWidthF(1.5);
  painter.setPen(border);
  painter.setRenderHint(QPainter::Antialiasing, true);

  for (int i : activeWells())
  {
    drawPieChart(painter, 255, 12, i, optimizedLithofractions_[i]);
  }
}

void Grid2DView::updateMinMaxData()
{

}

void Grid2DView::setOptimizedLithofractions(const QVector<OptimizedLithofraction>& optimizedLithofractions)
{
  optimizedLithofractions_ = optimizedLithofractions;
  update();
}

void Grid2DView::mousePressEvent(QMouseEvent* event)
{
  setToolTipVisible(false);
  mousePosition_ = event->pos();

  if (validPosition(mousePosition_.x(), mousePosition_.y()))
  {
    initializeToolTip(mousePosition_);
  }
}


void Grid2DView::initializeToolTip(const QPoint& mousePosition)
{
  const QPointF domainPosition = pointToVal(mousePosition.x(), mousePosition.y());
  lithoMapsToolTip_->setDomainPosition(domainPosition);

  lithoMapsToolTip_->move(mousePosition);
  lithoMapsToolTip_->setMaximumHeight(height()/2);

  emit toolTipCreated(domainPosition);
}

double Grid2DView::getValue(const QPointF& point) const
{
  const int valuesX = values_[0].size();
  const int valuesY = values_.size();

  const int xIndex = (point.x() - xAxisMinValue()) / (xAxisMaxValue() - xAxisMinValue()) * valuesX;
  const int yIndex = (point.y() - yAxisMinValue()) / (yAxisMaxValue() - yAxisMinValue()) * valuesY;

  return values_[yIndex][xIndex];
}

void Grid2DView::setToolTipData(const std::vector<double>& lithofractionsAtPoint, const int activePlot)
{
  lithoMapsToolTip_->setLithofractions(lithofractionsAtPoint, activePlot);
}

void Grid2DView::correctToolTipPositioning()
{
  QPoint toolTipPosition = lithoMapsToolTip_->pos();
  const bool moveX = !validPosition(toolTipPosition.x() + lithoMapsToolTip_->width(), toolTipPosition.y());
  const bool moveY = !validPosition(toolTipPosition.x(), toolTipPosition.y() + lithoMapsToolTip_->height());
  if (moveY)
  {
    lithoMapsToolTip_->move(toolTipPosition - (QPoint(0,lithoMapsToolTip_->height())));
    toolTipPosition = lithoMapsToolTip_->pos();
  }
  if (moveX)
  {
    lithoMapsToolTip_->move(toolTipPosition - (QPoint(lithoMapsToolTip_->width(),0)));
  }
  lithoMapsToolTip_->setCorner(moveX, moveY);
  lithoMapsToolTip_->setVisible(true);
}

void Grid2DView::setToolTipVisible(const bool visible)
{
  lithoMapsToolTip_->setVisible(visible);
}

void Grid2DView::determineRange()
{
  if (fixedRange_)
  {
    return;
  }

  double min = values_[0][0];
  double max = values_[0][0];

  for ( const QVector<double>& rows : values_)
  {
    for ( const double& value : rows )
    {
      min = value < min ? value : min;
      max = value > max ? value : max;
    }
  }

  if (min == max)
  {
    min -= 1e-8;
    max += 1e-8;
  }

  range_.reset(new std::pair<double, double>(min, max));
}

} // namespace sac

} // namespace casaWizard

//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "grid2Dview.h"

#include "../colormap.h"
#include "model/input/cmbDataAccess.h"

// QT
#include <QGridLayout>
#include <QPainter>

#include <cmath>

namespace casaWizard
{

namespace sac
{

Grid2DView::Grid2DView(const ColorMap& colormap, QWidget* parent) :
  Plot(parent),
  colorMap_{colormap},
  range_{new std::pair<double, double>(0, 100)},
  values_{},
  fixedRange_{false},
  stretched_{false},
  singleValue_{false}
{
}

void Grid2DView::updatePlots(const std::vector<std::vector<double>>& values, const std::vector<std::vector<double>>& depthMap)
{
  values_ = values;
  for (int i = 0; i< values.size(); ++i)
  {
    for (int j = 0; j<values[0].size(); ++j)
    {
      if (singleValue_)
      {
        values_[i][j] = 1.0;
      }
      if (depthMap[i][j] == CMBDataAccess::DefaultUndefinedMapValue)
      {
        values_[i][j] = CMBDataAccess::DefaultUndefinedMapValue;
      }
    }
  }

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

  Plot::clearData();

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

  determineRange();

  const QRect rect(valToPoint(xAxisMinValue(), yAxisMaxValue()).toPoint(), valToPoint(xAxisMaxValue(), yAxisMinValue()).toPoint());

  double dx = values_[0].size() * 1.0 / rect.width();
  double dy = values_.size() * 1.0 / rect.height();
  int nx = rect.width();
  int ny = rect.height();

  if (dx < 1.0)
  {
    nx = values_[0].size();
    dx = 1.0;
  }
  if (dy < 1.0)
  {
    ny = values_.size();
    dy = 1.0;
  }

  const QSize size(nx,ny);
  QImage image(size, QImage::Format_ARGB32);
  QPainter p(&image);

  QPen backPen = p.pen();
  backPen.setColor(colorMap_.getBackgroundColor());
  QPen pen = p.pen();

  int yd = ny - 1;
  for ( int j = 0; j < ny; ++j )
  {    
    int xd = 0.0;
    const std::vector<double>& row = values_[j*dy];
    for ( int i = 0; i < nx; ++i )
    {
      const double value = row[i*dx];

      if ( value == CMBDataAccess::DefaultUndefinedMapValue )
      {
        p.setPen(backPen);
      }
      else
      {
        pen.setColor(singleValue_ ? Qt::white : colorMap_.getColor(value, range_->first, range_->second));
        p.setPen(pen);
      }

      p.drawPoint(xd, yd);
      xd += 1;
    }
    yd -= 1;
  }

  painter.drawImage(rect, image);
}

void Grid2DView::updateMinMaxData()
{
}

void Grid2DView::setSingleValue(const bool singleValue)
{
  singleValue_ = singleValue;
}

bool Grid2DView::singleValue() const
{
  return singleValue_;
}

double Grid2DView::getValue(const QPointF& point) const
{
  const int valuesX = values_[0].size();
  const int valuesY = values_.size();

  const int xIndex = (point.x() - xAxisMinValue()) / (xAxisMaxValue() - xAxisMinValue()) * valuesX;
  const int yIndex = (point.y() - yAxisMinValue()) / (yAxisMaxValue() - yAxisMinValue()) * valuesY;

  return values_[yIndex][xIndex];
}

void Grid2DView::determineRange()
{
  if (fixedRange_)
  {
    return;
  }

  double min = 1.e99;
  double max = -min;

  for ( const std::vector<double>& rows : values_)
  {
    for ( const double& value : rows )
    {
      if ( value == CMBDataAccess::DefaultUndefinedMapValue)
      {
        continue;
      }

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

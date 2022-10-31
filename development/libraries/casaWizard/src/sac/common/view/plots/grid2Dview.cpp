//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "grid2Dview.h"

#include "view/colormap.h"
#include "model/input/cmbDataAccess.h"

// QT
#include <QPainter>

#include <cmath>

namespace casaWizard
{

namespace sac
{

Grid2DView::Grid2DView(const ColorMap& colormap, QWidget* parent) :
  Plot(parent),
  m_colorMap{colormap},
  m_range{new std::pair<double, double>(0, 100)},
  m_values{},
  m_fixedRange{false},
  m_stretched{false}
{
   stretch();
}

void Grid2DView::updatePlots(const std::vector<std::vector<double>>& values)
{
  m_values = values;
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
  if (m_stretched)
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
  m_stretched = stretched;
  stretch();
  update();
}

void Grid2DView::clearData()
{
  m_values.clear();
  m_range.reset(new std::pair<double, double>(0, 100));

  Plot::clearData();

  update();
}

const std::pair<double, double>& Grid2DView::getValueRange() const
{
  return *m_range;
}

void Grid2DView::setFixedValueRange(const std::pair<double, double>& valueRange)
{
  m_range.reset(new std::pair<double,double>(valueRange));
  m_fixedRange = true;
  update();
}

void Grid2DView::setVariableValueRange()
{
  m_fixedRange = false;
  determineRange();
  update();
}

void Grid2DView::drawData(QPainter& painter)
{
  if (m_values.empty())
  {
    return;
  }

  determineRange();

  const QRect rect(valToPoint(xAxisMinValue(), yAxisMaxValue()).toPoint(), valToPoint(xAxisMaxValue(), yAxisMinValue()).toPoint());

  double dx = m_values[0].size() * 1.0 / rect.width();
  double dy = m_values.size() * 1.0 / rect.height();
  int nx = rect.width();
  int ny = rect.height();

  if (dx < 1.0)
  {
    nx = m_values[0].size();
    dx = 1.0;
  }
  if (dy < 1.0)
  {
    ny = m_values.size();
    dy = 1.0;
  }

  const QSize size(nx,ny);
  QImage image(size, QImage::Format_ARGB32);
  QPainter p(&image);

  QPen backPen = p.pen();
  backPen.setColor(m_colorMap.getBackgroundColor());
  QPen pen = p.pen();

  int yd = ny - 1;
  for ( int j = 0; j < ny; ++j )
  {    
    int xd = 0.0;
    const std::vector<double>& row = m_values[j*dy];
    for ( int i = 0; i < nx; ++i )
    {
      const double value = row[i*dx];

      if ( value == CMBDataAccess::DefaultUndefinedMapValue || std::isnan(value))
      {
        p.setPen(backPen);
      }
      else
      {
        pen.setColor(m_colorMap.getColor(value, m_range->first, m_range->second));
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

double Grid2DView::getValue(const QPointF& point) const
{
  const int valuesX = m_values[0].size();
  const int valuesY = m_values.size();

  const int xIndex = (point.x() - xAxisMinValue()) / (xAxisMaxValue() - xAxisMinValue()) * valuesX;
  const int yIndex = (point.y() - yAxisMinValue()) / (yAxisMaxValue() - yAxisMinValue()) * valuesY;

  return m_values[yIndex][xIndex];
}

void Grid2DView::determineRange()
{
  if (m_fixedRange)
  {
    return;
  }

  double min = 1e+99;
  double max = -min;

  for ( const std::vector<double>& rows : m_values)
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

  m_range.reset(new std::pair<double, double>(min, max));
}

const ColorMap& Grid2DView::colorMap() const
{
   return m_colorMap;
}

} // namespace sac

} // namespace casaWizard

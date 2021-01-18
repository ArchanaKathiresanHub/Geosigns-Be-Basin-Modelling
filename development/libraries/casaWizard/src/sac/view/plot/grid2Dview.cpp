#include "grid2Dview.h"

#include <cmath>
#include "../colormap.h"

// QT
#include <QGridLayout>
#include <QLabel>
#include <QPainter>

namespace casaWizard
{

namespace sac
{

Grid2DView::Grid2DView(const ColorMap& colormap, QWidget* parent) :
  WellBirdsView(parent),
  colorMap_{colormap},
  range_{new std::pair<double, double>(0, 100)},
  values_{},
  fixedRange_{false},
  stretched_{false},
  wellsVisible_{true}
{
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
    WellBirdsView::drawData(painter);
  }
}

void Grid2DView::updateMinMaxData()
{

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

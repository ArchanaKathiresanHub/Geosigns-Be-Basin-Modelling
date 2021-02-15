//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "colorbar.h"

#include "colormap.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>

#include <cmath>
#include <iomanip>
#include <sstream>


namespace casaWizard
{

namespace sac
{

ColorBar::ColorBar(const ColorMap& colormap, QWidget *parent) :
  QWidget(parent),
  colorMap_(colormap),
  numberOfTicks_{11},
  range_{std::pair<double, double>(0, 100)}
{
}

void ColorBar::paintEvent(QPaintEvent* /*event*/)
{
  setColorbarProperties();
  obtainTickPositionsAndLabels();
  drawColorBar();
}

void ColorBar::setRange(const std::pair <double, double>& range)
{
  range_ = range;

  update();
}

void ColorBar::setColorbarProperties()
{
  const QSize pixels = size();
  colorbarHeight_ = static_cast<int>(0.95*pixels.height());
  colorbarWidth_ = static_cast<int>(0.25*pixels.width());
  colorbarOffsetY_ = (pixels.height() - colorbarHeight_) / 2;
}

void ColorBar::obtainTickPositionsAndLabels()
{
  const int decimals = 1;
  numberOfTicks_ = 11;
  bool appropriateNumberOfTicks = false;

  while (appropriateNumberOfTicks == false)
  {
    appropriateNumberOfTicks = placeTicks(decimals);
  }
}

bool ColorBar::placeTicks(const int decimals)
{
  tickPositions_.clear();
  tickLabels_.clear();

  if ((range_.second - range_.first) < 1e-5 || numberOfTicks_ == 1)
  {
    tickLabels_.push_back(doubleToString(range_.second, decimals));
    tickPositions_.push_back(0.5 * colorbarHeight_ - 1);
    return true;
  }

  tickLabels_.push_back(doubleToString(range_.second, decimals));
  tickPositions_.push_back(0);

  for (int i = 1; i < numberOfTicks_; i++)
  {
    double relativePosition = i/(numberOfTicks_-1.0);

    double labelValue = (1.0 - relativePosition) * (range_.second - range_.first) + range_.first;
    std::string label = doubleToString(labelValue, decimals);

    if (duplicateLabel(label))
    {
      numberOfTicks_--;
      return false;
    }

    tickPositions_.push_back(relativePosition * colorbarHeight_ - 1);
    tickLabels_.push_back(label);
  }

  return true;
}


std::string ColorBar::doubleToString(const double number, const int decimals) const
{
  std::ostringstream label;
  label << std::fixed;
  label << std::setprecision(decimals);
  label << number;
  return label.str();
}

bool ColorBar::duplicateLabel(const std::string& label) const
{
  for (const std::string& tickLabel : tickLabels_)
  {
    if (tickLabel == label)
    {
      return true;
    }
  }

  return false;
}

void ColorBar::drawColorBar()
{
  QPainter painter(this);
  QColor color;

  int tickCounter = 0;
  double xd = 0.0;
  for ( int j = 0; j < colorbarHeight_; j++)
  {
    if (isVerticalTickPosition(j))
    {
      painter.drawText(colorbarWidth_ + 5, j + 5 + colorbarOffsetY_, QString(tickLabels_[tickCounter].c_str()));
      tickCounter++;
    }

    double yd = 0.0;
    for ( int i = 0; i < colorbarWidth_; i++)
    {
      color = getColor(i,j);
      painter.fillRect(QRectF(i, j + colorbarOffsetY_, 1.0, 1.0), QBrush(color));
      yd += 1.0;
    }
    xd += 1.0;
  }
}

QColor ColorBar::getColor(const int i, const int j) const
{
  if (i == 0 || j == 0 || i == colorbarWidth_ - 1 || j == colorbarHeight_ - 1)
  {
    return QColor(0, 0, 0);
  }
  else if ( isVerticalTickPosition(j) && i > colorbarWidth_/2)
  {
    return QColor(0, 0, 0);
  }
  else
  {
    return colorMap_.getColor((range_.second - range_.first)*(colorbarHeight_ - 1.0*j)/colorbarHeight_ + range_.first, range_.first, range_.second);
  }
}

bool ColorBar::isVerticalTickPosition(int i) const
{
  for (int position : tickPositions_)
  {
    if (i == position)
    {
      return true;
    }
  }

  return false;
}

} // namespace sac

} // namespace casaWizard

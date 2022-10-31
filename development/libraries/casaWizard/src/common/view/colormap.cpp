//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "colormap.h"
#include <cmath>

namespace casaWizard
{

ColorMap::ColorMap() :
  m_colorMapType{ColorMapType::VIRIDIS}
{
}

ColorMap::ColorMap(ColorMapType type) :
   m_colorMapType{type}
{
}

QColor ColorMap::getBackgroundColor() const
{
  return Qt::lightGray;
}

QColor ColorMap::getContrastingColor() const
{
   switch(m_colorMapType)
   {
      case ColorMapType::GRAYSCALES:
         return Qt::red;
      case ColorMapType::RAINBOW:
         return Qt::lightGray;
      case ColorMapType::VIRIDIS:
         return Qt::red;
      default:
         return Qt::lightGray;
   }
}

void ColorMap::setColorMapType(const std::string& colorMapType)
{
  if (colorMapType == "Gray scale")
  {
    m_colorMapType = ColorMapType::GRAYSCALES;
  }
  else if (colorMapType == "Rainbow")
  {
    m_colorMapType = ColorMapType::RAINBOW;
  }
  else if (colorMapType == "Viridis")
  {
    m_colorMapType = ColorMapType::VIRIDIS;
  }
}

void ColorMap::setColorMapType(const ColorMapType& colorMapType){
   m_colorMapType = colorMapType;
}

QColor ColorMap::getColor(const double value, const double minValue, const double maxValue) const
{
  if (minValue == maxValue || value < minValue || value > maxValue)
  {
    return QColor(0,0,0);
  }

  const double relativeValue = (value - minValue) / (maxValue - minValue);
  switch (m_colorMapType)
  {
    case ColorMapType::GRAYSCALES :
      return getGrayScaleColor(relativeValue);
    case ColorMapType::RAINBOW :
      return getRainbowColor(relativeValue);
    case ColorMapType::VIRIDIS :
      return getViridisColor(relativeValue);
   }

  return QColor(0,0,0);
}

QColor ColorMap::getGrayScaleColor(const double relativeValue) const
{
  return QColor(relativeValue*255, relativeValue*255, relativeValue*255);
}

QColor ColorMap::getRainbowColor(const double relativeValue) const
{
  std::vector<QColor> colorsInMap;
  colorsInMap.push_back(QColor(128, 0, 128));
  colorsInMap.push_back(QColor(0, 0, 255));
  colorsInMap.push_back(QColor(0, 255, 255));
  colorsInMap.push_back(QColor(0, 255, 0));
  colorsInMap.push_back(QColor(255, 255, 0));
  colorsInMap.push_back(QColor(255, 0, 0));

  return interpolateColorMap(colorsInMap, relativeValue);
}

QColor ColorMap::getViridisColor(const double relativeValue) const
{
  std::vector<QColor> colorsInMap;
  colorsInMap.push_back(QColor(68, 1, 84));
  colorsInMap.push_back(QColor(72, 40, 120));
  colorsInMap.push_back(QColor(62, 74, 137));
  colorsInMap.push_back(QColor(49, 104, 142));
  colorsInMap.push_back(QColor(38, 130, 142));
  colorsInMap.push_back(QColor(31, 158, 137));
  colorsInMap.push_back(QColor(53, 183, 121));
  colorsInMap.push_back(QColor(109, 205, 89));
  colorsInMap.push_back(QColor(180, 222, 44));
  colorsInMap.push_back(QColor(253, 231, 37));

  return interpolateColorMap(colorsInMap, relativeValue);
}


QColor ColorMap::interpolateColorMap(const std::vector<QColor>& colorsInMap, const double relativeValue) const
{
  if (relativeValue >= 1.0)
  {
    return colorsInMap[colorsInMap.size()-1];
  }
  if (relativeValue <= 0.0)
  {
    return colorsInMap[0];
  }
  const int colorRegion = relativeValue * (colorsInMap.size() - 1);
  const double relativeSegmentValue = (colorsInMap.size() - 1) * (relativeValue - colorRegion/(colorsInMap.size() - 1.0));

  const QColor startColor = colorsInMap[colorRegion];
  const QColor endColor = colorsInMap[colorRegion+1];

  return interPolateColor(startColor, endColor, relativeSegmentValue);
}

QColor ColorMap::interPolateColor(const QColor& startColor, const QColor& endColor, const double relativeSegmentValue) const
{
  int RStart, GStart, BStart;
  startColor.getRgb(&RStart, &GStart, &BStart);

  int RValueEnd, GValueEnd, BValueEnd;
  endColor.getRgb(&RValueEnd, &GValueEnd, &BValueEnd);

  const double RInterpolated = interpolateDouble(RStart, RValueEnd, relativeSegmentValue);
  const double GInterpolated = interpolateDouble(GStart, GValueEnd, relativeSegmentValue);
  const double BInterpolated = interpolateDouble(BStart, BValueEnd, relativeSegmentValue);

  return QColor(RInterpolated, GInterpolated, BInterpolated);
}


double ColorMap::interpolateDouble(const double start, const double end, const double fraction) const
{
  return start + (end - start) * fraction;
}

} // namespace casaWizard

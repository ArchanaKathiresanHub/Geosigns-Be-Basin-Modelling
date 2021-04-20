#pragma once

#include <string>
#include <vector>
#include <QColor>

namespace casaWizard
{

namespace sac
{

enum ColorMapType
{
  GRAYSCALES,
  RAINBOW,
  VIRIDIS
};

class ColorMap
{
public:
  ColorMap();
  void setColorMapType(const std::string& colorMapType);  
  QColor getColor(const double value, const double minValue, const double maxValue) const;

  QColor getBackgroundColor() const;
private:
  ColorMapType m_colorMapType;
  QColor getGrayScaleColor(const double relativeValue) const;
  QColor getRainbowColor(const double relativeValue) const;
  QColor getViridisColor(const double relativeValue) const;
  QColor interpolateColorMap(const std::vector<QColor>& colorsInMap, const double relativeValue) const;
  double interpolateDouble(const double start, const double end, const double fraction) const;
  QColor interPolateColor(const QColor& startColor, const QColor& endColor, const double relativeSegmentValue) const;
};

} // namespace sac
} // namespace casaWizard



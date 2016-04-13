#include "ColorMap.h"

#include <MeshVizXLM/MxTimeStamp.h>

ColorMap::ColorMap()
  : m_undefinedColor(.5f, .5f, .5f, 1.f)
  , m_minVal(0.0)
  , m_maxVal(1.0)
  , m_logMinVal(0.0)
  , m_logMaxVal(1.0)
  , m_type(Default)
  , m_logarithmic(false)
  , m_timeStamp(MxTimeStamp::getTimeStamp())
{
}

SbColorRGBA ColorMap::getColor(double value) const
{
  double normalizedValue;
  if (m_logarithmic)
    //normalizedValue = (log10(value) - m_logMinVal) / (m_logMaxVal -m_logMinVal);
    normalizedValue = log10(value) / m_logMaxVal;
  else
    normalizedValue = (value - m_minVal) / (m_maxVal - m_minVal);

  //if (normalizedValue < 0.0 || normalizedValue > 1.0)
  //  return m_undefinedColor;

  normalizedValue = (normalizedValue < 0.0)
    ? 0.0
    : (normalizedValue > 1.0)
      ? 1.0
      : normalizedValue;

  SbColorRGBA color;
  switch (m_type)
  {
  case Grey:
    color.setHSVAValue(0.f, 0.f, (float)normalizedValue, 1.f);
    break;

  case Default:
  default:
    color.setHSVAValue((float)(.667 * (1.0 - normalizedValue)), 1.f, 1.f, 1.f);
    break;
  }

  return color;
}

bool ColorMap::getLogarithmic() const
{
  return m_logarithmic;
}

void ColorMap::setLogarithmic(bool enable)
{
  if (enable != m_logarithmic)
  {
    m_logarithmic = enable;

    if (m_logarithmic)
    {
      m_logMinVal = log10(m_minVal);
      m_logMaxVal = log10(m_maxVal);
    }

    m_timeStamp = MxTimeStamp::getTimeStamp();
  }
}

void ColorMap::setRange(double minval, double maxval)
{
  m_minVal = minval;
  m_maxVal = maxval;

  if (m_logarithmic)
  {
    m_logMinVal = log10(m_minVal);
    m_logMaxVal = log10(m_maxVal);
  }

  m_timeStamp = MxTimeStamp::getTimeStamp();
}

void ColorMap::getRange(double& minval, double& maxval) const
{
  minval = m_minVal;
  maxval = m_maxVal;
}

size_t ColorMap::getTimeStamp() const
{
  return m_timeStamp;
}



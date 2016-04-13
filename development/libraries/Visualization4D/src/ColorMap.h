#ifndef COLORMAP_H_INCLUDED
#define COLORMAP_H_INCLUDED

#include <Inventor/SbColorRGBA.h>
#include <MeshVizXLM/mapping/interfaces/MiColorMapping.h>

class ColorMap : public MiColorMapping<double, SbColorRGBA>
{
public:

  enum Type
  {
    Grey,
    Default
  };

private:

  SbColorRGBA m_undefinedColor;

  double m_minVal;
  double m_maxVal;
  double m_logMinVal;
  double m_logMaxVal;

  Type   m_type;
  bool   m_logarithmic;

  size_t m_timeStamp;

public:

  ColorMap();

  virtual SbColorRGBA getColor(double value) const;
  
  bool getLogarithmic() const;
  
  void setLogarithmic(bool enable);
  
  void setRange(double minval, double maxval);
  
  virtual void getRange(double& minval, double& maxval) const;
  
  virtual size_t getTimeStamp() const;
};

#endif
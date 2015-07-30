#include "Property.h"

#include "Interface/GridMap.h"

#include <MeshVizInterface/MxTimeStamp.h>

namespace di = DataAccess::Interface;

ScalarProperty::ScalarProperty(const std::string& name, const di::GridMap* values)
: m_values(values)
, m_name(name)
, m_binding(MiDataSet::PER_NODE)
, m_timestamp(MxTimeStamp::getTimeStamp())
{
  m_numI = values->numI();
  m_numJ = values->numJ();
  m_numK = values->getDepth();

  values->getMinMaxValue(m_minValue, m_maxValue);
}

double ScalarProperty::get(size_t index) const
{
  size_t rowStride = m_numI;
  size_t sliceStride = m_numI * m_numJ;

  size_t k = index / sliceStride;
  size_t j = (index - k * sliceStride) / rowStride;
  size_t i = index - k * sliceStride - j * rowStride;

  double val = m_values->getValue((unsigned int)(i), (unsigned int)(j), (unsigned int)k);
  return val;
}

MiDataSet::DataBinding ScalarProperty::getBinding() const
{
  return m_binding;
}

double ScalarProperty::getMin() const
{
  return m_minValue;
}

double ScalarProperty::getMax() const
{
  return m_maxValue;
}

std::string ScalarProperty::getName() const
{
  return m_name;
}

size_t ScalarProperty::getTimeStamp() const
{
  return m_timestamp;
}

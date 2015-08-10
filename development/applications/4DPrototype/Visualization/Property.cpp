#include "Property.h"

#include "Interface/GridMap.h"

#include <MeshVizInterface/MxTimeStamp.h>

namespace di = DataAccess::Interface;

ScalarProperty::ScalarProperty(const std::string& name, const di::GridMap* values)
: m_values(values)
, m_name(name)
, m_binding(MiDataSet::PER_CELL)
, m_timestamp(MxTimeStamp::getTimeStamp())
{
  //m_numI = values->numI();
  //m_numJ = values->numJ();
  //m_numK = values->getDepth();

  values->getMinMaxValue(m_minValue, m_maxValue);
}

double ScalarProperty::get(size_t i, size_t j, size_t k) const
{
  /*
  size_t rowStride = m_numI;
  size_t sliceStride = m_numI * m_numJ;

  size_t k = index / sliceStride;
  size_t j = (index - k * sliceStride) / rowStride;
  size_t i = index - k * sliceStride - j * rowStride;
  */
  
  return m_values->getValue((unsigned int)(i), (unsigned int)(j), (unsigned int)k);
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

MiMeshIjk::StorageLayout ScalarProperty::getStorageLayout() const
{
  return MiMeshIjk::LAYOUT_IJK;
}

//---------------------------------------------------------------------------------------
// VectorProperty
//---------------------------------------------------------------------------------------
VectorProperty::VectorProperty(const std::string& name, const DataAccess::Interface::GridMap* values[3])
  : m_name(name)
  , m_binding(MiDataSet::PER_CELL)
  , m_timestamp(MxTimeStamp::getTimeStamp())
{
  for (int i = 0; i < 3; ++i)
    m_values[i] = values[i];

  //m_numI = values[0]->numI();
  //m_numJ = values[0]->numJ();
  //m_numK = values[0]->getDepth();
}

MbVec3d VectorProperty::get(size_t i, size_t j, size_t k) const
{
  //size_t rowStride = m_numI;
  //size_t sliceStride = m_numI * m_numJ;

  //unsigned int k = (unsigned int)(index / sliceStride);
  //unsigned int j = (unsigned int)((index - k * sliceStride) / rowStride);
  //unsigned int i = (unsigned int)(index - k * sliceStride - j * rowStride);

  MbVec3d v(
    m_values[0]->getValue((unsigned int)i, (unsigned int)j, (unsigned int)k),
    m_values[1]->getValue((unsigned int)i, (unsigned int)j, (unsigned int)k),
    m_values[2]->getValue((unsigned int)i, (unsigned int)j, (unsigned int)k));

  return v;
}

MiDataSet::DataBinding VectorProperty::getBinding() const
{
  return m_binding;
}

MbVec3d VectorProperty::getMin() const
{
  return MbVec3d();
}

MbVec3d VectorProperty::getMax() const
{
  return MbVec3d();
}

std::string VectorProperty::getName() const
{
  return m_name;
}

size_t VectorProperty::getTimeStamp() const
{
  return m_timestamp;
}

MiMeshIjk::StorageLayout VectorProperty::getStorageLayout() const
{
  return MiMeshIjk::LAYOUT_IJK;
}

//---------------------------------------------------------------------------------------
// VectorProperty
//---------------------------------------------------------------------------------------
FormationIdProperty::FormationIdProperty(const std::vector<double>& ids)
: m_ids(ids)
, m_timeStamp(MxTimeStamp::getTimeStamp())
{
}

double FormationIdProperty::get(size_t i, size_t j, size_t k) const
{
  return m_ids[k];
}

MiDataSet::DataBinding FormationIdProperty::getBinding() const
{
  return MiDataSet::PER_CELL;
}

double FormationIdProperty::getMin() const
{
  return m_ids[0];
}

double FormationIdProperty::getMax() const
{
  return m_ids[m_ids.size() - 1];
}

std::string FormationIdProperty::getName() const
{
  return "FormationIds";
}

size_t FormationIdProperty::getTimeStamp() const
{
  return m_timeStamp;
}

MiMeshIjk::StorageLayout FormationIdProperty::getStorageLayout() const
{
  return MiMeshIjk::LAYOUT_IJK;
}

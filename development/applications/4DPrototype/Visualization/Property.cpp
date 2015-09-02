//
// Copyright (C) 2012-2015 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "Property.h"

#include "Interface/GridMap.h"

#include <MeshVizInterface/MxTimeStamp.h>

namespace di = DataAccess::Interface;

FormationProperty::FormationProperty(const std::string& name, const GridMapCollection& values)
  : m_values(values)
  , m_name(name)
  , m_binding(MiDataSet::PER_CELL)
  , m_timestamp(MxTimeStamp::getTimeStamp())
{
}

double FormationProperty::get(size_t i, size_t j, size_t k) const
{
  return m_values.getValue(i, j, k);
}

MiDataSet::DataBinding FormationProperty::getBinding() const
{
  return m_binding;
}

double FormationProperty::getMin() const
{
  return m_values.minValue();
}

double FormationProperty::getMax() const
{
  return m_values.maxValue();
}

std::string FormationProperty::getName() const
{
  return m_name;
}

size_t FormationProperty::getTimeStamp() const
{
  return m_timestamp;
}

MiMeshIjk::StorageLayout FormationProperty::getStorageLayout() const
{
  return MiMeshIjk::LAYOUT_IJK;
}


//---------------------------------------------------------------------------------------
// Formation2DProperty
//---------------------------------------------------------------------------------------
Formation2DProperty::Formation2DProperty(const std::string& name, const std::vector<const DataAccess::Interface::GridMap*>& values)
  : m_values(values)
  , m_name(name)
  , m_binding(MiDataSet::PER_CELL)
  , m_timestamp(MxTimeStamp::getTimeStamp())
  , m_minVal( std::numeric_limits<double>::max())
  , m_maxVal(-std::numeric_limits<double>::max())
{
  for (auto gridMap : values)
  {
    if (gridMap)
    {
      double minVal = 0.0, maxVal = 0.0;
      gridMap->getMinMaxValue(minVal, maxVal);

      m_minVal = std::min(m_minVal, minVal);
      m_maxVal = std::max(m_maxVal, maxVal);
    }
  }
}

double Formation2DProperty::get(size_t i, size_t j, size_t k) const
{
  const di::GridMap* gridMap = m_values[k];

  return gridMap 
    ? gridMap->getValue((unsigned int)i, (unsigned int)j) 
    : -std::numeric_limits<double>::infinity();
}

MiDataSet::DataBinding Formation2DProperty::getBinding() const
{
  return m_binding;
}

double Formation2DProperty::getMin() const
{
  return m_minVal;
}

double Formation2DProperty::getMax() const
{
  return m_maxVal;
}

std::string Formation2DProperty::getName() const
{
  return m_name;
}

size_t Formation2DProperty::getTimeStamp() const
{
  return m_timestamp;
}

MiMeshIjk::StorageLayout Formation2DProperty::getStorageLayout() const
{
  return MiMeshIjk::LAYOUT_IJK;
}

//---------------------------------------------------------------------------------------
// SurfaceProperty
//---------------------------------------------------------------------------------------
SurfaceProperty::SurfaceProperty(const std::string& name, const DataAccess::Interface::GridMap* values)
  : m_values(values)
  , m_numI(values->numI())
  , m_numJ(values->numJ())
  , m_name(name)
  , m_binding(MiDataSet::PER_CELL)
  , m_timestamp(MxTimeStamp::getTimeStamp())
  , m_minVal(0.0)
  , m_maxVal(0.0)
{
  values->getMinMaxValue(m_minVal, m_maxVal);
}

double SurfaceProperty::get(size_t index) const
{
  unsigned int i = (unsigned int)index % (m_numI - 1);
  unsigned int j = (unsigned int)index / (m_numI - 1);
  return m_values->getValue(i, j);
}

MiDataSet::DataBinding SurfaceProperty::getBinding() const
{
  return m_binding;
}

double SurfaceProperty::getMin() const
{
  return m_minVal;
}

double SurfaceProperty::getMax() const
{
  return m_maxVal;
}

std::string SurfaceProperty::getName() const
{
  return m_name;
}

size_t SurfaceProperty::getTimeStamp() const
{
  return m_timestamp;
}

//---------------------------------------------------------------------------------------
// VectorProperty
//---------------------------------------------------------------------------------------
ReservoirProperty::ReservoirProperty(const std::string& name, const DataAccess::Interface::GridMap* values)
  : m_values(values)
  , m_name(name)
  , m_binding(MiDataSet::PER_CELL)
  , m_timestamp(MxTimeStamp::getTimeStamp())
  , m_minVal(0.0)
  , m_maxVal(0.0)
{
  values->getMinMaxValue(m_minVal, m_maxVal);
}

double ReservoirProperty::get(size_t i, size_t j, size_t k) const
{
  return m_values->getValue(
    (unsigned int)i,
    (unsigned int)j,
    (unsigned int)k);
}

MiDataSet::DataBinding ReservoirProperty::getBinding() const
{
  return m_binding;
}

double ReservoirProperty::getMin() const
{
  return m_minVal;
}

double ReservoirProperty::getMax() const
{
  return m_maxVal;
}

std::string ReservoirProperty::getName() const
{
  return m_name;
}

size_t ReservoirProperty::getTimeStamp() const
{
  return m_timestamp;
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

double FormationIdProperty::get(size_t /*i*/, size_t /*j*/, size_t k) const
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

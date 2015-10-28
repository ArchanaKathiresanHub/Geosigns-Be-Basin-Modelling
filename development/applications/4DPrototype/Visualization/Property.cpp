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
#include "Mesh.h"

#include "Interface/GridMap.h"

#include <MeshVizXLM/MxTimeStamp.h>

namespace di = DataAccess::Interface;

FormationProperty::FormationProperty(const std::string& name, const std::vector<const di::GridMap*>& values)
  : m_values(values, GridMapCollection::SkipLastK)
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

Formation2DProperty::~Formation2DProperty()
{
  for (auto gridMap : m_values)
  {
    if (gridMap)
      gridMap->release();
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

SurfaceProperty::~SurfaceProperty()
{
  m_values->release();
}

double SurfaceProperty::get(size_t i, size_t j) const
{
  return m_values->getValue((unsigned int)i, (unsigned int)j);
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
void ReservoirProperty::updateMinMax() const
{
  m_values->getMinMaxValue(m_minVal, m_maxVal);
  m_minMaxValid = true;
}

ReservoirProperty::ReservoirProperty(const std::string& name, const DataAccess::Interface::GridMap* values, MiDataSet::DataBinding binding)
  : m_values(values)
  , m_binding(binding)
  , m_name(name)
  , m_timestamp(MxTimeStamp::getTimeStamp())
  , m_minMaxValid(false)
  , m_minVal(0.0)
  , m_maxVal(0.0)
{
}

ReservoirProperty::~ReservoirProperty()
{
  m_values->release();
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
  if (!m_minMaxValid)
    updateMinMax();

  return m_minVal;
}

double ReservoirProperty::getMax() const
{
  if (!m_minMaxValid)
    updateMinMax();

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
}

VectorProperty::~VectorProperty()
{
  for (int i = 0; i < 3; ++i)
    m_values[i]->release();
}

MbVec3d VectorProperty::get(size_t i, size_t j, size_t k) const
{
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
, m_minVal(0.0)
, m_maxVal(0.0)
, m_timeStamp(MxTimeStamp::getTimeStamp())
{
  if (!ids.empty())
  {
    m_minVal = ids[0];
    m_maxVal = ids[ids.size() - 1];
  }
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
  return m_minVal;
}

double FormationIdProperty::getMax() const
{
  return m_maxVal;
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

//---------------------------------------------------------------------------------------
// PersistentTrapIdProperty
//---------------------------------------------------------------------------------------
double PersistentTrapIdProperty::translateId(double id) const
{
  int index = (int)id - (int)m_minId;
  return (index < 0 || index >= (int)m_translationTable.size())
    ? 0.0
    : m_translationTable[index];
}

PersistentTrapIdProperty::PersistentTrapIdProperty(
  const DataAccess::Interface::GridMap* trapIds,
  const std::vector<unsigned int>& translationTable,
  unsigned int minId)
  : ReservoirProperty("PersistentTrapId", trapIds)
  , m_translationTable(translationTable)
  , m_minId(minId)
{
}

double PersistentTrapIdProperty::get(size_t i, size_t j, size_t k) const
{
  return translateId(ReservoirProperty::get(i, j, k));
}

double PersistentTrapIdProperty::getMin() const
{
  return translateId(ReservoirProperty::getMin());
}

double PersistentTrapIdProperty::getMax() const
{
  return translateId(ReservoirProperty::getMax());
}

//---------------------------------------------------------------------------------------
// FlowDirectionProperty
//---------------------------------------------------------------------------------------
FlowDirectionProperty::FlowDirectionProperty(
  const std::vector<const DataAccess::Interface::GridMap*>& values,
  const SnapshotTopology& topology)
: m_values(values, GridMapCollection::SkipFirstK)
, m_topology(topology)
, m_binding(MiDataSet::PER_CELL)
, m_numI(topology.getNumCellsI())
, m_numJ(topology.getNumCellsJ())
, m_numK(topology.getNumCellsK())
, m_timestamp(MxTimeStamp::getTimeStamp())
{
}

MbVec3<int32_t> FlowDirectionProperty::getDeltas(size_t i, size_t j, size_t k) const
{
  int code = (int)m_values.getValue(i, j, k);

  if (code == 0 || code == (int)di::DefaultUndefinedMapValue)
    return MbVec3<int32_t>();

  code += 111;
  int dk = code / 100 - 1;
  int dj = (code % 100) / 10 - 1;
  int di = (code % 10) - 1;

  if (
    dk <  0 || dk > 1 ||
    dj < -1 || dj > 1 ||
    di < -1 || di > 1)
  {
    return MbVec3<int32_t>();
  }

  return MbVec3<int32_t>(di, dj, dk);
}

MbVec3d FlowDirectionProperty::get(size_t i0, size_t j0, size_t k0) const
{
  MbVec3<int32_t> deltas = getDeltas(i0, j0, k0);
  if (
    deltas[0] == 0 &&
    deltas[1] == 0 &&
    deltas[2] == 0)
  {
    return MbVec3d();
  }

  int i1 = (int)i0 + deltas[0];
  int j1 = (int)j0 + deltas[1];
  int k1 = (int)k0 - deltas[2];

  if (
    i1 < 0 || i1 >= m_numI ||
    j1 < 0 || j1 >= m_numJ ||
    k1 < 0 || k1 >= m_numK ||
    m_topology.isDead((size_t)i1, (size_t)j1, (size_t)k1))
  {
    return MbVec3d();
  }

  MbVec3d p0 = m_topology.getCellCenter(i0, j0, k0);
  MbVec3d p1 = m_topology.getCellCenter((size_t)i1, (size_t)j1, (size_t)k1);

  return p1 - p0;
}

MiDataSet::DataBinding FlowDirectionProperty::getBinding() const
{
  return m_binding;
}

std::string FlowDirectionProperty::getName() const
{
  return "FlowDirectionIJK";
}

size_t FlowDirectionProperty::getTimeStamp() const
{
  return m_timestamp;
}

MiMeshIjk::StorageLayout FlowDirectionProperty::getStorageLayout() const
{
  return MiMeshIjk::LAYOUT_IJK;
}

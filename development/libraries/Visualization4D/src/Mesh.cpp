//
// Copyright (C) 2012-2015 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "Mesh.h"

// DataAccess stuff
#include "Interface/GridMap.h"

// meshviz interface
#include <MeshVizXLM/MxTimeStamp.h>

namespace di = DataAccess::Interface;

//--------------------------------------------------------------------------------------------------
// ReservoirGeometry
//--------------------------------------------------------------------------------------------------
ReservoirGeometry::ReservoirGeometry(
  const DataAccess::Interface::GridMap* depthMapTop,
  const DataAccess::Interface::GridMap* depthMapBottom)
  : m_timeStamp(MxTimeStamp::getTimeStamp())
{
  m_depthMaps[0] = depthMapTop;
  m_depthMaps[1] = depthMapBottom;

  m_values[0] = depthMapTop->getValues()[0][0];
  m_values[1] = depthMapBottom->getValues()[0][0];

  m_numI = depthMapTop->numI();
  m_numJ = depthMapTop->numJ();

  m_deltaX = depthMapTop->deltaI();
  m_deltaY = depthMapTop->deltaJ();
}

ReservoirGeometry::~ReservoirGeometry()
{
  m_depthMaps[0]->release();
  m_depthMaps[1]->release();
}

size_t ReservoirGeometry::numI() const
{
  return m_numI;
}

size_t ReservoirGeometry::numJ() const
{
  return m_numJ;
}

size_t ReservoirGeometry::numK() const
{
  return 2;
}

bool ReservoirGeometry::isUndefined(size_t i, size_t j, size_t k) const
{
  return m_values[0][i * m_numJ + j] == di::DefaultUndefinedMapValue;
}

MbVec3d ReservoirGeometry::getCoord(size_t i, size_t j, size_t k) const
{
  return MbVec3d(
    i * m_deltaX,
    j * m_deltaY,
    -m_values[k][i * m_numJ + j]);
}

void ReservoirGeometry::getCellCoords(
  size_t i, size_t j, size_t k,
  MbVec3d& v0, MbVec3d& v1, MbVec3d& v2, MbVec3d& v3,
  MbVec3d& v4, MbVec3d& v5, MbVec3d& v6, MbVec3d& v7) const
{
  double x0 = i * m_deltaX;
  double y0 = j * m_deltaY;
  double x1 = (i + 1) * m_deltaX;
  double y1 = (j + 1) * m_deltaY;

  const double* p0 = m_values[k];
  const double* p1 = m_values[k + 1];

  v0.setValue(x0, y0, -p0[i * m_numJ + j]);
  v1.setValue(x1, y0, -p0[(i + 1) * m_numJ + j]);
  v2.setValue(x1, y1, -p0[(i + 1) * m_numJ + (j + 1)]);
  v3.setValue(x0, y1, -p0[i * m_numJ + (j + 1)]);

  v4.setValue(x0, y0, -p1[i * m_numJ + j]);
  v5.setValue(x1, y0, -p1[(i + 1) * m_numJ + j]);
  v6.setValue(x1, y1, -p1[(i + 1) * m_numJ + (j + 1)]);
  v7.setValue(x0, y1, -p1[i * m_numJ + (j + 1)]);
}

MiMeshIjk::StorageLayout ReservoirGeometry::getStorageLayout() const
{
  return MiMeshIjk::LAYOUT_KIJ;
}

size_t ReservoirGeometry::getTimeStamp() const
{
  return m_timeStamp;
}

//--------------------------------------------------------------------------------------------------
// SnapshotGeometry
//--------------------------------------------------------------------------------------------------
SnapshotGeometry::SnapshotGeometry(const std::vector<const DataAccess::Interface::GridMap*>& depthMaps)
: m_depthMaps(depthMaps, GridMapCollection::EliminateBoundaries)
, m_timeStamp(MxTimeStamp::getTimeStamp())
{
  assert(!depthMaps.empty());

  const di::GridMap* depthMapTop = depthMaps[0];

  m_deltaX = depthMapTop->deltaI();
  m_deltaY = depthMapTop->deltaJ();
}

size_t SnapshotGeometry::numI() const
{
  return m_depthMaps.numI();
}

size_t SnapshotGeometry::numJ() const
{
  return m_depthMaps.numJ();
}

size_t SnapshotGeometry::numK() const
{
  return m_depthMaps.numK();
}

double SnapshotGeometry::deltaX() const
{
  return m_deltaX;
}

double SnapshotGeometry::deltaY() const
{
  return m_deltaY;
}

bool SnapshotGeometry::isUndefined(size_t i, size_t j, size_t k) const
{
  return m_depthMaps.getValue(i, j, k) == di::DefaultUndefinedMapValue;
}

double SnapshotGeometry::getDepth(size_t i, size_t j, size_t k) const 
{ 
  return m_depthMaps.getValue(i, j, k); 
}

MbVec3d SnapshotGeometry::getCoord(size_t i, size_t j, size_t k) const
{
  return MbVec3d(
    i * m_deltaX,
    j * m_deltaY,
    -m_depthMaps.getValue(i, j, k));
}

void SnapshotGeometry::getCellCoords(
  size_t i, size_t j, size_t k,
  MbVec3d& v0, MbVec3d& v1, MbVec3d& v2, MbVec3d& v3,
  MbVec3d& v4, MbVec3d& v5, MbVec3d& v6, MbVec3d& v7) const
{
  double x0 = i * m_deltaX;
  double y0 = j * m_deltaY;
  double x1 = (i + 1) * m_deltaX;
  double y1 = (j + 1) * m_deltaY;

  size_t k0 = k;
  size_t k1 = k+1;

  v0.setValue(x0, y0, -m_depthMaps.getValue(i, j, k0));
  v1.setValue(x1, y0, -m_depthMaps.getValue(i + 1, j, k0));
  v2.setValue(x1, y1, -m_depthMaps.getValue(i + 1, j + 1, k0));
  v3.setValue(x0, y1, -m_depthMaps.getValue(i, j + 1, k0));

  v4.setValue(x0, y0, -m_depthMaps.getValue(i, j, k1));
  v5.setValue(x1, y0, -m_depthMaps.getValue(i + 1, j, k1));
  v6.setValue(x1, y1, -m_depthMaps.getValue(i + 1, j + 1, k1));
  v7.setValue(x0, y1, -m_depthMaps.getValue(i, j + 1, k1));
}

size_t SnapshotGeometry::getTimeStamp() const
{
  return m_timeStamp;
}

//--------------------------------------------------------------------------------------------------
// SnapshotTopology
//--------------------------------------------------------------------------------------------------

SnapshotTopology::SnapshotTopology(std::shared_ptr<SnapshotGeometry> geometry, const bool* deadMap)
  : m_numI(geometry->numI() - 1)
  , m_numJ(geometry->numJ() - 1)
  , m_numK(geometry->numK() - 1)
  , m_deadMap(deadMap)
  , m_timeStamp(MxTimeStamp::getTimeStamp())
  , m_geometry(geometry)
{
}

SnapshotTopology::~SnapshotTopology()
{
}

MbVec3d SnapshotTopology::getCellCenter(size_t i, size_t j, size_t k) const
{
  return .125 * (
    m_geometry->getCoord(i, j, k) +
    m_geometry->getCoord(i + 1, j, k) +
    m_geometry->getCoord(i, j + 1, k) +
    m_geometry->getCoord(i + 1, j + 1, k) +
    m_geometry->getCoord(i, j, k + 1) +
    m_geometry->getCoord(i + 1, j, k + 1) +
    m_geometry->getCoord(i, j + 1, k + 1) +
    m_geometry->getCoord(i + 1, j + 1, k + 1));
}

size_t SnapshotTopology::getNumCellsI() const
{
  return m_numI;
}

size_t SnapshotTopology::getNumCellsJ() const
{
  return m_numJ;
}

size_t SnapshotTopology::getNumCellsK() const
{
  return m_numK;
}

size_t SnapshotTopology::getTimeStamp() const
{
  return m_timeStamp;
}

bool SnapshotTopology::hasDeadCells() const
{
  return true; //probably
}

bool SnapshotTopology::isDead(size_t i, size_t j, size_t /*k*/) const
{
  return m_deadMap[i * m_numJ + j];
}

//--------------------------------------------------------------------------------------------------
// ReservoirTopology
//--------------------------------------------------------------------------------------------------
ReservoirTopology::ReservoirTopology(std::shared_ptr<ReservoirGeometry> geometry, const bool* deadMap)
  : m_geometry(geometry)
  , m_timestamp(MxTimeStamp::getTimeStamp())
  , m_deadMap(deadMap)
{
  m_numI = m_geometry->numI() - 1;
  m_numJ = m_geometry->numJ() - 1;
}

size_t ReservoirTopology::getNumCellsI() const
{
  return m_numI;
}

size_t ReservoirTopology::getNumCellsJ() const
{
  return m_numJ;
}

size_t ReservoirTopology::getNumCellsK() const
{
  return 1;
}

bool ReservoirTopology::isDead(size_t i, size_t j, size_t k) const
{
  return m_deadMap[i * m_numJ + j];
}

bool ReservoirTopology::hasDeadCells() const
{
  return true;
}

size_t ReservoirTopology::getTimeStamp() const
{
  return m_timestamp;
}

//--------------------------------------------------------------------------------------------------
// SurfaceGeometry
//--------------------------------------------------------------------------------------------------
SurfaceGeometry::SurfaceGeometry(const DataAccess::Interface::GridMap* values)
: m_values(values)
, m_timestamp(MxTimeStamp::getTimeStamp())
, m_deltaX(values->deltaI())
, m_deltaY(values->deltaJ())
{
}

SurfaceGeometry::~SurfaceGeometry()
{
  m_values->release();
}

MbVec3d SurfaceGeometry::getCoord(size_t i, size_t j) const
{
  return MbVec3d(
    i * m_deltaX,
    j * m_deltaY,
    -m_values->getValue((unsigned int)i, (unsigned int)j));
}

size_t SurfaceGeometry::getTimeStamp() const
{
  return m_timestamp;
}

bool SurfaceGeometry::isUndefined(size_t i, size_t j) const
{
  return m_values->getValue((unsigned int)i, (unsigned int)j) == di::DefaultUndefinedMapValue;
}

//--------------------------------------------------------------------------------------------------
// SurfaceTopology
//--------------------------------------------------------------------------------------------------
SurfaceTopology::SurfaceTopology(size_t numI, size_t numJ, const SurfaceGeometry& geometry)
  : m_numI(numI)
  , m_numJ(numJ)
  , m_timeStamp(MxTimeStamp::getTimeStamp())
  , m_geometry(geometry)
{
}

size_t SurfaceTopology::getNumCellsI() const
{
  return m_numI;
}

size_t SurfaceTopology::getNumCellsJ() const
{
  return m_numJ;
}

bool SurfaceTopology::isDead(size_t i, size_t j) const
{
  return
    m_geometry.isUndefined(i, j) ||
    m_geometry.isUndefined(i + 1, j) ||
    m_geometry.isUndefined(i + 1, j + 1) ||
    m_geometry.isUndefined(i, j + 1);
}

size_t SurfaceTopology::getTimeStamp() const
{
  return m_timeStamp;
}

bool SurfaceTopology::hasDeadCells() const
{
  return true;
}

//--------------------------------------------------------------------------------------------------
// FaultGeometry
//--------------------------------------------------------------------------------------------------
FaultGeometry::FaultGeometry(const std::vector<MbVec3d> coords)
  : m_coords(coords)
  , m_timeStamp(MxTimeStamp::getTimeStamp())
{

}

MbVec3d FaultGeometry::getCoord(size_t index) const
{
  return m_coords[index];
}

size_t FaultGeometry::getTimeStamp() const
{
  return m_timeStamp;
}

//--------------------------------------------------------------------------------------------------
// FaultTopology
//--------------------------------------------------------------------------------------------------
FaultTopology::FaultTopology(size_t numCells)
{
  for (size_t i = 0; i < numCells; ++i)
    m_cells.emplace_back(2 * i, 2 * i + 1, 2 * i + 3, 2 * i + 2);
}

const MiSurfaceCell* FaultTopology::getCell(size_t id) const
{
  return &m_cells[id];
}

size_t FaultTopology::getBeginNodeId() const
{
  return 0;
}

size_t FaultTopology::getEndNodeId() const
{
  return 2 * (m_cells.size() + 1);
}

size_t FaultTopology::getNumCells() const
{
  return m_cells.size();
}

bool FaultTopology::isDead(size_t /*i*/) const
{
  return false;
}

size_t FaultTopology::getTimeStamp() const
{
  return m_timeStamp;
}

bool FaultTopology::hasDeadCells() const
{
  return false;
}

//--------------------------------------------------------------------------------------------------
// FaultMesh
//--------------------------------------------------------------------------------------------------
FaultMesh::FaultMesh(
  std::shared_ptr<FaultGeometry> geometry,
  std::shared_ptr<FaultTopology> topology)
  : m_geometry(geometry)
  , m_topology(topology)
{

}

const MiSurfaceTopologyExplicitI& FaultMesh::getTopology() const
{
  return *m_topology;
}

const MiGeometryI& FaultMesh::getGeometry() const
{
  return *m_geometry;
}

#include "Mesh.h"

// DataAccess stuff
#include "Interface/ProjectHandle.h"
#include "Interface/GridMap.h"
#include "Interface/Property.h"
#include "Interface/PropertyValue.h"
#include "Interface/Snapshot.h"

// meshviz interface
#include <MeshVizInterface/MxTimeStamp.h>
#include <MeshVizInterface/mesh/data/MiDataSetIjk.h>
#include <MeshVizInterface/mesh/topology/MiHexahedronTopologyExplicitIjk.h>

namespace di = DataAccess::Interface;

//--------------------------------------------------------------------------------------------------
// Geometry
//--------------------------------------------------------------------------------------------------
Geometry::Geometry(const di::GridMap* depthMap)
  : m_depthMap(depthMap)
  , m_timeStamp(MxTimeStamp::getTimeStamp())
{
  m_numI = depthMap->numI();
  m_numJ = depthMap->numJ();
  m_numK = depthMap->getDepth();

  m_minX = depthMap->minI();
  m_minY = depthMap->minJ();

  m_deltaX = depthMap->deltaI();
  m_deltaY = depthMap->deltaJ();

  double minDepth, maxDepth;
  depthMap->getMinMaxValue(minDepth, maxDepth);

  m_minZ = -maxDepth;
  m_maxZ = -minDepth;
}

bool Geometry::isUndefined(size_t i, size_t j, size_t k) const
{
  return m_depthMap->getValue((unsigned int)i, (unsigned int)j, (unsigned int)k) == di::DefaultUndefinedMapValue;
}

MbVec3d Geometry::getCoord(unsigned int i, unsigned int j, unsigned int k) const
{
  return MbVec3d(
    m_minX + i * m_deltaX,
    m_minY + j * m_deltaY,
    -m_depthMap->getValue(i, j, k));
}

MbVec3d Geometry::getCoord(size_t index) const
{
  size_t rowStride = m_numI;
  size_t sliceStride = rowStride * m_numJ;

  unsigned int k = (unsigned int)(index / sliceStride);
  unsigned int j = (unsigned int)((index - k * sliceStride) / rowStride);
  unsigned int i = (unsigned int)(index - k * sliceStride - j * rowStride);

  return getCoord(i, j, k);
}

MbVec3d Geometry::getMin() const
{
  return MbVec3d(m_minX, m_minY, m_minZ);
}

MbVec3d Geometry::getMax() const
{
  return MbVec3d(
    m_minX + (m_numI - 1) * m_deltaX,
    m_minY + (m_numJ - 1) * m_deltaY,
    m_maxZ);
}

size_t Geometry::getTimeStamp() const
{
  return m_timeStamp;
}

//--------------------------------------------------------------------------------------------------
// Geometry2
//--------------------------------------------------------------------------------------------------
Geometry2::Geometry2(
  const DataAccess::Interface::GridMap* depthMapTop,
  const DataAccess::Interface::GridMap* depthMapBottom)
  : m_timeStamp(MxTimeStamp::getTimeStamp())
{
  m_depthMaps[0] = depthMapTop;
  m_depthMaps[1] = depthMapBottom;

  m_numI = depthMapTop->numI();
  m_numJ = depthMapTop->numJ();

  m_minX = depthMapTop->minI();
  m_minY = depthMapTop->minJ();

  m_deltaX = depthMapTop->deltaI();
  m_deltaY = depthMapTop->deltaJ();

  double minDepth, maxDepth;
  
  depthMapTop->getMinMaxValue(minDepth, maxDepth);
  m_maxZ = -minDepth;

  depthMapBottom->getMinMaxValue(minDepth, maxDepth);
  m_minZ = -maxDepth;
}

bool Geometry2::isUndefined(size_t i, size_t j, size_t k) const
{
  return m_depthMaps[k]->getValue((unsigned int)i, (unsigned int)j, 0u) == di::DefaultUndefinedMapValue;
}

MbVec3d Geometry2::getCoord(unsigned int i, unsigned int j, unsigned int k) const
{
  return MbVec3d(
    m_minX + i * m_deltaX,
    m_minY + j * m_deltaY,
    -m_depthMaps[k]->getValue(i, j, 0u));
}

MbVec3d Geometry2::getCoord(size_t index) const
{
  size_t rowStride = m_numI;
  size_t sliceStride = rowStride * m_numJ;

  unsigned int k = (unsigned int)(index / sliceStride);
  unsigned int j = (unsigned int)((index - k * sliceStride) / rowStride);
  unsigned int i = (unsigned int)(index - k * sliceStride - j * rowStride);

  return getCoord(i, j, k);
}

MbVec3d Geometry2::getMin() const
{
  return MbVec3d(m_minX, m_minY, m_minZ);
}

MbVec3d Geometry2::getMax() const
{
  return MbVec3d(
    m_minX + (m_numI - 1) * m_deltaX,
    m_minY + (m_numJ - 1) * m_deltaY,
    m_maxZ);
}

size_t Geometry2::getTimeStamp() const
{
  return m_timeStamp;
}

//--------------------------------------------------------------------------------------------------
// VolumeTopology
//--------------------------------------------------------------------------------------------------

VolumeTopology::VolumeTopology(size_t numI, size_t numJ, size_t numK)
  : m_numI(numI)
  , m_numJ(numJ)
  , m_numK(numK)
  , m_timeStamp(MxTimeStamp::getTimeStamp())
{
}

/*
From the docs for MiHexahedronVolumeTopologyExplicitIjk:

          J
          |
          |
         n3----------n2   facet 0 = 0123
         /|          /|   facet 1 = 4765
       /  |        /  |   facet 2 = 0374
     /    |      /    |   facet 3 = 1562
   n7---------n6      |   facet 4 = 0451
    |     |    |      |   facet 5 = 3267
    |    n0----|-----n1    --- I
    |    /     |     /
    |  /       |   /
    |/         | /
   n4---------n5
  /
 /
K

*/
void VolumeTopology::getCellNodeIndices(
  size_t i, size_t j, size_t k,
  size_t& n0, size_t& n1, size_t& n2, size_t& n3,
  size_t& n4, size_t& n5, size_t& n6, size_t& n7) const
{
  size_t rowStride = m_numI + 1;
  size_t sliceStride = rowStride * (m_numJ + 1);

  n0 = k * sliceStride + j * rowStride + i;
  n1 = n0 + 1;
  n2 = n1 + rowStride;
  n3 = n0 + rowStride;

  n4 = n0 + sliceStride;
  n5 = n1 + sliceStride;
  n6 = n2 + sliceStride;
  n7 = n3 + sliceStride;
}

MiMeshIjk::StorageLayout VolumeTopology::getStorageLayout() const
{
  // This means that K changes fastest, then J, then I
  return MiMeshIjk::LAYOUT_IJK;
}

size_t VolumeTopology::getBeginNodeId() const
{
  return 0;
}

size_t VolumeTopology::getEndNodeId() const
{
  return (m_numI + 1) * (m_numJ + 1) * (m_numK + 1);
}

std::string VolumeTopology::getNodeName(size_t i) const
{
  return "";
}

size_t VolumeTopology::getNumCellsI() const
{
  return m_numI;
}

size_t VolumeTopology::getNumCellsJ() const
{
  return m_numJ;
}

size_t VolumeTopology::getNumCellsK() const
{
  return m_numK;
}

size_t VolumeTopology::getTimeStamp() const
{
  return m_timeStamp;
}

bool VolumeTopology::hasDeadCells() const
{
  return true;
}

//--------------------------------------------------------------------------------------------------
// FormationTopology
//--------------------------------------------------------------------------------------------------
FormationTopology::FormationTopology(size_t numI, size_t numJ, size_t numK, const Geometry& geometry)
  : VolumeTopology(numI, numJ, numK)
  , m_geometry(geometry)
{
}
  

bool FormationTopology::isDead(size_t i, size_t j, size_t k) const
{
  return
    m_geometry.isUndefined(i, j, k) ||
    m_geometry.isUndefined(i, j + 1, k) ||
    m_geometry.isUndefined(i + 1, j, k) ||
    m_geometry.isUndefined(i + 1, j + 1, k) ||
    m_geometry.isUndefined(i, j, k + 1) ||
    m_geometry.isUndefined(i, j + 1, k + 1) ||
    m_geometry.isUndefined(i + 1, j, k + 1) ||
    m_geometry.isUndefined(i + 1, j + 1, k + 1);
}

//--------------------------------------------------------------------------------------------------
// ReservoirTopology
//--------------------------------------------------------------------------------------------------
ReservoirTopology::ReservoirTopology(size_t numI, size_t numJ, const Geometry2& geometry)
  : VolumeTopology(numI, numJ, 1)
  , m_geometry(geometry)
{
}

bool ReservoirTopology::isDead(size_t i, size_t j, size_t k) const
{
  return
    m_geometry.isUndefined(i, j, k) ||
    m_geometry.isUndefined(i, j + 1, k) ||
    m_geometry.isUndefined(i + 1, j, k) ||
    m_geometry.isUndefined(i + 1, j + 1, k) ||
    m_geometry.isUndefined(i, j, k + 1) ||
    m_geometry.isUndefined(i, j + 1, k + 1) ||
    m_geometry.isUndefined(i + 1, j, k + 1) ||
    m_geometry.isUndefined(i + 1, j + 1, k + 1);
}

//--------------------------------------------------------------------------------------------------
// SurfaceTopology
//--------------------------------------------------------------------------------------------------
SurfaceTopology::SurfaceTopology(size_t numI, size_t numJ, Geometry& geometry)
  : m_numI(numI)
  , m_numJ(numJ)
  , m_timeStamp(MxTimeStamp::getTimeStamp())
  , m_geometry(geometry)
{
  m_cells.reserve(numI * numJ);

  for (size_t j = 0; j < numJ; ++j)
  {
    for (size_t i = 0; i < numI; ++i)
    {
      size_t base = j * (numI + 1) + i;
      m_cells.emplace_back(
        base, 
        base + 1, 
        base + numI + 2, 
        base + numI + 1);
    }
  }
}

const MiSurfaceCell* SurfaceTopology::getCell(size_t id) const
{
  return &m_cells[id];
}

size_t SurfaceTopology::getBeginNodeId() const
{
  return 0;
}

size_t SurfaceTopology::getEndNodeId() const
{
  return (m_numI + 1) * (m_numJ + 1);
}

size_t SurfaceTopology::getNumCells() const
{
  return m_cells.size();
}

bool SurfaceTopology::isDead(size_t id) const
{
  size_t i = id % m_numI;
  size_t j = id / m_numI;

  return
    m_geometry.isUndefined(i, j, 0) ||
    m_geometry.isUndefined(i + 1, j, 0) ||
    m_geometry.isUndefined(i + 1, j + 1, 0) ||
    m_geometry.isUndefined(i, j + 1, 0);
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
// FormationMesh
//--------------------------------------------------------------------------------------------------
FormationMesh::FormationMesh(const DataAccess::Interface::GridMap* depthMap)
{
  m_geometry = std::make_shared<Geometry>(depthMap);

  size_t ni = depthMap->numI() - 1;
  size_t nj = depthMap->numJ() - 1;
  size_t nk = depthMap->getDepth() - 1;
  m_topology = std::make_shared<FormationTopology>(ni, nj, nk, *m_geometry);
}

const MiHexahedronTopologyExplicitIjk& FormationMesh::getTopology() const
{
  return *m_topology;
}

const MiGeometryI& FormationMesh::getGeometry() const
{
  return *m_geometry;
}

//--------------------------------------------------------------------------------------------------
// ReservoirMesh
//--------------------------------------------------------------------------------------------------
ReservoirMesh::ReservoirMesh(
  const DataAccess::Interface::GridMap* depthMapTop,
  const DataAccess::Interface::GridMap* depthMapBottom)
{
  m_geometry = std::make_shared<Geometry2>(depthMapTop, depthMapBottom);

  size_t ni = depthMapTop->numI() - 1;
  size_t nj = depthMapTop->numJ() - 1;
  size_t nk = 2;
  m_topology = std::make_shared<ReservoirTopology>(ni, nj, *m_geometry);
}

const MiHexahedronTopologyExplicitIjk& ReservoirMesh::getTopology() const
{
  return *m_topology;
}

const MiGeometryI& ReservoirMesh::getGeometry() const
{
  return *m_geometry;
}

//--------------------------------------------------------------------------------------------------
// SurfaceMesh
//--------------------------------------------------------------------------------------------------
SurfaceMesh::SurfaceMesh(const DataAccess::Interface::GridMap* depthMap)
{
  m_geometry = std::make_shared<Geometry>(depthMap);

  size_t ni = depthMap->numI() - 1;
  size_t nj = depthMap->numJ() - 1;
  m_topology = std::make_shared<SurfaceTopology>(ni, nj, *m_geometry);
}

const MiSurfaceTopologyExplicitI& SurfaceMesh::getTopology() const
{
  return *m_topology;
}

const MiGeometryI& SurfaceMesh::getGeometry() const
{
  return *m_geometry;
}

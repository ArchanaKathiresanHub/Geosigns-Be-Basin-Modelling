#ifndef MESH_H_INCLUDED
#define MESH_H_INCLUDED

#include "Interface/Interface.h"

#include <MeshVizInterface/mesh/MiVolumeMeshHexahedronIjk.h>
#include <MeshVizInterface/mesh/MiSurfaceMeshUnstructured.h>
#include <MeshVizInterface/mesh/topology/MiSurfaceTopologyExplicitI.h>
#include <MeshVizInterface/mesh/cell/MiSurfaceCell.h>

/**
* Stores the node coordinates for a mesh
*/
class Geometry : public MiGeometryI
{
  const DataAccess::Interface::GridMap* m_depthMap;

  size_t m_numI;
  size_t m_numJ;
  size_t m_numK;

  double m_minX;
  double m_minY;
  double m_minZ;
  double m_maxZ;

  double m_deltaX;
  double m_deltaY;

  size_t m_timeStamp;

public:

  explicit Geometry(const DataAccess::Interface::GridMap* depthMap);

  bool isUndefined(size_t i, size_t j, size_t k) const;

  MbVec3d getCoord(unsigned int i, unsigned int j, unsigned int k) const;

  virtual MbVec3d getCoord(size_t index) const;

  virtual MbVec3d getMin() const;

  virtual MbVec3d getMax() const;

  virtual size_t getTimeStamp() const;
};

/**
* Geometry class for reservoirs, which are defined by the top and bottom properties
*/
class Geometry2 : public MiGeometryI
{
  const DataAccess::Interface::GridMap* m_depthMaps[2];

  size_t m_numI;
  size_t m_numJ;

  double m_minX;
  double m_minY;
  double m_minZ;
  double m_maxZ;

  double m_deltaX;
  double m_deltaY;

  size_t m_timeStamp;

public:

  Geometry2(
    const DataAccess::Interface::GridMap* depthMapTop, 
    const DataAccess::Interface::GridMap* depthMapBottom);

  bool isUndefined(size_t i, size_t j, size_t k) const;

  MbVec3d getCoord(unsigned int i, unsigned int j, unsigned int k) const;

  virtual MbVec3d getCoord(size_t index) const;

  virtual MbVec3d getMin() const;

  virtual MbVec3d getMax() const;

  virtual size_t getTimeStamp() const;
};

/**
* Defines the cell topology for a volume mesh
*/
class VolumeTopology : public MiHexahedronTopologyExplicitIjk
{
  size_t m_numI;
  size_t m_numJ;
  size_t m_numK;

  size_t m_timeStamp;

public:

  VolumeTopology(size_t numI, size_t numJ, size_t numK);

  virtual void getCellNodeIndices(
    size_t i, size_t j, size_t k,
    size_t& n0, size_t& n1, size_t& n2, size_t& n3,
    size_t& n4, size_t& n5, size_t& n6, size_t& n7) const;

  virtual MiMeshIjk::StorageLayout getStorageLayout() const;

  virtual size_t getBeginNodeId() const;

  virtual size_t getEndNodeId() const;

  virtual std::string getNodeName(size_t i) const;

  virtual size_t getNumCellsI() const;

  virtual size_t getNumCellsJ() const;

  virtual size_t getNumCellsK() const;

  virtual size_t getTimeStamp() const;

  virtual bool hasDeadCells() const;
};

class FormationTopology : public VolumeTopology
{
  const Geometry& m_geometry;

public:

  FormationTopology(size_t numI, size_t numJ, size_t numK, const Geometry& geometry);

  virtual bool isDead(size_t i, size_t j, size_t k) const;
};

class ReservoirTopology : public VolumeTopology
{
  const Geometry2& m_geometry;

public:

  ReservoirTopology(size_t numI, size_t numJ, const Geometry2& geometry);

  virtual bool isDead(size_t i, size_t j, size_t k) const;
};

class QuadCell : public MiSurfaceCell
{
  size_t m_indices[4];

public:

  explicit QuadCell(size_t i0, size_t i1, size_t i2, size_t i3)
  {
    m_indices[0] = i0;
    m_indices[1] = i1;
    m_indices[2] = i2;
    m_indices[3] = i3;
  }

  virtual size_t getNodeIndex(size_t node) const
  {
    return m_indices[node];
  }

  virtual size_t getNumNodes() const
  {
    return 4;
  }
};

/**
* Defines the cell topology for a surface mesh
*/
class SurfaceTopology : public MiSurfaceTopologyExplicitI
{
  size_t m_numI;
  size_t m_numJ;

  size_t m_timeStamp;

  Geometry& m_geometry;

  std::vector<QuadCell> m_cells;

public:

  SurfaceTopology(size_t numI, size_t numJ, Geometry& geometry);

  virtual const MiSurfaceCell* getCell(size_t id) const;

  virtual size_t getBeginNodeId() const;

  virtual size_t getEndNodeId() const;

  virtual size_t getNumCells() const;

  virtual bool isDead(size_t i) const;

  virtual size_t getTimeStamp() const;

  virtual bool hasDeadCells() const;
};

/**
* Represents the mesh for a single formation
*/
class FormationMesh : public MiVolumeMeshHexahedronIjk
{
  std::shared_ptr<Geometry> m_geometry;
  std::shared_ptr<FormationTopology> m_topology;

public:

  explicit FormationMesh(const DataAccess::Interface::GridMap* depthMap);

  const MiHexahedronTopologyExplicitIjk& getTopology() const;

  const MiGeometryI& getGeometry() const;
};

/**
* Represents the mesh for a single reservoir
*/
class ReservoirMesh: public MiVolumeMeshHexahedronIjk
{
  std::shared_ptr<Geometry2> m_geometry;
  std::shared_ptr<ReservoirTopology> m_topology;

public:

  explicit ReservoirMesh(
    const DataAccess::Interface::GridMap* depthMapTop,
    const DataAccess::Interface::GridMap* depthMapBottom);

  const MiHexahedronTopologyExplicitIjk& getTopology() const;

  const MiGeometryI& getGeometry() const;
};

/**
*
*/
class SurfaceMesh : public MiSurfaceMeshUnstructured
{
  std::shared_ptr<Geometry> m_geometry;
  std::shared_ptr<SurfaceTopology> m_topology;

public:

  explicit SurfaceMesh(const DataAccess::Interface::GridMap* depthMap);

  virtual const MiSurfaceTopologyExplicitI& getTopology() const;

  virtual const MiGeometryI& getGeometry() const;
};

#endif

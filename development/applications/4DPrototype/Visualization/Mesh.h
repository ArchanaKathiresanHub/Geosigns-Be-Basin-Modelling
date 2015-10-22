//
// Copyright (C) 2012-2015 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef MESH_H_INCLUDED
#define MESH_H_INCLUDED

#include "GridMapCollection.h"

#include "Interface/Interface.h"
#include "Interface/GridMap.h"

#include <MeshVizXLM/mesh/MiVolumeMeshHexahedronIjk.h>
#include <MeshVizXLM/mesh/MiSurfaceMeshUnstructured.h>
#include <MeshVizXLM/mesh/topology/MiSurfaceTopologyExplicitI.h>
#include <MeshVizXLM/mesh/cell/MiSurfaceCell.h>

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
class ReservoirGeometry : public MiGeometryI
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

  ReservoirGeometry(
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
 * Contains the geometry for an entire snapshot
 */
class SnapshotGeometry : public MiGeometryI
{
  GridMapCollection m_depthMaps;

  double m_minX;
  double m_minY;

  double m_deltaX;
  double m_deltaY;

  size_t m_timeStamp;

public:

  explicit SnapshotGeometry(const std::vector<const DataAccess::Interface::GridMap*>& depthMaps);

  virtual ~SnapshotGeometry();

  size_t numI() const;

  size_t numJ() const;

  size_t numK() const;

  double deltaX() const;

  double deltaY() const;

  bool isUndefined(size_t i, size_t j, size_t k) const;

  double getDepth(unsigned int i, unsigned int j, unsigned int k) const;

  MbVec3d getCoord(unsigned int i, unsigned int j, unsigned int k) const;

  virtual MbVec3d getCoord(size_t index) const;

  virtual MbVec3d getMin() const;

  virtual MbVec3d getMax() const;

  virtual size_t getTimeStamp() const;
};

/**
* Defines the cell topology for a 'chunk' of a snapshot
*/
class SnapshotTopology : public MiHexahedronTopologyExplicitIjk
{
  size_t m_numI; // num cells in I direction
  size_t m_numJ; // num cells in J direction
  size_t m_numK; // num cells in K direction

  bool* m_deadMap;

  size_t m_timeStamp;

  std::shared_ptr<SnapshotGeometry> m_geometry;

  void initDeadMap();

public:

  explicit SnapshotTopology(std::shared_ptr<SnapshotGeometry> geometry);

  ~SnapshotTopology();

  SnapshotTopology(const SnapshotTopology&) = delete;
  SnapshotTopology& operator=(const SnapshotTopology&) = delete;

  MbVec3d getCellCenter(size_t i, size_t j, size_t k) const;

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

  virtual bool isDead(size_t i, size_t j, size_t k) const;
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
  const ReservoirGeometry& m_geometry;

public:

  ReservoirTopology(size_t numI, size_t numJ, const ReservoirGeometry& geometry);

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
* Represents the mesh for a single formation
*/
class HexahedronMesh : public MiVolumeMeshHexahedronIjk
{
  std::shared_ptr<MiGeometryI> m_geometry;
  std::shared_ptr<MiHexahedronTopologyExplicitIjk> m_topology;

public:

  HexahedronMesh(
    std::shared_ptr<MiGeometryI> geometry,
    std::shared_ptr<MiHexahedronTopologyExplicitIjk> topology);

  const MiHexahedronTopologyExplicitIjk& getTopology() const;

  const MiGeometryI& getGeometry() const;
};

/**
* Represents the mesh for a single reservoir
*/
class ReservoirMesh: public MiVolumeMeshHexahedronIjk
{
  std::shared_ptr<ReservoirGeometry> m_geometry;
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

class FaultGeometry : public MiGeometryI
{
  std::vector<MbVec3d> m_coords;
  size_t m_timeStamp;

public:

  explicit FaultGeometry(const std::vector<MbVec3d> coords);

  virtual MbVec3d getCoord(size_t index) const;

  //virtual MbVec3d getMin() const;

  //virtual MbVec3d getMax() const;

  virtual size_t getTimeStamp() const;
};

/**
 *
 */
class FaultTopology : public MiSurfaceTopologyExplicitI
{
  std::vector<QuadCell> m_cells;
  size_t m_timeStamp;

public:

  explicit FaultTopology(size_t numCells);

  virtual const MiSurfaceCell* getCell(size_t id) const;

  virtual size_t getBeginNodeId() const;

  virtual size_t getEndNodeId() const;

  virtual size_t getNumCells() const;

  virtual bool isDead(size_t i) const;

  virtual size_t getTimeStamp() const;

  virtual bool hasDeadCells() const;
};

/**
*
*/
class FaultMesh : public MiSurfaceMeshUnstructured
{
  std::shared_ptr<FaultGeometry> m_geometry;
  std::shared_ptr<FaultTopology> m_topology;

public:

  FaultMesh(
    std::shared_ptr<FaultGeometry> geometry,
    std::shared_ptr<FaultTopology> topology);

  virtual const MiSurfaceTopologyExplicitI& getTopology() const;

  virtual const MiGeometryI& getGeometry() const;
};
#endif

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
#include <MeshVizXLM/mesh/MiVolumeMeshCurvilinear.h>
#include <MeshVizXLM/mesh/MiSurfaceMeshCurvilinear.h>
#include <MeshVizXLM/mesh/MiSurfaceMeshUnstructured.h>
#include <MeshVizXLM/mesh/topology/MiSurfaceTopologyExplicitI.h>
#include <MeshVizXLM/mesh/cell/MiSurfaceCell.h>

/**
* Geometry class for reservoirs, which are defined by the top and bottom properties
*/
class ReservoirGeometry : public MiGeometryIjk
{
  const DataAccess::Interface::GridMap* m_depthMaps[2];

  double m_deltaX;
  double m_deltaY;

  size_t m_timeStamp;

public:

  ReservoirGeometry(
    const DataAccess::Interface::GridMap* depthMapTop, 
    const DataAccess::Interface::GridMap* depthMapBottom);

  ReservoirGeometry(const ReservoirGeometry&) = delete;
  ReservoirGeometry& operator=(const ReservoirGeometry&) = delete;

  virtual ~ReservoirGeometry();

  size_t numI() const;

  size_t numJ() const;

  size_t numK() const;

  bool isUndefined(size_t i, size_t j, size_t k) const;

  virtual MbVec3d getCoord(size_t i, size_t j, size_t k) const;

  virtual size_t getTimeStamp() const;
};


/**
 * Contains the geometry for an entire snapshot
 */
class SnapshotGeometry : public MiGeometryIjk
{
  GridMapCollection m_depthMaps;

  double m_deltaX;
  double m_deltaY;

  size_t m_timeStamp;

public:

  explicit SnapshotGeometry(const std::vector<const DataAccess::Interface::GridMap*>& depthMaps);

  size_t numI() const;

  size_t numJ() const;

  size_t numK() const;

  double deltaX() const;

  double deltaY() const;

  bool isUndefined(size_t i, size_t j, size_t k) const;

  double getDepth(size_t i, size_t j, size_t k) const;

  virtual MbVec3d getCoord(size_t i, size_t j, size_t k) const;

  virtual size_t getTimeStamp() const;
};

/**
* Defines the cell topology for a 'chunk' of a snapshot
*/
class SnapshotTopology : public MiTopologyIjk
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

  virtual MiMeshIjk::StorageLayout getStorageLayout() const;

  virtual size_t getNumCellsI() const;

  virtual size_t getNumCellsJ() const;

  virtual size_t getNumCellsK() const;

  virtual size_t getTimeStamp() const;

  virtual bool hasDeadCells() const;

  virtual bool isDead(size_t i, size_t j, size_t k) const;
};

/**
 *
 */
class ReservoirTopology : public MiTopologyIjk
{
  std::shared_ptr<ReservoirGeometry> m_geometry;
  size_t m_timestamp;

public:

  explicit ReservoirTopology(std::shared_ptr<ReservoirGeometry> geometry);

  virtual size_t getNumCellsI() const;

  virtual size_t getNumCellsJ() const;

  virtual size_t getNumCellsK() const;

  virtual bool isDead(size_t i, size_t j, size_t k) const;

  virtual bool hasDeadCells() const;

  virtual size_t getTimeStamp() const;
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

class SurfaceGeometry : public MiGeometryIj
{
  const DataAccess::Interface::GridMap* m_values;
  size_t m_timestamp;

  double m_deltaX;
  double m_deltaY;

public:

  explicit SurfaceGeometry(const DataAccess::Interface::GridMap* values);

  virtual ~SurfaceGeometry();

  virtual MbVec3d getCoord(size_t i, size_t j) const;

  virtual size_t getTimeStamp() const;

  bool isUndefined(size_t i, size_t j) const;
};

/**
* Defines the cell topology for a surface mesh
*/
class SurfaceTopology : public MiTopologyIj
{
  size_t m_numI;
  size_t m_numJ;

  size_t m_timeStamp;

  const SurfaceGeometry& m_geometry;

public:

  SurfaceTopology(size_t numI, size_t numJ, const SurfaceGeometry& geometry);

  virtual size_t getNumCellsI() const;

  virtual size_t getNumCellsJ() const;

  virtual bool isDead(size_t i, size_t j) const;

  virtual size_t getTimeStamp() const;

  virtual bool hasDeadCells() const;
};

/**
* Represents the mesh for a single formation
*/
class SnapshotMesh: public MiVolumeMeshCurvilinear
{
  std::shared_ptr<SnapshotGeometry> m_geometry;
  std::shared_ptr<SnapshotTopology> m_topology;

public:

  SnapshotMesh(
    std::shared_ptr<SnapshotGeometry> geometry,
    std::shared_ptr<SnapshotTopology> topology);

  const SnapshotTopology& getTopology() const;

  const SnapshotGeometry& getGeometry() const;
};

/**
* Represents the mesh for a single reservoir
*/
class ReservoirMesh: public MiVolumeMeshCurvilinear
{
  std::shared_ptr<ReservoirGeometry> m_geometry;
  std::shared_ptr<ReservoirTopology> m_topology;

public:

  ReservoirMesh(
    const DataAccess::Interface::GridMap* depthMapTop,
    const DataAccess::Interface::GridMap* depthMapBottom);

  ReservoirMesh(
    std::shared_ptr<ReservoirGeometry> geometry,
    std::shared_ptr<ReservoirTopology> topology);

  const MiTopologyIjk& getTopology() const;

  const MiGeometryIjk& getGeometry() const;
};

/**
*
*/
class SurfaceMesh : public MiSurfaceMeshCurvilinear
{
  std::shared_ptr<SurfaceGeometry> m_geometry;
  std::shared_ptr<SurfaceTopology> m_topology;

public:

  explicit SurfaceMesh(const DataAccess::Interface::GridMap* depthMap);

  virtual const MiTopologyIj& getTopology() const;

  virtual const MiGeometryIj& getGeometry() const;
};

class FaultGeometry : public MiGeometryI
{
  std::vector<MbVec3d> m_coords;
  size_t m_timeStamp;

public:

  explicit FaultGeometry(const std::vector<MbVec3d> coords);

  virtual MbVec3d getCoord(size_t index) const;

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

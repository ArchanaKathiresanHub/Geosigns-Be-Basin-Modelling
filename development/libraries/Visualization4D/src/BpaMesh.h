//
// Copyright (C) 2012-2015 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef BPAMESH_H_INCLUDED
#define BPAMESH_H_INCLUDED

#include "defines.h"

// DataAccess
#include "Interface/Interface.h"

// OIV
#include <MeshVizInterface/mesh/MiVolumeMeshHexahedronIjk.h>
#include <MeshVizInterface/mesh/data/MiDataSetI.h>

#include <memory>

class MiHexahedronTopologyExplicitIjk;
class MiGeometryI;

class BpaTopology;

/**
 * Struct meant to quickly map a global k-index to a gridmap index and a 
 * local k-index within that gridmap. 
 */
struct GridMapKPair
{
  unsigned int gridMapIndex;
  unsigned int kIndex;
};

/**
 * Stores the node coordinates for a BpaMesh
 */
class BpaGeometry : public MiGeometryI
{
  std::shared_ptr<DataAccess::Interface::PropertyValueList> m_depthValues; 
  std::vector<DataAccess::Interface::GridMap*> m_gridMaps;
  std::vector<GridMapKPair> m_gridMapKs;

  size_t m_subdivision;

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

  BpaGeometry(const DataAccess::Interface::Grid* grid, std::shared_ptr<DataAccess::Interface::PropertyValueList> depthValues, size_t subdivision);

  bool isUndefined(size_t i, size_t j, size_t k) const;

  MbVec3d getCoord(unsigned int i, unsigned int j, unsigned int k) const;

  virtual MbVec3d getCoord(size_t index) const;

  virtual MbVec3d getMin() const;

  virtual MbVec3d getMax() const;

  virtual size_t getTimeStamp() const;

  size_t getSubdivision() const;
};

/**
 * Represents the mesh for a single snapshot
 */
class BpaMesh : public MiVolumeMeshHexahedronIjk
{
  std::shared_ptr<BpaTopology> m_topology;
  std::shared_ptr<BpaGeometry> m_geometry;

  size_t m_subdivision;

public:

  BpaMesh(
    const DataAccess::Interface::Grid* grid, 
    std::shared_ptr<DataAccess::Interface::PropertyValueList> depthValues,
    size_t subdivision=1);

  virtual const MiHexahedronTopologyExplicitIjk& getTopology() const;

  virtual const MiGeometryI& getGeometry() const;
};

/**
 * Stores property values for a single snapshot
 */
class BpaProperty : public MiDataSetI<double>
{
  std::shared_ptr<DataAccess::Interface::PropertyValueList> m_values;
  
  std::vector<GridMapKPair> m_gridMapKs;

  size_t m_numI;
  size_t m_numJ;
  size_t m_numK;
  size_t m_subdivision;

  double m_minValue;
  double m_maxValue;

  size_t m_timeStamp;

  void initMinMaxValues();

public:

  BpaProperty(size_t numI, size_t numJ, std::shared_ptr<DataAccess::Interface::PropertyValueList> values, size_t subdivision);

  virtual MiDataSet::DataBinding getBinding() const;

  virtual double get(size_t index) const;

  virtual double getMin() const;

  virtual double getMax() const;

  virtual MiMeshIjk::StorageLayout getStorageLayout() const;

  virtual size_t getTimeStamp() const;

  virtual std::string getName() const;
};

#endif

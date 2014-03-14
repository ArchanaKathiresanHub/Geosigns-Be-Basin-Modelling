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
class BpaGeometry;

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
 * Represents the mesh for a single snapshot
 */
class BpaMesh : public MiVolumeMeshHexahedronIjk
{
  std::shared_ptr<BpaTopology> m_topology;
  std::shared_ptr<BpaGeometry> m_geometry;

public:

  BpaMesh(
    const DataAccess::Interface::Grid* grid, 
    std::shared_ptr<DataAccess::Interface::PropertyValueList> depthValues);

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

  double m_minValue;
  double m_maxValue;

  size_t m_timeStamp;

  void initMinMaxValues();

public:

  BpaProperty(size_t numI, size_t numJ, std::shared_ptr<DataAccess::Interface::PropertyValueList> values);

  virtual MiDataSet::DataBinding getBinding() const;

  virtual double get(size_t index) const;

  virtual double getMin() const;

  virtual double getMax() const;

  virtual MiMeshIjk::StorageLayout getStorageLayout() const;

  virtual size_t getTimeStamp() const;

  virtual std::string getName() const;
};

#endif

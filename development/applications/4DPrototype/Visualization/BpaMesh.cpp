#include "BpaMesh.h"

// DataAccess stuff
#include "Interface/ProjectHandle.h"
#include "Interface/Grid.h"
#include "Interface/GridMap.h"
#include "Interface/Property.h"
#include "Interface/PropertyValue.h"
#include "Interface/Snapshot.h"

// meshviz interface
#include <MeshVizInterface/MxTimeStamp.h>
#include <MeshVizInterface/mesh/data/MiDataSetIjk.h>
#include <MeshVizInterface/mesh/topology/MiHexahedronTopologyExplicitIjk.h>

namespace di = DataAccess::Interface; 

/**
 * Defines the cell topology for a BpaMesh
 */
class BpaTopology : public MiHexahedronTopologyExplicitIjk
{
  size_t m_numI;
  size_t m_numJ;
  size_t m_numK;

  size_t m_timeStamp;

  MiGeometryI& m_geometry;

public:

  explicit BpaTopology(size_t numI, size_t numJ, size_t numK, MiGeometryI& geometry)
    : m_numI(numI)
    , m_numJ(numJ)
    , m_numK(numK)
    , m_timeStamp(MxTimeStamp::getTimeStamp())
    , m_geometry(geometry)
  {
  }

  /*
  From the docs for MiHexahedronTopologyExplicitIjk:

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
  virtual void getCellNodeIndices(
    size_t i, size_t j, size_t k,
    size_t& n0, size_t& n1, size_t& n2, size_t& n3,
    size_t& n4, size_t& n5, size_t& n6, size_t& n7) const
  {
    size_t rowStride = m_numI;
    size_t sliceStride = m_numI * m_numJ;

    n0 = k * sliceStride + j * rowStride + i;
    n1 = n0 + 1;
    n2 = n1 + rowStride;
    n3 = n0 + rowStride;

    n4 = n0 + sliceStride;
    n5 = n1 + sliceStride;
    n6 = n2 + sliceStride;
    n7 = n3 + sliceStride;
  }

  virtual MiMeshIjk::StorageLayout getStorageLayout() const
  {
    // This means that I changes fastest, then J, then K
    return MiMeshIjk::LAYOUT_KJI;
  }

  virtual size_t getBeginNodeId() const
  {
    return 0;
  }

  virtual size_t getEndNodeId() const
  {
    return m_numI * m_numJ * m_numK;
  }

  virtual std::string getNodeName(size_t i) const
  {
    return "";
  }

  virtual size_t getNumCellsI() const
  {
    return m_numI - 1;
  }

  virtual size_t getNumCellsJ() const
  {
    return m_numJ - 1;
  }

  virtual size_t getNumCellsK() const
  {
    return m_numK - 1;
  }

  virtual size_t getTimeStamp() const
  {
    return m_timeStamp;
  }

  virtual bool hasDeadCells() const
  {
    return true;
  }

  virtual bool isDead(size_t i, size_t j, size_t k) const
  {
    size_t indices[8];
    getCellNodeIndices(
      i, j, k, 
      indices[0], 
      indices[1], 
      indices[2], 
      indices[3], 
      indices[4], 
      indices[5], 
      indices[6], 
      indices[7]);

    for(size_t i=0; i < 8; ++i)
    {
      if(m_geometry.getCoord(indices[i])[2] == -di::DefaultUndefinedMapValue)
        return true;
    }

    return false;
  }
};

/**
 * Stores the node coordinates for a BpaMesh
 */
class BpaGeometry : public MiGeometryI
{
  std::shared_ptr<di::PropertyValueList> m_depthValues; 

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

  explicit BpaGeometry(const di::Grid* grid, std::shared_ptr<di::PropertyValueList> depthValues)
    : m_depthValues(depthValues)
    , m_timeStamp(MxTimeStamp::getTimeStamp())
  {
    m_numI = grid->numI();
    m_numJ = grid->numJ();
    m_numK = m_depthValues->size();

    m_minX = grid->minI();
    m_minY = grid->minJ();

    m_deltaX = grid->deltaI();
    m_deltaY = grid->deltaJ();

    // Prep all gridmaps so multithreaded OIV access will be OK
    for(size_t k=0; k < m_numK; ++k)
      (*m_depthValues)[k]->getGridMap();

    double minDepth0, maxDepth0, minDepth1, maxDepth1;
    (*m_depthValues)[0]->getGridMap()->getMinMaxValue(minDepth0, maxDepth0);
    (*m_depthValues)[m_depthValues->size()-1]->getGridMap()->getMinMaxValue(minDepth1, maxDepth1);

    m_minZ = -maxDepth1;
    m_maxZ = -minDepth0;
  }

  virtual MbVec3d getCoord(size_t index) const
  {
    size_t rowStride = m_numI;
    size_t sliceStride = m_numI * m_numJ;

    size_t k = index / sliceStride;
    size_t j = (index - k * sliceStride) / rowStride;
    size_t i = index - k * sliceStride - j * rowStride;

    const di::GridMap* gridMap = (*m_depthValues)[k]->getGridMap();

    return MbVec3d(
      m_minX + i * m_deltaX,
      m_minY + j * m_deltaY,
      -gridMap->getValue((unsigned int)i, (unsigned int)j));
  }

  virtual MbVec3d getMin() const
  {
    return MbVec3d(m_minX, m_minY, m_minZ);
  }

  virtual MbVec3d getMax() const
  {
    return MbVec3d(
      m_minX + (m_numI - 1) * m_deltaX,
      m_minY + (m_numJ - 1) * m_deltaY,
      m_maxZ);
  }

  virtual size_t getTimeStamp() const
  {
    return m_timeStamp;
  }
};

BpaMesh::BpaMesh(const di::Grid* grid, std::shared_ptr<di::PropertyValueList> depthValues)
{
  m_geometry.reset(new BpaGeometry(grid, depthValues));
  m_topology.reset(new BpaTopology(grid->numI(), grid->numJ(), depthValues->size(), *m_geometry));
}

const MiHexahedronTopologyExplicitIjk& BpaMesh::getTopology() const
{
  return *m_topology;
}

const MiGeometryI& BpaMesh::getGeometry() const
{
  return *m_geometry;
}

//------------------------------------------------------------------------------
// BpaProperty implementation
//------------------------------------------------------------------------------

/**
 *
 */
void BpaProperty::initMinMaxValues()
{
  if(m_values->empty())
    return;

  (*m_values)[0]->getGridMap()->getMinMaxValue(m_minValue, m_maxValue);

  for(size_t i=1; i < m_values->size(); ++i)
  {
    double minValue, maxValue;
    (*m_values)[i]->getGridMap()->getMinMaxValue(minValue, maxValue);
    m_minValue = minValue < m_minValue ? minValue : m_minValue;
    m_maxValue = maxValue > m_maxValue ? maxValue : m_maxValue;
  }
}

BpaProperty::BpaProperty(size_t numI, size_t numJ, std::shared_ptr<di::PropertyValueList> values)
  : m_values(values)
  , m_numI(numI)
  , m_numJ(numJ)
  , m_numK(values->size())
  , m_minValue(0.0)
  , m_maxValue(0.0)
  , m_timeStamp(MxTimeStamp::getTimeStamp())
{
  initMinMaxValues();
}

MiDataSet::DataBinding BpaProperty::getBinding() const
{
  return MiDataSet::PER_NODE;
}

double BpaProperty::get(size_t index) const
{
  // TODO: this is not very nice, is it?
  if(m_values->empty())
    return 0.0;

  size_t rowStride = m_numI;
  size_t sliceStride = m_numI * m_numJ;

  size_t k = index / sliceStride;
  size_t j = (index - k * sliceStride) / rowStride;
  size_t i = index - k * sliceStride - j * rowStride;

  // Some (or all?) properties not defined for crust and mantle
  // TODO: handle this in a better way
  if(k >= m_numK)
    k = m_numK-1;

  di::GridMap* gridMap = (*m_values)[k]->getGridMap();
  return gridMap->getValue((unsigned int)i, (unsigned int)j);
}

double BpaProperty::getMin() const
{
  return m_minValue;
}

double BpaProperty::getMax() const
{
  return m_maxValue;
}

MiMeshIjk::StorageLayout BpaProperty::getStorageLayout() const
{
  return MiMeshIjk::LAYOUT_KJI;
}

size_t BpaProperty::getTimeStamp() const
{
  return m_timeStamp;
}

std::string BpaProperty::getName() const
{
  return "Dummy";
}



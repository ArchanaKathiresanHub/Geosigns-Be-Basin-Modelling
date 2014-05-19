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

BpaGeometry::BpaGeometry(const di::Grid* grid, std::shared_ptr<di::PropertyValueList> depthValues, size_t subdivision)
  : m_depthValues(depthValues)
  , m_subdivision(subdivision)
  , m_timeStamp(MxTimeStamp::getTimeStamp())
{
  m_numI = grid->numI();
  m_numJ = grid->numJ();

  m_numK = 0;
  for(unsigned int i=0; i < depthValues->size(); ++i)
  {
    di::GridMap* gridMap = (*depthValues)[i]->getGridMap();
    m_gridMaps.push_back(gridMap);

    unsigned int depth = gridMap->getDepth();
    m_numK += depth;
        
    for(unsigned int j=0; j < depth; ++j)
    {
      GridMapKPair gmpair = { i, j };
      m_gridMapKs.push_back(gmpair);
    }
  }

  m_minX = grid->minI();
  m_minY = grid->minJ();

  m_deltaX = grid->deltaI();
  m_deltaY = grid->deltaJ();

  double minDepth0, maxDepth0, minDepth1, maxDepth1;
  (*m_depthValues)[0]->getGridMap()->getMinMaxValue(minDepth0, maxDepth0);
  (*m_depthValues)[m_depthValues->size()-1]->getGridMap()->getMinMaxValue(minDepth1, maxDepth1);

  m_minZ = -maxDepth1;
  m_maxZ = -minDepth0;
}

bool BpaGeometry::isUndefined(size_t i, size_t j, size_t k) const
{
  GridMapKPair gmpair = m_gridMapKs[k];
  const di::GridMap* gridMap = m_gridMaps[gmpair.gridMapIndex];

  return gridMap->getValue((unsigned int)(i / m_subdivision), (unsigned int)(j / m_subdivision), gmpair.kIndex) == di::DefaultUndefinedMapValue;
}

MbVec3d BpaGeometry::getCoord(unsigned int i, unsigned int j, unsigned int k) const
{
  GridMapKPair gmpair = m_gridMapKs[k];
  const di::GridMap* gridMap = m_gridMaps[gmpair.gridMapIndex];

  return MbVec3d(
    m_minX + i * m_deltaX / m_subdivision,
    m_minY + j * m_deltaY / m_subdivision,
    -gridMap->getValue((unsigned int)(i / m_subdivision), (unsigned int)(j / m_subdivision), gmpair.kIndex));
}

MbVec3d BpaGeometry::getCoord(size_t index) const
{
  size_t rowStride = m_numI * m_subdivision;
  size_t sliceStride = rowStride * m_numJ * m_subdivision;

  unsigned int k = (unsigned int)(index / sliceStride);
  unsigned int j = (unsigned int)((index - k * sliceStride) / rowStride);
  unsigned int i = (unsigned int)(index - k * sliceStride - j * rowStride);

  return getCoord(i, j, k);
}

MbVec3d BpaGeometry::getMin() const
{
  return MbVec3d(m_minX, m_minY, m_minZ);
}

MbVec3d BpaGeometry::getMax() const
{
  return MbVec3d(
    m_minX + (m_numI - 1) * m_deltaX,
    m_minY + (m_numJ - 1) * m_deltaY,
    m_maxZ);
}

size_t BpaGeometry::getTimeStamp() const
{
  return m_timeStamp;
}

size_t BpaGeometry::getSubdivision() const
{
  return m_subdivision;
}

/**
 * Defines the cell topology for a BpaMesh
 */
class BpaTopology : public MiHexahedronTopologyExplicitIjk
{
  size_t m_numI;
  size_t m_numJ;
  size_t m_numK;
  size_t m_subdivision;

  size_t m_timeStamp;

  BpaGeometry& m_geometry;

public:

  explicit BpaTopology(size_t numI, size_t numJ, size_t numK, BpaGeometry& geometry)
    : m_numI(numI)
    , m_numJ(numJ)
    , m_numK(numK)
    , m_subdivision((size_t)geometry.getSubdivision())
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
    size_t rowStride = m_numI * m_subdivision;
    size_t sliceStride = rowStride * m_numJ * m_subdivision;

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
    // This means that K changes fastest, then J, then I
    return MiMeshIjk::LAYOUT_IJK;
  }

  virtual size_t getBeginNodeId() const
  {
    return 0;
  }

  virtual size_t getEndNodeId() const
  {
    return m_numI * m_numJ * m_numK * m_subdivision * m_subdivision;
  }

  virtual std::string getNodeName(size_t i) const
  {
    return "";
  }

  virtual size_t getNumCellsI() const
  {
    return m_subdivision * m_numI - 1;
  }

  virtual size_t getNumCellsJ() const
  {
    return m_subdivision * m_numJ - 1;
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
    if(
      m_geometry.isUndefined(i, j, k)     ||
      m_geometry.isUndefined(i, j, k+1)   ||
      m_geometry.isUndefined(i, j+1, k)   ||
      m_geometry.isUndefined(i, j+1, k+1) ||
      m_geometry.isUndefined(i+1, j, k)   ||
      m_geometry.isUndefined(i+1, j, k+1) ||
      m_geometry.isUndefined(i+1, j+1, k) ||
      m_geometry.isUndefined(i+1, j+1, k+1))
    {
      return true;
    }

    return false;
  }
};


BpaMesh::BpaMesh(const di::Grid* grid, std::shared_ptr<di::PropertyValueList> depthValues, size_t subdivision)
  : m_subdivision(subdivision)
{
  m_geometry.reset(new BpaGeometry(grid, depthValues, subdivision));

  unsigned int nK=0;
  for(size_t i=0; i < depthValues->size(); ++i)
    nK += (*depthValues)[i]->getGridMap()->getDepth();

  m_topology.reset(new BpaTopology(grid->numI(), grid->numJ(), nK, *m_geometry));
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

BpaProperty::BpaProperty(size_t numI, size_t numJ, std::shared_ptr<di::PropertyValueList> values, size_t subdivision)
  : m_values(values)
  , m_numI(numI)
  , m_numJ(numJ)
  , m_numK(0)
  , m_subdivision(subdivision)
  , m_minValue(0.0)
  , m_maxValue(0.0)
  , m_timeStamp(MxTimeStamp::getTimeStamp())
{
  initMinMaxValues();

  m_numK = 0;
  for(unsigned int i=0; i < values->size(); ++i)
  {
    unsigned int depth = (*values)[i]->getGridMap()->getDepth();
    m_numK += depth;

    for(unsigned int j=0; j < depth; ++j)
    {
      GridMapKPair gmpair = { i, j };
      m_gridMapKs.push_back(gmpair);
    }
  }

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

  size_t rowStride = m_numI * m_subdivision;
  size_t sliceStride = rowStride * m_numJ * m_subdivision;

  size_t k = index / sliceStride;
  size_t j = (index - k * sliceStride) / rowStride;
  size_t i = index - k * sliceStride - j * rowStride;

  // Some (or all?) properties not defined for crust and mantle
  // TODO: handle this in a better way
  if(k >= m_numK)
    k = m_numK-1;

  GridMapKPair gmpair = m_gridMapKs[k];
  di::GridMap* gridMap = (*m_values)[gmpair.gridMapIndex]->getGridMap();
  return gridMap->getValue((unsigned int)(i / m_subdivision), (unsigned int)(j / m_subdivision), gmpair.kIndex);
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
    // This means that K changes fastest, then J, then I
    return MiMeshIjk::LAYOUT_IJK;
}

size_t BpaProperty::getTimeStamp() const
{
  return m_timeStamp;
}

std::string BpaProperty::getName() const
{
  return "Dummy";
}



#include "BpaMesh.h"

// DataAccess stuff
#include "Interface/ProjectHandle.h"
#include "Interface/Formation.h"
#include "Interface/Surface.h"
#include "Interface/Grid.h"
#include "Interface/GridMap.h"
#include "Interface/Property.h"
#include "Interface/PropertyValue.h"
#include "Interface/Snapshot.h"

// OIV nodes
#include <Inventor/nodes/SoNode.h>
#include <Inventor/nodes/SoGroup.h>
#include <Inventor/nodes/SoSwitch.h>
#include <Inventor/nodes/SoSeparator.h>

// meshviz interface
#include <MeshVizInterface/mesh/topology/MiHexahedronTopologyExplicitIjk.h>
#include <MeshVizInterface/mesh/data/MiDataSetIjk.h>

// meshviz nodes
#include <MeshVizInterface/mapping/nodes/MoMesh.h>
#include <MeshVizInterface/mapping/nodes/MoMeshSkin.h>
#include <MeshVizInterface/mapping/nodes/MoMaterial.h>
#include <MeshVizInterface/mapping/nodes/MoDrawStyle.h>
#include <MeshVizInterface/mapping/nodes/MoScalarSetI.h>
#include <MeshVizInterface/mapping/nodes/MoPredefinedColorMapping.h>
#include <MeshVizInterface/mapping/nodes/MoDataBinding.h>

namespace di = DataAccess::Interface; 

/**
 *
 */
class BpaTopology : public MiHexahedronTopologyExplicitIjk
{
  size_t m_numI;
  size_t m_numJ;
  size_t m_numK;

public:

  explicit BpaTopology(size_t numI, size_t numJ, size_t numK)
    : m_numI(numI)
    , m_numJ(numJ)
    , m_numK(numK)
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
    return 0;
  }

  virtual bool hasDeadCells() const
  {
    return false;
  }

  virtual bool isDead(size_t i, size_t j, size_t k) const
  {
    return false;
  }
};

/**
 *
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

public:

  explicit BpaGeometry(const di::Grid* grid, std::shared_ptr<di::PropertyValueList> depthValues)
    : m_depthValues(depthValues)
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
    return 0;
  }
};

BpaMesh::BpaMesh(const DataAccess::Interface::Snapshot* snapshot)
{
  di::ProjectHandle* handle = snapshot->getProjectHandle();
  const di::Grid* grid = handle->getLowResolutionOutputGrid();

  const di::Property* depthProperty = handle->findProperty("Depth");
  int flags = di::FORMATION;// | di::SURFACE | di::FORMATIONSURFACE;
  int type  = di::VOLUME;

  std::shared_ptr<di::PropertyValueList> depthValues(handle->getPropertyValues(flags, depthProperty, snapshot, 0, 0, 0, type));

  m_topology.reset(new BpaTopology(grid->numI(), grid->numJ(), depthValues->size()));
  m_geometry.reset(new BpaGeometry(grid, depthValues));

}

const MiHexahedronTopologyExplicitIjk& BpaMesh::getTopology() const
{
  return *m_topology;
}

const MiGeometryI& BpaMesh::getGeometry() const
{
  return *m_geometry;
}

/**
 *
 */
class BpaProperty : public MiDataSetI<double>
{
  std::shared_ptr<di::PropertyValueList> m_values;
  
  size_t m_numI;
  size_t m_numJ;
  size_t m_numK;

  double m_minValue;
  double m_maxValue;

  void initMinMaxValues()
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

public:

  BpaProperty(size_t numI, size_t numJ, std::shared_ptr<di::PropertyValueList> values)
    : m_values(values)
    , m_numI(numI)
    , m_numJ(numJ)
    , m_numK(values->size())
    , m_minValue(0.0)
    , m_maxValue(0.0)
  {
    initMinMaxValues();
  }

  virtual MiDataSet::DataBinding getBinding() const
  {
    return MiDataSet::PER_NODE;
  }

  virtual double get(size_t index) const
  {
    // TODO: this is not very nice, is it?
    if(m_values->empty())
      return 0.0;

    size_t rowStride = m_numI;
    size_t sliceStride = m_numI * m_numJ;

    size_t k = index / sliceStride;
    size_t j = (index - k * sliceStride) / rowStride;
    size_t i = index - k * sliceStride - j * rowStride;

    if(k >= m_numK)
      k = m_numK-1;

    return (*m_values)[k]->getGridMap()->getValue((unsigned int)i, (unsigned int)j);
  }

  virtual double getMin() const
  {
    return m_minValue;
  }

  virtual double getMax() const
  {
    return m_maxValue;
  }

  virtual MiMeshIjk::StorageLayout getStorageLayout() const
  {
    return MiMeshIjk::LAYOUT_KJI;
  }

  virtual size_t getTimeStamp() const
  {
    return 0;
  }

  virtual std::string getName() const
  {
    return "Dummy";
  }
};


SoNode* createOIVTree(const di::Snapshot* snapshot, BpaProperty* bpaProperty)
{
  BpaMesh* bpaMesh = new BpaMesh(snapshot);
  
  MoMesh* mesh = new MoMesh;
  mesh->setMesh(bpaMesh);

  MoScalarSetI* scalarSet = new MoScalarSetI;
  scalarSet->setScalarSet(bpaProperty);

  MoMaterial* material = new MoMaterial;
  material->faceColoring = MoMaterial::CONTOURING;
  material->faceColor = SbColor(1.0f, 0.0f, 0.0f);
  material->lineColoring = MoMaterial::COLOR;
  material->lineColor = SbColor(0.0f, 0.0f, 0.0f);
  //material->transparency = .75f;

  MoDrawStyle* drawStyle = new MoDrawStyle;
  drawStyle->displayFaces = true;
  drawStyle->displayEdges = true;
  drawStyle->displayPoints = false;

  MoDataBinding* binding = new MoDataBinding;
  binding->dataBinding = MoDataBinding::PER_NODE;

  MoPredefinedColorMapping* colorMap = new MoPredefinedColorMapping;
  colorMap->predefColorMap = MoPredefinedColorMapping::STANDARD;
  colorMap->minValue = (float)scalarSet->getScalarSet()->getMin();
  colorMap->maxValue = (float)scalarSet->getScalarSet()->getMax();

  MoMeshSkin* skin = new MoMeshSkin;
  skin->colorScalarSetId = 0;

  SoSeparator* group = new SoSeparator;
  group->addChild(mesh);
  group->addChild(scalarSet);
  group->addChild(material);
  group->addChild(drawStyle);
  group->addChild(binding);
  group->addChild(colorMap);
  group->addChild(skin);

  return group;
}

SoNode* createOIVTree(const char* filename)
{
	const char* access = "r";
	di::ProjectHandle* handle = di::OpenCauldronProject(filename, access);

  int flags = di::FORMATION;// | di::SURFACE | di::FORMATIONSURFACE;
  int type  = di::VOLUME;
  const di::Property* prop = handle->findProperty("Temperature");
  const di::Grid* grid = handle->getLowResolutionOutputGrid();

  SoSwitch* snapshotSwitch = new SoSwitch;

  di::SnapshotList* snapshots = handle->getSnapshots(di::MAJOR);
  for(size_t i=0; i < snapshots->size(); ++i)
  {
    const di::Snapshot* snapshot = (*snapshots)[i];

    std::shared_ptr<di::PropertyValueList> values(handle->getPropertyValues(flags, prop, snapshot, 0, 0, 0, type));
    BpaProperty* bpaProperty = new BpaProperty(grid->numI(), grid->numJ(), values);

    snapshotSwitch->addChild(createOIVTree(snapshot, bpaProperty));
  }

  snapshotSwitch->whichChild = 0;

  return snapshotSwitch;
}

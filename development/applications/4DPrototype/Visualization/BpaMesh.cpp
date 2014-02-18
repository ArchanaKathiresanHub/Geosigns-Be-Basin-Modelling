#include "BpaMesh.h"

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
#include <Inventor/nodes/SoSeparator.h>

// meshviz interface
#include <MeshVizInterface/mesh/topology/MiHexahedronTopologyExplicitIjk.h>
#include <MeshVizInterface/mesh/data/MiDataSetIjk.h>

// meshviz nodes
#include <MeshVizInterface/mapping/nodes/MoMesh.h>
#include <MeshVizInterface/mapping/nodes/MoMeshSkin.h>
#include <MeshVizInterface/mapping/nodes/MoMaterial.h>
#include <MeshVizInterface/mapping/nodes/MoDrawStyle.h>
#include <MeshVizInterface/mapping/nodes/MoScalarSetIjk.h>
#include <MeshVizInterface/mapping/nodes/MoPredefinedColorMapping.h>
#include <MeshVizInterface/mapping/nodes/MoDataBinding.h>

namespace di = DataAccess::Interface; 

void foo(const char* filename)
{
	const char* access = "r";
	di::ProjectHandle* handle = di::OpenCauldronProject(filename, access);
  
  const di::Grid* grid = handle->getLowResolutionOutputGrid();

  char buffer[128];
  sprintf(buffer, "Grid: %d x %d\n", grid->numI(), grid->numJ());
  OutputDebugString(buffer);

  di::SnapshotList* snapshots = handle->getSnapshots(di::MAJOR);
  for(size_t i=0; i < snapshots->size(); ++i)
  {
    const di::Snapshot* snapshot = (*snapshots)[i];
    double t = snapshot->getTime();
    char buffer[128];
    sprintf(buffer, "snapshot %d: time = %f\n", i, t);
    OutputDebugString(buffer);
  }


  int flags = di::FORMATION | di::SURFACE | di::FORMATIONSURFACE;
  //int flags = di::FORMATION;
  //int flags = di::RESERVOIR;
  //int type  = di::MAP;
  int type = di::VOLUME;
  const di::Snapshot* presentDay = (*snapshots)[0];

  // Handle depth property
  const di::Property* depthProperty = handle->findProperty("Depth");

  di::PropertyValueList* values = handle->getPropertyValues(flags, depthProperty, presentDay, 0, 0, 0, type);
  for(size_t j=0; j < values->size(); ++j)
  {
    const di::PropertyValue* propValue = (*values)[j];
    const di::GridMap* gridMap = propValue->getGridMap();

    double v = gridMap->getValue((unsigned int)0, (unsigned int)0);
  }

  di::PropertyList* properties = handle->getProperties(true, flags, presentDay, 0, 0, 0, type);
  for(size_t i=0; i < properties->size(); ++i)
  {
    const di::Property* prop = (*properties)[i];
    char buffer[256];
    sprintf(buffer, "Property %d: %s\n", i, prop->getName().c_str());
    OutputDebugString(buffer);
  }

  const di::FormationList* formations = handle->getFormations(presentDay);
  for(size_t i=0; i < formations->size(); ++i)
  {
    const di::Formation* formation = (*formations)[i];
  }

  di::SurfaceList* surfaces = handle->getSurfaces();
  for(size_t i=0; i < surfaces->size(); ++i)
  {
    const di::Surface* surface = (*surfaces)[i];
  }
	//di::InitializeSerializedIO();
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------

/**
 *
 */
class BpaTopology : public MiHexahedronTopologyExplicitIjk
{
  size_t m_numI;
  size_t m_numJ;
  size_t m_numK;

public:

  explicit BpaTopology(const di::Snapshot* snapshot)
  {
    di::ProjectHandle* handle = snapshot->getProjectHandle();
    
    const di::Grid* grid = handle->getLowResolutionOutputGrid();

    m_numI = grid->numI();
    m_numJ = grid->numJ();

    int flags = di::FORMATION | di::SURFACE | di::FORMATIONSURFACE;
    int type  = di::VOLUME;

    const di::Property* depthProperty = handle->findProperty("Depth");

    const di::PropertyValueList* values = handle->getPropertyValues(flags, depthProperty, snapshot, 0, 0, 0, type);

    m_numK = values->size();
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
  const di::Snapshot* m_snapshot;
  
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

  explicit BpaGeometry(const di::Snapshot* snapshot)
    : m_snapshot(snapshot)
  {
    di::ProjectHandle* handle = snapshot->getProjectHandle();
    
    const di::Grid* grid = handle->getLowResolutionOutputGrid();

    m_numI = grid->numI();
    m_numJ = grid->numJ();

    int flags = di::FORMATION | di::SURFACE | di::FORMATIONSURFACE;
    int type  = di::VOLUME;

    const di::Property* depthProperty = handle->findProperty("Depth");
    m_depthValues.reset(handle->getPropertyValues(flags, depthProperty, snapshot, 0, 0, 0, type));

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
  : m_snapshot(snapshot)
  , m_topology(new BpaTopology(snapshot))
  , m_geometry(new BpaGeometry(snapshot))
{
}

const MiHexahedronTopologyExplicitIjk& BpaMesh::getTopology() const
{
  return *m_topology;
}

const MiGeometryI& BpaMesh::getGeometry() const
{
  return *m_geometry;
}

class BpaProperty : public MiDataSetIjk<double>
{
  size_t m_numI;
  size_t m_numJ;
  size_t m_numK;

public:

  BpaProperty(size_t numI, size_t numJ, size_t numK)
    : m_numI(numI)
    , m_numJ(numJ)
    , m_numK(numK)
  {
  }

  virtual MiDataSet::DataBinding getBinding() const
  {
    return MiDataSet::PER_CELL;
  }

  virtual double get(size_t i, size_t j, size_t k) const
  {
    double x = i - .5 * m_numI;
    double y = j - .5 * m_numJ;
    double z = k - .5 * m_numK;

    return sqrt(x*x + y*y + z*z);
  }

  virtual double getMin() const
  {
    return 0.0;
  }

  virtual double getMax() const
  {
    return .5 * sqrt((double)(m_numI * m_numI + m_numJ * m_numJ + m_numK * m_numK));
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


SoNode* createOIVTree(const di::Snapshot* snapshot)
{
  BpaMesh* bpaMesh = new BpaMesh(snapshot);
  
  MoMesh* mesh = new MoMesh;
  mesh->setMesh(bpaMesh);

  //MoScalarSetIjk* scalarSet = new MoScalarSetIjk;
  //scalarSet->setScalarSet(new MyDataSetIjk(numI, numJ, numK));

  MoMaterial* material = new MoMaterial;
  material->faceColoring = MoMaterial::COLOR;
  material->faceColor = SbColor(1.0f, 0.0f, 0.0f);
  material->lineColoring = MoMaterial::COLOR;
  material->lineColor = SbColor(0.0f, 0.0f, 0.0f);
  //material->transparency = .75f;

  MoDrawStyle* drawStyle = new MoDrawStyle;
  drawStyle->displayFaces = true;
  drawStyle->displayEdges = true;
  drawStyle->displayPoints = false;

  MoDataBinding* binding = new MoDataBinding;
  binding->dataBinding = MoDataBinding::PER_CELL;

  //MoPredefinedColorMapping* colorMap = new MoPredefinedColorMapping;
  //colorMap->predefColorMap = MoPredefinedColorMapping::STANDARD;
  //colorMap->minValue = (float)scalarSet->getScalarSet()->getMin();
  //colorMap->maxValue = (float)scalarSet->getScalarSet()->getMax();

  MoMeshSkin* skin = new MoMeshSkin;
  skin->colorScalarSetId = 0;

  SoSeparator* group = new SoSeparator;
  group->addChild(mesh);
  //group->addChild(scalarSet);
  group->addChild(material);
  group->addChild(drawStyle);
  group->addChild(binding);
  //group->addChild(colorMap);
  group->addChild(skin);

  return group;
}

SoNode* createOIVTree(const char* filename)
{
	const char* access = "r";
	di::ProjectHandle* handle = di::OpenCauldronProject(filename, access);
  di::SnapshotList* snapshots = handle->getSnapshots(di::MAJOR);

  return createOIVTree((*snapshots)[0]);
}

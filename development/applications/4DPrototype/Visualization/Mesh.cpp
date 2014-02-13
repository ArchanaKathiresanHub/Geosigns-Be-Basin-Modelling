#include "Mesh.h"

#include "Interface/ProjectHandle.h"
#include "Interface/Formation.h"
#include "Interface/Grid.h"
#include "Interface/Property.h"

// OIV nodes
#include <Inventor/nodes/SoNode.h>
#include <Inventor/nodes/SoGroup.h>
#include <Inventor/nodes/SoSeparator.h>

// meshviz interface
#include <MeshVizInterface/mesh/MiVolumeMeshHexahedronIjk.h>
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

#include <string>

void foo(const char* filename)
{
	const char* access = "r";
	DataAccess::Interface::ProjectHandle* handle = DataAccess::Interface::OpenCauldronProject(filename, access);
  
  const DataAccess::Interface::Grid* grid = handle->getLowResolutionOutputGrid();
  const DataAccess::Interface::PropertyList* properties = handle->getProperties();

  DataAccess::Interface::FormationList* formations = handle->getFormations();
  for(size_t i=0; i < formations->size(); ++i)
  {
    const DataAccess::Interface::Formation* formation = (*formations)[i];
  }

  DataAccess::Interface::SurfaceList* surfaces = handle->getSurfaces();
  for(size_t i=0; i < surfaces->size(); ++i)
  {
    const DataAccess::Interface::Surface* surface = (*surfaces)[i];
  }
	//DataAccess::Interface::InitializeSerializedIO();
}

class MyTopology : public MiHexahedronTopologyExplicitIjk
{
  size_t m_numI;
  size_t m_numJ;
  size_t m_numK;

public:

  MyTopology(size_t numI, size_t numJ, size_t numK)
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

class MyGeometry : public MiGeometryI
{
  size_t m_numI;
  size_t m_numJ;
  size_t m_numK;

  double m_minX;
  double m_minY;
  double m_minZ;

  double m_deltaX;
  double m_deltaY;
  double m_deltaZ;

public:

  MyGeometry(size_t numI, size_t numJ, size_t numK)
    : m_numI(numI)
    , m_numJ(numJ)
    , m_numK(numK)
    , m_minX(0.0)
    , m_minY(0.0)
    , m_minZ(0.0)
    , m_deltaX(1.0)
    , m_deltaY(1.0)
    , m_deltaZ(-1.0)
  {
  }

  virtual MbVec3d getCoord(size_t index) const
  {
    size_t rowStride = m_numI;
    size_t sliceStride = m_numI * m_numJ;

    size_t k = index / sliceStride;
    size_t j = (index - k * sliceStride) / rowStride;
    size_t i = index - k * sliceStride - j * rowStride;

    double A = 2.0;
    double dzi = A * sin(i * 2 * M_PI / m_numI);
    double dzj = A * sin(j * 2 * M_PI / m_numJ);

    return MbVec3d(
      m_minX + i * m_deltaX,
      m_minY + j * m_deltaY,
      m_minZ + k * m_deltaZ + dzi + dzj);
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
      m_minZ + (m_numK - 1) * m_deltaZ);
  }

  virtual size_t getTimeStamp() const
  {
    return 0;
  }
};

class MyMesh : public MiVolumeMeshHexahedronIjk
{
  MyGeometry m_geometry;
  MyTopology m_topology;

public:

  MyMesh(size_t numI, size_t numJ, size_t numK)
    : m_geometry(numI, numJ, numK)
    , m_topology(numI, numJ, numK)
  {
  }

  virtual const MiHexahedronTopologyExplicitIjk& getTopology() const
  {
    return m_topology;
  }

  virtual const MiGeometryI& getGeometry() const
  {
    return m_geometry;
  }
};

class MyDataSetIjk : public MiDataSetIjk<double>
{
  size_t m_numI;
  size_t m_numJ;
  size_t m_numK;

public:

  MyDataSetIjk(size_t numI, size_t numJ, size_t numK)
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

SoNode* createOIVTree()
{
  size_t numI = 100;
  size_t numJ = 100;
  size_t numK =  20;

  MyMesh* myMesh = new MyMesh(numI, numJ, numK);

  MoMesh* mesh = new MoMesh;
  mesh->setMesh(myMesh);

  MoScalarSetIjk* scalarSet = new MoScalarSetIjk;
  scalarSet->setScalarSet(new MyDataSetIjk(numI, numJ, numK));

  MoMaterial* material = new MoMaterial;
  material->faceColoring = MoMaterial::CONTOURING;
  material->faceColor = SbColor(1.0f, 0.0f, 0.0f);
  material->lineColoring = MoMaterial::COLOR;
  material->lineColor = SbColor(0.0f, 0.0f, 0.0f);

  MoDrawStyle* drawStyle = new MoDrawStyle;
  drawStyle->displayFaces = true;
  drawStyle->displayEdges = true;
  drawStyle->displayPoints = false;

  MoDataBinding* binding = new MoDataBinding;
  binding->dataBinding = MoDataBinding::PER_CELL;

  MoPredefinedColorMapping* colorMap = new MoPredefinedColorMapping;
  colorMap->predefColorMap = MoPredefinedColorMapping::STANDARD;
  colorMap->minValue = scalarSet->getScalarSet()->getMin();
  colorMap->maxValue = scalarSet->getScalarSet()->getMax();

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

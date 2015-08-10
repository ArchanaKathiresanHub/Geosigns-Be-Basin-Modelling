
#include "SceneGraph.h"
#include "SnapshotNode.h"
#include "Property.h"
#include "Mesh.h"
#include "BpaMesh.h"
#include "ROICellFilter.h"
#include "SkinExtractor.h"

// DataAccess
#include "Interface/Grid.h"
#include "Interface/Snapshot.h"
#include "Interface/Property.h"
#include "Interface/Formation.h"

//TMP
#include "Interface/PropertyValue.h"
#include "Interface/GridMap.h"

#include "Interface/ProjectHandle.h"

// OIV
#include <Inventor/nodes/SoGroup.h>
#include <Inventor/nodes/SoScale.h>
#include <Inventor/nodes/SoSwitch.h>
#include <Inventor/manips/SoClipPlaneManip.h>
#include <Inventor/draggers/SoJackDragger.h>

// meshviz nodes
#include <MeshVizInterface/mapping/nodes/MoMaterial.h>
#include <MeshVizInterface/mapping/nodes/MoDrawStyle.h>
#include <MeshVizInterface/mapping/nodes/MoScalarSetI.h>
#include <MeshVizInterface/mapping/nodes/MoScalarSetIjk.h>
#include <MeshVizInterface/mapping/nodes/MoDataBinding.h>
#include <MeshVizInterface/mapping/nodes/MoPredefinedColorMapping.h>
#include <MeshVizInterface/mapping/nodes/MoCellFilter.h>
#include <MeshVizInterface/mapping/nodes/MoMesh.h>
#include <MeshVizInterface/mapping/nodes/MoMeshSkin.h>

#include <algorithm>

namespace di = DataAccess::Interface; 

//#define PRE_EXTRACT
//#define CUSTOM_EXTRACTION

namespace
{

  SbPlane getDefaultPlane()
  {
    SbPlane plane(SbVec3f(0.0f, 1.0f, 0.0f), -10.0f);
    return plane;
  }

}

SO_NODE_SOURCE(SceneGraph);

void SceneGraph::initClass()
{
  SO_NODE_INIT_CLASS(SceneGraph, SoGroup, "Group");
}

void SceneGraph::exitClass()
{
  SO__NODE_EXIT_CLASS(SceneGraph);
}

void SceneGraph::createFilterNode()
{
  m_cellFilterSwitch = new SoSwitch;
  m_cellFilter = new MoCellFilter;
  m_roiFilter = new ROICellFilter;

  m_cellFilter->setCellFilter(m_roiFilter);
  m_cellFilterSwitch->addChild(m_cellFilter);
  m_cellFilterSwitch->whichChild = SO_SWITCH_NONE;
}

void SceneGraph::createAppearanceNode()
{
  m_colorMap = new MoPredefinedColorMapping;
  m_colorMap->predefColorMap = MoPredefinedColorMapping::STANDARD;
  m_colorMap->minValue = 0.0f;
  m_colorMap->maxValue = 100.0f;

  m_drawStyle = new MoDrawStyle;
  m_drawStyle->displayFaces = true;
  m_drawStyle->displayEdges = true;
  m_drawStyle->displayPoints = false;
  //m_drawStyle->fadingThreshold = 5.0f;

  MoMaterial* material = new MoMaterial;
  material->faceColoring = MoMaterial::CONTOURING;
  material->faceColor = SbColor(1.0f, 0.0f, 0.0f);
  material->lineColoring = MoMaterial::COLOR;
  material->lineColor = SbColor(0.0f, 0.0f, 0.0f);
  //material->enhancedColoring = true;
  //material->transparency = .75f;

  MoDataBinding* binding = new MoDataBinding;
  binding->dataBinding = MoDataBinding::PER_CELL;

  m_appearance = new SoGroup;
  m_appearance->addChild(m_colorMap);
  m_appearance->addChild(m_drawStyle);
  m_appearance->addChild(material);
  m_appearance->addChild(binding);
}

struct SnapshotCompare
{
  bool operator()(const di::Snapshot* s0, const di::Snapshot* s1) const
  {
    return s0->getTime() > s1->getTime();
  }
};

SoGroup* SceneGraph::createSnapshotNode(di::ProjectHandle* handle, const di::Snapshot* snapshot)
{
  std::vector<const di::GridMap*> depthMaps;

  std::string depthKey = "Depth";
  const di::Property* depthProperty = handle->findProperty(depthKey);

  std::vector<double> formationIds;

  std::unique_ptr<di::FormationList> formations(handle->getFormations(snapshot, false));
  for (size_t i = 0; i < formations->size(); ++i)
  {
    const di::Formation* formation = (*formations)[i];
    std::unique_ptr<di::PropertyValueList> values(handle->getPropertyValues(di::FORMATION, depthProperty, snapshot, 0, formation, 0, di::VOLUME));

    assert(values->size() == 1);

    const di::GridMap* depthMap = (*values)[0]->getGridMap();
    depthMaps.push_back(depthMap);

    // Add formation id for each k-layer of this formation
    double id = (double)m_formationIdMap[(*formations)[i]->getName()];
    for (int j = 0; j < (int)depthMap->getDepth() - 1; ++j)
      formationIds.push_back(id);
  }

  SoGroup* group = new SoGroup;

  if (!depthMaps.empty())
  {
    //std::shared_ptr<SnapshotGeometry> geometry(new SnapshotGeometry(depthMaps));
    //std::shared_ptr<ChunkTopology> topology(new ChunkTopology(geometry, 0, geometry->numK() - 1));
    //HexahedronMesh* meshData = new HexahedronMesh(geometry, topology);

    //MoMesh* mesh = new MoMesh;
    //mesh->setMesh(meshData);

    //MoScalarSetIjk* scalarSet = new MoScalarSetIjk;
    //scalarSet->setScalarSet(new FormationIdProperty(formationIds));

    //MoMeshSkin* meshSkin = new MoMeshSkin;

    //group->addChild(mesh);
    //group->addChild(scalarSet);
    //group->addChild(meshSkin);
  }

  return group;
}

void SceneGraph::createSnapshotsNode(di::ProjectHandle* handle)
{
  // Setup formation id lookup
  std::unique_ptr<di::FormationList> formations(handle->getFormations(0, false));
  for (size_t i = 0; i < formations->size(); ++i)
    m_formationIdMap[(*formations)[i]->getName()] = (int)i;

  std::cout << "Creating snapshots" << std::endl;

  int flags = di::FORMATION;// | di::SURFACE | di::FORMATIONSURFACE;
  int type  = di::VOLUME;

  std::string depthKey = "Depth";
  const di::Property* depthProperty = handle->findProperty(depthKey);

  m_snapshots = new SoSwitch;

  std::unique_ptr<di::SnapshotList> snapshots(handle->getSnapshots(di::MAJOR));

  // Sort snapshots so oldest is first in list
  std::vector<const di::Snapshot*> tmpSnapshotList(*snapshots);
  std::sort(tmpSnapshotList.begin(), tmpSnapshotList.end(), SnapshotCompare());

  for(size_t i=0; i < tmpSnapshotList.size(); ++i)
  {
    const di::Snapshot* snapshot = tmpSnapshotList[i];
    m_snapshots->addChild(createSnapshotNode(handle, snapshot));
    
    //SnapshotNode* snapshotNode = new SnapshotNode;
    //snapshotNode->setup(snapshot);

    //// connect fields from scenegraph
    //snapshotNode->SliceI.connectFrom(&SliceI);
    //snapshotNode->SliceJ.connectFrom(&SliceJ);
    //snapshotNode->Plane.connectFrom(&Plane);

    //m_snapshots->addChild(snapshotNode);
  }

  m_snapshots->whichChild = 0;
  m_colorMap->maxValue = (float)(formations->size() - 1);

  std::cout << "Done creating snapshots" << std::endl;
}

void SceneGraph::createRootNode()
{
  m_verticalScale = new SoScale;
  m_verticalScale->scaleFactor = SbVec3f(1.0f, 1.0f, 1.0f);

  addChild(m_verticalScale);
  addChild(m_cellFilterSwitch);
  addChild(m_appearance);
  addChild(m_snapshots);

  m_planeManipSwitch->addChild(m_planeManip);
  m_planeManipSwitch->whichChild = SO_SWITCH_NONE;
  addChild(m_planeManipSwitch);

  Plane.connectFrom(&m_planeManip->plane);
}

/**
 * Initializes the object that allows the user to manipulate (rotate / translate) the cross section 
 */
void SceneGraph::initializeManip()
{
  // Get scene bounding box by sending an SoGetBoundingBoxAction down the tree. 
  // This needs a viewport region, but since we don't have any viewport dependent
  // elements, just use a dummy region
  SbViewportRegion dummyRegion;
  SoGetBoundingBoxAction gba(dummyRegion);
  gba.apply(this);

  SbBox3f bbox = gba.getBoundingBox();
  SbVec3f size   = bbox.getSize();
  SbVec3f center = bbox.getCenter();

  // Use this info to come up with an initial plane for the cross section
  SbVec3f normal(1.0f, 1.0f, 0.0f);
  normal.normalize();
  SbPlane plane(normal, center);

  // Put manipulator in the center of the scene
  m_planeManip->draggerPosition = center;
  m_planeManip->on = false;
  m_planeManip->plane = plane;

  // Set the size of the manipulator to 10% of mesh bounding box size
  float scale = .1f * size.length();
  SoJackDragger* dragger = dynamic_cast<SoJackDragger*>(m_planeManip->getDragger());
  dragger->scaleFactor = SbVec3f(scale, scale, scale);
}

SceneGraph::SceneGraph()
  : m_verticalScale(0)
  , m_cellFilterSwitch(0)
  , m_cellFilter(0)
  , m_roiFilter(0)
  , m_appearance(0)
  , m_drawStyle(0)
  , m_snapshots(0)
  , m_colorMap(0)
  , m_planeManipInitialized(false)
  , m_planeManipSwitch(new SoSwitch)
  , m_planeManip(new SoClipPlaneManip)
{
  SO_NODE_CONSTRUCTOR(SceneGraph);
  SO_NODE_ADD_FIELD(SliceI, (0));
  SO_NODE_ADD_FIELD(SliceJ, (0));
  SO_NODE_ADD_FIELD(Plane, (getDefaultPlane()));
}

SceneGraph::~SceneGraph()
{

}

void SceneGraph::setup(di::ProjectHandle* handle)
{
  const di::Grid* loresGrid = handle->getLowResolutionOutputGrid();
  const di::Grid* hiresGrid = handle->getHighResolutionOutputGrid();

  m_numI = loresGrid->numI();
  m_numJ = loresGrid->numJ();
  m_numIHiRes = hiresGrid->numI();
  m_numJHiRes = hiresGrid->numJ();

  createFilterNode();
  createAppearanceNode();
  createSnapshotsNode(handle);

  //m_extractor.start();

  createRootNode();
}

void SceneGraph::setProperty(const DataAccess::Interface::Property* prop)
{
  double globalMinVal = std::numeric_limits<double>::max();
  double globalMaxVal = -globalMinVal;

  for(int i=0; i < m_snapshots->getNumChildren(); ++i)
  {
    SnapshotNode* node = dynamic_cast<SnapshotNode*>(m_snapshots->getChild(i));
    if(node != 0)
    {
      node->setProperty(prop);

      double minVal = 0.0, maxVal = 0.0;
      node->getPropertyValueRange(minVal, maxVal);

      globalMaxVal = (maxVal > globalMaxVal) ? maxVal : globalMaxVal;
      globalMinVal = (minVal < globalMinVal) ? minVal : globalMinVal;
    }
  }

  m_colorMap->minValue = (float)globalMinVal;
  m_colorMap->maxValue = (float)globalMaxVal;
}

void SceneGraph::setVectorProperty(const DataAccess::Interface::Property* prop[3])
{
  for (int i = 0; i < m_snapshots->getNumChildren(); ++i)
  {
    SnapshotNode* node = dynamic_cast<SnapshotNode*>(m_snapshots->getChild(i));
    if (node != 0)
      node->setVectorProperty(prop);
  }
}

int SceneGraph::snapshotCount() const
{
  return m_snapshots->getNumChildren();
}

const di::Snapshot* SceneGraph::getSnapshot(int index) const
{
  return ((SnapshotNode*)m_snapshots->getChild(index))->getSnapShot();
}

void SceneGraph::setCurrentSnapshot(int index)
{
  m_snapshots->whichChild = index;
}

void SceneGraph::showPlaneManip(bool show)
{
  if(show && !m_planeManipInitialized)
  {
    initializeManip();
    m_planeManipInitialized = true;
  }

  m_planeManipSwitch->whichChild = show ? SO_SWITCH_ALL : SO_SWITCH_NONE;
}

void SceneGraph::enableROI(bool enable)
{
  m_cellFilterSwitch->whichChild = enable ? SO_SWITCH_ALL : SO_SWITCH_NONE;
}

void SceneGraph::setROI(size_t minI, size_t minJ, size_t minK, size_t maxI, size_t maxJ, size_t maxK)
{
  m_roiFilter->setROI(minI, minJ, minK, maxI, maxJ, maxK);
  m_cellFilter->touch();
}

void SceneGraph::setVerticalScale(float scale)
{
  m_verticalScale->scaleFactor = SbVec3f(1.0f, 1.0f, scale);
}

void SceneGraph::setRenderStyle(bool drawFaces, bool drawEdges)
{
  m_drawStyle->displayFaces = drawFaces;
  m_drawStyle->displayEdges = drawEdges;
}

void SceneGraph::getRenderStyle(bool& drawFaces, bool& drawEdges)
{
  drawFaces = m_drawStyle->displayFaces.getValue();
  drawEdges = m_drawStyle->displayEdges.getValue();
}

void SceneGraph::setFormationVisibility(const std::string& name, bool visible)
{
  for (int i = 0; i < m_snapshots->getNumChildren(); ++i)
    static_cast<SnapshotNode*>(m_snapshots->getChild(i))->setFormationVisibility(name, visible);
}

void SceneGraph::setSurfaceVisibility(const std::string& name, bool visible)
{
  for (int i = 0; i < m_snapshots->getNumChildren(); ++i)
    static_cast<SnapshotNode*>(m_snapshots->getChild(i))->setSurfaceVisibility(name, visible);
}

void SceneGraph::setReservoirVisibility(const std::string& name, bool visible)
{
  for (int i = 0; i < m_snapshots->getNumChildren(); ++i)
    static_cast<SnapshotNode*>(m_snapshots->getChild(i))->setReservoirVisibility(name, visible);
}

int SceneGraph::numI() const
{
  return (int)m_numI;
}

int SceneGraph::numJ() const
{
  return (int)m_numJ;
}

int SceneGraph::numIHiRes() const
{
  return (int)m_numIHiRes;
}

int SceneGraph::numJHiRes() const
{
  return (int)m_numJHiRes;
}

void BpaVizInit()
{
  SnapshotNode::initClass();
  SceneGraph::initClass();
}

void BpaVizFinish()
{
  SnapshotNode::exitClass();
  SceneGraph::exitClass();
}

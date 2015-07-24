
#include "SceneGraph.h"
#include "SnapshotNode.h"
#include "Mesh.h"
#include "BpaMesh.h"
#include "ROICellFilter.h"
#include "SkinExtractor.h"

// DataAccess
#include "Interface/Grid.h"
#include "Interface/Snapshot.h"
#include "Interface/Property.h"

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
  binding->dataBinding = MoDataBinding::PER_NODE;

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

void SceneGraph::createSnapshotsNode(di::ProjectHandle* handle)
{
  int flags = di::FORMATION;// | di::SURFACE | di::FORMATIONSURFACE;
  int type  = di::VOLUME;

  std::string depthKey = "Depth";
  const di::Property* depthProperty = handle->findProperty(depthKey);

  m_snapshots = new SoSwitch;

  std::shared_ptr<di::SnapshotList> snapshots(handle->getSnapshots(di::MAJOR));

  // Sort snapshots so oldest is first in list
  std::vector<const di::Snapshot*> tmpSnapshotList(*snapshots);
  std::sort(tmpSnapshotList.begin(), tmpSnapshotList.end(), SnapshotCompare());

  for(size_t i=0; i < tmpSnapshotList.size(); ++i)
  {
    const di::Snapshot* snapshot = tmpSnapshotList[i];
    
    SnapshotNode* snapshotNode = new SnapshotNode;
    snapshotNode->setup(snapshot);

    // connect fields from scenegraph
    snapshotNode->RenderMode.connectFrom(&RenderMode);
    snapshotNode->SliceI.connectFrom(&SliceI);
    snapshotNode->SliceJ.connectFrom(&SliceJ);
    snapshotNode->Plane.connectFrom(&Plane);

    m_snapshots->addChild(snapshotNode);
  }

  m_snapshots->whichChild = 0;
}

void SceneGraph::createRootNode()
{
  m_verticalScale = new SoScale;
  m_verticalScale->scaleFactor = SbVec3f(1.0f, 1.0f, 1.0f);

  addChild(m_verticalScale);
  addChild(m_cellFilterSwitch);
  addChild(m_appearance);
  
  m_resolutionSwitch = new SoSwitch;
  m_resolutionSwitch->addChild(m_snapshots);
  m_resolutionSwitch->whichChild = 0;
  addChild(m_resolutionSwitch);

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
  , m_resolutionSwitch(0)
  , m_colorMap(0)
  , m_planeManipInitialized(false)
  , m_planeManipSwitch(new SoSwitch)
  , m_planeManip(new SoClipPlaneManip)
{
  SO_NODE_CONSTRUCTOR(SceneGraph);
  SO_NODE_ADD_FIELD(RenderMode, (0));
  SO_NODE_ADD_FIELD(SliceI, (0));
  SO_NODE_ADD_FIELD(SliceJ, (0));
  SO_NODE_ADD_FIELD(Plane, (getDefaultPlane()));
}

SceneGraph::~SceneGraph()
{

}

void SceneGraph::setup(di::ProjectHandle* handle, size_t subdivision)
{
  m_subdivision = subdivision;

  const di::Grid* loresGrid = handle->getLowResolutionOutputGrid();
  const di::Grid* hiresGrid = handle->getHighResolutionOutputGrid();

  m_numI = loresGrid->numI();
  m_numJ = loresGrid->numJ();
  m_numIHiRes = hiresGrid->numI();
  m_numJHiRes = hiresGrid->numJ();

  createFilterNode();
  createAppearanceNode();
  std::cout << "Creating snapshots"<< std::endl;
  createSnapshotsNode(handle);
  std::cout << "Done creating snapshots"<< std::endl;

  m_extractor.start();

  createRootNode();
}

void SceneGraph::setProperty(const DataAccess::Interface::Property* prop, SoSwitch* snapshots)
{
  double globalMinVal = std::numeric_limits<double>::max();
  double globalMaxVal = -globalMinVal;

  for(int i=0; i < snapshots->getNumChildren(); ++i)
  {
    SnapshotNode* node = dynamic_cast<SnapshotNode*>(snapshots->getChild(i));
    if(node != 0)
    {
      node->setProperty(prop);

      double minVal = node->scalarSet()->getScalarSet()->getMin();
      double maxVal = node->scalarSet()->getScalarSet()->getMax();

      globalMaxVal = (maxVal > globalMaxVal) ? maxVal : globalMaxVal;
      globalMinVal = (minVal < globalMinVal) ? minVal : globalMinVal;
    }
  }

  m_colorMap->minValue = (float)globalMinVal;
  m_colorMap->maxValue = (float)globalMaxVal;
}

void SceneGraph::setProperty(const DataAccess::Interface::Property* prop)
{
  if (prop->getType() == di::RESERVOIRPROPERTY)
    ;// setProperty(prop, m_snapshotsHiRes);
  else
    setProperty(prop, m_snapshots);
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

void SceneGraph::setMeshMode(MeshMode mode)
{
  int child = 0;

  switch (mode)
  {
  case MeshMode_All:        child = 0; break;
  case MeshMode_Reservoirs: child = 1; break;
  case MeshMode_Surfaces:   child = 2; break;
  }

  m_resolutionSwitch->whichChild = child;
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
  return (int)(m_numI * m_subdivision);
}

int SceneGraph::numJ() const
{
  return (int)(m_numJ * m_subdivision);
}

int SceneGraph::numIHiRes() const
{
  return (int)(m_numIHiRes * m_subdivision);
}

int SceneGraph::numJHiRes() const
{
  return (int)(m_numJHiRes * m_subdivision);
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

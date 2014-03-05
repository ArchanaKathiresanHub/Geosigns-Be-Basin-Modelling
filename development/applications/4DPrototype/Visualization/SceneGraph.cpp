#include "SceneGraph.h"
#include "BpaMesh.h"

// DataAccess
#include "Interface/Grid.h"
#include "Interface/Snapshot.h"
#include "Interface/ProjectHandle.h"

// OIV
#include <Inventor/nodes/SoNode.h>
#include <Inventor/nodes/SoGroup.h>
#include <Inventor/nodes/SoSwitch.h>
#include <Inventor/nodes/SoShapeHints.h>

// meshviz nodes
#include <MeshVizInterface/mapping/nodes/MoMesh.h>
#include <MeshVizInterface/mapping/nodes/MoMeshSkin.h>
#include <MeshVizInterface/mapping/nodes/MoMeshLogicalSlice.h>
#include <MeshVizInterface/mapping/nodes/MoMeshPlaneSlice.h>
#include <MeshVizInterface/mapping/nodes/MoMaterial.h>
#include <MeshVizInterface/mapping/nodes/MoDrawStyle.h>
#include <MeshVizInterface/mapping/nodes/MoScalarSetI.h>
#include <MeshVizInterface/mapping/nodes/MoDataBinding.h>
#include <MeshVizInterface/mapping/nodes/MoPredefinedColorMapping.h>

namespace di = DataAccess::Interface; 

SO_NODE_SOURCE(SnapshotNode);

void SnapshotNode::initClass()
{
  SO_NODE_INIT_CLASS(SnapshotNode, SoSeparator, "Separator");
}

SnapshotNode::SnapshotNode()
  : m_snapshot(0)
  , m_mesh(new MoMesh)
  , m_scalarSet(new MoScalarSetI)
  , m_skin(new MoMeshSkin)
  , m_sliceI(new MoMeshLogicalSlice)
  , m_sliceJ(new MoMeshLogicalSlice)
  , m_planeSlice(new MoMeshPlaneSlice)
  , m_sliceGroup(new SoGroup)
  , m_renderSwitch(new SoSwitch)
{
  SO_NODE_CONSTRUCTOR(SnapshotNode);
  SO_NODE_ADD_FIELD(RenderMode, (0));
  SO_NODE_ADD_FIELD(SliceI, (0));
  SO_NODE_ADD_FIELD(SliceJ, (0));

  m_renderSwitch->whichChild.connectFrom(&RenderMode);
  m_sliceI->sliceIndex.connectFrom(&SliceI);
  m_sliceJ->sliceIndex.connectFrom(&SliceJ);
  m_planeSlice->plane.connectFrom(&Plane);
}

void SnapshotNode::setup(const di::Snapshot* snapshot, std::shared_ptr<di::PropertyValueList> depthValues)
{
  m_snapshot = snapshot;

  di::ProjectHandle* handle = snapshot->getProjectHandle();
  const di::Grid* grid = handle->getLowResolutionOutputGrid();

  BpaMesh* bpaMesh = new BpaMesh(grid, depthValues);
  m_mesh->setMesh(bpaMesh);

  // This is necessary to enable double-sided lighting on slices
  SoShapeHints* shapeHints = new SoShapeHints;
  shapeHints->shapeType = SoShapeHints::UNKNOWN_SHAPE_TYPE;
  shapeHints->vertexOrdering = SoShapeHints::COUNTERCLOCKWISE;

  m_sliceI->sliceAxis = MoMeshLogicalSlice::SLICE_I;
  m_sliceJ->sliceAxis = MoMeshLogicalSlice::SLICE_J;

  addChild(m_scalarSet);
  addChild(m_mesh);

  m_sliceGroup->addChild(shapeHints);
  m_sliceGroup->addChild(m_sliceI);
  m_sliceGroup->addChild(m_sliceJ);

  m_renderSwitch->addChild(m_skin);
  m_renderSwitch->addChild(m_sliceGroup);
  m_renderSwitch->whichChild = 0;

  addChild(m_renderSwitch);
}

void SnapshotNode::setProperty(const di::Property* prop)
{
  di::ProjectHandle* handle = m_snapshot->getProjectHandle();
  const di::Grid* grid = handle->getLowResolutionOutputGrid();

  // Extract property values for this snapshot
  int flags = di::FORMATION;
  int type  = di::VOLUME;
  std::shared_ptr<di::PropertyValueList> values(
    handle->getPropertyValues(flags, prop, m_snapshot, 0, 0, 0, type));

  BpaProperty* bpaProperty = new BpaProperty(grid->numI(), grid->numJ(), values);

  const MiScalardSetI* scalarSet = m_scalarSet->getScalarSet();
  delete scalarSet; // not reference counted, so delete it ourselves!!

  m_scalarSet->setScalarSet(bpaProperty);
  m_scalarSet->touch();

  m_skin->colorScalarSetId = 0;
}

MoScalarSetI* SnapshotNode::scalarSet() const
{
  return m_scalarSet;
}

SO_NODE_SOURCE(SceneGraph);

void SceneGraph::initClass()
{
  SO_NODE_INIT_CLASS(SceneGraph, SoGroup, "Group");
}

void SceneGraph::createAppearanceNode()
{
  m_colorMap = new MoPredefinedColorMapping;
  m_colorMap->predefColorMap = MoPredefinedColorMapping::STANDARD;
  m_colorMap->minValue = 0.0f;
  m_colorMap->maxValue = 100.0f;

  MoDrawStyle* drawStyle = new MoDrawStyle;
  drawStyle->displayFaces = true;
  drawStyle->displayEdges = true;
  drawStyle->displayPoints = false;

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
  m_appearance->addChild(drawStyle);
  m_appearance->addChild(material);
  m_appearance->addChild(binding);
}

void SceneGraph::createSnapshotsNode(di::ProjectHandle* handle)
{
  int flags = di::FORMATION;
  int type  = di::VOLUME;

  const di::Property* depthProperty = handle->findProperty("Depth");
  const di::Grid* grid = handle->getLowResolutionOutputGrid();

  m_snapshots = new SoSwitch;
  
  di::SnapshotList* snapshots = handle->getSnapshots(di::MAJOR);
  for(size_t i=0; i < snapshots->size(); ++i)
  {
    const di::Snapshot* snapshot = (*snapshots)[i];

    std::shared_ptr<di::PropertyValueList> depthValues(
      handle->getPropertyValues(flags, depthProperty, snapshot, 0, 0, 0, type));

    SnapshotNode* snapshotNode = new SnapshotNode;
    snapshotNode->setup(snapshot, depthValues);
    // connect fields from scenegraph
    snapshotNode->RenderMode.connectFrom(&RenderMode);
    snapshotNode->SliceI.connectFrom(&SliceI);
    snapshotNode->SliceJ.connectFrom(&SliceJ);
    snapshotNode->Plane.connectFrom(&Plane);

    m_snapshots->insertChild(snapshotNode, 0);
  }

  m_snapshots->whichChild = 0;
}

void SceneGraph::createRootNode()
{
  addChild(m_appearance);
  addChild(m_snapshots);
}

SceneGraph::SceneGraph()
  : m_appearance(0)
  , m_snapshots(0)
  , m_colorMap(0)
{
  SO_NODE_CONSTRUCTOR(SceneGraph);
  SO_NODE_ADD_FIELD(RenderMode, (0));
  SO_NODE_ADD_FIELD(SliceI, (0));
  SO_NODE_ADD_FIELD(SliceJ, (0));
}

void SceneGraph::setup(di::ProjectHandle* handle)
{
  createAppearanceNode();
  createSnapshotsNode(handle);
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

      double minVal = node->scalarSet()->getScalarSet()->getMin();
      double maxVal = node->scalarSet()->getScalarSet()->getMax();

      globalMaxVal = (maxVal > globalMaxVal) ? maxVal : globalMaxVal;
      globalMinVal = (minVal < globalMinVal) ? minVal : globalMinVal;
    }
  }

  m_colorMap->minValue = (float)globalMinVal;
  m_colorMap->maxValue = (float)globalMaxVal;
}

int SceneGraph::snapshotCount() const
{
  return m_snapshots->getNumChildren();
}

void SceneGraph::setCurrentSnapshot(int index)
{
  m_snapshots->whichChild = index;
}

void BpaVizInit()
{
  SnapshotNode::initClass();
  SceneGraph::initClass();
}

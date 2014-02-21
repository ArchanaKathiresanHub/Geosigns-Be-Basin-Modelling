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

// meshviz nodes
#include <MeshVizInterface/mapping/nodes/MoMesh.h>
#include <MeshVizInterface/mapping/nodes/MoMeshSkin.h>
#include <MeshVizInterface/mapping/nodes/MoMaterial.h>
#include <MeshVizInterface/mapping/nodes/MoDrawStyle.h>
#include <MeshVizInterface/mapping/nodes/MoScalarSetI.h>
#include <MeshVizInterface/mapping/nodes/MoDataBinding.h>
#include <MeshVizInterface/mapping/nodes/MoPredefinedColorMapping.h>

namespace di = DataAccess::Interface; 

SnapshotNode::SnapshotNode(const di::Snapshot* snapshot, std::shared_ptr<di::PropertyValueList> depthValues)
  : m_snapshot(snapshot)
  , m_mesh(new MoMesh)
  , m_scalarSet(new MoScalarSetI)
  , m_skin(new MoMeshSkin)
{
  di::ProjectHandle* handle = snapshot->getProjectHandle();
  const di::Grid* grid = handle->getLowResolutionOutputGrid();

  BpaMesh* bpaMesh = new BpaMesh(grid, depthValues);
  m_mesh->setMesh(bpaMesh);

  addChild(m_scalarSet);
  addChild(m_mesh);
  addChild(m_skin);
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

    SnapshotNode* snapshotNode = new SnapshotNode(snapshot, depthValues);
    m_snapshots->insertChild(snapshotNode, 0);
  }

  m_snapshots->whichChild = 0;
}

void SceneGraph::createRootNode()
{
  m_root = new SoGroup;
  m_root->addChild(m_appearance);
  m_root->addChild(m_snapshots);
}

SceneGraph::SceneGraph(di::ProjectHandle* handle)
  : m_root(0)
  , m_appearance(0)
  , m_snapshots(0)
  , m_colorMap(0)
{
  createAppearanceNode();
  createSnapshotsNode(handle);
  createRootNode();
}

SoGroup* SceneGraph::root() const
{
  return m_root;
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

#include "SnapshotNode.h"
#include "BpaMesh.h"

// DataAccess
#include "Interface/Grid.h"
#include "Interface/GridMap.h"
#include "Interface/Snapshot.h"
#include "Interface/Reservoir.h"
#include "Interface/Property.h"
#include "Interface/ProjectHandle.h"
#include "Interface/PropertyValue.h"

// OIV
#include <Inventor/nodes/SoNode.h>
#include <Inventor/nodes/SoGroup.h>
#include <Inventor/nodes/SoScale.h>
#include <Inventor/nodes/SoSwitch.h>
#include <Inventor/nodes/SoShapeHints.h>
#include <Inventor/manips/SoClipPlaneManip.h>
#include <Inventor/draggers/SoJackDragger.h>

// MeshVizXLM
#include <MeshVizInterface/mapping/nodes/MoMesh.h>
#include <MeshVizInterface/mapping/nodes/MoMeshSkin.h>
#include <MeshVizInterface/mapping/nodes/MoMeshLogicalSlice.h>
#include <MeshVizInterface/mapping/nodes/MoMeshPlaneSlice.h>
#include <MeshVizInterface/mapping/nodes/MoMeshOutline.h>
#include <MeshVizInterface/mapping/nodes/MoMaterial.h>
#include <MeshVizInterface/mapping/nodes/MoDrawStyle.h>
#include <MeshVizInterface/mapping/nodes/MoScalarSetI.h>
#include <MeshVizInterface/mapping/nodes/MoDataBinding.h>
#include <MeshVizInterface/mapping/nodes/MoPredefinedColorMapping.h>
#include <MeshVizInterface/mapping/nodes/MoCellFilter.h>
#include <MeshVizInterface/mapping/nodes/MoMeshSurface.h>

#include <MeshVizInterface/extractors/MiSkinExtractUnstructuredIjk.h>

namespace di = DataAccess::Interface;

namespace
{

  SbPlane getDefaultPlane()
  {
    SbPlane plane(SbVec3f(0.0f, 1.0f, 0.0f), -10.0f);
    return plane;
  }

}

SO_NODE_SOURCE(SnapshotNode);

void SnapshotNode::initClass()
{
  SO_NODE_INIT_CLASS(SnapshotNode, SoSeparator, "Separator");
}

void SnapshotNode::exitClass()
{
  SO__NODE_EXIT_CLASS(SnapshotNode);
}

SnapshotNode::SnapshotNode()
: m_snapshot(0)
, m_mesh(new MoMesh)
, m_scalarSet(new MoScalarSetI)
, m_skin(new MoMeshSkin)
, m_sliceI(new MoMeshLogicalSlice)
, m_sliceJ(new MoMeshLogicalSlice)
, m_sliceGroup(new SoGroup)
, m_planeSlice(new MoMeshPlaneSlice)
, m_outline(new MoMeshOutline)
, m_planeGroup(new SoGroup)
, m_renderSwitch(new SoSwitch)
, m_skinExtractor(0)
{
  SO_NODE_CONSTRUCTOR(SnapshotNode);
  SO_NODE_ADD_FIELD(RenderMode, (0));
  SO_NODE_ADD_FIELD(SliceI, (0));
  SO_NODE_ADD_FIELD(SliceJ, (0));
  SO_NODE_ADD_FIELD(Plane, (getDefaultPlane()));

  m_renderSwitch->whichChild.connectFrom(&RenderMode);
  m_sliceI->sliceIndex.connectFrom(&SliceI);
  m_sliceJ->sliceIndex.connectFrom(&SliceJ);
  m_planeSlice->plane.connectFrom(&Plane);
}

void SnapshotNode::setup(const di::Snapshot* snapshot, std::shared_ptr<di::PropertyValueList> depthValues, bool hires, Extractor& extractor, size_t subdivision)
{
  m_snapshot = snapshot;
  m_subdivision = subdivision;

  di::ProjectHandle* handle = snapshot->getProjectHandle();

  const di::Grid* grid;
  if (hires)
    grid = handle->getHighResolutionOutputGrid();
  else
    grid = handle->getLowResolutionOutputGrid();

  BpaMesh* bpaMesh = new BpaMesh(grid, depthValues, m_subdivision);
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

#ifdef PRE_EXTRACT
  //std::cout << "Extracting skin" << std::endl;

  MoMesh* skinMesh = new MoMesh;

  MoMeshSurface* meshSurface = new MoMeshSurface;
  meshSurface->colorScalarSetId = 0;

  SoGroup* skinGroup = new SoGroup;
  skinGroup->addChild(skinMesh);
  skinGroup->addChild(meshSurface);
  m_renderSwitch->addChild(skinGroup);

#ifndef CUSTOM_EXTRACTION
  MiSkinExtractUnstructuredIjk* oivExtractor = MiSkinExtractUnstructuredIjk::getNewInstance(*bpaMesh);
  const MeXSurfaceMeshUnstructured& surfaceMesh = oivExtractor->extractSkin();
  skinMesh->setMesh(&surfaceMesh);
  std::cout << '.' << std::flush;
#else
  //m_skinExtractor = new SkinExtractor(*bpaMesh);
  //const MiSurfaceMeshUnstructured& surfaceMesh = m_skinExtractor->extractSkin(0);
  Extractor::WorkItem workItem;
  workItem.bpaMesh = bpaMesh;
  workItem.parentMesh = skinMesh;
  extractor.put(workItem);
#endif

#else
  m_renderSwitch->addChild(m_skin);
#endif

  m_renderSwitch->addChild(m_sliceGroup);

  m_planeGroup->addChild(m_outline);
  m_planeGroup->addChild(m_planeSlice);
  m_renderSwitch->addChild(m_planeGroup);

  m_renderSwitch->whichChild = 0;

  addChild(m_renderSwitch);
}

const DataAccess::Interface::Snapshot* SnapshotNode::getSnapShot() const
{
  return m_snapshot;
}

void SnapshotNode::setProperty(const di::Property* prop)
{
  di::PropertyType propType = prop->getType();

  di::ProjectHandle* handle = m_snapshot->getProjectHandle();

  const di::Grid* grid;
  int flags;
  int type;

  if (propType == di::RESERVOIRPROPERTY)
  {
    grid = handle->getHighResolutionOutputGrid();
    flags = di::RESERVOIR;
    type = di::MAP;
  }
  else
  {
    grid = handle->getLowResolutionOutputGrid();
    flags = di::FORMATION;
    type = di::VOLUME;
  }

  std::shared_ptr<di::PropertyValueList> values(
    handle->getPropertyValues(flags, prop, m_snapshot, 0, 0, 0, type));

  BpaProperty* bpaProperty = new BpaProperty(grid->numI(), grid->numJ(), values, m_subdivision);

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

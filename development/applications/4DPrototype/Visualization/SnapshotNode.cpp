#include "SnapshotNode.h"
#include "BpaMesh.h"
#include "Mesh.h"
#include "Property.h"

// DataAccess
#include "Interface/Grid.h"
#include "Interface/GridMap.h"
#include "Interface/Snapshot.h"
#include "Interface/Formation.h"
#include "Interface/Surface.h"
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
#include <MeshVizInterface/mapping/nodes/MoMeshVector.h>
#include <MeshVizInterface/mapping/nodes/MoMeshLogicalSlice.h>
#include <MeshVizInterface/mapping/nodes/MoMeshPlaneSlice.h>
#include <MeshVizInterface/mapping/nodes/MoMeshOutline.h>
#include <MeshVizInterface/mapping/nodes/MoMaterial.h>
#include <MeshVizInterface/mapping/nodes/MoDrawStyle.h>
#include <MeshVizInterface/mapping/nodes/MoScalarSetIjk.h>
#include <MeshVizInterface/mapping/nodes/MoVec3SetIjk.h>
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
, m_scalarSet(new MoScalarSetIjk)
, m_skin(new MoMeshSkin)
, m_sliceI(new MoMeshLogicalSlice)
, m_sliceJ(new MoMeshLogicalSlice)
, m_sliceGroup(new SoGroup)
, m_planeSlice(new MoMeshPlaneSlice)
, m_outline(new MoMeshOutline)
, m_planeGroup(new SoGroup)
, m_renderSwitch(new SoSwitch)
, m_skinExtractor(0)
, m_formationsGroup(new SoGroup)
, m_surfacesGroup(new SoGroup)
, m_reservoirsGroup(new SoGroup)
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

  m_surfacesGroup->setName("surfaces");
  m_formationsGroup->setName("formations");
  m_reservoirsGroup->setName("reservoirs");
}

void SnapshotNode::setup(const di::Snapshot* snapshot)
{
  m_snapshot = snapshot;

  di::ProjectHandle* handle = snapshot->getProjectHandle();

  const std::string depthKey = "Depth";
  const std::string depthTopKey = "ResRockTop";
  const std::string depthBottomKey = "ResRockBottom";

  const di::Property* depthProperty = handle->findProperty(depthKey);

  bool includeBasement = false;

  // SURFACES

  std::unique_ptr<di::SurfaceList> surfaces(handle->getSurfaces(snapshot, includeBasement));
  for (auto surface : *surfaces)
  {
    std::shared_ptr<di::PropertyValueList> depthValues(
      handle->getPropertyValues(di::SURFACE, depthProperty, snapshot, nullptr, nullptr, surface, di::SURFACE));
    assert(depthValues->size() <= 1);
    if (depthValues && !depthValues->empty())
    {
      SurfaceMesh* mesh = new SurfaceMesh((*depthValues)[0]->getGridMap());
      MoMesh* meshNode = new MoMesh;
      meshNode->setMesh(mesh);

      MoMeshSurface* surfaceMesh = new MoMeshSurface;

      SoSwitch* group = new SoSwitch;
      group->addChild(meshNode);
      group->addChild(surfaceMesh);
      group->whichChild = SO_SWITCH_NONE;

      m_surfacesMap[surface->getName()] = group;

      m_surfacesGroup->addChild(group);
    }
  }
  addChild(m_surfacesGroup);

  // FORMATIONS

  // Create colormap for all formations
  std::unique_ptr<di::FormationList> allFormations(handle->getFormations(nullptr, includeBasement));
  std::map<std::string, SbColor> colorMap;
  for (size_t i = 0; i < allFormations->size(); ++i)
  {
    SbColor color;
    float hue = (float)i / (float)allFormations->size();
    color.setHSVValue(hue, 1.f, 1.f);
    std::string name = (*allFormations)[i]->getName();
    colorMap[name] = color;
  }

  std::unique_ptr<di::FormationList> formations(handle->getFormations(snapshot, includeBasement));
  for (auto formation : *formations)
  {
    std::shared_ptr<di::PropertyValueList> depthValues(
      handle->getPropertyValues(di::FORMATION, depthProperty, snapshot, nullptr, formation, nullptr, di::VOLUME));
    assert(depthValues->size() <= 1);
    if (depthValues && !depthValues->empty())
    {
      FormationMesh* mesh = new FormationMesh((*depthValues)[0]->getGridMap());
      MoMesh* meshNode = new MoMesh;
      meshNode->setMesh(mesh);

      MoMaterial* material = new MoMaterial;
      material->faceColoring = MoMaterial::COLOR;
      material->faceColor = colorMap[formation->getName()];
      material->lineColoring = MoMaterial::COLOR;
      material->lineColor = SbColor(0.0f, 0.0f, 0.0f);

      MoScalarSetIjk* scalarSet = new MoScalarSetIjk;
      MoVec3SetIjk* vectorSet = new MoVec3SetIjk;

      MoMeshSkin* meshSkin = new MoMeshSkin;
      MoMeshVector* meshVector = new MoMeshVector;
      SoSwitch* vectorSwitch = new SoSwitch;
      vectorSwitch->addChild(meshVector);
      vectorSwitch->whichChild = SO_SWITCH_NONE;

      SoSeparator* sep = new SoSeparator;
      sep->addChild(meshNode);
      sep->addChild(material);
      sep->addChild(scalarSet);
      sep->addChild(vectorSet);
      sep->addChild(meshSkin);
      sep->addChild(vectorSwitch);

      SoSwitch* group = new SoSwitch;
      group->whichChild = SO_SWITCH_ALL;
      group->addChild(sep);

      m_formationsMap[formation->getName()] = group;

      m_formationsGroup->addChild(group);
    }
  }
  addChild(m_formationsGroup);

  // RESERVOIRS

  const di::Property* depthTopProperty = handle->findProperty(depthTopKey);
  const di::Property* depthBottomProperty = handle->findProperty(depthBottomKey);

  for (auto formation : *formations)
  {
    std::unique_ptr<di::ReservoirList> reservoirs(handle->getReservoirs(formation));

    for (auto reservoir : *reservoirs)
    {
      std::unique_ptr<di::PropertyValueList> depthTopValues(
        handle->getPropertyValues(di::RESERVOIR, depthTopProperty, snapshot, reservoir, 0, 0, di::MAP));

      std::shared_ptr<di::PropertyValueList> depthBottomValues(
        handle->getPropertyValues(di::RESERVOIR, depthBottomProperty, snapshot, reservoir, 0, 0, di::MAP));

      if (depthTopValues->size() == 1 && depthBottomValues->size() == 1)
      {
        ReservoirMesh* mesh = new ReservoirMesh((*depthTopValues)[0]->getGridMap(), (*depthBottomValues)[0]->getGridMap());
        MoMesh* meshNode = new MoMesh;
        meshNode->setMesh(mesh);

        MoMaterial* material = new MoMaterial;
        material->faceColoring = MoMaterial::COLOR;
        material->faceColor = SbColor(.3f, .3f, .3f);
        material->lineColoring = MoMaterial::COLOR;
        material->lineColor = SbColor(0.0f, 0.0f, 0.0f);

        MoMeshSkin* skinMesh = new MoMeshSkin;

        SoSwitch* group = new SoSwitch;
        group->addChild(meshNode);
        group->addChild(material);
        group->addChild(skinMesh);
        group->whichChild = SO_SWITCH_NONE;

        m_reservoirsMap[reservoir->getName()] = group;

        m_reservoirsGroup->addChild(group);
      }
    }
  }

  addChild(m_reservoirsGroup);
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

  // This is necessary to enable double-sided lighting on slices
  SoShapeHints* shapeHints = new SoShapeHints;
  shapeHints->shapeType = SoShapeHints::UNKNOWN_SHAPE_TYPE;
  shapeHints->vertexOrdering = SoShapeHints::COUNTERCLOCKWISE;

  /*
  BpaMesh* bpaMesh = new BpaMesh(grid, depthValues, m_subdivision);
  m_mesh->setMesh(bpaMesh);

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

  m_renderSwitch->whichChild = SO_SWITCH_NONE; // was 0

  addChild(m_renderSwitch);
*/

}

const DataAccess::Interface::Snapshot* SnapshotNode::getSnapShot() const
{
  return m_snapshot;
}

void SnapshotNode::setProperty(const di::Property* prop)
{
  di::ProjectHandle* handle = m_snapshot->getProjectHandle();

  const di::Grid* grid;
  int flags;
  int type;

  if (prop->getType() == di::RESERVOIRPROPERTY)
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

  std::unique_ptr<di::FormationList> formations(handle->getFormations(m_snapshot, false));
  for (auto formation : *formations)
  {
    auto iter = m_formationsMap.find(formation->getName());
    if (iter == m_formationsMap.end())
      continue;

    SoSeparator* sep = static_cast<SoSeparator*>(iter->second->getChild(0));
    MoScalarSetIjk* scalarSet = static_cast<MoScalarSetIjk*>(sep->getChild(2));
    const MiScalardSetIjk* prevDataSet = scalarSet->getScalarSet();
    delete prevDataSet;

    std::unique_ptr<di::PropertyValueList> values(handle->getPropertyValues(flags, prop, m_snapshot, nullptr, formation, nullptr, type));

    ScalarProperty* dataSet = nullptr;
    MoMaterial::ColoringType colorType = MoMaterial::COLOR;

    if (values->size() == 1)
    {
      dataSet = new ScalarProperty(prop->getName(), (*values)[0]->getGridMap());
      colorType = MoMaterial::CONTOURING;
    }

    MoMaterial* material = static_cast<MoMaterial*>(sep->getChild(1));
    material->faceColoring.setValue(colorType);

    scalarSet->setScalarSet(dataSet);
    scalarSet->touch();
  }
}

void SnapshotNode::setVectorProperty(const DataAccess::Interface::Property* prop[3])
{
  di::ProjectHandle* handle = m_snapshot->getProjectHandle();

  std::unique_ptr<di::FormationList> formations(handle->getFormations(m_snapshot, false));
  for (auto formation : *formations)
  {
    auto iter = m_formationsMap.find(formation->getName());
    if (iter == m_formationsMap.end())
      continue;

    SoSeparator* sep = static_cast<SoSeparator*>(iter->second->getChild(0));
    MoVec3SetIjk* vectorSet = static_cast<MoVec3SetIjk*>(sep->getChild(3));
    const MiVec3dSetIjk* prevDataSet = vectorSet->getVec3Set();
    delete prevDataSet;

    bool ok = true;

    const di::GridMap* gridMaps[3] = { 0, 0, 0 };
    for (int i = 0; ok && i < 3; ++i)
    {
      std::unique_ptr<di::PropertyValueList> values(
        handle->getPropertyValues(
        di::FORMATION,
        prop[i],
        m_snapshot,
        nullptr,
        formation,
        nullptr,
        di::VOLUME));

      if (values->size() == 1)
        gridMaps[i] = (*values)[0]->getGridMap();
      else
        ok = false;
    }

    if (ok)
    {
      VectorProperty* dataSet = new VectorProperty(prop[0]->getName(), gridMaps);
      vectorSet->setVec3Set(dataSet);
      vectorSet->touch();
    }

    SoSwitch* vectorSwitch = static_cast<SoSwitch*>(sep->getChild(5));
    vectorSwitch->whichChild = ok ? SO_SWITCH_ALL : SO_SWITCH_NONE;
  }
}

void SnapshotNode::setFormationVisibility(const std::string& name, bool visible)
{
  auto iter = m_formationsMap.find(name);
  if (iter != m_formationsMap.end())
    iter->second->whichChild = visible 
      ? SO_SWITCH_ALL 
      : SO_SWITCH_NONE;
}

void SnapshotNode::setSurfaceVisibility(const std::string& name, bool visible)
{
  auto iter = m_surfacesMap.find(name);
  if (iter != m_surfacesMap.end())
    iter->second->whichChild = visible
      ? SO_SWITCH_ALL
      : SO_SWITCH_NONE;
}

void SnapshotNode::setReservoirVisibility(const std::string& name, bool visible)
{
  auto iter = m_reservoirsMap.find(name);
  if (iter != m_reservoirsMap.end())
    iter->second->whichChild = visible
    ? SO_SWITCH_ALL
    : SO_SWITCH_NONE;
}

void SnapshotNode::getPropertyValueRange(double& minVal, double& maxVal) const
{
  for (int i = 0; i < m_formationsGroup->getNumChildren(); ++i)
  {
    SoGroup* group = static_cast<SoGroup*>(m_formationsGroup->getChild(i));
    SoSeparator* sep = static_cast<SoSeparator*>(group->getChild(0));
    MoScalarSetIjk* scalarSet = static_cast<MoScalarSetIjk*>(sep->getChild(2));
    const MiScalardSetIjk* dataSet = scalarSet->getScalarSet();
    if (dataSet)
    {
      minVal = dataSet->getMin();
      maxVal = dataSet->getMax();
    }
  }
}
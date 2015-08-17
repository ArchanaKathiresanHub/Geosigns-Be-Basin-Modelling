//
// Copyright (C) 2012-2015 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "SceneGraphManager.h"
#include "Mesh.h"
#include "Property.h"

#include <Inventor/nodes/SoGroup.h>
#include <Inventor/nodes/SoSwitch.h>
#include <Inventor/nodes/SoScale.h>
#include <Inventor/nodes/SoShapeHints.h>
#include <Inventor/nodes/SoAnnotation.h>

#include <MeshVizInterface/MxTimeStamp.h>
#include <MeshVizInterface/mapping/nodes/MoDrawStyle.h>
#include <MeshVizInterface/mapping/nodes/MoMaterial.h>
#include <MeshVizInterface/mapping/nodes/MoDataBinding.h>
#include <MeshVizInterface/mapping/nodes/MoPredefinedColorMapping.h>
#include <MeshVizInterface/mapping/nodes/MoMesh.h>
#include <MeshVizInterface/mapping/nodes/MoMeshSkin.h>
#include <MeshVizInterface/mapping/nodes/MoMeshSlab.h>
#include <MeshVizInterface/mapping/nodes/MoMeshSurface.h>
#include <MeshVizInterface/mapping/nodes/MoScalarSetIjk.h>
#include <MeshVizInterface/mapping/nodes/MoLegend.h>

#include <Interface/ProjectHandle.h>
#include <Interface/Grid.h>
#include <Interface/GridMap.h>
#include <Interface/Property.h>
#include <Interface/PropertyValue.h>
#include <Interface/Formation.h>
#include <Interface/Surface.h>
#include <Interface/Snapshot.h>

#include <memory>

namespace di = DataAccess::Interface;

SnapshotInfo::SnapshotInfo()
  : snapshot(0)
  , currentProperty(0)
  , root(0)
  , mesh(0)
  , meshData(0)
  , scalarSet(0)
  , formationIdProperty(0)
  , scalarProperty(0)
  , chunksGroup(0)
  , surfacesGroup(0)
  , slicesGroup(0)
  , formationsTimeStamp(MxTimeStamp::getTimeStamp())
  , surfacesTimeStamp(MxTimeStamp::getTimeStamp())
{
  for (int i = 0; i < 3; ++i)
  {
    sliceSwitch[i] = 0;
    slice[i] = 0;
  }
}

void SceneGraphManager::updateSnapshotFormations(size_t index)
{
  SnapshotInfo& snapshot = m_snapshots[index];


  // Update formations
  if (
    snapshot.formationsTimeStamp != m_formationsTimeStamp &&
    snapshot.chunksGroup != 0)
  {
    snapshot.chunksGroup->removeAllChildren();

    bool buildingChunk = false;
    int minK, maxK;

    std::vector<SnapshotInfo::Chunk> tmpChunks;

    for (size_t i = 0; i < snapshot.formations.size(); ++i)
    {
      int id = snapshot.formations[i].id;
      if (!buildingChunk && m_formations[id].visible)
      {
        buildingChunk = true;
        minK = snapshot.formations[i].minK;
      }
      else if (buildingChunk && !m_formations[id].visible)
      {
        buildingChunk = false;
        tmpChunks.push_back(SnapshotInfo::Chunk(minK, maxK));
      }

      maxK = snapshot.formations[i].maxK;
    }

    // don't forget the last one
    if (buildingChunk)
      tmpChunks.push_back(SnapshotInfo::Chunk(minK, maxK));

    for (size_t i = 0; i < tmpChunks.size(); ++i)
    {
      MoMeshSkin* meshSkin = new MoMeshSkin;
      uint32_t rangeMin[] = { 0, 0, tmpChunks[i].minK };
      uint32_t rangeMax[] = { m_numI - 1, m_numJ - 1, tmpChunks[i].maxK - 1 };
      meshSkin->minCellRanges.setValues(0, 3, rangeMin);
      meshSkin->maxCellRanges.setValues(0, 3, rangeMax);

      snapshot.chunksGroup->addChild(meshSkin);
    }

    snapshot.chunks.swap(tmpChunks);
    snapshot.formationsTimeStamp = m_formationsTimeStamp;
  }
}

void SceneGraphManager::updateSnapshotSurfaces(size_t index)
{
  SnapshotInfo& snapshot = m_snapshots[index];

  // Update surfaces
  if (snapshot.surfacesTimeStamp != m_surfacesTimeStamp)
  {
    for (size_t i = 0; i < snapshot.surfaces.size(); ++i)
    {
      int id = snapshot.surfaces[i].id;

      if (m_surfaces[id].visible && snapshot.surfaces[i].root == 0)
      {
        const di::Surface* surface = m_surfaces[id].surface;
        std::unique_ptr<di::PropertyValueList> values(m_projectHandle->getPropertyValues(
          di::SURFACE, m_depthProperty, snapshot.snapshot, 0, 0, surface, di::MAP));

        assert(values->size() == 1); //TODO: should not be an assert

        snapshot.surfaces[i].meshData = new SurfaceMesh((*values)[0]->getGridMap());
        snapshot.surfaces[i].mesh = new MoMesh;
        snapshot.surfaces[i].mesh->setMesh(snapshot.surfaces[i].meshData);
        snapshot.surfaces[i].surfaceMesh = new MoMeshSurface;

        SoSeparator* root = new SoSeparator;
        root->addChild(snapshot.surfaces[i].mesh);
        root->addChild(snapshot.surfaces[i].surfaceMesh);

        snapshot.surfaces[i].root = root;
        snapshot.surfacesGroup->addChild(root);
      }
      else
      {
        if (!m_surfaces[id].visible && snapshot.surfaces[i].root != 0)
        {
          // The meshData is not reference counted, need to delete this ourselves
          delete snapshot.surfaces[i].meshData;
          snapshot.surfacesGroup->removeChild(snapshot.surfaces[i].root);
          snapshot.surfaces[i].root = 0;
          snapshot.surfaces[i].mesh = 0;
          snapshot.surfaces[i].meshData = 0;
          snapshot.surfaces[i].surfaceMesh = 0;
        }
      }
    }

    snapshot.surfacesTimeStamp = m_surfacesTimeStamp;
  }
}

void SceneGraphManager::updateSnapshotProperties(size_t index)
{
  SnapshotInfo& snapshot = m_snapshots[index];

  // Update properties
  if (snapshot.currentProperty != m_currentProperty)
  {
    std::vector<const di::GridMap*> gridMaps;

    for (size_t i = 0; i < snapshot.formations.size(); ++i)
    {
      int id = snapshot.formations[i].id;
      const di::Formation* formation = m_formations[id].formation;

      std::unique_ptr<di::PropertyValueList> values(m_projectHandle->getPropertyValues(
        di::FORMATION, m_currentProperty, snapshot.snapshot, 0, formation, 0, di::VOLUME));

      assert(values->size() == 1); //TODO: change to exception

      gridMaps.push_back((*values)[0]->getGridMap());
    }

    if (gridMaps.empty())
      return;

    const MiScalardSetIjk* prevDataSet = snapshot.scalarSet->getScalarSet();
    delete prevDataSet;

    ScalarProperty* dataSet = new ScalarProperty(m_currentProperty->getName(), gridMaps);

    snapshot.scalarSet->setScalarSet(dataSet);
    snapshot.scalarSet->touch();

    static_cast<MoPredefinedColorMapping*>(m_colorMap)->minValue = (float)dataSet->getMin();
    static_cast<MoPredefinedColorMapping*>(m_colorMap)->maxValue = (float)dataSet->getMax();

    snapshot.currentProperty = m_currentProperty;
  }
}

void SceneGraphManager::updateSnapshotSlices(size_t index)
{
  SnapshotInfo& snapshot = m_snapshots[index];

  if (snapshot.slicesGroup == 0)
    return;

  for (int i = 0; i < 2; ++i)
  {
    if (m_sliceEnabled[i])
    {
      if (snapshot.slice[i] == 0)
      {
        auto slice = new MoMeshSlab;
        slice->dimension = MiMesh::DIMENSION_I + i;
        slice->thickness = 1;

        snapshot.sliceSwitch[i] = new SoSwitch;
        snapshot.sliceSwitch[i]->addChild(slice);
        snapshot.slice[i] = slice;

        snapshot.slicesGroup->addChild(snapshot.sliceSwitch[i]);
      }

      snapshot.slice[i]->index = (int)m_slicePosition[i];
    }

    if (snapshot.sliceSwitch[i] != 0)
    {
      snapshot.sliceSwitch[i]->whichChild = m_sliceEnabled[i]
        ? SO_SWITCH_ALL
        : SO_SWITCH_NONE;
    }
  }
}

void SceneGraphManager::updateSnapshot(size_t index)
{
  updateSnapshotFormations(index);
  updateSnapshotSurfaces(index);
  updateSnapshotProperties(index);
  updateSnapshotSlices(index);
}

SnapshotInfo SceneGraphManager::createSnapshotNode(const di::Snapshot* snapshot)
{
  SnapshotInfo info;
  info.snapshot = snapshot;

  std::vector<const di::GridMap*> depthMaps;
  std::vector<double> formationIds;

  int k = 0;

  std::unique_ptr<di::FormationList> formations(m_projectHandle->getFormations(snapshot, false));
  for (size_t i = 0; i < formations->size(); ++i)
  {
    const di::Formation* formation = (*formations)[i];

    std::unique_ptr<di::PropertyValueList> values(m_projectHandle->getPropertyValues(
      di::FORMATION, m_depthProperty, snapshot, 0, formation, 0, di::VOLUME));

    assert(values->size() == 1);

    const di::GridMap* depthMap = (*values)[0]->getGridMap();
    depthMaps.push_back(depthMap);

    // Depth of the formation in cells
    int depth = depthMap->getDepth() - 1;

    SnapshotInfo::Formation bounds;
    bounds.id = m_formationIdMap[formation->getName()];
    bounds.minK = k;
    bounds.maxK = k + depth;

    info.formations.push_back(bounds);

    k = bounds.maxK;

    // Add formation id for each k-layer of this formation
    for (int j = 0; j < depth; ++j)
      formationIds.push_back((double)bounds.id);
  }

  std::unique_ptr<di::SurfaceList> surfaces(m_projectHandle->getSurfaces(snapshot, false));
  for (size_t i = 0; i < surfaces->size(); ++i)
  {
    const di::Surface* surface = (*surfaces)[i];

    SnapshotInfo::Surface srfc;
    srfc.id = m_surfaceIdMap[surface->getName()];

    info.surfaces.push_back(srfc);
  }

  info.root = new SoSeparator;

  if (!depthMaps.empty())
  {
    std::shared_ptr<SnapshotGeometry> geometry(new SnapshotGeometry(depthMaps));
    std::shared_ptr<SnapshotTopology> topology(new SnapshotTopology(geometry));

    info.meshData = new HexahedronMesh(geometry, topology);

    info.mesh = new MoMesh;
    info.mesh->setMesh(info.meshData);

    info.formationIdProperty = new FormationIdProperty(formationIds);

    info.scalarSet = new MoScalarSetIjk;
    info.scalarSet->setName("formationID");
    info.scalarSet->setScalarSet(info.formationIdProperty);

    info.chunksGroup = new SoGroup;
    info.chunksGroup->setName("chunks");
    info.surfacesGroup = new SoGroup;
    info.surfacesGroup->setName("surfaces");
    info.slicesGroup = new SoGroup;
    info.slicesGroup->setName("slices");

    info.root->addChild(info.mesh);
    info.root->addChild(info.scalarSet);
    info.root->addChild(info.chunksGroup);
    info.root->addChild(info.slicesGroup);
    // Add surfaceShapeHints to prevent backface culling, and enable double-sided lighting
    info.root->addChild(m_surfaceShapeHints);
    info.root->addChild(info.surfacesGroup);
  }

  return info;
}

  struct SnapshotCompare
  {
    bool operator()(const di::Snapshot* s0, const di::Snapshot* s1) const
    {
      return s0->getTime() > s1->getTime();
    }
  };

void SceneGraphManager::setupSnapshots()
{
  std::unique_ptr<di::SnapshotList> snapshots(m_projectHandle->getSnapshots(di::MAJOR));

  // Sort snapshots so oldest is first in list
  std::vector<const di::Snapshot*> tmpSnapshotList(*snapshots);
  std::sort(tmpSnapshotList.begin(), tmpSnapshotList.end(), SnapshotCompare());

  for (size_t i = 0; i < tmpSnapshotList.size(); ++i)
  {
    const di::Snapshot* snapshot = tmpSnapshotList[i];
    SnapshotInfo info = createSnapshotNode(snapshot);
    m_snapshots.push_back(info);
    m_snapshotsSwitch->addChild(info.root);
  }

  m_snapshotsSwitch->whichChild = 0;
}

void SceneGraphManager::setupSceneGraph()
{
  // Backface culling is enabled for solid shapes with ordered vertices
  m_formationShapeHints = new SoShapeHints;
  m_formationShapeHints->shapeType = SoShapeHints::SOLID;
  m_formationShapeHints->vertexOrdering = SoShapeHints::COUNTERCLOCKWISE;

  // Double sided lighting is enabled for surfaces with ordered vertices
  m_surfaceShapeHints = new SoShapeHints;
  m_surfaceShapeHints->shapeType = SoShapeHints::UNKNOWN_SHAPE_TYPE;
  m_surfaceShapeHints->vertexOrdering = SoShapeHints::COUNTERCLOCKWISE;

  m_scale = new SoScale;
  m_scale->scaleFactor = SbVec3f(1.f, 1.f, 1.f);

  m_drawStyle = new MoDrawStyle;
  m_drawStyle->displayFaces = true;
  m_drawStyle->displayEdges = true;
  m_drawStyle->displayPoints = false;

  m_material = new MoMaterial;
  m_material->faceColoring = MoMaterial::CONTOURING;
  m_material->lineColoring = MoMaterial::COLOR;
  m_material->lineColor = SbColor(0, 0, 0);

  m_dataBinding = new MoDataBinding;
  m_dataBinding->dataBinding = MoDataBinding::PER_CELL;

  MoPredefinedColorMapping* colorMap = new MoPredefinedColorMapping;
  colorMap->predefColorMap = MoPredefinedColorMapping::STANDARD;
  m_colorMap = colorMap;

  m_appearanceNode = new SoGroup;
  m_appearanceNode->setName("appearance");
  m_appearanceNode->addChild(m_drawStyle);
  m_appearanceNode->addChild(m_material);
  m_appearanceNode->addChild(m_dataBinding);
  m_appearanceNode->addChild(m_colorMap);

  m_legend = new MoLegend;
  m_legend->vertical = true;
  m_legend->topRight.setValue(-.8f, 1.0f);
  m_legend->bottomLeft.setValue(-1.f, .0f);

  m_legendSwitch = new SoSwitch;
  m_legendSwitch->addChild(m_legend);
  m_legendSwitch->whichChild = SO_SWITCH_NONE;

  SoAnnotation* annotation = new SoAnnotation;
  annotation->boundingBoxIgnoring = true;
  annotation->addChild(m_legendSwitch);

  m_snapshotsSwitch = new SoSwitch;
  m_snapshotsSwitch->setName("snapshots");
  m_snapshotsSwitch->whichChild = SO_SWITCH_NONE;

  m_root = new SoGroup;
  m_root->setName("root");
  m_root->addChild(m_formationShapeHints);
  m_root->addChild(m_scale);
  m_root->addChild(m_appearanceNode);
  m_root->addChild(annotation);
  m_root->addChild(m_snapshotsSwitch);

  setupSnapshots();

  static_cast<MoPredefinedColorMapping*>(m_colorMap)->maxValue = (float)(m_formationIdMap.size() - 1);
}

SceneGraphManager::SceneGraphManager()
  : m_projectHandle(0)
  , m_depthProperty(0)
  , m_currentProperty(0)
  , m_numI(0)
  , m_numJ(0)
  , m_numIHiRes(0)
  , m_numJHiRes(0)
  , m_formationsTimeStamp(MxTimeStamp::getTimeStamp())
  , m_surfacesTimeStamp(MxTimeStamp::getTimeStamp())
  , m_currentSnapshot(0)
  , m_root(0)
  , m_formationShapeHints(0)
  , m_surfaceShapeHints(0)
  , m_scale(0)
  , m_appearanceNode(0)
  , m_drawStyle(0)
  , m_material(0)
  , m_dataBinding(0)
  , m_colorMap(0)
  , m_legend(0)
  , m_legendSwitch(0)
  , m_snapshotsSwitch(0)
{
}

SoNode* SceneGraphManager::getRoot() const
{
  return m_root;
}

void SceneGraphManager::setCurrentSnapshot(size_t index)
{
  updateSnapshot(index);
  m_currentSnapshot = index;
  m_snapshotsSwitch->whichChild = (int)index;
}

size_t SceneGraphManager::getSnapshotCount() const
{
  return (size_t)m_snapshotsSwitch->getNumChildren();
}

int SceneGraphManager::numI() const
{
  return m_numI;
}

int SceneGraphManager::numJ() const
{
  return m_numJ;
}

int SceneGraphManager::numIHiRes() const
{
  return m_numIHiRes;
}

int SceneGraphManager::numJHiRes() const
{
  return m_numJHiRes;
}

void SceneGraphManager::setVerticalScale(float scale)
{
  m_scale->scaleFactor = SbVec3f(1.f, 1.f, scale);
}

void SceneGraphManager::setRenderStyle(bool drawFaces, bool drawEdges)
{
  m_drawStyle->displayFaces = drawFaces;
  m_drawStyle->displayEdges = drawEdges;
}

void SceneGraphManager::setProperty(const std::string& name)
{
  const di::Property* prop = m_projectHandle->findProperty(name);
  if (prop == 0 || prop == m_currentProperty || !prop->hasPropertyValues(di::FORMATION, 0, 0, 0, 0, di::VOLUME))
    return;

  m_currentProperty = prop;

  updateSnapshot(m_currentSnapshot);
}

void SceneGraphManager::enableFormation(const std::string& name, bool enabled)
{
  auto iter = m_formationIdMap.find(name);
  if (iter == m_formationIdMap.end())
    return;

  int id = iter->second;
  if (m_formations[id].visible == enabled)
    return;

  m_formations[id].visible = enabled;
  m_formationsTimeStamp = MxTimeStamp::getTimeStamp();

  updateSnapshot(m_currentSnapshot);
}

void SceneGraphManager::enableSurface(const std::string& name, bool enabled)
{
  auto iter = m_surfaceIdMap.find(name);
  if (iter == m_surfaceIdMap.end())
    return;

  int id = iter->second;
  if (m_surfaces[id].visible == enabled)
    return;

  m_surfaces[id].visible = enabled;
  m_surfacesTimeStamp = MxTimeStamp::getTimeStamp();

  updateSnapshot(m_currentSnapshot);
}

void SceneGraphManager::enableSlice(int slice, bool enabled)
{
  m_sliceEnabled[slice] = enabled;
  updateSnapshot(m_currentSnapshot);
}

void SceneGraphManager::setSlicePosition(int slice, int position)
{
  m_slicePosition[slice] = position;
  updateSnapshot(m_currentSnapshot);
}

void SceneGraphManager::setup(const di::ProjectHandle* handle)
{
  m_projectHandle = handle;

  std::string depthKey = "Depth";
  m_depthProperty = handle->findProperty(depthKey);

  const di::Grid* loresGrid = handle->getLowResolutionOutputGrid();
  const di::Grid* hiresGrid = handle->getHighResolutionOutputGrid();

  m_numI = loresGrid->numI();
  m_numJ = loresGrid->numJ();
  m_numIHiRes = hiresGrid->numI();
  m_numJHiRes = hiresGrid->numJ();

  m_currentSnapshot = 0;
  for (int i = 0; i < 3; ++i)
  {
    m_sliceEnabled[i] = false;
    m_slicePosition[i] = 0;
  }

  // Get all available formations
  std::unique_ptr<di::FormationList> formations(handle->getFormations(0, false));
  for (size_t i = 0; i < formations->size(); ++i)
  {
    m_formationIdMap[(*formations)[i]->getName()] = (int)i;

    FormationInfo info;
    info.formation = (*formations)[i];
    info.id = (int)i;
    info.visible = true;

    m_formations.push_back(info);
  }

  // Get all available surfaces
  std::unique_ptr<di::SurfaceList> surfaces(handle->getSurfaces(0, false));
  for (size_t i = 0; i < surfaces->size(); ++i)
  {
    m_surfaceIdMap[(*surfaces)[i]->getName()] = (int)i;

    SurfaceInfo info;
    info.surface = (*surfaces)[i];
    info.id = (int)i;
    info.visible = false;

    m_surfaces.push_back(info);
  }

  setupSceneGraph();
}

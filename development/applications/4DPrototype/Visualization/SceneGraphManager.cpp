#include "SceneGraphManager.h"
#include "Mesh.h"
#include "Property.h"

#include <Inventor/nodes/SoGroup.h>
#include <Inventor/nodes/SoSwitch.h>
#include <Inventor/nodes/SoScale.h>

#include <MeshVizInterface/MxTimeStamp.h>
#include <MeshVizInterface/mapping/nodes/MoDrawStyle.h>
#include <MeshVizInterface/mapping/nodes/MoMaterial.h>
#include <MeshVizInterface/mapping/nodes/MoDataBinding.h>
#include <MeshVizInterface/mapping/nodes/MoPredefinedColorMapping.h>
#include <MeshVizInterface/mapping/nodes/MoMesh.h>
#include <MeshVizInterface/mapping/nodes/MoMeshSkin.h>
#include <MeshVizInterface/mapping/nodes/MoMeshSlab.h>
#include <MeshVizInterface/mapping/nodes/MoScalarSetIjk.h>

#include <Interface/ProjectHandle.h>
#include <Interface/Grid.h>
#include <Interface/GridMap.h>
#include <Interface/Property.h>
#include <Interface/PropertyValue.h>
#include <Interface/Formation.h>
#include <Interface/Snapshot.h>

#include <memory>

namespace di = DataAccess::Interface;

SnapshotInfo::SnapshotInfo()
  : root(0)
  , mesh(0)
  , meshData(0)
  , scalarSet(0)
  , formationIdProperty(0)
  , chunksGroup(0)
  , formationVisibilityTimestamp(MxTimeStamp::getTimeStamp())
{
  for (int i = 0; i < 3; ++i)
  {
    sliceSwitch[i] = 0;
    slice[i] = 0;
  }
}

void SceneGraphManager::updateSnapshot(size_t index)
{
  SnapshotInfo& snapshot = m_snapshots[index];

  if (
    snapshot.formationVisibilityTimestamp != m_formationVisibilityTimestamp && 
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
    snapshot.formationVisibilityTimestamp = m_formationVisibilityTimestamp;
  }

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

SnapshotInfo SceneGraphManager::createSnapshotNode(const di::Snapshot* snapshot)
{
  SnapshotInfo info;

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

    SnapshotInfo::FormationBounds bounds;
    bounds.id = m_formationIdMap[formation->getName()];
    bounds.minK = k;
    bounds.maxK = k + depth;
    info.formations.push_back(bounds);

    k = bounds.maxK;

    // Add formation id for each k-layer of this formation
    for (int j = 0; j < depth; ++j)
      formationIds.push_back((double)bounds.id);
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
    info.scalarSet->setScalarSet(info.formationIdProperty);

    info.chunksGroup = new SoGroup;
    info.slicesGroup = new SoGroup;

    // setup slabs here...

    info.root->addChild(info.mesh);
    info.root->addChild(info.scalarSet);
    info.root->addChild(info.chunksGroup);
    info.root->addChild(info.slicesGroup);
  }

  return info;
}

void SceneGraphManager::setupSnapshots()
{
  std::unique_ptr<di::SnapshotList> snapshots(m_projectHandle->getSnapshots(di::MAJOR));

  struct SnapshotCompare
  {
    bool operator()(const di::Snapshot* s0, const di::Snapshot* s1) const
    {
      return s0->getTime() > s1->getTime();
    }
  };

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
  m_appearanceNode->addChild(m_drawStyle);
  m_appearanceNode->addChild(m_material);
  m_appearanceNode->addChild(m_dataBinding);
  m_appearanceNode->addChild(m_colorMap);

  m_snapshotsSwitch = new SoSwitch;
  m_snapshotsSwitch->whichChild = SO_SWITCH_NONE;

  m_root = new SoGroup;
  m_root->addChild(m_scale);
  m_root->addChild(m_appearanceNode);
  m_root->addChild(m_snapshotsSwitch);

  setupSnapshots();

  static_cast<MoPredefinedColorMapping*>(m_colorMap)->maxValue = (float)(m_formationIdMap.size() - 1);
}

SceneGraphManager::SceneGraphManager()
: m_projectHandle(0)
, m_depthProperty(0)
, m_formationVisibilityTimestamp(MxTimeStamp::getTimeStamp())
{
}

SoGroup* SceneGraphManager::getRoot() const
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

void SceneGraphManager::setFormationVisibility(const std::string& name, bool visible)
{
  auto iter = m_formationIdMap.find(name);
  if (iter == m_formationIdMap.end())
    return;

  int id = iter->second;
  if (m_formations[id].visible == visible)
    return;

  m_formations[id].visible = visible;
  m_formationVisibilityTimestamp = MxTimeStamp::getTimeStamp();

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

  std::unique_ptr<di::FormationList> formations(handle->getFormations(0, false));
  for (size_t i = 0; i < formations->size(); ++i)
  {
    m_formationIdMap[(*formations)[i]->getName()] = (int)i;

    FormationInfo info;
    info.id = (int)i;
    info.visible = true;
    info.name = (*formations)[i]->getName();
    m_formations.push_back(info);
  }

  setupSceneGraph();
}
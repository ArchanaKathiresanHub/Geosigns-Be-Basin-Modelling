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
#include "Scheduler.h"
#include "Mesh.h"
#include "Property.h"
#include "FlowLines.h"
#include "OutlineBuilder.h"
#include "FluidContacts.h"
#include "GeometryUtil.h"
#include "ColorMap.h"
#include "PropertyValueCellFilter.h"
#include "Seismic.h"

#include <Inventor/nodes/SoCallback.h>
#include <Inventor/nodes/SoGroup.h>
#include <Inventor/nodes/SoSwitch.h>
#include <Inventor/nodes/SoSphere.h>
#include <Inventor/nodes/SoScale.h>
#include <Inventor/nodes/SoShapeHints.h>
#include <Inventor/nodes/SoAnnotation.h>
#include <Inventor/nodes/SoVertexProperty.h>
#include <Inventor/nodes/SoMaterial.h>
#include <Inventor/nodes/SoPerspectiveCamera.h>
#include <Inventor/nodes/SoOrthographicCamera.h>
#include <Inventor/nodes/SoTranslation.h>
#include <Inventor/nodes/SoTransformSeparator.h>
#include <Inventor/nodes/SoText2.h>
#include <Inventor/nodes/SoLineSet.h>
#include <Inventor/nodes/SoBaseColor.h>
#include <Inventor/nodes/SoIndexedLineSet.h>
#include <Inventor/nodes/SoShaderProgram.h>
#include <Inventor/nodes/SoAlgebraicSphere.h>

#include <Inventor/devices/SoCpuBufferObject.h>
#include <Inventor/nodes/SoEventCallback.h>
#include <Inventor/events/SoLocation2Event.h>
#include <Inventor/events/SoMouseButtonEvent.h>
#include <Inventor/SoPickedPoint.h>

#include <MeshVizXLM/MxTimeStamp.h>
#include <MeshVizXLM/mapping/nodes/MoDrawStyle.h>
#include <MeshVizXLM/mapping/nodes/MoMaterial.h>
#include <MeshVizXLM/mapping/nodes/MoDataBinding.h>
#include <MeshVizXLM/mapping/nodes/MoCustomColorMapping.h>
#include <MeshVizXLM/mapping/nodes/MoLevelColorMapping.h>
#include <MeshVizXLM/mapping/nodes/MoMesh.h>
#include <MeshVizXLM/mapping/nodes/MoMeshSkin.h>
#include <MeshVizXLM/mapping/nodes/MoMeshSlab.h>
#include <MeshVizXLM/mapping/nodes/MoMeshFenceSlice.h>
#include <MeshVizXLM/mapping/nodes/MoMeshIsoline.h>
#include <MeshVizXLM/mapping/nodes/MoScalarSet.h>
#include <MeshVizXLM/mapping/nodes/MoLegend.h>
#include <MeshVizXLM/mapping/nodes/MoCellFilter.h>
#include <MeshVizXLM/mapping/nodes/MoMeshSurface.h>
#include <MeshVizXLM/mapping/details/MoFaceDetailI.h>
#include <MeshVizXLM/mapping/details/MoFaceDetailIj.h>
#include <MeshVizXLM/mapping/details/MoFaceDetailIjk.h>
#include <MeshVizXLM/extractors/MiSkinExtractIjk.h>

#define USE_OIV_COORDINATE_GRID

#ifdef USE_OIV_COORDINATE_GRID
#include <MeshViz/graph/PoAutoCubeAxis.h>
#include <MeshViz/graph/PoLinearAxis.h>
#include <MeshViz/graph/PoGenAxis.h>
#endif

#include <VolumeViz/nodes/SoVolumeRenderingQuality.h>

#include <memory>

SoSwitch* createCompass();

struct LoadSnapshotMeshTask : public Task
{
  std::shared_ptr<Project> project;
  size_t snapshotIndex;

  std::shared_ptr<MiVolumeMeshCurvilinear> result;

public:

  LoadSnapshotMeshTask(std::shared_ptr<Project> proj, size_t index)
    : project(proj)
    , snapshotIndex(index)
  {
    affinity = IOTASK;
  }

  void run() override
  {
    result = project->createSnapshotMesh(snapshotIndex);
  }
};

SnapshotInfo::SnapshotInfo()
  : index(0)
  , currentPropertyId(-1)
  , minZ(0.0)
  , maxZ(0.0)
  , root(0)
  , formationsRoot(0)
  , mesh(0)
  , scalarSet(0)
  , chunksGroup(0)
  , flowLinesGroup(0)
  , surfacesGroup(0)
  , reservoirsGroup(0)
  , faultsGroup(0)
  , slicesGroup(0)
  , fencesGroup(0)
  , formationsTimeStamp(MxTimeStamp::getTimeStamp())
  , surfacesTimeStamp(MxTimeStamp::getTimeStamp())
  , reservoirsTimeStamp(MxTimeStamp::getTimeStamp())
  , faultsTimeStamp(MxTimeStamp::getTimeStamp())
  , flowLinesTimeStamp(MxTimeStamp::getTimeStamp())
  , fencesTimeStamp(MxTimeStamp::getTimeStamp())
  , cellFilterTimeStamp(MxTimeStamp::getTimeStamp())
  , seismicPlaneSliceTimeStamp(MxTimeStamp::getTimeStamp())
{
}

void SceneGraphManager::mousePressedCallback(void* userData, SoEventCallback* node)
{
  reinterpret_cast<SceneGraphManager*>(userData)->onMousePressed(node);
}

void SceneGraphManager::mouseMovedCallback(void* userData, SoEventCallback* node)
{
  reinterpret_cast<SceneGraphManager*>(userData)->onMouseMoved(node);
}

void SceneGraphManager::onMousePressed(SoEventCallback* node)
{
  if (!SoMouseButtonEvent::isButtonPressEvent(node->getEvent(), SoMouseButtonEvent::BUTTON1))
    return;

  PickResult result;

  auto pickedPoint = node->getPickedPoint();
  if (pickedPoint)
  {
    node->setHandled();
    result = processPickedPoint(pickedPoint);
  }

  showPickResult(result);
}

void SceneGraphManager::onMouseMoved(SoEventCallback* node)
{
}

int SceneGraphManager::getSurfaceId(MoMeshSurface* surface) const
{
  assert(!m_snapshotInfoCache.empty());

  const SnapshotInfo& snapshot = *m_snapshotInfoCache.begin();
  for (auto const& s : snapshot.surfaces)
  {
    if (s.surfaceMesh == surface)
      return s.id;
  }

  return -1;
}

int SceneGraphManager::getFormationId(/*MoMeshSkin* skin, */size_t k) const
{
  assert(!m_snapshotInfoCache.empty());

  const SnapshotInfo& snapshot = *m_snapshotInfoCache.begin();
  for (auto const& fmt : snapshot.formations)
  {
    if ((int)k >= fmt.minK && (int)k < fmt.maxK)
      return fmt.id;
  }

  return -1;
}

int SceneGraphManager::getReservoirId(MoMeshSurface* skin) const
{
  assert(!m_snapshotInfoCache.empty());

  const SnapshotInfo& snapshot = *m_snapshotInfoCache.begin();
  for (auto const& res : snapshot.reservoirs)
  {
    if (res.skin == skin)
      return res.id;
  }

  return -1;
}

void SceneGraphManager::updateCoordinateGrid()
{
#ifdef USE_OIV_COORDINATE_GRID

  if (!m_viewState.showGrid)
    return;

  assert(!m_snapshotInfoCache.empty());

  SbViewportRegion vpregion;
  SoGetBoundingBoxAction action(vpregion);
  action.apply(m_snapshotsSwitch);
  SbBox3f box = action.getBoundingBox();
  SbVec3f minvec = box.getMin();
  SbVec3f maxvec = box.getMax();
  SbVec3f size = box.getSize();

  const float margin = .05f;
  const float dx = margin * size[0];
  const float dy = margin * size[1];
  SbVec3f gradStart(minvec[0] - dx, minvec[1] - dy, minvec[2]);
  SbVec3f gradEnd(maxvec[0] + dx, maxvec[1] + dy, maxvec[2]);
  SbVec3f start(gradStart[0], gradStart[1], m_viewState.verticalScale * gradStart[2]);
  SbVec3f end(gradEnd[0], gradEnd[1], m_viewState.verticalScale * gradEnd[2]);

  m_coordinateGrid->start = start;
  m_coordinateGrid->end = end;
  m_coordinateGrid->gradStart = gradStart;
  m_coordinateGrid->gradEnd = gradEnd;

#endif
}

void SceneGraphManager::updateSnapshotMesh()
{
  assert(!m_snapshotInfoCache.empty());

  SnapshotInfo& snapshot = *m_snapshotInfoCache.begin();
  
  // skip if we already have the mesh data, or are in the process of loading it
  if (!!snapshot.meshData || !snapshot.loadSnapshotMeshTask.expired())
    return;

  bool needMesh = false;

  // Check if formations need mesh
  if (snapshot.formationsTimeStamp != m_formationsTimeStamp)
  {
    for (auto const& formation : snapshot.formations)
    {
      if (m_viewState.formationVisibility[formation.id])
        needMesh = true;
    }
  }

  // Check if flowlines need mesh
  if (!needMesh && snapshot.flowLinesTimeStamp != m_flowLinesTimeStamp)
  {
    for (auto const& flowlines : snapshot.flowlines)
    {
      if (m_viewState.flowLinesVisibility[flowlines.id])
        needMesh = true;
    }
  }

  // Check if slices need mesh
  if (!needMesh)
  {
    for (int i = 0; i < 3; ++i)
    {
      if (m_viewState.sliceEnabled[i])
        needMesh = true;
    }
  }

  // Check if fences need mesh
  if (!needMesh)
  {
    for (auto const& fence : m_viewState.fences)
    {
      if (fence.visible)
        needMesh = true;
    }
  }

  // Check if interpolated slice needs mesh
  needMesh = needMesh || m_seismicSwitch->getNumChildren() > 0;

  if (needMesh)
  {
    auto task = std::make_shared<LoadSnapshotMeshTask>(m_project, snapshot.index);
    task->type = TaskLoadSnapshotMesh;
    task->source = this;
    m_scheduler.put(task);

    // Keep a weak ref to this task, so we know we're loading it
    snapshot.loadSnapshotMeshTask = task;
  }
}

namespace
{
  SnapshotInfo::Chunk createChunk(const MiVolumeMeshCurvilinear& mesh, const MiCellFilterIjk* cellFilter, int kmin, int kmax)
  {
    SnapshotInfo::Chunk chunk;
    chunk.minK = kmin;
    chunk.maxK = kmax;

    auto const& topology = mesh.getTopology();
    MbVec3ui rangeMin(0u, 0u, (size_t)kmin);
    MbVec3ui rangeMax(
      topology.getNumCellsI(),
      topology.getNumCellsJ(),
      (size_t)kmax);

    chunk.extractor.reset(MiSkinExtractIjk::getNewInstance(mesh));
    chunk.extractor->addCellRange(rangeMin, rangeMax);
    chunk.root = new SoSeparator;
    chunk.mesh = new MoMesh;
    chunk.mesh->setMesh(&chunk.extractor->extractSkin(cellFilter));
    chunk.scalarSet = new MoScalarSet;
    chunk.skin = new MoMeshSurface;
    chunk.skin->colorScalarSetId = -1;
    chunk.root->addChild(chunk.mesh);
    chunk.root->addChild(chunk.scalarSet);
    chunk.root->addChild(chunk.skin);

    return chunk;
  }

  void updateChunkScalarSet(SnapshotInfo::Chunk& chunk, MiDataSetIjk<double>* dataSet)
  {
    if(dataSet)
    {
      auto chunkDataSet = &chunk.extractor->extractScalarSet(*dataSet);
      chunk.scalarSet->setScalarSet(chunkDataSet);
      chunk.skin->colorScalarSetId = 0;
    }
    else
    {
      chunk.scalarSet->setScalarSet(nullptr);
      chunk.skin->colorScalarSetId = -1;
    }
  }
}

void SceneGraphManager::updateSnapshotFormations()
{
  assert(!m_snapshotInfoCache.empty());

  SnapshotInfo& snapshot = *m_snapshotInfoCache.begin(); 

  // Update formations
  if (
    snapshot.formationsTimeStamp == m_formationsTimeStamp || 
    !snapshot.meshData ||
    !snapshot.chunksGroup || 
    snapshot.formations.empty())
    {
      return;
    }

  snapshot.chunksGroup->removeAllChildren();

  const MiCellFilterIjk* cellFilter = nullptr;
  if (m_viewState.cellFilterEnabled && snapshot.scalarDataSet)
    cellFilter = snapshot.propertyValueCellFilter.get();

  bool buildingChunk = false;
  int minK=0, maxK=0;
  std::vector<SnapshotInfo::Chunk> tmpChunks;
  for (size_t i = 0; i < snapshot.formations.size(); ++i)
  {
    bool visible = m_viewState.formationVisibility[snapshot.formations[i].id];

    if (!buildingChunk && visible)
      minK = snapshot.formations[i].minK;
    else if (buildingChunk && !visible)
      tmpChunks.push_back(createChunk(*snapshot.meshData, cellFilter, minK, maxK));

    maxK = snapshot.formations[i].maxK;
    buildingChunk = visible;
  }

  // don't forget the last one
  if (buildingChunk)
    tmpChunks.push_back(createChunk(*snapshot.meshData, cellFilter, minK, maxK));

  for (size_t i = 0; i < tmpChunks.size(); ++i)
  {
    updateChunkScalarSet(tmpChunks[i], snapshot.scalarDataSet.get());
    snapshot.chunksGroup->addChild(tmpChunks[i].root);
  }

  snapshot.chunks.swap(tmpChunks);
  snapshot.formationsTimeStamp = m_formationsTimeStamp;
}

namespace
{
  MoColorMapping* createTrapsColorMap(unsigned int maxId)
  {
    MoLevelColorMapping* colorMapping = new MoLevelColorMapping;

    colorMapping->colors.set1Value(0, SbColorRGBA(.5f, .5f, .5f, 1.f));

    for (unsigned int i = 1; i <= maxId; ++i)
    {
      float h = fmodf(.13f * i, 1.f);
      SbColorRGBA color;
      color.setHSVAValue(h, 1.f, 1.f, 1.f);
      colorMapping->colors.set1Value(i, color);
    }

    for (int i = 0; i <= (int)(maxId + 1); ++i)
      colorMapping->values.set1Value(i, i - .5f);

    return colorMapping;
  }
}

void SceneGraphManager::updateSnapshotSurfaces()
{
  assert(!m_snapshotInfoCache.empty());

  SnapshotInfo& snapshot = *m_snapshotInfoCache.begin();

  // Update surfaces
  if (snapshot.surfacesTimeStamp == m_surfacesTimeStamp)
    return;

  for (auto &surf : snapshot.surfaces)
  {
    if (m_viewState.surfaceVisibility[surf.id] && surf.root == 0)
    {
      surf.meshData = m_project->createSurfaceMesh(snapshot.index, surf.id);
      surf.mesh = new MoMesh;
      surf.mesh->setMesh(surf.meshData.get());
      surf.scalarSet = new MoScalarSet;
      surf.propertyData = m_project->createSurfaceProperty(snapshot.index, surf.id, snapshot.currentPropertyId);
      surf.scalarSet->setScalarSet(surf.propertyData.get());
      surf.surfaceMesh = new MoMeshSurface;

      SoSeparator* root = new SoSeparator;
      root->addChild(surf.mesh);
      root->addChild(surf.scalarSet);
      root->addChild(surf.surfaceMesh);

      surf.root = root;
      snapshot.surfacesGroup->addChild(root);
    }
    else if (!m_viewState.surfaceVisibility[surf.id] && surf.root != 0)
    {
      snapshot.surfacesGroup->removeChild(surf.root);
      surf.root = 0;
      surf.mesh = 0;
      surf.meshData = 0;
      surf.scalarSet = 0;
      surf.propertyData = 0;
      surf.surfaceMesh = 0;
    }
  }

  snapshot.surfacesTimeStamp = m_surfacesTimeStamp;
}

void SceneGraphManager::updateSnapshotReservoirs()
{
  assert(!m_snapshotInfoCache.empty());

  SnapshotInfo& snapshot = *m_snapshotInfoCache.begin();

  if (snapshot.reservoirsTimeStamp == m_reservoirsTimeStamp)
    return;

  for (auto &res : snapshot.reservoirs)
  {
    if (m_viewState.reservoirVisibility[res.id] && res.root == 0)
    {
      res.root = new SoSeparator;
      res.mesh = new MoMesh;
      res.scalarSet = new MoScalarSet;
      res.skin = new MoMeshSurface;

      res.meshData = m_project->createReservoirMesh(snapshot.index, res.id);
      res.propertyData = createReservoirProperty(snapshot, res, snapshot.currentPropertyId);

      res.extractor.reset(MiSkinExtractIjk::getNewInstance(*res.meshData));
      res.mesh->setMesh(&res.extractor->extractSkin());
      if(res.propertyData)
        res.scalarSet->setScalarSet(&res.extractor->extractScalarSet(*res.propertyData));

      res.root->addChild(res.mesh);
      res.root->addChild(res.scalarSet);
      res.root->addChild(res.skin);

      snapshot.reservoirsGroup->addChild(res.root);
    }
    else if (!m_viewState.reservoirVisibility[res.id] && res.root != 0)
    {
      snapshot.reservoirsGroup->removeChild(res.root);
      res.clear();
    }
  }

  snapshot.reservoirsTimeStamp = m_reservoirsTimeStamp;
}

void SceneGraphManager::updateSnapshotTraps()
{
  assert(!m_snapshotInfoCache.empty());

  SnapshotInfo& snapshot = *m_snapshotInfoCache.begin();
  for (auto &res : snapshot.reservoirs)
  {
    if (!res.root)
      continue;

    if (m_viewState.reservoirVisibility[res.id])
    {
      // See if the vertical scale needs updating for existing traps
      if (m_viewState.showTraps && res.traps.root() != 0 && res.traps.verticalScale() != m_viewState.verticalScale)
      {
        res.traps.setVerticalScale(m_viewState.verticalScale);
      }
      // See if we need to create new traps
      else if (m_viewState.showTraps && res.traps.root() == 0)
      {
        std::vector<Project::Trap> traps = m_project->getTraps(snapshot.index, res.id);
        if (!traps.empty())
        {
          float radius = (float)std::min(
            m_projectInfo.dimensions.deltaXHiRes,
            m_projectInfo.dimensions.deltaYHiRes);
          res.traps = Traps(traps, radius, m_viewState.verticalScale);
          if (res.traps.root() != 0)
            res.root->insertChild(res.traps.root(), 0); // 1st because of blending
        }
      }
      // See if we need to remove existing traps
      else if (!m_viewState.showTraps && res.traps.root() != 0)
      {
        res.root->removeChild(res.traps.root());
        res.traps = Traps();
      }

      // Trap outlines
      if (m_viewState.showTrapOutlines && !res.trapOutlines)
      {
        std::shared_ptr<MiDataSetIjk<double> > dataSet;

        int propertyId = m_project->getPropertyId("ResRockTrapId");
        if (propertyId == snapshot.currentPropertyId)
          dataSet = res.propertyData;
        else
          dataSet = m_project->createReservoirProperty(snapshot.index, res.id, propertyId);
        const MiGeometryIjk& geometry = res.meshData->getGeometry();
        res.trapOutlines = m_outlineBuilder->createOutline(dataSet.get(), &geometry);

        res.root->addChild(res.trapOutlines);
      }
      else if (!m_viewState.showTrapOutlines && res.trapOutlines)
      {
        res.root->removeChild(res.trapOutlines);
        res.trapOutlines = 0;
      }

      // Drainage area outlines
      if (m_viewState.drainageAreaType != DrainageAreaFluid && res.drainageAreaOutlinesFluid)
      {
        res.root->removeChild(res.drainageAreaOutlinesFluid);
        res.drainageAreaOutlinesFluid = 0;
      }

      if (m_viewState.drainageAreaType != DrainageAreaGas && res.drainageAreaOutlinesGas)
      {
        res.root->removeChild(res.drainageAreaOutlinesGas);
        res.drainageAreaOutlinesGas = 0;
      }

      if (m_viewState.drainageAreaType == DrainageAreaFluid && !res.drainageAreaOutlinesFluid)
      {
        std::shared_ptr<MiDataSetIjk<double> > dataSet;

        int propertyId = m_project->getPropertyId("ResRockDrainageIdFluidPhase");
        if (propertyId == snapshot.currentPropertyId)
          dataSet = res.propertyData;
        else
          dataSet = m_project->createReservoirProperty(snapshot.index, res.id, propertyId);
        const MiGeometryIjk& geometry = res.meshData->getGeometry();
        res.drainageAreaOutlinesFluid = m_outlineBuilder->createOutline(dataSet.get(), &geometry);

        res.root->addChild(res.drainageAreaOutlinesFluid);
      }
      else if (m_viewState.drainageAreaType == DrainageAreaGas && !res.drainageAreaOutlinesGas)
      {
        std::shared_ptr<MiDataSetIjk<double> > dataSet;

        int propertyId = m_project->getPropertyId("ResRockDrainageIdGasPhase");
        if (propertyId == snapshot.currentPropertyId)
          dataSet = res.propertyData;
        else
          dataSet = m_project->createReservoirProperty(snapshot.index, res.id, propertyId);
        const MiGeometryIjk& geometry = res.meshData->getGeometry();
        res.drainageAreaOutlinesGas = m_outlineBuilder->createOutline(dataSet.get(), &geometry);

        res.root->addChild(res.drainageAreaOutlinesGas);
      }
    }
  }
}

std::shared_ptr<FaultMesh> SceneGraphManager::generateFaultMesh(
  const std::vector<SbVec2d>& points, 
  const MiVolumeMeshCurvilinear& mesh, 
  int k0, 
  int k1)
{
  const MiGeometryIjk& geometry = mesh.getGeometry();

  const Project::Dimensions& dim = m_projectInfo.dimensions;
  double minX = 0.0;// m_project->minX();
  double minY = 0.0;// m_project->minY();

  double maxX = minX + dim.deltaX * dim.numCellsI;
  double maxY = minY + dim.deltaY * dim.numCellsJ;

  std::vector<MbVec3d> coords;
  coords.reserve(2 * points.size());

  for (auto p : points)
  {
    double i = std::max(std::min(p[0], maxX), 0.0) / dim.deltaX;
    double j = std::max(std::min(p[1], maxY), 0.0) / dim.deltaY;
    double z0 = getZ(geometry, dim.numCellsI, dim.numCellsJ, i, j, k0);
    double z1 = getZ(geometry, dim.numCellsI, dim.numCellsJ, i, j, k1);

    coords.emplace_back(p[0], p[1], z0);
    coords.emplace_back(p[0], p[1], z1);
  }

  auto faultGeometry = std::make_shared<FaultGeometry>(coords);
  auto faultTopology = std::make_shared<FaultTopology>(points.size() - 1);

  return std::make_shared<FaultMesh>(faultGeometry, faultTopology);
}

void SceneGraphManager::updateSnapshotFaults()
{
  assert(!m_snapshotInfoCache.empty());

  SnapshotInfo& snapshot = *m_snapshotInfoCache.begin();

  // Update formations
  if (snapshot.faultsTimeStamp == m_faultsTimeStamp)
    return;

  for (size_t i = 0; i < snapshot.faults.size(); ++i)
  {
    int id = snapshot.faults[i].id;

    if (m_viewState.faultVisibility[id] && snapshot.faults[i].root == 0)
    {
      std::vector<SbVec2d> faultLine = m_project->getFaultLine(id);
      
      snapshot.faults[i].meshData = generateFaultMesh(faultLine, *snapshot.meshData, snapshot.faults[i].minK, snapshot.faults[i].maxK);
      snapshot.faults[i].root = new SoSeparator;
      snapshot.faults[i].mesh = new MoMesh;
      snapshot.faults[i].mesh->setMesh(snapshot.faults[i].meshData.get());
      snapshot.faults[i].surfaceMesh = new MoMeshSurface;

      snapshot.faults[i].root->addChild(snapshot.faults[i].mesh);
      snapshot.faults[i].root->addChild(snapshot.faults[i].surfaceMesh);
      snapshot.faultsGroup->addChild(snapshot.faults[i].root);
    }
    else if (!m_viewState.faultVisibility[id] && snapshot.faults[i].root != 0)
    {
      snapshot.faultsGroup->removeChild(snapshot.faults[i].root);
      snapshot.faults[i].root = 0;
      snapshot.faults[i].mesh = 0;
      snapshot.faults[i].surfaceMesh = 0;
      snapshot.faults[i].meshData.reset();
    }
  }

  snapshot.faultsTimeStamp = m_faultsTimeStamp;
}

void SceneGraphManager::updateSnapshotProperties()
{
  assert(!m_snapshotInfoCache.empty());

  SnapshotInfo& snapshot = *m_snapshotInfoCache.begin();

  // Update properties
  if (snapshot.currentPropertyId == m_viewState.currentPropertyId)
    return;

  snapshot.scalarDataSet = createFormationProperty(snapshot, m_viewState.currentPropertyId);
  snapshot.scalarSet->setScalarSet(snapshot.scalarDataSet.get());

  // the cell filter depends on the current property, so update it
  m_cellFilterTimeStamp = MxTimeStamp::getTimeStamp();
  updateSnapshotCellFilter();

  for (auto &surf : snapshot.surfaces)
  {
    if (surf.root)
    {
      surf.propertyData = m_project->createSurfaceProperty(snapshot.index, surf.id, m_viewState.currentPropertyId);
      surf.scalarSet->setScalarSet(surf.propertyData.get());
    }
  }

  for (auto &res : snapshot.reservoirs)
  {
    if (res.root)
    {
      res.propertyData = createReservoirProperty(snapshot, res, m_viewState.currentPropertyId);
      if(res.propertyData)
        res.scalarSet->setScalarSet(&res.extractor->extractScalarSet(*res.propertyData));
    }
  }

  for (auto &chunk : snapshot.chunks)
    updateChunkScalarSet(chunk, snapshot.scalarDataSet.get());

  snapshot.currentPropertyId = m_viewState.currentPropertyId;
}

namespace
{
  void setupSlice(SnapshotInfo::Slice& slice, const MiVolumeMeshCurvilinear& mesh)
  {
    slice.position = -1;
    slice.extractor.reset(MiSkinExtractIjk::getNewInstance(mesh));
    slice.root = new SoSeparator;
    slice.mesh = new MoMesh;
    slice.scalarSet = new MoScalarSet;
    slice.skin = new MoMeshSurface;
    slice.skin->colorScalarSetId = -1;
    slice.root->addChild(slice.mesh);
    slice.root->addChild(slice.scalarSet);
    slice.root->addChild(slice.skin);
  }

  void updateSliceGeometry(SnapshotInfo::Slice& slice, int axis, int position)
  {
    assert(axis == 0 || axis == 1);

    MbVec3ui rangeMin(0u, 0u, 0u);
    MbVec3ui rangeMax(UNDEFINED_ID, UNDEFINED_ID, UNDEFINED_ID);

    rangeMin[axis] = (size_t)position;
    rangeMax[axis] = (size_t)position + 1;

    slice.extractor->clearCellRanges();
    slice.extractor->addCellRange(rangeMin, rangeMax);
    slice.mesh->setMesh(&slice.extractor->extractSkin());
  }

  void updateSliceScalarSet(SnapshotInfo::Slice& slice, MiDataSetIjk<double>* dataSet)
  {
    if (dataSet)
    {
      slice.scalarSet->setScalarSet(&slice.extractor->extractScalarSet(*dataSet));
      slice.skin->colorScalarSetId = 0;
    }
    else
    {
      slice.scalarSet->setScalarSet(nullptr);
      slice.skin->colorScalarSetId = -1;
    }
  }
}

void SceneGraphManager::updateSnapshotSlices()
{
  assert(!m_snapshotInfoCache.empty());

  SnapshotInfo& snapshot = *m_snapshotInfoCache.begin();

  if (snapshot.slicesGroup == 0)
    return;

  for (int i = 0; i < 2; ++i)
  {
    if (m_viewState.sliceEnabled[i])
    {
      assert(snapshot.meshData);

      if (!snapshot.slices[i].root)
      {
        setupSlice(snapshot.slices[i], *snapshot.meshData);
        snapshot.slicesGroup->addChild(snapshot.slices[i].root);
      }

      if (snapshot.slices[i].position != m_viewState.slicePosition[i])
      {
        updateSliceGeometry(snapshot.slices[i], i, (int)m_viewState.slicePosition[i]);
        updateSliceScalarSet(snapshot.slices[i], snapshot.scalarDataSet.get());
      }
    }
    else if (snapshot.slices[i].root)
    {
      snapshot.slicesGroup->removeChild(snapshot.slices[i].root);
      snapshot.slices[i].clear();
    }
  }
}

void SceneGraphManager::updateSnapshotFlowLines()
{
  assert(!m_snapshotInfoCache.empty());

  SnapshotInfo& snapshot = *m_snapshotInfoCache.begin();

  if (snapshot.flowLinesTimeStamp == m_flowLinesTimeStamp)
    return;

  for (auto& flowlines : snapshot.flowlines)
  {
    int id = flowlines.id;

    if (m_viewState.flowLinesVisibility[id])
    {
      if (!snapshot.flowDirScalarSet)
        snapshot.flowDirScalarSet = m_project->createFlowDirectionProperty(snapshot.index);

      auto type = m_projectInfo.flowLines[id].type;
      int step = (type == Project::FlowLines::Expulsion) 
        ? m_viewState.flowLinesExpulsionStep 
        : m_viewState.flowLinesLeakageStep;
      double threshold = (type == Project::FlowLines::Expulsion) 
        ? m_viewState.flowLinesExpulsionThreshold 
        : m_viewState.flowLinesLeakageThreshold;

      assert(snapshot.meshData);

      if (!flowlines.root)
      {
        int formationId = m_projectInfo.flowLines[id].formationId;
        int reservoirId = m_projectInfo.flowLines[id].reservoirId;

        flowlines.root = new SoSeparator;
        flowlines.color = new SoBaseColor;
        if (type == Project::FlowLines::Expulsion)
        {
          flowlines.color->rgb.setValue(1.f, .5f, 1.f);
          flowlines.expulsionData = generateExpulsionProperty(*m_project, snapshot.index, formationId);
        }
        else // Leakage
        {
          flowlines.color->rgb.setValue(1.f, 1.f, .5f);
          flowlines.expulsionData = generateLeakageProperty(*m_project, snapshot.index, reservoirId);
        }

        flowlines.lines = generateFlowLines(
          *snapshot.flowDirScalarSet, 
          flowlines.expulsionData, 
          *snapshot.meshData, 
          flowlines.startK, 
          step,
          threshold);

        flowlines.root->addChild(flowlines.color);
        flowlines.root->addChild(flowlines.lines);
        snapshot.flowLinesGroup->addChild(flowlines.root);
      }
      else
      {
        auto newlines = generateFlowLines(
          *snapshot.flowDirScalarSet, 
          flowlines.expulsionData, 
          *snapshot.meshData, 
          flowlines.startK, 
          step,
          threshold);

        flowlines.root->replaceChild(flowlines.lines, newlines);
        flowlines.lines = newlines;
      }
    }
    else if (!m_viewState.flowLinesVisibility[id] && flowlines.root)
    {
      snapshot.flowLinesGroup->removeChild(flowlines.root);
      flowlines.clear();
    }
  }

  snapshot.flowLinesTimeStamp = m_flowLinesTimeStamp;
}

void SceneGraphManager::updateSnapshotCellFilter()
{
  assert(!m_snapshotInfoCache.empty());

  SnapshotInfo& snapshot = *m_snapshotInfoCache.begin();
  if (snapshot.cellFilterTimeStamp == m_cellFilterTimeStamp)
    return;

  snapshot.propertyValueCellFilter->setDataSet(snapshot.scalarDataSet.get());
  snapshot.propertyValueCellFilter->setRange(
    m_viewState.cellFilterMinValue, 
    m_viewState.cellFilterMaxValue);

  snapshot.cellFilterTimeStamp = m_cellFilterTimeStamp;

  // cell filter changed, so update formations
  if (m_viewState.cellFilterEnabled)
  {
    m_formationsTimeStamp = MxTimeStamp::getTimeStamp();
    updateSnapshotFormations();
  }
}

void SceneGraphManager::updateSnapshotFences()
{
  assert(!m_snapshotInfoCache.empty());

  SnapshotInfo& snapshot = *m_snapshotInfoCache.begin();
  if (snapshot.fencesTimeStamp == m_fencesTimeStamp)
	return;

  for (auto const& fence : m_viewState.fences)
  {
	auto iter = std::find_if(
	  snapshot.fences.begin(), 
	  snapshot.fences.end(), 
	  [fence](const SnapshotInfo::Fence& f) { return f.id == fence.id; });

	if (iter == snapshot.fences.end())
	{
	  SnapshotInfo::Fence f;
	  f.id = fence.id;
	  f.timestamp = MxTimeStamp::getTimeStamp();
	  f.slice = new MoMeshFenceSlice;
	  f.slice->direction.setValue(0.f, 0.f, -1.f);
	  f.root = new SoSwitch;
	  f.root->addChild(f.slice);

	  snapshot.fences.push_back(f);
	  snapshot.fencesGroup->addChild(f.root);

	  iter = snapshot.fences.end();
	  iter--;
	}

	if (iter->timestamp != fence.timestamp)
	{
	  iter->slice->polyline.setValues(0, (int)fence.points.size(), fence.points.data());
	  iter->timestamp = fence.timestamp;
	}

	iter->root->whichChild = fence.visible 
	  ? SO_SWITCH_ALL 
	  : SO_SWITCH_NONE;
  }
}

void SceneGraphManager::updateColorMap()
{
  if (m_viewState.currentPropertyId < 0)
    return;

  int index = 0;
  int trapId = m_project->getPropertyId("ResRockTrapId");
  if (m_viewState.currentPropertyId == trapId || m_viewState.currentPropertyId == PersistentTrapIdPropertyId)
    index = 1;
  else if (m_viewState.currentPropertyId == FluidContactsPropertyId)
    index = 2;

  m_colorMapSwitch->whichChild = index;

  assert(!m_snapshotInfoCache.empty());

  SnapshotInfo& snapshot = *m_snapshotInfoCache.begin();

  double minValue = std::numeric_limits<double>::max();
  double maxValue = -std::numeric_limits<double>::max();

  m_colors->setLogarithmic(m_viewState.colorScaleParams.mapping == ColorScaleParams::Logarithmic);
  if (m_viewState.colorScaleParams.range == ColorScaleParams::Manual)
  {
    minValue = m_viewState.colorScaleParams.minValue;
    maxValue = m_viewState.colorScaleParams.maxValue;
  }
  else
  {
    if (m_viewState.currentPropertyId == FormationIdPropertyId)
    {
      minValue = 0.0;
      maxValue = (double)(m_projectInfo.formations.size() - 1);
    }
    else
    {
      if (snapshot.scalarDataSet)
      {
        minValue = std::min(minValue, snapshot.scalarDataSet->getMin());
        maxValue = std::max(maxValue, snapshot.scalarDataSet->getMax());
      }

      for (auto const& surf : snapshot.surfaces)
      {
        if (surf.propertyData)
        {
          minValue = std::min(minValue, surf.propertyData->getMin());
          maxValue = std::max(maxValue, surf.propertyData->getMax());
        }
      }

      for (auto const& res : snapshot.reservoirs)
      {
        if (res.propertyData)
        {
          minValue = std::min(minValue, res.propertyData->getMin());
          maxValue = std::max(maxValue, res.propertyData->getMax());
        }
      }

      // Round minValue and maxValue down resp. up to 'nice' numbers
      if (minValue != maxValue)
      {
        double e = round(log10(maxValue - minValue)) - 1.0;
        double delta = pow(10.0, e);

        minValue = delta * floor(minValue / delta);
        maxValue = delta * ceil(maxValue / delta);
      }
    }
  }

  m_colors->setRange(minValue, maxValue);

  m_legend->minValue = minValue;
  m_legend->maxValue = maxValue;
}

void SceneGraphManager::updateText()
{
  assert(!m_snapshotInfoCache.empty());

  SnapshotInfo& snapshot = *m_snapshotInfoCache.begin();
  const Project::Dimensions& dim = m_projectInfo.dimensions;

  SbString str1, str2;
  str1.sprintf("Snapshot %d/%d (Age %.1f)", (int)(snapshot.index + 1), (int)m_project->getSnapshotCount(), snapshot.time);
  str2.sprintf("Resolution %dx%d (%dx%d)",
    dim.numCellsI,
    dim.numCellsJ,
    dim.numCellsIHiRes,
    dim.numCellsJHiRes);

  m_text->string.set1Value(1, str1);
  m_text->string.set1Value(2, str2);
}

void SceneGraphManager::updateSnapshot()
{
  updateSnapshotMesh();
  updateSnapshotProperties();
  updateSnapshotCellFilter();
  updateSnapshotFormations();
  updateSnapshotSurfaces();
  updateSnapshotReservoirs();
  updateSnapshotTraps();
  updateSnapshotFaults();
  updateSnapshotSlices();
  updateSnapshotFlowLines();
  updateSnapshotFences();

  updateColorMap();
  updateText();
  updateCoordinateGrid();
}

#ifdef USE_OIV_COORDINATE_GRID
namespace
{
  void initAutoAxis(PoLinearAxis *axis, PbMiscTextAttr *textAtt, SbBool isXYAxis)
  {
    if (isXYAxis) 
    {
      axis->marginType.setValue(PoCartesianAxis::FIXED_MARGIN);
      axis->marginStart.setValue(0.5);
      axis->marginEnd.setValue(0.5);
      axis->tickSubDef.setValue(PoCartesianAxis::NUM_SUB_TICK);
      axis->tickNumOrPeriod.setValue(1);
    }

    axis->titleFontSize = 0.03F;
    axis->setMiscTextAttr(textAtt);
    axis->gradFontSize = 0.02F;
    axis->gradVisibility = PoAxis::VISIBILITY_ON;
    axis->titleVisibility = PoAxis::VISIBILITY_ON;
    //axis->tickMainLength = 1.f;
    //axis->gradFontSize = 1.f;

    axis->arrowVisibility = PoAxis::VISIBILITY_ON;

    SoMaterial *axisMtl = SO_GET_PART(axis, "bodyApp.material", SoMaterial);
    axisMtl->diffuseColor.setHSVValue(0.56F, 0.25F, 1);
    axis->set("bodyApp.drawStyle", "lineWidth 1.5");
  }

  PoAutoCubeAxis* initCoordinateGrid(double minX, double minY, double maxX, double maxY)
  {
    const double margin = .05;
    double dx = (maxX - minX) * margin;
    double dy = (maxY - minY) * margin;

    SbVec3f start((float)(-dx), (float)(-dy), -1000.f);
    SbVec3f end((float)(maxX - minX + dx), (float)(maxY - minY + dy), 0.0f);
      
    PoAutoCubeAxis* grid = new PoAutoCubeAxis(
      start,
      end,
      PoAutoCubeAxis::LINEAR,
      PoAutoCubeAxis::LINEAR,
      PoAutoCubeAxis::LINEAR,
      "X", "Y", "Depth");

    grid->gradStart = start;
    grid->gradEnd = end;

    grid->isGridLinesXVisible = true;
    grid->isGridLinesYVisible = true;
    grid->isGridLinesZVisible = true;

    grid->set("appearance.material", "diffuseColor 0.5 0.5 1.0");
    grid->set("mainGradGridApp.drawStyle", "lineWidth 0.2");
    grid->set("mainGradGridApp.drawStyle", "linePattern 0xF0F0");
    grid->set("subGradGridApp.drawStyle", "style INVISIBLE");
    grid->isIntersectingGradsVisible = TRUE;

    float maxSize = (float)std::max(maxX - minX, maxY - minY);
    SbBox3f bbox(.0f, .0f, -maxSize, maxSize, maxSize, .0f);
    PbDomain* domain = new PbDomain(bbox);
    grid->setDomain(domain);

    const char* axisNames[] = {
      "xAxis03", "xAxis12", "xAxis65", "xAxis74",
      "yAxis01", "yAxis76", "yAxis45", "yAxis32",
      "zAxis07", "zAxis34", "zAxis25", "zAxis16"
    };

    for (int i = 0; i < 12; ++i)
    {
      PoLinearAxis* axis = SO_GET_PART(grid, axisNames[i], PoLinearAxis);
      initAutoAxis(axis, 0, i < 8);
    }

    return grid;
  }
}
#endif

void SceneGraphManager::showPickResult(const PickResult& pickResult)
{
  SbString line1, line2, line3;

  switch (pickResult.type)
  {
  case PickResult::Formation:
  case PickResult::Surface:
  case PickResult::Reservoir:
  case PickResult::Slice:
  case PickResult::Fence:
    line1.sprintf("[%d, %d, %d]",
      (int)pickResult.cellIndex[0],
      (int)pickResult.cellIndex[1],
      (int)pickResult.cellIndex[2]);
    line2.sprintf("Value %g", pickResult.propertyValue);
    line3 = pickResult.name;
    break;

  case PickResult::Trap:
    line1.sprintf("Trap id %d", pickResult.trapID);
    line2.sprintf("Persistent id %d", pickResult.persistentTrapID);
    break;
  }

  m_pickText->string.set1Value(0, line1);
  m_pickText->string.set1Value(1, line2);
  m_pickText->string.set1Value(2, line3);

  m_pickTextSwitch->whichChild = (pickResult.type == PickResult::Unknown)
    ? SO_SWITCH_NONE
    : SO_SWITCH_ALL;
}

SnapshotInfo SceneGraphManager::createSnapshotNode(size_t index)
{
  auto snapshotContents = m_project->getSnapshotContents(index);

  SnapshotInfo info;
  info.index = index;
  info.time = snapshotContents.age;
  info.formations = snapshotContents.formations;

  for (auto id : snapshotContents.surfaces)
  {
    SnapshotInfo::Surface surface;
    surface.id = id;
    info.surfaces.push_back(surface);
  }

  for (auto id : snapshotContents.reservoirs)
  {
    SnapshotInfo::Reservoir reservoir;
    reservoir.id = id;
    info.reservoirs.push_back(reservoir);
  }

  for (auto id : snapshotContents.flowlines)
  {
    SnapshotInfo::FlowLines flowlines;
    flowlines.id = id;

    int formationId = m_projectInfo.flowLines[id].formationId;
    auto type = m_projectInfo.flowLines[id].type;

    for (auto formation : snapshotContents.formations)
    {
      if (formation.id == formationId)
      {
        flowlines.startK = formation.minK;
        if (type == Project::FlowLines::Leakage)
          flowlines.startK--;
        break;
      }
    }

    if(flowlines.startK > 0)
      info.flowlines.push_back(flowlines);
  }

  int collectionId = 0;
  for (auto const& coll : m_projectInfo.faultCollections)
  {
    // ids of formations in this snapshot containing faults
    std::vector<int> formationIds;
    size_t i = 0;
    // match formation ids from the fault collection to formation ids in this snapshot
    for (auto id : coll.formations)
    {
      // don't do full linear search; take advantage of the fact that ids
      // are ordered, so we can resume searching at the point we left off
      for (; i < snapshotContents.formations.size(); ++i)
      {
        if (snapshotContents.formations[i].id == id)
        {
          formationIds.push_back(id);
          break;
        }
      }
    }

    if (!formationIds.empty())
    {
      // find minK and maxK for this collection
      int minId = formationIds[0];
      int maxId = formationIds[formationIds.size() - 1];

      int minK = 0, maxK = 0;
      for (auto const& fmt : info.formations)
      {
        if (fmt.id == minId)
          minK = fmt.minK;

        if (fmt.id == maxId)
          maxK = fmt.maxK;
      }

      int faultId = 0;
      for (auto const& fault : m_projectInfo.faults)
      {
        if (fault.collectionId == collectionId)
        {
          SnapshotInfo::Fault newFault;
          newFault.id = faultId;
          newFault.minK = minK;
          newFault.maxK = maxK;

          info.faults.push_back(newFault);
        }

        faultId++;
      }
    }

    collectionId++;
  }

  // Build the scenegraph
  info.root = new SoGroup;
  info.root->setName("snapshot");

  info.formationsRoot = new SoGroup;
  info.formationsRoot->setName("formations");

  info.minZ = -snapshotContents.maxDepth;
  info.maxZ = -snapshotContents.minDepth;

  info.mesh = new MoMesh;
  info.mesh->setName("snapshotMesh");

  info.scalarSet = new MoScalarSet;
  info.scalarSet->setName("formationProperty");

  // Setup cell filter
  info.propertyValueCellFilter = std::make_shared<PropertyValueCellFilter>();

  info.chunksGroup = new SoGroup;
  info.chunksGroup->setName("chunks");
  info.flowLinesGroup = new SoSeparator;
  info.flowLinesGroup->setName("flowlines");
  info.flowLinesGroup->addChild(createFlowLinesVectorShader());
  info.surfacesGroup = new SoGroup;
  info.surfacesGroup->setName("surfaces");
  info.reservoirsGroup = new SoGroup;
  info.reservoirsGroup->setName("reservoirs");
  info.faultsGroup = new SoGroup;
  info.faultsGroup->setName("faults");
  info.slicesGroup = new SoGroup;
  info.slicesGroup->setName("slices");
  info.fencesGroup = new SoGroup;
  info.fencesGroup->setName("fences");

  info.formationsRoot->addChild(info.mesh);
  info.formationsRoot->addChild(info.flowLinesGroup);
  info.formationsRoot->addChild(info.slicesGroup);
  info.formationsRoot->addChild(m_surfaceShapeHints);
  info.formationsRoot->addChild(info.fencesGroup);
  info.formationsRoot->addChild(info.chunksGroup);

  // Add surfaceShapeHints to prevent backface culling, and enable double-sided lighting
  info.root->addChild(m_surfaceShapeHints);
  info.root->addChild(info.surfacesGroup);
  info.root->addChild(info.faultsGroup);
  info.root->addChild(info.reservoirsGroup);
  info.root->addChild(info.formationsRoot);

  info.currentPropertyId = -1;

  return info;
}

void SceneGraphManager::setupCoordinateGrid()
{
  m_coordinateGridSwitch = new SoSwitch;
  m_coordinateGridSwitch->setName("coordinateGrid");
  m_coordinateGridSwitch->whichChild = SO_SWITCH_NONE;

#ifdef USE_OIV_COORDINATE_GRID
  const Project::Dimensions& dim = m_projectInfo.dimensions;
  double maxX = dim.minX + dim.numCellsI * dim.deltaX;
  double maxY = dim.minY + dim.numCellsJ * dim.deltaY;
  m_coordinateGrid = initCoordinateGrid(
    dim.minX,
    dim.minY,
    maxX,
    maxY);

  m_coordinateGridSwitch->addChild(m_coordinateGrid);
#endif
}

void SceneGraphManager::setupSceneGraph()
{
  // Backface culling is enabled for solid shapes with ordered vertices
  m_formationShapeHints = new SoShapeHints;
  m_formationShapeHints->setName("formationShapeHints");
  m_formationShapeHints->shapeType = SoShapeHints::SOLID;
  m_formationShapeHints->vertexOrdering = SoShapeHints::CLOCKWISE;

  // Double sided lighting is enabled for surfaces with ordered vertices
  m_surfaceShapeHints = new SoShapeHints;
  m_surfaceShapeHints->setName("surfaceShapeHints");
  m_surfaceShapeHints->shapeType = SoShapeHints::UNKNOWN_SHAPE_TYPE;
  m_surfaceShapeHints->vertexOrdering = SoShapeHints::COUNTERCLOCKWISE;

  m_decorationShapeHints = new SoShapeHints;
  m_decorationShapeHints->setName("decorationShapeHints");
  m_decorationShapeHints->shapeType = SoShapeHints::SOLID;
  m_decorationShapeHints->vertexOrdering = SoShapeHints::COUNTERCLOCKWISE;

  setupCoordinateGrid();

  m_scale = new SoScale;
  m_scale->scaleFactor = SbVec3f(1.f, 1.f, m_viewState.verticalScale);

  m_transparencyType = new SoTransparencyType;
  m_transparencyType->type = SoTransparencyType::BLEND;

  m_drawStyle = new MoDrawStyle;
  m_drawStyle->displayFaces = true;
  m_drawStyle->displayEdges = true;
  m_drawStyle->displayPoints = false;
  m_drawStyle->fadingThreshold = 10.0f;

  m_material = new MoMaterial;
  m_material->faceColoring = MoMaterial::CONTOURING;
  m_material->lineColoring = MoMaterial::COLOR;
  m_material->faceColor = SbColor(.5f, .5f, .5f);
  m_material->lineColor = SbColor(.0f, .0f, .0f);

  m_dataBinding = new MoDataBinding;
  m_dataBinding->dataBinding = MoDataBinding::PER_CELL;

  m_colors = std::make_shared<ColorMap>();
  MoCustomColorMapping* colorMap = new MoCustomColorMapping;
  colorMap->setColorMapping(m_colors.get());
  m_colorMap = colorMap;

  unsigned int maxTrapId = m_project->getMaxPersistentTrapId();
  // if there is no trap info in the project file, maxId will be 0
  if (maxTrapId == 0)
	maxTrapId = 100;
  m_trapIdColorMap = createTrapsColorMap(maxTrapId);
  m_fluidContactsColorMap = createFluidContactsColorMap();
  m_colorMapSwitch = new SoSwitch;
  m_colorMapSwitch->addChild(m_colorMap);
  m_colorMapSwitch->addChild(m_trapIdColorMap);
  m_colorMapSwitch->addChild(m_fluidContactsColorMap);
  m_colorMapSwitch->whichChild = 0;

  m_appearanceNode = new SoGroup;
  m_appearanceNode->setName("appearance");
  m_appearanceNode->addChild(m_transparencyType);
  m_appearanceNode->addChild(m_drawStyle);
  m_appearanceNode->addChild(m_material);
  m_appearanceNode->addChild(m_dataBinding);
  m_appearanceNode->addChild(m_colorMapSwitch);

  float right = .8f, top = .9f, w = .1f, h = .4f;
  m_legend = new MoLegend;
  m_legend->vertical = true;
  m_legend->topRight.setValue(right, top);
  m_legend->bottomLeft.setValue(right - w, top - h);
  m_legend->title="Property";
  m_legend->titlePosition = MoLegend::POS_TOP;
  m_legend->titleFontSize = 18.f;
  m_legend->valuesFontSize = 16.f;
  m_legend->numValues = 5;
  m_legend->displayValues = true;

  m_legendSwitch = new SoSwitch;
  m_legendSwitch->setName("legend");
  m_legendSwitch->whichChild = SO_SWITCH_NONE;
  m_legendSwitch->addChild(m_legend);

  MoDrawStyle* legendDrawStyle = new MoDrawStyle;
  legendDrawStyle->displayEdges = false;

  SoPickStyle* pickStyle = new SoPickStyle;
  pickStyle->style = SoPickStyle::UNPICKABLE;

  m_annotation = new SoAnnotation;
  m_annotation->setName("annotation");
  m_annotation->boundingBoxIgnoring = true;
  m_annotation->addChild(pickStyle);
  m_annotation->addChild(legendDrawStyle);
  m_annotation->addChild(m_legendSwitch);

  // Text area
  m_text = new SoText2;
  m_text->string.set1Value(0, "");

  m_pickText = new SoText2;
  m_pickText->string.set1Value(0, "");
  m_pickText->justification = SoText2::RIGHT;
  m_pickTextSwitch = new SoSwitch;
  m_pickTextSwitch->addChild(m_pickText);
  m_pickTextSwitch->whichChild = SO_SWITCH_NONE;

  // Set up a camera for the 2D overlay
  SoOrthographicCamera* camera = new SoOrthographicCamera;
  camera->viewportMapping = SoCamera::LEAVE_ALONE;
  SoTranslation* translation = new SoTranslation;
  translation->translation.setValue(-.99f, .99f, 0.f);
  SoTranslation* pickTextTranslation = new SoTranslation;
  pickTextTranslation->translation.setValue(.95f, -.9f, .0f);

  SoFont* font = new SoFont;
  font->name = "Arial";
  font->size = 14.f;
  font->renderStyle = SoFont::TEXTURE;

  SoTransformSeparator* textGroup = new SoTransformSeparator;
  textGroup->addChild(translation);
  textGroup->addChild(m_text);

  SoTransformSeparator* pickTextGroup = new SoTransformSeparator;
  pickTextGroup->addChild(pickTextTranslation);
  pickTextGroup->addChild(m_pickTextSwitch);

  SoSeparator* textSeparator = new SoSeparator;
  textSeparator->addChild(camera);
  textSeparator->addChild(font);
  textSeparator->addChild(textGroup);
  textSeparator->addChild(pickTextGroup);

  m_textSwitch = new SoSwitch;
  m_textSwitch->addChild(textSeparator);
  m_textSwitch->whichChild = SO_SWITCH_ALL;
  m_annotation->addChild(m_textSwitch);

  m_compassSwitch = createCompass();

  m_seismicSwitch = new SoSwitch;
  m_seismicSwitch->setName("seismic");
  m_seismicSwitch->whichChild = SO_SWITCH_ALL;

  m_snapshotsSwitch = new SoSwitch;
  m_snapshotsSwitch->setName("snapshots");
  m_snapshotsSwitch->whichChild = SO_SWITCH_ALL;

  m_root = new SoGroup;
  m_root->setName("root");
  m_root->addChild(m_formationShapeHints);
  m_root->addChild(m_coordinateGridSwitch);
  m_root->addChild(m_scale);
  m_root->addChild(m_appearanceNode);
  m_root->addChild(m_snapshotsSwitch);
  m_root->addChild(m_seismicSwitch);
  m_root->addChild(m_decorationShapeHints);
  m_root->addChild(m_annotation);
  m_root->addChild(m_compassSwitch);

  SoEventCallback* mouseMovedCallbackNode = new SoEventCallback;
  mouseMovedCallbackNode->setName("MouseMoved");
  mouseMovedCallbackNode->addEventCallback(SoLocation2Event::getClassTypeId(), mouseMovedCallback, this);

  SoEventCallback* mousePressedCallbackNode = new SoEventCallback;
  mousePressedCallbackNode->setName("MousePressed");
  mousePressedCallbackNode->addEventCallback(SoMouseButtonEvent::getClassTypeId(), mousePressedCallback, this);

  m_root->addChild(mousePressedCallbackNode);
  m_root->addChild(mouseMovedCallbackNode);

  m_colors->setRange(0.0, (double)(m_projectInfo.formations.size() - 1));
}

std::shared_ptr<MiDataSetIjk<double> > SceneGraphManager::createFormationProperty(
  const SnapshotInfo& snapshot,
  int propertyId)
{
  if (propertyId == FormationIdPropertyId)
  {
    // collect array of formation ids for FormationIdProperty
    std::vector<double> formationIds;
    for (auto formation : snapshot.formations)
    {
      int minK = formation.minK;
      int maxK = formation.maxK;

      for (int i = minK; i < maxK; ++i)
        formationIds.push_back((double)formation.id);
    }

    return std::make_shared<FormationIdProperty>(formationIds);
  }
  else
  {
    return m_project->createFormationProperty(snapshot.index, propertyId);
  }
}

std::shared_ptr<MiDataSetIjk<double> > SceneGraphManager::createReservoirProperty(
  const SnapshotInfo& snapshot,
  const SnapshotInfo::Reservoir& res,
  int propertyId)
{
  std::shared_ptr<MiDataSetIjk<double> > result;

  if (propertyId < DerivedPropertyBaseId)
  {
    result = m_project->createReservoirProperty(snapshot.index, res.id, m_viewState.currentPropertyId);
  }
  else if (m_viewState.currentPropertyId == FluidContactsPropertyId)
  {
    std::shared_ptr<MiDataSetIjk<double> > trapIdPropertyData;
    int trapIdPropertyId = m_project->getPropertyId("ResRockTrapId");
    if (snapshot.currentPropertyId == trapIdPropertyId)
      trapIdPropertyData = res.propertyData;
    else
      trapIdPropertyData = m_project->createReservoirProperty(snapshot.index, res.id, trapIdPropertyId);

	if (!trapIdPropertyData)
	  return nullptr;

    auto traps = m_project->getTraps(snapshot.index, res.id);
	if (traps.empty())
	  return nullptr;

    result = createFluidContactsProperty(traps, *trapIdPropertyData, *res.meshData);
  }

  return result;
}

SceneGraphManager::SceneGraphManager(Scheduler& scheduler)
  : m_scheduler(scheduler)
  , m_maxCacheItems(3)
  , m_formationsTimeStamp(MxTimeStamp::getTimeStamp())
  , m_surfacesTimeStamp(MxTimeStamp::getTimeStamp())
  , m_reservoirsTimeStamp(MxTimeStamp::getTimeStamp())
  , m_faultsTimeStamp(MxTimeStamp::getTimeStamp())
  , m_flowLinesTimeStamp(MxTimeStamp::getTimeStamp())
  , m_fencesTimeStamp(MxTimeStamp::getTimeStamp())
  , m_cellFilterTimeStamp(MxTimeStamp::getTimeStamp())
  , m_root(0)
  , m_formationShapeHints(0)
  , m_surfaceShapeHints(0)
  , m_coordinateGrid(0)
  , m_coordinateGridSwitch(0)
  , m_scale(0)
  , m_appearanceNode(0)
  , m_transparencyType(0)
  , m_drawStyle(0)
  , m_material(0)
  , m_dataBinding(0)
  , m_colorMap(0)
  , m_trapIdColorMap(0)
  , m_fluidContactsColorMap(0)
  , m_annotation(0)
  , m_legend(0)
  , m_legendSwitch(0)
  , m_text(0)
  , m_textSwitch(0)
  , m_seismicSwitch(0)
  , m_snapshotsSwitch(0)
{
}

SoNode* SceneGraphManager::getRoot() const
{
  return m_root;
}

namespace
{
  MbVec3ui getMappedCellIndex(size_t cellIndex, const MeXSurfaceMeshUnstructured& extract, const MiMeshIjk& mesh)
  {
    MbVec3ui index;

    auto const& topology = mesh.getTopology();
    auto const& extractedTopology = extract.getTopology();

    if (extractedTopology.hasInputCellMapping())
    {
      index[0] = extractedTopology.getInputCellIdI(cellIndex);
      index[1] = extractedTopology.getInputCellIdJ(cellIndex);
      index[2] = extractedTopology.getInputCellIdK(cellIndex);

      // workaround for stupid OIV limitation, see docs for MeXTopologyI::getInputCellIdI(size_t id)
      if (index[1] == UNDEFINED_ID || index[2] == UNDEFINED_ID)
      {
        size_t ni = topology.getNumCellsI();
        size_t nj = topology.getNumCellsJ();
        size_t flatId = index[0];
        size_t ij = flatId % (ni * nj);

        index[0] = ij % ni;
        index[1] = ij / ni;
        index[2] = flatId / (ni * nj);
      }
    }

    return index;
  }
}

SceneGraphManager::PickResult SceneGraphManager::processPickedPoint(const SoPickedPoint* pickedPoint)
{
  assert(!m_snapshotInfoCache.empty());
  const SnapshotInfo& snapshot = *m_snapshotInfoCache.begin();

  auto p = pickedPoint->getPoint();

  PickResult pickResult;
  pickResult.position = p;
  
  auto path = pickedPoint->getPath();
  auto tail = path->getTail();
  auto detail = pickedPoint->getDetail();

  if (detail)
  {
    // check for formation / reservoir / slice
    if (
      detail->isOfType(MoFaceDetailI::getClassTypeId()) && 
      tail->isOfType(MoMeshSurface::getClassTypeId()))
    {
      MoFaceDetailI* fdetail = (MoFaceDetailI*)detail;
      pickResult.propertyValue = fdetail->getValue(p);
      size_t cellIndex = fdetail->getCellIndex();

      auto skin = static_cast<MoMeshSurface*>(tail);

      // find corresponding reservoir
      for(auto const& res : snapshot.reservoirs)
      {
        if (res.skin == skin)
        {
          pickResult.type = PickResult::Reservoir;
          pickResult.name = m_projectInfo.reservoirs[res.id].name;
          pickResult.cellIndex = getMappedCellIndex(cellIndex, res.extractor->getExtract(), *res.meshData);

          return pickResult;
        }
      }

      // find corresponding chunk
      for (auto const& chunk : snapshot.chunks)
      {
        if (chunk.skin == skin)
        {
          pickResult.type = PickResult::Formation;
          pickResult.cellIndex = getMappedCellIndex(cellIndex, chunk.extractor->getExtract(), *snapshot.meshData);
          int id = getFormationId(pickResult.cellIndex[2]);
          pickResult.name = m_projectInfo.formations[id].name;

          return pickResult;
        }
      }

      // find corresponding slice
      for (auto const& slice : snapshot.slices)
      {
        if (slice.skin == skin)
        {
          pickResult.type = PickResult::Slice;
          pickResult.cellIndex = getMappedCellIndex(cellIndex, slice.extractor->getExtract(), *snapshot.meshData);
          int id = getFormationId(pickResult.cellIndex[2]);
          pickResult.name = m_projectInfo.formations[id].name;

          return pickResult;
        }
      }
    }
    // Check for surface
    else if (
      detail->isOfType(MoFaceDetailIj::getClassTypeId()) &&
      tail->isOfType(MoMeshSurface::getClassTypeId()))
    {
      pickResult.type = PickResult::Surface;

      MoFaceDetailIj* fdetail = (MoFaceDetailIj*)detail;
      pickResult.cellIndex[0] = fdetail->getCellIndexI();
      pickResult.cellIndex[1] = fdetail->getCellIndexJ();

      int id = getSurfaceId(static_cast<MoMeshSurface*>(tail));
      if (id != -1)
        pickResult.name = m_projectInfo.surfaces[id].name;

      pickResult.propertyValue = fdetail->getValue(p);
    }
    // Check for slab and fence
    else if (detail->isOfType(MoFaceDetailIjk::getClassTypeId()))
    {
      MoFaceDetailIjk* fdetail = (MoFaceDetailIjk*)detail;
      pickResult.cellIndex[0] = fdetail->getCellIndexI();
      pickResult.cellIndex[1] = fdetail->getCellIndexJ();
      pickResult.cellIndex[2] = fdetail->getCellIndexK();
      pickResult.propertyValue = fdetail->getValue(p);

      if (tail->isOfType(MoMeshFenceSlice::getClassTypeId()))
      {
        int id = getFormationId(pickResult.cellIndex[2]);
        pickResult.type = PickResult::Fence;
        pickResult.name = m_projectInfo.formations[id].name;
      }
    }
  }
  // Check for trap
  else if (tail->isOfType(SoAlgebraicSphere::getClassTypeId()))
  {
    for (auto const& res : snapshot.reservoirs)
    {
      Project::Trap pickedTrap;
      if (res.traps.getTrap(p, pickedTrap))
      {
        pickResult.type = PickResult::Trap;
        pickResult.trapID = pickedTrap.id;
        pickResult.persistentTrapID = pickedTrap.persistentId;
        break;
      }
    }
  }

  return pickResult;
}

const SceneGraphManager::ViewState& SceneGraphManager::getViewState() const
{
  return m_viewState;
}

void SceneGraphManager::setCurrentSnapshot(size_t index)
{
  // Don't do anything if this is already the current snapshot
  if (!m_snapshotInfoCache.empty() && m_snapshotInfoCache.begin()->index == index)
    return;

  m_viewState.currentSnapshotIndex = (int)index;

  // See if we have this node in the cache
  auto iter = std::find_if(
    m_snapshotInfoCache.begin(), 
    m_snapshotInfoCache.end(), 
    [index](const SnapshotInfo& info) 
      { 
        return info.index == index; 
      });

  if (iter == m_snapshotInfoCache.end()) // not available
  {
    // Create a new node, and add it to the front of the cache
    SnapshotInfo newNode = createSnapshotNode(index);
    m_snapshotInfoCache.push_front(newNode);

    // Limit cache size 
    if (m_snapshotInfoCache.size() > m_maxCacheItems)
      m_snapshotInfoCache.pop_back();
  }
  else // node is in cache
  {
    // Move node to the front of the list, making it the current one
    m_snapshotInfoCache.splice(m_snapshotInfoCache.begin(), m_snapshotInfoCache, iter);
  }

  m_snapshotsSwitch->removeAllChildren();
  m_snapshotsSwitch->addChild(m_snapshotInfoCache.begin()->root.ptr());

  updateSnapshot();
}

void SceneGraphManager::setVerticalScale(float scale)
{
  if (m_viewState.verticalScale != scale)
  {
    m_viewState.verticalScale = scale;
    m_scale->scaleFactor = SbVec3f(1.f, 1.f, scale);

    updateSnapshot();
  }
}

void SceneGraphManager::setTransparency(float transparency)
{
  if (transparency != m_viewState.transparency)
  {
    m_viewState.transparency = transparency;
    m_material->transparency = transparency;
  }
}

void SceneGraphManager::setRenderStyle(bool drawFaces, bool drawEdges)
{
  m_viewState.showFaces = drawFaces;
  m_viewState.showEdges = drawEdges;

  m_drawStyle->displayFaces = drawFaces;
  m_drawStyle->displayEdges = drawEdges;
}

void SceneGraphManager::setProperty(int propertyId)
{
  if (m_viewState.currentPropertyId == propertyId)
    return;

  m_viewState.currentPropertyId = propertyId;

  int trapIdPropertyId = m_project->getPropertyId("ResRockTrapId");

  if (propertyId >= 0 && propertyId < DerivedPropertyBaseId && propertyId != trapIdPropertyId)
  {
    std::string name = m_projectInfo.properties[propertyId].name;
    std::string unit = m_projectInfo.properties[propertyId].unit;
    std::string title = name + " [" + unit + "]";

    m_legend->title = title.c_str();
    m_legendSwitch->whichChild = SO_SWITCH_ALL;
  }
  else
  {
    m_legendSwitch->whichChild = SO_SWITCH_NONE;
  }

  updateSnapshot();
}

void SceneGraphManager::setFlowLinesStep(FlowLinesType type, int step)
{
  if (type == FlowLinesExpulsion)
  {
    if (step != m_viewState.flowLinesExpulsionStep)
    {
      m_viewState.flowLinesExpulsionStep = step;
      m_flowLinesTimeStamp = MxTimeStamp::getTimeStamp();

      updateSnapshot();
    }
  }
  else
  {
    if (step != m_viewState.flowLinesLeakageStep)
    {
      m_viewState.flowLinesLeakageStep = step;
      m_flowLinesTimeStamp = MxTimeStamp::getTimeStamp();

      updateSnapshot();
    }
  }
}

void SceneGraphManager::setFlowLinesThreshold(FlowLinesType type, double threshold)
{
  if (type == FlowLinesExpulsion)
  {
    if (threshold != m_viewState.flowLinesExpulsionThreshold)
    {
      m_viewState.flowLinesExpulsionThreshold = threshold;
      m_flowLinesTimeStamp = MxTimeStamp::getTimeStamp();

      updateSnapshot();
    }
  }
  else
  {
    if (threshold != m_viewState.flowLinesLeakageThreshold)
    {
      m_viewState.flowLinesLeakageThreshold = threshold;
      m_flowLinesTimeStamp = MxTimeStamp::getTimeStamp();

      updateSnapshot();
    }
  }
}

void SceneGraphManager::enableFormation(int formationId, bool enabled)
{
  if (m_viewState.formationVisibility[formationId] == enabled)
    return;

  m_viewState.formationVisibility[formationId] = enabled;
  m_formationsTimeStamp = MxTimeStamp::getTimeStamp();

  updateSnapshot();
}

void SceneGraphManager::enableAllFormations(bool enabled)
{
  for (size_t i = 0; i < m_viewState.formationVisibility.size(); ++i)
      m_viewState.formationVisibility[i] = enabled;

  m_formationsTimeStamp = MxTimeStamp::getTimeStamp();

  updateSnapshot();
}

void SceneGraphManager::enableSurface(int surfaceId, bool enabled)
{
  if (m_viewState.surfaceVisibility[surfaceId] == enabled)
    return;

  m_viewState.surfaceVisibility[surfaceId] = enabled;
  m_surfacesTimeStamp = MxTimeStamp::getTimeStamp();

  updateSnapshot();
}

void SceneGraphManager::enableAllSurfaces(bool enabled)
{
  for (size_t i = 0; i < m_viewState.surfaceVisibility.size(); ++i)
    m_viewState.surfaceVisibility[i] = enabled;

  m_surfacesTimeStamp = MxTimeStamp::getTimeStamp();

  updateSnapshot();
}

void SceneGraphManager::enableReservoir(int reservoirId, bool enabled)
{
  if (m_viewState.reservoirVisibility[reservoirId] == enabled)
    return;

  m_viewState.reservoirVisibility[reservoirId] = enabled;
  m_reservoirsTimeStamp = MxTimeStamp::getTimeStamp();

  updateSnapshot();
}

void SceneGraphManager::enableAllReservoirs(bool enabled)
{
  for (size_t i = 0; i < m_viewState.reservoirVisibility.size(); ++i)
    m_viewState.reservoirVisibility[i] = enabled;

  m_reservoirsTimeStamp = MxTimeStamp::getTimeStamp();

  updateSnapshot();
}

void SceneGraphManager::enableFault(int faultId, bool enabled)
{
  if (m_viewState.faultVisibility[faultId] == enabled)
    return;

  m_viewState.faultVisibility[faultId] = enabled;
  m_faultsTimeStamp = MxTimeStamp::getTimeStamp();

  updateSnapshot();
}

void SceneGraphManager::enableAllFaults(bool enabled)
{
  for (size_t i = 0; i < m_viewState.faultVisibility.size(); ++i)
    m_viewState.faultVisibility[i] = enabled;

  m_faultsTimeStamp = MxTimeStamp::getTimeStamp();

  updateSnapshot();
}

void SceneGraphManager::enableFlowLines(int flowLinesId, bool enabled)
{
  if (m_viewState.flowLinesVisibility[flowLinesId] == enabled)
    return;

  m_viewState.flowLinesVisibility[flowLinesId] = enabled;
  m_flowLinesTimeStamp = MxTimeStamp::getTimeStamp();

  updateSnapshot();
}

void SceneGraphManager::enableAllFlowLines(bool enabled)
{
  for (size_t i = 0; i < m_viewState.flowLinesVisibility.size(); ++i)
    m_viewState.flowLinesVisibility[i] = enabled;

  m_flowLinesTimeStamp = MxTimeStamp::getTimeStamp();

  updateSnapshot();
}

void SceneGraphManager::enableSlice(int slice, bool enabled)
{
  m_viewState.sliceEnabled[slice] = enabled;
  updateSnapshot();
}

void SceneGraphManager::setSlicePosition(int slice, int position)
{
  m_viewState.slicePosition[slice] = position;
  updateSnapshot();
}

int SceneGraphManager::addFence(const std::vector<SbVec3f>& polyline)
{
  int maxId = 0;
  for (auto const& f : m_viewState.fences)
  {
    if (f.id > maxId)
      maxId = f.id;
  }

  FenceParams fence;
  fence.id = maxId + 1;
  fence.visible = true;
  fence.timestamp = MxTimeStamp::getTimeStamp();
  fence.points = polyline;

  m_viewState.fences.push_back(fence);
  m_fencesTimeStamp = MxTimeStamp::getTimeStamp();

  updateSnapshotFences();

  return fence.id;
}

void SceneGraphManager::updateFence(int id, const std::vector<SbVec3f>& polyline)
{
  auto iter = std::find_if(
	m_viewState.fences.begin(), 
	m_viewState.fences.end(), 
	[id](const FenceParams& f) 
  { 
	return f.id == id; 
  });

  if (iter == m_viewState.fences.end())
    return;

  iter->points = polyline;
  iter->timestamp = MxTimeStamp::getTimeStamp();

  updateSnapshotFences();
}

void SceneGraphManager::removeFence(int id)
{

}

void SceneGraphManager::enableFence(int id, bool enabled)
{
  auto iter = std::find_if(
	m_viewState.fences.begin(), 
	m_viewState.fences.end(), 
	[id](const FenceParams& f) 
  { 
	return f.id == id; 
  });

  if (iter == m_viewState.fences.end() || iter->visible == enabled)
    return;

  iter->visible = enabled;
  m_fencesTimeStamp = MxTimeStamp::getTimeStamp();
  updateSnapshotFences();
}

void SceneGraphManager::setColorScaleParams(const SceneGraphManager::ColorScaleParams& params)
{
  if (
    params.mapping != m_viewState.colorScaleParams.mapping ||
    params.range != m_viewState.colorScaleParams.range ||
    params.minValue != m_viewState.colorScaleParams.minValue ||
    params.maxValue != m_viewState.colorScaleParams.maxValue)
  {
    m_viewState.colorScaleParams = params;

    updateSnapshot();
  }
}

void SceneGraphManager::showCoordinateGrid(bool show)
{
  if (show != m_viewState.showGrid)
  {
    m_viewState.showGrid = show;

    if (!show)
    {
      m_coordinateGridSwitch->whichChild = SO_SWITCH_NONE;
    }
    else
    {
      updateCoordinateGrid();
      m_coordinateGridSwitch->whichChild = SO_SWITCH_ALL;
    }
  }
}

void SceneGraphManager::showCompass(bool show)
{
  if (show != m_viewState.showCompass)
  {
    m_viewState.showCompass = show;

    m_compassSwitch->whichChild = show ? SO_SWITCH_ALL : SO_SWITCH_NONE;
  }
}

void SceneGraphManager::showText(bool show)
{
  if (show != m_viewState.showText)
  {
    m_viewState.showText = show;

    m_textSwitch->whichChild = show ? SO_SWITCH_ALL : SO_SWITCH_NONE;
  }
}

void SceneGraphManager::showTraps(bool show)
{
  if (show != m_viewState.showTraps)
  {
    m_viewState.showTraps = show;

    updateSnapshot();
  }
}

void SceneGraphManager::showTrapOutlines(bool show)
{
  if (show != m_viewState.showTrapOutlines)
  {
    m_viewState.showTrapOutlines = show;

    updateSnapshot();
  }
}

void SceneGraphManager::showDrainageAreaOutlines(DrainageAreaType type)
{
  if (type != m_viewState.drainageAreaType)
  {
    m_viewState.drainageAreaType = type;

    updateSnapshot();
  }
}

void SceneGraphManager::enableCellFilter(bool enabled)
{
  if (enabled != m_viewState.cellFilterEnabled)
  {
    m_viewState.cellFilterEnabled = enabled;

    m_cellFilterTimeStamp = MxTimeStamp::getTimeStamp();
    updateSnapshotCellFilter();
  }
}

void SceneGraphManager::setCellFilterRange(double minValue, double maxValue)
{
  if (minValue != m_viewState.cellFilterMinValue || maxValue != m_viewState.cellFilterMaxValue)
  {
    m_viewState.cellFilterMinValue = minValue;
    m_viewState.cellFilterMaxValue = maxValue;

    m_cellFilterTimeStamp = MxTimeStamp::getTimeStamp();
    updateSnapshotCellFilter();
  }
}

void SceneGraphManager::addSeismicScene(std::shared_ptr<SeismicScene> seismicScene)
{
  m_seismicScene = seismicScene;

  if (!m_snapshotInfoCache.empty())
    m_seismicScene->setMesh(m_snapshotInfoCache.begin()->meshData.get());

  m_seismicSwitch->addChild(seismicScene->getRoot());
}

void SceneGraphManager::setup(std::shared_ptr<Project> project)
{
  m_project = project;
  m_projectInfo = project->getProjectInfo();

  m_viewState.formationVisibility.assign(m_projectInfo.formations.size(), true);
  m_viewState.surfaceVisibility.assign(m_projectInfo.surfaces.size(), false);
  m_viewState.reservoirVisibility.assign(m_projectInfo.reservoirs.size(), false);
  m_viewState.faultVisibility.assign(m_projectInfo.faults.size(), false);
  m_viewState.flowLinesVisibility.assign(m_projectInfo.flowLines.size(), false);

  for (int i = 0; i < 3; ++i)
  {
    m_viewState.sliceEnabled[i] = false;
    m_viewState.slicePosition[i] = 0;
  }

  m_outlineBuilder = std::make_shared<OutlineBuilder>(
    m_projectInfo.dimensions.numCellsIHiRes,
    m_projectInfo.dimensions.numCellsJHiRes);

  //m_maxPersistentTrapId = getMaxPersistentTrapId(handle);

  setupSceneGraph();
  setCurrentSnapshot(0);
}

void SceneGraphManager::onTaskCompleted(Task& task)
{
  if (task.type == TaskLoadSnapshotMesh)
  {
    auto &t = static_cast<LoadSnapshotMeshTask&>(task);

    auto iter = m_snapshotInfoCache.begin();
    while (iter != m_snapshotInfoCache.end() && iter->index != t.snapshotIndex)
      iter++;

    if (iter != m_snapshotInfoCache.end())
    {
      iter->meshData = t.result;
      iter->mesh->setMesh(iter->meshData.get());

      if (m_seismicScene)
        m_seismicScene->setMesh(iter->meshData.get());

      updateSnapshot();
    }
  }
}

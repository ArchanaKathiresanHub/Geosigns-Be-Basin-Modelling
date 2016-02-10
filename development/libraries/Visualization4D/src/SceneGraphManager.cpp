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
#include "FlowLines.h"
#include "OutlineBuilder.h"
#include "FluidContacts.h"

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

#include <Inventor/nodes/SoEventCallback.h>
#include <Inventor/events/SoLocation2Event.h>
#include <Inventor/SoPickedPoint.h>

#include <MeshVizXLM/MxTimeStamp.h>
#include <MeshVizXLM/mapping/nodes/MoDrawStyle.h>
#include <MeshVizXLM/mapping/nodes/MoMaterial.h>
#include <MeshVizXLM/mapping/nodes/MoDataBinding.h>
#include <MeshVizXLM/mapping/nodes/MoPredefinedColorMapping.h>
#include <MeshVizXLM/mapping/nodes/MoLevelColorMapping.h>
#include <MeshVizXLM/mapping/nodes/MoMesh.h>
#include <MeshVizXLM/mapping/nodes/MoMeshSkin.h>
#include <MeshVizXLM/mapping/nodes/MoMeshSlab.h>
#include <MeshVizXLM/mapping/nodes/MoMeshVector.h>
#include <MeshVizXLM/mapping/nodes/MoMeshSurface.h>
#include <MeshVizXLM/mapping/nodes/MoMeshIsoline.h>
#include <MeshVizXLM/mapping/nodes/MoScalarSetIj.h>
#include <MeshVizXLM/mapping/nodes/MoScalarSetIjk.h>
#include <MeshVizXLM/mapping/nodes/MoVec3SetIjk.h>
#include <MeshVizXLM/mapping/nodes/MoLegend.h>
#include <MeshVizXLM/mapping/details/MoFaceDetailIj.h>
#include <MeshVizXLM/mapping/details/MoFaceDetailIjk.h>

#include <MeshViz/graph/PoAutoCubeAxis.h>
#include <MeshViz/graph/PoLinearAxis.h>
#include <MeshViz/graph/PoGenAxis.h>

#include <memory>

SoSwitch* createCompass();

SnapshotInfo::SnapshotInfo()
  : index(0)
  , currentPropertyId(-1)
  , minZ(0.0)
  , maxZ(0.0)
  , root(0)
  , formationsRoot(0)
  , mesh(0)
  , meshData(0)
  , scalarSet(0)
  , flowDirSet(0)
  , chunksGroup(0)
  , flowLinesGroup(0)
  , flowVectorsGroup(0)
  , surfacesGroup(0)
  , reservoirsGroup(0)
  , faultsGroup(0)
  , slicesGroup(0)
  , formationsTimeStamp(MxTimeStamp::getTimeStamp())
  , surfacesTimeStamp(MxTimeStamp::getTimeStamp())
  , reservoirsTimeStamp(MxTimeStamp::getTimeStamp())
  , faultsTimeStamp(MxTimeStamp::getTimeStamp())
  , flowLinesTimeStamp(MxTimeStamp::getTimeStamp())
{
  for (int i = 0; i < 3; ++i)
  {
    sliceSwitch[i] = 0;
    slice[i] = 0;
  }
}

void SceneGraphManager::mouseMovedCallback(void* userData, SoEventCallback* node)
{
  reinterpret_cast<SceneGraphManager*>(userData)->onMouseMoved(node);
}

void SceneGraphManager::onMouseMoved(SoEventCallback* node)
{
  auto pickedPoint = node->getPickedPoint();
  if (pickedPoint)
    processPickedPoint(pickedPoint);
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

int SceneGraphManager::getFormationId(MoMeshSkin* skin, size_t k) const
{
  assert(!m_snapshotInfoCache.empty());

  const SnapshotInfo& snapshot = *m_snapshotInfoCache.begin();
  for (auto const& chunk : snapshot.chunks)
  {
    if (chunk.skin == skin)
    {
      for (auto const& fmt : snapshot.formations)
      {
        if ((int)k >= fmt.minK && (int)k < fmt.maxK)
          return fmt.id;
      }
    }
  }

  return -1;
}

int SceneGraphManager::getReservoirId(MoMeshSkin* skin) const
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
  if (!m_showGrid)
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
  SbVec3f start(gradStart[0], gradStart[1], m_verticalScale * gradStart[2]);
  SbVec3f end(gradEnd[0], gradEnd[1], m_verticalScale * gradEnd[2]);

  m_coordinateGrid->start = start;
  m_coordinateGrid->end = end;
  m_coordinateGrid->gradStart = gradStart;
  m_coordinateGrid->gradEnd = gradEnd;
}

void SceneGraphManager::updateSnapshotFormations()
{
  assert(!m_snapshotInfoCache.empty());

  SnapshotInfo& snapshot = *m_snapshotInfoCache.begin();

  // Update formations
  if (snapshot.formationsTimeStamp == m_formationsTimeStamp || !snapshot.chunksGroup || snapshot.formations.empty())
    return;

  snapshot.chunksGroup->removeAllChildren();

  bool buildingChunk = false;
  int minK=0, maxK=0;

  std::vector<SnapshotInfo::Chunk> tmpChunks;

  for (size_t i = 0; i < snapshot.formations.size(); ++i)
  {
    int id = snapshot.formations[i].id;
    if (!buildingChunk && m_formationVisibility[id])
    {
      buildingChunk = true;
      minK = snapshot.formations[i].minK;
    }
    else if (buildingChunk && !m_formationVisibility[id])
    {
      buildingChunk = false;
      tmpChunks.push_back(SnapshotInfo::Chunk(minK, maxK));
    }

    maxK = snapshot.formations[i].maxK;
  }

  // don't forget the last one
  if (buildingChunk)
    tmpChunks.push_back(SnapshotInfo::Chunk(minK, maxK));

  // load meshData if needed
  if (!snapshot.meshData && !tmpChunks.empty())
  {
    snapshot.meshData = m_project->createSnapshotMesh(snapshot.index);
    snapshot.mesh->setMesh(snapshot.meshData.get());
  }

  if (snapshot.meshData)
  {
    const MiTopologyIjk& topology = snapshot.meshData->getTopology();
    for (size_t i = 0; i < tmpChunks.size(); ++i)
    {
      MoMeshSkin* meshSkin = new MoMeshSkin;
      uint32_t rangeMin[] = { 0, 0, (uint32_t)tmpChunks[i].minK };
      uint32_t rangeMax[] = {
        (uint32_t)(topology.getNumCellsI()),
        (uint32_t)(topology.getNumCellsJ()),
        (uint32_t)(tmpChunks[i].maxK - 1) };

      meshSkin->minCellRanges.setValues(0, 3, rangeMin);
      meshSkin->maxCellRanges.setValues(0, 3, rangeMax);
#ifdef _DEBUG
      meshSkin->parallel = false;
#endif
      tmpChunks[i].skin = meshSkin;
      snapshot.chunksGroup->addChild(meshSkin);
    }
  }

  snapshot.chunks.swap(tmpChunks);
  snapshot.formationsTimeStamp = m_formationsTimeStamp;

  if (!snapshot.scalarDataSet)
  {
    snapshot.scalarDataSet = createFormationProperty(snapshot, snapshot.currentPropertyId);
    snapshot.scalarSet->setScalarSet(snapshot.scalarDataSet.get());
  }
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
    if (m_surfaceVisibility[surf.id] && surf.root == 0)
    {
      surf.meshData = m_project->createSurfaceMesh(snapshot.index, surf.id);
      surf.mesh = new MoMesh;
      surf.mesh->setMesh(surf.meshData.get());
      surf.scalarSet = new MoScalarSetIj;
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
    else if (!m_surfaceVisibility[surf.id] && surf.root != 0)
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
    if (m_reservoirVisibility[res.id] && res.root == 0)
    {
      res.root = new SoSeparator;
      res.mesh = new MoMesh;

      res.meshData = m_project->createReservoirMesh(snapshot.index, res.id);
      res.mesh->setMesh(res.meshData.get());

      res.propertyData = createReservoirProperty(snapshot, res, snapshot.currentPropertyId);
      res.scalarSet = new MoScalarSetIjk;
      res.scalarSet->setScalarSet(res.propertyData.get());

      res.skin = new MoMeshSkin;

      res.root->addChild(res.mesh);
      res.root->addChild(res.scalarSet);
      res.root->addChild(res.skin);

      snapshot.reservoirsGroup->addChild(res.root);
    }
    else if (!m_reservoirVisibility[res.id] && res.root != 0)
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

    if (m_reservoirVisibility[res.id])
    {
      // See if the vertical scale needs updating for existing traps
      if (m_showTraps && res.traps.root() != 0 && res.traps.verticalScale() != m_verticalScale)
      {
        res.traps.setVerticalScale(m_verticalScale);
      }
      // See if we need to create new traps
      else if (m_showTraps && res.traps.root() == 0)
      {
        std::vector<Project::Trap> traps = m_project->getTraps(snapshot.index, res.id);
        if (!traps.empty())
        {
          float radius = (float)std::min(
            m_projectInfo.dimensions.deltaXHiRes,
            m_projectInfo.dimensions.deltaYHiRes);
          res.traps = Traps(traps, radius, m_verticalScale);
          if (res.traps.root() != 0)
            res.root->insertChild(res.traps.root(), 0); // 1st because of blending
        }
      }
      // See if we need to remove existing traps
      else if (!m_showTraps && res.traps.root() != 0)
      {
        res.root->removeChild(res.traps.root());
        res.traps = Traps();
      }

      // Trap outlines
      if (m_showTrapOutlines && !res.trapOutlines)
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
      else if (!m_showTrapOutlines && res.trapOutlines)
      {
        res.root->removeChild(res.trapOutlines);
        res.trapOutlines = 0;
      }

      // Drainage area outlines
      if (m_drainageAreaType != DrainageAreaFluid && res.drainageAreaOutlinesFluid)
      {
        res.root->removeChild(res.drainageAreaOutlinesFluid);
        res.drainageAreaOutlinesFluid = 0;
      }

      if (m_drainageAreaType != DrainageAreaGas && res.drainageAreaOutlinesGas)
      {
        res.root->removeChild(res.drainageAreaOutlinesGas);
        res.drainageAreaOutlinesGas = 0;
      }

      if (m_drainageAreaType == DrainageAreaFluid && !res.drainageAreaOutlinesFluid)
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
      else if (m_drainageAreaType == DrainageAreaGas && !res.drainageAreaOutlinesGas)
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

namespace
{
  /**
   * Linear interpolation
   */
  inline double lerp(double x0, double x1, double a)
  {
    return (1.0 - a) * x0 + a * x1;
  }

  /**
   * Get a depth value from geometry, using floating point x and y coordinates
   * and bilinear interpolation
   */
  double getZ(const MiGeometryIjk& geometry, int maxI, int maxJ, double i, double j, int k)
  {
    int i0 = (int)floor(i);
    int j0 = (int)floor(j);

    // Take care not to read outside the bounds of the 3D array
    int i1 = std::min(i0 + 1, maxI);
    int j1 = std::min(j0 + 1, maxJ);

    double di = i - i0;
    double dj = j - j0;

    double z00 = geometry.getCoord(i0, j0, k)[2];
    double z01 = geometry.getCoord(i0, j1, k)[2];
    double z10 = geometry.getCoord(i1, j0, k)[2];
    double z11 = geometry.getCoord(i1, j1, k)[2];

    return lerp(
      lerp(z00, z01, dj),
      lerp(z10, z11, dj),
      di);
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

    if (m_faultVisibility[id] && snapshot.faults[i].root == 0)
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
    else if (!m_faultVisibility[id] && snapshot.faults[i].root != 0)
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
  if (snapshot.currentPropertyId == m_currentPropertyId)
    return;

  snapshot.scalarDataSet = createFormationProperty(snapshot, m_currentPropertyId);
  snapshot.scalarSet->setScalarSet(snapshot.scalarDataSet.get());

  for (auto &surf : snapshot.surfaces)
  {
    if (surf.root)
    {
      surf.propertyData = m_project->createSurfaceProperty(snapshot.index, surf.id, m_currentPropertyId);
      surf.scalarSet->setScalarSet(surf.propertyData.get());
    }
  }

  for (auto &res : snapshot.reservoirs)
  {
    if (res.root)
    {
      res.propertyData = createReservoirProperty(snapshot, res, m_currentPropertyId);
      res.scalarSet->setScalarSet(res.propertyData.get());
    }
  }

  snapshot.currentPropertyId = m_currentPropertyId;
}

void SceneGraphManager::updateSnapshotSlices()
{
  assert(!m_snapshotInfoCache.empty());

  SnapshotInfo& snapshot = *m_snapshotInfoCache.begin();

  if (snapshot.slicesGroup == 0)
    return;

  for (int i = 0; i < 2; ++i)
  {
    if (m_sliceEnabled[i])
    {
      if (!snapshot.meshData)
      {
        snapshot.meshData = m_project->createSnapshotMesh(snapshot.index);
        snapshot.mesh->setMesh(snapshot.meshData.get());
      }

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

void SceneGraphManager::updateSnapshotFlowLines()
{
  assert(!m_snapshotInfoCache.empty());

  SnapshotInfo& snapshot = *m_snapshotInfoCache.begin();

  if (snapshot.flowLinesTimeStamp == m_flowLinesTimeStamp)
    return;

  if (!snapshot.flowDirScalarSet)
    snapshot.flowDirScalarSet = m_project->createFlowDirectionProperty(snapshot.index);

  for (auto& flowlines : snapshot.flowlines)
  {
    int id = flowlines.id;

    if (m_flowLinesVisibility[id])
    {
      auto type = m_projectInfo.flowLines[id].type;
      int step = (type == Project::FlowLines::Expulsion) 
        ? m_flowLinesExpulsionStep 
        : m_flowLinesLeakageStep;
      double threshold = (type == Project::FlowLines::Expulsion) 
        ? m_flowLinesExpulsionThreshold 
        : m_flowLinesLeakageThreshold;

      if (!snapshot.meshData)
      {
        snapshot.meshData = m_project->createSnapshotMesh(snapshot.index);
        snapshot.mesh->setMesh(snapshot.meshData.get());
      }

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
    else if (!m_flowLinesVisibility[id] && flowlines.root)
    {
      snapshot.flowLinesGroup->removeChild(flowlines.root);
      flowlines.clear();
    }
  }

  snapshot.flowLinesTimeStamp = m_flowLinesTimeStamp;
  return;

  //if (m_flowVizType == FlowVizNone)
  //{
  //  snapshot.flowDirScalarSet.reset();
  //  snapshot.flowDirVectorSet.reset();
  //  snapshot.flowLinesGroup->removeAllChildren();
  //  snapshot.flowVectorsGroup->removeAllChildren();
  //}
  //else
  //{
  //  if (!snapshot.flowDirScalarSet)
  //  {
  //    snapshot.flowDirScalarSet = m_project->createFlowDirectionProperty(snapshot.index);
  //  }

  //  if (m_flowVizType == FlowVizLines && snapshot.flowLinesGroup->getNumChildren() == 0)
  //  {
  //    // Add color node
  //    SoBaseColor* color = new SoBaseColor;
  //    color->rgb.setValue(1.f, .5f, 1.f);
  //    snapshot.flowLinesGroup->addChild(color);

  //    for (auto& fmt : snapshot.formations)
  //    {
  //      if (m_projectInfo.formations[fmt.id].isSourceRock)
  //      {
  //        auto expulsionProperty = generateExpulsionProperty(*m_project, snapshot.index, fmt.id);
  //        SoLineSet* flowLines = generateFlowLines(*snapshot.flowDirScalarSet, expulsionProperty, *snapshot.meshData, fmt.minK, 10);
  //        snapshot.flowLinesGroup->addChild(flowLines);
  //      }
  //    }

  //    snapshot.flowVectorsGroup->removeAllChildren();
  //  }
  //  else if (m_flowVizType == FlowVizVectors && snapshot.flowVectorsGroup->getNumChildren() == 0)
  //  {
  //    // Add color node
  //    SoBaseColor* color = new SoBaseColor;
  //    color->rgb.setValue(1.f, 1.f, .5f);
  //    snapshot.flowVectorsGroup->addChild(color);

  //    snapshot.flowDirSet = new MoVec3SetIjk;
  //    snapshot.flowDirVectorSet = std::make_shared<FlowDirectionProperty>(*snapshot.flowDirScalarSet, *snapshot.meshData);
  //    snapshot.flowDirSet->setVec3Set(snapshot.flowDirVectorSet.get());
  //    snapshot.flowVectorsGroup->addChild(snapshot.flowDirSet);

  //    MoMeshVector* meshVector = new MoMeshVector;
  //    meshVector->colorScalarSetId = -1;
  //    meshVector->scaleFactor = .5f;
  //    snapshot.flowVectorsGroup->addChild(meshVector);

  //    snapshot.flowLinesGroup->removeAllChildren();
  //  }
  //}
}

void SceneGraphManager::updateColorMap()
{
  if (m_currentPropertyId < 0)
    return;

  int index = 0;
  int trapId = m_project->getPropertyId("ResRockTrapId");
  if (m_currentPropertyId == trapId || m_currentPropertyId == PersistentTrapIdPropertyId)
    index = 1;
  else if (m_currentPropertyId == FluidContactsPropertyId)
    index = 2;

  m_colorMapSwitch->whichChild = index;

  assert(!m_snapshotInfoCache.empty());

  SnapshotInfo& snapshot = *m_snapshotInfoCache.begin();

  double minValue = std::numeric_limits<double>::max();
  double maxValue = -std::numeric_limits<double>::max();

  if (m_currentPropertyId == FormationIdPropertyId)
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

  static_cast<MoPredefinedColorMapping*>(m_colorMap)->minValue = (float)minValue;
  static_cast<MoPredefinedColorMapping*>(m_colorMap)->maxValue = (float)maxValue;

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
  updateSnapshotFormations();
  updateSnapshotSurfaces();
  updateSnapshotReservoirs();
  updateSnapshotTraps();
  updateSnapshotFaults();
  updateSnapshotProperties();
  updateSnapshotSlices();
  updateSnapshotFlowLines();

  updateColorMap();
  updateText();
  updateCoordinateGrid();
}

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

void SceneGraphManager::showPickResult(const PickResult& pickResult)
{
  SbString line1, line2, line3;

  switch (pickResult.type)
  {
  case PickResult::Formation:
  case PickResult::Surface:
  case PickResult::Reservoir:
    line1.sprintf("[%d, %d]",
      (int)pickResult.i,
      (int)pickResult.j,
      (int)pickResult.k);
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
  info.root = new SoSeparator;
  info.root->ref();
  info.formationsRoot = new SoSeparator;
  info.formationsRoot->setName("formations");
  //info.meshData = m_project->createSnapshotMesh(index);

  info.minZ = -snapshotContents.maxDepth;
  info.maxZ = -snapshotContents.minDepth;

  info.mesh = new MoMesh;
  info.mesh->setName("snapshotMesh");
  //info.mesh->setMesh(info.meshData.get());

  info.scalarSet = new MoScalarSetIjk;
  info.scalarSet->setName("formationID");

  info.chunksGroup = new SoGroup;
  info.chunksGroup->setName("chunks");
  info.flowLinesGroup = new SoSeparator;
  info.flowLinesGroup->setName("flowlines");
  info.flowLinesGroup->addChild(createFlowLinesVectorShader());
  info.flowVectorsGroup = new SoSeparator;
  info.flowVectorsGroup->setName("flowvectors");
  info.surfacesGroup = new SoGroup;
  info.surfacesGroup->setName("surfaces");
  info.reservoirsGroup = new SoGroup;
  info.reservoirsGroup->setName("reservoirs");
  info.faultsGroup = new SoGroup;
  info.faultsGroup->setName("faults");
  info.slicesGroup = new SoGroup;
  info.slicesGroup->setName("slices");

  info.formationsRoot->addChild(info.mesh);
  info.formationsRoot->addChild(info.flowLinesGroup);
  info.formationsRoot->addChild(info.flowVectorsGroup);
  info.formationsRoot->addChild(info.scalarSet);
  info.formationsRoot->addChild(info.chunksGroup);
  info.formationsRoot->addChild(info.slicesGroup);

  info.root->addChild(info.formationsRoot);
  // Add surfaceShapeHints to prevent backface culling, and enable double-sided lighting
  info.root->addChild(info.reservoirsGroup);
  info.root->addChild(m_surfaceShapeHints);
  info.root->addChild(info.surfacesGroup);
  info.root->addChild(info.faultsGroup);

  // set property id, so when updateSnapshot() is called, all elements (formations, surfaces
  // and reservoirs) are created with the correct property
  info.currentPropertyId = m_currentPropertyId;

  return info;
}

void SceneGraphManager::setupCoordinateGrid()
{
  const Project::Dimensions& dim = m_projectInfo.dimensions;
  double maxX = dim.minX + dim.numCellsI * dim.deltaX;
  double maxY = dim.minY + dim.numCellsJ * dim.deltaY;
  m_coordinateGrid = initCoordinateGrid(
    dim.minX,
    dim.minY,
    maxX,
    maxY);

  m_coordinateGridSwitch = new SoSwitch;
  m_coordinateGridSwitch->setName("coordinateGrid");
  m_coordinateGridSwitch->addChild(m_coordinateGrid);
  m_coordinateGridSwitch->whichChild = SO_SWITCH_NONE;
}

void SceneGraphManager::setupSceneGraph()
{
  m_perspectiveCamera = new SoPerspectiveCamera;
  m_orthographicCamera = new SoOrthographicCamera;
  m_perspectiveCamera->orientation.connectFrom(&m_orthographicCamera->orientation);
  m_orthographicCamera->orientation.connectFrom(&m_perspectiveCamera->orientation);
  m_perspectiveCamera->position.connectFrom(&m_orthographicCamera->position);
  m_orthographicCamera->position.connectFrom(&m_perspectiveCamera->position);
  m_perspectiveCamera->nearDistance.connectFrom(&m_orthographicCamera->nearDistance);
  m_orthographicCamera->nearDistance.connectFrom(&m_perspectiveCamera->nearDistance);
  m_perspectiveCamera->farDistance.connectFrom(&m_orthographicCamera->farDistance);
  m_orthographicCamera->farDistance.connectFrom(&m_perspectiveCamera->farDistance);
  m_perspectiveCamera->focalDistance.connectFrom(&m_orthographicCamera->focalDistance);
  m_orthographicCamera->focalDistance.connectFrom(&m_perspectiveCamera->focalDistance);

  m_cameraSwitch = new SoSwitch;
  m_cameraSwitch->addChild(m_perspectiveCamera);
  m_cameraSwitch->addChild(m_orthographicCamera);
  m_cameraSwitch->whichChild = SO_SWITCH_NONE;// (m_projectionType == PerspectiveProjection) ? 0 : 1;

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
  m_scale->scaleFactor = SbVec3f(1.f, 1.f, m_verticalScale);

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

  MoPredefinedColorMapping* colorMap = new MoPredefinedColorMapping;
  colorMap->predefColorMap = MoPredefinedColorMapping::STANDARD;
  m_colorMap = colorMap;
  m_trapIdColorMap = createTrapsColorMap(m_project->getMaxPersistentTrapId());
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

  m_snapshotsSwitch = new SoSwitch;
  m_snapshotsSwitch->setName("snapshots");
  m_snapshotsSwitch->whichChild = SO_SWITCH_ALL;

  m_root = new SoGroup;
  m_root->setName("root");
  m_root->addChild(m_cameraSwitch);
  m_root->addChild(m_formationShapeHints);
  m_root->addChild(m_coordinateGridSwitch);
  m_root->addChild(m_scale);
  m_root->addChild(m_appearanceNode);
  m_root->addChild(m_snapshotsSwitch);
  m_root->addChild(m_decorationShapeHints);
  m_root->addChild(m_annotation);
  m_root->addChild(m_compassSwitch);

  SoEventCallback* callback = new SoEventCallback;
  callback->setName("MouseMoved");
  callback->addEventCallback(SoLocation2Event::getClassTypeId(), mouseMovedCallback, this);
  //m_root->addChild(callback);

  static_cast<MoPredefinedColorMapping*>(m_colorMap)->maxValue = (float)(m_projectInfo.formations.size() - 1);
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
    result = m_project->createReservoirProperty(snapshot.index, res.id, m_currentPropertyId);
  }
  else if (m_currentPropertyId == FluidContactsPropertyId)
  {
    std::shared_ptr<MiDataSetIjk<double> > trapIdPropertyData;
    int trapIdPropertyId = m_project->getPropertyId("ResRockTrapId");
    if (snapshot.currentPropertyId == trapIdPropertyId)
      trapIdPropertyData = res.propertyData;
    else
      trapIdPropertyData = m_project->createReservoirProperty(snapshot.index, res.id, trapIdPropertyId);

    auto traps = m_project->getTraps(snapshot.index, res.id);

    result = createFluidContactsProperty(traps, *trapIdPropertyData, *res.meshData);
  }

  return result;
}

SceneGraphManager::SceneGraphManager()
  : m_maxCacheItems(3)
  , m_currentPropertyId(FormationIdPropertyId)
  , m_showGrid(false)
  , m_showCompass(true)
  , m_showText(true)
  , m_showTraps(false)
  , m_showTrapOutlines(false)
  , m_showFlowVectors(false)
  , m_drainageAreaType(DrainageAreaNone)
  , m_flowLinesExpulsionStep(1)
  , m_flowLinesLeakageStep(1)
  , m_flowLinesExpulsionThreshold(0.0)
  , m_flowLinesLeakageThreshold(0.0)
  , m_verticalScale(1.f)
  , m_projectionType(PerspectiveProjection)
  , m_formationsTimeStamp(MxTimeStamp::getTimeStamp())
  , m_surfacesTimeStamp(MxTimeStamp::getTimeStamp())
  , m_reservoirsTimeStamp(MxTimeStamp::getTimeStamp())
  , m_faultsTimeStamp(MxTimeStamp::getTimeStamp())
  , m_flowLinesTimeStamp(MxTimeStamp::getTimeStamp())
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
  , m_snapshotsSwitch(0)
{
}

SoNode* SceneGraphManager::getRoot() const
{
  return m_root;
}

SceneGraphManager::PickResult SceneGraphManager::processPickedPoint(const SoPickedPoint* pickedPoint)
{
  auto p = pickedPoint->getPoint();

  PickResult pickResult;
  pickResult.position = p;

  auto path = pickedPoint->getPath();
  auto tail = path->getTail();
  auto detail = pickedPoint->getDetail();

  if (detail)
  {
    // Check for surface
    if (
      detail->isOfType(MoFaceDetailIj::getClassTypeId()) &&
      tail->isOfType(MoMeshSurface::getClassTypeId()))
    {
      MoFaceDetailIj* fdetail = (MoFaceDetailIj*)detail;
      pickResult.i = fdetail->getCellIndexI();
      pickResult.j = fdetail->getCellIndexJ();

      int id = getSurfaceId(static_cast<MoMeshSurface*>(tail));
      if (id != -1)
        pickResult.name = m_projectInfo.surfaces[id].name;

      pickResult.propertyValue = fdetail->getValue(p);
    }
    // Check for formation / reservoir
    else if (
      detail->isOfType(MoFaceDetailIjk::getClassTypeId()) &&
      tail->isOfType(MoMeshSkin::getClassTypeId()))
    {
      MoFaceDetailIjk* fdetail = (MoFaceDetailIjk*)detail;
      pickResult.i = fdetail->getCellIndexI();
      pickResult.j = fdetail->getCellIndexJ();
      pickResult.k = fdetail->getCellIndexK();

      pickResult.propertyValue = fdetail->getValue(p);

      MoMeshSkin* skin = static_cast<MoMeshSkin*>(tail);
      int id = getFormationId(skin, pickResult.k);
      if (id != -1)
      {
        pickResult.type = PickResult::Formation;
        pickResult.name = m_projectInfo.formations[id].name;
      }
      else
      {
        id = getReservoirId(skin);
        if (id != -1)
        {
          pickResult.type = PickResult::Reservoir;
          pickResult.name = m_projectInfo.reservoirs[id].name;
        }
      }
    }
  }
  // Check for trap
  else if (tail->isOfType(SoAlgebraicSphere::getClassTypeId()))
  {
    assert(!m_snapshotInfoCache.empty());
    const SnapshotInfo& snapshot = *m_snapshotInfoCache.begin();

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


void SceneGraphManager::setCurrentSnapshot(size_t index)
{
  // Don't do anything if this is already the current snapshot
  if (!m_snapshotInfoCache.empty() && m_snapshotInfoCache.begin()->index == index)
    return;

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
    {
      m_snapshotInfoCache.back().root->unref();
      m_snapshotInfoCache.pop_back();
    }
  }
  else // node is in cache
  {
    // Move node to the front of the list, making it the current one
    m_snapshotInfoCache.splice(m_snapshotInfoCache.begin(), m_snapshotInfoCache, iter);
  }

  m_snapshotsSwitch->removeAllChildren();
  m_snapshotsSwitch->addChild(m_snapshotInfoCache.begin()->root);

  updateSnapshot();
}

SoCamera* SceneGraphManager::getCamera() const
{
  bool perspective = m_cameraSwitch->whichChild.getValue() == 0;
  if (perspective)
    return m_perspectiveCamera;
  else
    return m_orthographicCamera;
}

void SceneGraphManager::setProjection(SceneGraphManager::ProjectionType type)
{
  m_projectionType = type;
  m_cameraSwitch->whichChild = (type == PerspectiveProjection) ? 0 : 1;
}

void SceneGraphManager::setVerticalScale(float scale)
{
  m_verticalScale = scale;
  m_scale->scaleFactor = SbVec3f(1.f, 1.f, scale);

  updateSnapshot();
}

void SceneGraphManager::setTransparency(float transparency)
{
  m_material->transparency = transparency;
}

void SceneGraphManager::setRenderStyle(bool drawFaces, bool drawEdges)
{
  m_drawStyle->displayFaces = drawFaces;
  m_drawStyle->displayEdges = drawEdges;
}

void SceneGraphManager::setProperty(int propertyId)
{
  if (m_currentPropertyId == propertyId)
    return;

  m_currentPropertyId = propertyId;

  if (propertyId >= 0 && propertyId < DerivedPropertyBaseId)
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
    if (step != m_flowLinesExpulsionStep)
    {
      m_flowLinesExpulsionStep = step;
      m_flowLinesTimeStamp = MxTimeStamp::getTimeStamp();

      updateSnapshot();
    }
  }
  else
  {
    if (step != m_flowLinesLeakageStep)
    {
      m_flowLinesLeakageStep = step;
      m_flowLinesTimeStamp = MxTimeStamp::getTimeStamp();

      updateSnapshot();
    }
  }
}

void SceneGraphManager::setFlowLinesThreshold(FlowLinesType type, double threshold)
{
  if (type == FlowLinesExpulsion)
  {
    if (threshold != m_flowLinesExpulsionThreshold)
    {
      m_flowLinesExpulsionThreshold = threshold;
      m_flowLinesTimeStamp = MxTimeStamp::getTimeStamp();

      updateSnapshot();
    }
  }
  else
  {
    if (threshold != m_flowLinesLeakageThreshold)
    {
      m_flowLinesLeakageThreshold = threshold;
      m_flowLinesTimeStamp = MxTimeStamp::getTimeStamp();

      updateSnapshot();
    }
  }
}

void SceneGraphManager::enableFormation(int formationId, bool enabled)
{
  if (m_formationVisibility[formationId] == enabled)
    return;

  m_formationVisibility[formationId] = enabled;
  m_formationsTimeStamp = MxTimeStamp::getTimeStamp();

  updateSnapshot();
}

void SceneGraphManager::enableAllFormations(bool enabled)
{
  for (size_t i = 0; i < m_formationVisibility.size(); ++i)
      m_formationVisibility[i] = enabled;

  m_formationsTimeStamp = MxTimeStamp::getTimeStamp();

  updateSnapshot();
}

void SceneGraphManager::enableSurface(int surfaceId, bool enabled)
{
  if (m_surfaceVisibility[surfaceId] == enabled)
    return;

  m_surfaceVisibility[surfaceId] = enabled;
  m_surfacesTimeStamp = MxTimeStamp::getTimeStamp();

  updateSnapshot();
}

void SceneGraphManager::enableAllSurfaces(bool enabled)
{
  for (size_t i = 0; i < m_surfaceVisibility.size(); ++i)
    m_surfaceVisibility[i] = enabled;

  m_surfacesTimeStamp = MxTimeStamp::getTimeStamp();

  updateSnapshot();
}

void SceneGraphManager::enableReservoir(int reservoirId, bool enabled)
{
  if (m_reservoirVisibility[reservoirId] == enabled)
    return;

  m_reservoirVisibility[reservoirId] = enabled;
  m_reservoirsTimeStamp = MxTimeStamp::getTimeStamp();

  updateSnapshot();
}

void SceneGraphManager::enableAllReservoirs(bool enabled)
{
  for (size_t i = 0; i < m_reservoirVisibility.size(); ++i)
    m_reservoirVisibility[i] = enabled;

  m_reservoirsTimeStamp = MxTimeStamp::getTimeStamp();

  updateSnapshot();
}

void SceneGraphManager::enableFault(int faultId, bool enabled)
{
  if (m_faultVisibility[faultId] == enabled)
    return;

  m_faultVisibility[faultId] = enabled;
  m_faultsTimeStamp = MxTimeStamp::getTimeStamp();

  updateSnapshot();
}

void SceneGraphManager::enableAllFaults(bool enabled)
{
  for (size_t i = 0; i < m_faultVisibility.size(); ++i)
    m_faultVisibility[i] = enabled;

  m_faultsTimeStamp = MxTimeStamp::getTimeStamp();

  updateSnapshot();
}

void SceneGraphManager::enableFlowLines(int flowLinesId, bool enabled)
{
  if (m_flowLinesVisibility[flowLinesId] == enabled)
    return;

  m_flowLinesVisibility[flowLinesId] = enabled;
  m_flowLinesTimeStamp = MxTimeStamp::getTimeStamp();

  updateSnapshot();
}

void SceneGraphManager::enableAllFlowLines(bool enabled)
{
  for (size_t i = 0; i < m_flowLinesVisibility.size(); ++i)
    m_flowLinesVisibility[i] = enabled;

  m_flowLinesTimeStamp = MxTimeStamp::getTimeStamp();

  updateSnapshot();
}

void SceneGraphManager::enableSlice(int slice, bool enabled)
{
  m_sliceEnabled[slice] = enabled;
  updateSnapshot();
}

void SceneGraphManager::setSlicePosition(int slice, int position)
{
  m_slicePosition[slice] = position;
  updateSnapshot();
}

void SceneGraphManager::showCoordinateGrid(bool show)
{
  if (show != m_showGrid)
  {
    m_showGrid = show;

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
  if (show != m_showCompass)
  {
    m_showCompass = show;

    m_compassSwitch->whichChild = show ? SO_SWITCH_ALL : SO_SWITCH_NONE;
  }
}

void SceneGraphManager::showText(bool show)
{
  if (show != m_showText)
  {
    m_showText = show;

    m_textSwitch->whichChild = show ? SO_SWITCH_ALL : SO_SWITCH_NONE;
  }
}

void SceneGraphManager::showTraps(bool show)
{
  if (show != m_showTraps)
  {
    m_showTraps = show;

    updateSnapshot();
  }
}

void SceneGraphManager::showTrapOutlines(bool show)
{
  if (show != m_showTrapOutlines)
  {
    m_showTrapOutlines = show;

    updateSnapshot();
  }
}

void SceneGraphManager::showDrainageAreaOutlines(DrainageAreaType type)
{
  if (type != m_drainageAreaType)
  {
    m_drainageAreaType = type;

    updateSnapshot();
  }
}

void SceneGraphManager::setup(std::shared_ptr<Project> project)
{
  m_project = project;
  m_projectInfo = project->getProjectInfo();

  //Project::SnapshotContents contents = project->getSnapshotContents(0);
  m_formationVisibility.assign(m_projectInfo.formations.size(), true);
  m_surfaceVisibility.assign(m_projectInfo.surfaces.size(), false);
  m_reservoirVisibility.assign(m_projectInfo.reservoirs.size(), false);
  m_faultVisibility.assign(m_projectInfo.faults.size(), false);
  m_flowLinesVisibility.assign(m_projectInfo.flowLines.size(), false);

  for (int i = 0; i < 3; ++i)
  {
    m_sliceEnabled[i] = false;
    m_slicePosition[i] = 0;
  }

  m_outlineBuilder = std::make_shared<OutlineBuilder>(
    m_projectInfo.dimensions.numCellsIHiRes,
    m_projectInfo.dimensions.numCellsJHiRes);

  //m_maxPersistentTrapId = getMaxPersistentTrapId(handle);

  setupSceneGraph();
  setCurrentSnapshot(0);
}

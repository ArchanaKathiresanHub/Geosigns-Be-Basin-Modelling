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
#include <Inventor/nodes/SoText2.h>
#include <Inventor/nodes/SoLineSet.h>
#include <Inventor/nodes/SoBaseColor.h>
#include <Inventor/nodes/SoIndexedLineSet.h>

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

#include <MeshViz/graph/PoAutoCubeAxis.h>
#include <MeshViz/graph/PoLinearAxis.h>
#include <MeshViz/graph/PoGenAxis.h>

#include <Interface/ProjectHandle.h>
#include <Interface/Grid.h>
#include <Interface/GridMap.h>
#include <Interface/Property.h>
#include <Interface/PropertyValue.h>
#include <Interface/Formation.h>
#include <Interface/Surface.h>
#include <Interface/Reservoir.h>
#include <Interface/Snapshot.h>
#include <Interface/FaultCollection.h>
#include <Interface/Faulting.h>
#include <Interface/Trap.h>
#include <Interface/Trapper.h>

#include <memory>

namespace di = DataAccess::Interface;

SoSwitch* createCompass();

SnapshotInfo::SnapshotInfo()
  : minZ(0.0)
  , maxZ(0.0)
  , snapshot(0)
  , currentProperty(0)
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
{
  for (int i = 0; i < 3; ++i)
  {
    sliceSwitch[i] = 0;
    slice[i] = 0;
  }
}

void SceneGraphManager::updateCoordinateGrid()
{
  assert(!m_snapshotInfoCache.empty());

  SnapshotInfo& snapshot = *m_snapshotInfoCache.begin();

  float sizeX = (float)(m_maxX - m_minX);
  float sizeY = (float)(m_maxY - m_minY);
  float sizeZ = (float)(snapshot.maxZ - snapshot.minZ);

  const float margin = .05f;
  float dx = sizeX * margin;
  float dy = sizeY * margin;

  SbVec3f start(-dx, -dy, -sizeZ * m_verticalScale);
  SbVec3f end(sizeX + dx, sizeY + dy, 0.0f);
  SbVec3f gradStart(start[0], start[1], (float)-snapshot.minZ);
  SbVec3f gradEnd(end[0], end[1], (float)-snapshot.maxZ);

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
  if (snapshot.formationsTimeStamp == m_formationsTimeStamp || snapshot.chunksGroup == 0)
    return;

  snapshot.chunksGroup->removeAllChildren();

  bool buildingChunk = false;
  int minK=0, maxK=0;

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
    uint32_t rangeMin[] = { 0, 0, (uint32_t)tmpChunks[i].minK };
    uint32_t rangeMax[] = { 
      (uint32_t)(m_numI - 1), 
      (uint32_t)(m_numJ - 1), 
      (uint32_t)(tmpChunks[i].maxK - 1) };

    meshSkin->minCellRanges.setValues(0, 3, rangeMin);
    meshSkin->maxCellRanges.setValues(0, 3, rangeMax);
#ifdef _DEBUG
    meshSkin->parallel = false;
#endif
    snapshot.chunksGroup->addChild(meshSkin);
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

  // A PersistentTrapIdProperty consists of the values of the ResRockTrapId property, combined with
  // a translation table to convert the regular trap ids to persistent trap ids. This function builds
  // the translation table, gets the ResRockTrapId values, and creates the PersistentTrapIdProperty.
  std::shared_ptr<PersistentTrapIdProperty> createPersistentTrapIdProperty(
    const di::Property* trapIdProperty, const di::Reservoir* reservoir, const di::Snapshot* snapshot)
  {
    di::ProjectHandle* handle = snapshot->getProjectHandle();

    // Create a mapping from id -> persistentId for all traps in this snapshot
    std::unique_ptr<di::TrapperList> trappers(handle->getTrappers(reservoir, snapshot, 0, 0));
    if (trappers->empty())
      return nullptr;

    // Sort on id
    std::sort(trappers->begin(), trappers->end(),
      [](const di::Trapper* t1, const di::Trapper* t2) { return t1->getId() < t2->getId(); });

    // Create translation table
    unsigned int minId = (*trappers)[0]->getId();
    unsigned int maxId = (*trappers)[trappers->size() - 1]->getId();
    unsigned int index = 0;
    std::vector<unsigned int> table;
    for (unsigned int id = minId; id <= maxId; ++id)
    {
      if (id == (*trappers)[index]->getId())
        table.push_back((*trappers)[index++]->getPersistentId());
      else
        table.push_back(0); // add 0 if there is no trapper with this id
    }

    // Get the property values
    std::unique_ptr<di::PropertyValueList> trapIdValues(
      trapIdProperty->getPropertyValues(di::RESERVOIR, snapshot, reservoir, 0, 0));
    if (trapIdValues->empty())
      return nullptr;

    const di::GridMap* trapIds = (*trapIdValues)[0]->getGridMap();
    return std::make_shared<PersistentTrapIdProperty>(trapIds, table, minId);
  }

  // Utility function to get the maximum persistent trap id. This is useful for constructing
  // a colormap for the trap id property.
  unsigned int getMaxPersistentTrapId(const di::ProjectHandle* handle)
  {
    unsigned int maxPersistentId = 0;

    std::unique_ptr<di::TrapperList> trappers(handle->getTrappers(nullptr, nullptr, 0, 0));
    for (auto trapper : *trappers)
      maxPersistentId = std::max(maxPersistentId, trapper->getPersistentId());

    return maxPersistentId;
  }

  std::shared_ptr<MiDataSetIj<double> > createSurfaceProperty(const di::Property* prop, const di::Surface* surface, const di::Snapshot* snapshot)
  {
    if (!prop || prop->getPropertyAttribute() != DataModel::CONTINUOUS_3D_PROPERTY)
      return nullptr;

    std::unique_ptr<di::PropertyValueList> values(
      prop->getPropertyValues(di::SURFACE, snapshot, nullptr, nullptr, surface));

    if (!values || values->empty())
      return nullptr;

    return std::make_shared<SurfaceProperty>(prop->getName(), (*values)[0]->getGridMap());
  }

  MiDataSetIjk<double>* createFormationProperty(const di::Property* prop, const SnapshotInfo& info)
  {
    return nullptr;
  }

  std::shared_ptr<ReservoirProperty> createReservoirProperty(const di::Property* prop, const di::Reservoir* reservoir, const di::Snapshot* snapshot)
  {
    if (!prop)
      return nullptr;

    if (prop->getName() == "ResRockTrapId")
      return createPersistentTrapIdProperty(prop, reservoir, snapshot);
      
    DataModel::PropertyAttribute attr = prop->getPropertyAttribute();
    DataAccess::Interface::PropertyType type = prop->getType();

    if (attr != DataModel::FORMATION_2D_PROPERTY || type != di::RESERVOIRPROPERTY)
      return nullptr;

    std::unique_ptr<di::PropertyValueList> values(
      prop->getPropertyValues(di::RESERVOIR, snapshot, reservoir, nullptr, nullptr));

    if (!values || values->empty())
      return nullptr;

    return std::make_shared<ReservoirProperty>(prop->getName(), (*values)[0]->getGridMap());
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
    int id = surf.id;

    if (m_surfaces[id].visible && surf.root == 0)
    {
      const di::Surface* surface = m_surfaces[id].object;
      std::unique_ptr<di::PropertyValueList> values(m_projectHandle->getPropertyValues(
        di::SURFACE, m_depthProperty, snapshot.snapshot, 0, 0, surface, di::MAP));

      assert(values->size() == 1); //TODO: should not be an assert

      surf.meshData = std::make_shared<SurfaceMesh>((*values)[0]->getGridMap());
      surf.mesh = new MoMesh;
      surf.mesh->setMesh(surf.meshData.get());
      surf.scalarSet = new MoScalarSetIj;
      surf.propertyData = createSurfaceProperty(snapshot.currentProperty, surface, snapshot.snapshot);
      surf.scalarSet->setScalarSet(surf.propertyData.get());
      surf.surfaceMesh = new MoMeshSurface;

      SoSeparator* root = new SoSeparator;
      root->addChild(surf.mesh);
      root->addChild(surf.scalarSet);
      root->addChild(surf.surfaceMesh);

      surf.root = root;
      snapshot.surfacesGroup->addChild(root);
    }
    else if (!m_surfaces[id].visible && surf.root != 0)
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
    int id = res.id;

    if (m_reservoirs[id].visible && res.root == 0)
    {
      const di::Reservoir* reservoir = m_reservoirs[id].object;
      std::unique_ptr<di::PropertyValueList> topValues(m_projectHandle->getPropertyValues(
        di::RESERVOIR, m_resRockTopProperty, snapshot.snapshot, reservoir, 0, 0, di::MAP));
      std::unique_ptr<di::PropertyValueList> bottomValues(m_projectHandle->getPropertyValues(
        di::RESERVOIR, m_resRockBottomProperty, snapshot.snapshot, reservoir, 0, 0, di::MAP));

      if (topValues && !topValues->empty() && bottomValues && !bottomValues->empty())
      {
        res.root = new SoSeparator;
        res.mesh = new MoMesh;

        res.meshData = std::make_shared<ReservoirMesh>((*topValues)[0]->getGridMap(), (*bottomValues)[0]->getGridMap());
        res.mesh->setMesh(res.meshData.get());

        res.propertyData = createReservoirProperty(snapshot.currentProperty, reservoir, snapshot.snapshot);
        res.scalarSet = new MoScalarSetIjk;
        res.scalarSet->setScalarSet(res.propertyData.get());

        res.skin = new MoMeshSkin;

        res.root->addChild(res.mesh);
        res.root->addChild(res.scalarSet);
        res.root->addChild(res.skin);

        snapshot.reservoirsGroup->addChild(res.root);
      }
    }
    else if (!m_reservoirs[id].visible && res.root != 0)
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
    int id = res.id;

    if (m_reservoirs[id].visible)
    {
      // See if the vertical scale needs updating for existing traps
      if (m_showTraps && res.traps.root() != 0 && res.traps.verticalScale() != m_verticalScale)
      {
        res.traps.setVerticalScale(m_verticalScale);
      }
      // See if we need to create new traps
      else if (m_showTraps && res.traps.root() == 0)
      {
        res.traps = Traps(snapshot.snapshot, m_reservoirs[id].object, m_verticalScale);
        if (res.traps.root() != 0)
          res.root->addChild(res.traps.root());
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
          res.trapOutlines = m_outlineBuilder->createOutline(
            snapshot.snapshot, m_reservoirs[id].object, m_resRockTrapIdProperty, m_resRockTopProperty);

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

      if (m_drainageAreaType == DrainageAreaFluid && !res.drainageAreaOutlinesFluid && m_resRockDrainageIdFluidPhaseProperty)
      {
        res.drainageAreaOutlinesFluid = m_outlineBuilder->createOutline(
          snapshot.snapshot, m_reservoirs[id].object, m_resRockDrainageIdFluidPhaseProperty, m_resRockTopProperty);

        res.root->addChild(res.drainageAreaOutlinesFluid);
      }
      else if (m_drainageAreaType == DrainageAreaGas && !res.drainageAreaOutlinesGas && m_resRockDrainageIdGasPhaseProperty)
      {
        res.drainageAreaOutlinesGas = m_outlineBuilder->createOutline(
          snapshot.snapshot, m_reservoirs[id].object, m_resRockDrainageIdGasPhaseProperty, m_resRockTopProperty);

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
  double getZ(const SnapshotGeometry& geometry, double x, double y, int k)
  {
    MbVec3d minVec = geometry.getMin();
    MbVec3d maxVec = geometry.getMax();

    x = std::max(std::min(x, maxVec[0]), minVec[0]);
    y = std::max(std::min(y, maxVec[1]), minVec[1]);

    double normX = (x - minVec[0]) / geometry.deltaX();
    double normY = (y - minVec[1]) / geometry.deltaY();

    int i0 = (int)floor(normX);
    int j0 = (int)floor(normY);
    // Take care not to read outside the bounds of the 3D array
    int i1 = std::min(i0 + 1, (int)geometry.numI() - 1);
    int j1 = std::min(j0 + 1, (int)geometry.numJ() - 1);

    double di = normX - i0;
    double dj = normY - j0;

    double z00 = -geometry.getDepth(i0, j0, k);
    double z01 = -geometry.getDepth(i0, j1, k);
    double z10 = -geometry.getDepth(i1, j0, k);
    double z11 = -geometry.getDepth(i1, j1, k);

    return lerp(
      lerp(z00, z01, dj),
      lerp(z10, z11, dj),
      di);
  }
}

std::shared_ptr<FaultMesh> generateFaultMesh(const std::vector<SbVec2d>& points, const SnapshotGeometry& geometry, int k0, int k1)
{
  std::vector<MbVec3d> coords;
  coords.reserve(2 * points.size());

  for (auto p : points)
  {
    double z0 = getZ(geometry, p[0], p[1], k0);
    double z1 = getZ(geometry, p[0], p[1], k1);

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

    if (m_faults[id].visible && snapshot.faults[i].root == 0)
    {
      const di::PointSequence& points = m_faults[id].object->getFaultLine();

      std::vector<SbVec2d> oivpoints;
      oivpoints.reserve(points.size());
      for (auto p : points)
        oivpoints.emplace_back(p(di::X_COORD) - m_minX, p(di::Y_COORD) - m_minY);
      
      snapshot.faults[i].meshData = generateFaultMesh(oivpoints, *snapshot.geometry, snapshot.faults[i].minK, snapshot.faults[i].maxK);
      snapshot.faults[i].root = new SoSeparator;
      snapshot.faults[i].mesh = new MoMesh;
      snapshot.faults[i].mesh->setMesh(snapshot.faults[i].meshData.get());
      snapshot.faults[i].surfaceMesh = new MoMeshSurface;

      snapshot.faults[i].root->addChild(snapshot.faults[i].mesh);
      snapshot.faults[i].root->addChild(snapshot.faults[i].surfaceMesh);
      snapshot.faultsGroup->addChild(snapshot.faults[i].root);
    }
    else if (!m_faults[id].visible && snapshot.faults[i].root != 0)
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

std::vector<const di::GridMap*> SceneGraphManager::getFormationPropertyGridMaps(const SnapshotInfo& snapshot, const di::Property* prop, bool formation3D) const
{
  std::vector<const di::GridMap*> gridMaps;

  std::unique_ptr<di::PropertyValueList> values(prop->getPropertyValues(di::FORMATION, snapshot.snapshot, 0, 0, 0));

  // This is necessary to filter out duplicate entries in the property value list. For some reason the project3D file
  // can contain multiple entries for the same property value, corresponding to multiple simulation runs. The HDF5 files
  // get overwritten each time, so there's only one actual property value available anyway. Since the list is already 
  // sorted by depo age, we can filter duplicates by checking the associated formations.
  size_t n = std::distance(
    values->begin(), 
    std::unique(
      values->begin(), 
      values->end(), 
      [](const di::PropertyValue* v0, const di::PropertyValue* v1) 
        { 
          return v0->getFormation() == v1->getFormation(); 
        }));

  size_t i = 0;
  for (auto fmt : snapshot.formations)
  {
    const di::Formation* formation = m_formations[fmt.id].object;
    const di::GridMap* gridMap = nullptr;
    if (i < n && formation == (*values)[i]->getFormation())
      gridMap = (*values)[i++]->getGridMap();

    if (formation3D && gridMap)
    {
      gridMaps.push_back(gridMap);
    }
    else
    {
      for (int k = fmt.minK; k < fmt.maxK; ++k)
        gridMaps.push_back(gridMap);
    }
  }

  return gridMaps;
}

std::shared_ptr<MiDataSetIjk<double> > SceneGraphManager::createFormation2DProperty(
  const std::string& name, 
  const SnapshotInfo& snapshot,
  const di::Property* prop) const
{
  std::vector<const di::GridMap*> gridMaps = getFormationPropertyGridMaps(snapshot, prop, false);

  if (gridMaps.empty())
    return nullptr;

  return std::make_shared<Formation2DProperty>(name, gridMaps);
}

std::shared_ptr<MiDataSetIjk<double> > SceneGraphManager::createFormation3DProperty(
  const std::string& name, 
  const SnapshotInfo& snapshot,
  const di::Property* prop) const
{
  std::vector<const di::GridMap*> gridMaps = getFormationPropertyGridMaps(snapshot, prop, true);

  if (gridMaps.empty())
    return nullptr;

  return std::make_shared<FormationProperty>(name, gridMaps);
}

std::shared_ptr<MiDataSetIjk<double> > SceneGraphManager::createFormationProperty(const SnapshotInfo& snapshot, const di::Property* prop) const
{
  if (!prop || prop->getType() != di::FORMATIONPROPERTY)
    return nullptr;

  std::string name = prop->getName();

  if (prop->getPropertyAttribute() == DataModel::FORMATION_2D_PROPERTY)
    return createFormation2DProperty(name, snapshot, prop);
  else
    return createFormation3DProperty(name, snapshot, prop);
}

void SceneGraphManager::updateSnapshotProperties()
{
  assert(!m_snapshotInfoCache.empty());

  SnapshotInfo& snapshot = *m_snapshotInfoCache.begin();

  // Update properties
  if (snapshot.currentProperty == m_currentProperty)
    return;

  snapshot.scalarDataSet = createFormationProperty(snapshot, m_currentProperty);
  snapshot.scalarSet->setScalarSet(snapshot.scalarDataSet.get());

  for (auto &surf : snapshot.surfaces)
  {
    if (surf.root)
    {
      surf.propertyData = createSurfaceProperty(m_currentProperty, m_surfaces[surf.id].object, snapshot.snapshot);
      surf.scalarSet->setScalarSet(surf.propertyData.get());
    }
  }

  for (auto &res : snapshot.reservoirs)
  {
    if (res.root)
    {
      res.propertyData = createReservoirProperty(m_currentProperty, m_reservoirs[res.id].object, snapshot.snapshot);
      res.scalarSet->setScalarSet(res.propertyData.get());
    }
  }

  snapshot.currentProperty = m_currentProperty;
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

  if (m_flowVizType == FlowVizNone)
  {
    snapshot.flowDirDataSet.reset();
    snapshot.flowLinesGroup->removeAllChildren();
    snapshot.flowVectorsGroup->removeAllChildren();
  }
  else
  {
    if (!snapshot.flowDirDataSet)
    {
      std::vector<const di::GridMap*> gridMaps = getFormationPropertyGridMaps(snapshot, m_flowDirectionProperty, true);
      if (gridMaps.empty())
        return;
      else
        snapshot.flowDirDataSet = std::make_shared<FlowDirectionProperty>(gridMaps, *snapshot.topology);
    }

    if (m_flowVizType == FlowVizLines && snapshot.flowLinesGroup->getNumChildren() == 0)
    {
      // Add color node
      SoBaseColor* color = new SoBaseColor;
      color->rgb.setValue(1.f, .5f, 1.f);
      snapshot.flowLinesGroup->addChild(color);

      for (auto& fmt : snapshot.formations)
      {
        const di::Formation* formation = m_formations[fmt.id].object;
        if (formation->isSourceRock())
        {
          SoLineSet* flowLines = generateFlowLines(*snapshot.flowDirDataSet, fmt.minK, *snapshot.topology);
          snapshot.flowLinesGroup->addChild(flowLines);
        }
      }

      snapshot.flowVectorsGroup->removeAllChildren();
    }
    else if (m_flowVizType == FlowVizVectors && snapshot.flowVectorsGroup->getNumChildren() == 0)
    {
      // Add color node
      SoBaseColor* color = new SoBaseColor;
      color->rgb.setValue(1.f, 1.f, .5f);
      snapshot.flowVectorsGroup->addChild(color);

      snapshot.flowDirSet = new MoVec3SetIjk;
      snapshot.flowDirSet->setVec3Set(snapshot.flowDirDataSet.get());
      snapshot.flowVectorsGroup->addChild(snapshot.flowDirSet);

      MoMeshVector* meshVector = new MoMeshVector;
      meshVector->colorScalarSetId = -1;
      meshVector->scaleFactor = .5f;
      snapshot.flowVectorsGroup->addChild(meshVector);

      snapshot.flowLinesGroup->removeAllChildren();
    }
  }
}

void SceneGraphManager::updateColorMap()
{
  if (!m_currentProperty)
    return;

  m_colorMapSwitch->whichChild = (m_currentProperty == m_resRockTrapIdProperty) ? 1 : 0;

  assert(!m_snapshotInfoCache.empty());

  SnapshotInfo& snapshot = *m_snapshotInfoCache.begin();

  double minValue =  std::numeric_limits<double>::max();
  double maxValue = -std::numeric_limits<double>::max();

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

  static_cast<MoPredefinedColorMapping*>(m_colorMap)->minValue = (float)minValue;
  static_cast<MoPredefinedColorMapping*>(m_colorMap)->maxValue = (float)maxValue;

  m_legend->minValue = minValue;
  m_legend->maxValue = maxValue;
}

void SceneGraphManager::updateText()
{
  assert(!m_snapshotInfoCache.empty());

  SnapshotInfo& snapshot = *m_snapshotInfoCache.begin();
  size_t i = std::distance(
    m_snapshotList.begin(), 
    std::find(
      m_snapshotList.begin(), 
      m_snapshotList.end(), 
      snapshot.snapshot));

  SbString str1, str2;
  str1.sprintf("Snapshot %d/%d (Age %.1f)", (int)(i + 1), (int)m_snapshotList.size(), snapshot.snapshot->getTime());
  str2.sprintf("Resolution %dx%d (%dx%d)", m_numI, m_numJ, m_numIHiRes, m_numJHiRes);

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

    //axis->titleFontSize = 0.10F;
    //axis->setMiscTextAttr(textAtt);
    //axis->gradFontSize = 0.06F;
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

SnapshotInfo SceneGraphManager::createSnapshotNode(const di::Snapshot* snapshot)
{
  SnapshotInfo info;
  info.snapshot = snapshot;

  std::vector<const di::GridMap*> depthMaps;
  
  int k = 0;

  // Get a list of all the formation depth values in this snapshot. This list is already 
  // sorted from top to bottom when we get it from DataAccess lib
  std::unique_ptr<di::PropertyValueList> depthValues(m_projectHandle->getPropertyValues(
    di::FORMATION, m_depthProperty, snapshot, 0, 0, 0, di::VOLUME));

  for (auto depthValue : *depthValues)
  {
    auto formation = depthValue->getFormation();
    if (formation->kind() == di::BASEMENT_FORMATION)
      continue;

    depthMaps.push_back(depthValue->getGridMap());

    SnapshotInfo::Formation bounds;
    bounds.id = m_formationIdMap[formation->getName()];
    bounds.minK = k;
    bounds.maxK = k + depthMaps.back()->getDepth() - 1;
    info.formations.push_back(bounds);

    k = bounds.maxK;
  }

  // Generate formation ids list
  std::vector<double> formationIds;
  for (auto bounds : info.formations)
  {
    int depth = bounds.maxK - bounds.minK;
    for (int i = 0; i < depth; ++i)
      formationIds.push_back((double)bounds.id);
  }

  // Get faults for each formation
  for (auto bounds : info.formations)
  {
    auto formation = m_formations[bounds.id].object;

    // Add faults, if any
    std::unique_ptr<di::FaultCollectionList> faultCollections(formation->getFaultCollections());
    if (!faultCollections)
      continue;

    for (auto collection : *faultCollections)
    {
      std::unique_ptr<di::FaultList> faults(collection->getFaults());
      if (!faults)
        continue;

      for (auto fault : *faults)
      {
        SnapshotInfo::Fault flt;
        flt.id = m_faultIdMap[std::make_tuple(collection->getName(), fault->getName())];
        flt.minK = bounds.minK;
        flt.maxK = bounds.maxK;

        info.faults.push_back(flt);
      }
    }
  }

  std::unique_ptr<di::FormationList> formationList(m_projectHandle->getFormations(snapshot, false));

  // Add reservoirs
  for (auto formation : *formationList)
  {
    std::unique_ptr<di::ReservoirList> reservoirList(formation->getReservoirs());
    if (!reservoirList)
      continue;

    for (auto reservoir : *reservoirList)
    {
      SnapshotInfo::Reservoir res;
      res.id = m_reservoirIdMap[reservoir->getName()];

      info.reservoirs.push_back(res);
    }
  }

  // Add surfaces
  std::unique_ptr<di::SurfaceList> surfaces(m_projectHandle->getSurfaces(snapshot, false));
  for (size_t i = 0; i < surfaces->size(); ++i)
  {
    SnapshotInfo::Surface srfc;
    srfc.id = m_surfaceIdMap[(*surfaces)[i]->getName()];

    info.surfaces.push_back(srfc);
  }

  // Build the scenegraph
  info.root = new SoSeparator;
  info.root->ref();
  info.formationsRoot = new SoSeparator;
  info.formationsRoot->setName("formations");

  if (!depthMaps.empty())
  {
    info.geometry = std::make_shared<SnapshotGeometry>(depthMaps);
    info.topology = std::make_shared<SnapshotTopology>(info.geometry);
    info.meshData = std::make_shared<HexahedronMesh>(info.geometry, info.topology);

    // Find minimum and maximum depth value
    double minValue1, maxValue1, minValue2, maxValue2;
    depthMaps[0]->getMinMaxValue(minValue1, maxValue1);
    depthMaps[depthMaps.size() - 1]->getMinMaxValue(minValue2, maxValue2);

    info.minZ = -std::max(maxValue1, maxValue2);
    info.maxZ = -std::min(minValue1, minValue2);
  }

  info.mesh = new MoMesh;
  info.mesh->setName("snapshotMesh");
  info.mesh->setMesh(info.meshData.get());

  info.scalarDataSet = std::make_shared<FormationIdProperty>(formationIds);

  info.scalarSet = new MoScalarSetIjk;
  info.scalarSet->setName("formationID");
  info.scalarSet->setScalarSet(info.scalarDataSet.get());

  info.chunksGroup = new SoGroup;
  info.chunksGroup->setName("chunks");
  info.flowLinesGroup = new SoSeparator;
  info.flowLinesGroup->setName("flowlines");
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
  info.formationsRoot->addChild(info.scalarSet);
  info.formationsRoot->addChild(info.chunksGroup);
  info.formationsRoot->addChild(info.flowLinesGroup);
  info.formationsRoot->addChild(info.flowVectorsGroup);
  info.formationsRoot->addChild(info.slicesGroup);

  info.root->addChild(info.formationsRoot);
  // Add surfaceShapeHints to prevent backface culling, and enable double-sided lighting
  info.root->addChild(m_surfaceShapeHints);
  info.root->addChild(info.surfacesGroup);
  info.root->addChild(info.reservoirsGroup);
  info.root->addChild(info.faultsGroup);

  return info;
}

void SceneGraphManager::setupSnapshots()
{
  std::unique_ptr<di::SnapshotList> snapshots(m_projectHandle->getSnapshots(di::MAJOR));

  // Sort snapshots so oldest is first in list
  m_snapshotList = *snapshots;
  std::sort(m_snapshotList.begin(), m_snapshotList.end(), 
    [](const di::Snapshot* s1, const di::Snapshot* s2) { return s1->getTime() > s2->getTime(); });
}

void SceneGraphManager::setupCoordinateGrid()
{
  m_coordinateGrid = initCoordinateGrid(m_minX, m_minY, m_maxX, m_maxY);
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

  m_drawStyle = new MoDrawStyle;
  m_drawStyle->displayFaces = true;
  m_drawStyle->displayEdges = true;
  m_drawStyle->displayPoints = false;
  m_drawStyle->fadingThreshold = 20.0f; // does this actually work???

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
  m_trapIdColorMap = createTrapsColorMap(m_maxPersistentTrapId);
  m_colorMapSwitch = new SoSwitch;
  m_colorMapSwitch->addChild(m_colorMap);
  m_colorMapSwitch->addChild(m_trapIdColorMap);
  m_colorMapSwitch->whichChild = 0;

  m_appearanceNode = new SoGroup;
  m_appearanceNode->setName("appearance");
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

  m_annotation = new SoAnnotation;
  m_annotation->setName("annotation");
  m_annotation->boundingBoxIgnoring = true;
  m_annotation->addChild(legendDrawStyle);
  m_annotation->addChild(m_legendSwitch);

  // Text area
  m_text = new SoText2;
  m_text->string.set1Value(0, "");
  SoOrthographicCamera* camera = new SoOrthographicCamera;
  camera->viewportMapping = SoCamera::LEAVE_ALONE;
  SoTranslation* translation = new SoTranslation;
  translation->translation.setValue(-.99f, .99f, 0.f);
  SoFont* font = new SoFont;
  font->name = "Arial";
  font->size = 14.f;
  font->renderStyle = SoFont::TEXTURE;
  SoSeparator* textSeparator = new SoSeparator;
  textSeparator->addChild(camera);
  textSeparator->addChild(translation);
  textSeparator->addChild(font);
  textSeparator->addChild(m_text);
  m_textSwitch = new SoSwitch;
  m_textSwitch->addChild(textSeparator);
  m_textSwitch->whichChild = SO_SWITCH_ALL;
  m_annotation->addChild(m_textSwitch);

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
  m_root->addChild(createCompass());

  setupSnapshots();

  static_cast<MoPredefinedColorMapping*>(m_colorMap)->maxValue = (float)(m_formationIdMap.size() - 1);
}

SceneGraphManager::SceneGraphManager()
  : m_projectHandle(0)
  , m_depthProperty(0)
  , m_resRockTopProperty(0)
  , m_resRockBottomProperty(0)
  , m_resRockTrapIdProperty(0)
  , m_resRockDrainageIdGasPhaseProperty(0)
  , m_resRockDrainageIdFluidPhaseProperty(0)
  , m_flowDirectionProperty(0)
  , m_currentProperty(0)
  , m_numI(0)
  , m_numJ(0)
  , m_numIHiRes(0)
  , m_numJHiRes(0)
  , m_deltaI(0.0)
  , m_deltaJ(0.0)
  , m_minX(0.0)
  , m_minY(0.0)
  , m_maxX(0.0)
  , m_maxY(0.0)
  , m_maxPersistentTrapId(0)
  , m_maxCacheItems(5)
  , m_outlineBuilder(std::make_shared<OutlineBuilder>())
  , m_showGrid(false)
  , m_showTraps(false)
  , m_showTrapOutlines(false)
  , m_drainageAreaType(DrainageAreaNone)
  , m_flowVizType(FlowVizNone)
  , m_verticalScale(1.f)
  , m_projectionType(PerspectiveProjection)
  , m_formationsTimeStamp(MxTimeStamp::getTimeStamp())
  , m_surfacesTimeStamp(MxTimeStamp::getTimeStamp())
  , m_reservoirsTimeStamp(MxTimeStamp::getTimeStamp())
  , m_faultsTimeStamp(MxTimeStamp::getTimeStamp())
  , m_root(0)
  , m_formationShapeHints(0)
  , m_surfaceShapeHints(0)
  , m_coordinateGrid(0)
  , m_coordinateGridSwitch(0)
  , m_scale(0)
  , m_appearanceNode(0)
  , m_drawStyle(0)
  , m_material(0)
  , m_dataBinding(0)
  , m_colorMap(0)
  , m_trapIdColorMap(0)
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

void SceneGraphManager::setCurrentSnapshot(size_t index)
{
  const di::Snapshot* snapshot = m_snapshotList[index];

  // Don't do anything if this is already the current snapshot
  if (!m_snapshotInfoCache.empty() && m_snapshotInfoCache.begin()->snapshot == snapshot)
    return;

  // See if we have this node in the cache
  auto iter = std::find_if(
    m_snapshotInfoCache.begin(), 
    m_snapshotInfoCache.end(), 
    [snapshot](const SnapshotInfo& info) { return info.snapshot == snapshot; });

  if (iter == m_snapshotInfoCache.end()) // not available
  {
    // Create a new node, and add it to the front of the cache
    SnapshotInfo newNode = createSnapshotNode(snapshot);
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

size_t SceneGraphManager::getSnapshotCount() const
{
  return m_snapshotList.size();
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

void SceneGraphManager::setRenderStyle(bool drawFaces, bool drawEdges)
{
  m_drawStyle->displayFaces = drawFaces;
  m_drawStyle->displayEdges = drawEdges;
}

void SceneGraphManager::setProperty(const std::string& name)
{
  const di::Property* prop = m_projectHandle->findProperty(name);
  if (prop == 0 || prop == m_currentProperty)
    return;

  m_currentProperty = prop;

  std::string title = name + " [" + prop->getUnit() + "]";
  m_legend->title = title.c_str();
  m_legendSwitch->whichChild = SO_SWITCH_ALL;

  updateSnapshot();
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

  updateSnapshot();
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

  updateSnapshot();
}

void SceneGraphManager::enableReservoir(const std::string& name, bool enabled)
{
  auto iter = m_reservoirIdMap.find(name);
  if (iter == m_reservoirIdMap.end())
    return;

  int id = iter->second;
  if (m_reservoirs[id].visible == enabled)
    return;

  m_reservoirs[id].visible = enabled;
  m_reservoirsTimeStamp = MxTimeStamp::getTimeStamp();

  updateSnapshot();
}

void SceneGraphManager::enableFault(const std::string& collectionName, const std::string& name, bool enabled)
{
  auto iter = m_faultIdMap.find(std::make_tuple(collectionName, name));
  if (iter == m_faultIdMap.end())
    return;

  int id = iter->second;
  if (m_faults[id].visible == enabled)
    return;

  m_faults[id].visible = enabled;
  m_faultsTimeStamp = MxTimeStamp::getTimeStamp();

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

void SceneGraphManager::showFlowDirection(FlowVizType type)
{
  if (type != m_flowVizType)
  {
    m_flowVizType = type;

    updateSnapshot();
  }
}

void SceneGraphManager::setup(const di::ProjectHandle* handle)
{
  m_projectHandle = handle;

  const std::string depthKey = "Depth";
  const std::string resRockTopKey = "ResRockTop";
  const std::string resRockBottomKey = "ResRockBottom";
  const std::string resRockTrapIdKey = "ResRockTrapId";
  const std::string resRockDrainageIdGasPhasePropertyKey = "ResRockDrainageIdGasPhase"; 
  const std::string resRockDrainageIdFluidPhasePropertyKey = "ResRockDrainageIdFluidPhase";
  const std::string flowDirectionKey = "FlowDirectionIJK";

  m_depthProperty = handle->findProperty(depthKey);
  m_resRockTopProperty = handle->findProperty(resRockTopKey);
  m_resRockBottomProperty = handle->findProperty(resRockBottomKey);
  m_resRockTrapIdProperty = handle->findProperty(resRockTrapIdKey);
  m_resRockDrainageIdGasPhaseProperty = handle->findProperty(resRockDrainageIdGasPhasePropertyKey);
  m_resRockDrainageIdFluidPhaseProperty = handle->findProperty(resRockDrainageIdFluidPhasePropertyKey);
  m_flowDirectionProperty = handle->findProperty(flowDirectionKey);

  const di::Grid* loresGrid = handle->getLowResolutionOutputGrid();
  const di::Grid* hiresGrid = handle->getHighResolutionOutputGrid();

  m_numI = loresGrid->numI();
  m_numJ = loresGrid->numJ();
  m_numIHiRes = hiresGrid->numI();
  m_numJHiRes = hiresGrid->numJ();
  m_deltaI = loresGrid->deltaI();
  m_deltaJ = loresGrid->deltaJ();
  m_minX = loresGrid->minI();
  m_minY = loresGrid->minJ();
  m_maxX = loresGrid->maxI();
  m_maxY = loresGrid->maxJ();

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
    info.object = (*formations)[i];
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
    info.object = (*surfaces)[i];
    info.id = (int)i;
    info.visible = false;

    m_surfaces.push_back(info);
  }

  // Get reservoirs
  std::unique_ptr<di::ReservoirList> reservoirs(handle->getReservoirs());
  for (size_t i = 0; i < reservoirs->size(); ++i)
  {
    m_reservoirIdMap[(*reservoirs)[i]->getName()] = (int)i;

    ReservoirInfo info;
    info.object = (*reservoirs)[i];
    info.id = (int)i;
    info.visible = false;

    m_reservoirs.push_back(info);
  }

  // Get faults
  std::unique_ptr<di::FaultCollectionList> faultCollections(handle->getFaultCollections(0));
  if (faultCollections && !faultCollections->empty())
  {
    for (size_t i = 0; i < faultCollections->size(); ++i)
    {
      std::unique_ptr<di::FaultList> faults((*faultCollections)[i]->getFaults());
      for (size_t j = 0; j < faults->size(); ++j)
      {
        m_faultIdMap[
          std::make_tuple(
            (*faultCollections)[i]->getName(), 
            (*faults)[j]->getName())] = (int)j;

        FaultInfo info;
        info.object = (*faults)[j];
        info.id = (int)j;
        info.visible = false;

        m_faults.push_back(info);
      }
    }
  }

  m_maxPersistentTrapId = getMaxPersistentTrapId(handle);

  setupSceneGraph();
  setCurrentSnapshot(0);
}

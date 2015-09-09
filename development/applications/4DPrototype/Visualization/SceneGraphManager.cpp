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
#include <Inventor/nodes/SoVertexProperty.h>
#include <Inventor/nodes/SoMaterial.h>
#include <Inventor/nodes/SoPerspectiveCamera.h>
#include <Inventor/nodes/SoOrthographicCamera.h>

#include <MeshVizInterface/MxTimeStamp.h>
#include <MeshVizInterface/mapping/nodes/MoDrawStyle.h>
#include <MeshVizInterface/mapping/nodes/MoMaterial.h>
#include <MeshVizInterface/mapping/nodes/MoDataBinding.h>
#include <MeshVizInterface/mapping/nodes/MoPredefinedColorMapping.h>
#include <MeshVizInterface/mapping/nodes/MoLevelColorMapping.h>
#include <MeshVizInterface/mapping/nodes/MoMesh.h>
#include <MeshVizInterface/mapping/nodes/MoMeshSkin.h>
#include <MeshVizInterface/mapping/nodes/MoMeshSlab.h>
#include <MeshVizInterface/mapping/nodes/MoMeshSurface.h>
#include <MeshVizInterface/mapping/nodes/MoScalarSet.h>
#include <MeshVizInterface/mapping/nodes/MoScalarSetIjk.h>
#include <MeshVizInterface/mapping/nodes/MoLegend.h>

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
  , chunksGroup(0)
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
  SnapshotInfo& snapshot = m_snapshots[m_currentSnapshot];

  float sizeX = (float)(m_maxX - m_minX);
  float sizeY = (float)(m_maxY - m_minY);
  float sizeZ = (float)(snapshot.maxZ - snapshot.minZ);

  const float margin = .05f;
  float dx = sizeX * margin;
  float dy = sizeY * margin;

  float verticalScale = m_scale->scaleFactor.getValue()[2];
  SbVec3f start(-dx, -dy, -sizeZ * verticalScale);
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
  SnapshotInfo& snapshot = m_snapshots[m_currentSnapshot];

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
  SoGroup* createArrows()
  {
    return nullptr;
  }

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

  PersistentTrapIdProperty* createPersistentTrapIdProperty(const di::Property* trapIdProperty, const di::Reservoir* reservoir, const di::Snapshot* snapshot)
  {
    di::ProjectHandle* handle = snapshot->getProjectHandle();

    // Create a mapping from id -> persistentId for all traps in this snapshot
    std::vector<std::tuple<unsigned int, unsigned int> > idmap;
    std::unique_ptr<di::TrapList> traps(handle->getTraps(reservoir, snapshot, 0));
    for (auto trap : *traps)
    {
      unsigned int id = trap->getId();
      const di::Trapper* trapper = handle->findTrapper(reservoir, snapshot, id, 0);
      if (trapper)
      {
        unsigned int persistentId = trapper->getPersistentId();
        idmap.push_back(std::make_tuple(id, persistentId));
      }
    }

    if (idmap.empty())
      return nullptr;

    std::sort(idmap.begin(), idmap.end());

    // Create translation table
    unsigned int minId = std::get<0>(idmap[0]);
    unsigned int maxId = std::get<0>(idmap[idmap.size() - 1]);
    unsigned int index = 0;
    std::vector<unsigned int> table;
    for (unsigned int i = minId; i <= maxId; ++i)
    {
      if (i == std::get<0>(idmap[index]))
        table.push_back(std::get<1>(idmap[index++]));
      else
        table.push_back(0);
    }

    std::unique_ptr<di::PropertyValueList> trapIdValues(trapIdProperty->getPropertyValues(di::RESERVOIR, snapshot, reservoir, 0, 0));
    if (!trapIdValues || trapIdValues->empty())
      return nullptr;

    const di::GridMap* trapIds = (*trapIdValues)[0]->getGridMap();

    return new PersistentTrapIdProperty(trapIds, table, minId);
  }

  unsigned int getMaxPersistentTrapId(const di::ProjectHandle* handle)
  {
    unsigned int maxPersistentId = 0;

    std::unique_ptr<di::SnapshotList> snapshots(handle->getSnapshots());
    for (auto snapshot : *snapshots)
    {
      // Create a mapping from id -> persistentId for all traps in this snapshot
      std::unique_ptr<di::TrapList> traps(handle->getTraps(nullptr, nullptr, 0));
      for (auto trap : *traps)
      {
        const di::Trapper* trapper = handle->findTrapper(trap->getReservoir(), trap->getSnapshot(), trap->getId(), 0);
        if (!trapper)
          continue;

        unsigned int persistentId = trapper->getPersistentId();
        maxPersistentId = std::max(maxPersistentId, persistentId);
      }
    }

    return maxPersistentId;
  }

  MiDataSetI<double>* createSurfaceProperty(const di::Property* prop, const di::Surface* surface, const di::Snapshot* snapshot)
  {
    if (!prop)
      return nullptr;

    DataModel::PropertyAttribute attr = prop->getPropertyAttribute();
    if (attr != DataModel::CONTINUOUS_3D_PROPERTY)
      return nullptr;

    std::unique_ptr<di::PropertyValueList> values(
      prop->getPropertyValues(di::SURFACE, snapshot, nullptr, nullptr, surface));

    if (!values || values->empty())
      return nullptr;

    return new SurfaceProperty(prop->getName(), (*values)[0]->getGridMap());
  }

  MiDataSetIjk<double>* createFormationProperty(const di::Property* prop, const SnapshotInfo& info)
  {
    return nullptr;
  }

  MiDataSetIjk<double>* createReservoirProperty(const di::Property* prop, const di::Reservoir* reservoir, const di::Snapshot* snapshot)
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

    return new ReservoirProperty(prop->getName(), (*values)[0]->getGridMap());
  }

}

void SceneGraphManager::updateSnapshotSurfaces()
{
  SnapshotInfo& snapshot = m_snapshots[m_currentSnapshot];

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

      surf.meshData = new SurfaceMesh((*values)[0]->getGridMap());
      surf.mesh = new MoMesh;
      surf.mesh->setMesh(surf.meshData);
      surf.scalarSet = new MoScalarSet;
      surf.propertyData= createSurfaceProperty(m_currentProperty, surface, snapshot.snapshot);
      surf.scalarSet->setScalarSet(surf.propertyData);
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
      // The meshData is not reference counted, need to delete this ourselves
      delete surf.meshData;
      delete surf.propertyData;
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
  SnapshotInfo& snapshot = m_snapshots[m_currentSnapshot];

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
        res.meshData = new ReservoirMesh((*topValues)[0]->getGridMap(), (*bottomValues)[0]->getGridMap());
        res.mesh->setMesh(res.meshData);
        res.scalarSet = new MoScalarSet;
        res.propertyData = createReservoirProperty(m_currentProperty, reservoir, snapshot.snapshot);
        res.scalarSet->setScalarSet(res.propertyData);
        res.skin = new MoMeshSkin;

        res.root->addChild(res.mesh);
        res.root->addChild(res.scalarSet);
        res.root->addChild(res.skin);

        snapshot.reservoirsGroup->addChild(res.root);
      }
    }
    else if (!m_reservoirs[id].visible && res.root != 0)
    {
      delete res.meshData;
      delete res.propertyData;
      snapshot.reservoirsGroup->removeChild(res.root);
      res.root = 0;
      res.mesh = 0;
      res.meshData = 0;
      res.scalarSet = 0;
      res.propertyData = 0;
      res.skin = 0;
    }
  }

  snapshot.reservoirsTimeStamp = m_reservoirsTimeStamp;
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
  SnapshotInfo& snapshot = m_snapshots[m_currentSnapshot];

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

MiDataSetIjk<double>* SceneGraphManager::createFormationProperty(const di::Property* prop, const SnapshotInfo& snapshot)
{
  if (!prop)
    return nullptr;

  if (prop->getType() != di::FORMATIONPROPERTY)
    return nullptr;

  DataModel::PropertyAttribute attr = prop->getPropertyAttribute();

  std::vector<const di::GridMap*> gridMaps;
  bool gridMapsOK = true;

  for (size_t i = 0; i < snapshot.formations.size(); ++i)
  {
    int id = snapshot.formations[i].id;
    const di::Formation* formation = m_formations[id].object;

    std::unique_ptr<di::PropertyValueList> values(
      prop->getPropertyValues(di::FORMATION, snapshot.snapshot, nullptr, formation, nullptr));

    di::GridMap* gridMap = nullptr;
    if (values && !values->empty())
      gridMap = (*values)[0]->getGridMap();

    if (attr == DataModel::FORMATION_2D_PROPERTY)
    {
      int k0 = snapshot.formations[i].minK;
      int k1 = snapshot.formations[i].maxK;
      for (int k = k0; k < k1; ++k)
        gridMaps.push_back(gridMap);
    }
    else
    {
      if (gridMap != nullptr)
        gridMaps.push_back(gridMap);
      else
        gridMapsOK = false;
    }
  }

  if (gridMaps.empty() || !gridMapsOK)
    return nullptr;

  const std::string& name = prop->getName();
  if (attr == DataModel::FORMATION_2D_PROPERTY)
    return new Formation2DProperty(name, gridMaps);
  else
    return new FormationProperty(name, gridMaps);
}

void SceneGraphManager::updateSnapshotProperties()
{
  SnapshotInfo& snapshot = m_snapshots[m_currentSnapshot];

  // Update properties
  if (snapshot.currentProperty == m_currentProperty)
    return;

  snapshot.scalarDataSet.reset(createFormationProperty(m_currentProperty, snapshot));
  snapshot.scalarSet->setScalarSet(snapshot.scalarDataSet.get());

  for (auto &surf : snapshot.surfaces)
  {
    if (surf.root)
    {
      delete surf.propertyData;
      surf.propertyData = createSurfaceProperty(m_currentProperty, m_surfaces[surf.id].object, snapshot.snapshot);
      surf.scalarSet->setScalarSet(surf.propertyData);
    }
  }

  for (auto &res : snapshot.reservoirs)
  {
    if (res.root)
    {
      delete res.propertyData;
      res.propertyData = createReservoirProperty(m_currentProperty, m_reservoirs[res.id].object, snapshot.snapshot);
      res.scalarSet->setScalarSet(res.propertyData);
    }
  }

  snapshot.currentProperty = m_currentProperty;
}

void SceneGraphManager::updateSnapshotSlices()
{
  SnapshotInfo& snapshot = m_snapshots[m_currentSnapshot];

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

void SceneGraphManager::updateColorMap()
{
  if (!m_currentProperty)
    return;

  m_colorMapSwitch->whichChild = (m_currentProperty == m_resRockTrapIdProperty) ? 1 : 0;

  SnapshotInfo& snapshot = m_snapshots[m_currentSnapshot];

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

void SceneGraphManager::updateSnapshot()
{
  updateSnapshotFormations();
  updateSnapshotSurfaces();
  updateSnapshotReservoirs();
  updateSnapshotFaults();
  updateSnapshotProperties();
  updateSnapshotSlices();
  updateColorMap();

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

    // Add faults, if any
    std::unique_ptr<di::FaultCollectionList> faultCollections(formation->getFaultCollections());
    if (faultCollections)
    {
      for (auto collection : *faultCollections)
      {
        std::unique_ptr<di::FaultList> faults(collection->getFaults());
        if (faults)
        {
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
    }

    // Add reservoirs
    std::unique_ptr<di::ReservoirList> reservoirList(formation->getReservoirs());
    if (reservoirList)
    {
      for (auto reservoir : *reservoirList)
      {
        SnapshotInfo::Reservoir res;
        res.id = m_reservoirIdMap[reservoir->getName()];

        info.reservoirs.push_back(res);
      }
    }
  }

  std::unique_ptr<di::SurfaceList> surfaces(m_projectHandle->getSurfaces(snapshot, false));
  for (size_t i = 0; i < surfaces->size(); ++i)
  {
    SnapshotInfo::Surface srfc;
    srfc.id = m_surfaceIdMap[(*surfaces)[i]->getName()];

    info.surfaces.push_back(srfc);
  }

  info.root = new SoSeparator;
  info.formationsRoot = new SoSeparator;
  info.formationsRoot->setName("formations");

  if (!depthMaps.empty())
  {
    info.geometry.reset(new SnapshotGeometry(depthMaps));
    info.topology.reset(new SnapshotTopology(info.geometry));
    info.meshData = new HexahedronMesh(info.geometry, info.topology);

    info.minZ = info.geometry->getMin()[2];
    info.maxZ = info.geometry->getMax()[2];
  }

  info.mesh = new MoMesh;
  info.mesh->setName("snapshotMesh");
  info.mesh->setMesh(info.meshData);

  info.scalarDataSet.reset(new FormationIdProperty(formationIds));

  info.scalarSet = new MoScalarSetIjk;
  info.scalarSet->setName("formationID");
  info.scalarSet->setScalarSet(info.scalarDataSet.get());

  info.chunksGroup = new SoGroup;
  info.chunksGroup->setName("chunks");
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
  std::vector<const di::Snapshot*> tmpSnapshotList(*snapshots);
  std::sort(tmpSnapshotList.begin(), tmpSnapshotList.end(), 
    [](const di::Snapshot* s1, const di::Snapshot* s2) { return s1->getTime() > s2->getTime(); });

  for (size_t i = 0; i < tmpSnapshotList.size(); ++i)
  {
    std::cout << "Creating node " << i << std::endl;
    const di::Snapshot* snapshot = tmpSnapshotList[i];
    SnapshotInfo info = createSnapshotNode(snapshot);
    m_snapshots.push_back(info);
    m_snapshotsSwitch->addChild(info.root);
  }

  m_snapshotsSwitch->whichChild = 0;
}

void SceneGraphManager::setupCoordinateGrid()
{
  m_coordinateGrid = initCoordinateGrid(m_minX, m_minY, m_maxX, m_maxY);
  m_coordinateGridSwitch = new SoSwitch;
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
  m_formationShapeHints->vertexOrdering = SoShapeHints::COUNTERCLOCKWISE;

  // Double sided lighting is enabled for surfaces with ordered vertices
  m_surfaceShapeHints = new SoShapeHints;
  m_surfaceShapeHints->setName("surfaceShapeHints");
  m_surfaceShapeHints->shapeType = SoShapeHints::UNKNOWN_SHAPE_TYPE;
  m_surfaceShapeHints->vertexOrdering = SoShapeHints::COUNTERCLOCKWISE;

  setupCoordinateGrid();

  m_scale = new SoScale;
  m_scale->scaleFactor = SbVec3f(1.f, 1.f, 1.f);

  m_drawStyle = new MoDrawStyle;
  m_drawStyle->displayFaces = true;
  m_drawStyle->displayEdges = true;
  m_drawStyle->displayPoints = false;

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
  m_root->addChild(m_cameraSwitch);
  m_root->addChild(m_formationShapeHints);
  m_root->addChild(m_coordinateGridSwitch);
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
  , m_resRockTopProperty(0)
  , m_resRockBottomProperty(0)
  , m_resRockTrapIdProperty(0)
  , m_currentProperty(0)
  , m_numI(0)
  , m_numJ(0)
  , m_numIHiRes(0)
  , m_numJHiRes(0)
  , m_minX(0.0)
  , m_minY(0.0)
  , m_maxX(0.0)
  , m_maxY(0.0)
  , m_maxPersistentTrapId(0)
  , m_showGrid(false)
  , m_projectionType(PerspectiveProjection)
  , m_formationsTimeStamp(MxTimeStamp::getTimeStamp())
  , m_surfacesTimeStamp(MxTimeStamp::getTimeStamp())
  , m_reservoirsTimeStamp(MxTimeStamp::getTimeStamp())
  , m_faultsTimeStamp(MxTimeStamp::getTimeStamp())
  , m_currentSnapshot(0)
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
  m_currentSnapshot = index;
  m_snapshotsSwitch->whichChild = (int)index;

  updateSnapshot();
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
  m_scale->scaleFactor = SbVec3f(1.f, 1.f, scale);
  updateCoordinateGrid();
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

void SceneGraphManager::setup(const di::ProjectHandle* handle)
{
  m_projectHandle = handle;

  const std::string depthKey = "Depth";
  const std::string resRockTopKey = "ResRockTop";
  const std::string resRockBottomKey = "ResRockBottom";
  const std::string resRockTrapIdKey = "ResRockTrapId";

  m_depthProperty = handle->findProperty(depthKey);
  m_resRockTopProperty = handle->findProperty(resRockTopKey);
  m_resRockBottomProperty = handle->findProperty(resRockBottomKey);
  m_resRockTrapIdProperty = handle->findProperty(resRockTrapIdKey);

  const di::Grid* loresGrid = handle->getLowResolutionOutputGrid();
  const di::Grid* hiresGrid = handle->getHighResolutionOutputGrid();

  m_numI = loresGrid->numI();
  m_numJ = loresGrid->numJ();
  m_numIHiRes = hiresGrid->numI();
  m_numJHiRes = hiresGrid->numJ();
  m_minX = loresGrid->minI();
  m_minY = loresGrid->minJ();
  m_maxX = loresGrid->maxI();
  m_maxY = loresGrid->maxJ();

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
}

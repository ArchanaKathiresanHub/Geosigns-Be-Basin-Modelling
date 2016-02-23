//
// Copyright (C) 2012-2015 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "DataAccessProject.h"

#include <Interface/ProjectHandle.h>
#include <Interface/ObjectFactory.h>
#include <Interface/Snapshot.h>
#include <Interface/Grid.h>
#include <Interface/Formation.h>
#include <Interface/Surface.h>
#include <Interface/Reservoir.h>
#include <Interface/FaultCollection.h>
#include <Interface/Faulting.h>
#include <Interface/Trapper.h>
#include <Interface/Property.h>
#include <Interface/PropertyValue.h>

#include <MeshVizXLM/mesh/data/MiDataSetIj.h>
#include <MeshVizXLM/mesh/data/MiDataSetIjk.h>

#include "Mesh.h"
#include "Property.h"

namespace di = DataAccess::Interface;

int DataAccessProject::getPropertyId(const std::string& name) const
{
  auto iter = m_propertyIdMap.find(name);
  if (iter != m_propertyIdMap.end())
    return iter->second;
  else
    return -1;
}

size_t DataAccessProject::getSnapshotCount() const
{
  return m_snapshotList.size();
}

Project::SnapshotContents DataAccessProject::getSnapshotContents(size_t snapshotIndex) const
{
  const di::Snapshot* snapshot = m_snapshotList[snapshotIndex];

  SnapshotContents contents;
  contents.minDepth = 0.0;
  contents.maxDepth = 0.0;

  // Instead of looping over the formation list, we get all the formation depth values. This 
  // allows us to also compute the minK and maxK values for each formation.
  std::unique_ptr<di::PropertyValueList> depthValues(
    m_depthProperty->getPropertyValues(
      di::FORMATION, 
      snapshot, 
      nullptr, 
      nullptr, 
      nullptr));

  // Remove basement formations
  if (!m_includeBasement)
  {
    auto iter = std::remove_if(
      depthValues->begin(),
      depthValues->end(),
      [](const di::PropertyValue* val)
    {
      return val->getFormation()->kind() == di::BASEMENT_FORMATION;
    });
    depthValues->erase(iter, depthValues->end());
  }

  int k = 0;
  for (auto item : *depthValues)
  {
    const di::Formation* fmt = item->getFormation();
    const di::GridMap* gridMap = item->getGridMap();

    DataAccessProject::SnapshotFormation ssf;
    auto iter = m_formationIdMap.find(fmt->getName());
    if (iter == m_formationIdMap.end())
      continue; // e.g. for basement formation

    ssf.id = iter->second;
    ssf.minK = k;
    ssf.maxK = k + gridMap->getDepth() - 1;
    contents.formations.push_back(ssf);

    k = ssf.maxK;

    // See if this formation has any reservoirs
    std::unique_ptr<di::ReservoirList> reservoirList(m_projectHandle->getReservoirs(fmt));
    for (auto res : *reservoirList)
    {
      int reservoirId = m_reservoirIdMap.at(res->getName());
      contents.reservoirs.push_back(reservoirId);

      // Push a flowline entry for each reservoir
      for (int id = 0; id < (int)m_projectInfo.flowLines.size(); ++id)
      {
        if (m_projectInfo.flowLines[id].reservoirId == reservoirId)
        {
          contents.flowlines.push_back(id);
          break;
        }
      }
    }

    // Push a flowline entry for each formation that is a source rock
    if (fmt->isSourceRock())
    {
      for (int id = 0; id < (int)m_projectInfo.flowLines.size(); ++id)
      {
        if (m_projectInfo.flowLines[id].formationId == ssf.id)
        {
          contents.flowlines.push_back(id);
          break;
        }
      }
    }
  }

  if (!depthValues->empty())
  {
    double minDepth0, maxDepth0, minDepth1, maxDepth1;
    (*depthValues)[0]->getGridMap()->getMinMaxValue(minDepth0, maxDepth0);
    (*depthValues)[depthValues->size() - 1]->getGridMap()->getMinMaxValue(minDepth1, maxDepth1);

    contents.minDepth = std::min(minDepth0, minDepth1);
    contents.maxDepth = std::max(maxDepth0, maxDepth1);
  }

  std::unique_ptr<di::SurfaceList> surfaceList(m_projectHandle->getSurfaces(snapshot));
  for (auto item : *surfaceList)
    contents.surfaces.push_back(m_surfaceIdMap.at(item->getName()));

  contents.age = snapshot->getTime();

  return contents;
}

std::vector<const di::GridMap*> DataAccessProject::getFormationPropertyGridMaps(
  size_t snapshotIndex,
  const di::Property* prop,
  bool formation3D) const
{
  const di::Snapshot* snapshot = m_snapshotList[snapshotIndex];

  std::unique_ptr<di::PropertyValueList> values(prop->getPropertyValues(di::FORMATION, snapshot, 0, 0, 0));

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

  std::vector<const di::GridMap*> gridMaps;
  std::unique_ptr<di::FormationList> formationList(m_projectHandle->getFormations(snapshot, m_includeBasement));
  size_t i = 0;
  for (auto formation : *formationList)
  {
    const di::GridMap* gridMap = nullptr;
    if (i < n && formation == (*values)[i]->getFormation())
      gridMap = (*values)[i++]->getGridMap();

    if (formation3D && gridMap)
    {
      gridMaps.push_back(gridMap);
    }
    else
    {
      int id = m_formationIdMap.at(formation->getName());
      int n = m_projectInfo.formations[id].numCellsK;
      for (int k = 0; k < n; ++k)
        gridMaps.push_back(gridMap);
    }
  }

  return gridMaps;
}

void DataAccessProject::init()
{
  const std::string depthKey = "Depth";
  const std::string resRockTopKey = "ResRockTop";
  const std::string resRockBottomKey = "ResRockBottom";
  const std::string resRockTrapIdKey = "ResRockTrapId";
  const std::string resRockDrainageIdGasPhasePropertyKey = "ResRockDrainageIdGasPhase";
  const std::string resRockDrainageIdFluidPhasePropertyKey = "ResRockDrainageIdFluidPhase";
  const std::string resRockLeakagePropertyKey = "ResRockLeakage";
  const std::string flowDirectionKey = "FlowDirectionIJK";

  m_depthProperty = m_projectHandle->findProperty(depthKey);
  m_resRockTopProperty = m_projectHandle->findProperty(resRockTopKey);
  m_resRockBottomProperty = m_projectHandle->findProperty(resRockBottomKey);
  m_resRockTrapIdProperty = m_projectHandle->findProperty(resRockTrapIdKey);
  m_resRockDrainageIdGasPhaseProperty = m_projectHandle->findProperty(resRockDrainageIdGasPhasePropertyKey);
  m_resRockDrainageIdFluidPhaseProperty = m_projectHandle->findProperty(resRockDrainageIdFluidPhasePropertyKey);
  m_resRockLeakageProperty = m_projectHandle->findProperty(resRockLeakagePropertyKey);
  m_flowDirectionProperty = m_projectHandle->findProperty(flowDirectionKey);

  const di::Grid* loresGrid = m_projectHandle->getLowResolutionOutputGrid();
  const di::Grid* hiresGrid = m_projectHandle->getHighResolutionOutputGrid();

  m_projectInfo.dimensions.numCellsI = loresGrid->numI() - 1;
  m_projectInfo.dimensions.numCellsJ = loresGrid->numJ() - 1;
  m_projectInfo.dimensions.numCellsIHiRes = hiresGrid->numI() - 1;
  m_projectInfo.dimensions.numCellsJHiRes = hiresGrid->numJ() - 1;
  m_projectInfo.dimensions.deltaX = loresGrid->deltaI();
  m_projectInfo.dimensions.deltaY = loresGrid->deltaJ();
  m_projectInfo.dimensions.deltaXHiRes = hiresGrid->deltaI();
  m_projectInfo.dimensions.deltaYHiRes = hiresGrid->deltaJ();
  m_projectInfo.dimensions.minX = loresGrid->minI();
  m_projectInfo.dimensions.minY = loresGrid->minJ();

  // Get snapshots
  std::unique_ptr<di::SnapshotList> snapshots(m_projectHandle->getSnapshots());
  m_snapshotList = *snapshots;
  m_projectInfo.snapshotCount = m_snapshotList.size();

  // Get all available formations
  std::unique_ptr<di::PropertyValueList> formationDepthValues(
    m_depthProperty->getPropertyValues(di::FORMATION, m_snapshotList[0], 0, 0, 0));
  int id = 0;
  for (auto depthValue : *formationDepthValues)
  {
    auto formation = depthValue->getFormation();
    if (formation->kind() == di::BASEMENT_FORMATION && !m_includeBasement)
      continue;

    m_formations.push_back(formation);
    m_formationIdMap[formation->getName()] = id;

    Formation fmt;
    fmt.name = formation->getName();
    fmt.numCellsK = depthValue->getGridMap()->getDepth() - 1;
    fmt.isSourceRock = formation->isSourceRock();
    m_projectInfo.formations.push_back(fmt);

    if (formation->isSourceRock())
    {
      FlowLines flowLines;
      flowLines.type = FlowLines::Expulsion;
      flowLines.formationId = id;
      flowLines.reservoirId = -1;
      flowLines.name = formation->getName();

      m_projectInfo.flowLines.push_back(flowLines);
    }

    id++;
  }

  // Get all available surfaces
  std::unique_ptr<di::SurfaceList> surfaces(m_projectHandle->getSurfaces());
  m_surfaces = *surfaces;
  id = 0;
  for (auto item : m_surfaces)
  {
    m_surfaceIdMap[item->getName()] = id++;

    Surface surface;
    surface.name = item->getName();
    m_projectInfo.surfaces.push_back(surface);
  }

  // Get reservoirs
  std::unique_ptr<di::ReservoirList> reservoirs(m_projectHandle->getReservoirs());
  m_reservoirs = *reservoirs;
  id = 0;
  for (auto item : *reservoirs)
  {
    m_reservoirIdMap[item->getName()] = id;

    Reservoir reservoir;
    reservoir.name = item->getName();
    m_projectInfo.reservoirs.push_back(reservoir);

    // Get flowlines from reservoir leakage
    auto formation = item->getFormation();
    FlowLines flowLines;
    flowLines.type = FlowLines::Leakage;
    flowLines.formationId = m_formationIdMap[formation->getName()];
    flowLines.reservoirId = id;
    flowLines.name = item->getName();

    m_projectInfo.flowLines.push_back(flowLines);

    id++;
  }

  // Get faults
  id = 0;
  std::unique_ptr<di::FaultCollectionList> faultCollections(m_projectHandle->getFaultCollections(0));
  for (int i = 0; i < (int)faultCollections->size(); ++i)
  {
    auto coll = (*faultCollections)[i];

    FaultCollection collection;
    collection.name = coll->getName();
    for (int j = 0; j < (int)m_formations.size(); ++j)
    {
      if (coll->appliesToFormation(m_formations[j]))
        collection.formations.push_back(j);
    }
    m_projectInfo.faultCollections.push_back(collection);

    std::unique_ptr<di::FaultList> faults(coll->getFaults());
    for (auto item : *faults)
    {
      m_faults.push_back(item);

      m_faultMap[
        std::make_tuple(
          coll->getName(),
          item->getName())] = id++;

      Fault fault;
      fault.collectionId = i;
      fault.name = item->getName();

      m_projectInfo.faults.push_back(fault);
    }
  }

  // Get properties
  std::unique_ptr<di::PropertyList> properties(m_projectHandle->getProperties(true));
  id = 0;
  for (auto item : *properties)
  {
    const int allFlags = di::FORMATION | di::SURFACE | di::RESERVOIR | di::FORMATIONSURFACE;
    const int allTypes = di::MAP | di::VOLUME;
    if (item->hasPropertyValues(allFlags, 0, 0, 0, 0, allTypes))
    {
      Property prop = { item->getName(), item->getUnit() };
      m_projectInfo.properties.push_back(prop);

      m_properties.push_back(item);
      m_propertyIdMap[item->getName()] = id++;
    }
  }
}

DataAccessProject::DataAccessProject(const std::string& path)
  : m_includeBasement(true)
  , m_loresDeadMap(nullptr)
  , m_hiresDeadMap(nullptr)
{
  m_objectFactory = std::make_shared<di::ObjectFactory>();
  m_projectHandle.reset(di::OpenCauldronProject(path, "r", m_objectFactory.get()));

  if (!m_projectHandle)
    throw std::runtime_error("Could not open project");

  init();
}

DataAccessProject::~DataAccessProject()
{
  delete[] m_loresDeadMap;
  delete[] m_hiresDeadMap;
}

Project::ProjectInfo DataAccessProject::getProjectInfo() const
{
  return m_projectInfo;
}

// Utility function to get the maximum persistent trap id. This is useful for constructing
// a colormap for the trap id property.
unsigned int DataAccessProject::getMaxPersistentTrapId() const
{
  unsigned int maxPersistentId = 0;

  std::unique_ptr<di::TrapperList> trappers(m_projectHandle->getTrappers(nullptr, nullptr, 0, 0));
  for (auto trapper : *trappers)
    maxPersistentId = std::max(maxPersistentId, trapper->getPersistentId());

  return maxPersistentId;
}

namespace
{

  bool* createDeadMap(const di::GridMap* gridMap)
  {
    unsigned int ni = gridMap->numI() - 1;
    unsigned int nj = gridMap->numJ() - 1;

    bool* deadMap = new bool[ni * nj];
    for (unsigned int i = 0; i < ni; ++i)
    {
      for (unsigned int j = 0; j < nj; ++j)
      {
        deadMap[i * nj + j] =
          gridMap->getValue(i, j) == di::DefaultUndefinedMapValue ||
          gridMap->getValue(i, j + 1) == di::DefaultUndefinedMapValue ||
          gridMap->getValue(i + 1, j) == di::DefaultUndefinedMapValue ||
          gridMap->getValue(i + 1, j + 1) == di::DefaultUndefinedMapValue;
      }
    }

    return deadMap;
  }

}

std::shared_ptr<MiVolumeMeshCurvilinear> DataAccessProject::createSnapshotMesh(size_t snapshotIndex) const
{
  const di::Snapshot* snapshot = m_snapshotList[snapshotIndex];

  std::vector<const di::GridMap*> depthMaps;

  // Get a list of all the formation depth values in this snapshot. This list is already 
  // sorted from top to bottom when we get it from DataAccess lib
  std::unique_ptr<di::PropertyValueList> depthValues(
    m_depthProperty->getPropertyValues(
      di::FORMATION,
      snapshot,
      nullptr,
      nullptr,
      nullptr));

  for (auto depthValue : *depthValues)
  {
    auto formation = depthValue->getFormation();
    if (formation->kind() == di::BASEMENT_FORMATION && !m_includeBasement)
      continue;

    depthMaps.push_back(depthValue->getGridMap());
  }

  if (depthMaps.empty())
    return nullptr;

  if (!m_loresDeadMap)
    m_loresDeadMap = createDeadMap(depthMaps[0]);

  auto geometry = std::make_shared<SnapshotGeometry>(depthMaps);
  auto topology = std::make_shared<SnapshotTopology>(geometry, m_loresDeadMap);
  return std::make_shared<SnapshotMesh>(geometry, topology);
}

std::shared_ptr<MiVolumeMeshCurvilinear> DataAccessProject::createReservoirMesh(
  size_t snapshotIndex, 
  int reservoirId) const
{
  const di::Snapshot* snapshot = m_snapshotList[snapshotIndex];
  const di::Reservoir* reservoir = m_reservoirs[reservoirId];

  std::unique_ptr<di::PropertyValueList> topValues(m_projectHandle->getPropertyValues(
    di::RESERVOIR, m_resRockTopProperty, snapshot, reservoir, 0, 0, di::MAP));
  std::unique_ptr<di::PropertyValueList> bottomValues(m_projectHandle->getPropertyValues(
    di::RESERVOIR, m_resRockBottomProperty, snapshot, reservoir, 0, 0, di::MAP));

  if (!topValues || topValues->empty() || !bottomValues || bottomValues->empty())
    return nullptr;

  if (!m_hiresDeadMap)
    m_hiresDeadMap = createDeadMap((*topValues)[0]->getGridMap());

  auto geometry = std::make_shared<ReservoirGeometry>(
    (*topValues)[0]->getGridMap(), 
    (*bottomValues)[0]->getGridMap());
  auto topology = std::make_shared<ReservoirTopology>(geometry, m_hiresDeadMap);

  return std::make_shared<ReservoirMesh>(geometry, topology);
}

std::shared_ptr<MiSurfaceMeshCurvilinear> DataAccessProject::createSurfaceMesh(
  size_t snapshotIndex, 
  int surfaceId) const
{
  const di::Snapshot* snapshot = m_snapshotList[snapshotIndex];
  const di::Surface* surface = m_surfaces[surfaceId];

  std::unique_ptr<di::PropertyValueList> values(m_projectHandle->getPropertyValues(
    di::SURFACE, m_depthProperty, snapshot, 0, 0, surface, di::MAP));

  assert(values->size() == 1); //TODO: should not be an assert

  auto depthMap = (*values)[0]->getGridMap();
  auto geometry = std::make_shared<SurfaceGeometry>(depthMap);
  auto topology = std::make_shared<SurfaceTopology>(depthMap->numI() - 1, depthMap->numJ() - 1, *geometry);

  return std::make_shared<SurfaceMesh>(geometry, topology);
}

std::shared_ptr<MiDataSetIjk<double> > DataAccessProject::createFormationProperty(
  size_t snapshotIndex, 
  int propertyId) const
{
  if (propertyId < 0 || propertyId >= (int)m_projectInfo.properties.size())
    return nullptr;

  const di::Property* prop = m_properties[propertyId];
  if (prop->getType() != di::FORMATIONPROPERTY)
    return nullptr;

  bool formation2D = prop->getPropertyAttribute() == DataModel::FORMATION_2D_PROPERTY;
  std::vector<const di::GridMap*> gridMaps = getFormationPropertyGridMaps(
    snapshotIndex, prop, !formation2D);

  if (gridMaps.empty())
    return nullptr;

  std::string name = prop->getName();
  if (formation2D)
    return std::make_shared<Formation2DProperty>(name, gridMaps);
  else
    return std::make_shared<FormationProperty>(name, gridMaps);
}

std::shared_ptr<MiDataSetIj<double> > DataAccessProject::createFormation2DProperty(
  size_t snapshotIndex,
  int formationId,
  int propertyId) const
{
  if (propertyId < 0 || propertyId >= (int)m_properties.size())
    return nullptr;

  const di::Property* prop = m_properties[propertyId];

  if (!prop || prop->getPropertyAttribute() != DataModel::FORMATION_2D_PROPERTY)
    return nullptr;

  const di::Snapshot* snapshot = m_snapshotList[snapshotIndex];
  const di::Formation* formation = m_formations[formationId];

  std::unique_ptr<di::PropertyValueList> values(
    prop->getPropertyValues(di::FORMATION, snapshot, nullptr, formation, nullptr));

  if (!values || values->empty())
    return nullptr;

  auto gridMap = (*values)[0]->getGridMap();
  if (!gridMap)
    return nullptr;

  return std::make_shared<SurfaceProperty>(prop->getName(), gridMap);
}

std::shared_ptr<MiDataSetIj<double> > DataAccessProject::createSurfaceProperty(
  size_t snapshotIndex, 
  int surfaceId,
  int propertyId) const
{
  if (propertyId < 0 || propertyId >= (int)m_properties.size())
    return nullptr;

  const di::Property* prop = m_properties[propertyId];

  if (!prop || prop->getPropertyAttribute() != DataModel::CONTINUOUS_3D_PROPERTY)
    return nullptr;

  const di::Snapshot* snapshot = m_snapshotList[snapshotIndex];
  const di::Surface* surface = m_surfaces[surfaceId];

  std::unique_ptr<di::PropertyValueList> values(
    prop->getPropertyValues(di::SURFACE, snapshot, nullptr, nullptr, surface));

  if (!values || values->empty())
    return nullptr;

  return std::make_shared<SurfaceProperty>(prop->getName(), (*values)[0]->getGridMap());
}

std::shared_ptr<MiDataSetIjk<double> > DataAccessProject::createReservoirProperty(
  size_t snapshotIndex, 
  int reservoirId,
  int propertyId) const
{
  if (propertyId < 0 || propertyId >= (int)m_properties.size())
    return nullptr;

  const di::Property* prop = m_properties[propertyId];
  //if (prop == m_resRockTrapIdProperty)
  //  return createPersistentTrapIdProperty(snapshotIndex, reservoirId);

  DataModel::PropertyAttribute attr = prop->getPropertyAttribute();
  DataAccess::Interface::PropertyType type = prop->getType();

  if (attr != DataModel::FORMATION_2D_PROPERTY || type != di::RESERVOIRPROPERTY)
    return nullptr;

  const di::Snapshot* snapshot = m_snapshotList[snapshotIndex];
  const di::Reservoir* reservoir = m_reservoirs[reservoirId];

  std::unique_ptr<di::PropertyValueList> values(
    prop->getPropertyValues(di::RESERVOIR, snapshot, reservoir, nullptr, nullptr));

  if (!values || values->empty())
    return nullptr;

  auto gridMap = (*values)[0]->getGridMap();
  if (!gridMap)
    return nullptr;

  auto result = std::make_shared<ReservoirProperty>(prop->getName(), gridMap);
  //if (prop == m_resRockLeakageProperty)
  //  result->setLogarithmic(true);

  return result;
}

// A PersistentTrapIdProperty consists of the values of the ResRockTrapId property, combined with
// a translation table to convert the regular trap ids to persistent trap ids. This function builds
// the translation table, gets the ResRockTrapId values, and creates the PersistentTrapIdProperty.
std::shared_ptr<MiDataSetIjk<double> > DataAccessProject::createPersistentTrapIdProperty(
  size_t snapshotIndex,
  int reservoirId) const
{
  const di::Snapshot* snapshot = m_snapshotList[snapshotIndex];
  const di::Reservoir* reservoir = m_reservoirs[reservoirId];

  // Create a mapping from id -> persistentId for all traps in this snapshot
  std::unique_ptr<di::TrapperList> trappers(m_projectHandle->getTrappers(reservoir, snapshot, 0, 0));
  if (trappers->empty())
    return nullptr;

  // Sort on id
  std::sort(
    trappers->begin(), 
    trappers->end(),
    [](const di::Trapper* t1, const di::Trapper* t2) 
      { 
        return t1->getId() < t2->getId(); 
      });

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
    m_resRockTrapIdProperty->getPropertyValues(di::RESERVOIR, snapshot, reservoir, 0, 0));
  if (trapIdValues->empty())
    return nullptr;

  const di::GridMap* trapIds = (*trapIdValues)[0]->getGridMap();
  return std::make_shared<PersistentTrapIdProperty>(trapIds, table, minId);
}

std::shared_ptr<MiDataSetIjk<double> > DataAccessProject::createFlowDirectionProperty(size_t snapshotIndex) const
{
  if (!m_flowDirectionProperty)
    return nullptr;

  std::vector<const di::GridMap*> gridMaps = getFormationPropertyGridMaps(snapshotIndex, m_flowDirectionProperty, true);
  if (gridMaps.empty())
    return nullptr;

  return std::make_shared<FormationProperty>("FlowDirectionIJK", gridMaps, GridMapCollection::SkipFirstK);
}

std::vector<Project::Trap> DataAccessProject::getTraps(size_t snapshotIndex, int reservoirId) const
{
  std::vector<Trap> traps;

  const Project::Dimensions& dim = m_projectInfo.dimensions;

  auto snapshot = m_snapshotList[snapshotIndex];
  auto reservoir = m_reservoirs[reservoirId];
  std::unique_ptr<di::TrapperList> trapperList(m_projectHandle->getTrappers(reservoir, snapshot, 0, 0));
  for (auto trapper : *trapperList)
  {
    Trap trap;

    double x, y, z;
    trapper->getSpillPointPosition(x, y);
    z = -trapper->getSpillDepth();
    trap.spillPoint = SbVec3f((float)(x - dim.minX), (float)(y - dim.minY), (float)z);

    trapper->getPosition(x, y);
    z = -trapper->getDepth();
    trap.leakagePoint = SbVec3f((float)(x - dim.minX), (float)(y - dim.minY), (float)z);

    trap.id = (int)trapper->getId();
    trap.persistentId = (int)trapper->getPersistentId();
    trap.gasOilContactDepth = trapper->getGOC();
    trap.oilWaterContactDepth = trapper->getOWC();

    const di::Trapper* dsTrapper = trapper->getDownstreamTrapper();
    trap.downStreamId = (dsTrapper != 0) ? (int)dsTrapper->getId() : -1;

    traps.push_back(trap);
  }

  std::sort(traps.begin(), traps.end(), [](const Trap& t0, const Trap& t1) { return t0.id < t1.id; });

  return traps;
}

std::vector<SbVec2d> DataAccessProject::getFaultLine(int faultId) const
{
  const di::PointSequence& points = m_faults[faultId]->getFaultLine();

  std::vector<SbVec2d> oivpoints;
  oivpoints.reserve(points.size());
  for (auto p : points)
    oivpoints.emplace_back(
    p(di::X_COORD) - m_projectInfo.dimensions.minX,
    p(di::Y_COORD) - m_projectInfo.dimensions.minY);

  return oivpoints;
}
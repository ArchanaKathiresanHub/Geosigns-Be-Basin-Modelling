//
// Copyright (C) 2012-2015 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "VisualizationIOProject.h"

#include <ImportExport.h>

#include <MeshVizXLM/mesh/geometry/MiGeometryIj.h>
#include <MeshVizXLM/mesh/topology/MiTopologyIj.h>
#include <MeshVizXLM/mesh/geometry/MiGeometryIjk.h>
#include <MeshVizXLM/mesh/topology/MiTopologyIjk.h>
#include <MeshVizXLM/mesh/MiSurfaceMeshCurvilinear.h>
#include <MeshVizXLM/mesh/MiVolumeMeshCurvilinear.h>
#include <MeshVizXLM/mesh/data/MiDataSetIj.h>
#include <MeshVizXLM/mesh/data/MiDataSetIjk.h>
#include <MeshVizXLM/MxTimeStamp.h>

#include <set>
#include <map>

namespace VizIO
{

  class VolumeGeometry : public MiGeometryIjk
  {
    boost::shared_ptr<CauldronIO::Volume> m_volume;

    double m_minX;
    double m_minY;
    double m_deltaX;
    double m_deltaY;

    size_t m_numI;
    size_t m_numJ;
    size_t m_numK;

    float  m_undefined;

    size_t m_timestamp;

  public:

    VolumeGeometry(boost::shared_ptr<CauldronIO::Volume> volume)
      : m_volume(volume)
      , m_timestamp(MxTimeStamp::getTimeStamp())
    {
      m_minX = volume->getMinI();
      m_minY = volume->getMinJ();
      m_deltaX = volume->getDeltaI();
      m_deltaY = volume->getDeltaJ();
      m_numI = volume->getNumI();
      m_numJ = volume->getNumJ();
      m_numK = volume->getNumK();

      m_undefined = volume->getUndefinedValue();
    }

    virtual MbVec3d getCoord(size_t i, size_t j, size_t k) const
    {
      double x = m_minX + i * m_deltaX;
      double y = m_minY + j * m_deltaY;
      double z = -m_volume->getValue(i, j, k);

      return MbVec3d(x, y, z);
    }

    virtual size_t getTimeStamp() const
    {
      return m_timestamp;
    }

    size_t numI() const
    {
      return m_numI;
    }

    size_t numJ() const
    {
      return m_numJ;
    }

    size_t numK() const
    {
      return m_numK;
    }

    bool isUndefined(size_t i, size_t j, size_t k) const
    {
      return m_volume->getValue(i, j, k) == m_undefined;
    }
  };

  class VolumeTopology : public MiTopologyIjk
  {
    const VolumeGeometry& m_geometry;

    size_t m_timestamp;

  public:

    VolumeTopology(const VolumeGeometry& geometry)
      : m_geometry(geometry)
      , m_timestamp(MxTimeStamp::getTimeStamp())
    {

    }

    virtual size_t getNumCellsI() const
    {
      return m_geometry.numI() - 1;
    }

    virtual size_t getNumCellsJ() const
    {
      return m_geometry.numJ() - 1;
    }

    virtual size_t getNumCellsK() const
    {
      return m_geometry.numK() - 1;
    }

    virtual bool isDead(size_t i, size_t j, size_t /*k*/) const
    {
      return
        m_geometry.isUndefined(i, j, 0) ||
        m_geometry.isUndefined(i + 1, j, 0) ||
        m_geometry.isUndefined(i, j + 1, 0) ||
        m_geometry.isUndefined(i + 1, j, 0);
    }

    virtual bool hasDeadCells() const
    {
      return true;
    }

    virtual size_t getTimeStamp() const
    {
      return m_timestamp;
    }
  };

  class VolumeMesh : public MiVolumeMeshCurvilinear
  {
    std::shared_ptr<VolumeGeometry> m_geometry;
    std::shared_ptr<VolumeTopology> m_topology;

  public:

    VolumeMesh(
      std::shared_ptr<VolumeGeometry> geometry,
      std::shared_ptr<VolumeTopology> topology)
      : m_geometry(geometry)
      , m_topology(topology)
    {

    }

    const MiGeometryIjk& getGeometry() const
    {
      return *m_geometry;
    }

    const MiTopologyIjk& getTopology() const
    {
      return *m_topology;
    }
  };

  class SurfaceGeometry : public MiGeometryIj
  {
    boost::shared_ptr<CauldronIO::Map> m_map;

    double m_minX;
    double m_minY;
    double m_deltaX;
    double m_deltaY;

    float  m_undefined;

    size_t m_timestamp;

  public:

    SurfaceGeometry(boost::shared_ptr<CauldronIO::Map> valueMap)
      : m_map(valueMap)
      , m_timestamp(MxTimeStamp::getTimeStamp())
    {
      m_minX = valueMap->getMinI();
      m_minY = valueMap->getMinJ();
      m_deltaX = valueMap->getDeltaI();
      m_deltaY = valueMap->getDeltaJ();

      m_undefined = valueMap->getUndefinedValue();
    }

    virtual MbVec3d getCoord(size_t i, size_t j) const
    {
      double x = m_minX + i * m_deltaX;
      double y = m_minY + j * m_deltaY;
      double z = -m_map->getValue(i, j);

      return MbVec3d(x, y, z);
    }

    virtual size_t getTimeStamp() const
    {
      return m_timestamp;
    }

    bool isUndefined(size_t i, size_t j) const
    {
      return m_map->getValue(i, j) == m_undefined;
    }

    size_t numI() const
    {
      return m_map->getNumI();
    }

    size_t numJ() const
    {
      return m_map->getNumJ();
    }
  };

  class SurfaceTopology : public MiTopologyIj
  {
    const SurfaceGeometry& m_geometry;

    size_t m_timestamp;

  public:

    SurfaceTopology(const SurfaceGeometry& geometry)
      : m_geometry(geometry)
      , m_timestamp(MxTimeStamp::getTimeStamp())
    {

    }

    virtual size_t getNumCellsI() const
    {
      return m_geometry.numI() - 1;
    }

    virtual size_t getNumCellsJ() const
    {
      return m_geometry.numJ() - 1;
    }

    virtual bool isDead(size_t i, size_t j) const
    {
      return
        m_geometry.isUndefined(i, j) ||
        m_geometry.isUndefined(i, j + 1) ||
        m_geometry.isUndefined(i + 1, j) ||
        m_geometry.isUndefined(i + 1, j + 1);
    }

    virtual bool hasDeadCells() const
    {
      return true;
    }

    virtual size_t getTimeStamp() const
    {
      return m_timestamp;
    }
  };

  class SurfaceMesh : public MiSurfaceMeshCurvilinear
  {
    std::shared_ptr<SurfaceGeometry> m_geometry;
    std::shared_ptr<SurfaceTopology> m_topology;

  public:

    SurfaceMesh(
      std::shared_ptr<SurfaceGeometry> geometry,
      std::shared_ptr<SurfaceTopology> topology)
      : m_geometry(geometry)
      , m_topology(topology)
    {

    }
    virtual const MiGeometryIj& getGeometry() const
    {
      return *m_geometry;
    }

    virtual const MiTopologyIj& getTopology() const
    {
      return *m_topology;
    }
  };

  class SurfaceProperty : public MiDataSetIj<double>
  {
    boost::shared_ptr<CauldronIO::Map> m_map;
    std::string m_name;
    size_t m_timestamp;

  public:

    SurfaceProperty(const std::string& name, boost::shared_ptr<CauldronIO::Map> map)
      : m_map(map)
      , m_name(name)
      , m_timestamp(MxTimeStamp::getTimeStamp())
    {
    }

    virtual double get(size_t i, size_t j) const
    {
      return m_map->getValue(i, j);
    }

    virtual MiDataSet::DataBinding getBinding() const
    {
      return MiDataSet::PER_CELL;
    }

    virtual std::string getName() const
    {
      return m_name;
    }

    virtual size_t getTimeStamp() const
    {
      return m_timestamp;
    }
  };

  class VolumeProperty : public MiDataSetIjk<double>
  {
    boost::shared_ptr<CauldronIO::Volume> m_volume;
    std::string m_name;
    size_t m_timestamp;

  public:

    VolumeProperty(const std::string& name, boost::shared_ptr<CauldronIO::Volume> volume)
      : m_volume(volume)
      , m_name(name)
      , m_timestamp(MxTimeStamp::getTimeStamp())
    {
    }

    virtual double get(size_t i, size_t j, size_t k) const
    {
      return m_volume->getValue(i, j, k);
    }

    virtual MiDataSet::DataBinding getBinding() const
    {
      return MiDataSet::PER_CELL;
    }

    virtual std::string getName() const
    {
      return m_name;
    }

    virtual size_t getTimeStamp() const
    {
      return m_timestamp;
    }
  };

  class DiscontinuousVolumeProperty : public MiDataSetIjk<double>
  {
    CauldronIO::FormationVolumeList m_formationVolumeList;

    std::vector<std::tuple<size_t, CauldronIO::Volume*> > m_index;

    std::string m_name;
    size_t m_timestamp;

    void init()
    {
      // found the right property, sort list on k
      std::sort(
        m_formationVolumeList.begin(),
        m_formationVolumeList.end(),
        [](
        boost::shared_ptr<CauldronIO::FormationVolume> vol1,
        boost::shared_ptr<CauldronIO::FormationVolume> vol2)
      {
        size_t start1, end1, start2, end2;
        vol1->first->getDepthRange(start1, end1);
        vol2->first->getDepthRange(start2, end2);
        return start1 < start2;
      });

      size_t startK, endK;
      m_formationVolumeList[0]->first->getDepthRange(startK, endK);
      for (size_t i = 0; i < startK; ++i)
        m_index.push_back(std::make_tuple(i, nullptr));

      for (auto fv : m_formationVolumeList)
      {
        fv->first->getDepthRange(startK, endK);
        for (size_t i = startK; i < endK; ++i)
          m_index.push_back(std::make_tuple(i - startK, fv->second.get()));
      }
    }

  public:

    DiscontinuousVolumeProperty(const std::string& name, const CauldronIO::FormationVolumeList& fvlist)
      : m_formationVolumeList(fvlist)
      , m_name(name)
      , m_timestamp(MxTimeStamp::getTimeStamp())
    {
      init();
    }

    virtual double get(size_t i, size_t j, size_t k) const
    {
      if (k >= m_index.size())
        return 0.0;

      auto item = m_index[k];
      auto localK = std::get<0>(item);
      auto volume = std::get<1>(item);
      
      if (!volume)
        return 0.0;

      return volume->getValue(i, j, localK);
    }

    virtual MiDataSet::DataBinding getBinding() const
    {
      return MiDataSet::PER_CELL;
    }

    virtual std::string getName() const
    {
      return m_name;
    }

    virtual size_t getTimeStamp() const
    {
      return m_timestamp;
    }

  };
}

void VisualizationIOProject::init()
{
  const std::string nullStr("null");

  std::map<std::string, boost::shared_ptr<const CauldronIO::Formation> > formationMap;
  std::set<std::string> surfaceNames;
  std::set<std::string> propertyNames;

  m_snapshots = m_project->getSnapShots();

  // Filter out minor snapshots
  auto iter = std::remove_if(
    m_snapshots.begin(), 
    m_snapshots.end(), 
    [](boost::shared_ptr<CauldronIO::SnapShot> snapshot)
      {
        return snapshot->isMinorShapshot();
      });

  m_snapshots.erase(iter, m_snapshots.end());


  auto snapshot = m_snapshots[0];
  auto surfaceList = snapshot->getSurfaceList();
  int i = 0;
  for (auto surface : surfaceList)
  {
    auto formation = surface->getFormation();
    if (formation)
      formationMap[formation->getName()] = formation;
    if (surface->getDepthSurface())
      surfaceNames.insert(surface->getName());
    propertyNames.insert(surface->getProperty()->getName());
  }

  std::vector<boost::shared_ptr<const CauldronIO::Formation> > formations;
  for (auto p : formationMap)
    formations.push_back(p.second);

  std::sort(
    formations.begin(), 
    formations.end(), 
    [](
        boost::shared_ptr<const CauldronIO::Formation> f1, 
        boost::shared_ptr<const CauldronIO::Formation> f2) 
      {
        size_t start1, end1, start2, end2;
        f1->getDepthRange(start1, end1);
        f2->getDepthRange(start2, end2);
        return start1 < start2;
      });

  for (auto fmt : formations)
  {
    Formation formation = { fmt->getName(), false };
    m_projectInfo.formations.push_back(formation);
  }

  for (auto name : surfaceNames)
  {
    Surface surface = { name };
    m_projectInfo.surfaces.push_back(surface);
  }

  for (auto name : propertyNames)
  {
    Property prop = { name, "???" };
    m_projectInfo.properties.push_back(prop);
  }
}

VisualizationIOProject::VisualizationIOProject(const std::string& path)
  : m_numCellsI(0)
  , m_numCellsJ(0)
  , m_numCellsIHiRes(0)
  , m_numCellsJHiRes(0)
  , m_deltaX(1.0)
  , m_deltaY(1.0)
  , m_minX(0.0)
  , m_minY(0.0)
{
  m_project = CauldronIO::ImportExport::importFromXML(path);

  init();
}

Project::ProjectInfo VisualizationIOProject::getProjectInfo() const
{
  return m_projectInfo;
}

unsigned int VisualizationIOProject::getMaxPersistentTrapId() const
{
  return 0;
}

int VisualizationIOProject::numCellsI() const
{
  return m_numCellsI;
}

int VisualizationIOProject::numCellsJ() const
{
  return m_numCellsJ;
}

int VisualizationIOProject::numCellsIHiRes() const
{
  return m_numCellsIHiRes;
}

int VisualizationIOProject::numCellsJHiRes() const
{
  return m_numCellsJHiRes;
}

double VisualizationIOProject::deltaX() const
{
  return m_deltaX;
}

double VisualizationIOProject::deltaY() const
{
  return m_deltaY;
}

double VisualizationIOProject::deltaXHiRes() const
{
  return m_deltaXHiRes;
}

double VisualizationIOProject::deltaYHiRes() const
{
  return m_deltaYHiRes;
}

double VisualizationIOProject::minX() const
{
  return m_minX;
}

double VisualizationIOProject::minY() const
{
  return m_minY;
}

int VisualizationIOProject::getPropertyId(const std::string& name) const
{
  return 0;
}

size_t VisualizationIOProject::getSnapshotCount() const
{
  return m_snapshots.size();
}

Project::SnapshotContents VisualizationIOProject::getSnapshotContents(size_t snapshotIndex) const
{
  SnapshotContents contents;

  auto snapshot = m_snapshots[snapshotIndex];

  contents.age = m_snapshots[snapshotIndex]->getAge();

  // Get all unique surface names in this snapshot
  std::set<std::string> surfaceNames;
  std::map<std::string, boost::shared_ptr<const CauldronIO::Formation> > formationMap;

  auto surfaceList = snapshot->getSurfaceList();
  for (auto surface : surfaceList)
  {
    if (surface->getDepthSurface())
      surfaceNames.insert(surface->getName());

    auto formation = surface->getFormation();
    if (formation)
      formationMap[formation->getName()] = formation;
  }

  for (auto name : surfaceNames)
  {
    // find id for the name
    for (int i = 0; i < (int)m_projectInfo.surfaces.size(); ++i)
    {
      if (m_projectInfo.surfaces[i].name == name)
      {
        contents.surfaces.push_back(i);
        break;
      }
    }
  }

  for (int i = 0; i < (int)m_projectInfo.formations.size(); ++i)
  {
    auto iter = formationMap.find(m_projectInfo.formations[i].name);
    if (iter != formationMap.end())
    {
      size_t startK, endK;
      iter->second->getDepthRange(startK, endK);

      Project::SnapshotFormation formation;
      formation.id = i;
      formation.minK = (int)startK;
      formation.maxK = (int)endK;

      contents.formations.push_back(formation);
    }
  }

  return contents;
}

std::shared_ptr<MiVolumeMeshCurvilinear> VisualizationIOProject::createSnapshotMesh(size_t snapshotIndex) const
{
  auto snapshot = m_snapshots[snapshotIndex];

  auto volumeList = snapshot->getVolumeList();
  for (auto volume : volumeList)
  {
    auto depthVolume = boost::const_pointer_cast<CauldronIO::Volume>(volume->getDepthVolume());
    if (depthVolume)
    {
      if (!depthVolume->isRetrieved())
        depthVolume->retrieve();

      auto geometry = std::make_shared<VizIO::VolumeGeometry>(depthVolume);
      auto topology = std::make_shared<VizIO::VolumeTopology>(*geometry);
      return std::make_shared<VizIO::VolumeMesh>(geometry, topology);
    }
  }

  return nullptr;
}

std::shared_ptr<MiVolumeMeshCurvilinear> VisualizationIOProject::createReservoirMesh(
  size_t snapshotIndex,
  int reservoirId) const
{
  return nullptr;
}

std::shared_ptr<MiSurfaceMeshCurvilinear> VisualizationIOProject::createSurfaceMesh(
  size_t snapshotIndex,
  int surfaceId) const
{
  auto snapshot = m_snapshots[snapshotIndex];

  auto surfaceList = snapshot->getSurfaceList();
  for (auto surface : surfaceList)
  {
    std::string name = m_projectInfo.surfaces[surfaceId].name;
    if (surface->getName() == name && surface->getDepthSurface())
    {
      auto depthMap = surface->getDepthSurface()->getValueMap();
      if (!depthMap->isRetrieved())
        depthMap->retrieve();

      auto geometry = std::make_shared<VizIO::SurfaceGeometry>(depthMap);
      auto topology = std::make_shared<VizIO::SurfaceTopology>(*geometry);
      return std::make_shared<VizIO::SurfaceMesh>(geometry, topology);
    }
  }

  return nullptr;
}

std::shared_ptr<MiDataSetIjk<double> > VisualizationIOProject::createFormationProperty(
  size_t snapshotIndex,
  int propertyId) const
{
  if (propertyId < 0 || propertyId >= (int)m_projectInfo.properties.size())
    return nullptr;

  auto snapshot = m_snapshots[snapshotIndex];

  std::string propertyName = m_projectInfo.properties[propertyId].name;

  auto volumeList = snapshot->getVolumeList();
  for (auto volume : volumeList)
  {
    if (volume->getProperty()->getName() == propertyName)
    {
      if (!volume->isRetrieved())
        volume->retrieve();
      return std::make_shared<VizIO::VolumeProperty>(propertyName, volume);
    }
  }

  auto discVolumeList = snapshot->getDiscontinuousVolumeList();
  for (auto discVolume : discVolumeList)
  {
    auto formationVolumeList = discVolume->getVolumeList();
    if (formationVolumeList.empty())
      continue;
    
    std::string name = formationVolumeList[0]->second->getProperty()->getName();
    if (name != propertyName)
      continue;

    for (auto fv : formationVolumeList)
    if (!fv->second->isRetrieved())
      fv->second->retrieve();

    return std::make_shared<VizIO::DiscontinuousVolumeProperty>(name, formationVolumeList);
  }

  return nullptr;

}

std::shared_ptr<MiDataSetIj<double> > VisualizationIOProject::createSurfaceProperty(
  size_t snapshotIndex,
  int surfaceId,
  int propertyId) const
{
  if (propertyId < 0 || propertyId >= (int)m_projectInfo.properties.size())
    return nullptr;

  auto snapshot = m_snapshots[snapshotIndex];

  std::string surfaceName = m_projectInfo.surfaces[surfaceId].name;
  std::string propertyName = m_projectInfo.properties[propertyId].name;

  auto surfaceList = snapshot->getSurfaceList();
  for (auto surface : surfaceList)
  {
    if (surface->getName() == surfaceName && surface->getProperty()->getName() == propertyName)
    {
      auto map = surface->getValueMap();
      if (!map->isRetrieved())
        map->retrieve();
      return std::make_shared<VizIO::SurfaceProperty>(propertyName, map);
    }
  }

  return nullptr;
}

std::shared_ptr<MiDataSetIjk<double> > VisualizationIOProject::createReservoirProperty(
  size_t snapshotIndex,
  int reservoirId,
  int propertyId) const
{
  return nullptr;
}

std::shared_ptr<MiDataSetIjk<double> > VisualizationIOProject::createPersistentTrapIdProperty(
  size_t snapshotIndex,
  int reservoirId) const
{
  return nullptr;
}

std::shared_ptr<MiDataSetIjk<double> > VisualizationIOProject::createFlowDirectionProperty(size_t snapshotIndex) const
{
  return nullptr;
}

std::vector<Project::Trap> VisualizationIOProject::getTraps(size_t snapshotIndex, int reservoirId) const
{
  std::vector<Trap> traps;

  return traps;
}

std::vector<SbVec2d> VisualizationIOProject::getFaultLine(int faultId) const
{
  std::vector<SbVec2d> line;

  return line;
}

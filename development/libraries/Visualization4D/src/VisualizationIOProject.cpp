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
      double x = /*m_minX + */ i * m_deltaX;
      double y = /*m_minY + */ j * m_deltaY;
      double z = -m_volume->getValue(i, j, k);

      return MbVec3d(x, y, z);
    }

    virtual MiMeshIjk::StorageLayout getStorageLayout() const
    {
      return MiMeshIjk::LAYOUT_KJI;
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

  template<class T>
  class VolumeTopology : public MiTopologyIjk
  {
    const T&    m_geometry;
    const bool* m_deadMap;
    size_t      m_timestamp;
    size_t      m_numI;
    size_t      m_numJ;
    size_t      m_numK;

  public:

    VolumeTopology(const T& geometry, const bool* deadMap)
      : m_geometry(geometry)
      , m_deadMap(deadMap)
      , m_timestamp(MxTimeStamp::getTimeStamp())
    {
      m_numI = m_geometry.numI() - 1;
      m_numJ = m_geometry.numJ() - 1;
      m_numK = m_geometry.numK() - 1;
    }

    virtual size_t getNumCellsI() const
    {
      return m_numI;
    }

    virtual size_t getNumCellsJ() const
    {
      return m_numJ;
    }

    virtual size_t getNumCellsK() const
    {
      return m_numK;
    }

    virtual bool isDead(size_t i, size_t j, size_t /*k*/) const
    {
      return m_deadMap[i * m_numJ + j];
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

  typedef VolumeTopology<VolumeGeometry> FormationTopology;

  class VolumeMesh : public MiVolumeMeshCurvilinear
  {
    std::shared_ptr<MiGeometryIjk> m_geometry;
    std::shared_ptr<MiTopologyIjk> m_topology;

  public:

    VolumeMesh(
      std::shared_ptr<MiGeometryIjk> geometry,
      std::shared_ptr<MiTopologyIjk> topology)
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

  class ReservoirGeometry : public MiGeometryIjk
  {
    boost::shared_ptr<CauldronIO::Map> m_depthMaps[2];

    double m_minX;
    double m_minY;
    double m_deltaX;
    double m_deltaY;

    size_t m_numI;
    size_t m_numJ;

    float  m_undefined;
    
    size_t m_timestamp;

  public:

    ReservoirGeometry(
      boost::shared_ptr<CauldronIO::Map> topMap,
      boost::shared_ptr<CauldronIO::Map> bottomMap)
      : m_timestamp(MxTimeStamp::getTimeStamp())
    {
      m_depthMaps[0] = topMap;
      m_depthMaps[1] = bottomMap;

      m_minX = topMap->getMinI();
      m_minY = topMap->getMinJ();
      m_deltaX = topMap->getDeltaI();
      m_deltaY = topMap->getDeltaJ();

      m_numI = topMap->getNumI();
      m_numJ = topMap->getNumJ();

      m_undefined = topMap->getUndefinedValue();
    }

    virtual MbVec3d getCoord(size_t i, size_t j, size_t k) const
    {
      return MbVec3d(
        i * m_deltaX,
        j * m_deltaY,
        -m_depthMaps[k]->getValue(i, j));
    }

    bool isUndefined(size_t i, size_t j, size_t k) const
    {
      return m_depthMaps[k]->getValue(i, j) == m_undefined;
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
      return 2;
    }

    virtual size_t getTimeStamp() const
    {
      return m_timestamp;
    }
  };

  typedef VolumeTopology<ReservoirGeometry> ReservoirTopology;

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
      double x = /*m_minX + */i * m_deltaX;
      double y = /*m_minY + */j * m_deltaY;
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

  void getMinMax(boost::shared_ptr<CauldronIO::Volume> volume, float& minValue, float& maxValue)
  {
    const float undefined = volume->getUndefinedValue();

    if (volume->isConstant())
    {
      minValue = volume->getConstantValue();
      maxValue = minValue;
    }
    else
    {
      const float* values = volume->hasDataIJK()
        ? volume->getVolumeValues_IJK()
        : volume->getVolumeValues_KIJ();

      size_t n = volume->getNumI() * volume->getNumJ() * volume->getNumK();
      minValue =  std::numeric_limits<float>::infinity();
      maxValue = -std::numeric_limits<float>::infinity();
      for (size_t i = 0; i < n; ++i)
      {
        float val = values[i];
        if (val != undefined)
        {
          minValue = std::min(minValue, val);
          maxValue = std::max(maxValue, val);
        }
      }
    }
  }

  void getMinMax(boost::shared_ptr<CauldronIO::Map> map, float& minValue, float& maxValue)
  {
    const float undefined = map->getUndefinedValue();

    if (map->isConstant())
    {
      minValue = map->getConstantValue();
      maxValue = minValue;
    }
    else
    {
      const float* values = map->getSurfaceValues();

      size_t n = map->getNumI() * map->getNumJ();
      minValue =  std::numeric_limits<float>::infinity();
      maxValue = -std::numeric_limits<float>::infinity();
      for (size_t i = 0; i < n; ++i)
      {
        float val = values[i];
        if (val != undefined)
        {
          minValue = std::min(minValue, val);
          maxValue = std::max(maxValue, val);
        }
      }
    }
  }

  class VolumeProperty : public MiDataSetIjk<double>
  {
    boost::shared_ptr<CauldronIO::Volume> m_volume;
    std::string m_name;
    size_t m_timestamp;

    // these are mutable because of lazy initialization
    mutable float m_minValue;
    mutable float m_maxValue;
    mutable bool m_minMaxValid;

    void initMinMax() const
    {
      getMinMax(m_volume, m_minValue, m_maxValue);
    }

  public:

    VolumeProperty(const std::string& name, boost::shared_ptr<CauldronIO::Volume> volume)
      : m_volume(volume)
      , m_name(name)
      , m_timestamp(MxTimeStamp::getTimeStamp())
      , m_minValue(0.0)
      , m_maxValue(0.0)
      , m_minMaxValid(false)
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

    virtual double getMin() const
    {
      if (!m_minMaxValid)
        initMinMax();

      return m_minValue;
    }

    virtual double getMax() const
    {
      if (!m_minMaxValid)
        initMinMax();

      return m_maxValue;
    }
  };

  class DiscontinuousVolumeProperty : public MiDataSetIjk<double>
  {
    CauldronIO::FormationVolumeList m_formationVolumeList;

    //std::vector<std::tuple<size_t, CauldronIO::Volume*> > m_index;
    std::vector<CauldronIO::Volume*> m_index;

    std::string m_name;
    size_t m_timestamp;

    mutable float m_minValue;
    mutable float m_maxValue;
    mutable bool m_minMaxValid;

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
        unsigned int start1, end1, start2, end2;
        vol1->first->getK_Range(start1, end1);
        vol2->first->getK_Range(start2, end2);
        return start1 < start2;
      });

      unsigned int startK, endK;
      m_formationVolumeList[0]->first->getK_Range(startK, endK);
      for (unsigned int i = 0; i < startK; ++i)
        m_index.push_back(nullptr);

      for (auto fv : m_formationVolumeList)
      {
        fv->first->getK_Range(startK, endK);
        for (unsigned int i = startK; i < endK; ++i)
          m_index.push_back(fv->second.get());
      }
    }

    void initMinMax() const
    {
      m_minValue =  std::numeric_limits<float>::infinity();
      m_maxValue = -std::numeric_limits<float>::infinity();

      for (auto fv : m_formationVolumeList)
      {
        float minval, maxval;
        getMinMax(fv->second, minval, maxval);

        m_minValue = std::min(m_minValue, minval);
        m_maxValue = std::max(m_maxValue, maxval);
      }

      m_minMaxValid = true;
    }

  public:

    DiscontinuousVolumeProperty(const std::string& name, const CauldronIO::FormationVolumeList& fvlist)
      : m_formationVolumeList(fvlist)
      , m_name(name)
      , m_timestamp(MxTimeStamp::getTimeStamp())
      , m_minValue(0.0)
      , m_maxValue(0.0)
      , m_minMaxValid(false)
    {
      init();
    }

    virtual double get(size_t i, size_t j, size_t k) const
    {
      if (k >= m_index.size())
        return 0.0;

      auto volume = m_index[k];
      
      if (!volume)
        return 0.0;

      return volume->getValue(i, j, k);
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

    virtual double getMin() const
    {
      if (!m_minMaxValid)
        initMinMax();

      return m_minValue;
    }

    virtual double getMax() const
    {
      if (!m_minMaxValid)
        initMinMax();

      return m_maxValue;
    }
  };

  class ReservoirProperty : public MiDataSetIjk<double>
  {
    boost::shared_ptr<CauldronIO::Map> m_map;
    std::string m_name;
    size_t m_timestamp;

    // these are mutable because of lazy initialization
    mutable float m_minValue;
    mutable float m_maxValue;
    mutable bool m_minMaxValid;

    void initMinMax() const
    {
      getMinMax(m_map, m_minValue, m_maxValue);
    }

  public:

    ReservoirProperty(const std::string& name, boost::shared_ptr<CauldronIO::Map> map)
      : m_map(map)
      , m_name(name)
      , m_timestamp(MxTimeStamp::getTimeStamp())
      , m_minValue(0.0)
      , m_maxValue(0.0)
      , m_minMaxValid(false)
    {
    }

    virtual double get(size_t i, size_t j, size_t /*k*/) const
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

    virtual double getMin() const
    {
      if (!m_minMaxValid)
        initMinMax();

      return m_minValue;
    }

    virtual double getMax() const
    {
      if (!m_minMaxValid)
        initMinMax();

      return m_maxValue;
    }
  };

}

namespace
{
  bool* createDeadMap(boost::shared_ptr<CauldronIO::Map> map)
  {
    const float undefined = map->getUndefinedValue();

    size_t ni = map->getNumI() - 1;
    size_t nj = map->getNumJ() - 1;

    bool* deadMap = new bool[ni * nj];
    for (size_t i = 0; i < ni; ++i)
    {
      for (size_t j = 0; j < nj; ++j)
      {
        deadMap[nj * i + j] =
          map->getValue(i, j) == undefined ||
          map->getValue(i, j + 1) == undefined ||
          map->getValue(i + 1, j) == undefined ||
          map->getValue(i + 1, j + 1) == undefined;
      }
    }

    return deadMap;
  }

  bool* createDeadMap(boost::shared_ptr<CauldronIO::Volume> volume)
  {
    const float undefined = volume->getUndefinedValue();

    size_t ni = volume->getNumI() - 1;
    size_t nj = volume->getNumJ() - 1;

    bool* deadMap = new bool[ni * nj];
    for (size_t i = 0; i < ni; ++i)
    {
      for (size_t j = 0; j < nj; ++j)
      {
        deadMap[nj * i + j] =
          volume->getValue(i, j, 0u) == undefined ||
          volume->getValue(i, j + 1, 0u) == undefined ||
          volume->getValue(i + 1, j, 0u) == undefined ||
          volume->getValue(i + 1, j + 1, 0u) == undefined;
      }
    }

    return deadMap;
  }

}

void VisualizationIOProject::init()
{
  const std::string nullStr("null");

  std::map<std::string, boost::shared_ptr<const CauldronIO::Formation> > formationMap;
  std::set<std::string> surfaceNames;
  std::set<std::string> propertyNames;
  std::set<std::string> reservoirNames;

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

  int numCellsIHiRes = 0;
  int numCellsJHiRes = 0;
  double deltaXHiRes = 0.0;
  double deltaYHiRes = 0.0;
  for (auto surface : surfaceList)
  {
    auto type = surface->getProperty()->getType();
    if (type == CauldronIO::ReservoirProperty)
    {
      reservoirNames.insert(surface->getReservoirName());
      if (numCellsIHiRes == 0 && numCellsJHiRes == 0)
      {
        auto map = surface->getValueMap();
        numCellsIHiRes = (int)map->getNumI() - 1;
        numCellsJHiRes = (int)map->getNumJ() - 1;
        deltaXHiRes = (int)map->getDeltaI();
        deltaYHiRes = (int)map->getDeltaJ();
      }
    }
    else
    {
      auto formation = surface->getFormation();
      if (formation)
        formationMap[formation->getName()] = formation;

      if (surface->getDepthSurface())
        surfaceNames.insert(surface->getName());
    }

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
        unsigned int start1, end1, start2, end2;
        f1->getK_Range(start1, end1);
        f2->getK_Range(start2, end2);
        return start1 < start2;
      });

  for (auto fmt : formations)
  {
    Formation formation = { fmt->getName(), 0, false };
    m_projectInfo.formations.push_back(formation);
  }

  for (auto name : surfaceNames)
  {
    Surface surface = { name };
    m_projectInfo.surfaces.push_back(surface);
  }

  for (auto name : reservoirNames)
  {
    Reservoir reservoir = { name };
    m_projectInfo.reservoirs.push_back(reservoir);
  }

  for (auto name : propertyNames)
  {
    Property prop = { name, "???" };
    m_projectInfo.properties.push_back(prop);
  }

  m_projectInfo.snapshotCount = m_snapshots.size();

  auto volume = m_snapshots[0]->getVolumeList()[0];
  m_projectInfo.dimensions.minX = volume->getMinI();
  m_projectInfo.dimensions.minY = volume->getMinJ();
  m_projectInfo.dimensions.deltaX = volume->getDeltaI();
  m_projectInfo.dimensions.deltaY = volume->getDeltaJ();
  m_projectInfo.dimensions.deltaXHiRes = deltaXHiRes;
  m_projectInfo.dimensions.deltaYHiRes = deltaYHiRes;
  m_projectInfo.dimensions.numCellsI = (int)volume->getNumI() - 1;
  m_projectInfo.dimensions.numCellsJ = (int)volume->getNumJ() - 1;
  m_projectInfo.dimensions.numCellsIHiRes = numCellsIHiRes;
  m_projectInfo.dimensions.numCellsJHiRes = numCellsJHiRes;
}

VisualizationIOProject::VisualizationIOProject(const std::string& path)
  : m_loresDeadMap(nullptr)
  , m_hiresDeadMap(nullptr)
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
  int maxId = 0;

  for (auto snapshot : m_snapshots)
  {
    auto const& traps = snapshot->getTrapperList();
    for (auto trap : traps)
    {
      int pid = trap->getPersistentID();
      maxId = std::max(maxId, pid);
    }
  }

  return (unsigned int)maxId;
}

int VisualizationIOProject::getPropertyId(const std::string& name) const
{
  for (int i = 0; i < (int)m_projectInfo.properties.size(); ++i)
  {
    if (m_projectInfo.properties[i].name == name)
      return i;
  }

  return -1;
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
  std::set<std::string> reservoirNames;
  std::map<std::string, boost::shared_ptr<const CauldronIO::Formation> > formationMap;

  auto surfaceList = snapshot->getSurfaceList();
  for (auto surface : surfaceList)
  {
    if (surface->getDepthSurface())
      surfaceNames.insert(surface->getName());

    auto formation = surface->getFormation();
    if (formation)
      formationMap[formation->getName()] = formation;

    if (surface->getProperty()->getType() == CauldronIO::ReservoirProperty)
      reservoirNames.insert(surface->getReservoirName());
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

  for (auto name : reservoirNames)
  {
    for (int i = 0; i < (int)m_projectInfo.reservoirs.size(); ++i)
    {
      if (m_projectInfo.reservoirs[i].name == name)
      {
        contents.reservoirs.push_back(i);
        break;
      }
    }
  }

  for (int i = 0; i < (int)m_projectInfo.formations.size(); ++i)
  {
    auto iter = formationMap.find(m_projectInfo.formations[i].name);
    if (iter != formationMap.end())
    {
      unsigned int startK, endK;
      iter->second->getK_Range(startK, endK);

      Project::SnapshotFormation formation;
      formation.id = i;
      formation.minK = (int)startK;
      formation.maxK = (int)endK;

      contents.formations.push_back(formation);
    }
  }

  // No formations? then no need to continue
  if (contents.formations.empty())
    return contents;

  // get minZ / maxZ
  auto volumeList = snapshot->getVolumeList();
  for (auto volume : volumeList)
  {
    auto depthVolume = boost::const_pointer_cast<CauldronIO::Volume>(volume->getDepthVolume());
    if (depthVolume)
    {
      if (!depthVolume->isRetrieved())
        depthVolume->retrieve();

      const float undefined = depthVolume->getUndefinedValue();

      float minDepth =  std::numeric_limits<float>::infinity();
      float maxDepth = -std::numeric_limits<float>::infinity();

      size_t ni = depthVolume->getNumI();
      size_t nj = depthVolume->getNumJ();
      size_t k = (size_t)contents.formations[contents.formations.size() - 1].maxK;

      for (size_t j = 0; j < nj; ++j)
      {
        for (size_t i = 0; i < ni; ++i)
        {
          float topval = depthVolume->getValue(i, j, 0);
          if (topval != undefined)
          {
            float bottomval = depthVolume->getValue(i, j, k);
            minDepth = std::min(minDepth, topval);
            maxDepth = std::max(maxDepth, bottomval);
          }
        }
      }

      contents.minDepth = minDepth;
      contents.maxDepth = maxDepth;

      break;
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

      if (!m_loresDeadMap)
        m_loresDeadMap = createDeadMap(depthVolume);

      auto geometry = std::make_shared<VizIO::VolumeGeometry>(depthVolume);
      auto topology = std::make_shared<VizIO::FormationTopology>(*geometry, m_loresDeadMap);
      return std::make_shared<VizIO::VolumeMesh>(geometry, topology);
    }
  }

  return nullptr;
}

std::shared_ptr<MiVolumeMeshCurvilinear> VisualizationIOProject::createReservoirMesh(
  size_t snapshotIndex,
  int reservoirId) const
{
  auto snapshot = m_snapshots[snapshotIndex];
  auto surfaceList = snapshot->getSurfaceList();

  std::string name = m_projectInfo.reservoirs[reservoirId].name;

  boost::shared_ptr<CauldronIO::Map> topMap, bottomMap;

  for (auto surface : surfaceList)
  {
    auto prop = surface->getProperty();
    if (prop->getType() == CauldronIO::ReservoirProperty && surface->getReservoirName() == name)
    {
      const std::string& name = prop->getName();
      if (name == "ResRockTop")
        topMap = surface->getValueMap();
      else if (name == "ResRockBottom")
        bottomMap = surface->getValueMap();
    }
  }

  if (topMap && bottomMap)
  {
    if (!topMap->isRetrieved())
      topMap->retrieve();
    if (!bottomMap->isRetrieved())
      bottomMap->retrieve();

    if (!m_hiresDeadMap)
      m_hiresDeadMap = createDeadMap(topMap);

    auto geometry = std::make_shared<VizIO::ReservoirGeometry>(topMap, bottomMap);
    auto topology = std::make_shared<VizIO::ReservoirTopology>(*geometry, m_hiresDeadMap);
    return std::make_shared<VizIO::VolumeMesh>(geometry, topology);
  }

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
    {
      if (!fv->second->isRetrieved())
        fv->second->retrieve();
    }

    return std::make_shared<VizIO::DiscontinuousVolumeProperty>(name, formationVolumeList);
  }

  return nullptr;

}

std::shared_ptr<MiDataSetIj<double> > VisualizationIOProject::createFormation2DProperty(
  size_t snapshotIndex,
  int formationId,
  int propertyId) const
{
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
  if (propertyId < 0 || propertyId >= (int)m_projectInfo.properties.size())
    return nullptr;

  auto snapshot = m_snapshots[snapshotIndex];

  std::string reservoirName = m_projectInfo.reservoirs[reservoirId].name;
  std::string propertyName = m_projectInfo.properties[propertyId].name;

  auto surfaceList = snapshot->getSurfaceList();
  for (auto surface : surfaceList)
  {
    auto prop = surface->getProperty();

    if (
      prop->getType() == CauldronIO::ReservoirProperty &&
      prop->getName() == propertyName &&
      surface->getReservoirName() == reservoirName)
    {
      auto map = surface->getValueMap();
      if (!map->isRetrieved())
        map->retrieve();
      return std::make_shared<VizIO::ReservoirProperty>(propertyName, map);
    }
  }

  return nullptr;
}

std::shared_ptr<MiDataSetIjk<double> > VisualizationIOProject::createPersistentTrapIdProperty(
  size_t /*snapshotIndex*/,
  int /*reservoirId*/) const
{
  return nullptr;
}

std::shared_ptr<MiDataSetIjk<double> > VisualizationIOProject::createFlowDirectionProperty(size_t /*snapshotIndex*/) const
{
  return nullptr;
}

std::vector<Project::Trap> VisualizationIOProject::getTraps(size_t snapshotIndex, int reservoirId) const
{
  auto snapshot = m_snapshots[snapshotIndex];

  std::string reservoirName = m_projectInfo.reservoirs[reservoirId].name;

  std::vector<Trap> traps;
  for (auto trapper : snapshot->getTrapperList())
  {
    if (trapper->getReservoirName() == reservoirName)
    {
      Project::Trap trap;

      float x, y, z;
      trapper->getSpillPointPosition(x, y);
      x -= (float)m_projectInfo.dimensions.minX;
      y -= (float)m_projectInfo.dimensions.minY;
      z = -trapper->getSpillDepth();
      trap.spillPoint = SbVec3f(x, y, z);

      trapper->getPosition(x, y);
      x -= (float)m_projectInfo.dimensions.minX;
      y -= (float)m_projectInfo.dimensions.minY;
      z = -trapper->getDepth();
      trap.leakagePoint = SbVec3f(x, y, z);

      trap.id = trapper->getID();
      trap.downStreamId = trapper->getDownStreamTrapperID();
      trap.gasOilContactDepth = 0.0; // not available in API
      trap.oilWaterContactDepth = 0.0; // not available in API

      traps.push_back(trap);
    }
  }

  std::sort(traps.begin(), traps.end(), [](const Trap& lhs, const Trap& rhs) { return lhs.id < rhs.id;  });

  return traps;
}

std::vector<SbVec2d> VisualizationIOProject::getFaultLine(int /*faultId*/) const
{
  std::vector<SbVec2d> line;

  return line;
}

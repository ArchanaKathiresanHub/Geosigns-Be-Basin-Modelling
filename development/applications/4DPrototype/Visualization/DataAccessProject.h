//
// Copyright (C) 2012-2015 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef PROJECTWRAPPER_H_INCLUDED
#define PROJECTWRAPPER_H_INCLUDED

#include "defines.h"
#include "Project.h"

#include <memory>
#include <vector>
#include <string>
#include <map>

#include <Inventor/SbVec.h>

class MiSurfaceMeshCurvilinear;
class MiVolumeMeshCurvilinear;
template<class T>
class MiDataSetIj;
template<class T>
class MiDataSetIjk;
template<class T>
class MbVec3;

namespace DataAccess
{
  namespace Interface
  {
    class ProjectHandle;
    class ObjectFactory;
    class Formation;
    class Surface;
    class Reservoir;
    class Fault;
    class Property;
    class Snapshot;
    class GridMap;
  }
}

class VISUALIZATIONDLL_API DataAccessProject : public Project
{
  std::shared_ptr<DataAccess::Interface::ObjectFactory> m_objectFactory;
  std::shared_ptr<DataAccess::Interface::ProjectHandle> m_projectHandle;

  ProjectInfo m_projectInfo;

  const DataAccess::Interface::Property* m_depthProperty;
  const DataAccess::Interface::Property* m_resRockTopProperty;
  const DataAccess::Interface::Property* m_resRockBottomProperty;
  const DataAccess::Interface::Property* m_resRockTrapIdProperty;
  const DataAccess::Interface::Property* m_resRockDrainageIdGasPhaseProperty;
  const DataAccess::Interface::Property* m_resRockDrainageIdFluidPhaseProperty;
  const DataAccess::Interface::Property* m_flowDirectionProperty;
  const DataAccess::Interface::Property* m_currentProperty;

  std::vector<const DataAccess::Interface::Property*>  m_properties;
  std::vector<const DataAccess::Interface::Snapshot*>  m_snapshotList;
  std::vector<const DataAccess::Interface::Formation*> m_formations;
  std::vector<const DataAccess::Interface::Surface*>   m_surfaces;
  std::vector<const DataAccess::Interface::Reservoir*> m_reservoirs;
  std::vector<const DataAccess::Interface::Fault*>     m_faults;

  std::map<std::string, int> m_propertyIdMap;
  std::map<std::string, int> m_formationIdMap;
  std::map<std::string, int> m_surfaceIdMap;
  std::map<std::string, int> m_reservoirIdMap;
  std::map<
    std::tuple<
      std::string, 
      std::string>, 
    int> m_faultMap;

  mutable bool* m_loresDeadMap;
  mutable bool* m_hiresDeadMap;

  std::vector<const DataAccess::Interface::GridMap*> getFormationPropertyGridMaps(
    size_t snapshotIndex,
    const DataAccess::Interface::Property* prop,
    bool formation3D) const;

  void init();

public:

  explicit DataAccessProject(const std::string& path);

  virtual ~DataAccessProject();

  virtual ProjectInfo getProjectInfo() const;

  virtual unsigned int getMaxPersistentTrapId() const;

  virtual int getPropertyId(const std::string& name) const;

  virtual size_t getSnapshotCount() const;

  virtual SnapshotContents getSnapshotContents(size_t snapshotIndex) const;

  virtual std::shared_ptr<MiVolumeMeshCurvilinear> createSnapshotMesh(size_t snapshotIndex) const;

  virtual std::shared_ptr<MiVolumeMeshCurvilinear> createReservoirMesh(
    size_t snapshotIndex,
    int reservoirId) const;

  virtual std::shared_ptr<MiSurfaceMeshCurvilinear> createSurfaceMesh(
    size_t snapshotIndex,
    int surfaceId) const;

  virtual std::shared_ptr<MiDataSetIjk<double> > createFormationProperty(
    size_t snapshotIndex,
    int propertyId) const;

  virtual std::shared_ptr<MiDataSetIj<double> > createSurfaceProperty(
    size_t snapshotIndex,
    int surfaceId,
    int propertyId) const;

  virtual std::shared_ptr<MiDataSetIjk<double> > createReservoirProperty(
    size_t snapshotIndex,
    int reservoirId,
    int propertyId) const;

  virtual std::shared_ptr<MiDataSetIjk<double> > createPersistentTrapIdProperty(
    size_t snapshotIndex,
    int reservoirId) const;

  virtual std::shared_ptr<MiDataSetIjk<double> > createFlowDirectionProperty(size_t snapshotIndex) const;

  virtual std::vector<Trap> getTraps(size_t snapshotIndex, int reservoirId) const;

  virtual std::vector<SbVec2d> getFaultLine(int faultId) const;
};

#endif

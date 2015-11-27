//
// Copyright (C) 2012-2015 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef PROJECT_H_INCLUDED
#define PROJECT_H_INCLUDED

#include "defines.h"

#include <memory>
#include <vector>
#include <string>

#include <Inventor/SbVec.h>

class MiSurfaceMeshCurvilinear;
class MiVolumeMeshCurvilinear;
template<class T>
class MiDataSetIj;
template<class T>
class MiDataSetIjk;
template<class T>
class MbVec3;

class VISUALIZATIONDLL_API Project
{
public:

  struct Formation
  {
    std::string name;
    int numCellsK;
    bool isSourceRock;
  };

  struct Surface
  {
    std::string name;
  };

  struct Reservoir
  {
    std::string name;
  };

  struct FaultCollection
  {
    std::string name;
    std::vector<int> formations;
  };

  struct Fault
  {
    int collectionId;
    std::string name;
  };

  struct Property
  {
    std::string name;
    std::string unit;
  };

  struct ProjectInfo
  {
    std::vector<Formation> formations;
    std::vector<Surface>   surfaces;
    std::vector<Reservoir> reservoirs;
    std::vector<FaultCollection> faultCollections;
    std::vector<Fault>     faults;
    std::vector<Property>  properties;
  };

  struct SnapshotFormation
  {
    int id;
    int minK;
    int maxK; //exclusive
  };

  struct SnapshotContents
  {
    double age;

    std::vector<SnapshotFormation> formations;
    std::vector<int> surfaces;
    std::vector<int> reservoirs;
  };

  struct Trap
  {
    int id;
    int downStreamId;

    SbVec3f spillPoint;
    SbVec3f leakagePoint;
  };

  virtual ~Project() {}

  virtual ProjectInfo getProjectInfo() const = 0;

  virtual unsigned int getMaxPersistentTrapId() const = 0;

  virtual int numCellsI() const = 0;

  virtual int numCellsJ() const = 0;

  virtual int numCellsIHiRes() const = 0;

  virtual int numCellsJHiRes() const = 0;

  virtual double deltaX() const = 0;

  virtual double deltaY() const = 0;

  virtual double deltaXHiRes() const = 0;

  virtual double deltaYHiRes() const = 0;

  virtual double minX() const = 0;

  virtual double minY() const = 0;

  virtual int getPropertyId(const std::string& name) const = 0;

  virtual size_t getSnapshotCount() const = 0;

  virtual SnapshotContents getSnapshotContents(size_t snapshotIndex) const = 0;

  virtual std::shared_ptr<MiVolumeMeshCurvilinear> createSnapshotMesh(size_t snapshotIndex) const = 0;

  virtual std::shared_ptr<MiVolumeMeshCurvilinear> createReservoirMesh(
    size_t snapshotIndex,
    int reservoirId) const = 0;

  virtual std::shared_ptr<MiSurfaceMeshCurvilinear> createSurfaceMesh(
    size_t snapshotIndex,
    int surfaceId) const = 0;

  virtual std::shared_ptr<MiDataSetIjk<double> > createFormationProperty(
    size_t snapshotIndex,
    int propertyId) const = 0;

  virtual std::shared_ptr<MiDataSetIj<double> > createSurfaceProperty(
    size_t snapshotIndex,
    int surfaceId,
    int propertyId) const = 0;

  virtual std::shared_ptr<MiDataSetIjk<double> > createReservoirProperty(
    size_t snapshotIndex,
    int reservoirId,
    int propertyId) const = 0;

  virtual std::shared_ptr<MiDataSetIjk<double> > createPersistentTrapIdProperty(
    size_t snapshotIndex,
    int reservoirId) const = 0;

  virtual std::shared_ptr<MiDataSetIjk<double> > createFlowDirectionProperty(size_t snapshotIndex) const = 0;

  virtual std::vector<Trap> getTraps(size_t snapshotIndex, int reservoirId) const = 0;

  virtual std::vector<SbVec2d> getFaultLine(int faultId) const = 0;

  static std::shared_ptr<Project> load(const std::string& path);
};

#endif

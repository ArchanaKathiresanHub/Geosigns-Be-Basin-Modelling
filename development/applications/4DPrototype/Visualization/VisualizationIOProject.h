//
// Copyright (C) 2012-2015 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef VISUALIZATIONIOPROJECT_H_INCLUDED
#define VISUALIZATIONIOPROJECT_H_INCLUDED

#include "Project.h"

#include <VisualizationAPI.h>

class VisualizationIOProject : public Project
{
  boost::shared_ptr<CauldronIO::Project> m_project;

  ProjectInfo m_projectInfo;

  CauldronIO::SnapShotList m_snapshots;

  int m_numCellsI;
  int m_numCellsJ;
  int m_numCellsIHiRes;
  int m_numCellsJHiRes;
  double m_deltaX;
  double m_deltaY;
  double m_deltaXHiRes;
  double m_deltaYHiRes;
  double m_minX;
  double m_minY;

  void init();

public:

  explicit VisualizationIOProject(const std::string& path);

  virtual ProjectInfo getProjectInfo() const;

  virtual unsigned int getMaxPersistentTrapId() const;

  virtual int numCellsI() const;

  virtual int numCellsJ() const;

  virtual int numCellsIHiRes() const;

  virtual int numCellsJHiRes() const;

  virtual double deltaX() const;

  virtual double deltaY() const;

  virtual double deltaXHiRes() const;

  virtual double deltaYHiRes() const;

  virtual double minX() const;

  virtual double minY() const;

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

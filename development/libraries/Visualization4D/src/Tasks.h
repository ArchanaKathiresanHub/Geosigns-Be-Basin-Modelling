//
// Copyright (C) 2012-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef TASKS_H_INCLUDED
#define TASKS_H_INCLUDED

#include "Scheduler.h"
#include "Project.h"
#include "SceneGraphManager.h"

#include <memory>
#include <vector>

class MiVolumeMeshCurvilinear;
class MiSurfaceMeshCurvilinear;
class MiCellFilterIjk;
class MiSkinExtractIjk;
template<class T> class MiDataSetIj;
template<class T> class MiDataSetIjk;

enum TaskType
{
  Task_LoadFormationMesh,
  Task_LoadReservoirMesh,
  Task_LoadSurfaceMesh,
  Task_LoadFormationProperty,
  Task_LoadFormation2DProperty,
  Task_LoadSurfaceProperty,
  Task_LoadReservoirProperty,
  Task_ExtractFormationSkin,
  Task_ExtractReservoirSkin
};

struct LoadFormationMeshTask : public Task
{
  std::shared_ptr<Project> project;
  size_t snapshotIndex = 0;

  std::shared_ptr<MiVolumeMeshCurvilinear> result;

  LoadFormationMeshTask();

  void run() override;
};

struct LoadReservoirMeshTask : public Task
{
  std::shared_ptr<Project> project;
  size_t snapshotIndex = 0;
  int reservoirId = 0;

  std::shared_ptr<MiVolumeMeshCurvilinear> result;

  LoadReservoirMeshTask();

  void run() override;
};

struct LoadSurfaceMeshTask : public Task
{
  std::shared_ptr<Project> project;
  size_t snapshotIndex;
  int surfaceId;

  std::shared_ptr<MiSurfaceMeshCurvilinear> result;

  LoadSurfaceMeshTask();

  void run() override;
};

struct LoadFormationPropertyTask : public Task
{
  std::shared_ptr<Project> project;
  size_t snapshotIndex = 0;
  int propertyId = 0;

  std::shared_ptr<MiDataSetIjk<double>> result;

  LoadFormationPropertyTask();

  void run() override;
};

struct LoadFormation2DPropertyTask : public Task
{
  std::shared_ptr<Project> project;
  size_t snapshotIndex;
  int formationId;
  int propertyId;

  std::shared_ptr<MiDataSetIj<double>> result;

  LoadFormation2DPropertyTask();

  void run() override;
};

struct LoadSurfacePropertyTask : public Task
{
  std::shared_ptr<Project> project;
  size_t snapshotIndex;
  int surfaceId;
  int propertyId;

  std::shared_ptr<MiDataSetIj<double>> result;

  LoadSurfacePropertyTask();

  void run() override;
};

struct LoadReservoirPropertyTask : public Task
{
  std::shared_ptr<Project> project;
  size_t snapshotIndex;
  int reservoirId;
  int propertyId;

  std::shared_ptr<MiDataSetIjk<double>> result;

  LoadReservoirPropertyTask();

  void run() override;
};

struct ExtractFormationSkinTask : public Task
{
  size_t snapshotIndex;
  size_t formationsTimestamp;

  std::vector<std::tuple<int, int>> ranges;
  std::shared_ptr<MiVolumeMeshCurvilinear> mesh;
  std::shared_ptr<MiDataSetIjk<double>> dataSet;
  std::shared_ptr<MiCellFilterIjk> cellFilter;

  std::vector<SnapshotInfo::Chunk> chunks;

  ExtractFormationSkinTask();

  void run() override;
};

struct ExtractReservoirSkinTask : public Task
{
  size_t snapshotIndex;
  int reservoirId;

  std::shared_ptr<MiVolumeMeshCurvilinear> mesh;

  std::shared_ptr<MiSkinExtractIjk> result;

  ExtractReservoirSkinTask();

  void run() override;
};

#endif

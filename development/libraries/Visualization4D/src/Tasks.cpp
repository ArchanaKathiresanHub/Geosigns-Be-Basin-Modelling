//
// Copyright (C) 2012-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "Tasks.h"
#include "MeshExtraction.h"

#include <MeshVizXLM/extractors/MiSkinExtractIjk.h>

//------------------------------------------------------------------------------
// LoadFormationMeshTask
//------------------------------------------------------------------------------
LoadFormationMeshTask::LoadFormationMeshTask()
{
  affinity = IOTASK;
  type = Task_LoadFormationMesh;
}

void LoadFormationMeshTask::run()
{
  try
  {
    result = project->createSnapshotMesh(snapshotIndex);
  }
  catch (std::runtime_error&)
  {
    error = true;
  }
}

//------------------------------------------------------------------------------
// LoadReservoirMeshTask
//------------------------------------------------------------------------------
LoadReservoirMeshTask::LoadReservoirMeshTask()
{
  affinity = IOTASK;
  type = Task_LoadReservoirMesh;
}

void LoadReservoirMeshTask::run()
{
  try
  {
    result = project->createReservoirMesh(snapshotIndex, reservoirId);
  }
  catch (std::runtime_error&)
  {
    error = true;
  }
}

//------------------------------------------------------------------------------
// LoadSurfaceMeshTask
//------------------------------------------------------------------------------
LoadSurfaceMeshTask::LoadSurfaceMeshTask()
{
  affinity = IOTASK;
  type = Task_LoadSurfaceMesh;
}

void LoadSurfaceMeshTask::run()
{
  try
  {
    result = project->createSurfaceMesh(snapshotIndex, surfaceId);
  }
  catch (std::runtime_error&)
  {
    error = true;
  }
}

//------------------------------------------------------------------------------
// LoadFormationPropertyTask
//------------------------------------------------------------------------------
LoadFormationPropertyTask::LoadFormationPropertyTask()
{
  affinity = IOTASK;
  type = Task_LoadFormationProperty;
}

void LoadFormationPropertyTask::run()
{
  try
  {
    result = project->createFormationProperty(snapshotIndex, propertyId);
  }
  catch (std::runtime_error&)
  {
    error = true;
  }
}

//------------------------------------------------------------------------------
// LoadFormation2DPropertyTask
//------------------------------------------------------------------------------
LoadFormation2DPropertyTask::LoadFormation2DPropertyTask()
{
  affinity = IOTASK;
  type = Task_LoadFormation2DProperty;
}

void LoadFormation2DPropertyTask::run()
{
  try
  {
    result = project->createFormation2DProperty(snapshotIndex, formationId, propertyId);
  }
  catch (std::runtime_error&)
  {
    error = true;
  }
}

//------------------------------------------------------------------------------
// LoadSurfacePropertyTask
//------------------------------------------------------------------------------
LoadSurfacePropertyTask::LoadSurfacePropertyTask()
{
  affinity = IOTASK;
  type = Task_LoadSurfaceProperty;
}

void LoadSurfacePropertyTask::run()
{
  try
  {
    result = project->createSurfaceProperty(snapshotIndex, surfaceId, propertyId);
  }
  catch (std::runtime_error&)
  {
    error = true;
  }
}

//------------------------------------------------------------------------------
// LoadReservoirPropertyTask
//------------------------------------------------------------------------------
LoadReservoirPropertyTask::LoadReservoirPropertyTask()
{
  affinity = IOTASK;
  type = Task_LoadReservoirProperty;
}

void LoadReservoirPropertyTask::run()
{
  try
  {
    result = project->createReservoirProperty(snapshotIndex, reservoirId, propertyId);
  }
  catch (std::runtime_error&)
  {
    error = true;
  }
}

//------------------------------------------------------------------------------
// ExtractFormationSkinTask
//------------------------------------------------------------------------------
ExtractFormationSkinTask::ExtractFormationSkinTask()
{
  affinity = CPUTASK;
  type = Task_ExtractFormationSkin;
}

void ExtractFormationSkinTask::run()
{
  for (auto range : ranges)
  {
    int minK = std::get<0>(range);
    int maxK = std::get<1>(range);

    auto chunk = createChunk(*mesh, cellFilter, minK, maxK);
    if(dataSet)
      updateChunkScalarSet(chunk, dataSet.get());

    chunks.push_back(chunk);
  }
}

//------------------------------------------------------------------------------
// ExtractFormationSkinTask
//------------------------------------------------------------------------------
ExtractReservoirSkinTask::ExtractReservoirSkinTask()
{
  affinity = CPUTASK;
  type = Task_ExtractReservoirSkin;
}

void ExtractReservoirSkinTask::run()
{
  result.reset(MiSkinExtractIjk::getNewInstance(*mesh));
  result->extractSkin();
}

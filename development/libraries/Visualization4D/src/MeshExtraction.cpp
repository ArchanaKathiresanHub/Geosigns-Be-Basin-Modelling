//
// Copyright (C) 2012-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "MeshExtraction.h"

#include <Inventor/nodes/SoSeparator.h>

#include <MeshVizXLM/mapping/nodes/MoMesh.h>
#include <MeshVizXLM/mapping/nodes/MoScalarSet.h>
#include <MeshVizXLM/mapping/nodes/MoMeshSurface.h>
#include <MeshVizXLM/mesh/MiVolumeMeshCurvilinear.h>
#include <MeshVizXLM/extractors/MiSkinExtractIjk.h>

SnapshotInfo::Chunk createChunk(const MiVolumeMeshCurvilinear& mesh, std::shared_ptr<MiCellFilterIjk> cellFilter, int kmin, int kmax)
{
  SnapshotInfo::Chunk chunk;
  chunk.minK = kmin;
  chunk.maxK = kmax;

  auto const& topology = mesh.getTopology();
  MbVec3ui rangeMin(0u, 0u, (size_t)kmin);
  MbVec3ui rangeMax(
    topology.getNumCellsI(),
    topology.getNumCellsJ(),
    (size_t)kmax);

  chunk.extractor.reset(MiSkinExtractIjk::getNewInstance(mesh));
  chunk.extractor->addCellRange(rangeMin, rangeMax);
  chunk.root = new SoSeparator;
  chunk.mesh = new MoMesh;
  chunk.mesh->setMesh(&chunk.extractor->extractSkin(cellFilter.get()));
  chunk.scalarSet = new MoScalarSet;
  chunk.skin = new MoMeshSurface;
  chunk.skin->colorScalarSetId = -1;
  chunk.root->addChild(chunk.mesh);
  chunk.root->addChild(chunk.scalarSet);
  chunk.root->addChild(chunk.skin);

  return chunk;
}

void updateChunkScalarSet(SnapshotInfo::Chunk& chunk, MiDataSetIjk<double>* dataSet)
{
  assert(chunk.extractor);

  if (dataSet)
  {
    auto chunkDataSet = &chunk.extractor->extractScalarSet(*dataSet);
    chunk.scalarSet->setScalarSet(chunkDataSet);
    chunk.skin->colorScalarSetId = 0;
  }
  else
  {
    chunk.scalarSet->setScalarSet(nullptr);
    chunk.skin->colorScalarSetId = -1;
  }
}

void updateReservoirScalarSet(SnapshotInfo::Reservoir& reservoir)
{
  assert(reservoir.extractor);

  if (reservoir.propertyData)
  {
    auto reservoirDataSet = &reservoir.extractor->extractScalarSet(*reservoir.propertyData);
    reservoir.scalarSet->setScalarSet(reservoirDataSet);
    reservoir.skin->colorScalarSetId = 0;
  }
  else
  {
    reservoir.scalarSet->setScalarSet(nullptr);
    reservoir.skin->colorScalarSetId = -1;
  }
}

void updateSurfaceScalarSet(SnapshotInfo::Surface& surface)
{
  if (surface.propertyData)
  {
    surface.scalarSet->setScalarSet(surface.propertyData.get());
    surface.surfaceMesh->colorScalarSetId = 0;
  }
  else
  {
    surface.scalarSet->setScalarSet(nullptr);
    surface.surfaceMesh->colorScalarSetId = -1;
  }
}

void setupSlice(SnapshotInfo::Slice& slice, const MiVolumeMeshCurvilinear& mesh)
{
  slice.position = -1;
  slice.extractor.reset(MiSkinExtractIjk::getNewInstance(mesh));
  slice.root = new SoSeparator;
  slice.mesh = new MoMesh;
  slice.scalarSet = new MoScalarSet;
  slice.skin = new MoMeshSurface;
  slice.skin->colorScalarSetId = -1;
  slice.root->addChild(slice.mesh);
  slice.root->addChild(slice.scalarSet);
  slice.root->addChild(slice.skin);
}

void updateSliceGeometry(SnapshotInfo::Slice& slice, int axis, int position)
{
  assert(axis == 0 || axis == 1);

  MbVec3ui rangeMin(0u, 0u, 0u);
  MbVec3ui rangeMax(UNDEFINED_ID, UNDEFINED_ID, UNDEFINED_ID);

  rangeMin[axis] = (size_t)position;
  rangeMax[axis] = (size_t)position + 1;

  slice.extractor->clearCellRanges();
  slice.extractor->addCellRange(rangeMin, rangeMax);
  slice.mesh->setMesh(&slice.extractor->extractSkin());
}

void updateSliceScalarSet(SnapshotInfo::Slice& slice, MiDataSetIjk<double>* dataSet)
{
  if (!slice.root)
    return;

  if (dataSet)
  {
    slice.scalarSet->setScalarSet(&slice.extractor->extractScalarSet(*dataSet));
    slice.skin->colorScalarSetId = 0;
  }
  else
  {
    slice.scalarSet->setScalarSet(nullptr);
    slice.skin->colorScalarSetId = -1;
  }
}

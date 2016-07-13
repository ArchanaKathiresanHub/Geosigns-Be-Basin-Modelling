//
// Copyright (C) 2012-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef MESHEXTRACTION_H_INCLUDED
#define MESHEXTRACTION_H_INCLUDED

#include "SceneGraphManager.h"

SnapshotInfo::Chunk createChunk(const MiVolumeMeshCurvilinear& mesh, std::shared_ptr<MiCellFilterIjk> cellFilter, int kmin, int kmax);

void updateChunkScalarSet(SnapshotInfo::Chunk& chunk, MiDataSetIjk<double>* dataSet);

void updateReservoirScalarSet(SnapshotInfo::Reservoir& reservoir);

void updateSurfaceScalarSet(SnapshotInfo::Surface& surface);

void setupSlice(SnapshotInfo::Slice& slice, const MiVolumeMeshCurvilinear& mesh);

void updateSliceGeometry(SnapshotInfo::Slice& slice, int axis, int position);

void updateSliceScalarSet(SnapshotInfo::Slice& slice, MiDataSetIjk<double>* dataSet);

#endif

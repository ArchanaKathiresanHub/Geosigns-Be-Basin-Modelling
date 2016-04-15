//
// Copyright (C) 2012-2015 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef GEOMETRYUTIL_H_INCLUDED
#define GEOMETRYUTIL_H_INCLUDED

class MiGeometryIjk;
class MiVolumeMeshCurvilinear;
class SoVolumeBufferedShape;
class SbBox2d;

#include "Project.h"

#include <vector>

#include <Inventor/SbVec.h>

double getZ(const MiGeometryIjk& geometry, int maxI, int maxJ, double i, double j, int k, bool* ok = nullptr);

bool clip(SbVec2d& p0, SbVec2d& p1, const SbBox2d& box);

std::vector<SbVec2d> computeGridLineIntersections(const SbVec2d& p0, const SbVec2d& p1, double stepX, double stepY);

SoVolumeBufferedShape* createCrossSection(const SbVec2d& p0, const SbVec2d& p1, const MiVolumeMeshCurvilinear& mesh, const Project::Dimensions& dim);

#endif

//
// Copyright (C) 2012-2015 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "GeometryUtil.h"

#include <Inventor/SbBox.h>
#include <Inventor/devices/SoCpuBufferObject.h>

#include <MeshVizXLM/mesh/geometry/MiGeometryIjk.h>
#include <MeshVizXLM/mesh/topology/MiTopologyIjk.h>
#include <MeshVizXLM/mesh/MiVolumeMeshCurvilinear.h>
#include <VolumeViz/nodes/SoVolumeBufferedShape.h>
#include <Inventor/nodes/SoBufferedShape.h>

#include <algorithm>

namespace
{
  /**
   * Linear interpolation
   */
  inline double lerp(double x0, double x1, double a)
  {
    return (1.0 - a) * x0 + a * x1;
  }
}

/**
 * Get a depth value from geometry, using floating point x and y coordinates
 * and bilinear interpolation
 */
double getZ(const MiGeometryIjk& geometry, int maxI, int maxJ, double i, double j, int k, bool* ok)
{
  double i_int, j_int;
  double di = modf(i, &i_int);
  double dj = modf(j, &j_int);

  int i0 = (int)i_int;
  int j0 = (int)j_int;

  // Take care not to read outside the bounds of the 3D array
  int i1 = std::min(i0 + 1, maxI);
  int j1 = std::min(j0 + 1, maxJ);

  double z00 = geometry.getCoord(i0, j0, k)[2];
  double z01 = geometry.getCoord(i0, j1, k)[2];
  double z10 = geometry.getCoord(i1, j0, k)[2];
  double z11 = geometry.getCoord(i1, j1, k)[2];

  if (ok)
  {
    const double undefined = -99999.0;
    *ok =
      z00 != undefined &&
      z01 != undefined &&
      z10 != undefined &&
      z11 != undefined;
  }

  return lerp(
    lerp(z00, z01, dj),
    lerp(z10, z11, dj),
    di);
}

/**
 * Clip a 2D line (defined by its 2 endpoints) to a 2D box
 */
bool clip(SbVec2d& p0, SbVec2d& p1, const SbBox2d& box)
{
  bool c0, c1; // true = in, false = out

  double minX = box.getMin()[0];
  double minY = box.getMin()[1];
  double maxX = box.getMax()[0];
  double maxY = box.getMax()[1];

  double dx = p1[0] - p0[0];
  double dy = p1[1] - p0[1];

  // check minX
  c0 = p0[0] >= minX;
  c1 = p1[0] >= minX;

  if (!c0 && !c1)
    return false;

  if (c0 != c1)
  {
    double y = p0[1] + (minX - p0[0]) * dy / dx;
    SbVec2d p(minX, y);

    if (c0)
      p1 = p;
    else
      p0 = p;
  }

  // check maxX
  c0 = p0[0] <= maxX;
  c1 = p1[0] <= maxX;

  if (!c0 && !c1)
    return false;

  if (c0 != c1)
  {
    double y = p0[1] + (maxX - p0[0]) * dy / dx;
    SbVec2d p(maxX, y);

    if (c0)
      p1 = p;
    else
      p0 = p;
  }

  // check minY
  c0 = p0[1] >= minY;
  c1 = p1[1] >= minY;

  if (!c0 && !c1)
    return false;

  if (c0 != c1)
  {
    double x = p0[0] + (minY - p0[1]) * dx / dy;
    SbVec2d p(x, minY);

    if (c0)
      p1 = p;
    else
      p0 = p;
  }

  // check maxY
  c0 = p0[1] <= maxY;
  c1 = p1[1] <= maxY;

  if (!c0 && !c1)
    return false;

  if (c0 != c1)
  {
    double x = p0[0] + (maxY - p0[1]) * dx / dy;
    SbVec2d p(x, maxY);

    if (c0)
      p1 = p;
    else
      p0 = p;
  }

  return true;
}

/**
 * Given a 2D line defined by its endpoints, compute the intersections with the grid defined by the
 * 'step' parameter: all horizontal lines are at y = j * step, all vertical lines are at x = i * step,
 * where i and j are integers
 */
std::vector<SbVec2d> computeGridLineIntersections(const SbVec2d& p0, const SbVec2d& p1, double stepX, double stepY)
{
  std::vector<SbVec2d> points;

  double dx = p1[0] - p0[0];
  double dy = p1[1] - p0[1];

  double lastX = p0[0];
  double lastY = p0[1];

  // x-coord of intersection with next vertical line
  double nextX = (dx >= 0.0)
    ? stepX * floor(lastX / stepX + 1.0) // moving in positive x direction
    : stepX * ceil(lastX / stepX - 1.0); // moving in negative x direction

  // y-coord of intersection with next horizontal line
  double nextY = (dy >= 0.0)
    ? stepY * floor(lastY / stepY + 1.0) // moving in positive y direction
    : stepY * ceil(lastY / stepY - 1.0); // moving in negative y direction

  bool valid = true;
  while (valid)
  {
    points.emplace_back(lastX, lastY);

    double tx = (nextX - lastX) / dx;
    double ty = (nextY - lastY) / dy;

    if (tx < ty) // nextX is closer
    {
      lastX = nextX;
      lastY = p0[1] + dy * ((nextX - p0[0]) / dx);

      nextX += (dx > 0.f) ? stepX : -stepX;
    }
    else // nextY is closer
    {
      lastX = p0[0] + dx * ((nextY - p0[1]) / dy);
      lastY = nextY;

      nextY += (dy > 0.f) ? stepY : -stepY;
    }

    valid =
      fabs(lastX - p0[0]) <= fabs(dx) &&
      fabs(lastY - p0[1]) <= fabs(dy);
  }

  if(points.back() != p1)
    points.push_back(p1); // don't forget final point

  return points;
}

std::vector<uint32_t> computeSurfaceIndices(const MiTopologyIjk& topology)
{
  uint32_t ni = (uint32_t)(topology.getNumCellsI() + 1);
  uint32_t nj = (uint32_t)(topology.getNumCellsJ() + 1);

  std::vector<uint32_t> indices;
  indices.reserve(4 * (ni - 1) * (nj - 1));

  for (uint32_t j = 0; j < nj - 1; ++j)
  {
    for (uint32_t i = 0; i < ni - 1; ++i)
    {
      if (!topology.isDead(i, j, 0u))
      {
        uint32_t base = i + j * ni;
        uint32_t quadIndices[] = { base, base + 1, base + 1 + ni, base + ni };

        indices.insert(indices.end(), quadIndices, quadIndices + 4);
      }
    }
  }

  return indices;
}

bool computeSurfaceCoordinates(const MiVolumeMeshCurvilinear& mesh, float invK, SoCpuBufferObject* vbo)
{
  auto const& geometry = mesh.getGeometry();
  auto const& topology = mesh.getTopology();

  float k = (float)topology.getNumCellsK() - invK;
  if (k < 0.f || k > topology.getNumCellsK())
    return false;

  uint32_t ni = (uint32_t)(topology.getNumCellsI() + 1);
  uint32_t nj = (uint32_t)(topology.getNumCellsJ() + 1);
  
  size_t k0 = (size_t)floor(k);
  size_t k1 = (size_t)ceil(k);
  float intpart = 0.f;
  float alpha = modf(k, &intpart);

  vbo->setSize(ni * nj * 3 * sizeof(float));
  float* p = (float*)vbo->map(SoBufferObject::SET);

  for (uint32_t j = 0; j < nj; ++j)
  {
    for (uint32_t i = 0; i < ni; ++i)
    {
      MbVec3d p0 = geometry.getCoord(i, j, k0);
      MbVec3d p1 = geometry.getCoord(i, j, k1);

      *p++ = (float)p0[0];
      *p++ = (float)p0[1];
      *p++ = (float)lerp(p0[2], p1[2], alpha);
    }
  }

  vbo->unmap();

  return true;
}

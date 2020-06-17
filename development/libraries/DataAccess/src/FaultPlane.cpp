//
// Copyright (C) 2020 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//


#include "FaultPlane.h"
#include "FaultCutCreator.h"
#include "GridMap.h"
#include "Triangle.h"

#include <algorithm>
#include <cmath>

#include <iostream>
#include <fstream>

using namespace DataAccess;
using namespace Interface;



FaultPlane::FaultPlane (const std::vector<PointSequence>& faultSticks) :
  m_faultSticks( faultSticks )
{
}

FaultPlane::~FaultPlane()
{
}

bool FaultPlane::determineIntersectionPoint(const GridMap* surfaceMap, double minZ, double maxZ, const std::pair<Point, Point>& faultSegment, Point& intersection) const
{
  const double origX = surfaceMap->minI();
  const double origY = surfaceMap->minJ();
  const double deltaX = surfaceMap->deltaI();
  const double deltaY = surfaceMap->deltaJ();

  const double firstIx = static_cast<double>(surfaceMap->firstI());

  unsigned int xMin = std::min(surfaceMap->lastI(),
                               static_cast<unsigned int>(std::max(firstIx,
                                                                  std::min(std::floor((faultSegment.first(X_COORD)-origX)/deltaX),
                                                                           std::floor((faultSegment.second(X_COORD)-origX)/deltaX)))));
  unsigned int xMax = std::min(surfaceMap->lastI(),
                               static_cast<unsigned int>(std::max(firstIx,
                                                                  std::max(std::ceil((faultSegment.first(X_COORD)-origX)/deltaX),
                                                                           std::ceil((faultSegment.second(X_COORD)-origX)/deltaX)))));


  const double firstIy = static_cast<double>(surfaceMap->firstJ());

  unsigned int yMin = std::min(surfaceMap->lastJ(),
                               static_cast<unsigned int>(std::max(firstIy,
                                                                  std::min(std::floor((faultSegment.first(Y_COORD)-origY)/deltaY),
                                                                           std::floor((faultSegment.second(Y_COORD)-origY)/deltaY)))));
  unsigned int yMax = std::min(surfaceMap->lastJ(),
                               static_cast<unsigned int>(std::max(firstIy,
                                                                  std::max(std::ceil((faultSegment.first(Y_COORD)-origY)/deltaY),
                                                                           std::ceil((faultSegment.second(Y_COORD)-origY)/deltaY)))));
  // Fix if the min and max indices are the same
  if (xMin == xMax)
  {
    if (xMin == surfaceMap->lastI())
    {
      xMin -= 1;
    }
    else
    {
      xMax += 1;
    }
  }

  if (yMin == yMax)
  {
    if (yMin == surfaceMap->lastJ())
    {
      yMin -= 1;
    }
    else
    {
      yMax += 1;
    }
  }

  for (unsigned int xIndex = xMin; xIndex < xMax; xIndex++)
  {
    const double x1 = surfaceMap->minI() + surfaceMap->deltaI() * ( xIndex - surfaceMap->firstI() );
    const double x2 = x1 + surfaceMap->deltaI();

    for (unsigned int yIndex = yMin; yIndex < yMax; yIndex++)
    {
      if (!surfaceMap->valueIsDefined(xIndex, yIndex))
      {
        continue;
      }
      const double y1 = surfaceMap->minJ() + surfaceMap->deltaJ() * ( yIndex - surfaceMap->firstJ() );
      const double y2 = y1 + surfaceMap->deltaJ();
      std::vector<double> xSegment = {x1, x2, x2, x1};
      std::vector<double> ySegment = {y1, y1, y2, y2};
      std::vector<double> zTopSegment = {surfaceMap->getValue(xIndex, yIndex), surfaceMap->getValue(xIndex+1, yIndex),
                                         surfaceMap->getValue(xIndex+1, yIndex+1), surfaceMap->getValue(xIndex, yIndex+1)};
      double maxTopZ = 0.0;
      double minTopZ = 1.e99;
      for (double element : zTopSegment)
      {
        if (element < minTopZ)
        {
          minTopZ = element;
        }
        if (element > maxTopZ)
        {
          maxTopZ = element;
        }
      }

      if( !( maxZ < minTopZ || minZ > maxTopZ) )
      {
        if ( findIntersection(xSegment,ySegment,zTopSegment, faultSegment, intersection) )
        {
          return true;
        }
      }
    }
  }
  return false;
}

std::vector<Triangle> FaultPlane::makeTriangles(const std::vector<double>& xSegment,
                                                const std::vector<double>& ySegment,
                                                const std::vector<double>& zSegment) const
{
  std::vector<Triangle> triangles;

  Point p0(xSegment[0], ySegment[0], zSegment[0]);
  Point p1(xSegment[1], ySegment[1], zSegment[1]);
  Point p2(xSegment[2], ySegment[2], zSegment[2]);
  Point p3(xSegment[3], ySegment[3], zSegment[3]);

  Point pMid = (p0 + p1 + p2 + p3)/4;

  triangles.push_back(Triangle({p0, p1, pMid}));
  triangles.push_back(Triangle({p1, p2, pMid}));
  triangles.push_back(Triangle({p2, p3, pMid}));
  triangles.push_back(Triangle({p3, p0, pMid}));

  return triangles;
}

bool FaultPlane::findIntersection(const std::vector<double>& xSegment, const std::vector<double> &ySegment,
                                  const std::vector<double>& zTopSegment, const std::pair<Point, Point> &segment, Point& intersection) const
{
  const std::vector<Triangle> triangles = makeTriangles(xSegment, ySegment, zTopSegment);

  for (const Triangle& triangle : triangles)
  {
    Point intersectionCandidate;
    if (triangle.findIntersection(segment, intersectionCandidate))
    {
      intersection = intersectionCandidate;
      return true;
    }
  }

  return false;
}

void writeSegmentsToCSV(std::vector<std::pair<Point, Point>> segments)
{
  std::ofstream myFile;
  myFile.open("segments.csv");

  for (std::pair<Point, Point> segment : segments)
  {
    myFile << segment.first(X_COORD) << ","
           << segment.first(Y_COORD) << ","
           << segment.first(Z_COORD) << ",,"
           << segment.second(X_COORD) << ","
           << segment.second(Y_COORD) << ","
           << segment.second(Z_COORD) << ",\n";
  }

  myFile.close();
}

std::vector<std::pair<Point, Point>> FaultPlane::createSegments() const
{
  std::vector<std::pair<Point, Point>> segments;

  const double maxAngle = 90.0; //Degrees

  int i = 0;
  for (const PointSequence& faultStick : m_faultSticks)
  {
    for (unsigned int pointIndex = 0; pointIndex < faultStick.size() - 1; pointIndex++)
    {
      const Point& p1 = faultStick[ pointIndex ];
      const Point& p2 = faultStick[ pointIndex + 1 ];
      segments.push_back({p1, p2});

      const double s = separationDistance( p1, p2 );
      const double dz = std::fabs( p1(Z_COORD) - p2(Z_COORD));
      if ( s < 1.e-10 || std::acos( dz/s ) * 180.0/M_PI > maxAngle )
      {
        continue;
      }

      const double lowerLimit = s*1.1;
      const double upperLimit = s*2.0;

      Point p3;
      double closestDistance = upperLimit;
      int k3 = -1;
      int j = -1;
      for ( const PointSequence& faultStick1 : m_faultSticks )
      {
        ++j;
        if ( i == j ) continue;

        for ( const Point& point : faultStick1 )
        {
          const double d = separationDistance(p1, point) + separationDistance(p2, point);
          if ( d >= closestDistance || d < lowerLimit || d > upperLimit ) continue;

          p3 = point;
          closestDistance = d;
          k3 = j;
        }
      }
      if (k3 == -1) continue;

      segments.push_back({p1, p3});
      segments.push_back({p2, p3});
      addSegments(segments, p1, p2, p3, 2);

      Point mPoint = midPoint(p1, p2);

      Point p4;
      closestDistance = upperLimit;
      int k4 = -1;
      j = -1;
      for ( const PointSequence& faultStick1 : m_faultSticks )
      {
        ++j;
        if ( i == j || j == k3 ) continue;

        for ( const Point& point : faultStick1 )
        {
          if ((point(X_COORD)-mPoint(X_COORD))*(p3(X_COORD)-mPoint(X_COORD)) +
              (point(Y_COORD)-mPoint(Y_COORD))*(p3(Y_COORD)-mPoint(Y_COORD)) +
              (point(Z_COORD)-mPoint(Z_COORD))*(p3(Z_COORD)-mPoint(Z_COORD)) > 0.0 ) continue;

          const double d = separationDistance(p1, point) + separationDistance(p2, point);
          if ( d >= closestDistance || d < lowerLimit || d > upperLimit ) continue;

          p4 = point;
          closestDistance = d;
          k4 = j;
        }
      }

      if ( k4 > -1)
      {
        segments.push_back({p1, p4});
        segments.push_back({p2, p4});
        addSegments(segments, p1, p2, p4, 2);
      }

    }
    ++i;
  }

#ifndef NDEBUG
  writeSegmentsToCSV(segments);
#endif

  return segments;
}

void FaultPlane::addSegments(std::vector<std::pair<Point, Point>>& segments, const Point& p1, const Point& p2, const Point& p3, int level) const
{
  if (level == 0) return;

  Point p4 = p1 + p2 + p3;
  p4 = p4 / 3.0;

  segments.push_back({p1, p4});
  segments.push_back({p2, p4});
  segments.push_back({p3, p4});

  addSegments(segments, p1, p2, p4, level-1);
  addSegments(segments, p1, p3, p4, level-1);
  addSegments(segments, p2, p3, p4, level-1);
}

bool FaultPlane::intersect(const GridMap* surfaceMap, std::vector<PointSequence>& faultCuts)
{
  surfaceMap->retrieveData();

  double minDepth;
  double maxDepth;
  surfaceMap->getMinMaxValue(minDepth, maxDepth);

  PointSequence intersections;
  for (const std::pair<Point, Point>& segment : createSegments() )
  {
    const double maxZ = std::max(segment.first(Interface::Z_COORD), segment.second(Interface::Z_COORD));
    if ( maxZ < minDepth ) continue;

    const double minZ = std::min(segment.first(Interface::Z_COORD), segment.second(Interface::Z_COORD));
    if ( minZ > maxDepth ) continue;

    Point intersection;
    if (determineIntersectionPoint(surfaceMap, minZ, maxZ, segment, intersection))
    {
      intersections.push_back(intersection);
    }
  }

  if (!intersections.empty())
  {
    const double splitDistance = (surfaceMap->deltaI() + surfaceMap->deltaJ())*10.0;
    FaultCutCreator faultCutCreator(intersections);
    faultCuts = faultCutCreator.createFaultCuts(splitDistance);
  }
  surfaceMap->release();

  return !faultCuts.empty();
}

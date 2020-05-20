//
// Copyright (C) 2020 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//


#include "FaultPlane.h"
#include "GridMap.h"
#include "Triangle.h"

#include <algorithm>
#include <cmath>

using namespace DataAccess;
using namespace Interface;

FaultPlane::FaultPlane (const std::vector<PointSequence>& faultSticks) :
  m_faultSticks( faultSticks ),
  m_segments()
{
  createSegments();
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

unsigned int FaultPlane::determineOptimumPlacement(const Point& candidate, const PointSequence& orderedFaultCut) const
{
  // Distance when new point is placed at the start
  unsigned int optimalPlacement = 0;
  double optimalDistance = separationDistance(candidate, orderedFaultCut.front());

  // Distance when new point placed at the end
  double tryDistance = separationDistance(candidate, orderedFaultCut.back());
  if (tryDistance < optimalDistance)
  {
    optimalDistance = tryDistance;
    optimalPlacement = static_cast<unsigned int>(orderedFaultCut.size() + 1);
  }

  // Try all other positions and find the optimum
  for (unsigned int i = 1; i < orderedFaultCut.size(); i++)
  {
    tryDistance = separationDistance(orderedFaultCut[i-1], candidate)
        + separationDistance(candidate, orderedFaultCut[i])
        - separationDistance(orderedFaultCut[i], orderedFaultCut[i-1]);

    if (tryDistance < optimalDistance)
    {
      optimalDistance = tryDistance;
      optimalPlacement = i;
    }
  }

  return optimalPlacement;
}

void FaultPlane::orderIntersections(PointSequence& faultCut) const
{
  PointSequence orderedFaultCut;

  for (const Point& candidate : faultCut)
  {
    if (orderedFaultCut.size() < 2)
    {
      orderedFaultCut.push_back(candidate);
      continue;
    }

    const unsigned int optimalPlacement = determineOptimumPlacement(candidate, orderedFaultCut);

    if (optimalPlacement > orderedFaultCut.size())
    {
      orderedFaultCut.push_back(candidate);
    }
    else
    {
      orderedFaultCut.emplace(orderedFaultCut.begin() + optimalPlacement, candidate);
    }
  }

  faultCut = orderedFaultCut;
}

std::vector<PointSequence> FaultPlane::createFaultCuts(const PointSequence& intersections, const double splitDistance) const
{
  std::vector<PointSequence> faultCuts;

  PointSequence faultCutSegment;
  for (unsigned int i = 0; i < intersections.size() - 1; i++)
  {
    faultCutSegment.push_back(intersections[i]);

    if (separationDistance(intersections[i], intersections[i+1]) >= splitDistance)
    {
      faultCuts.push_back(faultCutSegment);
      faultCutSegment.clear();
    }
  }

  // Add the last element and last segment to the faultcuts
  faultCutSegment.push_back(intersections.back());
  faultCuts.push_back(faultCutSegment);

  return faultCuts;
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

void FaultPlane::createSegments()
{
  for (const PointSequence& faultStick : m_faultSticks)
  {
    for (unsigned int pointIndex = 0; pointIndex < faultStick.size() - 1; pointIndex++)
    {
      const Point& p1 = faultStick[ pointIndex ];
      const Point& p2 = faultStick[ pointIndex + 1 ];

      m_segments.push_back({p1, p2});

      if (m_faultSticks.size() > 1)
      {
        double closestDistance = 1.0e99;
        Point closestPoint(0,0,0);
        for ( const PointSequence& faultStick1 : m_faultSticks )
        {
          if ( faultStick1 == faultStick ) continue;

          for ( const Point& p3 : faultStick1 )
          {
            double d =  separationDistance(p1, p3);  if ( d > closestDistance) continue;
            d += separationDistance(p2, p3); if ( d > closestDistance) continue;

            closestDistance = d;
            closestPoint = p3;
          }
        }
//        m_segments.push_back({p1, closestPoint});
//        m_segments.push_back({p2, closestPoint});
      }
    }
  }
}

bool FaultPlane::intersect(const GridMap* surfaceMap, const double splitDistance, std::vector<PointSequence>& faultCuts)
{
  surfaceMap->retrieveData();

  double minDepth;
  double maxDepth;
  surfaceMap->getMinMaxValue(minDepth, maxDepth);
  PointSequence intersections;
  for (const std::pair<Point, Point>& segment : m_segments )
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
  surfaceMap->release();


  if (!intersections.empty())
  {
    orderIntersections(intersections);
    faultCuts = createFaultCuts(intersections, splitDistance);
  }

  return !faultCuts.empty();
}

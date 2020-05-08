//
// Copyright (C) 2020 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include <vector>

#include "auxiliaryfaulttypes.h"
#include "Interface.h"
#include "Triangle.h"

namespace DataAccess
{
  namespace Interface
  {
    class GridMap;

    class FaultPlane
    {
    public:
      explicit FaultPlane(const std::vector<PointSequence>& faultSticks);
      ~FaultPlane();
      bool intersect(const GridMap* surfaceMap, const double splitDistance, std::vector<PointSequence>& faultCuts);

    private:
      bool determineIntersectionPoint(const GridMap* surfaceMap, double minZ, double maxZ, const std::pair<Point, Point>& faultSegment, Point& intersection) const;
      bool findIntersection(const std::vector<double>& xSegment, const std::vector<double>& ySegment,
                            const std::vector<double>& zTopSegment, const std::pair<Point, Point>& segment, Point& intersection) const;

      std::vector<Triangle> makeTriangles(const std::vector<double>& xSegment, const std::vector<double>& ySegment, const std::vector<double>& zSegment) const;

      void orderIntersections(PointSequence& faultCut) const;
      unsigned int determineOptimumPlacement(const Point& candidate, const PointSequence& orderedFaultCut) const;

      std::vector<PointSequence> createFaultCuts(const PointSequence& intersections, const double splitDistance) const;

      void createSegments();

      std::vector<PointSequence> m_faultSticks;
      std::vector<std::pair<Point, Point>> m_segments;
    };
  }
}


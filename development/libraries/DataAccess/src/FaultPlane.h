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

      bool intersect(const GridMap* surfaceMap, std::vector<PointSequence>& faultCuts) const;

    private:
      bool determineIntersectionPoint(const GridMap* surfaceMap, double minZ, double maxZ, const std::pair<Point, Point>& faultSegment, Point& intersection) const;
      bool findIntersection(const std::vector<double>& xSegment, const std::vector<double>& ySegment,
                            const std::vector<double>& zTopSegment, const std::pair<Point, Point>& segment, Point& intersection) const;

      std::vector<Triangle> makeTriangles(const std::vector<double>& xSegment, const std::vector<double>& ySegment, const std::vector<double>& zSegment) const;

      std::vector<std::pair<Point, Point>> createSegments() const;
      void addSegments(std::vector<std::pair<Point, Point>>& segments, const Point& p1, const Point& p2, const Point& p3, int level) const;

      std::vector<PointSequence> m_faultSticks;
    };
  }
}


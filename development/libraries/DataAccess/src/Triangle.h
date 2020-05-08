//
// Copyright (C) 2020 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once

#include "auxiliaryfaulttypes.h"

namespace DataAccess
{
  namespace Interface
  {
    class Triangle
    {
    public:
      explicit Triangle(const PointSequence& triangle);
      bool findIntersection(const std::pair<Point, Point>& segment, Point& intersection) const;

    private:
      bool sameSide(Point p1, Point p2, Point a, Point b) const;
      bool pointInTriangle(Point intersectionCandidate) const;

      PointSequence m_triangle;
    };
  }
}


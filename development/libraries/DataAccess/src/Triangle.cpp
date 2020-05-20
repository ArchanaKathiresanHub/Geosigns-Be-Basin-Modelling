//
// Copyright (C) 2020 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//


#include "Triangle.h"

#include <assert.h>
#include <iostream>
#include <map>
#include <sstream>
#include <cmath>

using namespace DataAccess;
using namespace Interface;

class Vector
{
private:
  Point m_point;

  double x() const { return m_point(X_COORD); }
  double y() const { return m_point(Y_COORD); }
  double z() const { return m_point(Z_COORD); }

public:
  explicit Vector(const Point& point) :
    m_point(point)
  {
  }

  Point point() const
  {
    return m_point;
  }

  Vector cross(const Vector& vec) const
  {
    const double xCoord = y()*vec.z()- z()*vec.y();
    const double yCoord = z()*vec.x()- x()*vec.z();
    const double zCoord = x()*vec.y()- y()*vec.x();

    return Vector(Point(xCoord, yCoord, zCoord));
  }

  void normalize()
  {
    const double norm = std::sqrt(x()*x() + y()*y() + z()*z());
    m_point = m_point/norm;
  }

  double dot(const Vector& vec) const
  {
    return x()*vec.x() + y()*vec.y() + z()*vec.z();
  }

  // Get normal of triangle formed by 3 points
  static Vector getNormal(const Point& p0, const Point& p1, const Point& p2)
  {
    return Vector(p1 - p0).cross(Vector(p2 - p0));
  }
};



bool Triangle::sameSide(Point p1, Point p2, Point a, Point b) const
{
  return ((Vector(b-a).cross(Vector(p1-a))).dot(Vector(b-a).cross(Vector(p2-a)))) >= 0.0;
}

bool Triangle::pointInTriangle(Point intersectionCandidate) const
{
  return sameSide(intersectionCandidate, m_triangle[0], m_triangle[1], m_triangle[2])
      && sameSide(intersectionCandidate, m_triangle[1], m_triangle[0], m_triangle[2])
      && sameSide(intersectionCandidate, m_triangle[2], m_triangle[0], m_triangle[1]);
}

Triangle::Triangle(const PointSequence &triangle):
  m_triangle(triangle)
{
  assert(m_triangle.size() == 3);
}

bool Triangle::findIntersection(const std::pair<Point, Point> &segment, Point &intersection) const
{
  Vector normal = Vector::getNormal(m_triangle[0], m_triangle[1], m_triangle[2]);
  normal.normalize();

  const Vector segmentDirection(segment.second - segment.first);

  const double normalDotSegmentDirection = segmentDirection.dot(normal);
  if (std::fabs(normalDotSegmentDirection) < 0.00001 /*epsilon*/)
  {
    return false;
  }

  const double si = -normal.dot(Vector((segment.first - m_triangle[0]) / normalDotSegmentDirection));

  const Point intersectionCandidate = segment.first + si * segmentDirection.point();

  if (si > 0 && si <= 1 && pointInTriangle(intersectionCandidate))
  {
    intersection = intersectionCandidate;
    intersection[Z_COORD] = 0.0;
    return true;
  }

  return false;
}

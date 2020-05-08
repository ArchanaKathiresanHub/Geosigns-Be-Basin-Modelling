//// Copyright (C) 2012-2020 Shell International Exploration & Production.
//// All rights reserved.

//// Confidential and proprietary source code of Shell.
//// Do not distribute without written permission from Shell.

#include "Triangle.h"

#include <math.h>

#include <memory>
#include <gtest/gtest.h>

using namespace DataAccess;
using namespace Interface;

TEST(TriangleTest, triangleTestNoneIntersection)
{  
  const Triangle test_test({Point{0.0, 0.0, 0.0},
                            Point{1.0, 0.0, 0.0},
                            Point{0.0, 1.0, 0.0}});

  const Point pointSegment_one (0.0 , 0.0, 1.0);
  const Point pointSegment_two (0.0 , 1.0, 1.0);

  const std::pair<Point, Point> segment(pointSegment_one, pointSegment_two);

  Point intersectionCandidate;

  EXPECT_FALSE(test_test.findIntersection(segment, intersectionCandidate));
}

TEST(TriangleTest, triangleTestWithIntersection)
{
  const Triangle triangle({Point{0.0, 0.0, 0.0},
                            Point{1.0, 0.0, 0.0},
                            Point{0.0, 1.0, 0.0}});

  const Point pointSegment_one (0.5 , 0.5, 1.0);
  const Point pointSegment_two (0.5 , 0.5, -1.0);

  const std::pair<Point, Point> segment(pointSegment_one, pointSegment_two);

  const Point pointIntersectionExpected(0.5 , 0.5, 0.0);
  Point pointIntersection;

  EXPECT_TRUE(triangle.findIntersection(segment, pointIntersection));
  EXPECT_EQ(pointIntersection,pointIntersectionExpected);
}

TEST(TriangleTest, triangleTestSomeIntersections)
{
  const Triangle triangle({Point{0.0, 0.0, 0.0},
                            Point{1.0, 0.0, 0.0},
                            Point{0.0, 1.0, 0.0}});

  const Point pointSegment_one (0.5 , 0.5, 1.0);
  const Point pointSegment_two (0.5 , 0.5, -1.0);

  const std::pair<Point, Point> segment(pointSegment_one, pointSegment_two);

  const Point pointIntersectionExpected(0.5 , 0.5, 0.0);
  Point pointIntersection;

  EXPECT_TRUE(triangle.findIntersection(segment, pointIntersection));
  EXPECT_EQ(pointIntersection,pointIntersectionExpected);
}

TEST(TriangleTest, triangleTestDinamicIntersections)
{
  const Triangle triangle({Point{0.0, 0.0, 0.0},
                            Point{1.0, 0.0, 0.0},
                            Point{0.0, 1.0, 0.0}});

  const Point pointSegment_one (0.0 , 0.0, 1.0);

  double b_triangle = std::sqrt( 0.5 * 0.5 + 0.5 * 0.5 );

  int i;
  for (i = 0; i <= 105 ; ++i)
  {
    const double s = i / 100.0;

    if( std::sqrt(s*s + s*s) / b_triangle > 2) //No more intersection expected
    {
      break;
    }

   Point pointSegment_two (s , s, -1.0);
   std::pair<Point, Point> segment(pointSegment_one, pointSegment_two);

   Point pointIntersectionExpected(s/2 , s/2, 0.0);
   Point pointIntersection;

   EXPECT_TRUE(triangle.findIntersection(segment, pointIntersection));
   EXPECT_EQ(pointIntersection,pointIntersectionExpected);
  }

  Point pointSegment_two (i/100.0 , i/100.0, -1.0);
  std::pair<Point, Point> segment(pointSegment_one, pointSegment_two);

  Point pointIntersection;
  EXPECT_FALSE(triangle.findIntersection(segment, pointIntersection));
}

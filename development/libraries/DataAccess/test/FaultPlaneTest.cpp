////// Copyright (C) 2012-2020 Shell International Exploration & Production.
////// All rights reserved.

////// Confidential and proprietary source code of Shell.
////// Do not distribute without written permission from Shell.

#include "FaultPlane.h"

#include "SerialGrid.h"
#include "SerialGridMap.h"

#include <memory>
#include <gtest/gtest.h>

#include <cmath>

using namespace DataAccess;
using namespace Interface;

class FaultPlaneTest : public ::testing::Test
{
public:
  FaultPlaneTest() :
    m_firstI( 0 ), m_firstJ( 0 ), m_lastI( 4 ), m_lastJ(4),
    m_minI(0.0), m_minJ(0.0), m_maxI(10000.0), m_maxJ(10000.0),
    m_numI(1000), m_numJ(1000)
  {
    m_grid    = new DataAccess::Interface::SerialGrid( m_minI, m_minJ, m_maxI, m_maxJ, m_numI, m_numJ );
    m_gridMap = new DataAccess::Interface::SerialGridMap( nullptr, 0, m_grid, 0, 1 );
  }

  ~FaultPlaneTest()
  {
    delete m_gridMap;
    delete m_grid;
  }

  // global grid size variables (no ghost nodes)
  const unsigned int m_firstI;
  const unsigned int m_firstJ;
  const unsigned int m_lastI;
  const unsigned int m_lastJ;
  const double m_minI;
  const double m_minJ;
  const double m_maxI;
  const double m_maxJ;
  const unsigned int m_numI;
  const unsigned int m_numJ;

  // grids
  const DataAccess::Interface::SerialGrid* m_grid;
  const DataAccess::Interface::SerialGridMap* m_gridMap;
};

class PlaneExpected {
public :
  explicit PlaneExpected ();
  std::vector<PointSequence> createFaultPlane (const bool& intersection) const;
  std::vector<PointSequence> createOutOfBoundsFaultPlane() const;
  std::vector<PointSequence> createFaultPlaneOnBoundaryGridPoints() const;
  std::vector<PointSequence> createFaultPlaneOnInnerGridPoints() const;
private:
  PointSequence createVerticalFaultStick(double xPosition, double yPosition) const;
  PointSequence createFaultStickWithTwoIntersections(double xCoordinate) const;
};

TEST_F(FaultPlaneTest, FaultPlaneWithoutIntersection)
{
  const PlaneExpected planeExpected;
  const FaultPlane faultPlane (planeExpected.createFaultPlane(false));

  std::vector<PointSequence> pntSeq;

  EXPECT_FALSE(faultPlane.intersect( m_gridMap, pntSeq ));
  EXPECT_EQ(pntSeq.size(), 0);
}

TEST_F(FaultPlaneTest, FaultPlaneIntersectionOutOfBounds)
{
  const PlaneExpected planeExpected;
  const FaultPlane faultPlane (planeExpected.createOutOfBoundsFaultPlane());

  std::vector<PointSequence> pntSeq;

  EXPECT_FALSE(faultPlane.intersect( m_gridMap, pntSeq ));
  EXPECT_EQ(pntSeq.size(), 0);
}

TEST_F(FaultPlaneTest, FaultPlaneWithIntersection)
{
  const PlaneExpected planeExpected;
  const FaultPlane faultPlane (planeExpected.createFaultPlane(true));

  std::vector<PointSequence> pntSeq;

  EXPECT_TRUE(faultPlane.intersect( m_gridMap, pntSeq ));
  ASSERT_TRUE(pntSeq.size() > 0);
  EXPECT_EQ(pntSeq[0].size(), 3);
}

TEST_F(FaultPlaneTest, NullFaultPlane)
{
  const std::vector<PointSequence> nullVectPointSequence;
  const FaultPlane faultPlaneNull(nullVectPointSequence);

  std::vector<PointSequence> pntSeq;

  EXPECT_FALSE(faultPlaneNull.intersect( m_gridMap, pntSeq ));
  EXPECT_EQ(pntSeq.size(), 0);
}

TEST_F(FaultPlaneTest, intersectionOnBoundaryGridPointCheck)
{
  const PlaneExpected planeExpected;
  const FaultPlane faultPlane (planeExpected.createFaultPlaneOnBoundaryGridPoints());

  std::vector<PointSequence> pntSeq;
  faultPlane.intersect( m_gridMap, pntSeq );

  EXPECT_EQ(pntSeq.size(), 4);
}

TEST_F(FaultPlaneTest, intersectionOnInnerGridPointCheck)
{
  const PlaneExpected planeExpected;
  const FaultPlane faultPlane (planeExpected.createFaultPlaneOnInnerGridPoints());

  std::vector<PointSequence> pntSeq;
  faultPlane.intersect( m_gridMap, pntSeq );
  ASSERT_TRUE(pntSeq.size() > 0 );

  EXPECT_EQ(pntSeq[0].size(), 2);
}

PlaneExpected::PlaneExpected ()
{
}

std::vector<PointSequence> PlaneExpected::createFaultPlane (const bool& intersection) const
{
  std::vector<PointSequence> faultSticks;

  PointSequence faultStick;
  Point faultPoint;

  faultPoint [Interface::X_COORD] = 0.0;
  faultPoint [Interface::Y_COORD] = 0.0;
  faultPoint [Interface::Z_COORD] = -1.0 * intersection;
  faultStick.push_back(faultPoint);

  faultPoint [Interface::X_COORD] = 100;
  faultPoint [Interface::Y_COORD] = 100;
  faultPoint [Interface::Z_COORD] = 1.0 * intersection;
  faultStick.push_back(faultPoint);

  faultPoint [Interface::X_COORD] = 200;
  faultPoint [Interface::Y_COORD] = 200;
  faultPoint [Interface::Z_COORD] = -1.0 * intersection;
  faultStick.push_back(faultPoint);

  faultPoint [Interface::X_COORD] = 300;
  faultPoint [Interface::Y_COORD] = 300;
  faultPoint [Interface::Z_COORD] = 1.0 * intersection;
  faultStick.push_back(faultPoint);

  faultSticks.push_back(faultStick);

  return faultSticks;
}

std::vector<PointSequence> PlaneExpected::createOutOfBoundsFaultPlane () const
{
  std::vector<PointSequence> faultSticks;

  faultSticks.push_back(createVerticalFaultStick(-100.0, 0.5));
  faultSticks.push_back(createVerticalFaultStick(0.5, -100.0));
  faultSticks.push_back(createVerticalFaultStick(15000, 1.0));
  faultSticks.push_back(createVerticalFaultStick(1.0, 15000));

  return faultSticks;
}

std::vector<PointSequence> PlaneExpected::createFaultPlaneOnBoundaryGridPoints () const
{
  std::vector<PointSequence> faultSticks;

  faultSticks.push_back(createVerticalFaultStick(0.0, 0.0));
  faultSticks.push_back(createVerticalFaultStick(0.0, 10000.0));
  faultSticks.push_back(createVerticalFaultStick(10000.0, 0.0));
  faultSticks.push_back(createVerticalFaultStick(10000.0, 10000.0));

  return faultSticks;
}

std::vector<PointSequence> PlaneExpected::createFaultPlaneOnInnerGridPoints () const
{
  std::vector<PointSequence> faultSticks;

  faultSticks.push_back(createVerticalFaultStick(20, 20));
  faultSticks.push_back(createVerticalFaultStick(50, 50));

  return faultSticks;
}

PointSequence PlaneExpected::createVerticalFaultStick(double xPosition, double yPosition) const
{
  PointSequence faultStick;
  Point faultPoint;

  faultPoint [Interface::X_COORD] = xPosition;
  faultPoint [Interface::Y_COORD] = yPosition;
  faultPoint [Interface::Z_COORD] = -1.0;
  faultStick.push_back(faultPoint);

  faultPoint [Interface::X_COORD] = xPosition;
  faultPoint [Interface::Y_COORD] = yPosition;
  faultPoint [Interface::Z_COORD] = 1.0;
  faultStick.push_back(faultPoint);

  return faultStick;
}


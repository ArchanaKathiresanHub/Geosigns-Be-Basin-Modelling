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
  std::vector<PointSequence> createDisorderedFaultPlane() const;
  std::vector<PointSequence> createClosedLoopFaultPlane() const;
  std::vector<PointSequence> createSplittedFaultPlane() const;
  std::vector<PointSequence> createOutOfBoundsFaultPlane() const;
  std::vector<PointSequence> createFaultPlaneOnBoundaryGridPoints() const;
  std::vector<PointSequence> createFaultPlaneOnInnerGridPoints() const;
  std::vector<PointSequence> createDifficultFaultPlaneShape() const;
private:
  PointSequence createVerticalFaultStick(double xPosition, double yPosition) const;
  PointSequence createFaultStickWithTwoIntersections(double xCoordinate) const;
};

TEST_F(FaultPlaneTest, FaultPlaneWithoutIntersection)
{
  PlaneExpected planeExpected;
  FaultPlane faultPlane (planeExpected.createFaultPlane(false));

  std::vector<PointSequence> pntSeq;

  EXPECT_FALSE(faultPlane.intersect( m_gridMap, pntSeq ));
  EXPECT_EQ(pntSeq.size(), 0);
}

TEST_F(FaultPlaneTest, FaultPlaneIntersectionOutOfBounds)
{
  PlaneExpected planeExpected;
  FaultPlane faultPlane (planeExpected.createOutOfBoundsFaultPlane());

  std::vector<PointSequence> pntSeq;

  EXPECT_FALSE(faultPlane.intersect( m_gridMap, pntSeq ));
  EXPECT_EQ(pntSeq.size(), 0);
}

TEST_F(FaultPlaneTest, FaultPlaneWithIntersection)
{
  PlaneExpected planeExpected;
  FaultPlane faultPlane (planeExpected.createFaultPlane(true));

  std::vector<PointSequence> pntSeq;

  EXPECT_TRUE(faultPlane.intersect( m_gridMap, pntSeq ));
  ASSERT_TRUE(pntSeq.size() > 0);
  EXPECT_EQ(pntSeq[0].size(), 3);
}

TEST_F(FaultPlaneTest, NullFaultPlane)
{
  std::vector<PointSequence> nullVectPointSequence;
  FaultPlane faultPlaneNull(nullVectPointSequence);

  std::vector<PointSequence> pntSeq;

  EXPECT_FALSE(faultPlaneNull.intersect( m_gridMap, pntSeq ));
  EXPECT_EQ(pntSeq.size(), 0);
}

TEST_F(FaultPlaneTest, checkOrderOfFaultCuts)
{
  PlaneExpected planeExpected;
  FaultPlane faultPlane (planeExpected.createDisorderedFaultPlane());

  std::vector<PointSequence> pntSeq;

  faultPlane.intersect( m_gridMap, pntSeq );

  double totalDistanceX = 0.0;
  double totalDistanceY = 0.0;

  for (unsigned int i = 0; i < pntSeq[0].size() - 1; i++)
  {
    totalDistanceX += std::fabs(pntSeq[0][i](X_COORD) - pntSeq[0][i+1](X_COORD));
    totalDistanceY += std::fabs(pntSeq[0][i](Y_COORD) - pntSeq[0][i+1](Y_COORD));
  }

  EXPECT_TRUE(std::fabs(totalDistanceX - 350) < 0.00001);
  EXPECT_TRUE(std::fabs(totalDistanceY - 350) < 0.00001);
}

TEST_F(FaultPlaneTest, intersectionOnBoundaryGridPointCheck)
{
  PlaneExpected planeExpected;
  FaultPlane faultPlane (planeExpected.createFaultPlaneOnBoundaryGridPoints());

  std::vector<PointSequence> pntSeq;
  faultPlane.intersect( m_gridMap, pntSeq );

  EXPECT_EQ(pntSeq.size(), 4);
}

TEST_F(FaultPlaneTest, intersectionOnInnerGridPointCheck)
{
  PlaneExpected planeExpected;
  FaultPlane faultPlane (planeExpected.createFaultPlaneOnInnerGridPoints());

  std::vector<PointSequence> pntSeq;
  faultPlane.intersect( m_gridMap, pntSeq );
  ASSERT_TRUE(pntSeq.size() > 0 );

  EXPECT_EQ(pntSeq[0].size(), 2);
}

TEST_F(FaultPlaneTest, checkIfLoopClosed)
{
  PlaneExpected planeExpected;
  FaultPlane faultPlane (planeExpected.createClosedLoopFaultPlane());

  std::vector<PointSequence> pntSeq;

  faultPlane.intersect( m_gridMap, pntSeq );

  EXPECT_EQ(pntSeq[0].front(), pntSeq[0].back());

  for ( unsigned int i = 0; i < pntSeq[0].size()-1; i++)
  {
    EXPECT_TRUE(std::fabs(separationDistance(pntSeq[0][i], pntSeq[0][i+1]) - 100) < 1e-5);
  }


}

TEST_F(FaultPlaneTest, checkSplittedFaultPlane)
{
  PlaneExpected planeExpected;
  FaultPlane faultPlane (planeExpected.createSplittedFaultPlane());

  std::vector<PointSequence> faultCuts;

  faultPlane.intersect( m_gridMap, faultCuts );

  EXPECT_EQ(faultCuts.size(), 2);
}


TEST_F(FaultPlaneTest, checkOrderOfDifficultFaultPlane)
{
  PlaneExpected planeExpected;
  FaultPlane faultPlane (planeExpected.createDifficultFaultPlaneShape());

  std::vector<PointSequence> faultCuts;

  faultPlane.intersect( m_gridMap, faultCuts );

  for (unsigned int i = 0; i < faultCuts[0].size() - 1; i++)
  {
    EXPECT_TRUE(faultCuts[0][i](X_COORD) > faultCuts[0][i + 1](X_COORD));
  }
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

std::vector<PointSequence> PlaneExpected::createDisorderedFaultPlane () const
{
  std::vector<PointSequence> faultSticks;

  faultSticks.push_back(createVerticalFaultStick(300, 300));
  faultSticks.push_back(createVerticalFaultStick(150, 150));
  faultSticks.push_back(createVerticalFaultStick(350, 350));
  faultSticks.push_back(createVerticalFaultStick(50, 50));
  faultSticks.push_back(createVerticalFaultStick(100, 100));
  faultSticks.push_back(createVerticalFaultStick(10, 10));
  faultSticks.push_back(createVerticalFaultStick(280, 280));
  faultSticks.push_back(createVerticalFaultStick(0.0, 0.0));

  return faultSticks;
}

std::vector<PointSequence> PlaneExpected::createDifficultFaultPlaneShape () const
{
  std::vector<PointSequence> faultSticks;

  faultSticks.push_back(createVerticalFaultStick(100, 50));
  faultSticks.push_back(createVerticalFaultStick(200, 50));
  faultSticks.push_back(createVerticalFaultStick(300, 50));
  faultSticks.push_back(createVerticalFaultStick(400, 50));
  faultSticks.push_back(createVerticalFaultStick(500, 50));
  faultSticks.push_back(createVerticalFaultStick(600, 50));
  faultSticks.push_back(createVerticalFaultStick(450, 150));
  faultSticks.push_back(createVerticalFaultStick(250, 150));

  return faultSticks;
}


std::vector<PointSequence> PlaneExpected::createSplittedFaultPlane () const
{
  std::vector<PointSequence> faultSticks;

  faultSticks.push_back(createVerticalFaultStick(0.5, 0.5));
  faultSticks.push_back(createVerticalFaultStick(0.1, 0.1));
  faultSticks.push_back(createVerticalFaultStick(3.0, 3.0));
  faultSticks.push_back(createVerticalFaultStick(9003.5, 9003.5));
  faultSticks.push_back(createVerticalFaultStick(9000.2, 9000.2));
  faultSticks.push_back(createVerticalFaultStick(9002.8, 9002.8));

  return faultSticks;
}


std::vector<PointSequence> PlaneExpected::createClosedLoopFaultPlane () const
{
  std::vector<PointSequence> faultSticks;

  faultSticks.push_back(createVerticalFaultStick(0, 0));
  faultSticks.push_back(createVerticalFaultStick(100, 0));
  faultSticks.push_back(createVerticalFaultStick(100, 100));
  faultSticks.push_back(createVerticalFaultStick(0, 100));

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

PointSequence PlaneExpected::createFaultStickWithTwoIntersections(double xCoordinate) const
{
  PointSequence faultStick;
  Point faultPoint;
  faultPoint [Interface::X_COORD] = xCoordinate;
  faultPoint [Interface::Y_COORD] = 0.5;
  faultPoint [Interface::Z_COORD] = 0.01;
  faultStick.push_back(faultPoint);

  faultPoint [Interface::X_COORD] = xCoordinate;
  faultPoint [Interface::Y_COORD] = 1.0;
  faultPoint [Interface::Z_COORD] = -1.0;
  faultStick.push_back(faultPoint);

  faultPoint [Interface::X_COORD] = xCoordinate;
  faultPoint [Interface::Y_COORD] = 1.5;
  faultPoint [Interface::Z_COORD] = 0.01;
  faultStick.push_back(faultPoint);

  return faultStick;
}

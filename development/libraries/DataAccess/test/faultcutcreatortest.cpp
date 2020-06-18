////// Copyright (C) 2012-2020 Shell International Exploration & Production.
////// All rights reserved.

////// Confidential and proprietary source code of Shell.
////// Do not distribute without written permission from Shell.

#include "FaultCutCreator.h"

#include "SerialGrid.h"
#include "SerialGridMap.h"

#include <gtest/gtest.h>

using namespace DataAccess;
using namespace Interface;

PointSequence createIntersectionsSeparateFaultCuts()
{
  PointSequence intersections;
  // First FaultCut
  intersections.push_back(Point(1.0, 1.0, 0.0));
  intersections.push_back(Point(1.0, 2.0, 0.0));
  intersections.push_back(Point(1.0, 3.0, 0.0));
  intersections.push_back(Point(1.0, 4.0, 0.0));
  intersections.push_back(Point(1.0, 5.0, 0.0));

  // Second FaultCut
  intersections.push_back(Point(5.0, 1.0, 0.0));
  intersections.push_back(Point(5.0, 2.0, 0.0));
  intersections.push_back(Point(5.0, 3.0, 0.0));
  intersections.push_back(Point(5.0, 4.0, 0.0));
  intersections.push_back(Point(5.0, 5.0, 0.0));

  return intersections;
}

std::vector<PointSequence> createSeparateFaultCuts()
{
  std::vector<PointSequence> expectedFaultCuts;
  PointSequence expectedFaultCut;

  //First FaultCut (order is reversed, but contains the same points and connections)
  expectedFaultCut.push_back(Point(1.0, 5.0, 0.0));
  expectedFaultCut.push_back(Point(1.0, 4.0, 0.0));
  expectedFaultCut.push_back(Point(1.0, 3.0, 0.0));
  expectedFaultCut.push_back(Point(1.0, 2.0, 0.0));
  expectedFaultCut.push_back(Point(1.0, 1.0, 0.0));
  expectedFaultCuts.push_back(expectedFaultCut);

  //Second FaultCut (order is reversed, but contains the same points and connections)
  expectedFaultCut.clear();
  expectedFaultCut.push_back(Point(5.0, 5.0, 0.0));
  expectedFaultCut.push_back(Point(5.0, 4.0, 0.0));
  expectedFaultCut.push_back(Point(5.0, 3.0, 0.0));
  expectedFaultCut.push_back(Point(5.0, 2.0, 0.0));
  expectedFaultCut.push_back(Point(5.0, 1.0, 0.0));
  expectedFaultCuts.push_back(expectedFaultCut);

  return expectedFaultCuts;
}

PointSequence createIntersectionsSeparateFaultCutsShuffle()
{
  PointSequence intersections;
  // The two faultcuts are shuffled now
  intersections.push_back(Point(1.0, 1.0, 0.0));
  intersections.push_back(Point(5.0, 1.0, 0.0));
  intersections.push_back(Point(5.0, 5.0, 0.0));
  intersections.push_back(Point(1.0, 5.0, 0.0));
  intersections.push_back(Point(5.0, 4.0, 0.0));
  intersections.push_back(Point(5.0, 3.0, 0.0));
  intersections.push_back(Point(1.0, 3.0, 0.0));
  intersections.push_back(Point(1.0, 4.0, 0.0));
  intersections.push_back(Point(5.0, 2.0, 0.0));
  intersections.push_back(Point(1.0, 2.0, 0.0));

  return intersections;
}

PointSequence createIntersectionsLoop()
{
  PointSequence intersections;
  intersections.push_back(Point(1.0, 1.0, 0.0));
  intersections.push_back(Point(1.0, 2.0, 0.0));
  intersections.push_back(Point(1.0, 3.0, 0.0));
  intersections.push_back(Point(2.0, 3.0, 0.0));
  intersections.push_back(Point(3.0, 3.0, 0.0));
  intersections.push_back(Point(3.0, 2.0, 0.0));
  intersections.push_back(Point(3.0, 1.0, 0.0));
  intersections.push_back(Point(2.0, 1.0, 0.0));

  return intersections;
}

std::vector<PointSequence> createClosedLoop()
{
  std::vector<PointSequence> expectedFaultCuts;
  PointSequence expectedFaultCut;

  // Expected Loop (order is reversed, but contains the same points and connections)
  expectedFaultCut.push_back(Point(2.0, 1.0, 0.0));
  expectedFaultCut.push_back(Point(3.0, 1.0, 0.0));
  expectedFaultCut.push_back(Point(3.0, 2.0, 0.0));
  expectedFaultCut.push_back(Point(3.0, 3.0, 0.0));
  expectedFaultCut.push_back(Point(2.0, 3.0, 0.0));
  expectedFaultCut.push_back(Point(1.0, 3.0, 0.0));
  expectedFaultCut.push_back(Point(1.0, 2.0, 0.0));
  expectedFaultCut.push_back(Point(1.0, 1.0, 0.0));

  // The extra point ensures that the loop is closed
  expectedFaultCut.push_back(Point(2.0, 1.0, 0.0));

  expectedFaultCuts.push_back(expectedFaultCut);

  return expectedFaultCuts;
}

PointSequence createIntersectionsLoopShuffled()
{
  PointSequence intersections;
  intersections.push_back(Point(1.0, 3.0, 0.0));
  intersections.push_back(Point(3.0, 1.0, 0.0));
  intersections.push_back(Point(3.0, 3.0, 0.0));
  intersections.push_back(Point(1.0, 2.0, 0.0));
  intersections.push_back(Point(1.0, 1.0, 0.0));
  intersections.push_back(Point(3.0, 2.0, 0.0));
  intersections.push_back(Point(2.0, 3.0, 0.0));
  intersections.push_back(Point(2.0, 1.0, 0.0));

  return intersections;
}

std::vector<PointSequence> createClosedLoopShuffled()
{
  std::vector<PointSequence> expectedFaultCuts;
  PointSequence expectedFaultCut;

  // Expected Loop
  expectedFaultCut.push_back(Point(2.0, 1.0, 0.0));
  expectedFaultCut.push_back(Point(1.0, 1.0, 0.0));
  expectedFaultCut.push_back(Point(1.0, 2.0, 0.0));
  expectedFaultCut.push_back(Point(1.0, 3.0, 0.0));
  expectedFaultCut.push_back(Point(2.0, 3.0, 0.0));
  expectedFaultCut.push_back(Point(3.0, 3.0, 0.0));
  expectedFaultCut.push_back(Point(3.0, 2.0, 0.0));
  expectedFaultCut.push_back(Point(3.0, 1.0, 0.0));

  // The extra point ensures that the loop is closed
  expectedFaultCut.push_back(Point(2.0, 1.0, 0.0));

  expectedFaultCuts.push_back(expectedFaultCut);

  return expectedFaultCuts;
}

PointSequence createIntersectionsIntersectingLoop()
{
  PointSequence intersections;
  intersections.push_back(Point(1.0, -2.0, 0.0));
  intersections.push_back(Point(1.0, -1.0, 0.0));
  intersections.push_back(Point(1.0, 0.0, 0.0));
  intersections.push_back(Point(1.0, 0.95, 0.0));
  intersections.push_back(Point(1.0, 1.05, 0.0));
  intersections.push_back(Point(1.0, 2.0, 0.0));
  intersections.push_back(Point(1.0, 3.0, 0.0));
  intersections.push_back(Point(2.0, 3.0, 0.0));
  intersections.push_back(Point(3.0, 3.0, 0.0));
  intersections.push_back(Point(3.0, 2.0, 0.0));
  intersections.push_back(Point(3.0, 1.0, 0.0));
  intersections.push_back(Point(2.0, 1.0, 0.0));
  intersections.push_back(Point(0.0, 1.0, 0.0));
  intersections.push_back(Point(-1.0, 1.0, 0.0));
  intersections.push_back(Point(-2.0, 1.0, 0.0));

  return intersections;
}

std::vector<PointSequence> createUnwindedLoop()
{
  std::vector<PointSequence> expectedFaultCuts;
  PointSequence expectedFaultCut;

  // Expected Unwinded Loop
  expectedFaultCut.push_back(Point(-2.0, 1.0, 0.0));
  expectedFaultCut.push_back(Point(-1.0, 1.0, 0.0));
  expectedFaultCut.push_back(Point(0.0, 1.0, 0.0));
  expectedFaultCut.push_back(Point(1.0, 1.05, 0.0));
  expectedFaultCut.push_back(Point(1.0, 2.0, 0.0));
  expectedFaultCut.push_back(Point(1.0, 3.0, 0.0));
  expectedFaultCut.push_back(Point(2.0, 3.0, 0.0));
  expectedFaultCut.push_back(Point(3.0, 3.0, 0.0));
  expectedFaultCut.push_back(Point(3.0, 2.0, 0.0));
  expectedFaultCut.push_back(Point(3.0, 1.0, 0.0));
  expectedFaultCut.push_back(Point(2.0, 1.0, 0.0));
  expectedFaultCut.push_back(Point(1.0, 0.95, 0.0));
  expectedFaultCut.push_back(Point(1.0, 0.0, 0.0));
  expectedFaultCut.push_back(Point(1.0, -1.0, 0.0));
  expectedFaultCut.push_back(Point(1.0, -2.0, 0.0));

  expectedFaultCuts.push_back(expectedFaultCut);

  return expectedFaultCuts;
}

PointSequence createIntersectionsZigZag()
{
  PointSequence intersections;
  intersections.push_back(Point(2.0, 0.0, 0.0));
  intersections.push_back(Point(2.0, 1.0, 0.0));
  intersections.push_back(Point(4.0, 2.0, 0.0));
  intersections.push_back(Point(2.0, 3.0, 0.0));
  intersections.push_back(Point(2.0, 4.0, 0.0));
  intersections.push_back(Point(0.0, 5.0, 0.0));
  intersections.push_back(Point(2.0, 6.0, 0.0));
  intersections.push_back(Point(2.0, 7.0, 0.0));

  return intersections;
}

std::vector<PointSequence> createZigZag()
{
  std::vector<PointSequence> expectedFaultCuts;
  PointSequence expectedFaultCut;

  // Expected Zig zag fault cut
  expectedFaultCut.push_back(Point(2.0, 7.0, 0.0));
  expectedFaultCut.push_back(Point(2.0, 6.0, 0.0));
  expectedFaultCut.push_back(Point(0.0, 5.0, 0.0));
  expectedFaultCut.push_back(Point(2.0, 4.0, 0.0));
  expectedFaultCut.push_back(Point(2.0, 3.0, 0.0));
  expectedFaultCut.push_back(Point(4.0, 2.0, 0.0));
  expectedFaultCut.push_back(Point(2.0, 1.0, 0.0));
  expectedFaultCut.push_back(Point(2.0, 0.0, 0.0));
  expectedFaultCuts.push_back(expectedFaultCut);

  return expectedFaultCuts;
}

PointSequence createIntersectionsZigZagShuffled()
{
  PointSequence intersections;
  intersections.push_back(Point(2.0, 0.0, 0.0));
  intersections.push_back(Point(4.0, 2.0, 0.0));
  intersections.push_back(Point(0.0, 5.0, 0.0));
  intersections.push_back(Point(2.0, 4.0, 0.0));
  intersections.push_back(Point(2.0, 7.0, 0.0));
  intersections.push_back(Point(2.0, 6.0, 0.0));
  intersections.push_back(Point(2.0, 1.0, 0.0));
  intersections.push_back(Point(2.0, 3.0, 0.0));

  return intersections;
}

TEST(FaultCutCreatorTest, TwoSeparateFaultCuts)
{
  // Given

  // FaultCuts:
  //
  //  x x x x x
  //
  //
  //  x x x x x
  //
  const PointSequence intersections = createIntersectionsSeparateFaultCuts();

  // When
  FaultCutCreator faultCutCreator(intersections);
  const std::vector<PointSequence> actualFaultCuts = faultCutCreator.createFaultCuts(1.5);

  // Then
  const std::vector<PointSequence> expectedFaultCuts = createSeparateFaultCuts();
  EXPECT_EQ(expectedFaultCuts, actualFaultCuts);
}

TEST(FaultCutCreatorTest, TwoSeparateFaultCutsShuffle)
{
  // Given
  //  x x x x x
  //
  //
  //  x x x x x
  const PointSequence intersections = createIntersectionsSeparateFaultCutsShuffle();

  // When
  FaultCutCreator faultCutCreator(intersections);
  const std::vector<PointSequence> actualFaultCuts = faultCutCreator.createFaultCuts(1.5);

  // Then
  const std::vector<PointSequence> expectedFaultCuts = createSeparateFaultCuts();
  EXPECT_EQ(expectedFaultCuts, actualFaultCuts);
}

TEST(FaultCutCreatorTest, ClosedLoop)
{
  // Given
  // Create a loop
  //     x x x
  //     x   x
  //     x x x
  //
  const PointSequence intersections = createIntersectionsLoop();

  // When
  FaultCutCreator faultCutCreator(intersections);
  const std::vector<PointSequence> actualFaultCuts = faultCutCreator.createFaultCuts(1.5);

  // Then
  const std::vector<PointSequence> expectedFaultCuts = createClosedLoop();
  EXPECT_EQ(expectedFaultCuts, actualFaultCuts);
}

TEST(FaultCutCreatorTest, ClosedLoopShuffled)
{
  // Given
  // Create a shuffled loop
  //     x x x
  //     x   x
  //     x x x
  //
  const PointSequence intersections = createIntersectionsLoopShuffled();

  // When
  FaultCutCreator faultCutCreator(intersections);
  const std::vector<PointSequence> actualFaultCuts = faultCutCreator.createFaultCuts(1.5);

  //Then
  const std::vector<PointSequence> expectedFaultCuts = createClosedLoopShuffled();
  EXPECT_EQ(expectedFaultCuts, actualFaultCuts);
}

TEST(FaultCutCreatorTest, UnwindIntersectingLoop)
{
  // Given
  // Create an intersecting loop
  //      x x x
  //      x   x
  //x x xx xx x
  //      x
  //      x
  //      x
  const PointSequence intersections = createIntersectionsIntersectingLoop();

  // When
  FaultCutCreator faultCutCreator(intersections);
  const std::vector<PointSequence> actualFaultCuts = faultCutCreator.createFaultCuts(3.0);

  // Then
  const std::vector<PointSequence> expectedFaultCuts = createUnwindedLoop();
  EXPECT_EQ(expectedFaultCuts, actualFaultCuts);
}

TEST(FaultCutCreatorTest, CheckOrderOfZigZagFaultCut)
{
  // Given
  // Create a zigzag pattern
  //      x
  //      x
  // x
  //      x
  //      x
  //           x
  //      x
  //      x
  const PointSequence intersections = createIntersectionsZigZag();

  // When
  FaultCutCreator faultCutCreator(intersections);
  const std::vector<PointSequence> actualFaultCuts = faultCutCreator.createFaultCuts(5.0);

  // Then
  const std::vector<PointSequence> expectedFaultCuts = createZigZag();
  EXPECT_EQ(expectedFaultCuts, actualFaultCuts);
}

TEST(FaultCutCreatorTest, CheckOrderOfShuffledZigZagFaultCut)
{
  // Create a shuffled zigzag pattern
  //      x
  //      x
  // x
  //      x
  //      x
  //           x
  //      x
  //      x
  const PointSequence intersections = createIntersectionsZigZagShuffled();

  // When
  FaultCutCreator faultCutCreator(intersections);
  const std::vector<PointSequence> actualFaultCuts = faultCutCreator.createFaultCuts(5.0);

  // Then
  const std::vector<PointSequence> expectedFaultCuts = createZigZag();
  EXPECT_EQ(expectedFaultCuts, actualFaultCuts);
}

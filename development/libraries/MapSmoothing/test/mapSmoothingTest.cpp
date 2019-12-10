//
// Copyright (C) 2015-2019 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include <gtest/gtest.h>

#include "../src/mapSmootherGridMap.h"

#include "ProjectHandle.h"
#include "ObjectFactory.h"
#include "SerialGrid.h"
#include "GridMap.h"

#include <string>
#include <cstring>
#include <memory>

TEST( MapSmoothingTests, GaussianSmootherUniform )
{
  DataAccess::Interface::ObjectFactory factory;
  DataAccess::Interface::SerialGrid grid( 0, 0, 10, 10, 11, 11 );
  DataAccess::Interface::GridMap* gridMap( factory.produceGridMap( nullptr, 0, &grid, 1.0 ) );

  std::unique_ptr<MapSmoothing::MapSmootherGridMap> mapSmoother( new MapSmoothing::MapSmootherGridMap( gridMap, 1.0 ) );
  mapSmoother->doSmoothing( MapSmoothing::FilterType::Gaussian );

  for ( unsigned int i = gridMap->firstI(); i<=gridMap->lastI(); ++i )
  {
    for ( unsigned int j = gridMap->firstJ(); j<=gridMap->lastJ(); ++j )
    {
      EXPECT_EQ( gridMap->getValue(i, j), 1.0 );
    }
  }

  delete gridMap;
}

TEST( MapSmoothingTests, MovingAverageUniform )
{
  DataAccess::Interface::ObjectFactory factory;
  DataAccess::Interface::SerialGrid grid( 0, 0, 10, 10, 11, 11 );
  DataAccess::Interface::GridMap* gridMap( factory.produceGridMap( nullptr, 0, &grid, 1.0 ) );

  std::unique_ptr<MapSmoothing::MapSmootherGridMap> mapSmoother( new MapSmoothing::MapSmootherGridMap( gridMap, 1.0 ) );
  mapSmoother->doSmoothing( MapSmoothing::FilterType::MovingAverage );

  for ( unsigned int i = gridMap->firstI(); i<=gridMap->lastI(); ++i )
  {
    for ( unsigned int j = gridMap->firstJ(); j<=gridMap->lastJ(); ++j )
    {
      EXPECT_EQ( gridMap->getValue(i, j), 1.0 );
    }
  }

  delete gridMap;
}

TEST( MapSmoothingTests, GaussianSmoother )
{
  DataAccess::Interface::ObjectFactory factory;
  DataAccess::Interface::SerialGrid grid( 0, 0, 10, 10, 11, 11 );
  DataAccess::Interface::GridMap* gridMap( factory.produceGridMap( nullptr, 0, &grid, 1.0 ) );

  double val = 0.0;
  for ( unsigned int i = gridMap->firstI(); i<=gridMap->lastI(); ++i )
  {
    for ( unsigned int j = gridMap->firstJ(); j<=gridMap->lastJ(); ++j )
    {
      gridMap->setValue( i, j, val );
      val += 1.0;
    }
  }

  std::unique_ptr<MapSmoothing::MapSmootherGridMap> mapSmoother(new MapSmoothing::MapSmootherGridMap(gridMap, 2.0));
  mapSmoother->doSmoothing( MapSmoothing::FilterType::Gaussian );

  const std::vector<double> expectedValues{ 6.23546, 6.72832, 7.55466, 8.6308,  9.71583, 10.7158, 11.7158, 12.8009, 13.877, 14.7034, 15.1962,
                                            11.6569, 12.1498, 12.9761, 14.0523, 15.1373, 16.1373, 17.1373, 18.2223, 19.2985, 20.1248, 20.6177,
                                            20.7467, 21.2395, 22.0659, 23.142,  24.2271, 25.2271, 26.2271, 27.3121, 28.3882, 29.2146, 29.7074,
                                            32.5842, 33.0771, 33.9034, 34.9795, 36.0646, 37.0646, 38.0646, 39.1496, 40.2258, 41.0521, 41.545,
                                            44.5196, 45.0125, 45.8388, 46.915,  48, 49, 50, 51.085, 52.1612, 52.9875, 53.4804,
                                            55.5196, 56.0125, 56.8388, 57.915,  59, 60, 61, 62.085, 63.1612, 63.9875, 64.4804,
                                            66.5196, 67.0125, 67.8388, 68.915,  70, 71, 72, 73.085, 74.1612, 74.9875, 75.4804,
                                            78.455,  78.9479, 79.7742, 80.8504, 81.9354, 82.9354, 83.9354, 85.0205, 86.0966, 86.9229, 87.4158,
                                            90.2926, 90.7854, 91.6118, 92.6879, 93.7729, 94.7729, 95.7729, 96.858, 97.9341, 98.7605, 99.2533,
                                            99.3823, 99.8752, 100.702, 101.778, 102.863, 103.863, 104.863, 105.948, 107.024, 107.85, 108.343,
                                            104.804, 105.297, 106.123, 107.199, 108.284, 109.284, 110.284, 111.369, 112.445, 113.272, 113.765};

  int k = 0;
  for ( unsigned int i = gridMap->firstI(); i<=gridMap->lastI(); ++i )
  {
    for ( unsigned int j = gridMap->firstJ(); j<=gridMap->lastJ(); ++j )
    {
      EXPECT_NEAR( gridMap->getValue(i, j), expectedValues[k++], 1e-3 );
    }
  }

  delete gridMap;
}

TEST( MapSmoothingTests, MovingAverageSmoother )
{
  DataAccess::Interface::ObjectFactory factory;
  DataAccess::Interface::SerialGrid grid( 0, 0, 10, 10, 11, 11 );
  DataAccess::Interface::GridMap* gridMap( factory.produceGridMap( nullptr, 0, &grid, 1.0 ) );

  double val = 0.0;
  for ( unsigned int i = gridMap->firstI(); i<=gridMap->lastI(); ++i )
  {
    for ( unsigned int j = gridMap->firstJ(); j<=gridMap->lastJ(); ++j )
    {
      gridMap->setValue( i, j, val );
      val += 1.0;
    }
  }

  std::unique_ptr<MapSmoothing::MapSmootherGridMap> mapSmoother( new MapSmoothing::MapSmootherGridMap( gridMap, 2.0 ) );
  mapSmoother->doSmoothing( MapSmoothing::FilterType::MovingAverage );

  const std::vector<double> expectedValues{ 7.2, 7.8, 8.6, 9.6, 10.6, 11.6, 12.6, 13.6, 14.6, 15.4, 16,
                                            13.8, 14.4, 15.2, 16.2, 17.2, 18.2, 19.2, 20.2, 21.2, 22, 22.6,
                                            22.6, 23.2, 24, 25, 26, 27, 28, 29, 30, 30.8, 31.4,
                                            33.6, 34.2, 35, 36, 37, 38, 39, 40, 41, 41.8, 42.4,
                                            44.6, 45.2, 46, 47, 48, 49, 50, 51, 52, 52.8, 53.4,
                                            55.6, 56.2, 57, 58, 59, 60, 61, 62, 63, 63.8, 64.4,
                                            66.6, 67.2, 68, 69, 70, 71, 72, 73, 74, 74.8, 75.4,
                                            77.6, 78.2, 79, 80, 81, 82, 83, 84, 85, 85.8, 86.4,
                                            88.6, 89.2, 90, 91, 92, 93, 94, 95, 96, 96.8, 97.4,
                                            97.4, 98, 98.8, 99.8, 100.8, 101.8, 102.8, 103.8, 104.8, 105.6, 106.2,
                                            104, 104.6, 105.4, 106.4, 107.4, 108.4, 109.4, 110.4, 111.4, 112.2, 112.8};

  int k = 0;
  for ( unsigned int i = gridMap->firstI(); i<=gridMap->lastI(); ++i )
  {
    for ( unsigned int j = gridMap->firstJ(); j<=gridMap->lastJ(); ++j )
    {
      EXPECT_NEAR( gridMap->getValue(i, j), expectedValues[k++], 1e-3 );
    }
  }

  delete gridMap;
}

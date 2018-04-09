//
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "../src/MapExtensions.h"

// google tests library
#include <gtest/gtest.h>

//std
#include <cmath>
#include <limits>

TEST( MapExtensions, float_type )
{
   constexpr float one = 1;
   const float oneEps = std::nextafter(one, std::numeric_limits<float>::max());
   constexpr float hundred = 100;
   const float hundredEps = std::nextafter(hundred, std::numeric_limits<float>::max());

   Utilities::floatKeyMap map;
   map[one]=1;
   map[oneEps]=2;
   EXPECT_EQ(1, map.size());
   EXPECT_EQ(2, map[one]);
   EXPECT_EQ(2, map[oneEps]);
   map[hundred]=3;
   EXPECT_EQ(2, map.size());
   map[hundredEps]=4;
   EXPECT_EQ(2, map.size());
   EXPECT_EQ(4, map[hundred]);
   EXPECT_EQ(4, map[hundredEps]);
}

TEST( MapExtensions, double_type )
{
   constexpr double one = 1;
   const double oneEps = std::nextafter(one, std::numeric_limits<double>::max());
   constexpr double hundred = 100;
   const double hundredEps = std::nextafter(hundred, std::numeric_limits<double>::max());

   Utilities::floatKeyMap map;
   map[one]=1;
   map[oneEps]=2;
   EXPECT_EQ(1, map.size());
   EXPECT_EQ(2, map[one]);
   EXPECT_EQ(2, map[oneEps]);
   map[hundred]=3;
   EXPECT_EQ(2, map.size());
   map[hundredEps]=4;
   EXPECT_EQ(2, map.size());
   EXPECT_EQ(4, map[hundred]);
   EXPECT_EQ(4, map[hundredEps]);
}
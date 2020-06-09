//
// Copyright (C) 2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by CGI.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// TestFastmig__Leak.cpp : Defines the entry point for the console application.
//

#include "gtest/gtest.h"
#include "../src/Spill.h"
using namespace migration;
using namespace distribute;
namespace
{
   const static int s_size = 3;
   const static double err = 1e-6;

}
// This test checks spilling of fluid
TEST(SpillTest, spilling) {
   const double level = 1.0;
   const double volume = 50.0; 
   double fluidVolumeSpilled = 0.0;
   std::vector< functions::Tuple2<double> > v(s_size, functions::Tuple2<double>(0.0,0.0 ));
   v[1][0] = level;
   v[1][1] = volume;
   v[2][0] = 2.*level;
   v[2][1] = 2.*volume;
   functions::MonotonicIncreasingPiecewiseLinearInvertableFunction f(v);
   Spill mySpill(&f);
// fluidVolume is higher than trap capacity
   mySpill.distribute(500., fluidVolumeSpilled);
   EXPECT_NEAR(fluidVolumeSpilled,400.,err);
// fluidVolume is lower than trap capacity
   fluidVolumeSpilled = 0.0;
   mySpill.distribute(80., fluidVolumeSpilled);
   EXPECT_NEAR(fluidVolumeSpilled,0.0,err);
}
// this test checks maxVLevel and maxVolume
TEST(SpillTest, maxLevelVolume) {
   const double level = 2.0;
   const double volume = 75.0;
   double fluidVolumeSpilled;
   // Changing Level to by one unit and Volume to by one unit
   std::vector< functions::Tuple2<double> > v(s_size, functions::Tuple2<double>(0.0,0.0 ));
   v[1][0] = level;
   v[1][1] = volume;
   v[2][0] = 2.*level;
   v[2][1] = 2.*volume;
   functions::MonotonicIncreasingPiecewiseLinearInvertableFunction f(v);
   Spill mySpill(&f);
   EXPECT_NEAR(mySpill.maxLevel(),4.,err);
   EXPECT_NEAR(mySpill.maxVolume(),150.,err);
}
// this test checks capacity of the trap
TEST(SpillTest, capacity) {
   const double level = 2.0;
   const double volume = 75.0;
   double fluidVolumeSpilled;
   // Changing Level to by one unit and Volume to by one unit
   std::vector< functions::Tuple2<double> > v(s_size, functions::Tuple2<double>(0.0,0.0 ));
   v[1][0] = level;
   v[1][1] = volume;
   v[2][0] = 2.*level;
   v[2][1] = 2.*volume;
   functions::MonotonicIncreasingPiecewiseLinearInvertableFunction f(v);
   Spill mySpill(&f);
   Tuple2<double> myCapacity;
   myCapacity = mySpill.maxContent();
   EXPECT_NEAR(myCapacity[0],4.,err);
   EXPECT_NEAR(myCapacity[1],150.,err);
}



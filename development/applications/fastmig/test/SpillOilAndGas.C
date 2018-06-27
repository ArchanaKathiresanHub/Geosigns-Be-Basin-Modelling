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
#include "../src/SpillOilAndGas.h"
using namespace migration;
using namespace distribute;
namespace
{
   const static int s_size = 3;
   const static double err = 1e-6;

}
// This test checks spilling of fluid
TEST(SpillOilAndGasTest, Spilling) {
   const double level = 1.0;
   const double volume = 50.0; 
   double gasVolumeSpilled = 0.0;
   double oilVolumeSpilled = 0.0;
   std::vector< functions::Tuple2<double> > v(s_size, functions::Tuple2<double>(0.0,0.0 ));
   v[1][0] = level;
   v[1][1] = volume;
   v[2][0] = 2.*level;
   v[2][1] = 2.*volume;
   functions::MonotonicIncreasingPiecewiseLinearInvertableFunction f(v);
   SpillOilAndGas mySpillOilAndGas(&f);
// fluid spilled is greater than oilVolume
   mySpillOilAndGas.distribute(140.,60., gasVolumeSpilled, oilVolumeSpilled);
   EXPECT_NEAR(gasVolumeSpilled,40.,err);
   EXPECT_NEAR(oilVolumeSpilled,60.,err);
// fluid spilled is less than oilVolume
   gasVolumeSpilled = 0.0;
   oilVolumeSpilled = 0.0;
   mySpillOilAndGas.distribute(80.,120., gasVolumeSpilled, oilVolumeSpilled);
   EXPECT_NEAR(gasVolumeSpilled,0.,err);
   EXPECT_NEAR(oilVolumeSpilled,100.,err);
// no fluid spilled
   gasVolumeSpilled = 0.0;
   oilVolumeSpilled = 0.0;
   mySpillOilAndGas.distribute(50.,40., gasVolumeSpilled, oilVolumeSpilled);
   EXPECT_NEAR(gasVolumeSpilled,0.,err);
   EXPECT_NEAR(oilVolumeSpilled,0.,err);
}
// this test checks maxLevel and maxVolume
TEST(SpillOilAndGasTest, maxLevelVolume) {
   const double level = 1.0;
   const double volume = 50.0;
   double gasVolumeSpilled;
   double oilVolumeSpilled;
   std::vector< functions::Tuple2<double> > v(s_size, functions::Tuple2<double>(0.0,0.0 ));
   v[1][0] = level;
   v[1][1] = volume;
   v[2][0] = 2.*level;
   v[2][1] = 2.*volume;
   functions::MonotonicIncreasingPiecewiseLinearInvertableFunction f(v);
   SpillOilAndGas mySpillOilAndGas(&f);
   EXPECT_NEAR(mySpillOilAndGas.maxLevel(),2.,err);
   EXPECT_NEAR(mySpillOilAndGas.maxVolume(),100.,err);
}
//this test checks capacity 
TEST(SpillOilAndGasTest, capacity) {
   const double level = 1.0;
   const double volume = 50.0;
   double gasVolumeSpilled;
   double oilVolumeSpilled;
   std::vector< functions::Tuple2<double> > v(s_size, functions::Tuple2<double>(0.0,0.0 ));
   v[1][0] = level;
   v[1][1] = volume;
   v[2][0] = 2.*level;
   v[2][1] = 2.*volume;
   functions::MonotonicIncreasingPiecewiseLinearInvertableFunction f(v);
   SpillOilAndGas mySpillOilAndGas(&f);
   Tuple2<double> myCapacity;
   myCapacity = mySpillOilAndGas.maxContent();
   EXPECT_NEAR(myCapacity[0],2.,err);
   EXPECT_NEAR(myCapacity[1],100.,err);
}


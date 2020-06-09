//
// Copyright (C) 2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by CGI.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// TestFastmig_LeakOrSpill.cpp : Defines the entry point for the console application.
//

#include "gtest/gtest.h"
#include "../src/Leak.h"
#include "../src/Spill.h"
#include "../src/LeakOrSpill.h"
using namespace migration;
using namespace distribute;
namespace
{
   const static int s_size = 3;
   const static double err = 1e-6;

}
// this test checks fluid is leaking and then fluidVolumedLeaked when fluidVolume is lower than maxFluidVolume
TEST(LeakOrSpillTest, Leaking_nofluidVolumeLeaked) {
   double fluidVolumeLeaked, fluidVolumeSpilled;
   const double level = 2.;
   const double volume = 50.;
   std::vector< functions::Tuple2<double> > v(s_size, functions::Tuple2<double>(0.0,0.0));
   v[1][0] = level;
   v[1][1] = volume;
   v[2][0] = 2.*level;
   v[2][1] = 3.*volume;
   functions::MonotonicIncreasingPiecewiseLinearInvertableFunction f(v);
   LeakOrSpill myLeakOrSpill(800.0, 1073.0, 0.2, 2.0, 10000.0, &f);
   EXPECT_TRUE(myLeakOrSpill.leaking());
   myLeakOrSpill.distribute(80.0, fluidVolumeLeaked, fluidVolumeSpilled);
   EXPECT_NEAR(fluidVolumeLeaked, 0.0, err);
   EXPECT_NEAR(fluidVolumeSpilled, 0.0, err);
}
// this test checks fluid is leaking and then fluidVolumedLeaked when fluidVolume is higher than maxFluidVolume
TEST(LeakOrSpillTest, Leaking_fluidVolumeLeaked) {
   double fluidVolumeLeaked, fluidVolumeSpilled;
   const double level = 2.;
   const double volume = 50.;
   std::vector< functions::Tuple2<double> > v(s_size, functions::Tuple2<double>(0.0,0.0));
   v[1][0] = level;
   v[1][1] = volume;
   v[2][0] = 2.*level;
   v[2][1] = 3.*volume;
   functions::MonotonicIncreasingPiecewiseLinearInvertableFunction f(v);
   LeakOrSpill myLeakOrSpill(800.0, 1073.0, 0.2, 2.0, 10000.0, &f);
   EXPECT_TRUE(myLeakOrSpill.leaking());
   myLeakOrSpill.distribute(200.0, fluidVolumeLeaked, fluidVolumeSpilled);
   EXPECT_NEAR(fluidVolumeLeaked,63.3095365,err);
   EXPECT_NEAR(fluidVolumeSpilled, 0.0, err);
}
// this test checks fluid is spilling and then fluidVolumeSpilled when fluidVolume is lower than maxCapacity
TEST(LeakOrSpillTest, NotLeaking_NotfluidVolumeSpilled) {
   double fluidVolumeLeaked, fluidVolumeSpilled;
   const double level = 2.;
   const double volume = 50.;
   std::vector< functions::Tuple2<double> > v(s_size, functions::Tuple2<double>(0.0,0.0));
   v[1][0] = level;
   v[1][1] = volume;
   v[2][0] = 2.*level;
   v[2][1] = 3.*volume;
   functions::MonotonicIncreasingPiecewiseLinearInvertableFunction f(v);
   LeakOrSpill myLeakOrSpill(1000.0, 1000.0, 0.2, 2.0, 10000.0, &f);
   EXPECT_FALSE(myLeakOrSpill.leaking());
   myLeakOrSpill.distribute(80.0, fluidVolumeLeaked, fluidVolumeSpilled);
   EXPECT_NEAR(fluidVolumeSpilled, 0.0, err);
   EXPECT_NEAR(fluidVolumeLeaked, 0.0, err);
}
// this test checks fluid is spilling and then fluidVolumeSpilled when fluidVolume is higher than maxCapacity
TEST(LeakOrSpillTest, NotLeaking_fluidVolumeSpilled) {
   double fluidVolumeLeaked, fluidVolumeSpilled;
   const double level = 2.;
   const double volume = 50.;
   std::vector< functions::Tuple2<double> > v(s_size, functions::Tuple2<double>(0.0,0.0));
   v[1][0] = level;
   v[1][1] = volume;
   v[2][0] = 2.*level;
   v[2][1] = 3.*volume;
   functions::MonotonicIncreasingPiecewiseLinearInvertableFunction f(v);
   LeakOrSpill myLeakOrSpill(1000.0, 1000.0, 0.2, 2.0, 10000.0, &f);
   EXPECT_FALSE(myLeakOrSpill.leaking());
   myLeakOrSpill.distribute(200.0, fluidVolumeLeaked, fluidVolumeSpilled);
   EXPECT_NEAR(fluidVolumeSpilled,50.0,err);
   EXPECT_NEAR(fluidVolumeLeaked, 0.0, err);
}

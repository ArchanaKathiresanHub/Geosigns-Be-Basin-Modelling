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
#include "../src/Leak.h"
using namespace migration;
using namespace distribute;
namespace
{
   const static int s_size = 3;
   const static double err = 1e-6;

}
// This test checks leaking of fluid
TEST(LeakTest, with_Leak) {
   const double FluidVolume = 1000;
   double fluidVolumeLeaked;
   const double level = 2.;
   const double volume = 50.;
   std::vector< functions::Tuple2<double> > v(s_size, functions::Tuple2<double>(0.0,0.0));
   v[1][0] = level;
   v[1][1] = volume;
   v[2][0] = 2.*level;
   v[2][1] = 3.*volume;
   functions::MonotonicIncreasingPiecewiseLinearInvertableFunction f(v);
   Leak myLeak(800.0, 1073.0, 0.2, 2.0, 10000.0, &f);
   myLeak.distribute(FluidVolume, fluidVolumeLeaked);
   EXPECT_NEAR(fluidVolumeLeaked,863.3095365225,err);
}
// This test checks no leaking of fluid
TEST(LeakTest, without_Leak) {
   const double FluidVolume = 100;
   double fluidVolumeLeaked;
   const double level = 2.;
   const double volume = 50.;
   std::vector< functions::Tuple2<double> > v(s_size, functions::Tuple2<double>(0.0,0.0));
   v[1][0] = level;
   v[1][1] = volume;
   v[2][0] = 2.*level;
   v[2][1] = 3.*volume;
   functions::MonotonicIncreasingPiecewiseLinearInvertableFunction f(v);
   Leak myLeak(800.0, 1073.0, 0.2, 2.0, 10000.0, &f);
   myLeak.distribute(FluidVolume, fluidVolumeLeaked);
   EXPECT_NEAR(fluidVolumeLeaked, 0.0,err);
}
// this test checks when there is no leak and crestColumnThickness is zero
TEST(LeakTest, without_Leak_crestColumnThickness_zero) {
   const double level = 2.;
   const double volume = 50.;
   std::vector< functions::Tuple2<double> > v(s_size, functions::Tuple2<double>(0.0,0.0));
   v[1][0] = level;
   v[1][1] = volume;
   v[2][0] = 2.*level;
   v[2][1] = 3.*volume;
   functions::MonotonicIncreasingPiecewiseLinearInvertableFunction f(v);
   Leak myLeak(800.0, 1073.0, 0.2, 0.0, 100000.0, &f);
   EXPECT_NEAR(myLeak.maxVolume(),150.0,err);
}
// this test checks fluidVolumeLeaked when buoyancyForce is zero and fluidVolume is lower than maxFluidVolume
TEST(LeakTest, sealFluidDensity_equal_fluiddensity_lower_fluidvolume) {
   const double FluidVolume = 30;
   double fluidVolumeLeaked;
   const double level = 2.;
   const double volume = 50.;
   std::vector< functions::Tuple2<double> > v(s_size, functions::Tuple2<double>(0.0,0.0));
   v[1][0] = level;
   v[1][1] = volume;
   v[2][0] = 2.*level;
   v[2][1] = 3.*volume;
   functions::MonotonicIncreasingPiecewiseLinearInvertableFunction f(v);
   Leak myLeak(1000.0, 1000.0, 0.2, 2.0, 10000.0, &f);
   myLeak.distribute(FluidVolume, fluidVolumeLeaked);
   EXPECT_NEAR(fluidVolumeLeaked, 0.0, err);
}
// this test checks fluidVolumeLeaked when buoyancyForce is zero and fluidVolume is higher than maxFluidVolume
TEST(LeakTest, sealFluidDensity_equal_fluiddensity_higher_fluidvolume) {
   const double FluidVolume = 200;
   double fluidVolumeLeaked;
   const double level = 2.;
   const double volume = 50.;
   std::vector< functions::Tuple2<double> > v(s_size, functions::Tuple2<double>(0.0,0.0));
   v[1][0] = level;
   v[1][1] = volume;
   v[2][0] = 2.*level;
   v[2][1] = 3.*volume;
   functions::MonotonicIncreasingPiecewiseLinearInvertableFunction f(v);
   Leak myLeak(1000.0, 1000.0, 0.2, 2.0, 10000.0, &f);
   myLeak.distribute(FluidVolume, fluidVolumeLeaked);
   EXPECT_NEAR(fluidVolumeLeaked,50.0,err);
}
// this test checks fluidVolumeLeaked when crestColumnThickness is zero and fluidVolume is lower than maxFluidVolume
TEST(LeakTest, lower_fluidvolume_crestColumnThickness_zero) {
   const double FluidVolume = 80;
   double fluidVolumeLeaked;
   const double level = 2.;
   const double volume = 50.;
   std::vector< functions::Tuple2<double> > v(s_size, functions::Tuple2<double>(0.0,0.0));
   v[1][0] = level;
   v[1][1] = volume;
   v[2][0] = 2.*level;
   v[2][1] = 3.*volume;
   functions::MonotonicIncreasingPiecewiseLinearInvertableFunction f(v);
   Leak myLeak(800.0, 1073.0, 0.2, 0.0, 10000.0, &f);
   myLeak.distribute(FluidVolume, fluidVolumeLeaked);
   EXPECT_NEAR(fluidVolumeLeaked, 0.0, err);
}
// this test checks fluidVolumeLeaked when crestColumnThickness is zero and fluidVolume is higher than maxFluidVolume
TEST(LeakTest, higher_fluidvolume_crestColumnThickness_zero) {
   const double FluidVolume = 200;
   double fluidVolumeLeaked;
   const double level = 2.;
   const double volume = 50.;
   std::vector< functions::Tuple2<double> > v(s_size, functions::Tuple2<double>(0.0,0.0));
   v[1][0] = level;
   v[1][1] = volume;
   v[2][0] = 2.*level;
   v[2][1] = 3.*volume;
   functions::MonotonicIncreasingPiecewiseLinearInvertableFunction f(v);
   Leak myLeak(800.0, 1073.0, 0.2, 0.0, 10000.0, &f);
   myLeak.distribute(FluidVolume, fluidVolumeLeaked);
   EXPECT_NEAR(fluidVolumeLeaked,63.3025655,err);
}
// this test checks fluidVolumeLeaked when overPressureContrast is negative value and fluidVolume is lower than maxFluidVolume
TEST(LeakTest, lower_fluidvolume_overPressureContrast_negative) {
   const double FluidVolume = 70;
   double fluidVolumeLeaked;
   const double level = 2.;
   const double volume = 50.;
   std::vector< functions::Tuple2<double> > v(s_size, functions::Tuple2<double>(0.0,0.0));
   v[1][0] = level;
   v[1][1] = volume;
   v[2][0] = 2.*level;
   v[2][1] = 3.*volume;
   functions::MonotonicIncreasingPiecewiseLinearInvertableFunction f(v);
   Leak myLeak(800.0, 1073.0, -0.2, 2.0, 10000.0, &f);
   myLeak.distribute(FluidVolume, fluidVolumeLeaked);
   EXPECT_NEAR(fluidVolumeLeaked, 0.0, err);
}
// this test checks fluidVolumeLeaked when overPressureContrast is negative value and fluidVolume is higher than maxFluidVolume
TEST(LeakTest, higher_fluidvolume_overPressureContrast_negative) {
   const double FluidVolume = 200;
   double fluidVolumeLeaked;
   const double level = 2.;
   const double volume = 50.;
   std::vector< functions::Tuple2<double> > v(s_size, functions::Tuple2<double>(0.0,0.0));
   v[1][0] = level;
   v[1][1] = volume;
   v[2][0] = 2.*level;
   v[2][1] = 3.*volume;
   functions::MonotonicIncreasingPiecewiseLinearInvertableFunction f(v);
   Leak myLeak(800.0, 1073.0, -0.2, 2.0, 10000.0, &f);
   myLeak.distribute(FluidVolume, fluidVolumeLeaked);
   EXPECT_NEAR(fluidVolumeLeaked,63.29559414,err);
}

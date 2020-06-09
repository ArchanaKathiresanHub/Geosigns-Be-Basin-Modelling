//
// Copyright (C) 2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by CGI.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// TestFastmig_LeakOil.cpp : Defines the entry point for the console application.
//

#include "gtest/gtest.h"
#include "../src/Leak.h"
#include "../src/LeakGas.h"
using namespace migration;
using namespace distribute;
namespace
{
   const static int s_size = 3;
   const static double err = 1e-6;

}
// This test checks limits of oilToGasLevelRatio.Though, oilToGasLevelRatio limits are >=0 and <= 1.0, they never reach either zero or one.  
TEST(LeakGasTest, oilToGasLevelRatio_limits) {
   const double level = 2.;
   const double volume = 50.;
   std::vector< functions::Tuple2<double> > v(s_size, functions::Tuple2<double>(0.0,0.0));
   v[1][0] = level;
   v[1][1] = volume;
   v[2][0] = 2.*level;
   v[2][1] = 3.*volume;
   functions::MonotonicIncreasingPiecewiseLinearInvertableFunction f(v);
   LeakGas myLeakGas(1.0, 900.0, 1073.0, 0.2, 2.0, 10000.0, 30.0, 50.0, &f);
   EXPECT_NEAR(myLeakGas.oilToGasLevelRatio(), 0.16138, err);
}
// This test checks gasvolumeleaked and no oilvolumeleaked
TEST(LeakGasTest, gasvolumeleaked_and_no_oilvolumeleaked) {
   double gasVolumeLeaked, oilVolumeLeaked;
   const double level = 2.;
   const double volume = 50.;
   std::vector< functions::Tuple2<double> > v(s_size, functions::Tuple2<double>(0.0,0.0));
   v[1][0] = level;
   v[1][1] = volume;
   v[2][0] = 2.*level;
   v[2][1] = 3.*volume;
   functions::MonotonicIncreasingPiecewiseLinearInvertableFunction f(v);
   LeakGas myLeakGas(5.0, 800.0, 1073.0, 0.2, 2.0, 10000.0, 5000.0, 20000.0, &f);
   myLeakGas.distribute(300.0, 136.6904634775, gasVolumeLeaked,oilVolumeLeaked);
   EXPECT_NEAR(gasVolumeLeaked, 300.0,err);
   EXPECT_NEAR(oilVolumeLeaked, 0.0,err);
}
// This test checks gasvolumeleaked and oilvolumeleaked
TEST(LeakGasTest, gasvolumeleaked_and_oilvolumeleaked) {
   double gasVolumeLeaked, oilVolumeLeaked;
   const double level = 2.;
   const double volume = 50.;
   std::vector< functions::Tuple2<double> > v(s_size, functions::Tuple2<double>(0.0,0.0));
   v[1][0] = level;
   v[1][1] = volume;
   v[2][0] = 2.*level;
   v[2][1] = 3.*volume;
   functions::MonotonicIncreasingPiecewiseLinearInvertableFunction f(v);
   LeakGas myLeakGas(5.0, 800.0, 1073.0, 0.2, 2.0, 10000.0, 5000.0, 20000.0, &f);
   myLeakGas.distribute(300.0, 200.0, gasVolumeLeaked, oilVolumeLeaked);
   EXPECT_NEAR(gasVolumeLeaked, 300.0, err);
   EXPECT_NEAR(oilVolumeLeaked,63.3095365225,err);
}
// This test checks maxGasVolume and maxOilVolume
TEST(LeakGasTest, gasvolumeleaked_and_oilvolumeleaked_leaking) {
   double gasVolumeLeaked, oilVolumeLeaked;
   const double level = 2.;
   const double volume = 50.;
   std::vector< functions::Tuple2<double> > v(s_size, functions::Tuple2<double>(0.0,0.0));
   v[1][0] = level;
   v[1][1] = volume;
   v[2][0] = 2.*level;
   v[2][1] = 3.*volume;
   functions::MonotonicIncreasingPiecewiseLinearInvertableFunction f(v);
   LeakGas myLeakGas(5.0, 800.0, 1073.0, 0.2, 2.0, 10000.0, 5000.0, 20000.0, &f);
   myLeakGas.distribute(300.0, 150.0, gasVolumeLeaked, oilVolumeLeaked);
   EXPECT_NEAR(myLeakGas.maxGasVolume(), 11.93069127,err);
   EXPECT_NEAR(myLeakGas.maxOilVolume(), 136.6904634775,err);
}

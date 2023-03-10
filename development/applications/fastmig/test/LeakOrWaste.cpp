//                                                                      
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by CGI India Pvt. Ltd.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include"gtest/gtest.h"

#include "ConstantsMathematics.h"
#include "ConstantsPhysics.h"
#include "../src/LeakOrWaste.h"

using namespace migration;
using namespace distribute;

namespace //  ALL in here is local to this translation unit
{
   typedef functions::Tuple2<double> MyTuple;
	//======== Parameters for the Unit Test Test========//
	//The Following properties are mostly same for all the tests
	// fluid density
		const double fluidDensity = 1000;
	// Seal fluid density
		const double sealFluidDensity = 1500;
	//
		const double overPressureContrast = 1;
	//
		const double crestColumnThickness = 0.0;
	//
		const double maxSealPressure = 50;
	//
	const double InitailwasteLevel = 20;
	//error bound for decimal comparison
	const double errLimit = 1e-6;
}

// This class assumes that there is either Leaking or Wasting of HCs not both together.

// This tests whether the system is leaking or not
TEST(LeakOrWasteTest, leakingTest) {
	// The starting point of a column can be any arbitrary real number but is noramised to zero; We use random value for testing here
   std::vector<MyTuple> v(2, MyTuple(11.75, 23.5));
   v[1][0] = 12.75;
   v[1][1] = 25.5;
   functions::MonotonicIncreasingPiecewiseLinearInvertableFunction c_(v);
	LeakOrWaste LOW(fluidDensity, sealFluidDensity, overPressureContrast, crestColumnThickness, maxSealPressure, InitailwasteLevel, &c_);

	ASSERT_TRUE(LOW.leaking());
}

// This tests for the value of the maximum leak level
TEST(LeakOrWasteTest, maxLevelCheckForLeaking) {
	// when there is no crest column
   std::vector<MyTuple> v(2, MyTuple(11.75, 23.5));
   v[1][0] = 12.75;
   v[1][1] = 25.5;
   functions::MonotonicIncreasingPiecewiseLinearInvertableFunction c_(v);
	LeakOrWaste LOW(fluidDensity, sealFluidDensity, overPressureContrast, crestColumnThickness, maxSealPressure, InitailwasteLevel, &c_);
	auto d_ = LOW.maxLevel();
	double buoyancyForce = (sealFluidDensity - fluidDensity) * Utilities::Physics::AccelerationDueToGravity;
	auto maxBuoyancyLevel = maxSealPressure / buoyancyForce;

	ASSERT_NEAR(d_, maxBuoyancyLevel, errLimit);
}

// This tests for the value of the maximum leak level with non zero crest column thickness
TEST(LeakOrWasteTest, maxLevelCheckForLeaking2) {
	// when there is a crest column
	double crestColumnThickness_ = 1;
   std::vector<MyTuple> v(2, MyTuple(11.75, 23.5));
   v[1][0] = 12.75;
   v[1][1] = 25.5;
   functions::MonotonicIncreasingPiecewiseLinearInvertableFunction c_(v);
	LeakOrWaste LOW(fluidDensity, sealFluidDensity, overPressureContrast, crestColumnThickness_, maxSealPressure, InitailwasteLevel, &c_);
	auto d_ = LOW.maxLevel(); 
	double buoyancyForce = (sealFluidDensity - fluidDensity) * Utilities::Physics::AccelerationDueToGravity;
	auto maxBuoyancyLevel = maxSealPressure / (buoyancyForce + overPressureContrast / crestColumnThickness_);
	ASSERT_NEAR(d_, maxBuoyancyLevel, errLimit);
}

// This tests for the value of the maximum leak level with a negative overPressureContrast
TEST(LeakOrWasteTest, maxLevelCheckForLeakingNegativeOPC) {
	//With a non-zero crest column & negative overPressureContrast(OPC)
	double crestColumnThickness_ =  1;
   std::vector<MyTuple> v(2, MyTuple(11.75, 23.5));
   v[1][0] = 12.75;
   v[1][1] = 25.5;
   functions::MonotonicIncreasingPiecewiseLinearInvertableFunction c_(v);
	auto overPressureContrast_ = -5000; // This chosen value is > than the buoyancyForce
	LeakOrWaste LOW(fluidDensity, sealFluidDensity, overPressureContrast_, crestColumnThickness_, maxSealPressure, InitailwasteLevel, &c_);
	auto d_ = LOW.maxLevel();
	
	EXPECT_GT(d_, 0);

	double buoyancyForce = (sealFluidDensity - fluidDensity) * Utilities::Physics::AccelerationDueToGravity;
	auto maxBuoyancyLevel = maxSealPressure / (buoyancyForce + overPressureContrast_ / crestColumnThickness_);
	//std::cout << maxBuoyancyLevel << std::endl;
	EXPECT_LT(maxBuoyancyLevel, 0);
	ASSERT_NEAR(d_, InitailwasteLevel, errLimit);
}

// this tests for fluid volume leaking only
TEST(LeakOrWasteTest, distributeCheckForLeaking) {
	double crestColumnThickness_ = 1;
   std::vector<MyTuple> v(2, MyTuple(11.75, 23.5));
   v[1][0] = 12.75;
   v[1][1] = 25.5;
   functions::MonotonicIncreasingPiecewiseLinearInvertableFunction c_(v);
	LeakOrWaste LOW(fluidDensity, sealFluidDensity, overPressureContrast, crestColumnThickness_, maxSealPressure, InitailwasteLevel, &c_);
	// for leaking to occur fluid vol > maxVol(Capacity)
	double fluidVol = 50;
	double fluidVolLeaked;
	double fluidVolWasted;
	LOW.distribute(fluidVol, fluidVolLeaked, fluidVolWasted);
	ASSERT_NEAR(LOW.maxVolume(), 23.5, errLimit);
	ASSERT_TRUE(fluidVolWasted == 0);
	ASSERT_GT(fluidVolLeaked, 0);
}

// this tests for fluid volume wasting only
TEST(LeakOrWasteTest, fluidVolumeWastingOnly) {
	auto wasteLevel_ = 0.01;// Resetting waste level < leak level
   std::vector<MyTuple> v(2, MyTuple(11.75, 23.5));
   v[1][0] = 12.75;
   v[1][1] = 25.5;
   functions::MonotonicIncreasingPiecewiseLinearInvertableFunction c_(v);
	LeakOrWaste LOW(fluidDensity, sealFluidDensity, overPressureContrast, crestColumnThickness, maxSealPressure, wasteLevel_, &c_);
	//ASSERT_NEAR(LOW.maxLevel(), 0);
	EXPECT_EQ(LOW.leaking(), false);
}

// this tests for max fluid level wasting only
TEST(LeakOrWasteTest, maxLevelForWasting) {
	auto wasteLevel_ = 0.01;
   std::vector<MyTuple> v(2, MyTuple(11.75, 23.5));
   v[1][0] = 12.75;
   v[1][1] = 25.5;
   functions::MonotonicIncreasingPiecewiseLinearInvertableFunction c_(v);
	LeakOrWaste LOW(fluidDensity, sealFluidDensity, overPressureContrast, crestColumnThickness, maxSealPressure, wasteLevel_, &c_);
	auto d_ = LOW.maxLevel();
	ASSERT_NEAR(d_, wasteLevel_,errLimit);
}

// this tests for fluid volume wasting only
TEST(LeakOrWasteTest, distributeForWasting) {
	auto wasteLevel_ = 0.01;
   std::vector<MyTuple> v(2, MyTuple(11.75, 23.5));
   v[1][0] = 12.75;
   v[1][1] = 25.5;
   functions::MonotonicIncreasingPiecewiseLinearInvertableFunction c_(v);
	LeakOrWaste LOW(fluidDensity, sealFluidDensity, overPressureContrast, crestColumnThickness, maxSealPressure, wasteLevel_, &c_);
	// for wasting to occur fluid vol > maxVol(Capacity)
	double fluidVol = 25;
	double fluidVolLeaked;
	double fluidVolWasted;
	LOW.distribute(fluidVol, fluidVolLeaked, fluidVolWasted);
	ASSERT_NEAR(fluidVolWasted, (fluidVol -23.5),errLimit);
	ASSERT_TRUE(fluidVolLeaked == 0);
}
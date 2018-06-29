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
#include "../src/LeakGasAndSpillOil.h"


using namespace migration;
using namespace distribute;

namespace //  ALL in here is local to this translation unit
{
   typedef functions::Tuple2<double> MyTuple;
	//======== Parameters for the Unit Test Test========//
	
	// Class specific Parameters that is changed rarely
	const  double gasDensity = 500;
	const  double	oilDensity = 1023 ;
	const  double	sealFluidDensity = 1100 ;
	const  double	overPressureContrast = 1 ;
	const  double crestColumnThickness = 0.0 ;
	const  double fracturePressure = 1e6;
	const  double capPressure_H2O_Gas = 0.2e6; // sealPressures
	const  double capPressure_H2O_Oil = 0.22e6;

	// error bound for decimal comparison
	const  double errLimit = 1e-10;
	
};

TEST(LeakGasAndSpillOil_, maxCapacityLevelAndVolume) {
   std::vector<MyTuple> v(2, MyTuple(0, 0));
   v[1][0] = 12.75;
   v[1][1] = 24.5;
	functions::MonotonicIncreasingPiecewiseLinearInvertableFunction c_(v);

	LeakGasAndSpillOil LgSo(gasDensity, oilDensity, sealFluidDensity, overPressureContrast, crestColumnThickness
		, fracturePressure, capPressure_H2O_Gas, capPressure_H2O_Oil, &c_);

	ASSERT_NEAR(LgSo.maxCapacityLevel(), 12.75, errLimit);// this is actually spill capacity
	ASSERT_NEAR(LgSo.maxCapacityVolume(), 24.5, errLimit);// and spill volume
}

TEST(LeakGasAndSpillOil_, distributeWithAllGasLeaking) {
   std::vector<MyTuple> v(2, MyTuple(0, 0));
   v[1][0] = 13.75;
   v[1][1] = 24.5;
   functions::MonotonicIncreasingPiecewiseLinearInvertableFunction c_(v);
	auto capPressure_H2O_Gas_ = 1000.;
	LeakGasAndSpillOil LgSo(gasDensity, oilDensity, sealFluidDensity, overPressureContrast, crestColumnThickness
		, fracturePressure, capPressure_H2O_Gas_, capPressure_H2O_Oil, &c_);
	//Inputs
	double  gasVolume=50;
	double oilVolume=20; 
	//Results from distribute
	double gasVolumeLeaked;
	double gasVolumeSpilled;
	double oilVolumeLeaked; 
	double oilVolumeSpilled;
	//========================
	LgSo.distribute(gasVolume, oilVolume, gasVolumeLeaked, gasVolumeSpilled, oilVolumeLeaked, oilVolumeSpilled);
	
	EXPECT_TRUE(LgSo.maxCapacity()[1] < gasVolume+oilVolume);
	
	EXPECT_NEAR(gasVolumeLeaked , gasVolume, errLimit);// All the Gas is leaked
	EXPECT_NEAR(oilVolumeLeaked , 0, errLimit); // No Oil leakage
	// And no spillage
	ASSERT_TRUE(gasVolumeSpilled == 0);
	ASSERT_TRUE(oilVolumeSpilled == 0);
}

TEST(LeakGasAndSpillOil_, distributeWithOnlyLeaking) {
   std::vector<MyTuple> v(2, MyTuple(0, 0));
   v[1][0] = 12.75;
   v[1][1] = 24.5;
   functions::MonotonicIncreasingPiecewiseLinearInvertableFunction c_(v);
	auto capPressure_H2O_Gas_ = 1000.;
	LeakGasAndSpillOil LgSo(gasDensity, oilDensity, sealFluidDensity, overPressureContrast, crestColumnThickness
		, fracturePressure, capPressure_H2O_Gas_, capPressure_H2O_Oil, &c_);
	//Inputs
	double  gasVolume = 50;
	double oilVolume = 50;
	//Results from distribute
	double gasVolumeLeaked;
	double gasVolumeSpilled;
	double oilVolumeLeaked;
	double oilVolumeSpilled;
	//========================
	LgSo.distribute(gasVolume, oilVolume, gasVolumeLeaked, gasVolumeSpilled, oilVolumeLeaked, oilVolumeSpilled);
	ASSERT_TRUE(gasVolumeSpilled == 0); ASSERT_TRUE(oilVolumeSpilled == 0);
	ASSERT_NEAR(gasVolumeLeaked, gasVolume, errLimit);
	ASSERT_NEAR(oilVolumeLeaked, (oilVolume - v.back()[1]), errLimit);
}

TEST(LeakGasAndSpillOil_, distributeWithOnlySpilling) {
   std::vector<MyTuple> v(2, MyTuple(0, 0));
   v[1][0] = 12.75;
   v[1][1] = 24.5;
   functions::MonotonicIncreasingPiecewiseLinearInvertableFunction c_(v);

	LeakGasAndSpillOil LgSo(gasDensity, oilDensity, sealFluidDensity, overPressureContrast, crestColumnThickness
		, fracturePressure, capPressure_H2O_Gas, capPressure_H2O_Oil, &c_);
	//Inputs
	double  gasVolume = 25;
	double oilVolume = 10;
	//Results from distribute
	double gasVolumeLeaked;
	double gasVolumeSpilled;
	double oilVolumeLeaked;
	double oilVolumeSpilled;
	//========================
	LgSo.distribute(gasVolume, oilVolume, gasVolumeLeaked, gasVolumeSpilled, oilVolumeLeaked, oilVolumeSpilled);
	ASSERT_TRUE(gasVolumeLeaked == 0); ASSERT_TRUE(oilVolumeLeaked == 0);
	ASSERT_NEAR(gasVolumeSpilled, (gasVolume - v.back()[1]), errLimit);
	ASSERT_NEAR(oilVolumeSpilled, (oilVolume), errLimit);
}

// In this case oil will be spilled and gas will be leaked in such a way that the  the max buoyancy is below capacity
TEST(LeakGasAndSpillOil_, distributeWithAll) {
	std::vector<MyTuple> v(2, MyTuple(0, 0));
	v[1][0] = 6;
	v[1][1] = 25.5e4;
	functions::MonotonicIncreasingPiecewiseLinearInvertableFunction c_(v);
	auto capPressure_H2O_Gas_ = 10500.;
	LeakGasAndSpillOil LgSo(gasDensity, oilDensity, sealFluidDensity, overPressureContrast, crestColumnThickness
		, fracturePressure, capPressure_H2O_Gas_, capPressure_H2O_Oil, &c_);
	//Inputs
	double  gasVolume = 5e4;
	double oilVolume = 25e4;
	//Results from distribute
	double gasVolumeLeaked;
	double gasVolumeSpilled;
	double oilVolumeLeaked;
	double oilVolumeSpilled;
	//========================
	LgSo.distribute(gasVolume, oilVolume, gasVolumeLeaked, gasVolumeSpilled, oilVolumeLeaked, oilVolumeSpilled);
	EXPECT_NEAR(oilVolumeSpilled, 44434.601598634094,errLimit);
	EXPECT_NEAR(gasVolumeLeaked, 565.39840136592102,errLimit);
   EXPECT_EQ(gasVolumeSpilled, 0);
   EXPECT_EQ(oilVolumeLeaked, 0);
}




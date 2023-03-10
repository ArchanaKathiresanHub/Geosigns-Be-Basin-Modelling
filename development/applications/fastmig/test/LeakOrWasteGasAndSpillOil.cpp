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
#include<iostream>
#include "../src/LeakOrWasteGasAndSpillOil.h"

using namespace migration;
using namespace distribute;

namespace //  ALL in here is local to this translation unit
{
	//======== Parameters for the Unit Test Test========//
   typedef functions::Tuple2<double> MyTuple;
	// Class specific Parameters that is changed rarely
	const  double gasDensity = 500;
	const  double	oilDensity = 1023;
	const  double	sealFluidDensity = 1100;
	
	// error bound for decimal comparison
	const  double errLimit = 1e-10;

};

TEST(LeakOrWasteGasAndSpillOilTest, distributeForleakGasAndOil) {
   std::vector<MyTuple> v(2, MyTuple(11.75, 23.5));
   v[1][0] = 13.75;
   v[1][1] = 24.5;
   functions::MonotonicIncreasingPiecewiseLinearInvertableFunction c_(v);
	const  double	overPressureContrast = 0;
	const  double crestColumnThickness = 0;
	const  double fracturePressure = 1e6;
	const  double capPressure_H2O_Gas = 1e3; // tight sealPressures
	const  double capPressure_H2O_Oil = 1e3; // tight sealPressures
	const double wasteLevel = 12;
	LeakOrWasteGasAndSpillOil LgWSo(gasDensity, oilDensity, sealFluidDensity, overPressureContrast, crestColumnThickness
		, fracturePressure, capPressure_H2O_Gas, capPressure_H2O_Oil,wasteLevel, &c_);
	//Inputs

	double gasVolume=5;
	double oilVolume=25;
	double gasVolumeLeaked;
	double gasVolumeWasted;
	double gasVolumeSpilled;
	double oilVolumeLeaked;
	double oilVolumeSpilled;
	//========================
	LgWSo.distribute(gasVolume, oilVolume,
		gasVolumeLeaked, gasVolumeWasted, gasVolumeSpilled, oilVolumeLeaked,
		oilVolumeSpilled);
	ASSERT_NEAR(gasVolumeLeaked, gasVolume, errLimit);
	ASSERT_NEAR(oilVolumeLeaked, (oilVolume - v.front()[1]), errLimit);
}


TEST(LeakOrWasteGasAndSpillOilTest, distributeForWasteGasWithNoSpillage) {
   std::vector<MyTuple> v(2, MyTuple(0, 0));
   v[1][0] = 12.75;
   v[1][1] = 24.5;
	const  double capPressure_H2O_Gas = 1e7; // sealPressures
	const  double capPressure_H2O_Oil = 1e7; // sealPressures
	const  double	overPressureContrast = 0;
	const  double crestColumnThickness = 1;
	const  double fracturePressure = 1e6;
	const double wasteLevel= 12;
	functions::MonotonicIncreasingPiecewiseLinearInvertableFunction c_(v);
	LeakOrWasteGasAndSpillOil LgWSo(gasDensity, oilDensity, sealFluidDensity, overPressureContrast, crestColumnThickness
		, fracturePressure, capPressure_H2O_Gas, capPressure_H2O_Oil, wasteLevel, &c_);
	//Inputs
	double gasVolume = 25;
	double oilVolume = 5;
	double gasVolumeLeaked;
	double gasVolumeWasted;
	double gasVolumeSpilled;
	double oilVolumeLeaked;
	double oilVolumeSpilled;

	//========================
	LgWSo.distribute(gasVolume, oilVolume,
		gasVolumeLeaked, gasVolumeWasted, gasVolumeSpilled, oilVolumeLeaked,
		oilVolumeSpilled);
	ASSERT_NEAR(gasVolumeWasted, gasVolume-c_.apply(wasteLevel), errLimit);
}

TEST(LeakOrWasteGasAndSpillOilTest, distributeForGasWasteWithOilSpillage) {
   std::vector<MyTuple> v(2, MyTuple(0, 0));
   v[1][0] = 12.75;
   v[1][1] = 24.5;
   functions::MonotonicIncreasingPiecewiseLinearInvertableFunction c_(v);
	const  double capPressure_H2O_Gas = 1e7; // sealPressures
	const  double capPressure_H2O_Oil = 1e7; // sealPressures
	const  double	overPressureContrast = 0;
	const  double crestColumnThickness = 0;
	const  double fracturePressure = 1e6;
	const double wasteLevel = 1.1;
	LeakOrWasteGasAndSpillOil LgWSo(gasDensity, oilDensity, sealFluidDensity, overPressureContrast, crestColumnThickness
		, fracturePressure, capPressure_H2O_Gas, capPressure_H2O_Oil, wasteLevel, &c_);
	//Inputs
	double gasVolume = 60;
	double oilVolume = 100;
	double gasVolumeLeaked;
	double gasVolumeWasted;
	double gasVolumeSpilled;
	double oilVolumeLeaked;
	double oilVolumeSpilled;
	
	//========================
	LgWSo.distribute(gasVolume, oilVolume,
		gasVolumeLeaked, gasVolumeWasted, gasVolumeSpilled, oilVolumeLeaked,
		oilVolumeSpilled);
	EXPECT_NEAR(gasVolumeWasted, gasVolume - c_.apply(wasteLevel), errLimit);
	EXPECT_NEAR(oilVolumeSpilled,( (oilVolume+c_.apply(wasteLevel))-v.back()[1]),errLimit);// FluidVolume - capacity[1];
}

/* it???s actually not possible to spill gas in this case. The reason is that if you???re calling this class, there is wasting and 
the wasting depth is always shallower than the spilling point (otherwise there wouldn???t be wasting). 
Therefore, all gas that doesn???t fit, will be wasted before it can be spilled
*/


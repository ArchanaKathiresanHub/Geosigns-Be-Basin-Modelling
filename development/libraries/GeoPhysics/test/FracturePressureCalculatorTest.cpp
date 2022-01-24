//
// Copyright (C) 2014-2022 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by CGI Pvt. Ltd.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "../src/FracturePressureCalculator.h"
#include "../src/GeoPhysicsObjectFactory.h"
#include "FracturePressureFunctionParameters.h"


// google test
#include <gtest/gtest.h>

using namespace GeoPhysics;

class FracturePressureCalculationMethods : public testing::Test
{
public:
	FracturePressureCalculationMethods(){
	}
	~FracturePressureCalculationMethods(){
	}

	std::unique_ptr<ProjectHandle> data;
	const double depth = 1500.0;//m
	const double seaBottomDepth = 0.0;
	const double hydrostaticPressure = 15.0e6;//Pa
	const double lithostaticPressure = 30.0e6;//Pa
	const double pressureAtSeaBottom = 101325.0;//Pa
	const double sea_Temperature = 20.0;//degreeC
	const double porePressure = 16.0e6;
protected:
	void resetProjectHandle(std::string& fileName, ObjectFactory* undefinedFactory) {
		data.reset(dynamic_cast<ProjectHandle*>(OpenCauldronProject(fileName, undefinedFactory)));
	}
};

//Test 1: When FracturePressure is calculated as a function of depth
TEST_F(FracturePressureCalculationMethods, FunctionOfDepth)
{
	//Arrange
	ObjectFactory factory;
	std::string testProjectFileName = "FracturePressureFuncOfDepth.project3d";
	resetProjectHandle(testProjectFileName, &factory);
	FracturePressureCalculator myFracturePress(*data);
	CompoundLithology myTestLitho(*data);
	database::Table* fluidTypeIo_tbl(data->getTable("FluidtypeIoTbl"));
	database::Record* rec(fluidTypeIo_tbl->getRecord(static_cast<int>(0)));
	FluidType myTestFluid(*data, rec);

	//Act...
	double calculatedFracPress1 = myFracturePress.fracturePressure(&myTestLitho, depth, seaBottomDepth, hydrostaticPressure, lithostaticPressure, pressureAtSeaBottom);
	double calculatedFracPress2 = myFracturePress.fracturePressure(&myTestLitho, &myTestFluid, sea_Temperature, seaBottomDepth, depth, hydrostaticPressure, lithostaticPressure);

	bool hasNodefractured = myFracturePress.hasFractured(&myTestLitho, &myTestFluid, sea_Temperature, seaBottomDepth, depth, hydrostaticPressure, lithostaticPressure, porePressure);
	
	//Assert...
	EXPECT_DOUBLE_EQ(calculatedFracPress1, 15.0e6);
	EXPECT_DOUBLE_EQ(calculatedFracPress2, 15.0e6);
	EXPECT_DOUBLE_EQ(calculatedFracPress1, calculatedFracPress2);
	EXPECT_EQ(hasNodefractured, true);

}

//Test 2: When FracturePressure is calculated as a function of Lithostatic pressure
TEST_F(FracturePressureCalculationMethods, FunctionOfLithostaticPressure)
{
	//Arrange
	ObjectFactory factory;
	std::string testProjectFileName = "FracturePressureFuncOfLithostaticPress.project3d";
	resetProjectHandle(testProjectFileName, &factory);
	 
	FracturePressureCalculator myFracturePress(*data);
	CompoundLithology myTestLitho(*data);
	database::Table* fluidTypeIo_tbl(data->getTable("FluidtypeIoTbl"));
	database::Record* rec(fluidTypeIo_tbl->getRecord(static_cast<int>(0)));
	FluidType myTestFluid(*data, rec);
	FracturePressureCalculator myFracturePress1(*data);

	//Act
	double calculatedFracPress1 = myFracturePress.fracturePressure(&myTestLitho, depth, seaBottomDepth, hydrostaticPressure, lithostaticPressure, pressureAtSeaBottom);
	double calculatedFracPress2 = myFracturePress.fracturePressure(&myTestLitho, &myTestFluid, sea_Temperature, seaBottomDepth, depth, hydrostaticPressure, lithostaticPressure);

	//Assert...
	EXPECT_DOUBLE_EQ(calculatedFracPress1, 30.0e6);
	EXPECT_DOUBLE_EQ(calculatedFracPress2, 30.0e6);
	EXPECT_DOUBLE_EQ(calculatedFracPress1, calculatedFracPress2);

}
TEST_F(FracturePressureCalculationMethods, NoFracturePressure)
{
	//Arrange
	ObjectFactory factory;
	std::string testProjectFileName = "FracturePressureNoFracPress.project3d";
	resetProjectHandle(testProjectFileName, &factory);

	FracturePressureCalculator myFracturePress(*data);
	CompoundLithology myTestLitho(*data);
	database::Table* fluidTypeIo_tbl(data->getTable("FluidtypeIoTbl"));
	database::Record* rec(fluidTypeIo_tbl->getRecord(static_cast<int>(0)));
	FluidType myTestFluid(*data, rec);
	FracturePressureCalculator myFracturePress1(*data);

	//Act
	double calculatedFracPress1 = myFracturePress.fracturePressure(&myTestLitho, depth, seaBottomDepth, hydrostaticPressure, lithostaticPressure, pressureAtSeaBottom);
	double calculatedFracPress2 = myFracturePress.fracturePressure(&myTestLitho, &myTestFluid, sea_Temperature, seaBottomDepth, depth, hydrostaticPressure, lithostaticPressure);

	//Assert...
	EXPECT_DOUBLE_EQ(calculatedFracPress1, NoFracturePressureValue);
	EXPECT_DOUBLE_EQ(calculatedFracPress2, NoFracturePressureValue);
	EXPECT_DOUBLE_EQ(calculatedFracPress1, calculatedFracPress2);
}




//
// Copyright ( C ) 2013-2019 Shell International Exploration & Production.
// All rights reserved.
//
// Developed by CGI Information Systems & Management Consultants Pvt Ltd, India under license for Shell.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include <iostream>
using namespace std;

#include <gtest/gtest.h>

#include "../src/GenexSourceRock.h"

// Measured HI Values
std::vector<std::pair<double, double>> myVectorOfMaps
{
	std::make_pair(747.0, 1.45) 
	,std::make_pair(656.0, 1.350)
	,std::make_pair(471.0, 1.250)
	,std::make_pair(361.0, 1.200)
	,std::make_pair(94.0, 0.801)
	,std::make_pair(823.0, 1.560)
	,std::make_pair(656.0, 1.350)
	,std::make_pair(444.0, 1.240)
	,std::make_pair(281.0, 1.130)
	,std::make_pair(210.0, 1.030)
};

TEST(HiToHcForAllMeasuredHI, SimpleTest)
{
	for (auto myPair : myVectorOfMaps)
	{
		EXPECT_NEAR(Genex6::GenexSourceRock::convertHItoHC(myPair.first), myPair.second,0.0001);
	}
}

// Putting the other way conversion here too just in case 
TEST(HcToHiForAllMeasuredHC, SimpleTest2)
{
	for (auto myPair : myVectorOfMaps)
	{
		EXPECT_NEAR(Genex6::GenexSourceRock::convertHCtoHI(myPair.second), myPair.first,2);
	}
}
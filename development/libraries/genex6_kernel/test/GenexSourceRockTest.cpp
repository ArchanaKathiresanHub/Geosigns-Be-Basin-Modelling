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
#include "../../TableIO/src/ProjectFileHandler.h"


TEST(checkTargetHCValues, validateHCValue)
{
	using namespace database;
	ProjectFileHandlerPtr Pptr;
	string name = "mtTest";
	DataAccess::Interface::ProjectHandle projectHandle(Pptr,name,nullptr);
	
	
	Genex6::GenexSourceRock sr(projectHandle,nullptr);
	double minHc = 1.26;
	double	maxHc = 1.335;
	double RasterMap[][2] = {	{1.25,1.33},
								{1.45,1.299}
							};
	double maxValue = 0;
	int count = 0;
	
	for (auto& row : RasterMap) {
		for (auto col : row) {
			auto ind = sr.checkTargetHC(minHc, maxHc, col, maxValue, count);
			if(ind == CURT_MIN)
				EXPECT_NEAR(col,minHc,0.0001);
			if (ind == CURT_MAX)
				EXPECT_NEAR(col, maxHc, 0.0001);
		}
	}
	EXPECT_EQ(count, 2);
	
	EXPECT_NEAR(maxValue, 0.11499,0.0001);

}


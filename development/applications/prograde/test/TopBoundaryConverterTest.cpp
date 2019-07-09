//
// Copyright (C) 2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell .
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

//
#include <gtest/gtest.h>


#include "../src/TopBoundaryUpgradeManager.h"

// test surface age clipping, using the interval logic
TEST(TopBoundaryConditions, surfaceAgeInterval)
{
	
	std::vector<double> ageSequence = { 1, 2, 3, 999, 1000 };
	Prograde::TopBoundaryUpgradeManager::clipAndBufferSurfaceAges(ageSequence);
	double lastClippedAge = ageSequence.back();
	double secondLastClippedAge = ageSequence[ageSequence.size() - 2];
	EXPECT_NEAR(lastClippedAge, 999.0, 1e-6);
	EXPECT_NEAR(secondLastClippedAge, 998.5, 1e-6);
}

// test surface age clipping using single value between 998 and 999 
TEST(TopBoundaryConditions, surfaceAgeSingle)
{
	std::vector<double> ageSequence = { 1, 2, 3, 998.6 };
	Prograde::TopBoundaryUpgradeManager::clipAndBufferSurfaceAges(ageSequence);
	double lastClippedAge = ageSequence.back();
	
	EXPECT_NEAR(lastClippedAge, 998.6, 1e-6);
	
}

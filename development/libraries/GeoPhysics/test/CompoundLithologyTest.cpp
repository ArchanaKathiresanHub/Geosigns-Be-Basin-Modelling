//
// Copyright (C) 2014-2021 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by CGI Pvt. Ltd.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// Gtest headers
#include <gtest/gtest.h>
#include <gmock/gmock.h>

// Other custom headers
#include "../src/CompoundLithology.h"
#include "../src/GeoPhysicsProjectHandle.h"
#include "../src/GeoPhysicsObjectFactory.h"

using namespace GeoPhysics;
//using namespace database;

namespace CompoundLithology_UnitTests
{

	class CompoundLithologytestFamily : public testing::Test
	{
	public:
		CompoundLithologytestFamily()
		{
			CreateProjectHandle();
		}
		~CompoundLithologytestFamily()
		{
		}

		std::unique_ptr<ProjectHandle> data;
		const double topMaxVes = 2.5e6; // Pa
		const double botMaxVes = 2.6e6;
		const double densityDifference = 1250.0;
		const double solidThickness = 350.0;
	private:
		void CreateProjectHandle()
		{
			//Undefined model
			ObjectFactory factory;
			data.reset(dynamic_cast<ProjectHandle*>(OpenCauldronProject("ClaudiusDecompactionTest.project3d", &factory)));
		}
	};

	TEST_F(CompoundLithologytestFamily, computeSegmentThickness)
	{
		CompoundLithology myLitho(*data);
		myLitho.addLithology(nullptr, 100);
		
		myLitho.reCalcProperties();

		database::Table* runOptionsIo_tbl(data->getTable("RunOptionsIoTbl"));
		database::Record* rec(runOptionsIo_tbl->getRecord(static_cast<int>(0)));
		int legacy = rec->getValue<int>("Legacy");

		// BPA2 p3d
		EXPECT_EQ(legacy, 0);
		//Checking BPA2 segment thickness when (densityDifference > 0)
		auto checkThick = myLitho.computeSegmentThickness(topMaxVes,
			botMaxVes, densityDifference, solidThickness
		);
		EXPECT_DOUBLE_EQ(checkThick, 14.621605752588959);
		//Checking BPA2 segment thickness when (densityDifference == 0.0)
		// Should we also check value < 0 ??
		checkThick = myLitho.computeSegmentThickness(topMaxVes,
			botMaxVes, 0.0, solidThickness
		);
		EXPECT_DOUBLE_EQ(checkThick, 627.5443925591245);

		// BPA p3d
		//Checking BPA segment thickness when (densityDifference > 0) 
		// change the p3 entry for the parameter. 
		rec->setValue<int>("Legacy", 1);
		legacy = rec->getValue<int>("Legacy");
		//
		// to check is indeed the value was changed correctly.
		EXPECT_EQ(legacy, 1);
		//
		checkThick = myLitho.computeSegmentThickness(topMaxVes,
			botMaxVes, densityDifference, solidThickness
		);
		EXPECT_DOUBLE_EQ(checkThick, 14.618118050505961);

	}
  
}

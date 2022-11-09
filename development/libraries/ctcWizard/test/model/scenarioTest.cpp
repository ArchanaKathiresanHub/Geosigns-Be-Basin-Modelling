//                                                                      
// Copyright (C) 2018-2021 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by CGI India Pvt. Ltd.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//


#include "gtest/gtest.h"
#include "../../src/model/ctcScenario.h"

#include <QStringList>

// Test Fixture SetUp
namespace ctcWizard {
	class TestCTCWizard : public ::testing::Test {
	public:
		CtcScenario myCTCScenario;

	protected:
		virtual void SetUp() {
		}

		virtual void TearDown() {
		}
	};
}

using namespace ctcWizard;

// Unit Test for the CtcScenario::addLithosphereParameter and CtcScenario::clearLithosphereTable
TEST_F(TestCTCWizard, Add_Delete_Lithosphere_Parameter){	
	QVector<LithosphereParameter> myLithoParameter = myCTCScenario.lithosphereParameters();
	EXPECT_EQ(3, myLithoParameter.size());

	// Adding an element to the lithosphere table
	myCTCScenario.addLithosphereParameter("Dummy Parameter for Thickness [m]", "123.0");
	myLithoParameter = myCTCScenario.lithosphereParameters();
	EXPECT_EQ(4, myLithoParameter.size());

	EXPECT_EQ("Initial Crust Thickness [m]", myLithoParameter[0].param.toStdString());
	EXPECT_EQ("Initial Mantle Thickness [m]", myLithoParameter[1].param.toStdString());
	EXPECT_EQ("Smoothing Radius [number of cells]", myLithoParameter[2].param.toStdString());
	EXPECT_EQ("Dummy Parameter for Thickness [m]", myLithoParameter[3].param.toStdString());

	for (int i = 0; i <= 2; ++i) {
		EXPECT_EQ("", myLithoParameter[i].value.toStdString());
	}
	EXPECT_EQ("123.0", myLithoParameter[3].value.toStdString());

	// Changing some values of the Lithosphere Table
	myCTCScenario.setLithosphereParameter(2, 0, "Changed text Smoothing Radius");
	myCTCScenario.setLithosphereParameter(0, 1, "5000");
	myLithoParameter = myCTCScenario.lithosphereParameters();
	EXPECT_EQ("Changed text Smoothing Radius", myLithoParameter[2].param.toStdString());
	EXPECT_EQ("5000", myLithoParameter[0].value.toStdString());

	// Clearing and checking Lithosphere Table
	myCTCScenario.clearLithosphereTable();
	myLithoParameter = myCTCScenario.lithosphereParameters();
	EXPECT_EQ(0, myLithoParameter.size());
}

// Testing Set and Get for runMode
TEST_F(TestCTCWizard, Set_Get_RunMode) {
	auto myRunMode = myCTCScenario.runMode().toStdString();
	EXPECT_EQ("Decompaction", myRunMode);

	myCTCScenario.setrunMode("TestRunMode");
	myRunMode = myCTCScenario.runMode().toStdString();
	EXPECT_EQ("TestRunMode", myRunMode);
}

// Testing Set and Get for numProc
TEST_F(TestCTCWizard, Set_Get_NumProc) {
	auto myNumProc = myCTCScenario.numProc().toStdString();
	EXPECT_EQ("1", myNumProc);

	myCTCScenario.setnumProc("7");
	myNumProc = myCTCScenario.numProc().toStdString();
	EXPECT_EQ("7", myNumProc);
}

// Testing Set and Get for Project3dPath
TEST_F(TestCTCWizard, Set_Get_Project3d_File_Path) {
	auto myProject3dPath = myCTCScenario.project3dPath().toStdString();
	EXPECT_EQ("", myProject3dPath);

	myCTCScenario.setProject3dPath("TestProject3dPath");
	myProject3dPath = myCTCScenario.project3dPath().toStdString();
	EXPECT_EQ("TestProject3dPath", myProject3dPath);
}

// Testing Set and Get for CTCFilePath
TEST_F(TestCTCWizard, Set_Get_CTC_File_Path) {
	auto myCTCPath = myCTCScenario.ctcFilePathCTC().toStdString();
	EXPECT_EQ("", myCTCPath);

	myCTCScenario.setCtcFilePathCTC("TestCTCFilePath");
	myCTCPath = myCTCScenario.ctcFilePathCTC().toStdString();
	EXPECT_EQ("TestCTCFilePath", myCTCPath);
}

// Testing Rifting History table
TEST_F(TestCTCWizard, Rifting_History) {
	auto rhBasaltMaps = myCTCScenario.riftingHistoryBasaltMaps();
	EXPECT_EQ(1, rhBasaltMaps.size());
	EXPECT_EQ("", rhBasaltMaps[0].toStdString());

	auto rhRDAMaps = myCTCScenario.riftingHistoryRDAMaps();
	EXPECT_EQ(1, rhRDAMaps.size());
	EXPECT_EQ("", rhRDAMaps[0].toStdString());

	auto rhOptions = myCTCScenario.riftingHistoryOptions();
	EXPECT_EQ(3, rhOptions.size());
	EXPECT_EQ("Active Rifting", rhOptions[0].toStdString());
	EXPECT_EQ("Passive Margin", rhOptions[1].toStdString());
	EXPECT_EQ("Flexural Basin", rhOptions[2].toStdString());

	// Checking default Rifting History data
	auto rhInfo = myCTCScenario.riftingHistory();
	EXPECT_EQ(0, rhInfo.size());

	// Adding Rifting History data
	myCTCScenario.addRiftingHistory("25", "Y", "TestTectonicFlag", "1.25", "TestRDAMap.HDF", "10000.0", "TestBasaltMap.HDF");
	myCTCScenario.addRiftingHistory("55", "N", "TestTectonicFlagNew", "2.25", "TestRDAMapNew.HDF", "5000.0", "TestBasaltMapNew.HDF");

	// Checking if Rifting History data is successfully added
	rhInfo = myCTCScenario.riftingHistory();
	EXPECT_EQ(2, rhInfo.size());
	EXPECT_EQ("25", rhInfo[0].Age.toStdString());
	EXPECT_EQ("Y", rhInfo[0].HasPWD.toStdString());
	EXPECT_EQ("TestTectonicFlag", rhInfo[0].TectonicFlag.toStdString());
	EXPECT_EQ("1.25", rhInfo[0].RDA.toStdString());
	EXPECT_EQ("TestRDAMap.HDF", rhInfo[0].RDA_Map.toStdString());
	EXPECT_EQ("10000.0", rhInfo[0].Basalt_Thickness.toStdString());
	EXPECT_EQ("TestBasaltMap.HDF", rhInfo[0].Basalt_Thickness_Map.toStdString());

	EXPECT_EQ("55", rhInfo[1].Age.toStdString());
	EXPECT_EQ("N", rhInfo[1].HasPWD.toStdString());
	EXPECT_EQ("TestTectonicFlagNew", rhInfo[1].TectonicFlag.toStdString());
	EXPECT_EQ("2.25", rhInfo[1].RDA.toStdString());
	EXPECT_EQ("TestRDAMapNew.HDF", rhInfo[1].RDA_Map.toStdString());
	EXPECT_EQ("5000.0", rhInfo[1].Basalt_Thickness.toStdString());
	EXPECT_EQ("TestBasaltMapNew.HDF", rhInfo[1].Basalt_Thickness_Map.toStdString());

	// Changing a few values of the Rifting History table
	myCTCScenario.setRiftingHistory(0, 0, "10");
	myCTCScenario.setRiftingHistory(0, 1, "N");
	myCTCScenario.setRiftingHistory(0, 4, "ChangedRDAMap.HDF");
	myCTCScenario.setRiftingHistory(0, 5, "7000");
	myCTCScenario.setRiftingHistory(1, 2, "NewTestStr");
	myCTCScenario.setRiftingHistory(1, 3, "1.75");
	myCTCScenario.setRiftingHistory(1, 6, "ChangedBasaltMap.HDF");

	// Checking for the changed values
	rhInfo = myCTCScenario.riftingHistory();
	EXPECT_EQ("10", rhInfo[0].Age.toStdString());
	EXPECT_EQ("N", rhInfo[0].HasPWD.toStdString());
	EXPECT_EQ("ChangedRDAMap.HDF", rhInfo[0].RDA_Map.toStdString());
	EXPECT_EQ("7000", rhInfo[0].Basalt_Thickness.toStdString());
	EXPECT_EQ("NewTestStr", rhInfo[1].TectonicFlag.toStdString());
	EXPECT_EQ("1.75", rhInfo[1].RDA.toStdString());
	EXPECT_EQ("ChangedBasaltMap.HDF", rhInfo[1].Basalt_Thickness_Map.toStdString());


	// Clearing Rifting History table
	myCTCScenario.clearRiftingHistoryTable();
	rhInfo = myCTCScenario.riftingHistory();
	EXPECT_EQ(0, rhInfo.size());
}

//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "../src/ScenarioAnalysis.h"

#include <gtest/gtest.h>

TEST( ScenarioAnalysisTest, testBaseProjectName )
{
   // create new scenario analysis
   casa::ScenarioAnalysis sc;
   std::string nameWithNoPath = "Project.project3d";

   {
      sc.defineBaseCase(nameWithNoPath.c_str());
      std::string name = sc.baseCaseProjectFileName();
      EXPECT_EQ(nameWithNoPath, name);
   }

   {
      std::string nameWithWindowsPath = "a\\b\\c\\Project.project3d";
      sc.defineBaseCase(nameWithWindowsPath.c_str());
      std::string name = sc.baseCaseProjectFileName();
      EXPECT_EQ(nameWithNoPath, name);
   }

   {
      std::string nameWithLinuxPath = "a/b/c/Project.project3d";
      sc.defineBaseCase(nameWithLinuxPath.c_str());
      std::string name = sc.baseCaseProjectFileName();
      EXPECT_EQ(nameWithNoPath, name);
   }

}

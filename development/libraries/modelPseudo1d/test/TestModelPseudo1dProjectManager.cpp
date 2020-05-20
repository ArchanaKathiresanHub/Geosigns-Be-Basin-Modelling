#include "ModelPseudo1dProjectManager.h"

#include <fstream>
#include <iostream>
#include <stdio.h>

#include <gtest/gtest.h>

TEST(TestModelPseudo1dProjectManager, TestObjectInstantiation)
{
  ASSERT_NO_THROW(modelPseudo1d::ModelPseudo1dProjectManager projMgr("Project.project3d",""));
}

TEST(TestModelPseudo1dProjectManager, TestOutputCreation)
{
  std::ifstream ifile;
  ifile.open("out.project3d");
  if(ifile)
  {
    ifile.close();
    EXPECT_EQ(remove("out.project3d"), 0);
  }

  std::unique_ptr<modelPseudo1d::ModelPseudo1dProjectManager> projMgr = nullptr;
  ASSERT_NO_THROW(projMgr.reset(new modelPseudo1d::ModelPseudo1dProjectManager("Project.project3d","out.project3d")));
  EXPECT_NO_THROW(projMgr->save());

  ifile.open("out.project3d");
  std::string fileCreated = "File has not been created successfully";
  if(ifile)
  {
    ifile.close();
    fileCreated = "File has been created successfully";
  }
  EXPECT_EQ(fileCreated, "File has been created successfully");
}

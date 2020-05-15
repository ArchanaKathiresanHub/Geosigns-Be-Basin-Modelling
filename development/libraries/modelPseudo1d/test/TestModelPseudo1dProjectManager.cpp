#include "ModelPseudo1dProjectManager.h"

#include <gtest/gtest.h>

TEST(TestModelPseudo1dProjectManager, TestObjectInstantiation)
{
  ASSERT_NO_THROW(modelPseudo1d::ModelPseudo1dProjectManager projMgr("Project.project3d",""));
}

TEST(TestModelPseudo1dProjectManager, TestOutputCreation)
{
  std::unique_ptr<modelPseudo1d::ModelPseudo1dProjectManager> projMgr = nullptr;
  ASSERT_NO_THROW(projMgr.reset(new modelPseudo1d::ModelPseudo1dProjectManager("Project.project3d","out.project3d")));
  EXPECT_NO_THROW(projMgr->save());
}

#include "ModelPseudo1dCommonDefinitions.h"

#include "ModelPseudo1d.h"

#include <algorithm>
#include <gtest/gtest.h>

TEST(TestModelPseudo1d, TestObjectInstantiation)
{
  mbapi::Model testModel;
  ASSERT_EQ(ErrorHandler::NoError, testModel.loadModelFromProjectFile("Project.project3d"));

  ASSERT_NO_THROW(modelPseudo1d::ModelPseudo1d mdlPseudo1d(testModel,0,0));
}

TEST(TestModelPseudo1d, TestCheckInitialize)
{
  mbapi::Model testModel;
  ASSERT_EQ(ErrorHandler::NoError, testModel.loadModelFromProjectFile("Project.project3d"));

  modelPseudo1d::ModelPseudo1d mdlPseudo1d(testModel,0,0);

  EXPECT_NO_THROW(mdlPseudo1d.initialize());
}

TEST(TestModelPseudo1d, TestCheckNullModel)
{
  mbapi::Model testModel;

  modelPseudo1d::ModelPseudo1d mdlPseudo1d(testModel,0,0);

  EXPECT_THROW(mdlPseudo1d.initialize(), modelPseudo1d::ModelPseudo1dException);
}

TEST(TestModelPseudo1d, TestExtractScalarsFromInputMaps)
{
  mbapi::Model testModel;
  ASSERT_EQ(ErrorHandler::NoError, testModel.loadModelFromProjectFile("Project.project3d"));

  modelPseudo1d::ModelPseudo1d mdlPseudo1d(testModel,171000,613500);
  mdlPseudo1d.initialize();

  EXPECT_NO_THROW(mdlPseudo1d.extractScalarsFromInputMaps());
}

TEST(TestModelPseudo1d, TestSetScalarsInModel)
{
  mbapi::Model testModel;
  ASSERT_EQ(ErrorHandler::NoError, testModel.loadModelFromProjectFile("Project.project3d"));

  modelPseudo1d::ModelPseudo1d mdlPseudo1d(testModel,181000,600500);
  mdlPseudo1d.initialize();
  mdlPseudo1d.extractScalarsFromInputMaps();

  EXPECT_NO_THROW(mdlPseudo1d.setScalarsInModel());
  EXPECT_EQ(testModel.tableSize("GridMapIoTbl"), 0); // All maps references should be gone
}

TEST(TestModelPseudo1d, TestSetSingleCellwindowXY)
{
  mbapi::Model testModel;
  ASSERT_EQ(ErrorHandler::NoError, testModel.loadModelFromProjectFile("Project.project3d"));

  modelPseudo1d::ModelPseudo1d mdlPseudo1d(testModel,181000,600500);

  EXPECT_NO_THROW(mdlPseudo1d.setSingleCellWindowXY());
}

TEST(TestModelPseudo1d, TestRightXYIndices)
{
  mbapi::Model testModel;
  ASSERT_EQ(ErrorHandler::NoError, testModel.loadModelFromProjectFile("Project.project3d"));

  modelPseudo1d::ModelPseudo1d mdlPseudo1d(testModel,181000,613500);

  EXPECT_NO_THROW(mdlPseudo1d.initialize());
  EXPECT_NO_THROW(mdlPseudo1d.extractScalarsFromInputMaps());
  EXPECT_NO_THROW(mdlPseudo1d.setScalarsInModel());
  EXPECT_NO_THROW(mdlPseudo1d.setSingleCellWindowXY());

  const unsigned int indIExpected = 8;
  const unsigned int indJExpected = 40;

  const unsigned int indIActual = mdlPseudo1d.indI();
  const unsigned int indJActual = mdlPseudo1d.indJ();

  EXPECT_EQ(indIExpected, indIActual);
  EXPECT_EQ(indJExpected, indJActual);
}



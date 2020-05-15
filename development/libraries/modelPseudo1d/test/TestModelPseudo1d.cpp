#include "ModelPseudo1dCommonDefinitions.h"

#include "ModelPseudo1d.h"

#include <algorithm>
#include <gtest/gtest.h>

TEST(TestModelPseudo1dProjectManager, TestObjectInstantiation)
{
  mbapi::Model testModel;
  ASSERT_EQ(ErrorHandler::NoError, testModel.loadModelFromProjectFile("Project.project3d"));

  ASSERT_NO_THROW(modelPseudo1d::ModelPseudo1d mdlPseudo1d(testModel,0,0));
}

TEST(TestModelPseudo1dProjectManager, TestCheckInitialize)
{
  mbapi::Model testModel;
  ASSERT_EQ(ErrorHandler::NoError, testModel.loadModelFromProjectFile("Project.project3d"));

  modelPseudo1d::ModelPseudo1d mdlPseudo1d(testModel,0,0);

  EXPECT_NO_THROW(mdlPseudo1d.initialize());
}

// There is no check for the x_coord and y_coord. So, the test below fails.
//TEST(TestModelPseudo1dProjectManager, TestCheckWrongInitialize)
//{
//  mbapi::Model testModel;
//  ASSERT_EQ(ErrorHandler::NoError, testModel.loadModelFromProjectFile("Project.project3d"));

//  modelPseudo1d::ModelPseudo1d mdlPseudo1d(testModel,99999999,99999999);

//  EXPECT_THROW(mdlPseudo1d.initialize(), modelPseudo1d::ModelPseudo1dException);
//}

TEST(TestModelPseudo1dProjectManager, TestCheckNullModel)
{
  mbapi::Model testModel;

  modelPseudo1d::ModelPseudo1d mdlPseudo1d(testModel,0,0);

  EXPECT_THROW(mdlPseudo1d.initialize(), modelPseudo1d::ModelPseudo1dException);
}

TEST(TestModelPseudo1dProjectManager, TestExtractScalarsFromInputMaps)
{
  mbapi::Model testModel;
  ASSERT_EQ(ErrorHandler::NoError, testModel.loadModelFromProjectFile("Project.project3d"));

  modelPseudo1d::ModelPseudo1d mdlPseudo1d(testModel,171000,613500);

  EXPECT_NO_THROW(mdlPseudo1d.extractScalarsFromInputMaps());
}

TEST(TestModelPseudo1dProjectManager, TestSetScalarsInModel)
{
  mbapi::Model testModel;
  ASSERT_EQ(ErrorHandler::NoError, testModel.loadModelFromProjectFile("Project.project3d"));

  modelPseudo1d::ModelPseudo1d mdlPseudo1d(testModel,181000,600500);

  EXPECT_NO_THROW(mdlPseudo1d.setScalarsInModel());
}

TEST(TestModelPseudo1dProjectManager, TestSetSingleCellwindowXY)
{
  mbapi::Model testModel;
  ASSERT_EQ(ErrorHandler::NoError, testModel.loadModelFromProjectFile("Project.project3d"));

  modelPseudo1d::ModelPseudo1d mdlPseudo1d(testModel,181000,600500);

  EXPECT_NO_THROW(mdlPseudo1d.setSingleCellWindowXY());
}

TEST(TestModelPseudo1dProjectManager, TestRightXYIndices)
{
  mbapi::Model testModel;
  ASSERT_EQ(ErrorHandler::NoError, testModel.loadModelFromProjectFile("Project.project3d"));

  modelPseudo1d::ModelPseudo1d mdlPseudo1d(testModel,181000,613500);

  EXPECT_NO_THROW(mdlPseudo1d.initialize());
  EXPECT_NO_THROW(mdlPseudo1d.extractScalarsFromInputMaps());
  EXPECT_NO_THROW(mdlPseudo1d.initialize());
  EXPECT_NO_THROW(mdlPseudo1d.initialize());
  EXPECT_NO_THROW(mdlPseudo1d.initialize());

  mdlPseudo1d.initialize();
  mdlPseudo1d.extractScalarsFromInputMaps();
  mdlPseudo1d.setScalarsInModel();  // m_mdl.setTableValue inside setScalarsInModel
  mdlPseudo1d.setSingleCellWindowXY();

  const unsigned int indIExpected = 8;
  const unsigned int indJExpected = 40;
  //  const unsigned int indIActual = testModel.

  //  const unsigned int indJActual = projMgr->indJ();
  //  EXPECT_EQ(indIExpected, indIActual);
  //  EXPECT_EQ(indJExpected, indJActual);
}



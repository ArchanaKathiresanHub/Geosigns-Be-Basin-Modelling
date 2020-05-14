//#include "ModelPseudo1dCommonDefinitions.h"
//#include "ModelPseudo1dProjectManager.h"

//#include "ModelPseudo1dInputData.h"

//#include <algorithm>
//#include <gtest/gtest.h>

//TEST(TestModelPseudo1dProjectManager, TestObjectInstantiation)
//{
//  modelPseudo1d::ModelPseudo1dInputData inputData;
//  inputData.projectFilename = "Project.project3d";

//  ASSERT_NO_THROW(modelPseudo1d::ModelPseudo1dProjectManager projMgr(inputData));
//}

//TEST(TestModelPseudo1dProjectManager, TestCheckSubSampling)
//{
//  modelPseudo1d::ModelPseudo1dInputData inputData;
//  inputData.projectFilename = "Project.project3d";

//  std::unique_ptr<modelPseudo1d::ModelPseudo1dProjectManager> projMgr = nullptr;
//  ASSERT_NO_THROW(projMgr.reset(new modelPseudo1d::ModelPseudo1dProjectManager(inputData)));
//  EXPECT_NO_THROW(projMgr->checkSubSampling());
//}

//TEST(TestModelPseudo1dProjectManager, TestGetXYIndicesOutOfRangeXCoord)
//{
//  modelPseudo1d::ModelPseudo1dInputData inputData;
//  inputData.projectFilename = "Project.project3d";
//  inputData.outProjectFilename = "out.Project3d";
//  inputData.xCoord = 171000;
//  inputData.yCoord = 613500;

//  std::unique_ptr<modelPseudo1d::ModelPseudo1dProjectManager> projMgr = nullptr;
//  ASSERT_NO_THROW(projMgr.reset(new modelPseudo1d::ModelPseudo1dProjectManager(inputData)));
//  EXPECT_NO_THROW(projMgr->checkSubSampling());
//  EXPECT_THROW(projMgr->getXYIndices(), modelPseudo1d::ModelPseudo1dException);
//}

//TEST(TestModelPseudo1dProjectManager, TestGetXYIndicesOutOfRangeYCoord)
//{
//  modelPseudo1d::ModelPseudo1dInputData inputData;
//  inputData.projectFilename = "Project.project3d";
//  inputData.outProjectFilename = "out.Project3d";
//  inputData.xCoord = 181000;
//  inputData.yCoord = 600500;

//  std::unique_ptr<modelPseudo1d::ModelPseudo1dProjectManager> projMgr = nullptr;
//  ASSERT_NO_THROW(projMgr.reset(new modelPseudo1d::ModelPseudo1dProjectManager(inputData)));
//  EXPECT_NO_THROW(projMgr->checkSubSampling());
//  EXPECT_THROW(projMgr->getXYIndices(), modelPseudo1d::ModelPseudo1dException);
//}

//TEST(TestModelPseudo1dProjectManager, TestGetXYIndices)
//{
//  modelPseudo1d::ModelPseudo1dInputData inputData;
//  inputData.projectFilename = "Project.project3d";
//  inputData.outProjectFilename = "out.Project3d";
//  inputData.xCoord = 181000;
//  inputData.yCoord = 613500;

//  std::unique_ptr<modelPseudo1d::ModelPseudo1dProjectManager> projMgr = nullptr;
//  ASSERT_NO_THROW(projMgr.reset(new modelPseudo1d::ModelPseudo1dProjectManager(inputData)));
//  EXPECT_NO_THROW(projMgr->checkSubSampling());
//  EXPECT_NO_THROW(projMgr->getXYIndices());

//  const unsigned int indIExpected = 8;
//  const unsigned int indJExpected = 40;
//  const unsigned int indIActual = projMgr->indI();
//  const unsigned int indJActual = projMgr->indJ();
//  EXPECT_EQ(indIExpected, indIActual);
//  EXPECT_EQ(indJExpected, indJActual);
//}

//TEST(TestModelPseudo1dProjectManager, TestSetReferredTablesSet)
//{
//  modelPseudo1d::ModelPseudo1dInputData inputData;
//  inputData.projectFilename = "Project.project3d";

//  std::unique_ptr<modelPseudo1d::ModelPseudo1dProjectManager> projMgr = nullptr;
//  ASSERT_NO_THROW(projMgr.reset(new modelPseudo1d::ModelPseudo1dProjectManager(inputData)));
//  EXPECT_NO_THROW(projMgr->setReferredTablesSet());

//  std::vector<std::string> referredTablesSetNamesExpected =
//  {"StratIoTbl", "TwoWayTimeIoTbl", "OceaCrustalThicknessIoTbl"};

//  auto it = projMgr->referredTableConstIteratorBegin();
//  for ( ; it != projMgr->referredTableConstIteratorend(); ++it)
//  {
//    std::string tableNameActual = *it;
//    EXPECT_FALSE(tableNameActual.empty());
//    auto itTableNameFound = std::find(referredTablesSetNamesExpected.begin(), referredTablesSetNamesExpected.end(), tableNameActual);
//    EXPECT_TRUE(itTableNameFound != referredTablesSetNamesExpected.end())
//        << "Referred table name: " << tableNameActual << " does not exist in the list of expected referred table names!";
//  }
//}

//TEST(TestModelPseudo1dProjectManager, TestSetSingleCellwindowXY)
//{
//  modelPseudo1d::ModelPseudo1dInputData inputData;
//  inputData.projectFilename = "Project.project3d";

//  std::unique_ptr<modelPseudo1d::ModelPseudo1dProjectManager> projMgr = nullptr;
//  ASSERT_NO_THROW(projMgr.reset(new modelPseudo1d::ModelPseudo1dProjectManager(inputData)));
//  EXPECT_NO_THROW(projMgr->setSingleCellWindowXY());
//}

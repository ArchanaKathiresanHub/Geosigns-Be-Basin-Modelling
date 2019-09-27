#include "ModelPseudo1dInputData.h"
#include "ModelPseudo1dInputManager.h"

#include <gtest/gtest.h>

TEST(TestModelPseudo1dInputManager, TestInitialCheckArgsHelp)
{
  int argc = 2;
  char* argv[] = {"modelPseudo1d", "-help"};
  modelPseudo1d::ModelPseudo1dInputManager inputMgr(argc, argv);
  std::string ioErrorMssgActual = "";
  EXPECT_EQ(inputMgr.initialCheck(ioErrorMssgActual), modelPseudo1d::ModelPseudo1dInputManager::NO_ERROR_EXIT);
}

TEST(TestModelPseudo1dInputManager, TestInitialCheckArgsLessThanMinCompulsory)
{
  int argc = 3;
  char* argv[] = {"modelPseudo1d", "-project", "project.Project3d"};
  modelPseudo1d::ModelPseudo1dInputManager inputMgr(argc, argv);
  std::string ioErrorMssgActual = "";
  EXPECT_EQ(inputMgr.initialCheck(ioErrorMssgActual), modelPseudo1d::ModelPseudo1dInputManager::WITH_ERROR_EXIT);
}

TEST(TestModelPseudo1dInputManager, TestInitialCheckArgsMinimumCompulsory)
{
  int argc = 7;
  char* argv[] = {"modelPseudo1d", "-project", "project.Project3d", "-X", "1000.0", "-Y", "1000.0"};
  modelPseudo1d::ModelPseudo1dInputManager inputMgr(argc, argv);
  std::string ioErrorMssgActual = "";
  EXPECT_EQ(inputMgr.initialCheck(ioErrorMssgActual), modelPseudo1d::ModelPseudo1dInputManager::NO_EXIT);
}

TEST(TestModelPseudo1dInputManager, TestInitialCheckArgsEmptyValue)
{
  int argc = 7;
  // X coordinate value is empty!
  char* argv[] = {"modelPseudo1d", "-project", "project.Project3d", "-X", "-Y", "1000.0"};
  modelPseudo1d::ModelPseudo1dInputManager inputMgr(argc, argv);
  std::string ioErrorMssgActual = "";
  EXPECT_EQ(inputMgr.initialCheck(ioErrorMssgActual), modelPseudo1d::ModelPseudo1dInputManager::NO_EXIT);
  EXPECT_EQ(inputMgr.storeInput(ioErrorMssgActual), modelPseudo1d::ModelPseudo1dInputManager::WITH_ERROR_EXIT);
}

TEST(TestModelPseudo1dInputManager, TestInitialCheckArgsRepeatedArgument)
{
  int argc = 7;
  // X coordinate argument repeated!
  char* argv[] = {"modelPseudo1d", "-project", "project.Project3d", "-X", "1232", "-X", "1000.0"};
  modelPseudo1d::ModelPseudo1dInputManager inputMgr(argc, argv);
  std::string ioErrorMssgActual = "";
  EXPECT_EQ(inputMgr.initialCheck(ioErrorMssgActual), modelPseudo1d::ModelPseudo1dInputManager::NO_EXIT);
  EXPECT_EQ(inputMgr.storeInput(ioErrorMssgActual), modelPseudo1d::ModelPseudo1dInputManager::WITH_ERROR_EXIT);
}

TEST(TestModelPseudo1dInputManager, TestStoreInputsWithCorrectInput)
{
  int argc = 9;
  char* argv[] = {"modelPseudo1d", "-project", "project.Project3d", "-X", "1000.0", "-Y", "2000.0", "-out", "out.project3d"};
  modelPseudo1d::ModelPseudo1dInputManager inputMgr(argc, argv);
  std::string ioErrorMssgActual = "";
  EXPECT_EQ(inputMgr.initialCheck(ioErrorMssgActual), modelPseudo1d::ModelPseudo1dInputManager::NO_EXIT);
  EXPECT_EQ(inputMgr.storeInput(ioErrorMssgActual), modelPseudo1d::ModelPseudo1dInputManager::NO_EXIT);

  const modelPseudo1d::ModelPseudo1dInputData & inputData = inputMgr.inputData();

  const std::string projectNameExpected = "project.Project3d";
  const std::string & projectNameActual = inputData.projectFilename;
  EXPECT_EQ(projectNameExpected, projectNameActual);

  const double xCoordExpected = 1000.0;
  const double xCoordActual = inputData.xCoord;
  EXPECT_DOUBLE_EQ(xCoordExpected, xCoordActual);

  const double yCoordExpected = 2000.0;
  const double yCoordActual = inputData.yCoord;
  EXPECT_DOUBLE_EQ(yCoordExpected, yCoordActual);

  const std::string outProjectNameExpected = "out.project3d";
  const std::string & outProjectNameActual = inputData.outProjectFilename;
  EXPECT_EQ(outProjectNameExpected, outProjectNameActual);
}

TEST(TestModelPseudo1dInputManager, TestStoreInputsWithEmptyProjectName)
{
  int argc = 9;
  char* argv[] = {"modelPseudo1d", "-project", "", "-X", "1000.0", "-Y", "2000.0", "-out", "out.project3d"};
  modelPseudo1d::ModelPseudo1dInputManager inputMgr(argc, argv);
  std::string ioErrorMssgActual = "";
  EXPECT_EQ(inputMgr.initialCheck(ioErrorMssgActual), modelPseudo1d::ModelPseudo1dInputManager::NO_EXIT);
  EXPECT_EQ(inputMgr.storeInput(ioErrorMssgActual), modelPseudo1d::ModelPseudo1dInputManager::WITH_ERROR_EXIT);
}

TEST(TestModelPseudo1dInputManager, TestStoreInputsWithWrongArgument)
{
  int argc = 9;
  // Argument "-Y" is entered wrongly, i.e. as "Y"
  char* argv[] = {"modelPseudo1d", "-project", "project.Project3d", "-X", "1000.0", "Y", "2000.0", "-out", "out.project3d"};
  modelPseudo1d::ModelPseudo1dInputManager inputMgr(argc, argv);
  std::string ioErrorMssgActual = "";
  EXPECT_EQ(inputMgr.initialCheck(ioErrorMssgActual), modelPseudo1d::ModelPseudo1dInputManager::NO_EXIT);
  EXPECT_EQ(inputMgr.storeInput(ioErrorMssgActual), modelPseudo1d::ModelPseudo1dInputManager::WITH_ERROR_EXIT);
}

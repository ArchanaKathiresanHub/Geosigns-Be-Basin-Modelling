#include "Genex0dInputManager.h"

#include <gtest/gtest.h>

TEST(TestGenex0dInputManager, TestInitialCheckArgsHelp)
{
  int argc = 2;
  char* argv[] = {"genex0d", "-help"};
  genex0d::Genex0dInputManager inputMgr(argc, argv);
  std::string ioErrorMssgActual = "";
  EXPECT_EQ(inputMgr.initialCheck(ioErrorMssgActual), genex0d::Genex0dInputManager::NO_ERROR_EXIT);
}

TEST(TestGenex0dInputManager, TestInitialCheckArgsLessThanMinCompulsory)
{
  int argc = 6;
  char* argv[] = {"genex0d", "-project", "AcquiferScale1.Project3d", "-formation", "Formation6", "-SRType"};
  genex0d::Genex0dInputManager inputMgr(argc, argv);
  std::string ioErrorMssgActual = "";
  EXPECT_EQ(inputMgr.initialCheck(ioErrorMssgActual), genex0d::Genex0dInputManager::WITH_ERROR_EXIT);
}

TEST(TestGenex0dInputManager, TestInitialCheckArgsMinCompulsory)
{
  int argc = 7;
  char* argv[] = {"genex0d", "-project", "AcquiferScale1.project3d", "-formation", "Formation6", "-SRType", "Type I - Lacustrine"};
  genex0d::Genex0dInputManager inputMgr(argc, argv);
  std::string ioErrorMssgActual = "";
  EXPECT_EQ(inputMgr.initialCheck(ioErrorMssgActual), genex0d::Genex0dInputManager::NO_EXIT);
}

TEST(TestGenex0dInputManager, TestInitialCheckArgsEmptyValue)
{
  int argc = 10;
  // empty formation argument valu.
  char* argv[] = {"genex0d", "-project", "AcquiferScale1.project3d", "-formation", "-SRType", "Type I - Lacustrine", "-HC", "1.2", "-SC", "0.01"};
  genex0d::Genex0dInputManager inputMgr(argc, argv);
  std::string ioErrorMssgActual = "";
  EXPECT_EQ(inputMgr.initialCheck(ioErrorMssgActual), genex0d::Genex0dInputManager::NO_EXIT);
  EXPECT_EQ(inputMgr.storeInput(ioErrorMssgActual), genex0d::Genex0dInputManager::WITH_ERROR_EXIT);
}

TEST(TestGenex0dInputManager, TestInitialCheckArgsRepeatedArgument)
{
  int argc = 9;
  // formation repeated
  char* argv[] = {"genex0d", "-project", "AcquiferScale1.project3d", "-formation", "Formation6", "-SRType", "Type I - Lacustrine",
                 "-formation", "Formation1"};
  genex0d::Genex0dInputManager inputMgr(argc, argv);
  std::string ioErrorMssgActual = "";
  EXPECT_EQ(inputMgr.initialCheck(ioErrorMssgActual), genex0d::Genex0dInputManager::NO_EXIT);
  EXPECT_EQ(inputMgr.storeInput(ioErrorMssgActual), genex0d::Genex0dInputManager::WITH_ERROR_EXIT);
}

TEST(TestGenex0dInputManager, TestStoreInputsWithCorrectInput)
{
  int argc = 19;
  char* argv[] = {"genex0d", "-project", "AcquiferScale1.project3d", "-out", "outProj.project3d", "-formation", "Formation6", "-SRType", "Type I - Lacustrine",
                  "-X", "2005.0", "-Y", "8456.0", "-TOC", "10.0", "-HC", "1.2", "-SC", "0.01"};
  genex0d::Genex0dInputManager inputMgr(argc, argv);
  std::string ioErrorMssgActual = "";
  EXPECT_EQ(inputMgr.initialCheck(ioErrorMssgActual), genex0d::Genex0dInputManager::NO_EXIT);
  EXPECT_EQ(inputMgr.storeInput(ioErrorMssgActual), genex0d::Genex0dInputManager::NO_EXIT);

  const genex0d::Genex0dInputData & inputData = inputMgr.inputData();

  const std::string projectNameExpected = "AcquiferScale1.project3d";
  const std::string & projectNameActual = inputData.projectFilename;
  EXPECT_EQ(projectNameExpected, projectNameActual);

  const std::string outProjectNameExpected = "outProj.project3d";
  const std::string & outProjectNameActual = inputData.outProjectFilename;
  EXPECT_EQ(outProjectNameExpected, outProjectNameActual);

  const std::string formationNameExpected = "Formation6";
  const std::string & formationNameActual = inputData.formationName;
  EXPECT_EQ(formationNameExpected, formationNameActual);

  const std::string SRTypeExpected = "Type I - Lacustrine";
  const std::string & SRTypeActual = inputData.sourceRockType;
  EXPECT_EQ(SRTypeExpected, SRTypeActual);

  const double xCoordExpected = 2005.0;
  const double xCoordActual = inputData.xCoord;
  EXPECT_DOUBLE_EQ(xCoordExpected, xCoordActual);

  const double yCoordExpected = 8456.0;
  const double yCoordActual = inputData.yCoord;
  EXPECT_DOUBLE_EQ(yCoordExpected, yCoordActual);

  const double TOCExpected = 10.0;
  const double TOCActual = inputData.ToCIni;
  EXPECT_DOUBLE_EQ(TOCExpected, TOCActual);

  const double HCExpected = 1.2;
  const double HCActual = inputData.HCVRe05;
  EXPECT_DOUBLE_EQ(HCExpected, HCActual);

  const double SCExpected = 0.01;
  const double SCActual = inputData.SCVRe05;
  EXPECT_DOUBLE_EQ(SCExpected, SCActual);
}

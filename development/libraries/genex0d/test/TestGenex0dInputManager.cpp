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

TEST(TestGenex0dInputManager, TestInitialCheckArgsEmptyValue)
{
  int argc = 30;
  // empty formation argument value.
  char* argv[] = {"genex0d", "-project", "AcquiferScale1.project3d", "-out", "outProj.project3d", "-formation", "-SRType", "Type I - Lacustrine",
                  "-X", "2005.0", "-Y", "8456.0", "-TOC", "10.0", "-HC", "1.2", "-SC", "0.01", "-EA", "211", "-VesLimit", "10", "-Asph", "87", "-Resin", "80", "-C15Aro", "77", "-C15Sat", "71"};
  genex0d::Genex0dInputManager inputMgr(argc, argv);
  std::string ioErrorMssgActual = "";
  EXPECT_EQ(inputMgr.initialCheck(ioErrorMssgActual), genex0d::Genex0dInputManager::NO_EXIT);
  EXPECT_EQ(inputMgr.storeInput(ioErrorMssgActual), genex0d::Genex0dInputManager::WITH_ERROR_EXIT);
}

TEST(TestGenex0dInputManager, TestInitialCheckArgsRepeatedArgument)
{
  int argc = 33;
  // formation repeated
  char* argv[] = {"genex0d", "-project", "AcquiferScale1.project3d", "-out", "outProj.project3d", "-formation", "Formation6", "-SRType", "Type I - Lacustrine",
                  "-X", "2005.0", "-Y", "8456.0", "-TOC", "10.0", "-HC", "1.2", "-SC", "0.01", "-EA", "211", "-VesLimit", "10", "-Asph", "87", "-Resin",
                  "80", "-C15Aro", "77", "-C15Sat", "71", "-formation", "FormationDuplicated"};
  genex0d::Genex0dInputManager inputMgr(argc, argv);
  std::string ioErrorMssgActual = "";
  EXPECT_EQ(inputMgr.initialCheck(ioErrorMssgActual), genex0d::Genex0dInputManager::NO_EXIT);
  EXPECT_EQ(inputMgr.storeInput(ioErrorMssgActual), genex0d::Genex0dInputManager::WITH_ERROR_EXIT);
}

TEST(TestGenex0dInputManager, TestStoreInputsWithCorrectInput)
{
  int argc = 37;
  char* argv[] = {"genex0d", "-project", "AcquiferScale1.project3d", "-out", "outProj.project3d", "-formation", "Formation6", "-SRType", "Type I - Lacustrine",
                  "-X", "2005.0", "-Y", "8456.0", "-TOC", "10.0", "-HC", "1.2", "-SC", "0.01", "-EA", "211", "-VesLimit", "10", "-Asph", "87", "-Resin", "80",
                  "-C15Aro", "77", "-C15Sat", "71", "-AdsSimulator", "OTGCC1AdsorptionSimulator", "-AdsCapacityFunc", "Default Langmuir Isotherm", "-doOTGC", "1"};
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

  const double activationEnergyExpected = 211000.0;
  const double activationEnergyActual = inputData.activationEnergy;
  EXPECT_DOUBLE_EQ(activationEnergyExpected, activationEnergyActual);

  const double VesLimitExpected = 10;
  const double VesLimitActual = inputData.maxVes;
  EXPECT_DOUBLE_EQ(VesLimitExpected, VesLimitActual);

  const double asphalteneDiffusionEnergyExpected = 87000;
  const double asphalteneDiffusionEnergyActual = inputData.asphalteneDiffusionEnergy;
  EXPECT_DOUBLE_EQ(asphalteneDiffusionEnergyExpected, asphalteneDiffusionEnergyActual);

  const double C15AroDiffusionEnergyExpected = 77000;
  const double C15AroDiffusionEnergyActual = inputData.C15AroDiffusionEnergy;
  EXPECT_DOUBLE_EQ(C15AroDiffusionEnergyExpected, C15AroDiffusionEnergyActual);

  const double C15SatDiffusionEnergyExpected = 71000;
  const double C15SatDiffusionEnergyActual = inputData.C15SatDiffusionEnergy;
  EXPECT_DOUBLE_EQ(C15SatDiffusionEnergyExpected, C15SatDiffusionEnergyActual);

  const double resinDiffusionEnergyExpected = 80000;
  const double resinDiffusionEnergyActual = inputData.resinDiffusionEnergy;
  EXPECT_DOUBLE_EQ(resinDiffusionEnergyExpected, resinDiffusionEnergyActual);

  const std::string adsorptionSimulatorExpected = "OTGCC1AdsorptionSimulator";
  const std::string & adsorptionSimulatorActual = inputData.whichAdsorptionSimulator;
  EXPECT_EQ(adsorptionSimulatorExpected, adsorptionSimulatorActual);

  const std::string adsorptionCapacityFuncExpected = "Default Langmuir Isotherm";
  const std::string & adsorptionCapacityFuncActual = inputData.whichAdsorptionFunction;
  EXPECT_EQ(adsorptionCapacityFuncExpected, adsorptionCapacityFuncActual);

  EXPECT_TRUE(inputData.doOTCG);
}

TEST(TestGenex0dInputManager, TestMaxVesDisabled)
{
  // No VesLimit argument is given
  int argc = 29;
  char* argv[] = {"genex0d", "-project", "AcquiferScale1.project3d", "-out", "outProj.project3d", "-formation", "Formation6", "-SRType", "Type I - Lacustrine",
                  "-X", "2005.0", "-Y", "8456.0", "-TOC", "10.0", "-HC", "1.2", "-SC", "0.01", "-EA", "211", "-Asph", "87", "-Resin", "80", "-C15Aro", "77", "-C15Sat", "71"};
  genex0d::Genex0dInputManager inputMgr(argc, argv);
  std::string ioErrorMssgActual = "";
  EXPECT_EQ(inputMgr.initialCheck(ioErrorMssgActual), genex0d::Genex0dInputManager::NO_EXIT);
  EXPECT_EQ(inputMgr.storeInput(ioErrorMssgActual), genex0d::Genex0dInputManager::NO_EXIT);

  const genex0d::Genex0dInputData & inputData = inputMgr.inputData();

  EXPECT_FALSE(inputData.maxVesEnabled);
}

TEST(TestGenex0dInputManager, TestMissingYArgumentExit)
{
  int argc = 29;
  char* argv[] = {"genex0d", "-project", "AcquiferScale1.project3d", "-out", "outProj.project3d", "-formation", "Formation6", "-SRType", "Type I - Lacustrine",
                  "-X", "2005.0", "-TOC", "10.0", "-HC", "1.2", "-SC", "0.01", "-EA", "211", "-VesLimit", "10", "-Asph", "87", "-Resin", "80", "-C15Aro", "77", "-C15Sat", "71"};
  genex0d::Genex0dInputManager inputMgr(argc, argv);
  std::string ioErrorMssgActual = "";
  EXPECT_EQ(inputMgr.initialCheck(ioErrorMssgActual), genex0d::Genex0dInputManager::NO_EXIT);
  EXPECT_EQ(inputMgr.storeInput(ioErrorMssgActual), genex0d::Genex0dInputManager::WITH_ERROR_EXIT);
}

TEST(TestGenex0dInputManager, TestMissingXArgumentExit)
{
  int argc = 29;
  char* argv[] = {"genex0d", "-project", "AcquiferScale1.project3d", "-out", "outProj.project3d", "-formation", "Formation6", "-SRType", "Type I - Lacustrine",
                  "-Y", "8456.0", "-TOC", "10.0", "-HC", "1.2", "-SC", "0.01", "-EA", "211", "-VesLimit", "10", "-Asph", "87", "-Resin", "80", "-C15Aro", "77", "-C15Sat", "71"};
  genex0d::Genex0dInputManager inputMgr(argc, argv);
  std::string ioErrorMssgActual = "";
  EXPECT_EQ(inputMgr.initialCheck(ioErrorMssgActual), genex0d::Genex0dInputManager::NO_EXIT);
  EXPECT_EQ(inputMgr.storeInput(ioErrorMssgActual), genex0d::Genex0dInputManager::WITH_ERROR_EXIT);
}

TEST(TestGenex0dInputManager, TestMissingProjectArgumentExit)
{
  int argc = 29;
  char* argv[] = {"genex0d", "-out", "outProj.project3d", "-formation", "Formation6", "-SRType", "Type I - Lacustrine",
                  "-X", "2005.0", "-Y", "8456.0", "-TOC", "10.0", "-HC", "1.2", "-SC", "0.01", "-EA", "211", "-VesLimit", "10", "-Asph", "87", "-Resin", "80", "-C15Aro", "77", "-C15Sat", "71"};
  genex0d::Genex0dInputManager inputMgr(argc, argv);
  std::string ioErrorMssgActual = "";
  EXPECT_EQ(inputMgr.initialCheck(ioErrorMssgActual), genex0d::Genex0dInputManager::NO_EXIT);
  EXPECT_EQ(inputMgr.storeInput(ioErrorMssgActual), genex0d::Genex0dInputManager::WITH_ERROR_EXIT);
}

TEST(TestGenex0dInputManager, TestMissingFormationArgumentExit)
{
  int argc = 29;
  char* argv[] = {"genex0d", "-project", "AcquiferScale1.project3d", "-out", "outProj.project3d", "-SRType", "Type I - Lacustrine",
                  "-X", "2005.0", "-Y", "8456.0", "-TOC", "10.0", "-HC", "1.2", "-SC", "0.01", "-EA", "211", "-VesLimit", "10", "-Asph", "87", "-Resin", "80", "-C15Aro", "77", "-C15Sat", "71"};
  genex0d::Genex0dInputManager inputMgr(argc, argv);
  std::string ioErrorMssgActual = "";
  EXPECT_EQ(inputMgr.initialCheck(ioErrorMssgActual), genex0d::Genex0dInputManager::NO_EXIT);
  EXPECT_EQ(inputMgr.storeInput(ioErrorMssgActual), genex0d::Genex0dInputManager::WITH_ERROR_EXIT);
}

TEST(TestGenex0dInputManager, TestMissingSRTypeArgumentExit)
{
  int argc = 29;
  char* argv[] = {"genex0d", "-project", "AcquiferScale1.project3d", "-out", "outProj.project3d", "-formation", "Formation6",
                  "-X", "2005.0", "-Y", "8456.0", "-TOC", "10.0", "-HC", "1.2", "-SC", "0.01", "-EA", "211", "-VesLimit", "10", "-Asph", "87", "-Resin", "80", "-C15Aro", "77", "-C15Sat", "71"};
  genex0d::Genex0dInputManager inputMgr(argc, argv);
  std::string ioErrorMssgActual = "";
  EXPECT_EQ(inputMgr.initialCheck(ioErrorMssgActual), genex0d::Genex0dInputManager::NO_EXIT);
  EXPECT_EQ(inputMgr.storeInput(ioErrorMssgActual), genex0d::Genex0dInputManager::WITH_ERROR_EXIT);
}

TEST(TestGenex0dInputManager, TestMissingTOCArgumentExit)
{
  int argc = 29;
  char* argv[] = {"genex0d", "-project", "AcquiferScale1.project3d", "-out", "outProj.project3d", "-formation", "Formation6", "-SRType", "Type I - Lacustrine",
                  "-X", "2005.0", "-Y", "8456.0", "-HC", "1.2", "-SC", "0.01", "-EA", "211", "-VesLimit", "10", "-Asph", "87", "-Resin", "80", "-C15Aro", "77", "-C15Sat", "71"};

  genex0d::Genex0dInputManager inputMgr(argc, argv);
  std::string ioErrorMssgActual = "";
  EXPECT_EQ(inputMgr.initialCheck(ioErrorMssgActual), genex0d::Genex0dInputManager::NO_EXIT);
  EXPECT_EQ(inputMgr.storeInput(ioErrorMssgActual), genex0d::Genex0dInputManager::WITH_ERROR_EXIT);
}

TEST(TestGenex0dInputManager, TestMissingHCArgumentExit)
{
  int argc = 29;
  char* argv[] = {"genex0d", "-project", "AcquiferScale1.project3d", "-out", "outProj.project3d", "-formation", "Formation6", "-SRType", "Type I - Lacustrine",
                  "-X", "2005.0", "-Y", "8456.0", "-TOC", "10.0", "-SC", "0.01", "-EA", "211", "-VesLimit", "10", "-Asph", "87", "-Resin", "80", "-C15Aro", "77", "-C15Sat", "71"};
  genex0d::Genex0dInputManager inputMgr(argc, argv);
  std::string ioErrorMssgActual = "";
  EXPECT_EQ(inputMgr.initialCheck(ioErrorMssgActual), genex0d::Genex0dInputManager::NO_EXIT);
  EXPECT_EQ(inputMgr.storeInput(ioErrorMssgActual), genex0d::Genex0dInputManager::WITH_ERROR_EXIT);
}

TEST(TestGenex0dInputManager, TestMissingSCArgumentExit)
{
  int argc = 29;
  char* argv[] = {"genex0d", "-project", "AcquiferScale1.project3d", "-out", "outProj.project3d", "-formation", "Formation6", "-SRType", "Type I - Lacustrine",
                  "-X", "2005.0", "-Y", "8456.0", "-TOC", "10.0", "-HC", "1.2", "-EA", "211", "-VesLimit", "10", "-Asph", "87", "-Resin", "80", "-C15Aro", "77", "-C15Sat", "71"};
  genex0d::Genex0dInputManager inputMgr(argc, argv);
  std::string ioErrorMssgActual = "";
  EXPECT_EQ(inputMgr.initialCheck(ioErrorMssgActual), genex0d::Genex0dInputManager::NO_EXIT);
  EXPECT_EQ(inputMgr.storeInput(ioErrorMssgActual), genex0d::Genex0dInputManager::WITH_ERROR_EXIT);
}

TEST(TestGenex0dInputManager, TestMissingEAArgumentExit)
{
  int argc = 29;
  char* argv[] = {"genex0d", "-project", "AcquiferScale1.project3d", "-out", "outProj.project3d", "-formation", "Formation6", "-SRType", "Type I - Lacustrine",
                  "-X", "2005.0", "-Y", "8456.0", "-TOC", "10.0", "-HC", "1.2", "-SC", "0.01", "-VesLimit", "10", "-Asph", "87", "-Resin", "80", "-C15Aro", "77", "-C15Sat", "71"};
  genex0d::Genex0dInputManager inputMgr(argc, argv);
  std::string ioErrorMssgActual = "";
  EXPECT_EQ(inputMgr.initialCheck(ioErrorMssgActual), genex0d::Genex0dInputManager::NO_EXIT);
  EXPECT_EQ(inputMgr.storeInput(ioErrorMssgActual), genex0d::Genex0dInputManager::WITH_ERROR_EXIT);
}

TEST(TestGenex0dInputManager, TestMissingAsphArgumentExit)
{
  int argc = 29;
  char* argv[] = {"genex0d", "-project", "AcquiferScale1.project3d", "-out", "outProj.project3d", "-formation", "Formation6", "-SRType", "Type I - Lacustrine",
                  "-X", "2005.0", "-Y", "8456.0", "-TOC", "10.0", "-HC", "1.2", "-SC", "0.01", "-EA", "211", "-VesLimit", "10", "-Resin", "80", "-C15Aro", "77", "-C15Sat", "71"};
  genex0d::Genex0dInputManager inputMgr(argc, argv);
  std::string ioErrorMssgActual = "";
  EXPECT_EQ(inputMgr.initialCheck(ioErrorMssgActual), genex0d::Genex0dInputManager::NO_EXIT);
  EXPECT_EQ(inputMgr.storeInput(ioErrorMssgActual), genex0d::Genex0dInputManager::WITH_ERROR_EXIT);
}

TEST(TestGenex0dInputManager, TestMissingResinArgumentExit)
{
  int argc = 29;
  char* argv[] = {"genex0d", "-project", "AcquiferScale1.project3d", "-out", "outProj.project3d", "-formation", "Formation6", "-SRType", "Type I - Lacustrine",
                  "-X", "2005.0", "-Y", "8456.0", "-TOC", "10.0", "-HC", "1.2", "-SC", "0.01", "-EA", "211", "-VesLimit", "10", "-Asph", "87", "-C15Aro", "77", "-C15Sat", "71"};
  genex0d::Genex0dInputManager inputMgr(argc, argv);
  std::string ioErrorMssgActual = "";
  EXPECT_EQ(inputMgr.initialCheck(ioErrorMssgActual), genex0d::Genex0dInputManager::NO_EXIT);
  EXPECT_EQ(inputMgr.storeInput(ioErrorMssgActual), genex0d::Genex0dInputManager::WITH_ERROR_EXIT);
}

TEST(TestGenex0dInputManager, TestMissingC15AroArgumentExit)
{
  int argc = 29;
  char* argv[] = {"genex0d", "-project", "AcquiferScale1.project3d", "-out", "outProj.project3d", "-formation", "Formation6", "-SRType", "Type I - Lacustrine",
                  "-X", "2005.0", "-Y", "8456.0", "-TOC", "10.0", "-HC", "1.2", "-SC", "0.01", "-EA", "211", "-VesLimit", "10", "-Asph", "87", "-Resin", "80", "-C15Sat", "71"};
  genex0d::Genex0dInputManager inputMgr(argc, argv);
  std::string ioErrorMssgActual = "";
  EXPECT_EQ(inputMgr.initialCheck(ioErrorMssgActual), genex0d::Genex0dInputManager::NO_EXIT);
  EXPECT_EQ(inputMgr.storeInput(ioErrorMssgActual), genex0d::Genex0dInputManager::WITH_ERROR_EXIT);
}

TEST(TestGenex0dInputManager, TestMissingC15SatArgumentExit)
{
  int argc = 29;
  char* argv[] = {"genex0d", "-project", "AcquiferScale1.project3d", "-out", "outProj.project3d", "-formation", "Formation6", "-SRType", "Type I - Lacustrine",
                  "-X", "2005.0", "-Y", "8456.0", "-TOC", "10.0", "-HC", "1.2", "-SC", "0.01", "-EA", "211", "-VesLimit", "10", "-Asph", "87", "-Resin", "80", "-C15Aro", "77"};
  genex0d::Genex0dInputManager inputMgr(argc, argv);
  std::string ioErrorMssgActual = "";
  EXPECT_EQ(inputMgr.initialCheck(ioErrorMssgActual), genex0d::Genex0dInputManager::NO_EXIT);
  EXPECT_EQ(inputMgr.storeInput(ioErrorMssgActual), genex0d::Genex0dInputManager::WITH_ERROR_EXIT);
}

TEST(TestGenex0dInputManager, TestMissingAdsFuncArgumentWhenAdsSimulatorEnabledExit)
{
  int argc = 35;
  char* argv[] = {"genex0d", "-project", "AcquiferScale1.project3d", "-out", "outProj.project3d", "-formation", "Formation6", "-SRType", "Type I - Lacustrine",
                  "-X", "2005.0", "-Y", "8456.0", "-TOC", "10.0", "-HC", "1.2", "-SC", "0.01", "-EA", "211", "-VesLimit", "10", "-Asph", "87", "-Resin", "80",
                  "-C15Aro", "77", "-C15Sat", "71", "-AdsSimulator", "OTGCC1AdsorptionSimulator", "-doOTGC", "1"};
  genex0d::Genex0dInputManager inputMgr(argc, argv);
  std::string ioErrorMssgActual = "";
  EXPECT_EQ(inputMgr.initialCheck(ioErrorMssgActual), genex0d::Genex0dInputManager::NO_EXIT);
  EXPECT_EQ(inputMgr.storeInput(ioErrorMssgActual), genex0d::Genex0dInputManager::WITH_ERROR_EXIT);
  EXPECT_EQ(ioErrorMssgActual, "An adsorption simulator was provided, but no adsorption capacity function");
}

TEST(TestGenex0dInputManager, TestMissingAdsSimulatorArgumentWhenOTCGEnabledExit)
{
  int argc = 33;
  char* argv[] = {"genex0d", "-project", "AcquiferScale1.project3d", "-out", "outProj.project3d", "-formation", "Formation6", "-SRType", "Type I - Lacustrine",
                  "-X", "2005.0", "-Y", "8456.0", "-TOC", "10.0", "-HC", "1.2", "-SC", "0.01", "-EA", "211", "-VesLimit", "10", "-Asph", "87", "-Resin", "80",
                  "-C15Aro", "77", "-C15Sat", "71", "-doOTGC", "1"};
  genex0d::Genex0dInputManager inputMgr(argc, argv);
  std::string ioErrorMssgActual = "";
  EXPECT_EQ(inputMgr.initialCheck(ioErrorMssgActual), genex0d::Genex0dInputManager::NO_EXIT);
  EXPECT_EQ(inputMgr.storeInput(ioErrorMssgActual), genex0d::Genex0dInputManager::WITH_ERROR_EXIT);
  EXPECT_EQ(ioErrorMssgActual, "OTGC was enabled, but no adsorption simulator was provided");
}

TEST(TestGenex0dInputManager, TestMissingAdsSimulatorArgumentWhenAdsFuncGivenExit)
{
  int argc = 33;
  char* argv[] = {"genex0d", "-project", "AcquiferScale1.project3d", "-out", "outProj.project3d", "-formation", "Formation6", "-SRType", "Type I - Lacustrine",
                  "-X", "2005.0", "-Y", "8456.0", "-TOC", "10.0", "-HC", "1.2", "-SC", "0.01", "-EA", "211", "-VesLimit", "10", "-Asph", "87", "-Resin", "80",
                  "-C15Aro", "77", "-C15Sat", "71", "-AdsCapacityFunc", "Default Langmuir Isotherm"};
  genex0d::Genex0dInputManager inputMgr(argc, argv);
  std::string ioErrorMssgActual = "";
  EXPECT_EQ(inputMgr.initialCheck(ioErrorMssgActual), genex0d::Genex0dInputManager::NO_EXIT);
  EXPECT_EQ(inputMgr.storeInput(ioErrorMssgActual), genex0d::Genex0dInputManager::WITH_ERROR_EXIT);
  EXPECT_EQ(ioErrorMssgActual, "An adsorption capacity function was provided, but no adsorption simulator");
}

TEST(TestGenex0dInputManager, TestMissingLastArgument)
{
  int argc = 30;
  char* argv[] = {"genex0d", "-project", "AcquiferScale1.project3d", "-out", "outProj.project3d", "-formation", "Formation6", "-SRType", "Type I - Lacustrine",
                  "-X", "2005.0", "-Y", "8456.0", "-TOC", "10.0", "-HC", "1.2", "-SC", "0.01", "-EA", "211", "-VesLimit", "10", "-Asph", "87", "-Resin", "80",
                  "-C15Aro", "77", "-C15Sat"};
  genex0d::Genex0dInputManager inputMgr(argc, argv);
  std::string ioErrorMssgActual = "";
  EXPECT_EQ(inputMgr.initialCheck(ioErrorMssgActual), genex0d::Genex0dInputManager::NO_EXIT);
  EXPECT_EQ(inputMgr.storeInput(ioErrorMssgActual), genex0d::Genex0dInputManager::WITH_ERROR_EXIT);
}




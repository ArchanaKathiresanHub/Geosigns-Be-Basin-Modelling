#include "Genex0dInputManager.h"

#include <gtest/gtest.h>

std::vector<std::string> generateFullInput()
{
  return {"genex0d", "-project", "AcquiferScale1.project3d", "-out", "outProj.project3d", "-datFileName", "test.dat" , "-formation", "Formation6", "-SRType", "Type I - Lacustrine",
      "-X", "2005.0", "-Y", "8456.0", "-TOC", "10.0", "-HC", "1.2", "-SC", "0.01", "-EA", "211", "-VesLimit", "10", "-Asph", "87", "-Resin", "80",
      "-C15Aro", "77", "-C15Sat", "71", "-AdsSimulator", "OTGCC1AdsorptionSimulator", "-AdsCapacityFunc", "Default Langmuir Isotherm",
      "-AdsLangmuirTpvTable", "20 6.8 3.67 50 7.48 2.35 90 4.64 0.99 115 3.868571429 0.03286", "-doOTGC", "1",
      "-AdsIrrWatSat", "-0.15 0.9",
      "-SRType_SR2", "Type_II_Paleozoic_Marine_Shale_kin_s", "-HC_SR2", "1.13", "-SC_SR2", "0.0", "-EA_SR2", "210", "-Asph_SR2", "90", "-Resin_SR2", "84",
      "-C15Aro_SR2", "78", "-C15Sat_SR2", "73", "-MixingHI", "400"};
}

void removeArgumentAndValueFromInput(std::vector<std::string>& inputs, const std::string argument)
{
  int indexToRemove = 0;
  for (size_t i = 0; i < inputs.size(); i++)
  {
    if (inputs[i] == argument)
    {
      indexToRemove = i;
    }
  }

  inputs.erase(inputs.begin() + indexToRemove, inputs.begin() + indexToRemove + 2);
}

TEST(TestGenex0dInputManager, TestInitialCheckArgsHelp)
{
  int argc = 2;
  std::vector<std::string> argvVector = {"genex0d", "-help"};
  Genex0d::Genex0dInputManager inputMgr(argvVector);
  std::string ioErrorMssgActual = "";
  EXPECT_EQ(inputMgr.initialCheck(ioErrorMssgActual), Genex0d::Genex0dInputManager::NO_ERROR_EXIT);
}

TEST(TestGenex0dInputManager, TestInitialCheckArgsEmptyValue)
{
  // empty formation argument value.
  std::vector<std::string> argvVector= {"genex0d", "-project", "AcquiferScale1.project3d", "-out", "outProj.project3d", "-formation", "-SRType", "Type I - Lacustrine",
                                        "-X", "2005.0", "-Y", "8456.0", "-TOC", "10.0", "-HC", "1.2", "-SC", "0.01", "-EA", "211", "-VesLimit", "10", "-Asph", "87", "-Resin",
                                        "80", "-C15Aro", "77", "-C15Sat", "71"};
  Genex0d::Genex0dInputManager inputMgr(argvVector);
  std::string ioErrorMssgActual = "";
  EXPECT_EQ(inputMgr.initialCheck(ioErrorMssgActual), Genex0d::Genex0dInputManager::NO_EXIT);
  EXPECT_EQ(inputMgr.storeInput(ioErrorMssgActual), Genex0d::Genex0dInputManager::WITH_ERROR_EXIT);
}

TEST(TestGenex0dInputManager, TestInitialCheckArgsRepeatedArgument)
{
  std::vector<std::string> argvVector = generateFullInput();
  argvVector.push_back("-formation");
  argvVector.push_back("DuplicateFormation");

  Genex0d::Genex0dInputManager inputMgr(argvVector);
  std::string ioErrorMssgActual = "";
  EXPECT_EQ(inputMgr.initialCheck(ioErrorMssgActual), Genex0d::Genex0dInputManager::NO_EXIT);
  EXPECT_EQ(inputMgr.storeInput(ioErrorMssgActual), Genex0d::Genex0dInputManager::WITH_ERROR_EXIT);
}

TEST(TestGenex0dInputManager, TestStoreInputsWithCorrectInput)
{
  std::vector<std::string> argvVector = generateFullInput();

  Genex0d::Genex0dInputManager inputMgr(argvVector);
  std::string ioErrorMssgActual = "";
  EXPECT_EQ(inputMgr.initialCheck(ioErrorMssgActual), Genex0d::Genex0dInputManager::NO_EXIT);
  EXPECT_EQ(inputMgr.storeInput(ioErrorMssgActual), Genex0d::Genex0dInputManager::NO_EXIT);

  const Genex0d::Genex0dInputData & inputData = inputMgr.inputData();

  const std::string projectNameExpected = "AcquiferScale1.project3d";
  const std::string & projectNameActual = inputData.projectFilename;
  EXPECT_EQ(projectNameExpected, projectNameActual);

  const std::string outProjectNameExpected = "outProj.project3d";
  const std::string & outProjectNameActual = inputData.outProjectFilename;
  EXPECT_EQ(outProjectNameExpected, outProjectNameActual);

  const std::string outDatFileNameExpected = "test.dat";
  const std::string & outDatFileNameActual = inputData.nodeHistoryFileName;
  EXPECT_EQ(outDatFileNameExpected, outDatFileNameActual);

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

  const double activationEnergyExpected = 211;
  const double activationEnergyActual = inputData.activationEnergy;
  EXPECT_DOUBLE_EQ(activationEnergyExpected, activationEnergyActual);

  const double VesLimitExpected = 10;
  const double VesLimitActual = inputData.maxVes;
  EXPECT_DOUBLE_EQ(VesLimitExpected, VesLimitActual);

  const double asphalteneDiffusionEnergyExpected = 87;
  const double asphalteneDiffusionEnergyActual = inputData.asphalteneDiffusionEnergy;
  EXPECT_DOUBLE_EQ(asphalteneDiffusionEnergyExpected, asphalteneDiffusionEnergyActual);

  const double C15AroDiffusionEnergyExpected = 77;
  const double C15AroDiffusionEnergyActual = inputData.C15AroDiffusionEnergy;
  EXPECT_DOUBLE_EQ(C15AroDiffusionEnergyExpected, C15AroDiffusionEnergyActual);

  const double C15SatDiffusionEnergyExpected = 71;
  const double C15SatDiffusionEnergyActual = inputData.C15SatDiffusionEnergy;
  EXPECT_DOUBLE_EQ(C15SatDiffusionEnergyExpected, C15SatDiffusionEnergyActual);

  const double resinDiffusionEnergyExpected = 80;
  const double resinDiffusionEnergyActual = inputData.resinDiffusionEnergy;
  EXPECT_DOUBLE_EQ(resinDiffusionEnergyExpected, resinDiffusionEnergyActual);

  const std::string adsorptionSimulatorExpected = "OTGCC1AdsorptionSimulator";
  const std::string & adsorptionSimulatorActual = inputData.whichAdsorptionSimulator;
  EXPECT_EQ(adsorptionSimulatorExpected, adsorptionSimulatorActual);

  const std::string adsorptionCapacityFuncExpected = "Default Langmuir Isotherm";
  const std::string & adsorptionCapacityFuncActual = inputData.whichAdsorptionFunction;
  EXPECT_EQ(adsorptionCapacityFuncExpected, adsorptionCapacityFuncActual);

  const std::string adsorptionDataExpected = "20 6.8 3.67 50 7.48 2.35 90 4.64 0.99 115 3.868571429 0.03286";
  const std::string & adsorptionDataActual = inputData.adsorptionFunctionTPVData;
  EXPECT_EQ(adsorptionDataExpected, adsorptionDataActual);

  const std::string irreducibleWaterSaturationDataExpected = "-0.15 0.9";
  const std::string & irreducibleWaterSaturationDataActual = inputData.irreducibleWaterSaturationData;
  EXPECT_EQ(irreducibleWaterSaturationDataExpected, irreducibleWaterSaturationDataActual);

  const double HCExpectedSR2 = 1.13;
  const double HCActualSR2 = inputData.HCVRe05SR2;
  EXPECT_DOUBLE_EQ(HCExpectedSR2, HCActualSR2);

  const double SCExpectedSR2 = 0.0;
  const double SCActualSR2 = inputData.SCVRe05SR2;
  EXPECT_DOUBLE_EQ(SCExpectedSR2, SCActualSR2);

  const double activationEnergyExpectedSR2 = 210;
  const double activationEnergyActualSR2 = inputData.activationEnergySR2;
  EXPECT_DOUBLE_EQ(activationEnergyExpectedSR2, activationEnergyActualSR2);

  const double asphalteneDiffusionEnergyExpectedSR2 = 90;
  const double asphalteneDiffusionEnergyActualSR2 = inputData.asphalteneDiffusionEnergySR2;
  EXPECT_DOUBLE_EQ(asphalteneDiffusionEnergyExpectedSR2, asphalteneDiffusionEnergyActualSR2);

  const double C15AroDiffusionEnergyExpectedSR2 = 78;
  const double C15AroDiffusionEnergyActualSR2 = inputData.C15AroDiffusionEnergySR2;
  EXPECT_DOUBLE_EQ(C15AroDiffusionEnergyExpectedSR2, C15AroDiffusionEnergyActualSR2);

  const double C15SatDiffusionEnergyExpectedSR2= 73;
  const double C15SatDiffusionEnergyActualSR2 = inputData.C15SatDiffusionEnergySR2;
  EXPECT_DOUBLE_EQ(C15SatDiffusionEnergyExpectedSR2, C15SatDiffusionEnergyActualSR2);

  const double resinDiffusionEnergyExpectedSR2 = 84;
  const double resinDiffusionEnergyActualSR2 = inputData.resinDiffusionEnergySR2;
  EXPECT_DOUBLE_EQ(resinDiffusionEnergyExpectedSR2, resinDiffusionEnergyActualSR2);

  const std::string SRTypeExpectedSR2 = "Type_II_Paleozoic_Marine_Shale_kin_s";
  const std::string & SRTypeActualSR2 = inputData.sourceRockTypeSR2;
  EXPECT_EQ(SRTypeExpectedSR2, SRTypeActualSR2);

  const double mixingHIExpectedSR2 = 400;
  const double mixingHIActualSR2 = inputData.mixingHI;
  EXPECT_DOUBLE_EQ(mixingHIExpectedSR2, mixingHIActualSR2);

  EXPECT_TRUE(inputData.doOTCG);
}

TEST(TestGenex0dInputManager, TestMaxVesDisabled)
{
  std::vector<std::string> argvVector = generateFullInput();
  removeArgumentAndValueFromInput(argvVector, "-VesLimit");

  Genex0d::Genex0dInputManager inputMgr(argvVector);
  std::string ioErrorMssgActual = "";
  EXPECT_EQ(inputMgr.initialCheck(ioErrorMssgActual), Genex0d::Genex0dInputManager::NO_EXIT);
  EXPECT_EQ(inputMgr.storeInput(ioErrorMssgActual), Genex0d::Genex0dInputManager::NO_EXIT);

  const Genex0d::Genex0dInputData & inputData = inputMgr.inputData();

  EXPECT_FALSE(inputData.maxVesEnabled);

}

TEST(TestGenex0dInputManager, TestMissingYArgumentExit)
{
  std::vector<std::string> argvVector = generateFullInput();
  removeArgumentAndValueFromInput(argvVector, "-Y");

  Genex0d::Genex0dInputManager inputMgr(argvVector);
  std::string ioErrorMssgActual = "";
  EXPECT_EQ(inputMgr.initialCheck(ioErrorMssgActual), Genex0d::Genex0dInputManager::NO_EXIT);
  EXPECT_EQ(inputMgr.storeInput(ioErrorMssgActual), Genex0d::Genex0dInputManager::WITH_ERROR_EXIT);
}

TEST(TestGenex0dInputManager, TestMissingXArgumentExit)
{
  std::vector<std::string> argvVector = generateFullInput();
  removeArgumentAndValueFromInput(argvVector, "-X");

  Genex0d::Genex0dInputManager inputMgr(argvVector);
  std::string ioErrorMssgActual = "";
  EXPECT_EQ(inputMgr.initialCheck(ioErrorMssgActual), Genex0d::Genex0dInputManager::NO_EXIT);
  EXPECT_EQ(inputMgr.storeInput(ioErrorMssgActual), Genex0d::Genex0dInputManager::WITH_ERROR_EXIT);
}

TEST(TestGenex0dInputManager, TestMissingProjectArgumentExit)
{
  std::vector<std::string> argvVector = generateFullInput();
  removeArgumentAndValueFromInput(argvVector, "-project");

  Genex0d::Genex0dInputManager inputMgr(argvVector);
  std::string ioErrorMssgActual = "";
  EXPECT_EQ(inputMgr.initialCheck(ioErrorMssgActual), Genex0d::Genex0dInputManager::NO_EXIT);
  EXPECT_EQ(inputMgr.storeInput(ioErrorMssgActual), Genex0d::Genex0dInputManager::WITH_ERROR_EXIT);
}

TEST(TestGenex0dInputManager, TestMissingFormationArgumentExit)
{
  std::vector<std::string> argvVector = generateFullInput();
  removeArgumentAndValueFromInput(argvVector, "-formation");

  Genex0d::Genex0dInputManager inputMgr(argvVector);
  std::string ioErrorMssgActual = "";
  EXPECT_EQ(inputMgr.initialCheck(ioErrorMssgActual), Genex0d::Genex0dInputManager::NO_EXIT);
  EXPECT_EQ(inputMgr.storeInput(ioErrorMssgActual), Genex0d::Genex0dInputManager::WITH_ERROR_EXIT);
}

TEST(TestGenex0dInputManager, TestMissingSRTypeArgumentExit)
{
  std::vector<std::string> argvVector = generateFullInput();
  removeArgumentAndValueFromInput(argvVector, "-SRType");

  Genex0d::Genex0dInputManager inputMgr(argvVector);
  std::string ioErrorMssgActual = "";
  EXPECT_EQ(inputMgr.initialCheck(ioErrorMssgActual), Genex0d::Genex0dInputManager::NO_EXIT);
  EXPECT_EQ(inputMgr.storeInput(ioErrorMssgActual), Genex0d::Genex0dInputManager::WITH_ERROR_EXIT);
}

TEST(TestGenex0dInputManager, TestMissingTOCArgumentExit)
{
  std::vector<std::string> argvVector = generateFullInput();
  removeArgumentAndValueFromInput(argvVector, "-TOC");

  Genex0d::Genex0dInputManager inputMgr(argvVector);
  std::string ioErrorMssgActual = "";
  EXPECT_EQ(inputMgr.initialCheck(ioErrorMssgActual), Genex0d::Genex0dInputManager::NO_EXIT);
  EXPECT_EQ(inputMgr.storeInput(ioErrorMssgActual), Genex0d::Genex0dInputManager::WITH_ERROR_EXIT);
}

TEST(TestGenex0dInputManager, TestMissingHCArgumentExit)
{
  std::vector<std::string> argvVector = generateFullInput();
  removeArgumentAndValueFromInput(argvVector, "-HC");

  Genex0d::Genex0dInputManager inputMgr(argvVector);
  std::string ioErrorMssgActual = "";
  EXPECT_EQ(inputMgr.initialCheck(ioErrorMssgActual), Genex0d::Genex0dInputManager::NO_EXIT);
  EXPECT_EQ(inputMgr.storeInput(ioErrorMssgActual), Genex0d::Genex0dInputManager::WITH_ERROR_EXIT);
}

TEST(TestGenex0dInputManager, TestMissingSCArgumentExit)
{
  std::vector<std::string> argvVector = generateFullInput();
  removeArgumentAndValueFromInput(argvVector, "-SC");

  Genex0d::Genex0dInputManager inputMgr(argvVector);
  std::string ioErrorMssgActual = "";
  EXPECT_EQ(inputMgr.initialCheck(ioErrorMssgActual), Genex0d::Genex0dInputManager::NO_EXIT);
  EXPECT_EQ(inputMgr.storeInput(ioErrorMssgActual), Genex0d::Genex0dInputManager::WITH_ERROR_EXIT);
}

TEST(TestGenex0dInputManager, TestMissingEAArgumentExit)
{
  std::vector<std::string> argvVector = generateFullInput();
  removeArgumentAndValueFromInput(argvVector, "-EA");

  Genex0d::Genex0dInputManager inputMgr(argvVector);
  std::string ioErrorMssgActual = "";
  EXPECT_EQ(inputMgr.initialCheck(ioErrorMssgActual), Genex0d::Genex0dInputManager::NO_EXIT);
  EXPECT_EQ(inputMgr.storeInput(ioErrorMssgActual), Genex0d::Genex0dInputManager::WITH_ERROR_EXIT);
}

TEST(TestGenex0dInputManager, TestMissingAsphArgumentExit)
{
  std::vector<std::string> argvVector = generateFullInput();
  removeArgumentAndValueFromInput(argvVector, "-Asph");

  Genex0d::Genex0dInputManager inputMgr(argvVector);
  std::string ioErrorMssgActual = "";
  EXPECT_EQ(inputMgr.initialCheck(ioErrorMssgActual), Genex0d::Genex0dInputManager::NO_EXIT);
  EXPECT_EQ(inputMgr.storeInput(ioErrorMssgActual), Genex0d::Genex0dInputManager::WITH_ERROR_EXIT);
}

TEST(TestGenex0dInputManager, TestMissingResinArgumentExit)
{
  std::vector<std::string> argvVector = generateFullInput();
  removeArgumentAndValueFromInput(argvVector, "-Resin");

  Genex0d::Genex0dInputManager inputMgr(argvVector);
  std::string ioErrorMssgActual = "";
  EXPECT_EQ(inputMgr.initialCheck(ioErrorMssgActual), Genex0d::Genex0dInputManager::NO_EXIT);
  EXPECT_EQ(inputMgr.storeInput(ioErrorMssgActual), Genex0d::Genex0dInputManager::WITH_ERROR_EXIT);
}

TEST(TestGenex0dInputManager, TestMissingC15AroArgumentExit)
{
  std::vector<std::string> argvVector = generateFullInput();
  removeArgumentAndValueFromInput(argvVector, "-C15Aro");

  Genex0d::Genex0dInputManager inputMgr(argvVector);
  std::string ioErrorMssgActual = "";
  EXPECT_EQ(inputMgr.initialCheck(ioErrorMssgActual), Genex0d::Genex0dInputManager::NO_EXIT);
  EXPECT_EQ(inputMgr.storeInput(ioErrorMssgActual), Genex0d::Genex0dInputManager::WITH_ERROR_EXIT);
}

TEST(TestGenex0dInputManager, TestMissingC15SatArgumentExit)
{
  std::vector<std::string> argvVector = generateFullInput();
  removeArgumentAndValueFromInput(argvVector, "-C15Sat");

  Genex0d::Genex0dInputManager inputMgr(argvVector);
  std::string ioErrorMssgActual = "";
  EXPECT_EQ(inputMgr.initialCheck(ioErrorMssgActual), Genex0d::Genex0dInputManager::NO_EXIT);
  EXPECT_EQ(inputMgr.storeInput(ioErrorMssgActual), Genex0d::Genex0dInputManager::WITH_ERROR_EXIT);
}

TEST(TestGenex0dInputManager, TestMissingAdsFuncArgumentWhenAdsSimulatorEnabledExit)
{
  std::vector<std::string> argvVector = generateFullInput();
  removeArgumentAndValueFromInput(argvVector, "-AdsCapacityFunc");

  Genex0d::Genex0dInputManager inputMgr(argvVector);
  std::string ioErrorMssgActual = "";
  EXPECT_EQ(inputMgr.initialCheck(ioErrorMssgActual), Genex0d::Genex0dInputManager::NO_EXIT);
  EXPECT_EQ(inputMgr.storeInput(ioErrorMssgActual), Genex0d::Genex0dInputManager::WITH_ERROR_EXIT);
  EXPECT_EQ(ioErrorMssgActual, "An adsorption simulator was provided, but no adsorption capacity function");
}

TEST(TestGenex0dInputManager, TestMissingAdsSimulatorArgumentWhenOTCGEnabledExit)
{
  std::vector<std::string> argvVector = generateFullInput();
  removeArgumentAndValueFromInput(argvVector, "-AdsSimulator");
  removeArgumentAndValueFromInput(argvVector, "-AdsCapacityFunc");

  Genex0d::Genex0dInputManager inputMgr(argvVector);
  std::string ioErrorMssgActual = "";
  EXPECT_EQ(inputMgr.initialCheck(ioErrorMssgActual), Genex0d::Genex0dInputManager::NO_EXIT);
  EXPECT_EQ(inputMgr.storeInput(ioErrorMssgActual), Genex0d::Genex0dInputManager::WITH_ERROR_EXIT);
  EXPECT_EQ(ioErrorMssgActual, "OTGC was enabled, but no adsorption simulator was provided");
}

TEST(TestGenex0dInputManager, TestMissingAdsSimulatorArgumentWhenAdsFuncGivenExit)
{
  std::vector<std::string> argvVector = generateFullInput();
  removeArgumentAndValueFromInput(argvVector, "-AdsSimulator");
  removeArgumentAndValueFromInput(argvVector, "-doOTGC");

  Genex0d::Genex0dInputManager inputMgr(argvVector);
  std::string ioErrorMssgActual = "";
  EXPECT_EQ(inputMgr.initialCheck(ioErrorMssgActual), Genex0d::Genex0dInputManager::NO_EXIT);
  EXPECT_EQ(inputMgr.storeInput(ioErrorMssgActual), Genex0d::Genex0dInputManager::WITH_ERROR_EXIT);
  EXPECT_EQ(ioErrorMssgActual, "An adsorption capacity function was provided, but no adsorption simulator");
}

TEST(TestGenex0dInputManager, TestMissingAdsLangmuirTpvTableArgumentWhenAdsSimulatorGivenExit)
{
  std::vector<std::string> argvVector = generateFullInput();
  removeArgumentAndValueFromInput(argvVector, "-AdsLangmuirTpvTable");

  Genex0d::Genex0dInputManager inputMgr(argvVector);
  std::string ioErrorMssgActual = "";
  EXPECT_EQ(inputMgr.initialCheck(ioErrorMssgActual), Genex0d::Genex0dInputManager::NO_EXIT);
  EXPECT_EQ(inputMgr.storeInput(ioErrorMssgActual), Genex0d::Genex0dInputManager::WITH_ERROR_EXIT);
  EXPECT_EQ(ioErrorMssgActual, "An adsorption capacity function was provided, but no adsorption capacity function data");
}

TEST(TestGenex0dInputManager, TestMissingAdsIrrWatSatArgumentWhenAdsSimulatorGivenExit)
{
  std::vector<std::string> argvVector = generateFullInput();
  removeArgumentAndValueFromInput(argvVector, "-AdsIrrWatSat");

  Genex0d::Genex0dInputManager inputMgr(argvVector);
  std::string ioErrorMssgActual = "";
  EXPECT_EQ(inputMgr.initialCheck(ioErrorMssgActual), Genex0d::Genex0dInputManager::NO_EXIT);
  EXPECT_EQ(inputMgr.storeInput(ioErrorMssgActual), Genex0d::Genex0dInputManager::WITH_ERROR_EXIT);
  EXPECT_EQ(ioErrorMssgActual, "An adsorption capacity function was provided, but no irreducible water saturation data");
}

TEST(TestGenex0dInputManager, TestMissingLastArgument)
{
  std::vector<std::string> argvVector = generateFullInput();
  argvVector.pop_back();

  Genex0d::Genex0dInputManager inputMgr(argvVector);
  std::string ioErrorMssgActual = "";
  EXPECT_EQ(inputMgr.initialCheck(ioErrorMssgActual), Genex0d::Genex0dInputManager::NO_EXIT);
  EXPECT_EQ(inputMgr.storeInput(ioErrorMssgActual), Genex0d::Genex0dInputManager::WITH_ERROR_EXIT);
}

TEST(TestGenex0dInputManager, TestMissingSourceRockType2)
{
  std::vector<std::string> argvVector = generateFullInput();
  removeArgumentAndValueFromInput(argvVector, "-SRType_SR2");
  Genex0d::Genex0dInputManager inputMgr(argvVector);
  std::string ioErrorMssgActual = "";
  EXPECT_EQ(inputMgr.initialCheck(ioErrorMssgActual), Genex0d::Genex0dInputManager::NO_EXIT);
  EXPECT_EQ(inputMgr.storeInput(ioErrorMssgActual), Genex0d::Genex0dInputManager::WITH_ERROR_EXIT);
  EXPECT_EQ(ioErrorMssgActual, "No source rock type provided for source rock 2!");
}

TEST(TestGenex0dInputManager, TestMissingHC_SR2)
{
  std::vector<std::string> argvVector = generateFullInput();
  removeArgumentAndValueFromInput(argvVector, "-HC_SR2");

  Genex0d::Genex0dInputManager inputMgr(argvVector);
  std::string ioErrorMssgActual = "";
  EXPECT_EQ(inputMgr.initialCheck(ioErrorMssgActual), Genex0d::Genex0dInputManager::NO_EXIT);
  EXPECT_EQ(inputMgr.storeInput(ioErrorMssgActual), Genex0d::Genex0dInputManager::WITH_ERROR_EXIT);
  EXPECT_EQ(ioErrorMssgActual, "No H/C ratio provided for source rock 2!");
}

TEST(TestGenex0dInputManager, TestMissingSC_SR2)
{
  std::vector<std::string> argvVector = generateFullInput();
  removeArgumentAndValueFromInput(argvVector, "-SC_SR2");

  Genex0d::Genex0dInputManager inputMgr(argvVector);
  std::string ioErrorMssgActual = "";
  EXPECT_EQ(inputMgr.initialCheck(ioErrorMssgActual), Genex0d::Genex0dInputManager::NO_EXIT);
  EXPECT_EQ(inputMgr.storeInput(ioErrorMssgActual), Genex0d::Genex0dInputManager::WITH_ERROR_EXIT);
  EXPECT_EQ(ioErrorMssgActual, "No S/C ratio provided for source rock 2!");
}

TEST(TestGenex0dInputManager, TestMissingEA_SR2)
{
  std::vector<std::string> argvVector = generateFullInput();
  removeArgumentAndValueFromInput(argvVector, "-EA_SR2");

  Genex0d::Genex0dInputManager inputMgr(argvVector);
  std::string ioErrorMssgActual = "";
  EXPECT_EQ(inputMgr.initialCheck(ioErrorMssgActual), Genex0d::Genex0dInputManager::NO_EXIT);
  EXPECT_EQ(inputMgr.storeInput(ioErrorMssgActual), Genex0d::Genex0dInputManager::WITH_ERROR_EXIT);
  EXPECT_EQ(ioErrorMssgActual, "No activation energy provided for source rock 2!");
}

TEST(TestGenex0dInputManager, TestMissingAsph_SR2)
{
  std::vector<std::string> argvVector = generateFullInput();
  removeArgumentAndValueFromInput(argvVector, "-Asph_SR2");

  Genex0d::Genex0dInputManager inputMgr(argvVector);
  std::string ioErrorMssgActual = "";
  EXPECT_EQ(inputMgr.initialCheck(ioErrorMssgActual), Genex0d::Genex0dInputManager::NO_EXIT);
  EXPECT_EQ(inputMgr.storeInput(ioErrorMssgActual), Genex0d::Genex0dInputManager::WITH_ERROR_EXIT);
  EXPECT_EQ(ioErrorMssgActual, "No asphaltene diffusion energy provided for source rock 2!");
}

TEST(TestGenex0dInputManager, TestMissingResinSR2)
{
  std::vector<std::string> argvVector = generateFullInput();
  removeArgumentAndValueFromInput(argvVector, "-Resin_SR2");

  Genex0d::Genex0dInputManager inputMgr(argvVector);
  std::string ioErrorMssgActual = "";
  EXPECT_EQ(inputMgr.initialCheck(ioErrorMssgActual), Genex0d::Genex0dInputManager::NO_EXIT);
  EXPECT_EQ(inputMgr.storeInput(ioErrorMssgActual), Genex0d::Genex0dInputManager::WITH_ERROR_EXIT);
  EXPECT_EQ(ioErrorMssgActual, "No resin diffusion energy provided for source rock 2!");
}

TEST(TestGenex0dInputManager, TestMissingC15Aro_SR2)
{
  std::vector<std::string> argvVector = generateFullInput();
  removeArgumentAndValueFromInput(argvVector, "-C15Aro_SR2");

  Genex0d::Genex0dInputManager inputMgr(argvVector);
  std::string ioErrorMssgActual = "";
  EXPECT_EQ(inputMgr.initialCheck(ioErrorMssgActual), Genex0d::Genex0dInputManager::NO_EXIT);
  EXPECT_EQ(inputMgr.storeInput(ioErrorMssgActual), Genex0d::Genex0dInputManager::WITH_ERROR_EXIT);
  EXPECT_EQ(ioErrorMssgActual, "No C15 Aro diffusion energy provided for source rock 2!");
}

TEST(TestGenex0dInputManager, TestMissingC15Sat_SR2)
{
  std::vector<std::string> argvVector = generateFullInput();
  removeArgumentAndValueFromInput(argvVector, "-C15Sat_SR2");

  Genex0d::Genex0dInputManager inputMgr(argvVector);
  std::string ioErrorMssgActual = "";
  EXPECT_EQ(inputMgr.initialCheck(ioErrorMssgActual), Genex0d::Genex0dInputManager::NO_EXIT);
  EXPECT_EQ(inputMgr.storeInput(ioErrorMssgActual), Genex0d::Genex0dInputManager::WITH_ERROR_EXIT);
  EXPECT_EQ(ioErrorMssgActual, "No C15 Sat diffusion energy provided for source rock 2!");
}

TEST(TestGenex0dInputManager, TestMissingMixinHI_SR2)
{
  std::vector<std::string> argvVector = generateFullInput();
  removeArgumentAndValueFromInput(argvVector, "-MixingHI");

  Genex0d::Genex0dInputManager inputMgr(argvVector);
  std::string ioErrorMssgActual = "";
  EXPECT_EQ(inputMgr.initialCheck(ioErrorMssgActual), Genex0d::Genex0dInputManager::NO_EXIT);
  EXPECT_EQ(inputMgr.storeInput(ioErrorMssgActual), Genex0d::Genex0dInputManager::WITH_ERROR_EXIT);
  EXPECT_EQ(ioErrorMssgActual, "A second source rock was provided, but no mixing HI!");
}



#include "Genex0dSourceRock.h"

#include "ConstantsMathematics.h"
#include "ObjectFactory.h"
#include "ProjectHandle.h"

#include <gtest/gtest.h>

class TestGenex0dSourceRock : public ::testing::Test
{
protected:
  void SetUp() final
  {
    objectFactory = new DataAccess::Interface::ObjectFactory();
    projectHandle = DataAccess::Interface::OpenCauldronProject("AcquiferScale1.project3d", objectFactory);

  }

  void TearDown() final
  {
    delete projectHandle;
    delete objectFactory;
  }

  DataAccess::Interface::ObjectFactory* objectFactory;
  DataAccess::Interface::ProjectHandle* projectHandle;
};

Genex0d::Genex0dInputData setInputs()
{
  Genex0d::Genex0dInputData input;
  input.maxVes = 100;
  input.maxVesEnabled = true;
  input.HCVRe05 = 1.25;
  input.SCVRe05 = 0.0;
  input.formationName = "TestFormation";
  input.projectFilename = "AcquiferScale1.project3d";
  input.activationEnergy = 210;
  input.resinDiffusionEnergy = 80;
  input.C15AroDiffusionEnergy = 81;
  input.C15SatDiffusionEnergy = 82;
  input.asphalteneDiffusionEnergy = 83;

  return input;
}

Genex0d::Genex0dInputData setInputsWithAdsorption()
{
  Genex0d::Genex0dInputData input = setInputs();
  input.doOTCG = true;
  input.whichAdsorptionFunction = "Default Langmuir Isotherm";
  input.whichAdsorptionSimulator = "OTGCC1AdsorptionSimulator";

  return input;
}

Genex0d::Genex0dInputData setInputsWithSecondSourceRock()
{
  Genex0d::Genex0dInputData input = setInputs();
  input.HCVRe05SR2 = 1.25;
  input.SCVRe05SR2 = 0.0;
  input.activationEnergySR2 = 211;
  input.resinDiffusionEnergySR2 = 81;
  input.C15AroDiffusionEnergySR2 = 83;
  input.C15SatDiffusionEnergySR2 = 84;
  input.asphalteneDiffusionEnergySR2 = 85;
  input.sourceRockTypeSR2 = "Type_II_Paleozoic_Marine_Shale_kin_s";

  return input;
}


TEST_F( TestGenex0dSourceRock, TestInputs )
{
  // Given
  Genex0d::Genex0dInputData input = setInputsWithAdsorption();

  // When
  std::unique_ptr<Genex0d::Genex0dSourceRock> sourceRock(new Genex0d::Genex0dSourceRock(*projectHandle, input, 0, 0));

  // Then
  EXPECT_DOUBLE_EQ(input.maxVes * Utilities::Maths::MegaPaToPa, sourceRock->getVESMax());
  EXPECT_TRUE(input.maxVesEnabled);
  EXPECT_DOUBLE_EQ(input.HCVRe05, sourceRock->getHcVRe05());
  EXPECT_DOUBLE_EQ(input.SCVRe05, sourceRock->getScVRe05());
  EXPECT_EQ(input.sourceRockType, sourceRock->getType());
  EXPECT_EQ(input.formationName, sourceRock->getLayerName());
  EXPECT_EQ(input.projectFilename, sourceRock->getProjectHandle().getFileName());
  EXPECT_EQ(input.activationEnergy, sourceRock->getPreAsphaltStartAct());
  EXPECT_EQ(input.activationEnergy, sourceRock->getPreAsphaltStartAct());
  EXPECT_EQ(input.resinDiffusionEnergy, sourceRock->getResinDiffusionEnergy());
  EXPECT_EQ(input.C15AroDiffusionEnergy, sourceRock->getC15AroDiffusionEnergy());
  EXPECT_EQ(input.C15SatDiffusionEnergy, sourceRock->getC15SatDiffusionEnergy());
  EXPECT_EQ(input.asphalteneDiffusionEnergy, sourceRock->getAsphalteneDiffusionEnergy());
  EXPECT_EQ(input.doOTCG, sourceRock->doComputeOTGC());
  EXPECT_EQ(true, sourceRock->doApplyAdsorption());
  EXPECT_EQ(input.whichAdsorptionFunction, sourceRock->getAdsorptionCapacityFunctionName());
  EXPECT_EQ(input.whichAdsorptionSimulator, sourceRock->getAdsorptionSimulatorName());
}

TEST_F( TestGenex0dSourceRock, TestInputsWithoutAdsorption )
{
  // Given
  Genex0d::Genex0dInputData input = setInputs();

  // When
  std::unique_ptr<Genex0d::Genex0dSourceRock> sourceRock(new Genex0d::Genex0dSourceRock(*projectHandle, input, 0, 0));

  // Then
  EXPECT_EQ(false, sourceRock->doApplyAdsorption());
}

TEST_F( TestGenex0dSourceRock, TestInputsWithSecondSourceRock)
{
  // Given
  Genex0d::Genex0dInputData input = setInputsWithSecondSourceRock();

  // When
  std::unique_ptr<Genex0d::Genex0dSourceRock> sourceRock(new Genex0d::Genex0dSourceRock(*projectHandle, input, 0, 0));
  sourceRock->initialize();

  // Then
  EXPECT_FALSE(sourceRock->getChemicalModel2() == nullptr);
}

TEST_F( TestGenex0dSourceRock, TestInputsWithoutSecondSourceRock)
{
  // Given
  Genex0d::Genex0dInputData input = setInputs();

  // When
  std::unique_ptr<Genex0d::Genex0dSourceRock> sourceRock(new Genex0d::Genex0dSourceRock(*projectHandle, input, 0, 0));
  sourceRock->initialize();

  // Then
  EXPECT_TRUE(sourceRock->getChemicalModel2() == nullptr);
}


TEST_F( TestGenex0dSourceRock, TestTypenames )
{
  // Given
  Genex0d::Genex0dInputData input = setInputsWithAdsorption();
  input.sourceRockType = "Type_I_II_Cenozoic_Marine_Marl_lit_s";

  // When
  std::unique_ptr<Genex0d::Genex0dSourceRock> sourceRock(new Genex0d::Genex0dSourceRock(*projectHandle, input, 0, 0));

  // Then
  EXPECT_EQ("TypeIIN", sourceRock->getTypeID());
}

TEST_F( TestGenex0dSourceRock, TestNonValidTypenames )
{
  // Given
  Genex0d::Genex0dInputData input = setInputsWithAdsorption();
  input.sourceRockType = "NotValid";

  // When
  std::unique_ptr<Genex0d::Genex0dSourceRock> sourceRock(new Genex0d::Genex0dSourceRock(*projectHandle, input, 0, 0));

  // Then
  EXPECT_EQ("", sourceRock->getTypeID());
}





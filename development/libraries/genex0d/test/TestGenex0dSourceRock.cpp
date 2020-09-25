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

genex0d::Genex0dInputData setInputs()
{
  genex0d::Genex0dInputData input;
  input.maxVes = 100;
  input.maxVesEnabled = true;
  input.HCVRe05 = 1.25;
  input.SCVRe05 = 0.0;
  input.formationName = "TestFormation";
  input.projectFilename = "AcquiferScale1.project3d";
  input.activationEnergy = 210000;
  input.resinDiffusionEnergy = 80000;
  input.C15AroDiffusionEnergy = 81000;
  input.C15SatDiffusionEnergy = 82000;
  input.asphalteneDiffusionEnergy = 83000;

  return input;
}

TEST_F( TestGenex0dSourceRock, TestInputs )
{
  // Given
  genex0d::Genex0dInputData input = setInputs();

  // When
  std::unique_ptr<genex0d::Genex0dSourceRock> sourceRock(new genex0d::Genex0dSourceRock(*projectHandle, input));

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
}

TEST_F( TestGenex0dSourceRock, TestTypenames )
{
  // Given
  genex0d::Genex0dInputData input = setInputs();
  input.sourceRockType = "Type_I_II_Cenozoic_Marine_Marl_lit_s";

  // When
  std::unique_ptr<genex0d::Genex0dSourceRock> sourceRock(new genex0d::Genex0dSourceRock(*projectHandle, input));

  // Then
  EXPECT_EQ("TypeIIN", sourceRock->getTypeID());
}

TEST_F( TestGenex0dSourceRock, TestNonValidTypenames )
{
  // Given
  genex0d::Genex0dInputData input = setInputs();
  input.sourceRockType = "NotValid";

  // When
  std::unique_ptr<genex0d::Genex0dSourceRock> sourceRock(new genex0d::Genex0dSourceRock(*projectHandle, input));

  // Then
  EXPECT_EQ("", sourceRock->getTypeID());
}





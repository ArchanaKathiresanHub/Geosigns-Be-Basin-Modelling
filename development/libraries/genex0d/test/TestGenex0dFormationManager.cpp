#include "Genex0dFormationManager.h"

#include <gtest/gtest.h>

namespace
{

const double epsilon = 1e-6;
const std::string formationName = "Formation3";
const double xCoord = 2005.0;
const double yCoord = 8456.0;

} // namespace

//class TestGenex0dFormationManager : public ::testing::Test
//{
//protected:
//  void SetUp() final
//  {
//    ASSERT_NO_THROW(genex0dFormationMgr = new genex0d::Genex0dFormationManager(
//          projectFileName, formationName, xCoord, yCoord));
//  }

//  void TearDown() final
//  {
//    delete genex0dFormationMgr;
//  }

//  static const std::string projectFileName;
//  genex0d::Genex0dFormationManager * genex0dFormationMgr;
//};
//const std::string TestGenex0dFormationManager::projectFileName = "AcquiferScale1.project3d";

//TEST_F( TestGenex0dFormationManager, TestFunctionalities )
//{
//  const double thicknessExpected = 777.599853;
//  const double thicknessActual = genex0dFormationMgr->getThickness();
//  EXPECT_NEAR(thicknessExpected, thicknessActual, epsilon);

//  const double inorganicDensityExpected = 2650.0;
//  const double inorganicDensityActual = genex0dFormationMgr->getInorganicDensity();
//  EXPECT_NEAR(inorganicDensityExpected, inorganicDensityActual, epsilon);

//  const std::string topSurfaceNameExpected = "Surface2";
//  const std::string topSurfaceNameActual = genex0dFormationMgr->getTopSurfaceName();
//  EXPECT_EQ(topSurfaceNameExpected, topSurfaceNameActual);

//  const bool isFormationSourceRockExpected = false;
//  const bool isFormationSourceRockActual = genex0dFormationMgr->isFormationSourceRock();
//  EXPECT_TRUE(isFormationSourceRockExpected == isFormationSourceRockActual);
//}

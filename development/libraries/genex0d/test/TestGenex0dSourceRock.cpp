#include "Genex0dSourceRock.h"

#include "Genex0dFormationManager.h"
#include "Genex0dProjectManager.h"

#include <gtest/gtest.h>

namespace
{

const double thickness = 372.4;
const double inorganicDensity = 2650.0;
const std::vector<double> ages = {0, 2, 4, 6};
const std::vector<double> temperatures = {135.58, 128.9, 121.8, 114.16};
const std::vector<double> pressures = {38.46, 36.40, 34.29, 32.08};

} // namespace

//class TestGenex0dSourceRock : public ::testing::Test
//{
//protected:
//  void SetUp() final
//  {
//    ASSERT_NO_THROW(genex0dSR = new genex0d::Genex0dSourceRock(sourceRockType,
//                                                               (const)genex0d::Genex0dProjectManager("", 0.0, 0.0, ""),
//                                                               genex0d::Genex0dFormationManager("", "",0.0, 0.0) ));
//  }

//  void TearDown() final
//  {
//    delete genex0dSR;
//  }

//  static const std::string sourceRockType;
//  genex0d::Genex0dSourceRock * genex0dSR;
//};
//const std::string TestGenex0dSourceRock::sourceRockType = "Type I - Lacustrine";

//TEST_F( TestGenex0dSourceRock, TestExtractData )
//{
//  EXPECT_NO_THROW(genex0dSR->setToCIni(10.0));
//  EXPECT_NO_THROW(genex0dSR->setSCVRe05(0.0));
//  EXPECT_NO_THROW(genex0dSR->setHCVRe05(1.2));
//  EXPECT_NO_THROW(genex0dSR->setActivationEnergy(216.0));
//  EXPECT_NO_THROW(genex0dSR->computeData(thickness, inorganicDensity, ages, temperatures, pressures));
//}

#include "Genex0dProjectManager.h"

#include <gtest/gtest.h>

namespace
{

const double epsilon = 1e-6;
const std::string formationName = "Formation6";
const std::string topSurfaceName = "Surface5";
const double xCoord = 2005.0;
const double yCoord = 8456.0;

} // namespace

class TestGenex0dProjectManager : public ::testing::Test
{
protected:
  void SetUp() final
  {
    ASSERT_NO_THROW(genex0dProjectMgr =
        new genex0d::Genex0dProjectManager(projectFileName, xCoord, yCoord, topSurfaceName));
  }

  void TearDown() final
  {
    delete genex0dProjectMgr;
  }

  static const std::string projectFileName;
  genex0d::Genex0dProjectManager * genex0dProjectMgr;
};
const std::string TestGenex0dProjectManager::projectFileName = "AcquiferScale1.project3d";

TEST_F( TestGenex0dProjectManager, TestAgesFromMajorSnapShots )
{
  std::vector<double> agesActual;
  EXPECT_NO_THROW(agesActual = genex0dProjectMgr->agesFromMajorSnapShots());

  std::vector<double> agesExpected = {0.0, 8.0, 14.0, 23.0, 27.0, 33.0 ,45.0};

  int i = 0;
  for (auto tExpected : agesExpected)
  {
    EXPECT_NEAR(tExpected, agesActual[i], epsilon);
    ++i;
  }
}

TEST_F( TestGenex0dProjectManager, TestRequestPropertyHistory )
{
  std::string propertyName = "Temperature";
  std::vector<double> temperaturesActual;
  EXPECT_NO_THROW(temperaturesActual = genex0dProjectMgr->requestPropertyHistory(propertyName));

  std::vector<double> temperaturesExpected = {135.579962, 105.8059608, 67.5628889, 55.0993285, 29.3703104, 10.0, -9999.0};

  int i = 0;
  for (auto tempExpected : temperaturesExpected)
  {
    EXPECT_NEAR(tempExpected, temperaturesActual[i], epsilon);
    ++i;
  }

  propertyName = "Pressure";
  std::vector<double> pressuresActual;
  EXPECT_NO_THROW(pressuresActual = genex0dProjectMgr->requestPropertyHistory(propertyName));

  std::vector<double> pressuresExpected = {38.4593049, 29.7325433, 18.9847977, 11.1891257, 6.2057049, 0.1, -9999.0};

  i = 0;
  for (auto pExpected : pressuresExpected)
  {
    EXPECT_NEAR(pExpected, pressuresActual[i], epsilon);
    ++i;
  }
}

#include "Genex0dFormationManager.h"

#include "ObjectFactory.h"
#include "ProjectHandle.h"

#include <gtest/gtest.h>

namespace
{

const double epsilon = 1e-6;
const std::string formationName = "Formation3";
const double xCoord = 2005.0;
const double yCoord = 8456.0;

} // namespace

class TestGenex0dFormationManager : public ::testing::Test
{
protected:
  void SetUp() final
  {
    objectFactory = new DataAccess::Interface::ObjectFactory();
    projectHandle = DataAccess::Interface::OpenCauldronProject(projectFileName, objectFactory);
    ASSERT_NO_THROW(genex0dFormationMgr = new Genex0d::Genex0dFormationManager(
          *projectHandle, formationName, xCoord, yCoord));
  }

  void TearDown() final
  {
    delete genex0dFormationMgr;
    delete projectHandle;
    delete objectFactory;
  }

  static const std::string projectFileName;
  DataAccess::Interface::ObjectFactory * objectFactory;
  DataAccess::Interface::ProjectHandle * projectHandle;
  Genex0d::Genex0dFormationManager * genex0dFormationMgr;
};
const std::string TestGenex0dFormationManager::projectFileName = "AcquiferScale1.project3d";

TEST_F( TestGenex0dFormationManager, TestFunctionalities )
{
  const double thicknessExpected = 777.599853;
  const double thicknessActual = genex0dFormationMgr->getThickness();
  EXPECT_NEAR(thicknessExpected, thicknessActual, epsilon);

  const double inorganicDensityExpected = 2650.0;
  const double inorganicDensityActual = genex0dFormationMgr->getInorganicDensity();
  EXPECT_NEAR(inorganicDensityExpected, inorganicDensityActual, epsilon);

  const std::string topSurfaceNameExpected = "Surface2";
  const std::string topSurfaceNameActual = genex0dFormationMgr->topSurfaceName();
  EXPECT_EQ(topSurfaceNameExpected, topSurfaceNameActual);

  const bool isFormationSourceRockExpected = false;
  const bool isFormationSourceRockActual = genex0dFormationMgr->isFormationSourceRock();
  EXPECT_TRUE(isFormationSourceRockExpected == isFormationSourceRockActual);

  const double depoAgeExpected = 14.0;
  const double depoAgeActual = genex0dFormationMgr->depositionTimeTopSurface();
  EXPECT_NEAR(depoAgeExpected, depoAgeActual, epsilon);
}

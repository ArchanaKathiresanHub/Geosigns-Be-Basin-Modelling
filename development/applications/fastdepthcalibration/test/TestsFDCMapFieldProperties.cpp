#include "FDCMapFieldProperties.h"

#include <gtest/gtest.h>

class TestFDCMapFieldProperties : public ::testing::Test
{
protected:
  void SetUp() final
  {
    mdl.reset(new mbapi::Model());
    ASSERT_EQ(ErrorHandler::NoError, mdl->loadModelFromProjectFile(projectFileName.c_str()));

    fdcMFP = std::unique_ptr<fastDepthCalibration::FDCMapFieldProperties>(
          new fastDepthCalibration::FDCMapFieldProperties(mdl, referenceSurface, endSurface));
  }

  std::unique_ptr<fastDepthCalibration::FDCMapFieldProperties> fdcMFP;
  std::shared_ptr<mbapi::Model> mdl;
  static const std::string resultsMapFileName;
  static const std::string projectFileName;
  static const mbapi::StratigraphyManager::SurfaceID referenceSurface;
  static const mbapi::StratigraphyManager::SurfaceID endSurface;
};
const std::string TestFDCMapFieldProperties::resultsMapFileName = "CalibratedInputs.HDF";
const std::string TestFDCMapFieldProperties::projectFileName = "Project.project3d";
const mbapi::StratigraphyManager::SurfaceID TestFDCMapFieldProperties::referenceSurface = 2;
const mbapi::StratigraphyManager::SurfaceID TestFDCMapFieldProperties::endSurface = 4;

TEST_F( TestFDCMapFieldProperties, TestCalculateInitialMaps )
{
  EXPECT_NO_THROW(fdcMFP->calculateInitialMaps(std::string("./MasterInputs.HDF"), false));
}

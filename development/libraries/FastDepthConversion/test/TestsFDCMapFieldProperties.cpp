#include "FDCMapFieldProperties.h"

#include "FDCProjectManager.h"

#include <gtest/gtest.h>

class TestFDCMapFieldProperties : public ::testing::Test
{
protected:
  void SetUp() final
  {
    projectManager = std::unique_ptr<fastDepthConversion::FDCProjectManager>(
                       new fastDepthConversion::FDCProjectManager(projectFileName));

    fdcMFP = std::unique_ptr<fastDepthConversion::FDCMapFieldProperties>(
          new fastDepthConversion::FDCMapFieldProperties(*projectManager, referenceSurface, endSurface));
  }

  std::unique_ptr<fastDepthConversion::FDCProjectManager> projectManager;
  std::unique_ptr<fastDepthConversion::FDCMapFieldProperties> fdcMFP;
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

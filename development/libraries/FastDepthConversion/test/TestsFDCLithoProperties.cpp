#include "FDCLithoProperties.h"

#include <gtest/gtest.h>

namespace
{

static const double epsilon = 1e-6;

} // namespace

class TestsFDCLithoProperties : public ::testing::Test
{
protected:
  void SetUp() final
  {
    mdl.reset(new mbapi::Model());
    ASSERT_EQ(ErrorHandler::NoError, mdl->loadModelFromProjectFile(projectFileName));

    fdcLP = std::unique_ptr<fastDepthConversion::FDCLithoProperties>(
          new fastDepthConversion::FDCLithoProperties(mdl->stratigraphyManager(), mdl->lithologyManager()));
  }

  std::unique_ptr<fastDepthConversion::FDCLithoProperties> fdcLP;
  std::shared_ptr<mbapi::Model> mdl;
  static const std::string projectFileName;
};
const std::string TestsFDCLithoProperties::projectFileName = "Project.project3d";

TEST_F( TestsFDCLithoProperties, TestSetLithoSurfaces )
{
  mbapi::StratigraphyManager::SurfaceID currentSurface = 3;
  fdcLP->setLithoSurfaces(currentSurface);

  mbapi::StratigraphyManager & stMgrLocal = mdl->stratigraphyManager();
  mbapi::StratigraphyManager::SurfaceID nextSurfaceExpected = currentSurface + 1;
  mbapi::StratigraphyManager::LayerID currentLayerExpected = currentSurface;
  mbapi::StratigraphyManager::LayerID nextLayerExpected = nextSurfaceExpected;
  std::string currentTopNameExpected = stMgrLocal.surfaceName(currentLayerExpected);
  std::string nextTopNameExpected = stMgrLocal.surfaceName(nextLayerExpected);
  std::string currentLayerNameExpected = stMgrLocal.layerName(currentLayerExpected);

  EXPECT_EQ(nextSurfaceExpected, fdcLP->nextSurface());
  EXPECT_EQ(currentLayerExpected, fdcLP->currentLayer());
  EXPECT_EQ(currentTopNameExpected, fdcLP->currentTopName());
  EXPECT_EQ(nextTopNameExpected, fdcLP->nextTopName());
  EXPECT_EQ(currentLayerNameExpected, fdcLP->currentLayerName());
}

TEST_F( TestsFDCLithoProperties, TestSetlayerLithologiesListForCurrentLayer )
{
  mbapi::StratigraphyManager::SurfaceID currentSurface = 3;
  fdcLP->setLithoSurfaces(currentSurface);
  EXPECT_NO_THROW(fdcLP->setlayerLithologiesListForCurrentLayer());
}

TEST_F( TestsFDCLithoProperties, TestGetMaxSeismicVelocityForCurrentLayer )
{
  mbapi::StratigraphyManager::SurfaceID currentSurface = 3;
  fdcLP->setLithoSurfaces(currentSurface);
  fdcLP->setlayerLithologiesListForCurrentLayer();

  double maxSeisVelExpected = 6097.0;
  double maxSeisVelActual = fdcLP->getMaxSeismicVelocityForCurrentLayer();
  EXPECT_NEAR(maxSeisVelExpected, maxSeisVelActual, epsilon);
}

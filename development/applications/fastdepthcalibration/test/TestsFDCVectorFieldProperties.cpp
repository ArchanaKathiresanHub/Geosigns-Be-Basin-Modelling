#include "FDCVectorFieldProperties.h"

#include <gtest/gtest.h>

namespace
{

static const double epsilon = 1e-6;

} // namespace

class TestFDCVectorFieldProperties : public ::testing::Test
{
protected:
  void SetUp() final
  {
    mdl.reset(new mbapi::Model());
    ASSERT_EQ(ErrorHandler::NoError, mdl->loadModelFromProjectFile(projectFileName.c_str()));

    fdcVFP = std::unique_ptr<fastDepthCalibration::FDCVectorFieldProperties>(
          new fastDepthCalibration::FDCVectorFieldProperties(mdl, referenceSurface));
  }

  std::unique_ptr<fastDepthCalibration::FDCVectorFieldProperties> fdcVFP;
  std::shared_ptr<mbapi::Model> mdl;
  static const std::string projectFileName;
  static const mbapi::StratigraphyManager::SurfaceID referenceSurface;
};
const std::string TestFDCVectorFieldProperties::projectFileName = "Project.project3d";
const mbapi::StratigraphyManager::SurfaceID TestFDCVectorFieldProperties::referenceSurface = 2;

TEST_F( TestFDCVectorFieldProperties, TestSetMeasuredTwtAtSpecifiedSurface )
{

  std::vector<double> tarTwtsExpected;
  mbapi::StratigraphyManager::SurfaceID surfaceID = 3;
  const std::string twtMapNames = "MAP-1520799820-4";
  std::vector<double> tarTwtsActual = fdcVFP->setMeasuredTwtAtSpecifiedSurface(surfaceID, twtMapNames);

  mbapi::MapsManager & mapsMgrLocal = mdl->mapsManager();
  mbapi::MapsManager::MapID twtMapID = mapsMgrLocal.findID(twtMapNames);
  EXPECT_NO_THROW(mapsMgrLocal.mapGetValues(twtMapID, tarTwtsExpected));

  int i = 0;
  for (auto twt : tarTwtsExpected)
  {
    EXPECT_NEAR(twt, tarTwtsActual[i], epsilon);
    ++i;
  }
}

TEST_F( TestFDCVectorFieldProperties, TestSetTopSurfaceProperties )
{
  // for reference surface
  mbapi::StratigraphyManager::SurfaceID surfaceID = 2;
  const std::string twtMapNames = "MAP-1520799820-4";

  EXPECT_NO_THROW(fdcVFP->setTopSurfaceProperties(surfaceID, 2, twtMapNames));
  std::vector<double> refDepthsExpected;
  std::vector<double> refTwtsExpected;
  EXPECT_NO_THROW(mdl->getGridMapDepthValues(surfaceID, refDepthsExpected));
  mbapi::MapsManager & mapsMgrLocal = mdl->mapsManager();
  mbapi::MapsManager::MapID twtMapID = mapsMgrLocal.findID(twtMapNames);
  EXPECT_NO_THROW(mapsMgrLocal.mapGetValues(twtMapID, refTwtsExpected));

  std::vector<double> refDepthActual = fdcVFP->refDepths();
  int i = 0;
  for (auto depth : refDepthsExpected)
  {
    EXPECT_NEAR(depth, refDepthActual[i], epsilon);
    ++i;
  }

  std::vector<double> refTwtsActual = fdcVFP->refTwts();
  i = 0;
  for (auto twt : refTwtsExpected)
  {
    EXPECT_NEAR(twt, refTwtsActual[i], epsilon);
    ++i;
  }

  // for a surface that is not reference surface
  surfaceID = 3;
  EXPECT_NO_THROW(fdcVFP->setTopSurfaceProperties(surfaceID, 2, twtMapNames));
  refDepthsExpected = fdcVFP->newDepths();
  refTwtsExpected = fdcVFP->tarTwts();
  refDepthActual = fdcVFP->refDepths();
  refTwtsActual = fdcVFP->refTwts();
  i = 0;
  for (auto depth : refDepthsExpected)
  {
    EXPECT_NEAR(depth, refDepthActual[i], epsilon);
    ++i;
  }

  i = 0;
  for (auto twt : refTwtsExpected)
  {
    EXPECT_NEAR(twt, refTwtsActual[i], epsilon);
    ++i;
  }
}

TEST_F( TestFDCVectorFieldProperties, TestCalculateIncreasedDepthsIncludingPreservedErosion )
{
  mbapi::StratigraphyManager::SurfaceID surfaceID = 2;
  mbapi::StratigraphyManager::SurfaceID nextSurface = 3;
  const std::string twtMapNames = "MAP-1520799820-4";

  EXPECT_NO_THROW(fdcVFP->setTopSurfaceProperties(surfaceID, 2, twtMapNames));
  std::vector<double> refDepthsExpected;
  std::vector<double> refTwtsExpected;
  EXPECT_NO_THROW(mdl->getGridMapDepthValues(surfaceID, refDepthsExpected));
  mbapi::MapsManager & mapsMgrLocal = mdl->mapsManager();
  mbapi::MapsManager::MapID twtMapID = mapsMgrLocal.findID(twtMapNames);
  EXPECT_NO_THROW(mapsMgrLocal.mapGetValues(twtMapID, refTwtsExpected));

  const std::string twtMapNamesNextSurface = "MAP-1520799824-4";
  const std::vector<double> tarTwts = fdcVFP->setMeasuredTwtAtSpecifiedSurface(nextSurface, twtMapNamesNextSurface);
  fdcVFP->setTarTwts(tarTwts);

  std::ifstream ifs;
  ifs.open("increasedDepthsSurf3.txt");
  ASSERT_FALSE(ifs.fail()) << "coulnd not open data file increasedDepthsSurf3.txt!";

  std::vector<double> increasedDepthsExpected;
  double tmp;
  while (ifs >> tmp)
  {
    increasedDepthsExpected.push_back(tmp);
  }
  ifs.close();

  const double maxSeismicVelocity = 6097;
  std::vector<double> increasedDepthsActual = fdcVFP->calculateIncreasedDepthsIncludingPreservedErosion(surfaceID, maxSeismicVelocity);

  int i = 0;
  for (auto depth : increasedDepthsActual)
  {
    EXPECT_NEAR(depth, increasedDepthsExpected[i], epsilon) << "Value mismatch at index " << i << "\n";
    ++i;
  }
}

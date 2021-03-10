#include "FDCProjectManager.h"

#include "propinterface.h"

#include <gtest/gtest.h>

class TestFDCProjectManager : public ::testing::Test
{
protected:
  void SetUp() final
  {
    PetscInitialize( 0, nullptr, (char *)0, PETSC_IGNORE );
    fdcPM = std::unique_ptr<fastDepthConversion::FDCProjectManager>(new fastDepthConversion::FDCProjectManager(projectFileName));
  }

  std::unique_ptr<fastDepthConversion::FDCProjectManager> fdcPM;
  static const std::string resultsMapFileName;
  static const std::string projectFileName;
};

const std::string TestFDCProjectManager::resultsMapFileName = "CalibratedInputs.HDF";
const std::string TestFDCProjectManager::projectFileName = "Project.project3d";

TEST_F( TestFDCProjectManager, TestModel )
{
  EXPECT_NO_THROW(fdcPM->reloadModel("./Project.project3d"));
  EXPECT_TRUE(fdcPM->getModel() != nullptr);
}

TEST_F( TestFDCProjectManager, TestPrepareProject )
{
  EXPECT_NO_THROW(fdcPM->prepareProject());
}

TEST_F( TestFDCProjectManager, TestModifyTables )
{
  std::map<const mbapi::StratigraphyManager::SurfaceID, int> hiatus;
  std::map<const mbapi::StratigraphyManager::SurfaceID, std::string> correctedMapsNames;
  std::map<const mbapi::StratigraphyManager::SurfaceID, int> correctedMapsSequenceNbr;
  const std::vector<int> addedTwtmapsequenceNbr = {};
  const std::vector<mbapi::StratigraphyManager::SurfaceID> surfaceIDs =
  {0, 1, 2, 3, 4, 5, 6,};

  for (mbapi::StratigraphyManager::SurfaceID s = 2; s <= 4; ++s)
  {
    correctedMapsNames[s] = "";
    correctedMapsSequenceNbr[s] = -1;
    hiatus[s] = -1;
  }

  correctedMapsNames[3] = "T2Z[2019-Jul-26 11:34:00][45_TR_T_TDinput_SD]";
  correctedMapsSequenceNbr[3] = 0;

  const mbapi::StratigraphyManager::SurfaceID surfaceID = 4;
  EXPECT_NO_THROW(fdcPM->modifyTables(surfaceID, hiatus, correctedMapsNames, correctedMapsSequenceNbr, addedTwtmapsequenceNbr,
                                      resultsMapFileName, surfaceIDs, false));
}

TEST_F( TestFDCProjectManager, TestGenerateMapInGridMapIoTbl )
{
  const std::string mapName = "T2Z[2019-Jul-26 12:39:26][50_ZE_T_TDinput_SD]";
  const std::vector<double> increasedDepths(6527, 2000);
  const mbapi::StratigraphyManager::SurfaceID surfaceID = 4;
  std::size_t mapsSequenceNbrExpected = 0;
  std::size_t mapsSequenceNbrActual;

  EXPECT_NO_THROW(fdcPM->generateMapInGridMapIoTbl(mapName, increasedDepths, mapsSequenceNbrActual, resultsMapFileName, surfaceID));
  EXPECT_EQ(mapsSequenceNbrExpected, mapsSequenceNbrActual);
}

TEST_F( TestFDCProjectManager, TestSetMapNameInStratIoTbl )
{
  const std::string mapName = "T2Z[2019-Jul-26 12:39:26][50_ZE_T_TDinput_SD]";
  const mbapi::StratigraphyManager::SurfaceID surfaceID = 4;
  EXPECT_NO_THROW(fdcPM->setMapNameInStratIoTbl(surfaceID, mapName));
}

TEST_F( TestFDCProjectManager, TestGenerateMapInTwoWayTimeIoTbl )
{
  const std::vector<double> tarTwts(6527, 111);
  size_t mapsSequenceNbrExpected = 0;
  size_t mapsSequenceNbrActual;

  fdcPM->appendToAddedTwtMapNames("45_TR_T_TDinput_SD");
  EXPECT_EQ(fdcPM->addedTwtMapNames().back(), "Calculated_TWT_for_45_TR_T_TDinput_SD");

  EXPECT_NO_THROW(fdcPM->generateMapInTwoWayTimeIoTbl(tarTwts, mapsSequenceNbrActual, resultsMapFileName));
  EXPECT_EQ(mapsSequenceNbrExpected, mapsSequenceNbrActual);
}

TEST_F( TestFDCProjectManager, TestSetAlternativeTableNames )
{
  EXPECT_NO_THROW(fdcPM->setAlternativeTableNames("2019-Jul-26 12:39:26"));
}

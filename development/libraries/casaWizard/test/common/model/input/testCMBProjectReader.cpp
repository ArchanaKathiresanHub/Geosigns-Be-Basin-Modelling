//
// Copyright (C) 2020 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "model/input/cmbProjectReader.h"
#include "ConstantsNumerical.h"
#include "Interface.h"

#include <gtest/gtest.h>
#include <numeric>
#include <stdio.h>

using namespace Utilities::Numerical;

const double epsilon = std::numeric_limits<double>::epsilon();

class CMBProjectReaderTest : public ::testing::Test
{
public:
  casaWizard::CMBProjectReader reader_{};
  CMBProjectReaderTest()
  {
    reader_.load(QString("CMBProjectReaderTest.project3d"));
  }
};

TEST_F( CMBProjectReaderTest, testMapNames )
{
  const QStringList mapNamesActual = reader_.mapNames();
  const QStringList mapNamesExpected =
  {
    "MAP-1511782103-4",
    "MAP-1511782105-4",
    "MAP-1511782107-4",
    "MAP-1511782109-4",
    "MAP-1511782111-4",
    "MAP-1511782113-4",
    "MAP-1511782115-4",
    "MAP-1511782117-4",
    "T2Z[50_ZE_T_TDinput_SD]",
    "MAP-1511782121-4",
    "MAP-1511782123-4",
    "MAP-1511782125-4",
    "MAP-1511782127-4",
    "MAP-1511782129-4",
    "HeatMap"
  };

  const int nMapsActual = mapNamesActual.size();
  const int nMapsExpected = mapNamesActual.size();
  ASSERT_EQ(nMapsExpected, nMapsActual);

  for(int i = 0; i< nMapsActual; ++i)
  {
    EXPECT_EQ(mapNamesExpected[i], mapNamesActual[i]) << "Mismatch at entry [" << i << "]";
  }
}

TEST_F( CMBProjectReaderTest, testLithologyNames )
{
  const QStringList lithologyNamesActual = reader_.lithologyNames();
  const QStringList lithologyNamesExpected =
  {
    "Crust",
    "Litho. Mantle",
    "Astheno. Mantle",
    "Std. Basalt",
    "LSB Sand Hybrid LK03042015",
    "LSB Shale Hybrid LK03042015",
    "LSB Chalk Hybrid LK03042015",
    "Congo_Salt_Hybrid_LK25112015"
  };

  const int nLithoActual = lithologyNamesActual.size();
  const int nLithoExpected = lithologyNamesExpected.size();
  ASSERT_EQ(nLithoExpected, nLithoActual);

  for(int i = 0; i< nLithoActual; ++i)
  {
    EXPECT_EQ(lithologyNamesExpected[i], lithologyNamesActual[i]) << "Mismatch at entry [" << i << "]";
  }
}

TEST_F( CMBProjectReaderTest, testReadDepth )
{
   //not loaded
   casaWizard::CMBProjectReader readerNotLoaded{};
   EXPECT_EQ(readerNotLoaded.getDepth(4),Utilities::Numerical::IbsNoDataValue);

   //Read from table
   EXPECT_EQ(reader_.getDepth(4),Utilities::Numerical::IbsNoDataValue);

   //Out of table range
   EXPECT_EQ(readerNotLoaded.getDepth(20),Utilities::Numerical::IbsNoDataValue);
}

TEST_F( CMBProjectReaderTest, testreadThickness )
{
   //not loaded
   casaWizard::CMBProjectReader readerNotLoaded{};
   EXPECT_EQ(readerNotLoaded.getThickness(4),Utilities::Numerical::IbsNoDataValue);

   //Read from table
   EXPECT_EQ(reader_.getThickness(4),Utilities::Numerical::IbsNoDataValue);

   //Out of table range
   EXPECT_EQ(readerNotLoaded.getThickness(20),Utilities::Numerical::IbsNoDataValue);
}

TEST_F( CMBProjectReaderTest, testLayerNames )
{
  const QStringList layerNamesActual = reader_.layerNames();
  const QStringList layerNamesExpected =
  {
    "Miocene-Quaternary",
    "Lr_M_N_Sea_Gp",
    "Chalk",
    "Triassic",
    "Zechstein",
    "Rotliegend"
  };

  const int nLithoActual = layerNamesActual.size();
  const int nLithoExpected = layerNamesExpected.size();
  ASSERT_EQ(nLithoExpected, nLithoActual);

  for(int i = 0; i< nLithoActual; ++i)
  {
    EXPECT_EQ(layerNamesExpected[i], layerNamesActual[i]) << "Mismatch at entry [" << i << "]";
  }
}

TEST_F( CMBProjectReaderTest, testSurfaceNames )
{
  const QStringList surfaceNamesActual = reader_.surfaceNames();
  const QStringList surfaceNamesExpected =
  {
    "Water bottom",
    "20_NS_B_TDinput_SD",
    "30_CK_B_TDinput_SD",
    "45_TR_T_TDinput_SD",
    "50_ZE_T_TDinput_SD",
    "Nr_Top_Rotliegend",
    "Nr_Base_Rotliegend"
  };

  const int nSurfaceActual = surfaceNamesActual.size();
  const int nSurfaceExpected = surfaceNamesExpected.size();
  ASSERT_EQ(nSurfaceExpected, nSurfaceActual);

  for(int i = 0; i< nSurfaceActual; ++i)
  {
    EXPECT_EQ(surfaceNamesExpected[i], surfaceNamesActual[i]) << "Mismatch at entry [" << i << "]";
  }
}

TEST_F( CMBProjectReaderTest, testGetLayerUnderSurface )
{
   EXPECT_EQ(reader_.getLayerUnderSurface("Water bottom"), "Miocene-Quaternary");
   EXPECT_EQ(reader_.getLayerUnderSurface("45_TR_T_TDinput_SD"), "Triassic");
   EXPECT_EQ(reader_.getLayerUnderSurface("Nr_Base_Rotliegend"), ""); // Layer under basement surface does not exist
}

TEST_F( CMBProjectReaderTest, testGetSurfaceOnTopOfLayer )
{
   EXPECT_EQ(reader_.getSurfaceOnTopOfLayer("Miocene-Quaternary"), "Water bottom");
   EXPECT_EQ(reader_.getSurfaceOnTopOfLayer("Triassic"), "45_TR_T_TDinput_SD");
}

TEST_F( CMBProjectReaderTest, testGetLayerID )
{
  const QStringList layerNamesActual = reader_.layerNames();

  int layerIDExpected = 0;
  for (QString layerName : layerNamesActual)
  {
    int layerIDActual = reader_.getLayerID(layerName.toStdString());
    EXPECT_EQ(layerIDExpected, layerIDActual);

    layerIDExpected++;
  }
}

TEST_F( CMBProjectReaderTest, testGetLayerIDInvalidLayer )
{
  EXPECT_EQ(NoDataIDValue, reader_.getLayerID("Non-existent layer"));
}

TEST_F( CMBProjectReaderTest, testDomainRange )
{
  double xMinActual, xMaxActual, yMinActual, yMaxActual;
  reader_.domainRange(xMinActual, xMaxActual, yMinActual, yMaxActual);

  double xMinExpected = 179000;
  double yMinExpected = 603500;
  double xMaxExpected = 205500;
  double yMaxExpected = 618500;

  EXPECT_EQ(xMinExpected, xMinActual);
  EXPECT_EQ(yMinExpected, yMinActual);
  EXPECT_EQ(xMaxExpected, xMaxActual);
  EXPECT_EQ(yMaxExpected, yMaxActual);
}

TEST_F( CMBProjectReaderTest, testHeatProductionRate )
{
  const double heatProductionActual = reader_.heatProductionRate();
  const double heatProductionExpected = 123;
  EXPECT_EQ(heatProductionExpected, heatProductionActual);
}

TEST_F( CMBProjectReaderTest, testAgesFromMajorSnapshots )
{
  const QVector<double> majorSnapshotsExpected = {0, 23, 66, 100, 201, 252, 255, 299};
  const QVector<double>& majorSnapshotsActual = reader_.agesFromMajorSnapshots();

  int i = 0;
  for (const double& msExpected : majorSnapshotsExpected)
  {
    const double msActual = majorSnapshotsActual[i];
    EXPECT_NEAR(msExpected, msActual, epsilon);
    ++i;
  }
}

TEST_F( CMBProjectReaderTest, testLithologyTypesForLayer)
{
  QString sand = "LSB Sand Hybrid LK03042015";
  QString shale = "LSB Shale Hybrid LK03042015";
  QString chalk = "LSB Chalk Hybrid LK03042015";
  QString congo = "Congo_Salt_Hybrid_LK25112015";

  const QStringList layer0Names = reader_.lithologyTypesForLayer(0);
  ASSERT_EQ(3, layer0Names.size());
  EXPECT_EQ(sand, layer0Names[0]);
  EXPECT_EQ(shale, layer0Names[1]);
  EXPECT_EQ(chalk, layer0Names[2]);

  const QStringList layer2Names = reader_.lithologyTypesForLayer(2);
  ASSERT_EQ(3, layer2Names.size());
  EXPECT_EQ(chalk, layer2Names[0]);
  EXPECT_EQ(shale, layer2Names[1]);
  EXPECT_EQ(sand, layer2Names[2]);

  const QStringList layer4Names = reader_.lithologyTypesForLayer(4);
  ASSERT_EQ(3, layer4Names.size());
  EXPECT_EQ(congo, layer4Names[0]);
  EXPECT_EQ(shale, layer4Names[1]);
  EXPECT_EQ(sand, layer4Names[2]);
}

TEST_F( CMBProjectReaderTest, testReadMantleThickness)
{
  // When
  double mantleThickness = reader_.initialLithosphericMantleThickness();

  // Then
  EXPECT_DOUBLE_EQ(mantleThickness, 80000);
}

TEST_F( CMBProjectReaderTest, testLowestLayerWithTWT_WhenTWTMapsAreMissing)
{
  // Given
  //
  // Structure of the stratigraphy:
  // ----------------------------------
  // Surface 0   --    Water Bottom    --
  // Surface 1   -- 20_NS_B_TDinput_SD --
  // Surface 2   -- 30_CK_B_TDinput_SD --
  // Surface 3   -- 45_TR_T_TDinput_SD --
  // Surface 4   -- 50_ZE_T_TDinput_SD --
  // Surface 5   -- Nr_Top_Rotliegend  --
  // Surface 6   -- Nr_Base_Rotliegend --
  // ----------------------------------
  // Where the last two surfaces don't have TWT data (enforced by removing these entries
  // from the TwoWayTimeIoTbl. Therefore, the surfaceID of the lowest surface with TWT
  // data is 4

  // When
  int lowestSurface = reader_.lowestSurfaceWithTWTData();
  bool basementHasTWT = reader_.basementSurfaceHasTWT();

  // Then
  EXPECT_EQ(lowestSurface, 4);
  EXPECT_FALSE(basementHasTWT);
}

TEST_F( CMBProjectReaderTest, testNotLoadedReader)
{
  casaWizard::CMBProjectReader notLoadedReader;

  EXPECT_EQ(notLoadedReader.mapNames(), QStringList());
  EXPECT_EQ(notLoadedReader.layerNames(), QStringList());
  EXPECT_EQ(notLoadedReader.surfaceNames(), QStringList());
  EXPECT_EQ(notLoadedReader.lithologyTypesForLayer(0), QStringList());
  EXPECT_EQ(notLoadedReader.agesFromMajorSnapshots(), QVector<double>());
  EXPECT_EQ(notLoadedReader.lowestSurfaceWithTWTData(), DataAccess::Interface::DefaultUndefinedScalarIntValue);
  EXPECT_EQ(notLoadedReader.basementSurfaceHasTWT(), false);
  EXPECT_EQ(notLoadedReader.hasDepthDefinedInAllLayers(), false);
  EXPECT_EQ(notLoadedReader.getLayerID("Test"), NoDataIDValue);
  EXPECT_EQ(notLoadedReader.subSamplingFactor(), 1);
  EXPECT_DOUBLE_EQ(notLoadedReader.heatProductionRate(), 0.0);
}

TEST_F( CMBProjectReaderTest, testGetDepthGridName)
{
  QStringList mapNamesExpected = {"MAP-1511782103-4",
                                  "MAP-1511782107-4",
                                  "MAP-1511782111-4",
                                  "MAP-1511782115-4",
                                  "T2Z[50_ZE_T_TDinput_SD]",
                                  "MAP-1511782123-4",
                                  "MAP-1511782127-4"};

  for (int id = 0; id < reader_.surfaceNames().size(); id++)
  {
    EXPECT_EQ(mapNamesExpected[id], reader_.getDepthGridName(id));
  }
}

TEST_F( CMBProjectReaderTest, testMapNamesT2Z)
{
  QStringList mapNamesExpected = {"T2Z[50_ZE_T_TDinput_SD]"};

  EXPECT_EQ(mapNamesExpected, reader_.mapNamesT2Z());
}

TEST_F( CMBProjectReaderTest, testDepthDefinedInAllLayers)
{
  EXPECT_TRUE(reader_.hasDepthDefinedInAllLayers());
}

TEST_F(CMBProjectReaderTest, testReadSubSamplingFactor)
{
   EXPECT_EQ(5, reader_.subSamplingFactor());
}



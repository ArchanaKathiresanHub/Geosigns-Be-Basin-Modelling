//
// Copyright (C) 2020 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "model/input/cmbProjectReader.h"

#include <gtest/gtest.h>
#include <numeric>
#include <stdio.h>

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
    "MAP-1511782119-4",
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

TEST_F( CMBProjectReaderTest, testUpdateOutputProperties)
{
  // Given
  QStringList activeProperties = {"TwoWayTime", "VRe"};
  std::remove("Output.project3d");

  // When
  reader_.setRelevantOutputParameters(activeProperties, "Output.project3d");

  // Then
  reader_.load(QString("Output.project3d"));
  std::map<std::string, std::string> outputProperties = reader_.readOutputProperties();
  for (auto propertyPair : outputProperties)
  {
    if (propertyPair.first == "TwoWayTime")
    {
      EXPECT_EQ(propertyPair.second, "SedimentsOnly");
    }
    else if (propertyPair.first == "VrVec")
    {
      EXPECT_EQ(propertyPair.second, "SedimentsOnly");
    }
    else if (propertyPair.first == "Depth")
    {
      EXPECT_EQ(propertyPair.second, "SedimentsPlusBasement");
    }
    else
    {
      EXPECT_EQ(propertyPair.second, "None");
    }
  }
}

TEST_F( CMBProjectReaderTest, testReadOutputProperties)
{
  // When
  std::map<std::string, std::string> outputProperties = reader_.readOutputProperties();

  // Then
  EXPECT_EQ(outputProperties.at("Depth"), "SedimentsPlusBasement");
  EXPECT_EQ(outputProperties.at("Massflux"), "SedimentsPlusBasement");
  EXPECT_EQ(outputProperties.at("VrVec"), "SedimentsOnly");
  EXPECT_EQ(outputProperties.at("DryGasExpelledRate"), "None");
  EXPECT_EQ(outputProperties.at("SourceRockEndMember1"), "SourceRockOnly");
}


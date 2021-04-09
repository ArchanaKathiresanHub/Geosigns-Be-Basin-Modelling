//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include <gtest/gtest.h>

#include "cmbAPI.h"
#include "model/input/cmbProjectReader.h"
#include "model/output/cmbProjectWriter.h"

#include "OutputProperty.h"

#include <QDir>
#include <QFile>
#include <QStringList>

TEST( CMBProjectWriterTest, testUpdateOutputProperties)
{  
  const QString projectFile("CMBProjectWriterTest.project3d");
  const QString copyProjectFile("projectFileCopy.project3d");

  QFile::remove(copyProjectFile);
  QFile::copy(projectFile, copyProjectFile);

  casaWizard::CMBProjectWriter writer(copyProjectFile);

  // Given
  QStringList activeProperties = {"TwoWayTime", "VRe"};  

  // When
  writer.setRelevantOutputParameters(activeProperties);

  // Then
  mbapi::Model cmbModel;
  cmbModel.loadModelFromProjectFile(copyProjectFile.toStdString());
  std::unique_ptr<DataAccess::Interface::OutputPropertyList> outputPropertyList(cmbModel.projectHandle()->getTimeOutputProperties());
  for (const DataAccess::Interface::OutputProperty* property : *outputPropertyList)
  {
    const std::string name = property->getName();
    const DataAccess::Interface::PropertyOutputOption option = property->getOption();
    if (name == "TwoWayTime")
    {
      EXPECT_EQ(option, DataAccess::Interface::PropertyOutputOption::SEDIMENTS_ONLY_OUTPUT);
    }
    else if (name == "VrVec")
    {
      EXPECT_EQ(option, DataAccess::Interface::PropertyOutputOption::SEDIMENTS_ONLY_OUTPUT);
    }
    else if (name == "Depth")
    {
      EXPECT_EQ(option, DataAccess::Interface::PropertyOutputOption::SEDIMENTS_AND_BASEMENT_OUTPUT);
    }
    else
    {
      EXPECT_EQ(option, DataAccess::Interface::PropertyOutputOption::NO_OUTPUT);
    }
  }
}


TEST( CMBProjectWriterTest, testSetScale)
{
  const QString projectFile("CMBProjectWriterTest.project3d");
  const QString copyProjectFile("projectFileCopy2.project3d");

  QFile::remove(copyProjectFile);
  QFile::copy(projectFile, copyProjectFile);

  // Given
  casaWizard::CMBProjectWriter writer(copyProjectFile);

  // Then
  EXPECT_NO_THROW(writer.setScaling(4, 4));

  mbapi::Model cmbModel;
  cmbModel.loadModelFromProjectFile(copyProjectFile.toStdString());
  EXPECT_EQ(cmbModel.tableValueAsInteger("ProjectIoTbl", 0, "ScaleX" ), 4);
  EXPECT_EQ(cmbModel.tableValueAsInteger("ProjectIoTbl", 0, "ScaleY" ), 4);
}

TEST( CMBProjectWriterTest, testGenerateOutputProject )
{   
  QDir dir("export/copy");  
  if (dir.exists()) dir.removeRecursively();
  dir.mkpath("export/copy");

  for (const QString& file : {"Project.project3d", "Inputs.HDF", "CalibratedInputs.HDF"})
  {
    QFile::copy(QString("export/" + file), QString("export/copy/" + file));
  }

  const QString projectFile = "export/copy/Project.project3d";

  // Given
  casaWizard::CMBProjectWriter writer(projectFile);

  writer.generateOutputProject(QString("TimeStamp"));

  mbapi::Model cmbModel;
  cmbModel.loadModelFromProjectFile(projectFile.toStdString());

  mbapi::MapsManager& mapsManager = cmbModel.mapsManager();
  int mapId = 35;
  for ( const std::string& mapName : {
        "0_percent_1_TimeStamp",
        "0_percent_2_TimeStamp",
        "2_percent_1_TimeStamp",
        "2_percent_2_TimeStamp",
        "3_percent_1_TimeStamp",
        "3_percent_2_TimeStamp",
        "4_percent_1_TimeStamp",
        "4_percent_2_TimeStamp" })
  {
      mapId++;
      EXPECT_EQ(mapsManager.findID(mapName), mapId);
  }

  for ( const std::string& mapName : {
        "T2Z[Base_Breda_Clay]_TimeStamp",
        "T2Z[Top_Ieper]_TimeStamp",
        "T2Z[Top_Chalk_nr_20_NS_B]_TimeStamp",
        "T2Z[Top_Lower_Ommeland_Chalk]_TimeStamp",
        "T2Z[Top_Texel_nr_30_CK_B]_TimeStamp",
        "T2Z[Base_Cretaceous_Hiatus_Unconformity]_TimeStamp",
        "T2Z[Eroded_top_Buntsandstein]_TimeStamp",
        "T2Z[Base_Bunter_on_hiatus_nr_50_ZE_T]_TimeStamp",
        "T2Z[Top_Z2_nr_59_ZEZ2A_T]_TimeStamp",
        "T2Z[Nr_Top_Rotliegend]_TimeStamp",
        "T2ZIsoSurf[Nr_Base_Rotliegend]_TimeStamp"} )
  {
      mapId++;
      EXPECT_EQ(mapsManager.findID(mapName), mapId);
      EXPECT_EQ(cmbModel.tableValueAsString("GridMapIoTbl", mapId, "MapFileName"), "Inputs.HDF");
  }

  EXPECT_EQ(cmbModel.tableSize("OutputTablesFileIoTbl"), 0);
  EXPECT_EQ(cmbModel.tableSize("OutputTablesIoTbl"), 0);
  EXPECT_EQ(cmbModel.tableSize("TimeIoTbl"), 0);
  EXPECT_EQ(cmbModel.tableSize("3DTimeIoTbl"), 0);
  EXPECT_EQ(cmbModel.tableSize("1DTimeIoTbl"), 0);
  EXPECT_EQ(cmbModel.tableSize("SimulationDetailsIoTbl"), 0);
}




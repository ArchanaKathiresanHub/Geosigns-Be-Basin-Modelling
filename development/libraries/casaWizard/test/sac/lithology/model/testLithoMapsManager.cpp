//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "model/LithologyMapsManager.h"

#include "model/scenarioReader.h"
#include "model/scenarioWriter.h"

#include "stubProjectReader.h"
#include "stubmapreader.h"

#include "expectFileEq.h"

#include <QDir>

#include <gtest/gtest.h>

using namespace casaWizard::sac;
using namespace casaWizard;

TEST(LithoMapsManagerTest, testReadWrite)
{
    LithologyMapsManager writeMapsManager;
    writeMapsManager.setSmartGridding(false);

    ScenarioWriter writer("mapsManager.dat");
    writeMapsManager.writeToFile(writer);
    writer.close();

    LithologyMapsManager readMapsManager;
    ScenarioReader reader("mapsManager.dat");
    readMapsManager.readFromFile(reader);

    EXPECT_EQ(writeMapsManager.smartGridding(), readMapsManager.smartGridding());
}

TEST(LithoMapsManagerTest, testExportOptimizedLithofractionMapsToZycor)
{
    std::remove("Layer1_Type1.zyc");
    std::remove("Layer1_Type2.zyc");
    std::remove("Layer2_Type1.zyc");
    std::remove("Layer2_Type2.zyc");

    LithologyMapsManager mapsManager;
    StubProjectReader projectReader;
    StubMapReader mapReader;
    mapsManager.exportOptimizedMapsToZycor(projectReader, mapReader, ".");

    //No maps should be exported for the undefined lithologies:
    const QDir folder(QDir::currentPath());
    const QFileInfoList filesList = folder.entryInfoList(QDir::Files);
    EXPECT_FALSE(filesList.contains(QFileInfo("Layer1_.zyc")));
    EXPECT_FALSE(filesList.contains(QFileInfo("Layer2_.zyc")));

    expectFileEq("Layer_Type1Expected.zyc", "Layer1_Type1.zyc");
    expectFileEq("Layer_Type2Expected.zyc", "Layer1_Type2.zyc");
    expectFileEq("Layer_Type1Expected.zyc", "Layer2_Type1.zyc");
    expectFileEq("Layer_Type2Expected.zyc", "Layer2_Type2.zyc");
}

TEST(MapsManager, testTransformToActiveAndIncludedWithoutExcludedIndices)
{
    LithologyMapsManager mapsManager;
    QVector<int> selectedWellIndices = {0, 3, 4, 5};
    QVector<int> excludedIndices;

    QVector<int> activeAndIncludedIndices = mapsManager.transformToActiveAndIncluded(selectedWellIndices, excludedIndices);

    ASSERT_EQ(selectedWellIndices.size(), activeAndIncludedIndices.size());

    for (int i = 0; i < selectedWellIndices.size(); i++)
    {
        EXPECT_EQ(selectedWellIndices[i], activeAndIncludedIndices[i]);
    }
}

TEST(MapsManager, testTransformToActiveAndIncluded)
{
    LithologyMapsManager mapsManager;
    QVector<int> selectedWellIndices = {0, 3, 5, 9, 17};
    QVector<int> excludedIndices = {3, 9};

    QVector<int> activeAndIncludedIndices = mapsManager.transformToActiveAndIncluded(selectedWellIndices, excludedIndices);
    QVector<int> activeAndIncludedIndicesExpected = {0, 4, 15};

    ASSERT_EQ(activeAndIncludedIndicesExpected.size(), activeAndIncludedIndices.size());

    for (int i = 0; i < activeAndIncludedIndicesExpected.size(); i++)
    {
        EXPECT_EQ(activeAndIncludedIndicesExpected[i], activeAndIncludedIndices[i]);
    }
}


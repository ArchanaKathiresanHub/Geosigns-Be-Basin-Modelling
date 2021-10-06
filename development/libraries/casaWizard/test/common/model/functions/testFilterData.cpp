//
// Copyright (C) 2021 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "model/functions/filterData.h"

#include <gtest/gtest.h>

TEST(filterDataTest, testSmoothenData)
{
    // Given
    const QVector<double> depths = {100,200,400,450,600,660,700,710,1000,1200,2000};
    const QVector<double> values = {10,20,40,145,60,66,170,71,100,120,62};

    // When
    const QVector<double> smoothData = casaWizard::functions::smoothenData(depths, values, 100.0);

    // Then
    const QVector<double> expectedData = {14.132582678363796,
                                          21.20300843990821,
                                          82.79907693735224,
                                          89.224675941112906,
                                          89.759186899825565,
                                          93.277502780711743,
                                          96.326227951662716,
                                          97.082217428093372,
                                          102.51822191252319,
                                          117.61594155955703,
                                          62.000000000000000};

    ASSERT_EQ(smoothData.size(), expectedData.size());
    for (int i=0; i<smoothData.size(); ++i)
    {
        EXPECT_DOUBLE_EQ(smoothData[i], expectedData[i]);
    }
}

TEST(filterDataTest, testSubsampleData)
{
    // Given
    const QVector<double> depths = {100,200,400,450,600,660,700,710,1000,1200,2000};

    // When
    const QVector<int> subsampledDataIndices = casaWizard::functions::subsampleData(depths, 100.0);

    // Then
    const QVector<double> expectedData = {0,2,4,7,8,9,10};

    ASSERT_EQ(subsampledDataIndices.size(), expectedData.size());
    for (int i=0; i<subsampledDataIndices.size(); ++i)
    {
        EXPECT_EQ(subsampledDataIndices[i], expectedData[i]);
    }
}

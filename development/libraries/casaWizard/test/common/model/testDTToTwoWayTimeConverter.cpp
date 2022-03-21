//
// Copyright (C) 2020 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "model/DTToTwoWayTimeConverter.h"

#include <gtest/gtest.h>

using namespace casaWizard;

TEST(DTToTwoWayTimeConverterTest, TestConvert)
{
   const std::vector<double> DT = {3, 5, 3, 1, 5};
   const std::vector<double> depth = {0, 100, 300, 350, 600};

   const std::vector<double> TWT = DTToTwoWayTimeConverter::convertToTWT(DT, depth, {}, {});

   const std::vector<double> expectedTWT = {0,
                                            2 * (100 * 4) * 1e-3,
                                            2 * (100 * 4 + 200 * 4) * 1e-3,
                                            2 * (100 * 4 + 200 * 4 + 50 * 2) * 1e-3,
                                            2 * (100 * 4 + 200 * 4 + 50 * 2 + 250 * 3) * 1e-3};

   EXPECT_EQ(TWT.size(), expectedTWT.size());

   for (int i = 0; i < TWT.size(); i++)
   {
      EXPECT_DOUBLE_EQ(TWT[i], expectedTWT[i]);
   }
}

TEST(DTToTwoWayTimeConverterTest, TestConvertWithModelDataAndCompleteWellData)
{
   const std::vector<double> DT = {3, 5, 3, 1, 5};
   const std::vector<double> depth = {0, 100, 300, 350, 600};

   const std::vector<double> TWTModel = {13, 25, 13, 51, 15};
   const std::vector<double> depthModel = {0, 10, 30, 35, 60};

   const std::vector<double> TWT = DTToTwoWayTimeConverter::convertToTWT(DT, depth, TWTModel, depthModel);

   const std::vector<double> expectedTWT = {13,
                                            13 + 2 * (100 * 4) * 1e-3,
                                            13 + 2 * (100 * 4 + 200 * 4) * 1e-3,
                                            13 + 2 * (100 * 4 + 200 * 4 + 50 * 2) * 1e-3,
                                            13 + 2 * (100 * 4 + 200 * 4 + 50 * 2 + 250 * 3) * 1e-3};

   EXPECT_EQ(TWT.size(), expectedTWT.size());

   for (int i = 0; i < TWT.size(); i++)
   {
      EXPECT_DOUBLE_EQ(TWT[i], expectedTWT[i]);
   }
}

TEST(DTToTwoWayTimeConverterTest, TestConvertWithModelDataAndIncompleteWellData)
{
   // Given
   const std::vector<double> DT = {5, 3, 1, 5};
   const std::vector<double> depth = {100, 300, 350, 600};

   const std::vector<double> TWTModel = {3, 5, 7, 9, 11};
   const std::vector<double> depthModel = {0, 50, 90, 110, 600};

   // When
   const std::vector<double> TWT = DTToTwoWayTimeConverter::convertToTWT(DT, depth, TWTModel, depthModel);

   // Then
   const std::vector<double> expectedTWT = {8,
                                            8 + 2 * (200 * 4) * 1e-3,
                                            8 + 2 * (200 * 4 + 50 * 2) * 1e-3,
                                            8 + 2 * (200 * 4 + 50 * 2 + 250 * 3) * 1e-3};

   EXPECT_EQ(TWT.size(), expectedTWT.size());
   for (int i = 0; i < TWT.size(); i++)
   {
      EXPECT_DOUBLE_EQ(TWT[i], expectedTWT[i]);
   }
}

TEST(DTToTwoWayTimeConverterTest, TestConvertWithWellDataHigherThanModelData)
{
   // Given
   const std::vector<double> DT = {5, 3, 1, 5};
   const std::vector<double> depth = {0, 30, 35, 40};

   const std::vector<double> TWTModel = {3, 5, 7, 9, 11};
   const std::vector<double> depthModel = {30, 50, 90, 110, 600};

   // When
   const std::vector<double> TWT = DTToTwoWayTimeConverter::convertToTWT(DT, depth, TWTModel, depthModel);

   // Then
   const std::vector<double> expectedTWT = {3 - 2 * (4 * 30) * 1e-3,
                                            3,
                                            3 + 2 * (5 * 2) * 1e-3,
                                            3 + 2 * (5 * 2 + 5 * 3) * 1e-3};

   EXPECT_EQ(TWT.size(), expectedTWT.size());
   for (int i = 0; i < TWT.size(); i++)
   {
      EXPECT_DOUBLE_EQ(TWT[i], expectedTWT[i]);
   }
}

//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "../src/PrmLithoFraction.h"
#include "ConstantsNumerical.h"


#include <gtest/gtest.h>

TEST( PrmLithoFractionTest, testPercentageExtractionUndefinedIdx1 )
{
   const std::vector<double> lithoPercentagesIn{50.0,Utilities::Numerical::IbsNoDataValue};
   const std::vector<int> lithoFractionInds{0,1};

   const std::vector<double> lithofractions = casa::PrmLithoFraction::createLithoFractions( lithoPercentagesIn, lithoFractionInds );
   const std::vector<double> percentagesOut = casa::PrmLithoFraction::createLithoPercentages(lithofractions,lithoFractionInds);

   EXPECT_DOUBLE_EQ(percentagesOut[0],50);
   EXPECT_DOUBLE_EQ(percentagesOut[1],50);
}

TEST( PrmLithoFractionTest, testPercentageExtractionUndefinedIdx2)
{
   const std::vector<double> lithoPercentagesIn{50.0,Utilities::Numerical::IbsNoDataValue};
   const std::vector<int> lithoFractionInds{0,2};

   const std::vector<double> lithofractions = casa::PrmLithoFraction::createLithoFractions( lithoPercentagesIn, lithoFractionInds );
   const std::vector<double> percentagesOut = casa::PrmLithoFraction::createLithoPercentages(lithofractions,lithoFractionInds);

   EXPECT_DOUBLE_EQ(percentagesOut[0],50);
   EXPECT_DOUBLE_EQ(percentagesOut[1],50);
}

TEST( PrmLithoFractionTest, percTestIdx1)
{
   const std::vector<double> lithoPercentagesIn{50.0,33,17};
   const std::vector<int> lithoFractionInds{0,1};

   const std::vector<double> lithofractions = casa::PrmLithoFraction::createLithoFractions( lithoPercentagesIn, lithoFractionInds );
   const std::vector<double> percentagesOut = casa::PrmLithoFraction::createLithoPercentages(lithofractions,lithoFractionInds);

   EXPECT_DOUBLE_EQ(percentagesOut[0],50);
   EXPECT_DOUBLE_EQ(percentagesOut[1],33);
   EXPECT_DOUBLE_EQ(percentagesOut[2],17);
}

TEST( PrmLithoFractionTest, percTestIdx2)
{
   const std::vector<double> lithoPercentagesIn{50.0,33,17};
   const std::vector<int> lithoFractionInds{0,2};

   const std::vector<double> lithofractions = casa::PrmLithoFraction::createLithoFractions( lithoPercentagesIn, lithoFractionInds );
   const std::vector<double> percentagesOut = casa::PrmLithoFraction::createLithoPercentages(lithofractions,lithoFractionInds);

   EXPECT_DOUBLE_EQ(percentagesOut[0],50);
   EXPECT_DOUBLE_EQ(percentagesOut[1],33);
   EXPECT_DOUBLE_EQ(percentagesOut[2],17);
}

TEST( PrmLithoFractionTest, percTestPerc100)
{
   const std::vector<double> lithoPercentagesIn{100.0,0,0};
   const std::vector<int> lithoFractionInds{0,2};

   const std::vector<double> lithofractions = casa::PrmLithoFraction::createLithoFractions( lithoPercentagesIn, lithoFractionInds );
   const std::vector<double> percentagesOut = casa::PrmLithoFraction::createLithoPercentages(lithofractions,lithoFractionInds);

   EXPECT_DOUBLE_EQ(percentagesOut[0],100);
   EXPECT_DOUBLE_EQ(percentagesOut[1],0);
   EXPECT_DOUBLE_EQ(percentagesOut[2],0);
}

TEST( PrmLithoFractionTest, percTestPerc100_1)
{
   const std::vector<double> lithoPercentagesIn{0.0,0,100};
   const std::vector<int> lithoFractionInds{0,1};

   const std::vector<double> lithofractions = casa::PrmLithoFraction::createLithoFractions( lithoPercentagesIn, lithoFractionInds );
   const std::vector<double> percentagesOut = casa::PrmLithoFraction::createLithoPercentages(lithofractions,lithoFractionInds);

   EXPECT_DOUBLE_EQ(percentagesOut[0],0);
   EXPECT_DOUBLE_EQ(percentagesOut[1],0);
   EXPECT_DOUBLE_EQ(percentagesOut[2],100);
}

TEST( PrmLithoFractionTest, percTestPerc100_2)
{
   const std::vector<double> lithoPercentagesIn{0.0,0,100};
   const std::vector<int> lithoFractionInds{0,2};

   const std::vector<double> lithofractions = casa::PrmLithoFraction::createLithoFractions( lithoPercentagesIn, lithoFractionInds );
   const std::vector<double> percentagesOut = casa::PrmLithoFraction::createLithoPercentages(lithofractions,lithoFractionInds);

   EXPECT_DOUBLE_EQ(percentagesOut[0],0);
   EXPECT_DOUBLE_EQ(percentagesOut[1],0);
   EXPECT_DOUBLE_EQ(percentagesOut[2],100);
}

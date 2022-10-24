//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include <gtest/gtest.h>
#include "../src/MapInterpolatorInverseDistanceWeighting.h"
#include "ErrorHandler.h"

TEST(inverseDistanceWeightingTest, testIdw1D)
{
   using namespace casa;

   //Given:
   DomainData domainData;
   domainData.numI = 10;
   domainData.numJ = 1;
   domainData.xmin = -10;
   domainData.ymin = 0;
   domainData.deltaX = 2;
   domainData.deltaY = 2;

   double power = 3;
   MapInterpolatorInverseDistanceWeighting interpolator(power);

   std::vector<double> xin;
   std::vector<double> vin;
   for (int i = 0; i < 3; i++)
   {
      xin.push_back(-9+double(i)*9);
      vin.push_back(double(i));
   }

   std::vector<double> yin(xin.size(),1);

   //output:
   std::vector<double> xInt;
   std::vector<double> yInt;
   std::vector<double> vInt;

   //when:
   interpolator.generateInterpolatedMap(domainData,xin,yin,vin,xInt,yInt,vInt);

   //Then:
   EXPECT_EQ(vInt[0],vin[0]);
   EXPECT_EQ(vInt.back(),vin.back());
   EXPECT_DOUBLE_EQ(xInt[0],domainData.xmin+0.5*domainData.deltaX);
   EXPECT_DOUBLE_EQ(xInt.back(),domainData.xmin+(domainData.numI-0.5)*domainData.deltaX);
   EXPECT_EQ(domainData.numI*domainData.numJ,vInt.size());
}

TEST(inverseDistanceWeightingTest, testIdw2D)
{
   using namespace casa;

   //Given:
   DomainData domainData;
   domainData.numI = 10;
   domainData.numJ = 5;

   std::vector<double> xInt;
   std::vector<double> yInt;
   std::vector<double> vInt;

   MapInterpolatorInverseDistanceWeighting interpolator(1);

   //when:
   interpolator.generateInterpolatedMap(domainData,{},{},{},xInt,yInt,vInt);

   //Then:
   EXPECT_EQ(domainData.numI*domainData.numJ,vInt.size());
   EXPECT_EQ(domainData.numI*domainData.numJ,xInt.size());
   EXPECT_EQ(domainData.numI*domainData.numJ,yInt.size());
}

TEST(inverseDistanceWeightingTest, testException)
{
   using namespace casa;

   //Given input data of unequal size:
   std::vector<double> xin{0};
   std::vector<double> yin{0};
   std::vector<double> vin{0,1};

   std::vector<double> xInt;
   std::vector<double> yInt;
   std::vector<double> vInt;

   MapInterpolatorInverseDistanceWeighting interpolator(1);

   //when calling interpolation function, then expect exception:
   EXPECT_THROW(interpolator.generateInterpolatedMap(DomainData(),xin,yin,vin,xInt,yInt,vInt),ErrorHandler::Exception);
}

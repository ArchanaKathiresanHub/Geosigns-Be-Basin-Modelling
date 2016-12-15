//
// Copyright (C) 2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include <iostream>
#include <cmath>
#include <cassert>
#include <cstdlib>

#include "../src/LambdaMixer.h"

#include <gtest/gtest.h>


using namespace GeoPhysics;

const double relativeError = 1.0e-10; 

TEST( LambdaMixer, OneLithology )
{
   // First scenario
   std::vector<double> percentages1, lambdaPc1, lambdaKr1;
   percentages1.push_back(100.0);
   lambdaPc1.push_back(0.5);
   lambdaKr1.push_back(0.5);

   LambdaMixer lambdaMixer1 (percentages1, lambdaPc1, lambdaKr1);
   double lambdaPcMixed, lambdaKrMixed;
   lambdaMixer1.mixLambdas(lambdaPcMixed, lambdaKrMixed);

   EXPECT_NEAR( lambdaPcMixed, lambdaPc1[0], lambdaPc1[0] * relativeError);
   EXPECT_NEAR( lambdaKrMixed, lambdaKr1[0], lambdaKr1[0] * relativeError);


   // Second scenario
   std::vector<double> percentages2, lambdaPc2, lambdaKr2;
   percentages2.push_back(100.0);
   lambdaPc2.push_back(1.0);
   lambdaKr2.push_back(1.0);

   LambdaMixer lambdaMixer2 (percentages2, lambdaPc2, lambdaKr2);
   lambdaMixer2.mixLambdas(lambdaPcMixed, lambdaKrMixed);

   EXPECT_NEAR( lambdaPcMixed, lambdaPc2[0], lambdaPc2[0] * relativeError);
   EXPECT_NEAR( lambdaKrMixed, lambdaKr2[0], lambdaKr2[0] * relativeError);


   // Third scenario
   std::vector<double> percentages3, lambdaPc3, lambdaKr3;
   percentages3.push_back(100.0);
   lambdaPc3.push_back(2.0);
   lambdaKr3.push_back(2.0);

   LambdaMixer lambdaMixer3 (percentages3, lambdaPc3, lambdaKr3);
   lambdaMixer3.mixLambdas(lambdaPcMixed, lambdaKrMixed);

   EXPECT_NEAR( lambdaPcMixed, lambdaPc3[0], lambdaPc3[0] * relativeError);
   EXPECT_NEAR( lambdaKrMixed, lambdaKr3[0], lambdaKr3[0] * relativeError);

   
   // Fourth scenario
   std::vector<double> percentages4, lambdaPc4, lambdaKr4;
   percentages4.push_back(100.0);
   lambdaPc4.push_back(5.0);
   lambdaKr4.push_back(5.0);

   LambdaMixer lambdaMixer4 (percentages4, lambdaPc4, lambdaKr4);
   lambdaMixer4.mixLambdas(lambdaPcMixed, lambdaKrMixed);

   EXPECT_NEAR( lambdaPcMixed, lambdaPc4[0], lambdaPc4[0] * relativeError);
   EXPECT_NEAR( lambdaKrMixed, lambdaKr4[0], lambdaKr4[0] * relativeError);


   // Fifth scenario
   std::vector<double> percentages5, lambdaPc5, lambdaKr5;
   percentages5.push_back(100.0);
   lambdaPc5.push_back(10.0);
   lambdaKr5.push_back(10.0);

   LambdaMixer lambdaMixer5 (percentages5, lambdaPc5, lambdaKr5);
   lambdaMixer5.mixLambdas(lambdaPcMixed, lambdaKrMixed);

   EXPECT_NEAR( lambdaPcMixed, lambdaPc5[0], lambdaPc5[0] * relativeError);
   EXPECT_NEAR( lambdaKrMixed, lambdaKr5[0], lambdaKr5[0] * relativeError);


   // Sixth scenario
   std::vector<double> percentages6, lambdaPc6, lambdaKr6;
   percentages6.push_back(100.0);
   lambdaPc6.push_back(100.0);
   lambdaKr6.push_back(100.0);

   LambdaMixer lambdaMixer6 (percentages6, lambdaPc6, lambdaKr6);
   lambdaMixer6.mixLambdas(lambdaPcMixed, lambdaKrMixed);

   EXPECT_NEAR( lambdaPcMixed, lambdaPc6[0], lambdaPc6[0] * relativeError);
   EXPECT_NEAR( lambdaKrMixed, lambdaKr6[0], lambdaKr6[0] * relativeError);


   // Seventh scenario
   std::vector<double> percentages7, lambdaPc7, lambdaKr7;
   percentages7.push_back(100.0);
   lambdaPc7.push_back(0.0);
   lambdaKr7.push_back(0.0);

   LambdaMixer lambdaMixer7 (percentages7, lambdaPc7, lambdaKr7);
   lambdaMixer7.mixLambdas(lambdaPcMixed, lambdaKrMixed);

   EXPECT_NEAR( lambdaPcMixed, 0.0, relativeError);
   EXPECT_NEAR( lambdaKrMixed, 0.0, relativeError);
}


TEST( LambdaMixer, TwoLithologies )
{
   // First scenario
   std::vector<double> percentages1, lambdaPc1, lambdaKr1;
   percentages1.push_back(90.0);
   percentages1.push_back(10.0);
   lambdaPc1.push_back(0.5);
   lambdaPc1.push_back(3000.0);
   lambdaKr1.push_back(0.5);
   lambdaKr1.push_back(10000.0);

   LambdaMixer lambdaMixer1 (percentages1, lambdaPc1, lambdaKr1);
   double lambdaPcMixed, lambdaKrMixed;
   lambdaMixer1.mixLambdas(lambdaPcMixed, lambdaKrMixed);

   EXPECT_NEAR( lambdaPcMixed, lambdaPc1[0], lambdaPc1[0] * relativeError);
   EXPECT_NEAR( lambdaKrMixed, lambdaKr1[0], lambdaKr1[0] * relativeError);

   
   // Second scenario
   std::vector<double> percentages2, lambdaPc2, lambdaKr2;
   percentages2.push_back(20.0);
   percentages2.push_back(80.0);
   lambdaPc2.push_back(0.5);
   lambdaPc2.push_back(3000.0);
   lambdaKr2.push_back(0.5);
   lambdaKr2.push_back(10000.0);

   LambdaMixer lambdaMixer2 (percentages2, lambdaPc2, lambdaKr2);
   lambdaMixer2.mixLambdas(lambdaPcMixed, lambdaKrMixed);

   EXPECT_NEAR( lambdaPcMixed, lambdaPc2[1], lambdaPc2[1] * relativeError);
   EXPECT_NEAR( lambdaKrMixed, lambdaKr2[1], lambdaKr2[1] * relativeError);


   // Third scenario
   std::vector<double> percentages3, lambdaPc3, lambdaKr3;
   percentages3.push_back(30.0);
   percentages3.push_back(70.0);
   lambdaPc3.push_back(1.0);
   lambdaPc3.push_back(2.0);
   lambdaKr3.push_back(0.5);
   lambdaKr3.push_back(10.0);

   LambdaMixer lambdaMixer3 (percentages3, lambdaPc3, lambdaKr3);
   lambdaMixer3.mixLambdas(lambdaPcMixed, lambdaKrMixed);

   EXPECT_NEAR( lambdaPcMixed, lambdaPc3[1], lambdaPc3[1] * relativeError);
   EXPECT_NEAR( lambdaKrMixed, lambdaKr3[1], lambdaKr3[1] * relativeError);


   // Fourth scenario
   std::vector<double> percentages4, lambdaPc4, lambdaKr4;
   percentages4.push_back(40.0);
   percentages4.push_back(60.0);
   lambdaPc4.push_back(1.0);
   lambdaPc4.push_back(2.0);
   lambdaKr4.push_back(0.5);
   lambdaKr4.push_back(10.0);

   LambdaMixer lambdaMixer4 (percentages4, lambdaPc4, lambdaKr4);
   lambdaMixer4.mixLambdas(lambdaPcMixed, lambdaKrMixed);

   EXPECT_NEAR( lambdaPcMixed, lambdaPc4[1], lambdaPc4[1] * relativeError);
   EXPECT_NEAR( lambdaKrMixed, lambdaKr4[1], lambdaKr4[1] * relativeError);


   // Fifth scenario
   std::vector<double> percentages5, lambdaPc5, lambdaKr5;
   percentages5.push_back(55.0);
   percentages5.push_back(45.0);
   lambdaPc5.push_back(4.675);
   lambdaPc5.push_back(4.675);
   lambdaKr5.push_back(0.002);
   lambdaKr5.push_back(0.002);

   LambdaMixer lambdaMixer5 (percentages5, lambdaPc5, lambdaKr5);
   lambdaMixer5.mixLambdas(lambdaPcMixed, lambdaKrMixed);

   EXPECT_NEAR( lambdaPcMixed, lambdaPc5[1], lambdaPc5[1] * relativeError);
   EXPECT_NEAR( lambdaKrMixed, lambdaKr5[1], lambdaKr5[1] * relativeError);


   // Sixth scenario
   std::vector<double> percentages6, lambdaPc6, lambdaKr6;
   percentages6.push_back(43.0);
   percentages6.push_back(57.0);
   lambdaPc6.push_back(0.0111);
   lambdaPc6.push_back(0.0111);
   lambdaKr6.push_back(120.0);
   lambdaKr6.push_back(120.0);

   LambdaMixer lambdaMixer6 (percentages6, lambdaPc6, lambdaKr6);
   lambdaMixer6.mixLambdas(lambdaPcMixed, lambdaKrMixed);

   EXPECT_NEAR( lambdaPcMixed, lambdaPc6[1], lambdaPc6[1] * relativeError);
   EXPECT_NEAR( lambdaKrMixed, lambdaKr6[1], lambdaKr6[1] * relativeError);


   // Seventh scenario
   std::vector<double> percentages7, lambdaPc7, lambdaKr7;
   percentages7.push_back(48.0);
   percentages7.push_back(52.0);
   lambdaPc7.push_back(2.5);
   lambdaPc7.push_back(0.5);
   lambdaKr7.push_back(1.0);
   lambdaKr7.push_back(5.0);

   LambdaMixer lambdaMixer7 (percentages7, lambdaPc7, lambdaKr7);
   lambdaMixer7.mixLambdas(lambdaPcMixed, lambdaKrMixed);

   EXPECT_NEAR( lambdaPcMixed, 0.735294, lambdaPcMixed * 100000.0 * relativeError);
   EXPECT_NEAR( lambdaKrMixed, 1.923077, lambdaKrMixed * 100000.0 * relativeError);


   // Eighth scenario
   std::vector<double> percentages8, lambdaPc8, lambdaKr8;
   percentages8.push_back(44.0);
   percentages8.push_back(56.0);
   lambdaPc8.push_back(1.5);
   lambdaPc8.push_back(103.0);
   lambdaKr8.push_back(47.5);
   lambdaKr8.push_back(0.1);

   LambdaMixer lambdaMixer8 (percentages8, lambdaPc8, lambdaKr8);
   lambdaMixer8.mixLambdas(lambdaPcMixed, lambdaKrMixed);

   EXPECT_NEAR( lambdaPcMixed, 7.087156, lambdaPcMixed * 100000.0 * relativeError);
   EXPECT_NEAR( lambdaKrMixed, 0.124934, lambdaKrMixed * 100000.0 * relativeError);


   // Ninth scenario
   std::vector<double> percentages9, lambdaPc9, lambdaKr9;
   percentages9.push_back(44.0);
   percentages9.push_back(56.0);
   lambdaPc9.push_back(0.0);
   lambdaPc9.push_back(103.0);
   lambdaKr9.push_back(47.5);
   lambdaKr9.push_back(0.0);

   LambdaMixer lambdaMixer9 (percentages9, lambdaPc9, lambdaKr9);
   lambdaMixer9.mixLambdas(lambdaPcMixed, lambdaKrMixed);

   EXPECT_NEAR( lambdaPcMixed, 0.0, lambdaPcMixed * relativeError);
   EXPECT_NEAR( lambdaKrMixed, 0.0, lambdaKrMixed * relativeError);
}


TEST( LambdaMixer, ThreeLithologies )
{
   // First scenario
   std::vector<double> percentages1, lambdaPc1, lambdaKr1;
   percentages1.push_back(50.0);
   percentages1.push_back(30.0);
   percentages1.push_back(20.0);
   lambdaPc1.push_back(135678.123);
   lambdaPc1.push_back(1.0);
   lambdaPc1.push_back(3.0);
   lambdaKr1.push_back(0.5);
   lambdaKr1.push_back(100.0);
   lambdaKr1.push_back(10201980.0);

   LambdaMixer lambdaMixer1 (percentages1, lambdaPc1, lambdaKr1);
   double lambdaPcMixed, lambdaKrMixed;
   lambdaMixer1.mixLambdas(lambdaPcMixed, lambdaKrMixed);

   EXPECT_NEAR( lambdaPcMixed, lambdaPc1[0], lambdaPc1[0] * relativeError);
   EXPECT_NEAR( lambdaKrMixed, lambdaKr1[0], lambdaKr1[0] * relativeError);

   // Second scenario
   std::vector<double> percentages2, lambdaPc2, lambdaKr2;
   percentages2.push_back(33.333);
   percentages2.push_back(33.333);
   percentages2.push_back(33.333);
   lambdaPc2.push_back(0.123);
   lambdaPc2.push_back(0.123);
   lambdaPc2.push_back(0.123);
   lambdaKr2.push_back(30000.0);
   lambdaKr2.push_back(30000.0);
   lambdaKr2.push_back(30000.0);

   LambdaMixer lambdaMixer2 (percentages2, lambdaPc2, lambdaKr2);
   lambdaMixer2.mixLambdas(lambdaPcMixed, lambdaKrMixed);

   EXPECT_NEAR( lambdaPcMixed, lambdaPc2[0], lambdaPc2[0] * relativeError);
   EXPECT_NEAR( lambdaKrMixed, lambdaKr2[0], lambdaKr2[0] * relativeError);


   // Third scenario
   std::vector<double> percentages3, lambdaPc3, lambdaKr3;
   percentages3.push_back(25.0);
   percentages3.push_back(25.0);
   percentages3.push_back(50.0);
   lambdaPc3.push_back(0.1);
   lambdaPc3.push_back(15.0);
   lambdaPc3.push_back(2.222);
   lambdaKr3.push_back(100.0);
   lambdaKr3.push_back(3.0);
   lambdaKr3.push_back(0.0001);

   LambdaMixer lambdaMixer3 (percentages3, lambdaPc3, lambdaKr3);
   lambdaMixer3.mixLambdas(lambdaPcMixed, lambdaKrMixed);

   EXPECT_NEAR( lambdaPcMixed, lambdaPc3[2], lambdaPc3[2] * relativeError);
   EXPECT_NEAR( lambdaKrMixed, lambdaKr3[2], lambdaKr3[2] * relativeError);


   // Fourth scenario
   std::vector<double> percentages4, lambdaPc4, lambdaKr4;
   percentages4.push_back(35.0);
   percentages4.push_back(40.0);
   percentages4.push_back(25.0);
   lambdaPc4.push_back(0.1);
   lambdaPc4.push_back(15.0);
   lambdaPc4.push_back(2.222);
   lambdaKr4.push_back(100.0);
   lambdaKr4.push_back(3.0);
   lambdaKr4.push_back(0.0001);

   LambdaMixer lambdaMixer4 (percentages4, lambdaPc4, lambdaKr4);
   lambdaMixer4.mixLambdas(lambdaPcMixed, lambdaKrMixed);

   EXPECT_NEAR( lambdaPcMixed, 0.292366, lambdaPcMixed * 100000.0 * relativeError);
   EXPECT_NEAR( lambdaKrMixed, 0.000899847, lambdaKrMixed * 100000.0 * relativeError);


   // Fifth scenario
   std::vector<double> percentages5, lambdaPc5, lambdaKr5;
   percentages5.push_back(24.0);
   percentages5.push_back(31.0);
   percentages5.push_back(45.0);
   lambdaPc5.push_back(0.0);
   lambdaPc5.push_back(1.0);
   lambdaPc5.push_back(0.5);
   lambdaKr5.push_back(0.0);
   lambdaKr5.push_back(300.0);
   lambdaKr5.push_back(0.01);

   LambdaMixer lambdaMixer5 (percentages5, lambdaPc5, lambdaKr5);
   lambdaMixer5.mixLambdas(lambdaPcMixed, lambdaKrMixed);

   EXPECT_NEAR( lambdaPcMixed, 0.54054, lambdaPcMixed * 100000.0 * relativeError);
   EXPECT_NEAR( lambdaKrMixed, 0.0117646, lambdaKrMixed * 100000.0 * relativeError);


   // Sixth scenario
   std::vector<double> percentages6, lambdaPc6, lambdaKr6;
   percentages6.push_back(33.333);
   percentages6.push_back(33.333);
   percentages6.push_back(33.333);
   lambdaPc6.push_back(1.0);
   lambdaPc6.push_back(10.0);
   lambdaPc6.push_back(100.0);
   lambdaKr6.push_back(1.0);
   lambdaKr6.push_back(2.0);
   lambdaKr6.push_back(3.0);

   LambdaMixer lambdaMixer6 (percentages6, lambdaPc6, lambdaKr6);
   lambdaMixer6.mixLambdas(lambdaPcMixed, lambdaKrMixed);

   EXPECT_NEAR( lambdaPcMixed, 2.7027, lambdaPcMixed * 100000.0 * relativeError);
   EXPECT_NEAR( lambdaKrMixed, 1.63636, lambdaKrMixed * 100000.0 * relativeError);


   // Seventh scenario
   std::vector<double> percentages7, lambdaPc7, lambdaKr7;
   percentages7.push_back(40.0);
   percentages7.push_back(30.0);
   percentages7.push_back(30.0);
   lambdaPc7.push_back(1.0);
   lambdaPc7.push_back(0.0);
   lambdaPc7.push_back(100.0);
   lambdaKr7.push_back(1.0);
   lambdaKr7.push_back(0.0);
   lambdaKr7.push_back(3.0);

   LambdaMixer lambdaMixer7 (percentages7, lambdaPc7, lambdaKr7);
   lambdaMixer7.mixLambdas(lambdaPcMixed, lambdaKrMixed);

   EXPECT_NEAR( lambdaPcMixed, 0.0, lambdaPcMixed * relativeError);
   EXPECT_NEAR( lambdaKrMixed, 0.0, lambdaKrMixed * relativeError);
}


//
// Copyright (C) 2022 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "UndefinedValues.h"

#include <gtest/gtest.h>

TEST( UndefinedValues, testChecks )
{
   using namespace Utilities;

   int i = std::numeric_limits<int>::max();
   EXPECT_TRUE(isValueUndefined(i));

   i = Numerical::IbsNoDataValueInt;
   EXPECT_TRUE(isValueUndefined(i));

   i = Numerical::CauldronNoDataValueInt;
   EXPECT_TRUE(isValueUndefined(i));

   i -= 1;
   EXPECT_FALSE(isValueUndefined(i));

   double d = Numerical::CauldronNoDataValue;
   EXPECT_TRUE(isValueUndefined(d));

   d += Numerical::DefaultNumericalTolerance/2;
   EXPECT_TRUE(isValueUndefined(d));

   d = Numerical::IbsNoDataValue;
   EXPECT_TRUE(isValueUndefined(d));

   d += Numerical::DefaultNumericalTolerance/2;
   EXPECT_TRUE(isValueUndefined(d));

   d -= 1;
   EXPECT_FALSE(isValueUndefined(d));

   size_t s = std::numeric_limits<size_t>::max();
   EXPECT_TRUE(isValueUndefined(s));

   s +=1;
   EXPECT_FALSE(isValueUndefined(s));

   std::string str = Numerical::NoDataStringValue;
   EXPECT_TRUE(isValueUndefined(str));

   str = "";
   EXPECT_FALSE(isValueUndefined(str));
}

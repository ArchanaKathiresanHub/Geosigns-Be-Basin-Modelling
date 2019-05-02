//
// Copyright (C) 2012-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include <gtest/gtest.h>

#include "../src/SourceRock.h"

TEST( HItoHC, HItoHC )
{
   Genex6::SourceRock sr( 0, 0 );
   double hc = sr.convertHItoHC( 700 );

    EXPECT_DOUBLE_EQ( 1.378, hc );
}

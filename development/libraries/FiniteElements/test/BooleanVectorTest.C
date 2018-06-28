//
// Copyright (C) 2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by CGI.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

//
// Unit tests for FiniteElementMethod::BooleanVector
//

#include "../src/FiniteElementTypes.h"

#include <gtest/gtest.h>

TEST( BooleanVectorTest, methodsTest )
{
   FiniteElementMethod::BooleanVector boovec;
   
   // init test boolean vector
   for ( int i = 1; i <= 8; ++i ) boovec( i ) = false;
   
   // test orValues
   // to test all are false
   bool returnstatus = boovec.orValues();
   EXPECT_EQ( returnstatus, false );
   // to test any one is true
   boovec( 5 ) = true;
   returnstatus = boovec.orValues();
   EXPECT_EQ( returnstatus, true );   

   // test dimension
   EXPECT_EQ( boovec.dimension(), 8 );
   
   // test fill
   boovec.fill(true);
   for ( int i = 1; i <= 8; ++i ) EXPECT_EQ( boovec( i ), true );
}

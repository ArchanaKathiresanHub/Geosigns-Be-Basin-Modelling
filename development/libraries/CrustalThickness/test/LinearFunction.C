//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include "../src/LinearFunction.h"

// utilities library
#include "LogHandler.h"

#include <gtest/gtest.h>

///1. Test the thinning factor linear function
TEST( LinearFunction, thinningFactor )
{
   //First function test
   LinearFunction linearFunction;
   linearFunction.setTTS_crit ( 8000 );
   linearFunction.setTTS_onset( 3000 );
   linearFunction.setM1( 3e-5 );
   linearFunction.setM2( 5e-5 );
   linearFunction.setC2( 5e-3 );
   linearFunction.setMaxBasalticCrustThickness( 0 );
   linearFunction.setMagmaThicknessCoeff( 4.5 );

   //maximum basaltic crust thickness is equal to 0
   EXPECT_NEAR( 1.5e-2,  linearFunction.getCrustTF( 500   ), 1e-14 );
   EXPECT_NEAR( 1.5e-1,  linearFunction.getCrustTF( 5000  ), 1e-14 );
   EXPECT_NEAR( 0.3,     linearFunction.getCrustTF( 10000 ), 1e-14 );


   //maximum basaltic crust thickness is equal to 1000
   linearFunction.setMaxBasalticCrustThickness( 1000 );
   EXPECT_NEAR( 1.5e-2, linearFunction.getCrustTF( 500   ), 1e-14 );
   EXPECT_NEAR( 0.255,  linearFunction.getCrustTF( 5000  ), 1e-14 );
   EXPECT_EQ  ( 1,      linearFunction.getCrustTF( 10000 ) );

   //Second function test
   linearFunction.setTTS_crit ( 1000 );
   linearFunction.setTTS_onset( 500 );
   linearFunction.setM1( 1e-3   );
   linearFunction.setM2( 4e-3   );
   linearFunction.setC2( 100e-3 );
   linearFunction.setMaxBasalticCrustThickness( 0 );
   linearFunction.setMagmaThicknessCoeff( 4.5 );

   //maximum basaltic crust thickness is equal to 0
   EXPECT_NEAR( 0.2, linearFunction.getCrustTF( 200  ), 1e-14 );
   EXPECT_NEAR( 0.8, linearFunction.getCrustTF( 800  ), 1e-14 );
   EXPECT_NEAR( 2.0, linearFunction.getCrustTF( 2000 ), 1e-14 );

   //maximum basaltic crust thickness is equal to 2000
   linearFunction.setMaxBasalticCrustThickness( 2000 );
   EXPECT_NEAR( 0.2, linearFunction.getCrustTF( 200  ), 1e-14 );
   EXPECT_NEAR( 3.3, linearFunction.getCrustTF( 800  ), 1e-14 );
   EXPECT_EQ  ( 1.0, linearFunction.getCrustTF( 2000 ) );
}

///1. Test the basalt thickness linear function
TEST( LinearFunction, basaltThickness )
{
   //First function test
   LinearFunction linearFunction;
   linearFunction.setTTS_crit ( 8000 );
   linearFunction.setTTS_onset( 3000 );
   linearFunction.setM1( 3e-5   );
   linearFunction.setM2( 5e-5   );
   linearFunction.setC2( 500e-5 );
   linearFunction.setMaxBasalticCrustThickness( 0 );
   linearFunction.setMagmaThicknessCoeff( 0.1 );

   //maximum basaltic crust thickness is equal to 0
   EXPECT_EQ( 0.0, linearFunction.getBasaltThickness( 500   ) );
   EXPECT_EQ( 0.0, linearFunction.getBasaltThickness( 5000  ) );
   EXPECT_EQ( 0.0, linearFunction.getBasaltThickness( 10000 ) );


   //maximum basaltic crust thickness is equal to 1000
   linearFunction.setMaxBasalticCrustThickness( 1000 );
   EXPECT_EQ( 0.0, linearFunction.getBasaltThickness( 500 ) );
   EXPECT_NEAR( 400.0, linearFunction.getBasaltThickness( 5000   ), 1e-14 );
   EXPECT_NEAR( 800.0, linearFunction.getBasaltThickness( 10000  ), 1e-14 );
   EXPECT_EQ( 0.0, linearFunction.getBasaltThickness( 100000 ) );

   //Second function test
   linearFunction.setTTS_crit( 1000 );
   linearFunction.setTTS_onset( 500 );
   linearFunction.setM1( 1e-3   );
   linearFunction.setM2( 4e-3   );
   linearFunction.setC2( 100e-3 );
   linearFunction.setMaxBasalticCrustThickness( 0 );
   linearFunction.setMagmaThicknessCoeff( 1 );

   //maximum basaltic crust thickness is equal to 0
   EXPECT_EQ( 0.0, linearFunction.getBasaltThickness( 200  ) );
   EXPECT_EQ( 0.0, linearFunction.getBasaltThickness( 800  ) );
   EXPECT_EQ( 0.0, linearFunction.getBasaltThickness( 2000 ) );

   //maximum basaltic crust thickness is equal to 2000
   linearFunction.setMaxBasalticCrustThickness( 2000 );
   EXPECT_EQ( 0.0,    linearFunction.getBasaltThickness( 200    ) );
   EXPECT_NEAR( 1200.0, linearFunction.getBasaltThickness( 800    ) , 1e-14 );
   EXPECT_NEAR( 1100,   linearFunction.getBasaltThickness( 1900   ) , 1e-14 );
   EXPECT_EQ( 0.0,    linearFunction.getBasaltThickness( 100000 ) );

}

///3. Test the printing functionality
TEST( LinearFunction, print )
{
   LinearFunction linearFunction;
   linearFunction.setTTS_crit( 8000 );
   linearFunction.setTTS_onset( 3000 );
   linearFunction.setM1( 3e-5   );
   linearFunction.setM2( 5e-5   );
   linearFunction.setC2( 500e-5 );
   linearFunction.setMaxBasalticCrustThickness( 0 );
   linearFunction.setMagmaThicknessCoeff( 0.1 );
   LogHandler( "fastctc", LogHandler::DETAILED_LEVEL, 0 );
   testing::internal::CaptureStdout();
   linearFunction.printCoeffs();
   std::string output = testing::internal::GetCapturedStdout();
   std::stringstream expectedResult;
   expectedResult << "Linear function is defined by:\n"
                  << "   #m1 = " << 3e-5 << "\n"
                  << "   #m2 = " << 5e-5 << "\n"
                  << "   #c2 = " << 500e-5 << "\n";
   EXPECT_EQ( expectedResult.str(), output );
}


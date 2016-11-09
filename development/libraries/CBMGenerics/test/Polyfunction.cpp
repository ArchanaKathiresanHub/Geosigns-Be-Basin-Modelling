//
// Copyright (C) 2012-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "../src/Polyfunction.h"
#include <gtest/gtest.h>

//define test
void doTest( CBMGenerics::Polyfunction & interp, double xe[], double fec[], const int esize )
{

   for ( int i = 0; i < esize; ++i )
   {
      EXPECT_NEAR( interp.F(xe[i]), fec[i], 1e-10 );
   }
}


TEST(Polyfunction,Polyfunction)
{
   const int xsize = 3, esize = 7;
   double xs[xsize], fs[xsize], xe[esize], fec[esize];
   CBMGenerics::Polyfunction interp;

   //assigned x and f values
   xs[0] = 0.0;	fs[0] = 1.0;
   xs[1] = 1.0;	fs[1] = 0.0;
   xs[2] = 2.0;	fs[2] = 1.0;

   //add points to the interpolation table
   interp.AddPoint(xs[0], fs[0]);
   interp.AddPoint(xs[1], fs[1]);
   interp.AddPoint(xs[2], fs[2]);

   //expected f values at the x points (fec=constant extrapolation)
   xe[0] = -1;	  fec[0] = 1;
   xe[1] =  0;	  fec[1] = 1;
   xe[2] =  0.5; fec[2] = 0.5;
   xe[3] =  1;	  fec[3] = 0;
   xe[4] =  1.5; fec[4] = 0.5;
   xe[5] =  2;	  fec[5] = 1;
   xe[6] =  3;	  fec[6] = 1;

   doTest(interp, xe, fec, esize);
}


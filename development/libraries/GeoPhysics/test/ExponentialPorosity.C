//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "../src/ExponentialPorosity.h"

#include <iostream>
#include <iomanip>
#include <cstring>
#include <cassert>
#include <cmath>
#include <limits>

#include <gtest/gtest.h>

using namespace GeoPhysics;

namespace {
const double NaN = std::numeric_limits<double>::quiet_NaN();
}

TEST( ExponentialPorosity, Sandstone )
{

   EXPECT_NEAR(3.061535394818E-01, ExponentialPorosity( 0.39, 0.03, 2.66E-07,2.66E-08).calculate( 1E+5, 1.0E+6, false,0),  1E-10);
   
   EXPECT_NEAR(4.674003186721E-01, ExponentialPorosity( 0.48, 0.03, 2.66E-07,2.66E-08).calculate( 1E+5, 1.0E+5, false,0),  1E-10);
   
   EXPECT_NEAR(5.332096829496E-02, ExponentialPorosity( 0.60, 0.03, 2.66E-07,2.66E-08).calculate( 1E+6, 1.0E+7, false,0),  1E-10);
   
   EXPECT_NEAR(6.987048639445E-02, ExponentialPorosity( 0.60, 0.03, 2.66E-07,2.66E-08).calculate( 1E+7, 1.0E+7, true,0),  1E-10);
   
}

TEST( ExponentialPorosity, Shale )
{

   EXPECT_NEAR(6.413671276713E-01, ExponentialPorosity( 0.7, 0.03, 9.61300E-08,9.61300E-09).calculate( 1E+5, 1.0E+6, false,0),  1E-10);
   
   EXPECT_NEAR(6.441768903789E-01, ExponentialPorosity( 0.65, 0.03, 8.99900E-08,8.99900E-09).calculate( 1E+5, 1.0E+5, false,0),  1E-10);

   EXPECT_NEAR(2.892662986297E-01, ExponentialPorosity( 0.67, 0.03, 9.23000E-08,9.23000E-09).calculate( 1.00E+06, 1.0E+7, false,0), 1E-10);
   
   EXPECT_NEAR(2.545678104070E-01, ExponentialPorosity( 0.7, 0.03, 1.09310E-07,1.09310E-08).calculate( 1E+7, 1.0E+7, true,0),  1E-10);
   
}


TEST( ExponentialPorosity, DerivativesExactValues )
{
   // loading phase, no chemical compaction, variations wrt ves, compactionIncr = 2.66E-07, compactionDecr = 2.66E-08
   EXPECT_NEAR(-1.064000000000E-07, ExponentialPorosity( 0.4, 0.03, 2.66E-07, 2.66E-08).calculateDerivative( 0.0     , 0.0     , false,0),  1.0E-18);
   EXPECT_NEAR(-1.063997169764E-07, ExponentialPorosity( 0.4, 0.03, 2.66E-07, 2.66E-08).calculateDerivative( 1.00E+01, 1.00E+01, false,0),  1.0E-18);
   EXPECT_NEAR(-1.063971697976E-07, ExponentialPorosity( 0.4, 0.03, 2.66E-07, 2.66E-08).calculateDerivative( 1.00E+02, 1.00E+02, false,0),  1.0E-18);
   EXPECT_NEAR(-1.063717013639E-07, ExponentialPorosity( 0.4, 0.03, 2.66E-07, 2.66E-08).calculateDerivative( 1.00E+03, 1.00E+03, false,0),  1.0E-18);
   EXPECT_NEAR(-1.061173520884E-07, ExponentialPorosity( 0.4, 0.03, 2.66E-07, 2.66E-08).calculateDerivative( 1.00E+04, 1.00E+04, false,0),  1.0E-18);
   EXPECT_NEAR(-1.036070706390E-07, ExponentialPorosity( 0.4, 0.03, 2.66E-07, 2.66E-08).calculateDerivative( 1.00E+05, 1.00E+05, false,0),  1.0E-18);
   EXPECT_NEAR(-8.154912316611E-08, ExponentialPorosity( 0.4, 0.03, 2.66E-07, 2.66E-08).calculateDerivative( 1.00E+06, 1.00E+06, false,0),  1.0E-18);
   EXPECT_NEAR(-7.442490793631E-09, ExponentialPorosity( 0.4, 0.03, 2.66E-07, 2.66E-08).calculateDerivative( 1.00E+07, 1.00E+07, false,0),  1.0E-18);
   
   // unloading phase, no chemical compaction, variations wrt ves, compactionIncr = 2.66E-07, compactionDecr = 2.66E-08
   EXPECT_NEAR(-1.063994622558E-08, ExponentialPorosity( 0.4, 0.03, 2.66E-07, 2.66E-08).calculateDerivative( 1.00E+01, 2.00E+01, false,0),  1.0E-18);
   EXPECT_NEAR(-1.063946226799E-08, ExponentialPorosity( 0.4, 0.03, 2.66E-07, 2.66E-08).calculateDerivative( 1.00E+02, 2.00E+02, false,0),  1.0E-18);
   EXPECT_NEAR(-1.063462390265E-08, ExponentialPorosity( 0.4, 0.03, 2.66E-07, 2.66E-08).calculateDerivative( 1.00E+03, 2.00E+03, false,0),  1.0E-18);
   EXPECT_NEAR(-1.058636109968E-08, ExponentialPorosity( 0.4, 0.03, 2.66E-07, 2.66E-08).calculateDerivative( 1.00E+04, 2.00E+04, false,0),  1.0E-18);
   EXPECT_NEAR(-1.011561716829E-08, ExponentialPorosity( 0.4, 0.03, 2.66E-07, 2.66E-08).calculateDerivative( 1.00E+05, 2.00E+05, false,0),  1.0E-18);
   EXPECT_NEAR(-6.418731316450E-09, ExponentialPorosity( 0.4, 0.03, 2.66E-07, 2.66E-08).calculateDerivative( 1.00E+06, 2.00E+06, false,0),  1.0E-18);
   EXPECT_NEAR(-6.792307147247E-11, ExponentialPorosity( 0.4, 0.03, 2.66E-07, 2.66E-08).calculateDerivative( 1.00E+07, 2.00E+07, false,0),  1.0E-18);

   // loading phase, no chemical compaction, variations wrt ves, compactionIncr = 9.23E-08, compactionDecr = 9.23E-09
   EXPECT_NEAR(-3.692000000000E-08, ExponentialPorosity( 0.4, 0.03, 9.23E-08, 9.23E-09).calculateDerivative( 0.0     , 0.0     , false,0),  1.0E-18);
   EXPECT_NEAR(-3.691996592286E-08, ExponentialPorosity( 0.4, 0.03, 9.23E-08, 9.23E-09).calculateDerivative( 1.00E+01, 1.00E+01, false,0),  1.0E-18);
   EXPECT_NEAR(-3.691965922997E-08, ExponentialPorosity( 0.4, 0.03, 9.23E-08, 9.23E-09).calculateDerivative( 1.00E+02, 1.00E+02, false,0),  1.0E-18);
   EXPECT_NEAR(-3.691659244126E-08, ExponentialPorosity( 0.4, 0.03, 9.23E-08, 9.23E-09).calculateDerivative( 1.00E+03, 1.00E+03, false,0),  1.0E-18);
   EXPECT_NEAR(-3.688593856177E-08, ExponentialPorosity( 0.4, 0.03, 9.23E-08, 9.23E-09).calculateDerivative( 1.00E+04, 1.00E+04, false,0),  1.0E-18);
   EXPECT_NEAR(-3.658079623352E-08, ExponentialPorosity( 0.4, 0.03, 9.23E-08, 9.23E-09).calculateDerivative( 1.00E+05, 1.00E+05, false,0),  1.0E-18);
   EXPECT_NEAR(-3.366482115979E-08, ExponentialPorosity( 0.4, 0.03, 9.23E-08, 9.23E-09).calculateDerivative( 1.00E+06, 1.00E+06, false,0),  1.0E-18);
   EXPECT_NEAR(-1.466924917164E-08, ExponentialPorosity( 0.4, 0.03, 9.23E-08, 9.23E-09).calculateDerivative( 1.00E+07, 1.00E+07, false,0),  1.0E-18);
   
   // unloading phase, no chemical compaction, variations wrt ves, compactionIncr = 9.23000E-08, compactionDecr = 9.23000E-09
   EXPECT_NEAR(-3.691993525345E-09, ExponentialPorosity( 0.4, 0.03, 9.23E-08, 9.23E-09).calculateDerivative( 1.00E+01, 2.00E+01, false,0),  1.0E-18);
   EXPECT_NEAR(-3.691935253964E-09, ExponentialPorosity( 0.4, 0.03, 9.23E-08, 9.23E-09).calculateDerivative( 1.00E+02, 2.00E+02, false,0),  1.0E-18);
   EXPECT_NEAR(-3.691352590730E-09, ExponentialPorosity( 0.4, 0.03, 9.23E-08, 9.23E-09).calculateDerivative( 1.00E+03, 2.00E+03, false,0),  1.0E-18);
   EXPECT_NEAR(-3.685531013589E-09, ExponentialPorosity( 0.4, 0.03, 9.23E-08, 9.23E-09).calculateDerivative( 1.00E+04, 2.00E+04, false,0),  1.0E-18);
   EXPECT_NEAR(-3.627817822333E-09, ExponentialPorosity( 0.4, 0.03, 9.23E-08, 9.23E-09).calculateDerivative( 1.00E+05, 2.00E+05, false,0),  1.0E-18);
   EXPECT_NEAR(-3.098128800931E-09, ExponentialPorosity( 0.4, 0.03, 9.23E-08, 9.23E-09).calculateDerivative( 1.00E+06, 2.00E+06, false,0),  1.0E-18);
   EXPECT_NEAR(-6.392039638006E-10, ExponentialPorosity( 0.4, 0.03, 9.23E-08, 9.23E-09).calculateDerivative( 1.00E+07, 2.00E+07, false,0),  1.0E-18);

   // loading phase, chemical compaction, variations wrt chemical compaction, compactionIncr = 2.66E-07, compactionDecr = 2.66E-08
   EXPECT_NEAR(-9.839382376159E-08, ExponentialPorosity( 0.4, 0.03, 2.66E-07, 2.66E-08).calculateDerivative( 1.00E+03, 1.00E+03, true, 0.     ),  1.0E-18);
   EXPECT_NEAR(-9.839382376159E-08, ExponentialPorosity( 0.4, 0.03, 2.66E-07, 2.66E-08).calculateDerivative( 1.00E+03, 1.00E+03, true, -5.0E-2),  1.0E-18);
   EXPECT_NEAR(-9.839382376159E-08, ExponentialPorosity( 0.4, 0.03, 2.66E-07, 2.66E-08).calculateDerivative( 1.00E+03, 1.00E+03, true, -1.0E-1),  1.0E-18);
   EXPECT_NEAR(-9.839382376159E-08, ExponentialPorosity( 0.4, 0.03, 2.66E-07, 2.66E-08).calculateDerivative( 1.00E+03, 1.00E+03, true, -1.5E-1),  1.0E-18);
   EXPECT_NEAR(-9.839382376159E-08, ExponentialPorosity( 0.4, 0.03, 2.66E-07, 2.66E-08).calculateDerivative( 1.00E+03, 1.00E+03, true, -2.0E-1),  1.0E-18);
   EXPECT_NEAR(-9.839382376159E-08, ExponentialPorosity( 0.4, 0.03, 2.66E-07, 2.66E-08).calculateDerivative( 1.00E+03, 1.00E+03, true, -2.5E-1),  1.0E-18);
   EXPECT_NEAR(-9.839382376159E-08, ExponentialPorosity( 0.4, 0.03, 2.66E-07, 2.66E-08).calculateDerivative( 1.00E+03, 1.00E+03, true, -3.0E-1),  1.0E-18);
   EXPECT_NEAR(-9.839382376159E-08, ExponentialPorosity( 0.4, 0.03, 2.66E-07, 2.66E-08).calculateDerivative( 1.00E+03, 1.00E+03, true, -3.5E-1),  1.0E-18);
   EXPECT_NEAR(0.                 , ExponentialPorosity( 0.4, 0.03, 2.66E-07, 2.66E-08).calculateDerivative( 1.00E+03, 1.00E+03, true, -4.0E-1),  1.0E-18);
}


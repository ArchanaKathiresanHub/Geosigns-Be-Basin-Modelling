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

   EXPECT_NEAR( 3.12603267213E-01, ExponentialPorosity( 0.39, 0.03, 2.66E-07, 2.66E-08 ).calculate( 1E+5, 1.0E+6, false, 0 ), 1E-10 );
   
   EXPECT_NEAR( 4.68187798755E-01, ExponentialPorosity( 0.48, 0.03, 2.66E-07, 2.66E-08 ).calculate( 1E+5, 1.0E+5, false, 0 ), 1E-10 );
   
   EXPECT_NEAR( 8.06549198802E-02, ExponentialPorosity( 0.60, 0.03, 2.66E-07, 2.66E-08 ).calculate( 1E+6, 1.0E+7, false, 0 ), 1E-10 );
   
   EXPECT_NEAR( 6.98704863944E-02, ExponentialPorosity( 0.60, 0.03, 2.66E-07, 2.66E-08 ).calculate( 1E+7, 1.0E+7, true, 0 ),  1E-10 );
   
}

TEST( ExponentialPorosity, Shale )
{

   EXPECT_NEAR( 6.438799650567E-01, ExponentialPorosity( 0.7, 0.03, 9.61300E-08, 9.61300E-09 ).calculate( 1E+5, 1.0E+6, false, 0 ), 1E-10 );
   
   EXPECT_NEAR( 6.444456492845E-01, ExponentialPorosity( 0.65, 0.03, 8.99900E-08, 8.99900E-09 ).calculate( 1E+5, 1.0E+5, false, 0 ), 1E-10 );

   EXPECT_NEAR( 3.063140763029E-01, ExponentialPorosity( 0.67, 0.03, 9.23000E-08, 9.23000E-09 ).calculate( 1.00E+06, 1.0E+7, false, 0 ), 1E-10 );
   
   EXPECT_NEAR( 2.545678104070E-01, ExponentialPorosity( 0.7, 0.03, 1.09310E-07, 1.09310E-08 ).calculate( 1E+7, 1.0E+7, true, 0 ), 1E-10 );
   
}


TEST( ExponentialPorosity, DerivativesExactValues )
{
   // loading phase, no chemical compaction, variations wrt ves, compactionIncr = 2.66E-07, compactionDecr = 2.66E-08
   EXPECT_NEAR( -9.842000000000E-08, ExponentialPorosity( 0.4, 0.03, 2.66E-07, 2.66E-08 ).calculateDerivative( 0.0, 0.0, false, 0 ), 1.0E-18 );
   EXPECT_NEAR( -9.841973820314E-08, ExponentialPorosity( 0.4, 0.03, 2.66E-07, 2.66E-08 ).calculateDerivative( 1.00E+01, 1.00E+01, false, 0 ), 1.0E-18 );
   EXPECT_NEAR( -9.841738206281E-08, ExponentialPorosity( 0.4, 0.03, 2.66E-07, 2.66E-08 ).calculateDerivative( 1.00E+02, 1.00E+02, false, 0 ), 1.0E-18 );
   EXPECT_NEAR( -9.839382376159E-08, ExponentialPorosity( 0.4, 0.03, 2.66E-07, 2.66E-08 ).calculateDerivative( 1.00E+03, 1.00E+03, false, 0 ), 1.0E-18 );
   EXPECT_NEAR( -9.815855068175E-08, ExponentialPorosity( 0.4, 0.03, 2.66E-07, 2.66E-08 ).calculateDerivative( 1.00E+04, 1.00E+04, false, 0 ), 1.0E-18 );
   EXPECT_NEAR( -9.583654034106E-08, ExponentialPorosity( 0.4, 0.03, 2.66E-07, 2.66E-08 ).calculateDerivative( 1.00E+05, 1.00E+05, false, 0 ), 1.0E-18 );
   EXPECT_NEAR( -7.543293892865E-08, ExponentialPorosity( 0.4, 0.03, 2.66E-07, 2.66E-08 ).calculateDerivative( 1.00E+06, 1.00E+06, false, 0 ), 1.0E-18 );
   EXPECT_NEAR( -6.884303984108E-09, ExponentialPorosity( 0.4, 0.03, 2.66E-07, 2.66E-08 ).calculateDerivative( 1.00E+07, 1.00E+07, false, 0 ), 1.0E-18 );
   
   // unloading phase, no chemical compaction, variations wrt ves, compactionIncr = 2.66E-07, compactionDecr = 2.66E-08
   EXPECT_NEAR( -9.841950258657E-09, ExponentialPorosity( 0.4, 0.03, 2.66E-07, 2.66E-08 ).calculateDerivative( 1.00E+01, 2.00E+01, false, 0 ), 1.0E-18 );
   EXPECT_NEAR( -9.841502597889E-09, ExponentialPorosity( 0.4, 0.03, 2.66E-07, 2.66E-08 ).calculateDerivative( 1.00E+02, 2.00E+02, false, 0 ), 1.0E-18 );
   EXPECT_NEAR( -9.837027109955E-09, ExponentialPorosity( 0.4, 0.03, 2.66E-07, 2.66E-08 ).calculateDerivative( 1.00E+03, 2.00E+03, false, 0 ), 1.0E-18 );
   EXPECT_NEAR( -9.792384017199E-09, ExponentialPorosity( 0.4, 0.03, 2.66E-07, 2.66E-08 ).calculateDerivative( 1.00E+04, 2.00E+04, false, 0 ), 1.0E-18 );
   EXPECT_NEAR( -9.356945880672E-09, ExponentialPorosity( 0.4, 0.03, 2.66E-07, 2.66E-08 ).calculateDerivative( 1.00E+05, 2.00E+05, false, 0 ), 1.0E-18 );
   EXPECT_NEAR( -5.937326467716E-09, ExponentialPorosity( 0.4, 0.03, 2.66E-07, 2.66E-08 ).calculateDerivative( 1.00E+06, 2.00E+06, false, 0 ), 1.0E-18 );
   EXPECT_NEAR( -6.282884111203E-11, ExponentialPorosity( 0.4, 0.03, 2.66E-07, 2.66E-08 ).calculateDerivative( 1.00E+07, 2.00E+07, false, 0 ), 1.0E-18 );

   // loading phase, no chemical compaction, variations wrt ves, compactionIncr = 9.23E-08, compactionDecr = 9.23E-09
   EXPECT_NEAR( -3.415099999999E-08, ExponentialPorosity( 0.4, 0.03, 9.23E-08, 9.23E-09 ).calculateDerivative( 0.0, 0.0, false, 0 ), 1.0E-18 );
   EXPECT_NEAR( -3.415096847864E-08, ExponentialPorosity( 0.4, 0.03, 9.23E-08, 9.23E-09 ).calculateDerivative( 1.00E+01, 1.00E+01, false, 0 ), 1.0E-18 );
   EXPECT_NEAR( -3.415068478772E-08, ExponentialPorosity( 0.4, 0.03, 9.23E-08, 9.23E-09 ).calculateDerivative( 1.00E+02, 1.00E+02, false, 0 ), 1.0E-18 );
   EXPECT_NEAR( -3.414784800816E-08, ExponentialPorosity( 0.4, 0.03, 9.23E-08, 9.23E-09 ).calculateDerivative( 1.00E+03, 1.00E+03, false, 0 ), 1.0E-18 );
   EXPECT_NEAR( -3.411949316963E-08, ExponentialPorosity( 0.4, 0.03, 9.23E-08, 9.23E-09 ).calculateDerivative( 1.00E+04, 1.00E+04, false, 0 ), 1.0E-18 );
   EXPECT_NEAR( -3.383723651601E-08, ExponentialPorosity( 0.4, 0.03, 9.23E-08, 9.23E-09 ).calculateDerivative( 1.00E+05, 1.00E+05, false, 0 ), 1.0E-18 );
   EXPECT_NEAR( -3.113995957280E-08, ExponentialPorosity( 0.4, 0.03, 9.23E-08, 9.23E-09 ).calculateDerivative( 1.00E+06, 1.00E+06, false, 0 ), 1.0E-18 );
   EXPECT_NEAR( -1.356905548376E-08, ExponentialPorosity( 0.4, 0.03, 9.23E-08, 9.23E-09 ).calculateDerivative( 1.00E+07, 1.00E+07, false, 0 ), 1.0E-18 );
   
   // unloading phase, no chemical compaction, variations wrt ves, compactionIncr = 9.23000E-08, compactionDecr = 9.23000E-09
   EXPECT_NEAR( -3.415094010944E-09, ExponentialPorosity( 0.4, 0.03, 9.23E-08, 9.23E-09 ).calculateDerivative( 1.00E+01, 2.00E+01, false, 0 ), 1.0E-18 );
   EXPECT_NEAR( -3.415040109916E-09, ExponentialPorosity( 0.4, 0.03, 9.23E-08, 9.23E-09 ).calculateDerivative( 1.00E+02, 2.00E+02, false, 0 ), 1.0E-18 );
   EXPECT_NEAR( -3.414501146425E-09, ExponentialPorosity( 0.4, 0.03, 9.23E-08, 9.23E-09 ).calculateDerivative( 1.00E+03, 2.00E+03, false, 0 ), 1.0E-18 );
   EXPECT_NEAR( -3.409116187569E-09, ExponentialPorosity( 0.4, 0.03, 9.23E-08, 9.23E-09 ).calculateDerivative( 1.00E+04, 2.00E+04, false, 0 ), 1.0E-18 );
   EXPECT_NEAR( -3.355731485657E-09, ExponentialPorosity( 0.4, 0.03, 9.23E-08, 9.23E-09 ).calculateDerivative( 1.00E+05, 2.00E+05, false, 0 ), 1.0E-18 );
   EXPECT_NEAR( -2.865769140860E-09, ExponentialPorosity( 0.4, 0.03, 9.23E-08, 9.23E-09 ).calculateDerivative( 1.00E+06, 2.00E+06, false, 0 ), 1.0E-18 );
   EXPECT_NEAR( -5.912636665155E-10, ExponentialPorosity( 0.4, 0.03, 9.23E-08, 9.23E-09 ).calculateDerivative( 1.00E+07, 2.00E+07, false, 0 ), 1.0E-18 );

   // loading phase, chemical compaction, variations wrt chemical compaction, compactionIncr = 2.66E-07, compactionDecr = 2.66E-08
   EXPECT_NEAR( -9.839382376159E-08, ExponentialPorosity( 0.4, 0.03, 2.66E-07, 2.66E-08 ).calculateDerivative( 1.00E+03, 1.00E+03, true, 0. ), 1.0E-18 );
   EXPECT_NEAR( -9.839382376159E-08, ExponentialPorosity( 0.4, 0.03, 2.66E-07, 2.66E-08 ).calculateDerivative( 1.00E+03, 1.00E+03, true, -5.0E-2 ), 1.0E-18 );
   EXPECT_NEAR( -9.839382376159E-08, ExponentialPorosity( 0.4, 0.03, 2.66E-07, 2.66E-08 ).calculateDerivative( 1.00E+03, 1.00E+03, true, -1.0E-1 ), 1.0E-18 );
   EXPECT_NEAR( -9.839382376159E-08, ExponentialPorosity( 0.4, 0.03, 2.66E-07, 2.66E-08 ).calculateDerivative( 1.00E+03, 1.00E+03, true, -1.5E-1 ), 1.0E-18 );
   EXPECT_NEAR( -9.839382376159E-08, ExponentialPorosity( 0.4, 0.03, 2.66E-07, 2.66E-08 ).calculateDerivative( 1.00E+03, 1.00E+03, true, -2.0E-1 ), 1.0E-18 );
   EXPECT_NEAR( -9.839382376159E-08, ExponentialPorosity( 0.4, 0.03, 2.66E-07, 2.66E-08 ).calculateDerivative( 1.00E+03, 1.00E+03, true, -2.5E-1 ), 1.0E-18 );
   EXPECT_NEAR( -9.839382376159E-08, ExponentialPorosity( 0.4, 0.03, 2.66E-07, 2.66E-08 ).calculateDerivative( 1.00E+03, 1.00E+03, true, -3.0E-1 ), 1.0E-18 );
   EXPECT_NEAR( -9.839382376159E-08, ExponentialPorosity( 0.4, 0.03, 2.66E-07, 2.66E-08 ).calculateDerivative( 1.00E+03, 1.00E+03, true, -3.5E-1 ), 1.0E-18 );
   EXPECT_NEAR( 0., ExponentialPorosity( 0.4, 0.03, 2.66E-07, 2.66E-08 ).calculateDerivative( 1.00E+03, 1.00E+03, true, -4.0E-1 ), 1.0E-18 );
}
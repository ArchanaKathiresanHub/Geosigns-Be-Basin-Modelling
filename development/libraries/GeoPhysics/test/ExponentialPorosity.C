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

#include "../src/SoilMechanicsPorosity.h"

#include <iostream>
#include <iomanip>
#include <cstring>
#include <cassert>
#include <cmath>
#include <limits>

#include <gtest/gtest.h>

using namespace GeoPhysics;


TEST( Mudstone, Mudstone )
{

      EXPECT_NEAR(4.992470160211E-01, soilMechanicsPorosity( 0.6, 0.03, 0.222,1.50E+00).calculate( 1.0E+5, 1.0E+6, false,0),  1E-10);

      EXPECT_NEAR(5.632000000000E-01, soilMechanicsPorosity( 0.5632, 0.03, 0.199,1.289377289377E+00).calculate( 1.0E+5, 1.0E+5, false,0),  1E-10);
     
      EXPECT_NEAR(3.150631892440E-01, soilMechanicsPorosity( 0.62, 0.03, 0.257,1.631578947368E+00).calculate( 1.0E+6, 1.0E+7, false,0),  1E-10);

      EXPECT_NEAR(3.579932495799E-01, soilMechanicsPorosity( 0.6685, 0.03, 0.322,2.016591251885E+00).calculate( 1.0E+7, 1.0E+7, true,0.01),  1E-10);
}

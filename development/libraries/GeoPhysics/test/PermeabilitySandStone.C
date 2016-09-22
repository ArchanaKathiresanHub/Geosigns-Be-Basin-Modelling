//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "../src/PermeabilitySandStone.h"

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

//Input domain: PermeabilitySandStone :: permeability
// permeabilityIncr = 1.5
// depositionalPermeability = 6000 mD  
// depositionalPorosity = 39%, 41%, 42%, 48%, 50%, 60%, 70%

// ves: invalid (-infinity, -1.0e+5) [1.0e+5, 0). valid = {0}, (0, 1.0e+5), [1.0e+5, infinity)
// maxVes: invalid = (-infinity, 0), {0}, [0, ves).  valid = [ves, infinity)
// calculatedPorosity: invalid = (-infinity, 0), (1, infinity). valid = {0}, (0, 1), {1}
 
TEST( PermeabilitySandStonePermeability, invalidVes )
{
   // The weird/invalid cases
   // varying ves invalid classes
   EXPECT_NEAR(8.47522526773653202270e-03, PermeabilitySandStone( 0.39, 6000, 1.5).calculate( -1e+6, 1.0e+5, 0),  2e-17);

   EXPECT_DOUBLE_EQ(1.34323268314100546794e-01, PermeabilitySandStone( 0.41, 6000, 1.5).calculate( -1e+5, 1.0e+5, 0.1) );
}

TEST( PermeabilitySandStonePermeability, invalidMaxVes )
{   // varying maxVes invalid classes
   EXPECT_DOUBLE_EQ(1000.0 , PermeabilitySandStone( 0.48, 6000, 1.5).calculate( 0, -1e+6, 1) );

   EXPECT_DOUBLE_EQ(1.89736659610102758244e-04, PermeabilitySandStone( 0.50, 6000, 1.5).calculate( 1e+4, 0, 0) );

   EXPECT_DOUBLE_EQ(1.89736659610102775808e-01, PermeabilitySandStone( 0.60, 6000, 1.5).calculate( 1e+6, 1e+3, 0.3) );
}

TEST( PermeabilitySandStonePermeability, invalidCalculatedPorosity )
{  // varying calculatedPorosity invalid classes
   EXPECT_NEAR(1.89736659610102764348e-82, PermeabilitySandStone( 0.70, 6000, 1.5).calculate( 0,0, -5), 2e-96 );

   EXPECT_DOUBLE_EQ(1000.0, PermeabilitySandStone( 0.35, 6000, 1.5).calculate( 1e+4, 1e+5, 5) );
}

TEST( PermeabilitySandStonePermeability, validCases )
{
   // The normal cases
   EXPECT_NEAR(8.47522526773653202270e-03 , PermeabilitySandStone( 0.39, 6000, 1.5).calculate( 1.0e+5, 1.0e+5, 0.0), 2e-17 );

   EXPECT_NEAR(3.78574406688116593678e-01, PermeabilitySandStone( 0.48, 6000, 1.5).calculate( 1.0e+6, 1.0e+6, 0.2), 3e-15 );

   EXPECT_DOUBLE_EQ(1.89736659610102775808e-01, PermeabilitySandStone( 0.60, 6000, 1.5).calculate( 1.0e+6, 2.0e+6, 0.3) );

   EXPECT_DOUBLE_EQ(1000.0, PermeabilitySandStone( 0.60, 6000, 1.5).calculate( 1.0e+6, 2.0e+6, 0.6) );
}

//Input domain:
// permeabilityIncr = 1.5
// depositionalPermeability = 6000 mD  
// depositionalPorosity = 39%, 41%, 42%, 48%, 50%, 60%, 70%

// ves: invalid (-infinity, -1.0e+5) [1.0e+5, 0). valid = {0}, (0, 1.0e+5), [1.0e+5, infinity)
// maxVes: invalid = (-infinity, 0), {0}, [0, ves).  valid = [ves, infinity)
// calculatedPorosity: invalid = (-infinity, 0), (1, infinity). valid = {0}, (0, 1), {1}

// The weird cases
TEST( PermeabilitySandStonePermeabilityDerivative, invalidLargeNegativeVes)
{
   double permeability = NaN, derivative = NaN;
   PermeabilitySandStone( 0.39, 6000, 1.5).calculateDerivative( -1e+6, 1.0e+5, 0, 1, permeability, derivative);
   EXPECT_NEAR(8.47522526773653202270e-03, permeability, 2e-17);
   EXPECT_NEAR(2.92723910418849148432e-01, derivative, 1e-15 );
}

TEST( PermeabilitySandStonePermeabilityDerivative, invalidSmallNegativeVes)
{
   double permeability = NaN, derivative = NaN;

   PermeabilitySandStone( 0.41, 6000, 1.5).calculateDerivative( -1e+5, 1.0e+5, 0.1, 1, permeability, derivative);
   EXPECT_DOUBLE_EQ(1.34323268314100546794e-01, permeability);
   EXPECT_DOUBLE_EQ(4.63936132893431052437, derivative);
}

TEST( PermeabilitySandStonePermeabilityDerivative, invalidLargeNegativeMaxVes)
{
   double permeability = NaN, derivative = NaN;
   PermeabilitySandStone( 0.48, 6000, 1.5).calculateDerivative( 0, -1e+6, 1, 1, permeability, derivative);
   EXPECT_DOUBLE_EQ(1000.0 , permeability );
   EXPECT_DOUBLE_EQ(0.0, derivative );
}

TEST( PermeabilitySandStonePermeabilityDerivative, invalidSmallerwMaxVes)
{
   double permeability = NaN, derivative = NaN;
   PermeabilitySandStone( 0.50, 6000, 1.5).calculateDerivative( 1e+4, 0, 0, 1, permeability, derivative);
   EXPECT_DOUBLE_EQ(1.89736659610102758244e-04, permeability );
   EXPECT_DOUBLE_EQ(6.55327206019062116467e-03, derivative);
}

TEST( PermeabilitySandStonePermeabilityDerivative, invalidNormalVesSmallerMaxVes)
{
   double permeability = NaN, derivative = NaN;
   PermeabilitySandStone( 0.60, 6000, 1.5).calculateDerivative( 1e+6, 1e+3, 0.3, 1, permeability, derivative);
   EXPECT_DOUBLE_EQ(1.89736659610102775808e-01, permeability );
   EXPECT_DOUBLE_EQ(6.55327206019062219466, derivative);
}

TEST( PermeabilitySandStonePermeabilityDerivative, invalidNegativePorosity)
{
   double permeability = NaN, derivative = NaN;
   PermeabilitySandStone( 0.50, 6000, 1.5).calculateDerivative( 0, 0, -5, 1, permeability, derivative);
   EXPECT_NEAR(1.89736659610102742578e-79, permeability, 1e-92 );
   EXPECT_NEAR(6.55327206019062006483e-78, derivative, 1e-90);
}

TEST( PermeabilitySandStonePermeabilityDerivative, invalidZeroPorosity)
{
   double permeability = NaN, derivative = NaN;
   PermeabilitySandStone( 0.60, 6000, 1.5).calculateDerivative( 1.0e+5, 1.0e+5, 0.0, 1, permeability, derivative);
   EXPECT_NEAR(6.00000000000000015201e-06, permeability, 5e-21 );
   EXPECT_NEAR(2.07232658369464115328e-04, derivative, 1e-18 );
}

// The normal cases
TEST( PermeabilitySandStonePermeabilityDerivative, lowVes)
{
   double permeability = NaN, derivative = NaN;
   PermeabilitySandStone( 0.39, 6000, 1.5).calculateDerivative( 1.0e+5, 1.0e+5, 0, 1, permeability, derivative);
   EXPECT_NEAR(8.47522526773653202270e-03, permeability, 2e-17 );
   EXPECT_NEAR(2.92723910418849148432e-01 , derivative, 1e-13 );
}

TEST( PermeabilitySandStonePermeabilityDerivative, highVes)
{
   double permeability = NaN, derivative = NaN;
   PermeabilitySandStone( 0.48, 6000, 1.5).calculateDerivative( 1.0e+6, 1.0e+6, 0.2, 1, permeability, derivative);
   EXPECT_NEAR(3.78574406688116593678e-01, permeability, 5e-15 );
   EXPECT_NEAR(1.30754967814368405765e+01, derivative,   1e-13 );
}

TEST( PermeabilitySandStonePermeabilityDerivative, higherMaxVes)
{
   double permeability = NaN, derivative = NaN;
   PermeabilitySandStone( 0.60, 6000, 1.5).calculateDerivative( 1.0e+6, 2.0e+6, 0.3, 1, permeability, derivative);
   EXPECT_DOUBLE_EQ(1.89736659610102775808e-01, permeability );
   EXPECT_DOUBLE_EQ(6.55327206019062219466, derivative );
}

TEST( PermeabilitySandStonePermeabilityDerivative, highPermeability)
{
   double permeability = NaN, derivative = NaN;
   PermeabilitySandStone( 0.60, 6000, 1.5).calculateDerivative( 1.0e+6, 2.0e+6, 0.6, 1, permeability, derivative);
   EXPECT_DOUBLE_EQ(1000.0, permeability );
   EXPECT_DOUBLE_EQ(0.0, derivative );
}

TEST( PermeabilitySandStone, depoPerm )
{
   //Input domain:
   // depoPermeability = any value
   // model = any value
   using namespace DataAccess::Interface;
   EXPECT_EQ( 6000, PermeabilitySandStone( 0.60, 6000, 1.5).depoPerm() );
}

TEST( PermeabilitySandStone, model )
{
   //Input domain:
   // depoPermeability = any value
   // model = any value
   using namespace DataAccess::Interface;
   EXPECT_EQ( SANDSTONE_PERMEABILITY, PermeabilitySandStone( 0.50, 6000, 1.5).model());
}



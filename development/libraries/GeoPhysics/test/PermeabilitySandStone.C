#include "../src/PermeabilitySandStone.h"

#include <iostream>
#include <iomanip>
#include <cstring>
#include <cassert>
#include <cmath>

#include <gtest/gtest.h>

using namespace GeoPhysics;


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
   EXPECT_NEAR(8.47522526773653202270e-03, PermeabilitySandStone( 0.39, 6000, 1.5).permeability( -1e+6, 1.0e+5, 0),  2e-17);

   EXPECT_DOUBLE_EQ(1.34323268314100546794e-01, PermeabilitySandStone( 0.41, 6000, 1.5).permeability( -1e+5, 1.0e+5, 0.1) );
}

TEST( PermeabilitySandStonePermeability, invalidMaxVes )
{   // varying maxVes invalid classes
   EXPECT_DOUBLE_EQ(1000.0 , PermeabilitySandStone( 0.48, 6000, 1.5).permeability( 0, -1e+6, 1) );

   EXPECT_DOUBLE_EQ(1.89736659610102758244e-04, PermeabilitySandStone( 0.50, 6000, 1.5).permeability( 1e+4, 0, 0) );

   EXPECT_DOUBLE_EQ(1.89736659610102775808e-01, PermeabilitySandStone( 0.60, 6000, 1.5).permeability( 1e+6, 1e+3, 0.3) );
}

TEST( PermeabilitySandStonePermeability, invalidCalculatedPorosity )
{  // varying calculatedPorosity invalid classes
   EXPECT_NEAR(1.89736659610102764348e-82, PermeabilitySandStone( 0.70, 6000, 1.5).permeability( 0,0, -5), 2e-96 );

   EXPECT_DOUBLE_EQ(1000.0, PermeabilitySandStone( 0.35, 6000, 1.5).permeability( 1e+4, 1e+5, 5) );
}

TEST( PermeabilitySandStonePermeability, validCases )
{
   // The normal cases
   EXPECT_NEAR(8.47522526773653202270e-03 , PermeabilitySandStone( 0.39, 6000, 1.5).permeability( 1.0e+5, 1.0e+5, 0.0), 2e-17 );

   EXPECT_NEAR(3.78574406688116593678e-01, PermeabilitySandStone( 0.48, 6000, 1.5).permeability( 1.0e+6, 1.0e+6, 0.2), 3e-16 );

   EXPECT_DOUBLE_EQ(1.89736659610102775808e-01, PermeabilitySandStone( 0.60, 6000, 1.5).permeability( 1.0e+6, 2.0e+6, 0.3) );

   EXPECT_DOUBLE_EQ(1000.0, PermeabilitySandStone( 0.60, 6000, 1.5).permeability( 1.0e+6, 2.0e+6, 0.6) );
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
   double permeability = NAN, derivative = NAN;
   PermeabilitySandStone( 0.39, 6000, 1.5).permeabilityDerivative( -1e+6, 1.0e+5, 0, permeability, derivative);
   EXPECT_NEAR(8.47522526773653202270e-03, permeability, 2e-17);
   EXPECT_NEAR(2.92723910418849148432e-03, derivative, 6e-18 );
}

TEST( PermeabilitySandStonePermeabilityDerivative, invalidSmallNegativeVes)
{
   double permeability = NAN, derivative = NAN;

   PermeabilitySandStone( 0.41, 6000, 1.5).permeabilityDerivative( -1e+5, 1.0e+5, 0.1, permeability, derivative);
   EXPECT_DOUBLE_EQ(1.34323268314100546794e-01, permeability);
   EXPECT_DOUBLE_EQ(4.63936132893431052437e-02, derivative);
}

TEST( PermeabilitySandStonePermeabilityDerivative, invalidLargeNegativeMaxVes)
{
   double permeability = NAN, derivative = NAN;
   PermeabilitySandStone( 0.48, 6000, 1.5).permeabilityDerivative( 0, -1e+6, 1, permeability, derivative);
   EXPECT_DOUBLE_EQ(1000.0 , permeability );
   EXPECT_NEAR(1.30754967814368118286e+11, derivative, 1e-4);
}

TEST( PermeabilitySandStonePermeabilityDerivative, invalidSmallerwMaxVes)
{
   double permeability = NAN, derivative = NAN;
   PermeabilitySandStone( 0.50, 6000, 1.5).permeabilityDerivative( 1e+4, 0, 0, permeability, derivative);
   EXPECT_DOUBLE_EQ(1.89736659610102758244e-04, permeability );
   EXPECT_DOUBLE_EQ(6.55327206019062116467e-05, derivative);
}

TEST( PermeabilitySandStonePermeabilityDerivative, invalidNormalVesSmallerMaxVes)
{
   double permeability = NAN, derivative = NAN;
   PermeabilitySandStone( 0.60, 6000, 1.5).permeabilityDerivative( 1e+6, 1e+3, 0.3, permeability, derivative);
   EXPECT_DOUBLE_EQ(1.89736659610102775808e-01, permeability );
   EXPECT_DOUBLE_EQ(6.55327206019062219466e-02, derivative);
}

TEST( PermeabilitySandStonePermeabilityDerivative, invalidNegativePorosity)
{
   double permeability = NAN, derivative = NAN;
   PermeabilitySandStone( 0.50, 6000, 1.5).permeabilityDerivative( 0, 0, -5, permeability, derivative);
   EXPECT_NEAR(1.89736659610102742578e-79, permeability, 2e-93 );
   EXPECT_NEAR(6.55327206019062006483e-80, derivative, 5e-94);
}

TEST( PermeabilitySandStonePermeabilityDerivative, invalidZeroPorosity)
{
   double permeability = NAN, derivative = NAN;
   PermeabilitySandStone( 0.60, 6000, 1.5).permeabilityDerivative( 1.0e+5, 1.0e+5, 0.0, permeability, derivative);
   EXPECT_NEAR(6.00000000000000015201e-06, permeability, 5e-21 );
   EXPECT_DOUBLE_EQ(2.07232658369464115328e-06, derivative);
}

// The normal cases
TEST( PermeabilitySandStonePermeabilityDerivative, lowVes)
{
   double permeability = NAN, derivative = NAN;
   PermeabilitySandStone( 0.39, 6000, 1.5).permeabilityDerivative( 1.0e+5, 1.0e+5, 0, permeability, derivative);
   EXPECT_NEAR(8.47522526773653202270e-03, permeability, 2e-17 );
   EXPECT_NEAR(2.92723910418849148432e-03 , derivative, 1e-17 );
}

TEST( PermeabilitySandStonePermeabilityDerivative, highVes)
{
   double permeability = NAN, derivative = NAN;
   PermeabilitySandStone( 0.48, 6000, 1.5).permeabilityDerivative( 1.0e+6, 1.0e+6, 0.2, permeability, derivative);
   EXPECT_NEAR(3.78574406688116593678e-01, permeability, 5e-16 );
   EXPECT_DOUBLE_EQ(1.30754967814368405765e-01 , derivative );
}

TEST( PermeabilitySandStonePermeabilityDerivative, higherMaxVes)
{
   double permeability = NAN, derivative = NAN;
   PermeabilitySandStone( 0.60, 6000, 1.5).permeabilityDerivative( 1.0e+6, 2.0e+6, 0.3, permeability, derivative);
   EXPECT_DOUBLE_EQ(1.89736659610102775808e-01, permeability );
   EXPECT_DOUBLE_EQ(6.55327206019062219466e-02 , derivative );
}

TEST( PermeabilitySandStonePermeabilityDerivative, highPermeability)
{
   double permeability = NAN, derivative = NAN;
   PermeabilitySandStone( 0.60, 6000, 1.5).permeabilityDerivative( 1.0e+6, 2.0e+6, 0.6, permeability, derivative);
   EXPECT_DOUBLE_EQ(1000.0, permeability );
   EXPECT_DOUBLE_EQ(2.07232658369464115822e+03 , derivative );
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



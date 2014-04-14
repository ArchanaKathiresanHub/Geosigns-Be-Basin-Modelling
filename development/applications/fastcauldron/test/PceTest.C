#include <iostream>
#include <cmath>
#include <cassert>
#include <cstdlib>

#include "BrooksCorey.h"

#include <gtest/gtest.h>


static const double DefaultC1 = 0.33;
static const double DefaultC2 = pow ( 10.0, -0.2611 );

const double relativeError = 1.0e-10; 

TEST( PceTest, CalculationOfPceValues10EMin8 )
{
   double permeability = 1.0e-8;
   double pce = BrooksCorey::computeCapillaryEntryPressure ( permeability, DefaultC1, DefaultC2 );
   double expectance = 239276473.852775;
   EXPECT_NEAR( pce, expectance, relativeError * expectance );
}

TEST( PceTest, CalculationOfPceValues10EMin6 )
{
   double permeability = 1.0e-6;
   double pce = BrooksCorey::computeCapillaryEntryPressure ( permeability, DefaultC1, DefaultC2 );
   double expectance = 52347988.7009069;
   EXPECT_NEAR( pce, expectance, relativeError * expectance );
}

TEST( PceTest, CalculationOfPceValues10EMin4 )
{
   double permeability = 1.0e-4;
   double pce = BrooksCorey::computeCapillaryEntryPressure ( permeability, DefaultC1, DefaultC2 );
   double expectance = 11452492.0770789;
   EXPECT_NEAR( pce, expectance, relativeError * expectance );
}

TEST( PceTest, CalculationOfPceValues10EMin2 )
{
   double permeability = 1.0e-2;
   double pce = BrooksCorey::computeCapillaryEntryPressure ( permeability, DefaultC1, DefaultC2 );
   double expectance = 2505532.26648195;
   EXPECT_NEAR( pce, expectance, relativeError * expectance );
}

TEST( PceTest, CalculationOfPceValues1 )
{
   double permeability = 1.0;
   double pce = BrooksCorey::computeCapillaryEntryPressure ( permeability, DefaultC1, DefaultC2 );
   double expectance = 548150.734017655;
   EXPECT_NEAR( pce, expectance, relativeError * expectance );
}

TEST( PceTest, CalculationOfPceValues100 )
{
   double permeability = 1.0e2;
   double pce = BrooksCorey::computeCapillaryEntryPressure ( permeability, DefaultC1, DefaultC2 );
   double expectance = 119922.314002360;
   EXPECT_NEAR( pce, expectance, relativeError * expectance );
}



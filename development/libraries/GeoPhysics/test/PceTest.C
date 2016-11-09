//
// Copyright (C) 2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include <iostream>
#include <cmath>
#include <cassert>
#include <cstdlib>

#include "../src/BrooksCorey.h"

#include <gtest/gtest.h>


static const double DefaultC1 = 0.33;
static const double DefaultC2 = pow ( 10.0, -0.2611 );

const double relativeError = 1.0e-10; 

TEST( PceTest, CalculationOfPceValues10EMin8 )
{
   double permeability = 1.0e-8;
   GeoPhysics::BrooksCorey brooksCorey;
   double pce = brooksCorey.computeCapillaryEntryPressure ( permeability, DefaultC1, DefaultC2 );
   double expectance = 239276473.852775;
   EXPECT_NEAR( pce, expectance, relativeError * expectance );
}

TEST( PceTest, CalculationOfPceValues10EMin6 )
{
   double permeability = 1.0e-6;
   GeoPhysics::BrooksCorey brooksCorey;
   double pce = brooksCorey.computeCapillaryEntryPressure ( permeability, DefaultC1, DefaultC2 );
   double expectance = 52347988.7009069;
   EXPECT_NEAR( pce, expectance, relativeError * expectance );
}

TEST( PceTest, CalculationOfPceValues10EMin4 )
{
   double permeability = 1.0e-4;
   GeoPhysics::BrooksCorey brooksCorey;
   double pce = brooksCorey.computeCapillaryEntryPressure ( permeability, DefaultC1, DefaultC2 );
   double expectance = 11452492.0770789;
   EXPECT_NEAR( pce, expectance, relativeError * expectance );
}

TEST( PceTest, CalculationOfPceValues10EMin2 )
{
   double permeability = 1.0e-2;
   GeoPhysics::BrooksCorey brooksCorey;
   double pce = brooksCorey.computeCapillaryEntryPressure ( permeability, DefaultC1, DefaultC2 );
   double expectance = 2505532.26648195;
   EXPECT_NEAR( pce, expectance, relativeError * expectance );
}

TEST( PceTest, CalculationOfPceValues1 )
{
   double permeability = 1.0;
   GeoPhysics::BrooksCorey brooksCorey;
   double pce = brooksCorey.computeCapillaryEntryPressure ( permeability, DefaultC1, DefaultC2 );
   double expectance = 548150.734017655;
   EXPECT_NEAR( pce, expectance, relativeError * expectance );
}

TEST( PceTest, CalculationOfPceValues100 )
{
   double permeability = 1.0e2;
   GeoPhysics::BrooksCorey brooksCorey;
   double pce = brooksCorey.computeCapillaryEntryPressure ( permeability, DefaultC1, DefaultC2 );
   double expectance = 119922.314002360;
   EXPECT_NEAR( pce, expectance, relativeError * expectance );
}



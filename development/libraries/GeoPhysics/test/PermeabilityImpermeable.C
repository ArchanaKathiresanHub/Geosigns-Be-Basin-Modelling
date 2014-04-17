#include "../src/PermeabilityImpermeable.h"

#include <iostream>
#include <cstring>
#include <cassert>
#include <cmath>
#include <limits>

#include <gtest/gtest.h>

using namespace GeoPhysics;

static const double nan = std::numeric_limits<double>::quiet_NaN();

class PermeabilityImpermeableTest : public ::testing::Test
{
public:
   PermeabilityImpermeableTest()
      : m_p(0.0, DataAccess::Interface::IMPERMEABLE_PERMEABILITY)
   {}

   PermeabilityImpermeable m_p;
};

TEST_F( PermeabilityImpermeableTest, permeability )
{
   //Input domain:
   // ves >= 1.0e+5
   // maxVes >= ves
   // 0 <= calculatedPorosity < 1

   EXPECT_FLOAT_EQ( 1.0e-9, m_p.permeability( 1.0e+5, 1.0e+5, 0) );
   EXPECT_FLOAT_EQ( 1.0e-9, m_p.permeability( 1.0e+6, 1.0e+6, 0.2) );
   EXPECT_FLOAT_EQ( 1.0e-9, m_p.permeability( 1.0e+6, 2.0e+6, 0.3) );
}

  //Input domain:
   // ves >= 1.0e+5
   // maxVes >= ves
   // 0 <= calculatedPorosity < 1

TEST_F( PermeabilityImpermeableTest, permeabilityDerivativeAtmosphere )
{
   double permeability = nan, derivative = nan;
   m_p.permeabilityDerivative( 1.0e+5, 1.0e+5, 0, 0, permeability, derivative);
   EXPECT_FLOAT_EQ( 1e-9,  permeability );
   EXPECT_FLOAT_EQ( 0.0,  derivative );
}

TEST_F( PermeabilityImpermeableTest, permeabilityDerivativeUnderground )
{ 
   double permeability = nan, derivative = nan;
   m_p.permeabilityDerivative( 1.0e+6, 1.0e+6, 0.2, 0, permeability, derivative);
   EXPECT_FLOAT_EQ( 1e-9,  permeability );
   EXPECT_FLOAT_EQ( 0.0,  derivative );
}

TEST_F( PermeabilityImpermeableTest, permeabilityDerivativeUndergroundAndHigherMaxVes)
{  double permeability = nan, derivative = nan;
   m_p.permeabilityDerivative( 1.0e+6, 2.0e+6, 0.3, 0, permeability, derivative);
   EXPECT_FLOAT_EQ( 1e-9,  permeability );
   EXPECT_FLOAT_EQ( 0.0,  derivative );
}

TEST( PermeabilityImpermeable, depoPerm )
{
   //Input domain:
   // depoPermeability = any value
   // model = any value
   using namespace DataAccess::Interface;
   EXPECT_FLOAT_EQ( 0.0,  PermeabilityImpermeable(0.0, IMPERMEABLE_PERMEABILITY).depoPerm() );
   EXPECT_FLOAT_EQ( 5.0,  PermeabilityImpermeable(5.0, NONE_PERMEABILITY).depoPerm() );
}

TEST( PermeabilityImpermeable, model )
{
   //Input domain:
   // depoPermeability = any value
   // model = any value
   using namespace DataAccess::Interface;
   EXPECT_EQ( IMPERMEABLE_PERMEABILITY ,  PermeabilityImpermeable(0.0, IMPERMEABLE_PERMEABILITY).model() );
   EXPECT_EQ( NONE_PERMEABILITY,  PermeabilityImpermeable(5.0, NONE_PERMEABILITY).model() );
}


#include "../src/VitriniteReflectance.h"
#include "../src/VreArrheniusMethod.h"
#include "../src/VreTtiMethod.h"
#include "InputGrid.h"
#include "OutputGrid.h"

#include <iostream>
#include <cstring>
#include <cmath>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace GeoPhysics;

double quickTTI( double time1, double time2, double temp1, double temp2, double methodsF )
{
   double rt1 = std::pow( methodsF, ( temp1 - 105.0 ) / 10.0 );
   double rt2 = std::pow( methodsF, ( temp2 - 105.0 ) / 10.0 );

   double changeRate = rt1 - rt2;

   if ( std::abs( changeRate ) < 1.0e-3 )
      return rt1 * ( time1 - time2 );
   else
      return ( rt2 - rt1 ) / ( temp2 - temp1 ) / std::log( methodsF ) * ( time1 - time2 ) * 10.0;
}


/// Value-parametrized tests follow.

/// Struct containing the parameters that define the value of VR in 1 timestep
struct ParameterSet
{
   double time1, time2;
   double temperature1, temperature2;
};

/** Fixture class (empty), necessary to relate the type (ParameterSet)
    with the GetParam() function of gtest */
class VreTtiMethodTest : public ::testing::TestWithParam< ParameterSet >
{

};

/// Test to be repeated for a number of different parameter sets
TEST_P(  VreTtiMethodTest, OneTimeStepModLopatin )
{
   double time1 = GetParam().time1;
   double time2 = GetParam().time2;
   double temp1 = GetParam().temperature1;
   double temp2 = GetParam().temperature2;
   int gridSize = 10;

   InputGrid PreviousInput( time1, temp1, gridSize );
   InputGrid CurrentInput( time2, temp2, gridSize );

   OutputGrid CurrentOutput( gridSize );

   VreTtiMethod * methodObject1 = dynamic_cast<VreTtiMethod*>(VitriniteReflectance::create("Modified Lopatin"));
   
   methodObject1->doTimestep( PreviousInput, CurrentInput );
   methodObject1->getResults( CurrentOutput );

   double methodsF = 5.1;
   double methodsP = 0.0821;
   double methodsQ = 0.625;
   double ttiValue = quickTTI( time1, time2, temp1, temp2, methodsF );
   double s_initialVRe = 0.2;

   const double ttiBegin = std::exp( ( std::log( s_initialVRe ) + methodsQ ) / methodsP );

   double ttiSum = ttiBegin + ttiValue;
   double expected;

   if( ttiSum > 0.0 )
	  expected = std::exp( methodsP * std::log( ttiSum ) - methodsQ );
   else
	  expected = s_initialVRe;

   EXPECT_NEAR( expected, CurrentOutput.printVR(), expected * 1.0e-6 );
}

/// Test to be repeated for a number of different parameter sets
TEST_P(  VreTtiMethodTest, OneTimeStepLopatinDykstra )
{
   double time1 = GetParam().time1;
   double time2 = GetParam().time2;
   double temp1 = GetParam().temperature1;
   double temp2 = GetParam().temperature2;
   int gridSize = 10;

   InputGrid PreviousInput( time1, temp1, gridSize );
   InputGrid CurrentInput( time2, temp2, gridSize );

   OutputGrid CurrentOutput( gridSize );

   VreTtiMethod * methodObject1 = dynamic_cast<VreTtiMethod*>(VitriniteReflectance::create("Lopatin & Dykstra"));
   
   methodObject1->doTimestep( PreviousInput, CurrentInput );
   methodObject1->getResults( CurrentOutput );

   double methodsF = 2.0;
   double methodsP = 0.23697;
   double methodsQ = 1.04431;
   double ttiValue = quickTTI( time1, time2, temp1, temp2, methodsF );
   double s_initialVRe = 0.2;

   const double ttiBegin = std::exp( ( std::log( s_initialVRe ) + methodsQ ) / methodsP );

   double ttiSum = ttiBegin + ttiValue;
   double expected;

   if( ttiSum > 0.0 )
	  expected = std::exp( methodsP * std::log( ttiSum ) - methodsQ );
   else
	  expected = s_initialVRe;

   EXPECT_NEAR( expected, CurrentOutput.printVR(), expected * 1.0e-6 );
}

ParameterSet oneTimeStepTestCases[] = 
{// time1  time2 temperature1 temperature2
   { 100.0, 99.0, 180.0, 180.0 },
   { 100.0, 99.999, 180.0, -180.0 },
   { 100.0, 90.0, 80.0, 100.0 },
   { 100.0, 99.999, 160.0, 159.99 },
   { 100.0, 90.0, 80.0, 180.0 },
   { 100.0, 99.999, 80.0, 180.0 },
   { 100.0, 10.0, 181.0, 180.0 },
   { 100.0, 1.0, 0.0, 300.0 },
   { 100.0, 99.99, 0.0, 2000.0 },
   { 100.0, 99.0, 4000.0, 4000.0 },
   { 100.0, 1.0, 1.0e-13, 0.0  }
};

INSTANTIATE_TEST_CASE_P( SingleTimestep,
                        VreTtiMethodTest,
                        ::testing::ValuesIn(oneTimeStepTestCases));

/** Following tests checks continuity of VR values across temperature
differences that the TTI algorithm uses to decide how the calculation
will proceed */
TEST(  VreTtiMethodTest, OneTimeStepContinuity1 )
{
   double time1 = 10;
   double time2 = 9;
   double temp1 = 0;
   double temp2a = 0.001 - 1e-12;
   double temp2b = 0.001 + 1e-12;
   int gridSize = 10;

   InputGrid PreviousInput( time1, temp1, gridSize );
   InputGrid CurrentInputA( time2, temp2a, gridSize );
   InputGrid CurrentInputB( time2, temp2b, gridSize );

   OutputGrid CurrentOutputA( gridSize );
   OutputGrid CurrentOutputB( gridSize );

   LopatinModified methodObjectA;
   LopatinModified methodObjectB;
   
   methodObjectA.doTimestep( PreviousInput, CurrentInputA );
   methodObjectB.doTimestep( PreviousInput, CurrentInputB );
   methodObjectA.getResults( CurrentOutputA );
   methodObjectB.getResults( CurrentOutputB );

   EXPECT_NEAR( CurrentOutputA.printVR(), CurrentOutputB.printVR(), CurrentOutputA.printVR() * 1.0e-6);
}

TEST(  VreTtiMethodTest, OneTimeStepContinuity2 )
{
   double time1 = 10;
   double time2 = 9;
   double temp1 = 0;
   double temp2a = 0.001 - 1e-12;
   double temp2b = 0.001 + 1e-12;
   int gridSize = 10;

   InputGrid PreviousInput( time1, temp1, gridSize );
   InputGrid CurrentInputA( time2, temp2a, gridSize );
   InputGrid CurrentInputB( time2, temp2b, gridSize );

   OutputGrid CurrentOutputA( gridSize );
   OutputGrid CurrentOutputB( gridSize );

   LopatinWaples methodObjectA;
   LopatinWaples methodObjectB;
   
   methodObjectA.doTimestep( PreviousInput, CurrentInputA );
   methodObjectB.doTimestep( PreviousInput, CurrentInputB );
   methodObjectA.getResults( CurrentOutputA );
   methodObjectB.getResults( CurrentOutputB );

   EXPECT_NEAR( CurrentOutputA.printVR(), CurrentOutputB.printVR(), CurrentOutputA.printVR() * 1.0e-6);
}

/** Death test
    Asserts death if sizes if time1 < time2 ( => timestep < 0 )                */

#ifndef NDEBUG
TEST ( VreTtiMethod, DeathTest )
{
   ::testing::FLAGS_gtest_death_test_style="threadsafe";

   double time1 = 100.0;
   double time2 = 100.0000001;
   double temp1 = 180.0;
   double temp2 = 180.0;
   int gridSize = 10;
   
   InputGrid PreviousInput( time1, temp1, gridSize );
   InputGrid CurrentInput( time2, temp2, gridSize );

   OutputGrid CurrentOutput( gridSize );

   VreTtiMethod * methodObject9 = dynamic_cast<VreTtiMethod*>(VitriniteReflectance::create("Lopatin & Dykstra"));

   ASSERT_DEATH( methodObject9->doTimestep( PreviousInput, CurrentInput ), "Assertion.*timeStep > 0.0" );
}
#endif NDEBUG

/// First test of >1 timesteps
TEST( VreTtiMethod, timestep_test1 )
{
   double time1 = 100.0;
   double time2 = 99.0;
   double time3 = 98.0;
   double temp1 = 190.0;
   double temp2 = 180.0;
   double temp3 = 200.0;
   int gridSize = 10;

   InputGrid PreviousInput( time1, temp1, gridSize );
   InputGrid CurrentInput( time2, temp2, gridSize );
   InputGrid NextInput( time3, temp3, gridSize );

   OutputGrid CurrentOutput( gridSize );
   OutputGrid NextOutput( gridSize );

   VreTtiMethod * methodObject10 = dynamic_cast<VreTtiMethod*>(VitriniteReflectance::create("Lopatin & Dykstra"));

   methodObject10->doTimestep( PreviousInput, CurrentInput );
   methodObject10->getResults( CurrentOutput );
   methodObject10->doTimestep( CurrentInput, NextInput );
   methodObject10->getResults( NextOutput );

   double methodsF = 2.0;
   double methodsP = 0.23697;
   double methodsQ = 1.04431;
   double ttiValue1 = quickTTI( time1, time2, temp1, temp2, methodsF );
   double ttiValue2 = quickTTI( time2, time3, temp2, temp3, methodsF );
   const double ttiBegin = std::exp( ( std::log ( 0.2 ) + methodsQ ) / methodsP );

   ttiValue1 += ttiValue2;

   if ( ttiValue1 + ttiBegin > 0.0 )
	  ttiValue1 += ttiBegin;
   else
	  ttiValue1 = ttiBegin;
   
   double expected = std::exp( methodsP * std::log( ttiValue1 ) - methodsQ );

   EXPECT_NEAR( expected, NextOutput.printVR(), expected * 1.0e-6 );
}

/// First test of >1 timesteps
TEST( VreTtiMethod, timestep_test2 )
{
   double time1 = 100.0;
   double time2 = 9.0;
   double time3 = 8.0;
   double temp1 = 110.0;
   double temp2 = -180.0;
   double temp3 = 200.0;
   int gridSize = 10;

   InputGrid PreviousInput( time1, temp1, gridSize );
   InputGrid CurrentInput( time2, temp2, gridSize );
   InputGrid NextInput( time3, temp3, gridSize );

   OutputGrid CurrentOutput( gridSize );
   OutputGrid NextOutput( gridSize );

   VreTtiMethod * methodObject11 = dynamic_cast<VreTtiMethod*>(VitriniteReflectance::create("Lopatin & Dykstra"));

   methodObject11->doTimestep( PreviousInput, CurrentInput );
   methodObject11->getResults( CurrentOutput );
   methodObject11->doTimestep( CurrentInput, NextInput );
   methodObject11->getResults( NextOutput );

   double methodsF = 2.0;
   double methodsP = 0.23697;
   double methodsQ = 1.04431;
   double ttiValue1 = quickTTI( time1, time2, temp1, temp2, methodsF );
   double ttiValue2 = quickTTI( time2, time3, temp2, temp3, methodsF );
   const double ttiBegin = std::exp( ( std::log ( 0.2 ) + methodsQ ) / methodsP );

   ttiValue1 += ttiValue2;

   if ( ttiValue1 + ttiBegin > 0.0 )
	  ttiValue1 += ttiBegin;
   else
	  ttiValue1 = ttiBegin;
   
   double expected = std::exp( methodsP * std::log( ttiValue1 ) - methodsQ );

   EXPECT_NEAR( expected, NextOutput.printVR(), expected * 1.0e-6 );
}

/** In this test case we check the behaviour of the VreArrheniusMethod class.
    Various methods are checked including the function convertFractionToVR( double )
    from its derived classes SweeneyBurnham and Larter                                    */

#include "../src/VitriniteReflectance.h"
#include "../src/VreArrheniusMethod.h"
#include "../src/VreTtiMethod.h"
#include "InputGrid.h"
#include "OutputGrid.h"

#include <iostream>
#include <cstring>
#include <cmath>

#include "NumericFunctions.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace GeoPhysics;

double quickCalculationSweeney( double time1, double time2, double temp1, double temp2 );
double justDeltaI( double time1, double time2, double temp1, double temp2, int i );
double VreFromDeltas( double * deltaI );


/** Auxilliary classes for accessing the protected functions convertFractionToVR(double)
    for objects of both SweeneyBurnham and Larter Classes                                 */
class ConvertFractionToVR_Sweeney : public SweeneyBurnham
{
public:
   double compute(double x) const { return this->convertFractionToVR(x); }
};

class ConvertFractionToVR_Larter : public Larter
{
public:
   double compute(double x) const { return this->convertFractionToVR(x); }
};

/** The following two tests check that VRe is calculated correctly for a range of 
    input values (fConv in the source file) for which the results have been manually
    pre-calculated. Checks are also performed to ensure that a VR value greater than
    R_0 will always be returned, even for negative fConv                                  */
TEST( VreArrheniusMethod, testing_conversion_to_VR_Sweeney )
{   
   ConvertFractionToVR_Sweeney conversionCheck;

   EXPECT_GE( 0.2, conversionCheck.compute( -100.0 ) );
   EXPECT_GE( 0.2, conversionCheck.compute( -10.0 ) );
   EXPECT_GE( 0.2, conversionCheck.compute( -1.0 ) );

   EXPECT_NEAR( 0.200000, conversionCheck.compute( 0.0 ), 0.200000 * 1.0e-6 );
   EXPECT_NEAR( 0.419187, conversionCheck.compute( 0.2 ), 0.419187 * 1.0e-6 );
   EXPECT_NEAR( 0.878589, conversionCheck.compute( 0.4 ), 0.878589 * 1.0e-6 );
   EXPECT_NEAR( 1.841466, conversionCheck.compute( 0.6 ), 1.841466 * 1.0e-6 );
   EXPECT_NEAR( 3.859594, conversionCheck.compute( 0.8 ), 3.859594 * 1.0e-6 );
   EXPECT_NEAR( 8.089461, conversionCheck.compute( 1.0 ), 8.089461 * 1.0e-6 );
};

TEST( VreArrheniusMethod, testing_conversion_to_VR_Larter )
{   
   ConvertFractionToVR_Larter conversionCheck;

   EXPECT_GE( 0.2, conversionCheck.compute( -100.0 ) );
   EXPECT_GE( 0.2, conversionCheck.compute( -10.0 ) );
   EXPECT_GE( 0.2, conversionCheck.compute( -1.0 ) );

   EXPECT_NEAR( 0.450000, conversionCheck.compute( 0.0 ), 0.450000 * 1.0e-6 );
   EXPECT_NEAR( 0.676000, conversionCheck.compute( 0.2 ), 0.676000 * 1.0e-6 );
   EXPECT_NEAR( 0.902000, conversionCheck.compute( 0.4 ), 0.902000 * 1.0e-6 );
   EXPECT_NEAR( 1.128000, conversionCheck.compute( 0.6 ), 1.128000 * 1.0e-6 );
   EXPECT_NEAR( 1.354000, conversionCheck.compute( 0.8 ), 1.354000 * 1.0e-6 );
   EXPECT_NEAR( 1.580000, conversionCheck.compute( 1.0 ), 1.580000 * 1.0e-6 );
};


/** In what follows we check the doTimestep() and getResults() methods of
    the VitriniteReflectance Class specifically for the SweeneyBurnham subclass  */


/** Three auxilliary functions - quickCalculationSweeney(), justDeltaI() and VreFromDeltas() -
    are being used to calculate independently the final value of VRe and intermediate quantities */
double quickCalculationSweeney( double time1, double time2, double temp1, double temp2 )
{
   double fe[] =          /** stoichiometric factors */
      { 
         0.03, 0.03, 0.04, 0.04, 0.05, 
         0.05, 0.06, 0.04, 0.04, 0.07, 
         0.06, 0.06, 0.06, 0.05, 0.05, 
         0.04, 0.03, 0.02, 0.02, 0.01 
      };
 
   double en[]   =        /** activation energies */
      { 
         34000.0, 36000.0, 38000.0, 40000.0, 42000.0, 
         44000.0, 46000.0, 48000.0, 50000.0, 52000.0, 
         54000.0, 56000.0, 58000.0, 60000.0, 62000.0, 
         64000.0, 66000.0, 68000.0, 70000.0, 72000.0 
      }; 

   double bigA = 1.0e13;
   const double a1 = 2.334733;
   const double a2 = 0.250621;
   const double b1 = 3.330657;
   const double b2 = 1.681534;

   temp1 = std::max(1.0, temp1 + 273.15);
   temp2 = std::max(1.0, temp2 + 273.15);
   
   if ( std::abs(temp1-temp2) < 0.001 )
   {
      if ( temp2 > temp1 ) {
         temp2 = temp1 + 0.001;
      } else {
         temp1 += 0.001;
      }
   }

   double timeStep = ( time1 - time2 ) * 3.15576e13; // conversion to seconds

   const double gasConstant = 1.987; /// gas constant (also known as R)

   double theFraction = 0.0;

   double eOverRT1[20], eOverRT2[20];
   double deltaIij[20];

   for( int n = 0; n < 20; ++n )
   {
      eOverRT1[n] = en[n] / ( gasConstant * temp1 );
      eOverRT2[n] = en[n] / ( gasConstant * temp2 );

      deltaIij[n] = timeStep / ( temp2 - temp1 ) *
         ( temp2 * bigA * std::exp( - eOverRT2[n] )
           * ( 1.0 - ( std::pow( eOverRT2[n], 2.0 ) + eOverRT2[n] * a1 + a2 ) / ( std::pow( eOverRT2[n], 2.0 ) + eOverRT2[n] * b1 + b2 ) )
           - temp1 * bigA * std::exp( - eOverRT1[n] ) * ( 1.0 - ( std::pow( eOverRT1[n], 2.0 ) + eOverRT1[n] * a1 + a2 )
                                                          / ( std::pow( eOverRT1[n], 2.0 ) + eOverRT1[n] * b1 + b2 ) ) );

      theFraction += fe[n] * ( 1.0 - std::exp( - deltaIij[n] ) );
   }

   return std::exp( std::log( 0.2 ) + theFraction * 3.7 );
}

double justDeltaI( double time1, double time2, double temp1, double temp2, int n )
{
   double fe[] =          /** stoichiometric factors */
      { 
         0.03, 0.03, 0.04, 0.04, 0.05, 
         0.05, 0.06, 0.04, 0.04, 0.07, 
         0.06, 0.06, 0.06, 0.05, 0.05, 
         0.04, 0.03, 0.02, 0.02, 0.01 
      };
 
   double en[]   =        /** activation energies */
      { 
         34000.0, 36000.0, 38000.0, 40000.0, 42000.0, 
         44000.0, 46000.0, 48000.0, 50000.0, 52000.0, 
         54000.0, 56000.0, 58000.0, 60000.0, 62000.0, 
         64000.0, 66000.0, 68000.0, 70000.0, 72000.0 
      }; 

   double bigA = 1.0e13;
   const double a1 = 2.334733;
   const double a2 = 0.250621;
   const double b1 = 3.330657;
   const double b2 = 1.681534;

   temp1 = std::max(1.0, temp1 + 273.15);
   temp2 = std::max(1.0, temp2 + 273.15);
   
   if ( std::abs(temp1-temp2) < 0.001 )
   {
      if ( temp2 > temp1 )
         temp2 += 0.001;
      else
         temp1 += 0.001;
   }

   double timeStep = ( time1 - time2 ) * 3.15576e13; // conversion to seconds

   const double gasConstant = 1.987; /// gas constant (also known as R)

   double theFraction = 0.0;

   double eOverRT1[20], eOverRT2[20];
   double deltaIij[20];

   eOverRT1[n] = en[n] / ( gasConstant * temp1 );
   eOverRT2[n] = en[n] / ( gasConstant * temp2 );

   deltaIij[n] = timeStep / ( temp2 - temp1 ) *
      ( temp2 * bigA * std::exp( - eOverRT2[n] ) * 
        ( 1.0 - ( std::pow( eOverRT2[n], 2.0 ) + eOverRT2[n] * a1 + a2 ) / ( std::pow( eOverRT2[n], 2.0 ) + eOverRT2[n] * b1 + b2 ) ) -
        temp1 * bigA * std::exp( - eOverRT1[n] )
        * ( 1.0 - ( std::pow( eOverRT1[n], 2.0 ) + eOverRT1[n] * a1 + a2 ) / ( std::pow( eOverRT1[n], 2.0 ) + eOverRT1[n] * b1 + b2 ) ) );

   return deltaIij[n];
}

double VreFromDeltas( double * deltaI )
{
   double fe[] =   /** stoichiometric factors */
      { 
         0.03, 0.03, 0.04, 0.04, 0.05, 
         0.05, 0.06, 0.04, 0.04, 0.07, 
         0.06, 0.06, 0.06, 0.05, 0.05, 
         0.04, 0.03, 0.02, 0.02, 0.01 
      };

   double theFraction = 0.0;
   for( int n = 0; n < 20; ++n )
   {
      theFraction += fe[n] * ( 1.0 - std::exp( - deltaI[n] ) );
   }

   return std::exp( std::log( 0.2 ) + theFraction * 3.7 );
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
class VreArrheniusMethodTest : public ::testing::TestWithParam< ParameterSet >
{

};

/// Test to be repeated for a number of different parameter sets
TEST_P(  VreArrheniusMethodTest, OneTimeStep )
{
   double time1 = GetParam().time1;
   double time2 = GetParam().time2;
   double temp1 = GetParam().temperature1;
   double temp2 = GetParam().temperature2;
   int gridSize = 10;

   InputGrid PreviousInput( time1, temp1, gridSize );
   InputGrid CurrentInput( time2, temp2, gridSize );

   OutputGrid CurrentOutput( gridSize );

   SweeneyBurnham methodObject1;
   
   methodObject1.doTimestep( PreviousInput, CurrentInput );
   methodObject1.getResults( CurrentOutput );

   double expected = quickCalculationSweeney( time1, time2, temp1, temp2 );
   EXPECT_NEAR( expected, CurrentOutput.printVR(), expected * 1.0e-5);
}

ParameterSet oneTimeStepTestCases[] = 
   {// time1  time2 temperature1 temperature2
      { 100.0, 99.0, 150.0, 150.0 },
      { 100.0, 98.0, 200.0, 220.0 },
      { 100.0, 99.99, 200.0, 100.0 },
      { 100.0, 50.0, 100.0, 100.01 },
      { 100.0, 99.9999, -200.0, -200.0 },
      { 100.0, 1.0, 0.0, 300.0 },
      { 100.0, 99.99, 0.0, 2000.0 },
      { 100.0, 1.0, 1.0e-13, 0.0 },
      { 100.0, 99.0, 4000.0, 4000.0 },
      { 3000.0, 2999.999999, -500.0, -300.0 },
      { 0.001000001, 0.001, 10000.0, -10000.0 },
      { 1.0, 0.99999999999999, 1e10, 1e10 }
   };

INSTANTIATE_TEST_SUITE_P( SingleTimestep,
                          VreArrheniusMethodTest,
                          ::testing::ValuesIn(oneTimeStepTestCases));

/** Following tests checks continuity of VR values across temperature
    differences that the Arrhenius algorithms use to decide how the calculation
    will proceed */
TEST(  VreArrheniusMethodTest, OneTimeStepContinuity1 )
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

   SweeneyBurnham methodObjectA;
   SweeneyBurnham methodObjectB;
   
   methodObjectA.doTimestep( PreviousInput, CurrentInputA );
   methodObjectB.doTimestep( PreviousInput, CurrentInputB );
   methodObjectA.getResults( CurrentOutputA );
   methodObjectB.getResults( CurrentOutputB );

   EXPECT_NEAR( CurrentOutputA.printVR(), CurrentOutputB.printVR(), CurrentOutputA.printVR() * 1.0e-6);
}

TEST(  VreArrheniusMethodTest, OneTimeStepContinuity2 )
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

   Larter methodObjectA;
   Larter methodObjectB;
   
   methodObjectA.doTimestep( PreviousInput, CurrentInputA );
   methodObjectB.doTimestep( PreviousInput, CurrentInputB );
   methodObjectA.getResults( CurrentOutputA );
   methodObjectB.getResults( CurrentOutputB );

   EXPECT_NEAR( CurrentOutputA.printVR(), CurrentOutputB.printVR(), CurrentOutputA.printVR() * 1.0e-6);
}

/** Death test
    Asserts death if time1 < time2 ( => timestep < 0 )               */

#ifndef NDEBUG
TEST( VreArrheniusMethod, DeathTest )
{
   ::testing::FLAGS_gtest_death_test_style="threadsafe";

   double time1 = 100.0;
   double time2 = 1.0;
   double temp1 = 0.0;
   double temp2 = -300.0;
   int gridSize = 10;
   
   InputGrid PreviousInput( time2, temp1, gridSize );
   InputGrid CurrentInput( time1, temp2, gridSize );

   OutputGrid CurrentOutput( gridSize );

   SweeneyBurnham methodObject8;
   
   ASSERT_DEATH( methodObject8.doTimestep( PreviousInput, CurrentInput ), "Assertion.*timeStep > 0.0" );
}
#endif /// NDEBUG

/// First test of >1 timesteps
TEST( VreArrheniusMethod, timestep_test1 )
{
   double time1 = 100.0;
   double time2 = 99.0;
   double time3 = 98.0;
   double temp1 = 190.0;
   double temp2 = 180.0;
   double temp3 = 200.0;
   int gridSize = 10;

   double tempDelta1[20];
   double tempDelta2[20];

   for( int n = 0; n < 20; ++n )
   {
      tempDelta1[n] = justDeltaI( time1, time2, temp1, temp2, n );
      tempDelta2[n] = justDeltaI( time2, time3, temp2, temp3, n );
   }

   for( int n = 0; n < 20; ++n )
   {
      tempDelta1[n] += tempDelta2[n];
   }
   
   InputGrid PreviousInput( time1, temp1, gridSize );
   InputGrid CurrentInput( time2, temp2, gridSize );
   InputGrid NextInput( time3, temp3, gridSize );

   OutputGrid CurrentOutput( gridSize );
   OutputGrid NextOutput( gridSize );

   SweeneyBurnham methodObject9;
   
   methodObject9.doTimestep( PreviousInput, CurrentInput );
   methodObject9.getResults( CurrentOutput );
   methodObject9.doTimestep( CurrentInput, NextInput );
   methodObject9.getResults( NextOutput );

   double expected = VreFromDeltas( tempDelta1 );
   EXPECT_NEAR( expected, NextOutput.printVR(), expected * 1.0e-6);
}

/// Second test of >1 timesteps (more extreme parameters)
TEST( VreArrheniusMethod, timestep_test2 )
{
   double time1 = 100.0;
   double time2 = 9.0;
   double time3 = 8.0;
   double temp1 = 190.0;
   double temp2 = -180.0;
   double temp3 = 600.0;
   int gridSize = 10;

   double tempDelta1[20];
   double tempDelta2[20];

   for( int n = 0; n < 20; ++n )
   {
      tempDelta1[n] = justDeltaI( time1, time2, temp1, temp2, n );
      tempDelta2[n] = justDeltaI( time2, time3, temp2, temp3, n );
   }

   for( int n = 0; n < 20; ++n )
   {
      tempDelta1[n] += tempDelta2[n];
   }
   
   InputGrid PreviousInput( time1, temp1, gridSize );
   InputGrid CurrentInput( time2, temp2, gridSize );
   InputGrid NextInput( time3, temp3, gridSize );

   OutputGrid CurrentOutput( gridSize );
   OutputGrid NextOutput( gridSize );

   SweeneyBurnham methodObject10;
   
   methodObject10.doTimestep( PreviousInput, CurrentInput );
   methodObject10.getResults( CurrentOutput );
   methodObject10.doTimestep( CurrentInput, NextInput );
   methodObject10.getResults( NextOutput );

   double expected = VreFromDeltas( tempDelta1 );
   EXPECT_NEAR( expected, NextOutput.printVR(), expected * 1.0e-6);
}

/// Third test of >1 timesteps (3 timesteps, mix of parameters)
TEST( VreArrheniusMethod, timestep_test3 )
{
   double time1 = 300.0;
   double time2 = 90.0;
   double time3 = 8.0;
   double time4 = 0.0;
   double temp1 = -190.0;
   double temp2 = -189.0;
   double temp3 = 800.0;
   double temp4 = 100.0;
   int gridSize = 10;

   double tempDelta1[20];
   double tempDelta2[20];
   double tempDelta3[20];

   for( int n = 0; n < 20; ++n )
   {
      tempDelta1[n] = justDeltaI( time1, time2, temp1, temp2, n );
      tempDelta2[n] = justDeltaI( time2, time3, temp2, temp3, n );
      tempDelta3[n] = justDeltaI( time3, time4, temp3, temp4, n );
   }

   for( int n = 0; n < 20; ++n )
   {
      tempDelta1[n] += ( tempDelta2[n] + tempDelta3[n] );
   }
   
   InputGrid PreviousInput( time1, temp1, gridSize );
   InputGrid CurrentInput( time2, temp2, gridSize );
   InputGrid NextInput( time3, temp3, gridSize );
   InputGrid AfterNextInput( time4, temp4, gridSize );

   OutputGrid CurrentOutput( gridSize );
   OutputGrid NextOutput( gridSize );
   OutputGrid AfterNextOutput( gridSize );

   SweeneyBurnham methodObject11;
   
   methodObject11.doTimestep( PreviousInput, CurrentInput );
   methodObject11.getResults( CurrentOutput );
   methodObject11.doTimestep( CurrentInput, NextInput );
   methodObject11.getResults( NextOutput );
   methodObject11.doTimestep( NextInput, AfterNextInput );
   methodObject11.getResults( AfterNextOutput );

   double expected = VreFromDeltas( tempDelta1 );
   EXPECT_NEAR( expected, AfterNextOutput.printVR(), expected * 1.0e-6);
}


/// Fourth test of >1 timesteps (4 timesteps, mix of parameters)
TEST( VreArrheniusMethod, timestep_test4 )
{
   double time1 = 300.0;
   double time2 = 90.0;
   double time3 = 89.9999999;
   double time4 = 10.0;
   double time5 = -10.0;
   double temp1 = -290.0;
   double temp2 = -289.9999;
   double temp3 = 80000.0;
   double temp4 = -1000000.0;
   double temp5 = 1.0;
   int gridSize = 10;

   double tempDelta1[20];
   double tempDelta2[20];
   double tempDelta3[20];
   double tempDelta4[20];

   for( int n = 0; n < 20; ++n )
   {
      tempDelta1[n] = justDeltaI( time1, time2, temp1, temp2, n );
      tempDelta2[n] = justDeltaI( time2, time3, temp2, temp3, n );
      tempDelta3[n] = justDeltaI( time3, time4, temp3, temp4, n );
      tempDelta4[n] = justDeltaI( time4, time5, temp4, temp5, n );
   }

   for( int n = 0; n < 20; ++n )
   {
      tempDelta1[n] += ( tempDelta2[n] + tempDelta3[n] + tempDelta4[n] );
   }
   
   InputGrid PreviousInput( time1, temp1, gridSize );
   InputGrid CurrentInput( time2, temp2, gridSize );
   InputGrid NextInput( time3, temp3, gridSize );
   InputGrid AfterNextInput( time4, temp4, gridSize );
   InputGrid PostAfterNextInput( time5, temp5, gridSize );

   OutputGrid CurrentOutput( gridSize );
   OutputGrid NextOutput( gridSize );
   OutputGrid AfterNextOutput( gridSize );
   OutputGrid PostAfterNextOutput( gridSize );

   SweeneyBurnham methodObject11;
   
   methodObject11.doTimestep( PreviousInput, CurrentInput );
   methodObject11.getResults( CurrentOutput );
   methodObject11.doTimestep( CurrentInput, NextInput );
   methodObject11.getResults( NextOutput );
   methodObject11.doTimestep( NextInput, AfterNextInput );
   methodObject11.getResults( AfterNextOutput );
   methodObject11.doTimestep( AfterNextInput, PostAfterNextInput );
   methodObject11.getResults( PostAfterNextOutput );

   double expected = VreFromDeltas( tempDelta1 );
   EXPECT_NEAR( expected, PostAfterNextOutput.printVR(), expected * 1.0e-6);
}

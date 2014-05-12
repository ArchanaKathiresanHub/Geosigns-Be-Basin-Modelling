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

   if ( rt1 == rt2 )
      return rt1 * ( time1 - time2 );
   else
      return ( rt2 - rt1 ) / ( temp2 - temp1 ) / std::log( methodsF ) * ( time1 - time2 ) * 10.0;
}

/** The following test checks the VR calculation for modified Lopatin parameters
    dT = 0, dt = 1 Myr                                                              */
TEST ( VreTtiMethod, testing_VR_Calculations1 )
{
   double time1 = 100.0;
   double time2 = 99.0;
   double temp1 = 180.0;
   double temp2 = 180.0;
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

   double expected = std::exp( methodsP * std::log( ttiValue ) - methodsQ );

   EXPECT_NEAR( expected, CurrentOutput.printVR(), expected * 1.0e-6 );
}

/** The following test checks the VR calculation for modified Lopatin parameters
    dT = 360, dt = 0.001 Myr                                                              */
TEST ( VreTtiMethod, testing_VR_Calculations2 )
{
   double time1 = 100.0;
   double time2 = 99.999;
   double temp1 = 180.0;
   double temp2 = -180.0;
   int gridSize = 10;
   
   InputGrid PreviousInput( time1, temp1, gridSize );
   InputGrid CurrentInput( time2, temp2, gridSize );

   OutputGrid CurrentOutput( gridSize );

   VreTtiMethod * methodObject2 = dynamic_cast<VreTtiMethod*>(VitriniteReflectance::create("Modified Lopatin"));
   
   methodObject2->doTimestep( PreviousInput, CurrentInput );
   methodObject2->getResults( CurrentOutput );

   double methodsF = 5.1;
   double methodsP = 0.0821;
   double methodsQ = 0.625;
   double ttiValue = quickTTI( time1, time2, temp1, temp2, methodsF );

   double expected = std::exp( methodsP * std::log( ttiValue ) - methodsQ );

   EXPECT_NEAR( expected, CurrentOutput.printVR(), expected * 1.0e-6 );
}

/** The following test checks the VR calculation for modified Lopatin parameters
    dT = 20, dt = 10 Myr                                                              */
TEST ( VreTtiMethod, testing_VR_Calculations3 )
{
   double time1 = 100.0;
   double time2 = 90.0;
   double temp1 = 80.0;
   double temp2 = 100.0;
   int gridSize = 10;
   
   InputGrid PreviousInput( time1, temp1, gridSize );
   InputGrid CurrentInput( time2, temp2, gridSize );

   OutputGrid CurrentOutput( gridSize );

   VreTtiMethod * methodObject3 = dynamic_cast<VreTtiMethod*>(VitriniteReflectance::create("Modified Lopatin"));
   
   methodObject3->doTimestep( PreviousInput, CurrentInput );
   methodObject3->getResults( CurrentOutput );

   double methodsF = 5.1;
   double methodsP = 0.0821;
   double methodsQ = 0.625;
   double ttiValue = quickTTI( time1, time2, temp1, temp2, methodsF );
   const double ttiBegin = std::exp( ( std::log ( 0.2 ) + methodsQ ) / methodsP );

   if ( ttiValue + ttiBegin > 0.0 )
	  ttiValue += ttiBegin;
   else
	  ttiValue = ttiBegin;

   double expected = std::exp( methodsP * std::log( ttiValue ) - methodsQ );

   EXPECT_NEAR( expected, CurrentOutput.printVR(), expected * 1.0e-6 );
}

/** The following test checks the VR calculation for modified Lopatin parameters
    dT = 1e-2, dt = 1e-3 Myr                                                              */
TEST ( VreTtiMethod, testing_VR_Calculations4 )
{
   double time1 = 100.0;
   double time2 = 99.999;
   double temp1 = 160.0;
   double temp2 = 159.99;
   int gridSize = 10;
   
   InputGrid PreviousInput( time1, temp1, gridSize );
   InputGrid CurrentInput( time2, temp2, gridSize );

   OutputGrid CurrentOutput( gridSize );

   VreTtiMethod * methodObject4 = dynamic_cast<VreTtiMethod*>(VitriniteReflectance::create("Modified Lopatin"));
   
   methodObject4->doTimestep( PreviousInput, CurrentInput );
   methodObject4->getResults( CurrentOutput );

   double methodsF = 5.1;
   double methodsP = 0.0821;
   double methodsQ = 0.625;
   double ttiValue = quickTTI( time1, time2, temp1, temp2, methodsF );
   const double ttiBegin = std::exp( ( std::log ( 0.2 ) + methodsQ ) / methodsP );

   if ( ttiValue + ttiBegin > 0.0 )
	  ttiValue += ttiBegin;
   else
	  ttiValue = ttiBegin;

   double expected = std::exp( methodsP * std::log( ttiValue ) - methodsQ );

   EXPECT_NEAR( expected, CurrentOutput.printVR(), expected * 1.0e-6 );
}


/** The following test checks the VR calculation for Lopatin-Dykstra parameters
    dT = 100, dt = 10 Myr                                                              */
TEST ( VreTtiMethod, testing_VR_Calculations5 )
{
   double time1 = 100.0;
   double time2 = 90.0;
   double temp1 = 80.0;
   double temp2 = 180.0;
   int gridSize = 10;
   
   InputGrid PreviousInput( time1, temp1, gridSize );
   InputGrid CurrentInput( time2, temp2, gridSize );

   OutputGrid CurrentOutput( gridSize );

   VreTtiMethod * methodObject5 = dynamic_cast<VreTtiMethod*>(VitriniteReflectance::create("Lopatin & Dykstra"));
   
   methodObject5->doTimestep( PreviousInput, CurrentInput );
   methodObject5->getResults( CurrentOutput );

   double methodsF = 2.0;
   double methodsP = 0.23697;
   double methodsQ = 1.04431;
   double ttiValue = quickTTI( time1, time2, temp1, temp2, methodsF );
   const double ttiBegin = std::exp( ( std::log ( 0.2 ) + methodsQ ) / methodsP );

   if ( ttiValue + ttiBegin > 0.0 )
	  ttiValue += ttiBegin;
   else
	  ttiValue = ttiBegin;
   
   double expected = std::exp( methodsP * std::log( ttiValue ) - methodsQ );

   EXPECT_NEAR( expected, CurrentOutput.printVR(), expected * 1.0e-6 );
}

/** The following test checks the VR calculation for Lopatin-Dykstra parameters
    dT = 100, dt = 0.001 Myr                                                              */
TEST ( VreTtiMethod, testing_VR_Calculations6 )
{
   double time1 = 100.0;
   double time2 = 99.999;
   double temp1 = 80.0;
   double temp2 = 180.0;
   int gridSize = 10;
   
   InputGrid PreviousInput( time1, temp1, gridSize );
   InputGrid CurrentInput( time2, temp2, gridSize );

   OutputGrid CurrentOutput( gridSize );

   VreTtiMethod * methodObject6 = dynamic_cast<VreTtiMethod*>(VitriniteReflectance::create("Lopatin & Dykstra"));
   
   methodObject6->doTimestep( PreviousInput, CurrentInput );
   methodObject6->getResults( CurrentOutput );

   double methodsF = 2.0;
   double methodsP = 0.23697;
   double methodsQ = 1.04431;
   double ttiValue = quickTTI( time1, time2, temp1, temp2, methodsF );
   const double ttiBegin = std::exp( ( std::log ( 0.2 ) + methodsQ ) / methodsP );

   if ( ttiValue + ttiBegin > 0.0 )
	  ttiValue += ttiBegin;
   else
	  ttiValue = ttiBegin;
   
   double expected = std::exp( methodsP * std::log( ttiValue ) - methodsQ );

   EXPECT_NEAR( expected, CurrentOutput.printVR(), expected * 1.0e-6 );
}

/** The following test checks the VR calculation for Lopatin-Dykstra parameters
    dT = 1, dt = 0.001 Myr                                                              */
TEST ( VreTtiMethod, testing_VR_Calculations7 )
{
   double time1 = 100.0;
   double time2 = 10.0;
   double temp1 = 181.0;
   double temp2 = 180.0;
   int gridSize = 10;
   
   InputGrid PreviousInput( time1, temp1, gridSize );
   InputGrid CurrentInput( time2, temp2, gridSize );

   OutputGrid CurrentOutput( gridSize );

   VreTtiMethod * methodObject7 = dynamic_cast<VreTtiMethod*>(VitriniteReflectance::create("Lopatin & Dykstra"));
   
   methodObject7->doTimestep( PreviousInput, CurrentInput );
   methodObject7->getResults( CurrentOutput );

   double methodsF = 2.0;
   double methodsP = 0.23697;
   double methodsQ = 1.04431;
   double ttiValue = quickTTI( time1, time2, temp1, temp2, methodsF );
   const double ttiBegin = std::exp( ( std::log ( 0.2 ) + methodsQ ) / methodsP );

   if ( ttiValue + ttiBegin > 0.0 )
	  ttiValue += ttiBegin;
   else
	  ttiValue = ttiBegin;
   
   double expected = std::exp( methodsP * std::log( ttiValue ) - methodsQ );

   EXPECT_NEAR( expected, CurrentOutput.printVR(), expected * 1.0e-6 );
}

/** The following test checks the VR calculation for Lopatin-Dykstra parameters
    dT = 0, dt = 10 Myr                                                              */
TEST ( VreTtiMethod, testing_VR_Calculations8 )
{
   double time1 = 100.0;
   double time2 = 90.0;
   double temp1 = 180.0;
   double temp2 = 180.0;
   int gridSize = 10;
   
   InputGrid PreviousInput( time1, temp1, gridSize );
   InputGrid CurrentInput( time2, temp2, gridSize );

   OutputGrid CurrentOutput( gridSize );

   VreTtiMethod * methodObject8 = dynamic_cast<VreTtiMethod*>(VitriniteReflectance::create("Lopatin & Dykstra"));
   
   methodObject8->doTimestep( PreviousInput, CurrentInput );
   methodObject8->getResults( CurrentOutput );

   double methodsF = 2.0;
   double methodsP = 0.23697;
   double methodsQ = 1.04431;
   double ttiValue = quickTTI( time1, time2, temp1, temp2, methodsF );
   const double ttiBegin = std::exp( ( std::log ( 0.2 ) + methodsQ ) / methodsP );

   if ( ttiValue + ttiBegin > 0.0 )
	  ttiValue += ttiBegin;
   else
	  ttiValue = ttiBegin;
   
   double expected = std::exp( methodsP * std::log( ttiValue ) - methodsQ );

   EXPECT_NEAR( expected, CurrentOutput.printVR(), expected * 1.0e-6 );
}

/** Death test
    Asserts death if sizes if time1 < time2 ( => timestep < 0 )                */
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

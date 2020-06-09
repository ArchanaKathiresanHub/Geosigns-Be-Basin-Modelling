//
// Copyright (C) 2014 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "../src/BrinePhases.h"
#include "../src/BrineVelocity.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>



/// NOTE: Unit tests that follow depend on the correctness of the
///       hard-coded parameter ranges. If those change, tests will
///       either fail or no longer cover the entirety of the
///       parameter space.


/// Testing that velocity values across (but also outside the
/// allowed range of) the parameter space are not negative.
TEST ( BrineVelocity, testing_non_negative )
{
   GeoPhysics::Brine::Velocity velocity;
   for ( int j=0; j<=4; ++j )
   {
      GeoPhysics::Brine::PhaseStateScalar phase(0.1*double(j));
      for ( int i=0; i<=4000; ++i )
      {
         phase.set( -1000.0, 0.1*double(i) );
         EXPECT_GT( velocity.get(phase), 0.0 );
         phase.set(  -100.0, 0.1*double(i) );
         EXPECT_GT( velocity.get(phase), 0.0 );
         phase.set(     0.0, 0.1*double(i) );
         EXPECT_GT( velocity.get(phase), 0.0 );
         phase.set(    80.0, 0.1*double(i) );
         EXPECT_GT( velocity.get(phase), 0.0 );
         phase.set(   150.0, 0.1*double(i) );
         EXPECT_GT( velocity.get(phase), 0.0 );
         phase.set(   280.0, 0.1*double(i) );
         EXPECT_GT( velocity.get(phase), 0.0 );
         phase.set(   450.0, 0.1*double(i) );
         EXPECT_GT( velocity.get(phase), 0.0 );
         phase.set(   680.0, 0.1*double(i) );
         EXPECT_GT( velocity.get(phase), 0.0 );
         phase.set(   900.0, 0.1*double(i) );
         EXPECT_GT( velocity.get(phase), 0.0 );
         phase.set(  1200.0, 0.1*double(i) );
         EXPECT_GT( velocity.get(phase), 0.0 );
         phase.set(  2000.0, 0.1*double(i) );
         EXPECT_GT( velocity.get(phase), 0.0 );
         phase.set(  6800.0, 0.1*double(i) );
         EXPECT_GT( velocity.get(phase), 0.0 );
      }
   }
}


/// Testing that velocity values grow with growing salinity
/// in the aqueous phase and that they decrease with
/// increasing salinity in the vapour phase.
TEST ( BrineVelocity, testing_ordering_salinity )
{
   GeoPhysics::Brine::Velocity velocity;
   for ( int j=0; j<4; ++j )
   {
      GeoPhysics::Brine::PhaseStateScalar phase1( 0.1*double(j+1) );
      GeoPhysics::Brine::PhaseStateScalar phase2( 0.1*double(j) );
      for ( int i=0; i<=100; ++i )
      {
         for ( int k=0; k<=50; ++k )
         {
            double temp = 30.0*double(k);
            double pres = 0.1*double(10*i);
            if ( pres < 0.1 )
            {
               pres = 0.1;
            }
            // Does not depend on salinity so we can use one or the other
            double T2 = phase1.findT2( pres );
            double T1 = phase1.findT1( T2 );
              
            phase1.set( 30.0*double(k), 0.1*double(10*i) );
            phase2.set( 30.0*double(k), 0.1*double(10*i) );
            if ( temp < T1 )
            {
               EXPECT_GT( velocity.get( phase1 ), velocity.get( phase2 ) );
            }
            else if ( temp > T2 )
            {
               EXPECT_GT( velocity.get( phase2 ), velocity.get( phase1 ) );
            }
            else
               continue;
         }
      }
   }
}


/// Testing continuity across T1 and T2.
TEST ( BrineVelocity, testing_velocity_continuity )
{
   const double epsilon = 1.0e-15;
   
   GeoPhysics::Brine::Velocity velocity;
   for ( int j=0; j<8; ++j )
   {
      GeoPhysics::Brine::PhaseStateScalar phase( 0.1 * 0.5 * double(j) );
      for ( int i=0; i<100; ++i )
      {
         double pressure = 0.1 * double(20*i);

         if ( pressure < 0.1 )
         {
            pressure = 0.1;
         }
     
         double highTemp = phase.findT2( pressure );
         double lowTemp  = phase.findT1( highTemp );
         
         phase.set( lowTemp * (1.0 - epsilon) , pressure );
         double vel1 = velocity.get( phase );
         phase.set( lowTemp * (1.0 + epsilon) , pressure );
         double vel2 = velocity.get( phase );
         EXPECT_NEAR (vel1, vel2, 1.0e-10);
         
         phase.set( highTemp * (1.0 - epsilon) , pressure );
         vel1 = velocity.get( phase );
         phase.set( highTemp * (1.0 + epsilon) , pressure );
         vel2 = velocity.get( phase );
         EXPECT_NEAR (vel1, vel2, 1.0e-10);
      }
   }
}

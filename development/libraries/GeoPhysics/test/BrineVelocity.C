//
// Copyright (C) 2014 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "../src/BrineVelocity.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>



/// NOTE: Unit tests that follow depend on the correctness of the
///       hard-coded parameter ranges. If those change, tests will
///       either fail or no longer cover the entirety of the
///       parameter space.

class BrineVelocityTest: public GeoPhysics::Brine::Velocity
{
   BrineVelocityTest() = delete;
public:
   BrineVelocityTest( const double sal ) : GeoPhysics::Brine::Velocity(sal) {}
   double findT2Test( const double pressure ) const
   {
      return findT2 (pressure);
   }

   double findT1Test( const double higherTemperature ) const
   {
      return findT1 (higherTemperature);
   }

   double aqueousBatzleWangTest( const double temperature, const double pressure )const
   {
      return aqueousBatzleWang (temperature, pressure);
   }

   double vapourIdealGasTest( const double temperature )const
   {
      return vapourIdealGas (temperature);
   }

};


/// Testing that velocity values across (but also outside the
/// allowed range of) the parameter space are not negative.
TEST ( BrineVelocity, testing_non_negative )
{
   for ( int j=0; j<=4; ++j )
   {
      BrineVelocityTest valuesCheck(0.1*double(j));
      for ( int i=0; i<=4000; ++i )
      {
         EXPECT_GE( valuesCheck.phaseChange( -1000.0, 0.1*double(i) ), 0.0 );
         EXPECT_GE( valuesCheck.phaseChange( -100.0, 0.1*double(i) ), 0.0 );
         EXPECT_GE( valuesCheck.phaseChange( 0.0, 0.1*double(i) ), 0.0 );
         EXPECT_GE( valuesCheck.phaseChange( 80.0, 0.1*double(i) ), 0.0 );
         EXPECT_GE( valuesCheck.phaseChange( 150.0, 0.1*double(i) ), 0.0 );
         EXPECT_GE( valuesCheck.phaseChange( 280.0, 0.1*double(i) ), 0.0 );
         EXPECT_GE( valuesCheck.phaseChange( 450.0, 0.1*double(i) ), 0.0 );
         EXPECT_GE( valuesCheck.phaseChange( 680.0, 0.1*double(i) ), 0.0 );
         EXPECT_GE( valuesCheck.phaseChange( 900.0, 0.1*double(i) ), 0.0 );
         EXPECT_GE( valuesCheck.phaseChange( 1200.0, 0.1*double(i) ), 0.0 );
         EXPECT_GE( valuesCheck.phaseChange( 2000.0, 0.1*double(i) ), 0.0 );
         EXPECT_GE( valuesCheck.phaseChange( 6800.0, 0.1*double(i) ), 0.0 );
      }
   }
}


/// Testing that velocity values grow with growing salinity
/// in the aqueous phase and that they decrease with
/// increasing salinity in the vapour phase.
TEST ( BrineVelocity, testing_ordering_salinity )
{
   for ( int j=0; j<4; ++j )
   {
      BrineVelocityTest valuesCheck1(0.1*double(j+1));
      BrineVelocityTest valuesCheck2(0.1*double(j));
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
            double T2 = valuesCheck1.findT2Test( pres );
            double T1 = valuesCheck1.findT1Test( T2 );
              
            if ( temp < T1 )
            {
               EXPECT_GT( valuesCheck1.phaseChange( 30.0*double(k), 0.1*double(10*i) ), 
                          valuesCheck2.phaseChange( 30.0*double(k), 0.1*double(10*i) ) );
            }
            else if ( temp > T2 )
            {
               EXPECT_GT( valuesCheck2.phaseChange( 30.0*double(k), 0.1*double(10*i) ), 
                          valuesCheck1.phaseChange( 30.0*double(k), 0.1*double(10*i) ) );
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

   for ( int j=0; j<8; ++j )
   {
      BrineVelocityTest valuesCheck(0.1 * 0.5 * double(j));
      for ( int i=0; i<100; ++i )
      {
         double pressure = 0.1 * double(20*i);

         if ( pressure < 0.1 )
         {
            pressure = 0.1;
         }
     
         double highTemp = valuesCheck.findT2Test( pressure );
         double lowTemp = valuesCheck.findT1Test( highTemp );

         EXPECT_NEAR (valuesCheck.phaseChange (lowTemp * (1.0 - epsilon) , pressure) , 
                      valuesCheck.phaseChange (lowTemp * (1.0 + epsilon) , pressure) , 1.0e-10);
         EXPECT_NEAR (valuesCheck.phaseChange (highTemp * (1.0 - epsilon) , pressure) , 
                      valuesCheck.phaseChange (highTemp * (1.0 + epsilon) , pressure) , 1.0e-10);
      }
   }
}


/// Testing region selection of velocity.
TEST ( BrineVelocity, testing_velocity_region )
{
   const double epsilon = 1.0e-1;

   for ( int j=0; j<8; ++j )
   {
      BrineVelocityTest valuesCheck(0.1 * 0.5 * double(j));
      for ( int i=0; i<=100; ++i )
      {
         double pressure, salinity, highTemp, lowTemp;

         for ( int k=1; k<4; ++k )
         {
            pressure = 0.1 * double(10*i);
            if ( pressure < 0.1 )
            {
               pressure = 0.1;
            }
     
            highTemp = valuesCheck.findT2Test( pressure );
            lowTemp = valuesCheck.findT1Test( highTemp );

            EXPECT_NEAR( valuesCheck.phaseChange( ( lowTemp - epsilon) / double(k), pressure ), 
                         valuesCheck.aqueousBatzleWangTest( ( lowTemp - epsilon ) / double(k), pressure ), 1.0e-10 );

            EXPECT_NEAR( valuesCheck.phaseChange( ( highTemp + epsilon) * double(k), pressure ), 
                    valuesCheck.vapourIdealGasTest( ( highTemp + epsilon ) * double(k) ), 1.0e-10 );
         }

         EXPECT_NEAR( valuesCheck.phaseChange( 0.5 * ( highTemp + lowTemp), pressure ),
                      0.5 * ( valuesCheck.vapourIdealGasTest( highTemp ) + valuesCheck.aqueousBatzleWangTest( lowTemp, pressure ) ), 1.0e-10 );

      }
   }
}

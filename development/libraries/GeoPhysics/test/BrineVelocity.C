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

#include <cmath>
#include <assert.h>
#include <sstream>
#include <iostream>
#include <iomanip>

#include "NumericFunctions.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>



/// NOTE: Unit tests that follow depend on the correctness of the
///       hard-coded parameter ranges. If those change, tests will
///       either fail or no longer cover the entirety of the
///       parameter space.

class BrineVelocityTest: public GeoPhysics::BrineVelocity
{

public:
   double findT2Test( const double pressure ) const
   {
      return findT2 (pressure);
   }

   double findT1Test( const double higherTemperature ) const
   {
      return findT1 (higherTemperature);
   }

   double aqueousBatzleWangTest( const double temperature, const double pressure, const double salinity )const
   {
      return aqueousBatzleWang (temperature, pressure, salinity);
   }

   double vapourIdealGasTest( const double temperature, const double salinity )const
   {
      return vapourIdealGas (temperature, salinity);
   }

};


/// Testing that velocity values across (but also outside the
/// allowed range of) the parameter space are not negative.
TEST ( BrineVelocity, testing_non_negative )
{
   BrineVelocityTest valuesCheck;
  
   for ( int i=0; i<=4000; ++i )
   {
      for ( int j=0; j<=4; ++j )
      {
         EXPECT_GE( valuesCheck.phaseChange( -1000.0, 0.1*double(i), 0.1*double(j) ), 0.0 );
         EXPECT_GE( valuesCheck.phaseChange( -100.0, 0.1*double(i), 0.1*double(j) ), 0.0 );
         EXPECT_GE( valuesCheck.phaseChange( 0.0, 0.1*double(i), 0.1*double(j) ), 0.0 );
         EXPECT_GE( valuesCheck.phaseChange( 80.0, 0.1*double(i), 0.1*double(j) ), 0.0 );
         EXPECT_GE( valuesCheck.phaseChange( 150.0, 0.1*double(i), 0.1*double(j) ), 0.0 );
         EXPECT_GE( valuesCheck.phaseChange( 280.0, 0.1*double(i), 0.1*double(j) ), 0.0 );
         EXPECT_GE( valuesCheck.phaseChange( 450.0, 0.1*double(i), 0.1*double(j) ), 0.0 );
         EXPECT_GE( valuesCheck.phaseChange( 680.0, 0.1*double(i), 0.1*double(j) ), 0.0 );
         EXPECT_GE( valuesCheck.phaseChange( 900.0, 0.1*double(i), 0.1*double(j) ), 0.0 );
         EXPECT_GE( valuesCheck.phaseChange( 1200.0, 0.1*double(i), 0.1*double(j) ), 0.0 );
         EXPECT_GE( valuesCheck.phaseChange( 2000.0, 0.1*double(i), 0.1*double(j) ), 0.0 );
         EXPECT_GE( valuesCheck.phaseChange( 6800.0, 0.1*double(i), 0.1*double(j) ), 0.0 );
      }
   }
}


/// Testing that velocity values grow with growing salinity
/// in the aqueous phase and that they decrease with
/// increasing salinity in the vapour phase.
TEST ( BrineVelocity, testing_ordering_salinity )
{
   BrineVelocityTest valuesCheck;
  
   for ( int i=0; i<=100; ++i )
   {
      for ( int j=0; j<4; ++j )
      {
         for ( int k=0; k<=50; ++k )
         {
            double temp = 30.0*double(k);
            double pres = 0.1*double(10*i);
            if ( pres < 0.1 )
            {
               pres = 0.1;
            }
            double T2 = valuesCheck.findT2Test( pres );
            double T1 = valuesCheck.findT1Test( T2 );
              
            if ( temp < T1 )
            {
               EXPECT_GT( valuesCheck.phaseChange( 30.0*double(k), 0.1*double(10*i), 0.1*double(j+1) ), 
                          valuesCheck.phaseChange( 30.0*double(k), 0.1*double(10*i), 0.1*double(j) ) );
            }
            else if ( temp > T2 )
            {
               EXPECT_GT( valuesCheck.phaseChange( 30.0*double(k), 0.1*double(10*i), 0.1*double(j) ), 
                          valuesCheck.phaseChange( 30.0*double(k), 0.1*double(10*i), 0.1*double(j+1) ) );
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
   BrineVelocityTest valuesCheck;
   const double epsilon = 1.0e-15;

   for ( int i=0; i<100; ++i )
   {
      for ( int j=0; j<8; ++j )
      {
         double pressure = 0.1 * double(20*i);

         if ( pressure < 0.1 )
         {
            pressure = 0.1;
         }

         double salinity = 0.1 * 0.5 * double(j);
	  
         double highTemp = valuesCheck.findT2Test( pressure );
         double lowTemp = valuesCheck.findT1Test( highTemp );

         EXPECT_NEAR (valuesCheck.phaseChange (lowTemp * (1.0 - epsilon) , pressure, salinity) , 
                      valuesCheck.phaseChange (lowTemp * (1.0 + epsilon) , pressure, salinity) , 1.0e-10);
         EXPECT_NEAR (valuesCheck.phaseChange (highTemp * (1.0 - epsilon) , pressure, salinity) , 
                      valuesCheck.phaseChange (highTemp * (1.0 + epsilon) , pressure, salinity) , 1.0e-10);
      }
   }
}


/// Testing region selection of velocity.
TEST ( BrineVelocity, testing_velocity_region )
{
   BrineVelocityTest valuesCheck;
   const double epsilon = 1.0e-1;

   for ( int i=0; i<=100; ++i )
   {
      for ( int j=0; j<8; ++j )
      {
         double pressure, salinity, highTemp, lowTemp;

         for ( int k=1; k<4; ++k )
         {
            pressure = 0.1 * double(10*i);
            if ( pressure < 0.1 )
            {
               pressure = 0.1;
            }

            salinity = 0.1 * 0.5 * double(j);
	  
            highTemp = valuesCheck.findT2Test( pressure );
            lowTemp = valuesCheck.findT1Test( highTemp );

            EXPECT_NEAR( valuesCheck.phaseChange( ( lowTemp - epsilon) / double(k), pressure, salinity ), 
                         valuesCheck.aqueousBatzleWangTest( ( lowTemp - epsilon ) / double(k), pressure, salinity ), 1.0e-10 );

            EXPECT_NEAR( valuesCheck.phaseChange( ( highTemp + epsilon) * double(k), pressure, salinity ), 
	                 valuesCheck.vapourIdealGasTest( ( highTemp + epsilon ) * double(k), salinity ), 1.0e-10 );
         }

         EXPECT_NEAR( valuesCheck.phaseChange( 0.5 * ( highTemp + lowTemp), pressure, salinity ),
                      0.5 * ( valuesCheck.vapourIdealGasTest( highTemp, salinity ) + valuesCheck.aqueousBatzleWangTest( lowTemp, pressure, salinity ) ), 1.0e-10 );

      }
   }
}

//
// Copyright (C) 2014 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "../src/BrineViscosity.h"

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


class BrineViscosityTest: public GeoPhysics::BrineViscosity
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

   double aqueousBatzleWangTest( const double temperature, const double salinity )const
   {
      return aqueousBatzleWang (temperature, salinity);
   }

   double vapourConstantTest( )const
   {
      return vapourConstant ();
   }

};


/// Testing that viscosity values across (but also outside the
/// allowed range of) the parameter space are not negative.
TEST ( BrineViscosity, testing_non_negative )
{
   BrineViscosityTest valuesCheck;
  
   for ( int i=0; i<=4000; ++i )
   {
      for ( int j=0; j<=4; ++j )
      {
         EXPECT_GT( valuesCheck.phaseChange( -1000.0, 0.1*double(i), 0.1*double(j) ), 0.0 );
         EXPECT_GT( valuesCheck.phaseChange( -100.0, 0.1*double(i), 0.1*double(j) ), 0.0 );
         EXPECT_GT( valuesCheck.phaseChange( 0.0, 0.1*double(i), 0.1*double(j) ), 0.0 );
         EXPECT_GT( valuesCheck.phaseChange( 80.0, 0.1*double(i), 0.1*double(j) ), 0.0 );
         EXPECT_GT( valuesCheck.phaseChange( 150.0, 0.1*double(i), 0.1*double(j) ), 0.0 );
         EXPECT_GT( valuesCheck.phaseChange( 280.0, 0.1*double(i), 0.1*double(j) ), 0.0 );
         EXPECT_GT( valuesCheck.phaseChange( 450.0, 0.1*double(i), 0.1*double(j) ), 0.0 );
         EXPECT_GT( valuesCheck.phaseChange( 680.0, 0.1*double(i), 0.1*double(j) ), 0.0 );
         EXPECT_GT( valuesCheck.phaseChange( 900.0, 0.1*double(i), 0.1*double(j) ), 0.0 );
         EXPECT_GT( valuesCheck.phaseChange( 1200.0, 0.1*double(i), 0.1*double(j) ), 0.0 );
         EXPECT_GT( valuesCheck.phaseChange( 2000.0, 0.1*double(i), 0.1*double(j) ), 0.0 );
         EXPECT_GT( valuesCheck.phaseChange( 6800.0, 0.1*double(i), 0.1*double(j) ), 0.0 );
      }
   }
}


/// Testing that viscosity values grow with growing salinity.
/// Note that this is true up to moderate salinity values.
/// The viscosity curve for S=0.35, for example, goes partially
/// below lower-salinity curves. Hence j<3 in the for loop.
TEST ( BrineViscosity, testing_ordering_salinity )
{
   BrineViscosityTest valuesCheck;
 
   for ( int i=0; i<=100; ++i )
   {
      double pressure = 0.1*double(20*i);
      if ( pressure < 0.1 )
      {
         pressure = 0.1;
      }
      for ( int j=0; j<3; ++j )
      {
         for ( int k=0; k<=50; ++k )
         {
            if ( 30.0*double(k) >= valuesCheck.findT2Test( pressure ) )
            {
               break;
            }
            EXPECT_GT( valuesCheck.phaseChange( 30.0*double(k), pressure, 0.1*double(j+1) ), valuesCheck.phaseChange( 30.0*double(k), pressure, 0.1*double(j) ) );
         }
      }
   }
}

/// Testing that viscosity values grow with decreasing temperature
/// up until the constant-viscosity vapour phase.
TEST ( BrineViscosity, testing_ordering_temperature )
{
   BrineViscosityTest valuesCheck;
  
   for ( int i=0; i<100; ++i )
   {
      double pressure = 0.1*double(20*i);
      if ( pressure < 0.1 )
      {
         pressure = 0.1;
      }
      for ( int j=0; j<4; ++j )
      {
         for ( int k=0; k<150; ++k )
         {
            if ( 10.0*double(k+1) >= valuesCheck.findT2Test( pressure ) )
            {
               break;
            }
            EXPECT_GT( valuesCheck.phaseChange( 10.0*double(k), pressure, 0.1*double(j) ), valuesCheck.phaseChange( 10.0*double(k+1), pressure, 0.1*double(j) ) );
         }
      }
   }
}


/// Testing continuity across T1 and T2.
TEST ( BrineViscosity, testing_viscosity_continuity )
{
   BrineViscosityTest valuesCheck;

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

         EXPECT_NEAR( valuesCheck.phaseChange( lowTemp * (1.0 - epsilon) , pressure, salinity ),
                      valuesCheck.phaseChange( lowTemp * (1.0 + epsilon) , pressure, salinity ), 1.0e-10 );
         EXPECT_NEAR( valuesCheck.phaseChange( highTemp * (1.0 - epsilon) , pressure, salinity ),
                      valuesCheck.phaseChange( highTemp * (1.0 + epsilon) , pressure, salinity ), 1.0e-10 );
      }
   }
}


/// Testing region selection of viscosity.
TEST ( BrineViscosity, testing_viscosity_region )
{
   BrineViscosityTest valuesCheck;
   const double epsilon = 1.0e-1;

   for ( int i=0; i<=100; ++i )
   {
      for ( int j=0; j<8; ++j )
      {
         double pressure, highTemp, lowTemp, salinity;
         for ( int k=1; k<4; ++k )
         {
            pressure = 0.1 * double(20*i);
            if ( pressure < 0.1 )
            {
               pressure = 0.1;
            }

            salinity = 0.1 * 0.5 * double(j);
	  
            highTemp = valuesCheck.findT2Test( pressure );
            lowTemp = valuesCheck.findT1Test( highTemp );

            EXPECT_NEAR( valuesCheck.phaseChange( ( lowTemp - epsilon) / double(k), pressure, salinity ), valuesCheck.aqueousBatzleWangTest( ( lowTemp - epsilon ) / double(k), salinity ), 1.0e-10 );
	      
            EXPECT_NEAR( valuesCheck.phaseChange( ( highTemp + epsilon) * double(k), pressure, salinity ), valuesCheck.vapourConstantTest(), 1.0e-10 );
         }

         EXPECT_NEAR( valuesCheck.phaseChange( ( highTemp + lowTemp) * 0.5, pressure, salinity ),
                      0.5 * ( valuesCheck.vapourConstantTest() + valuesCheck.aqueousBatzleWangTest( lowTemp, salinity ) ), 1.0e-10 );

      }
   }
}


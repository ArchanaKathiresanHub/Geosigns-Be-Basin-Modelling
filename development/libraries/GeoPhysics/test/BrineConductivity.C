//
// Copyright (C) 2014 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "../src/BrineConductivity.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>



/// NOTE: Unit tests that follow depend on the correctness of the
///       hard-coded parameter ranges. If those change, tests will
///       either fail or no longer cover the entirety of the
///       parameter space.


class BrineConductivityTest: public GeoPhysics::Brine::Conductivity
{
   BrineConductivityTest() = delete;
public:
   BrineConductivityTest( const double sal ) : GeoPhysics::Brine::Conductivity(sal) {}
   double findT2Test( const double pressure ) const
   {
      return findT2 (pressure);
   }

   double findT1Test( const double higherTemperature ) const
   {
      return findT1 (higherTemperature);
   }

   double aqueousTableTest( const double temperature, const double pressure )const
   {
      return aqueousTable (temperature, pressure);
   }

   double vapourTableTest( const double temperature, const double pressure )const
   {
      return vapourTable (temperature, pressure);
   }

};


/// Testing that conductivity values across (but also outside the
/// allowed range of) the parameter space are not negative.
TEST ( BrineConductivity, testing_non_negative )
{
   BrineConductivityTest valuesCheck(0.0);

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


/// Testing continuity across T1 and T2.
TEST ( BrineConductivity, testing_conductivity_continuity )
{
   BrineConductivityTest valuesCheck(0.0);
   const double epsilon = 1.0e-15;

   for ( int i=0; i<100; ++i )
   {
      double pressure = 0.1 * double(10*i);

      if ( pressure < 0.1 )
      {
         pressure = 0.1;
      }

      double highTemp = valuesCheck.findT2Test( pressure );
      double lowTemp = valuesCheck.findT1Test( highTemp );

      EXPECT_NEAR( valuesCheck.phaseChange( lowTemp * (1.0 - epsilon) , pressure ), 
                   valuesCheck.phaseChange( lowTemp * (1.0 + epsilon) , pressure ), 1.0e-10 );
      EXPECT_NEAR( valuesCheck.phaseChange( highTemp * (1.0 - epsilon) , pressure ), 
                   valuesCheck.phaseChange( highTemp * (1.0 + epsilon) , pressure ), 1.0e-10 );
   }
}


/// Testing region selection of conductivity.
TEST ( BrineConductivity, testing_conductivity_region )
{
   BrineConductivityTest valuesCheck(0.0);
   const double epsilon = 1.0e-1;

   for ( int i=0; i<=100; ++i )
   {
      double pressure, highTemp, lowTemp;

      for ( int k=1; k<5; ++k )
      {
         pressure = 0.1 * double(10*i);
         if ( pressure < 0.1 )
         {
            pressure = 0.1;
         }
     
         highTemp = valuesCheck.findT2Test( pressure );
         lowTemp = valuesCheck.findT1Test( highTemp );

         EXPECT_NEAR( valuesCheck.phaseChange( lowTemp * ( 1.0 - epsilon) / double(k), pressure ), 
                      valuesCheck.aqueousTableTest( lowTemp * ( 1.0 - epsilon ) / double(k), pressure ), 1.0e-10 );

         double temp;
         if ( double(k) * (highTemp+epsilon) > 800.0 )
         {
            temp = 800.0;
         }
         else
         {
            temp =  double(k) * (highTemp+epsilon);
         }

         EXPECT_NEAR( valuesCheck.phaseChange( temp, pressure ), 
                      valuesCheck.vapourTableTest( temp, pressure ), 1.0e-10 );
      }

      EXPECT_NEAR( valuesCheck.phaseChange( 0.5 * ( highTemp + lowTemp), pressure ),
           0.5 * ( valuesCheck.vapourTableTest( highTemp, pressure ) + valuesCheck.aqueousTableTest( lowTemp, pressure ) ), 1.0e-10 );

   }
}





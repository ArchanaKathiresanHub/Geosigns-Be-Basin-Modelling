//
// Copyright (C) 2014 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "../src/BrineDensity.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>



/// NOTE: Unit tests that follow depend on the correctness of the
///       hard-coded parameter ranges. If those change, tests will
///       either fail or no longer cover the entirety of the
///       parameter space.


class BrineDensityTest: public GeoPhysics::Brine::Density
{
   BrineDensityTest() = delete;
public:
   BrineDensityTest( const double sal ) : GeoPhysics::Brine::Density(sal) {}
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

   double vapourIdealGasTest( const double temperature, const double pressure )const
   {
      return vapourIdealGas (temperature, pressure);
   }
};


/// Testing that density values across (but also outside the
/// allowed range of) the parameter space are not negative.
TEST ( BrineDensity, testing_non_negative )
{
   for ( int j=0; j<=4; ++j )
   {
      BrineDensityTest valuesCheck(0.1*double(j));
      for ( int i=0; i<=4000; ++i )
      {
         EXPECT_GT( valuesCheck.phaseChange( -1000.0, 0.1*double(i) ), 0.0 );
         EXPECT_GT( valuesCheck.phaseChange( -100.0, 0.1*double(i) ), 0.0 );
         EXPECT_GT( valuesCheck.phaseChange( 0.0, 0.1*double(i) ), 0.0 );
         EXPECT_GT( valuesCheck.phaseChange( 80.0, 0.1*double(i) ), 0.0 );
         EXPECT_GT( valuesCheck.phaseChange( 150.0, 0.1*double(i) ), 0.0 );
         EXPECT_GT( valuesCheck.phaseChange( 280.0, 0.1*double(i) ), 0.0 );
         EXPECT_GT( valuesCheck.phaseChange( 450.0, 0.1*double(i) ), 0.0 );
         EXPECT_GT( valuesCheck.phaseChange( 680.0, 0.1*double(i) ), 0.0 );
         EXPECT_GT( valuesCheck.phaseChange( 900.0, 0.1*double(i) ), 0.0 );
         EXPECT_GT( valuesCheck.phaseChange( 1200.0, 0.1*double(i) ), 0.0 );
         EXPECT_GT( valuesCheck.phaseChange( 2000.0, 0.1*double(i) ), 0.0 );
         EXPECT_GT( valuesCheck.phaseChange( 6800.0, 0.1*double(i) ), 0.0 );
      }
   }
}


/// Testing that density values grow with growing salinity.
TEST ( BrineDensity, testing_ordering_salinity )
{
   for ( int j=0; j<4; ++j )
   {
      BrineDensityTest valuesCheck1(0.1*double(j+1));
      BrineDensityTest valuesCheck2(0.1*double(j));
      for ( int i=0; i<=100; ++i )
      {
         for ( int k=0; k<=50; ++k )
         {
            EXPECT_GT( valuesCheck1.phaseChange( 30.0*double(k), 0.1*double(20*i) ),
                       valuesCheck2.phaseChange( 30.0*double(k), 0.1*double(20*i) ) );
         }
      }
   }
}


/// Testing that density values grow with growing pressure.
TEST ( BrineDensity, testing_ordering_pressure )
{
   for ( int j=0; j<4; ++j )
   {
      BrineDensityTest valuesCheck(0.1*double(j));
      for ( int i=1; i<2000; ++i )
      {
         for ( int k=0; k<=50; ++k )
         {
            EXPECT_GT( valuesCheck.phaseChange( 30.0*double(k), 0.1*double(i+1) ),
                       valuesCheck.phaseChange( 30.0*double(k), 0.1*double(i) ) );
         }
      }
   }
}


/// Testing that density values grow with decreasing temperature.
TEST ( BrineDensity, testing_ordering_temperature )
{
   for ( int j=0; j<4; ++j )
   {
      BrineDensityTest valuesCheck(0.1*double(j));
      for ( int i=0; i<=100; ++i )
      {
         for ( int k=0; k<150; ++k )
         {
            EXPECT_GT( valuesCheck.phaseChange( 10.0*double(k), 0.1*double(20*i) ),
                       valuesCheck.phaseChange( 10.0*double(k+1), 0.1*double(20*i) ) );
         }
      }
   }
}


/// Testing continuity across T1 and T2.
TEST ( BrineDensity, testing_density_continuity )
{
   
   const double epsilon = 1.0e-15;

   for ( int j=0; j<8; ++j )
   {
      BrineDensityTest valuesCheck(0.1 * 0.5 * double(j));
      for ( int i=0; i<100; ++i )
      {
         double pressure = 0.1 * double(20*i);

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
}


/// Testing region selection of density.
TEST ( BrineDensity, testing_density_region )
{
   const double epsilon = 1.0e-3;

   for ( int j=0; j<8; ++j )
   {
      BrineDensityTest valuesCheck(0.1 * 0.5 * double(j));
      for ( int i=0; i<=100; ++i )
      {
         double pressure, highTemp, lowTemp;
         for ( int k=1; k<3; ++k )
         {
            pressure = 0.1 * double(20*i);
            if ( pressure < 0.1 )
            {
               pressure = 0.1;
            }
     
            highTemp = valuesCheck.findT2Test( pressure );
            lowTemp = valuesCheck.findT1Test( highTemp );

            EXPECT_NEAR( valuesCheck.phaseChange( ( lowTemp - epsilon) / double(k), pressure ),
                         valuesCheck.aqueousBatzleWangTest( ( lowTemp - epsilon ) / double(k), pressure ), 1.0e-10 );

            EXPECT_NEAR( valuesCheck.phaseChange( ( highTemp + epsilon) * double(k), pressure ),
                         valuesCheck.vapourIdealGasTest( ( highTemp + epsilon ) * double(k), pressure ), 1.0e-10 );
         }
         EXPECT_NEAR( valuesCheck.phaseChange( ( highTemp + lowTemp) * 0.5, pressure ),
                      0.5 * ( valuesCheck.vapourIdealGasTest( highTemp, pressure ) +
                              valuesCheck.aqueousBatzleWangTest( lowTemp, pressure ) ), 1.0e-10 );
      }
   }
}


/// Testing that following the derivative reproduces density values.
TEST ( BrineDensity, testing_derivative_temperature )
{
   for ( int j=0; j<8; ++j )
   {
      BrineDensityTest valuesCheck(0.1 * 0.5 * double(j));
      for ( int i=0; i<=50; ++i )
      {
         double pressure = 0.1 * double(40*i);
         if ( pressure < 0.1 )
         {
            pressure = 0.1;
         }
         double temperature = 0.0;

         double densityIncrement = valuesCheck.phaseChange( temperature, pressure );

         while ( temperature <= 1500.0 )
         {
            densityIncrement += valuesCheck.computeDerivativeT( temperature, pressure ) * 1.0e-2 * pressure;
            temperature += 1.0e-2 * pressure;
         }

         temperature -= 1.0e-2 * pressure;

         double densityValue = valuesCheck.phaseChange( temperature, pressure );

         EXPECT_NEAR( densityIncrement, densityValue, 1.0e-2 * densityValue );
      }
   }
}


/// Testing that following the derivative reproduces density values.
TEST ( BrineDensity, testing_derivative_pressure )
{
   for ( int j=0; j<8; ++j )
   {
      BrineDensityTest valuesCheck(0.1 * 0.5 * double(j));
      for ( int i=0; i<=50; ++i )
      {
         double temperature = 30.0 * double (i);
         double pressure = 0.1;

         double densityIncrement = valuesCheck.phaseChange( temperature, pressure );

         double aux;

         while ( pressure <= 200.0 )
         {
            densityIncrement += valuesCheck.computeDerivativeP( temperature, pressure ) * 1.0e-3 * pressure;
            aux = pressure;
            pressure += 1.0e-3 * pressure;
         }

         pressure = aux;

         double densityValue = valuesCheck.phaseChange( temperature, pressure );

         EXPECT_NEAR( densityIncrement, densityValue, 1.0e-2 * densityValue );
      }
   }
}

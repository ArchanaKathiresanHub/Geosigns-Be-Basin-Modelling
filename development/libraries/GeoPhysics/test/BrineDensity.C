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
#include "../src/BrinePhases.h"
#include "ArrayDefinitions.h"
#include "AlignedMemoryAllocator.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>



/// NOTE: Unit tests that follow depend on the correctness of the
///       hard-coded parameter ranges. If those change, tests will
///       either fail or no longer cover the entirety of the
///       parameter space.

/// Testing that density values across (but also outside the
/// allowed range of) the parameter space are not negative.
TEST ( BrineDensity, testing_non_negative )
{
   GeoPhysics::Brine::Density density;
   for ( int j=0; j<=4; ++j )
   {
      GeoPhysics::Brine::PhaseStateScalar phase(0.1*double(j));
      for ( int i=0; i<=4000; ++i )
      {
         phase.set( -1000.0, 0.1*double(i) );
         EXPECT_GT( density.get(phase), 0.0 );
         phase.set(  -100.0, 0.1*double(i) );
         EXPECT_GT( density.get(phase), 0.0 );
         phase.set(     0.0, 0.1*double(i) );
         EXPECT_GT( density.get(phase), 0.0 );
         phase.set(    80.0, 0.1*double(i) );
         EXPECT_GT( density.get(phase), 0.0 );
         phase.set(   150.0, 0.1*double(i) );
         EXPECT_GT( density.get(phase), 0.0 );
         phase.set(   280.0, 0.1*double(i) );
         EXPECT_GT( density.get(phase), 0.0 );
         phase.set(   450.0, 0.1*double(i) );
         EXPECT_GT( density.get(phase), 0.0 );
         phase.set(   680.0, 0.1*double(i) );
         EXPECT_GT( density.get(phase), 0.0 );
         phase.set(   900.0, 0.1*double(i) );
         EXPECT_GT( density.get(phase), 0.0 );
         phase.set(  1200.0, 0.1*double(i) );
         EXPECT_GT( density.get(phase), 0.0 );
         phase.set(  2000.0, 0.1*double(i) );
         EXPECT_GT( density.get(phase), 0.0 );
         phase.set(  6800.0, 0.1*double(i) );
         EXPECT_GT( density.get(phase), 0.0 );
      }
   }
}


/// Testing that density values grow with growing salinity.
TEST ( BrineDensity, testing_ordering_salinity )
{
   GeoPhysics::Brine::Density density;
   for ( int j=0; j<4; ++j )
   {
      GeoPhysics::Brine::PhaseStateScalar phase1( 0.1*double(j+1) );
      GeoPhysics::Brine::PhaseStateScalar phase2( 0.1*double(j) );
      for ( int i=0; i<=100; ++i )
      {
         for ( int k=0; k<=50; ++k )
         {
            phase1.set( 30.0*double(k), 0.1*double(20*i) );
            phase2.set( 30.0*double(k), 0.1*double(20*i) );
            EXPECT_GT( density.get( phase1 ), density.get( phase2 ) );
         }
      }
   }
}


/// Testing that density values grow with growing pressure.
TEST ( BrineDensity, testing_ordering_pressure )
{
   GeoPhysics::Brine::Density density;
   for ( int j=0; j<4; ++j )
   {
      GeoPhysics::Brine::PhaseStateScalar phase( 0.1*double(j) );
      for ( int i=1; i<2000; ++i )
      {
         for ( int k=0; k<=50; ++k )
         {
            phase.set( 30.0*double(k), 0.1*double(i+1) );
            double dens1 = density.get( phase );
            phase.set( 30.0*double(k), 0.1*double(i) );
            double dens2 = density.get( phase );
            EXPECT_GT( dens1, dens2 );
         }
      }
   }
}


/// Testing that density values grow with decreasing temperature.
TEST ( BrineDensity, testing_ordering_temperature )
{
   GeoPhysics::Brine::Density density;
   for ( int j=0; j<4; ++j )
   {
      GeoPhysics::Brine::PhaseStateScalar phase( 0.1*double(j) );
      for ( int i=0; i<=100; ++i )
      {
         for ( int k=0; k<150; ++k )
         {
            phase.set( 10.0*double(k), 0.1*double(20*i) );
            double dens1 = density.get( phase );
            phase.set( 10.0*double(k+1), 0.1*double(20*i) );
            double dens2 = density.get( phase );
            EXPECT_GT( dens1, dens2 );
         }
      }
   }
}


/// Testing continuity across T1 and T2.
TEST ( BrineDensity, testing_density_continuity )
{
   const double epsilon = 1.0e-15;
   
   GeoPhysics::Brine::Density density;
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
         
         phase.set( lowTemp * (1.0 - epsilon), pressure );
         double dens1 = density.get( phase );
         phase.set( lowTemp * (1.0 + epsilon), pressure );
         double dens2 = density.get( phase );
         EXPECT_NEAR( dens1, dens2, 1.0e-10 );

         phase.set( highTemp * (1.0 - epsilon), pressure );
         dens1 = density.get( phase );
         phase.set( highTemp * (1.0 + epsilon), pressure );
         dens2 = density.get( phase );
         EXPECT_NEAR( dens1, dens2, 1.0e-10 );
      }
   }
}


/// Testing exact values for the scalar interface
TEST ( BrineDensity, testing_density_exact_values )
{
   const double salinity = 0.2;
   GeoPhysics::Brine::Density density;
   GeoPhysics::Brine::PhaseStateScalar phase( salinity );

   phase.set( 666.0, 271.0 );
   EXPECT_NEAR( density.get(phase), 6.724230217159792e+02, 1.0e-11 ); // vapor

   phase.set( 130.0, GeoPhysics::Brine::s_MinPressure );
   EXPECT_NEAR( density.get(phase), 7.833811630165068e-01, 1.0e-11 ); // vapor

   phase.set( 600.0, 100.0 );
   EXPECT_NEAR( density.get(phase), 3.616295632252603e+02, 1.0e-11 ); // vapor

   phase.set( GeoPhysics::Brine::s_MinTemperature, 127.0 );
   EXPECT_NEAR( density.get(phase), 1.203361193983992e+03, 1.0e-11 ); // aqueousBatzleWang

   phase.set( GeoPhysics::Brine::s_MinTemperature, GeoPhysics::Brine::s_MaxPressure );
   EXPECT_NEAR( density.get(phase), 1.228480973286224e+03, 1.0e-11 ); // aqueousBatzleWang

   phase.set( 130.0, GeoPhysics::Brine::s_MaxPressure );
   EXPECT_NEAR( density.get(phase), 1.128463321998839e+03, 1.0e-11 ); // aqueousBatzleWang

   phase.set( 50.0, 100.0 );
   EXPECT_NEAR( density.get(phase), 1.166077450814598e+03, 1.0e-11 ); // aqueousBatzleWang

   phase.set( 392.0, 60.0 );
   EXPECT_NEAR( density.get(phase), 6.036037330093739e+02, 1.0e-11 ); // transition
}


/// Testing exact values for the vector interface
TEST ( BrineDensity, testing_density_exact_values_vec )
{
   const int n = 8;
   const double salinity = 0.2;
   GeoPhysics::Brine::Density density;
   GeoPhysics::Brine::PhaseStateVec phase( n, salinity );

   ArrayDefs::Real_ptr temp = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( n );
   temp[0] = 666.0;
   temp[1] = 130.0;
   temp[2] = 600.0;
   temp[3] = GeoPhysics::Brine::s_MinTemperature;
   temp[4] = GeoPhysics::Brine::s_MinTemperature;
   temp[5] = 130.0;
   temp[6] = 50.0;
   temp[7] = 392.0;
   ArrayDefs::Real_ptr pres = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( n );
   pres[0] = 271.0;
   pres[1] = GeoPhysics::Brine::s_MinPressure;
   pres[2] = 100.0;
   pres[3] = 127.0;
   pres[4] = GeoPhysics::Brine::s_MaxPressure;
   pres[5] = GeoPhysics::Brine::s_MaxPressure;
   pres[6] = 100.0;
   pres[7] = 60.0;
   ArrayDefs::Real_ptr dens = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( n );
   for( int k=0; k<n; ++k ) dens[k] = 0.0;

   phase.set( n, temp, pres );
   density.get( phase, dens );

   EXPECT_NEAR( dens[0], 6.724230217159792e+02, 1.0e-11 ); // vapor
   EXPECT_NEAR( dens[1], 7.833811630165068e-01, 1.0e-11 ); // vapor
   EXPECT_NEAR( dens[2], 3.616295632252603e+02, 1.0e-11 ); // vapor
   EXPECT_NEAR( dens[3], 1.203361193983992e+03, 1.0e-11 ); // aqueousBatzleWang
   EXPECT_NEAR( dens[4], 1.228480973286224e+03, 1.0e-11 ); // aqueousBatzleWang
   EXPECT_NEAR( dens[5], 1.128463321998839e+03, 1.0e-11 ); // aqueousBatzleWang
   EXPECT_NEAR( dens[6], 1.166077450814598e+03, 1.0e-11 ); // aqueousBatzleWang
   EXPECT_NEAR( dens[7], 6.036037330093739e+02, 1.0e-11 ); // transition

   AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::free( temp );
   AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::free( pres );
   AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::free( dens );
}


/// Testing that following the derivative reproduces density values.
TEST ( BrineDensity, testing_derivative_temperature )
{
   GeoPhysics::Brine::Density density;
   for ( int j=0; j<8; ++j )
   {
      GeoPhysics::Brine::PhaseStateScalar phase( 0.1 * 0.5 * double(j) );
      for ( int i=0; i<=50; ++i )
      {
         double pressure = 0.1 * double(40*i);
         if ( pressure < 0.1 )
         {
            pressure = 0.1;
         }
         double temperature = 0.0;

         phase.set( temperature, pressure );
         double densityIncrement = density.get( phase );

         while ( temperature <= 1500.0 )
         {
            phase.set( temperature, pressure );
            densityIncrement += density.computeDerivativeT( phase ) * 1.0e-2 * pressure;
            temperature += 1.0e-2 * pressure;
         }

         temperature -= 1.0e-2 * pressure;
         
         phase.set( temperature, pressure );
         double densityValue = density.get( phase );

         EXPECT_NEAR( densityIncrement, densityValue, 1.0e-2 * densityValue );
      }
   }
}


/// Testing the exact values for density derivatives wrt temperature with vector interface.
TEST ( BrineDensity, testing_derivative_temperature_exact_values )
{
   const double salinity = 0.2;
   GeoPhysics::Brine::Density density;
   GeoPhysics::Brine::PhaseStateScalar phase( salinity );

   phase.set( 666.0, 271.0 );
   EXPECT_NEAR( density.computeDerivativeT(phase), -7.161054544366126e-01, 1.0e-11 ); // vapor

   phase.set( 130.0, GeoPhysics::Brine::s_MinPressure );
   EXPECT_NEAR( density.computeDerivativeT(phase), -1.943873853638975e-03, 1.0e-11 ); // vapor

   phase.set( 600.0, 100.0 );
   EXPECT_NEAR( density.computeDerivativeT(phase), -4.142377585627265e-01, 1.0e-11 ); // vapor

   phase.set( GeoPhysics::Brine::s_MinTemperature, 127.0 );
   EXPECT_NEAR( density.computeDerivativeT(phase), -5.736979999999999e-01, 1.0e-11 ); // aqueousBatzleWang

   phase.set( GeoPhysics::Brine::s_MinTemperature, GeoPhysics::Brine::s_MaxPressure );
   EXPECT_NEAR( density.computeDerivativeT(phase), -8.2e-01, 1.0e-11 ); // aqueousBatzleWang

   phase.set( 130.0, GeoPhysics::Brine::s_MaxPressure );
   EXPECT_NEAR( density.computeDerivativeT(phase), -7.33095e-01, 1.0e-11 ); // aqueousBatzleWang

   phase.set( 50.0, 100.0 );
   EXPECT_NEAR( density.computeDerivativeT(phase), -5.94625e-01, 1.0e-11 ); // aqueousBatzleWang

   phase.set( 392.0, 60.0 );
   EXPECT_NEAR( density.computeDerivativeT(phase), -1.427311698009724e+01, 1.0e-11 ); // transition with Finite Difference
}


/// Testing the exact values for density derivatives wrt temperature with scalar interface.
TEST ( BrineDensity, testing_derivative_temperature_exact_values_vec )
{
   const int n = 8;
   const double salinity = 0.2;
   GeoPhysics::Brine::Density density;
   GeoPhysics::Brine::PhaseStateVec phase( n, salinity );

   ArrayDefs::Real_ptr temp = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( n );
   temp[0] = 666.0;
   temp[1] = 130.0;
   temp[2] = 600.0;
   temp[3] = GeoPhysics::Brine::s_MinTemperature;
   temp[4] = GeoPhysics::Brine::s_MinTemperature;
   temp[5] = 130.0;
   temp[6] = 50.0;
   temp[7] = 392.0;
   ArrayDefs::Real_ptr pres = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( n );
   pres[0] = 271.0;
   pres[1] = GeoPhysics::Brine::s_MinPressure;
   pres[2] = 100.0;
   pres[3] = 127.0;
   pres[4] = GeoPhysics::Brine::s_MaxPressure;
   pres[5] = GeoPhysics::Brine::s_MaxPressure;
   pres[6] = 100.0;
   pres[7] = 60.0;
   ArrayDefs::Real_ptr densDers = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( n );
   for( int k=0; k<n; ++k ) densDers[k] = 0.0;

   phase.set( n, temp, pres );
   density.computeDerivativeT( phase, densDers );

   EXPECT_NEAR( densDers[0], -7.161054544366126e-01, 1.0e-11 ); // vapor
   EXPECT_NEAR( densDers[1], -1.943873853638975e-03, 1.0e-11 ); // vapor
   EXPECT_NEAR( densDers[2], -4.142377585627265e-01, 1.0e-11 ); // vapor
   EXPECT_NEAR( densDers[3], -5.736979999999999e-01, 1.0e-11 ); // aqueousBatzleWang
   EXPECT_NEAR( densDers[4], -8.2e-01              , 1.0e-11 ); // aqueousBatzleWang
   EXPECT_NEAR( densDers[5], -7.33095e-01          , 1.0e-11 ); // aqueousBatzleWang
   EXPECT_NEAR( densDers[6], -5.94625e-01          , 1.0e-11 ); // aqueousBatzleWang
   EXPECT_NEAR( densDers[7], -1.427311698009724e+01, 1.0e-11 ); // transition with Finite Difference

   AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::free( temp );
   AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::free( pres );
   AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::free( densDers );
}


/// Testing that following the derivative reproduces density values.
TEST ( BrineDensity, testing_derivative_pressure )
{
   GeoPhysics::Brine::Density density;
   for ( int j=0; j<8; ++j )
   {
      GeoPhysics::Brine::PhaseStateScalar phase( 0.1 * 0.5 * double(j) );
      for ( int i=0; i<=50; ++i )
      {
         double temperature = 30.0 * double (i);
         double pressure = 0.1;
         
         phase.set( temperature, pressure );
         double densityIncrement = density.get( phase );

         double aux;

         while ( pressure <= 200.0 )
         {
            phase.set( temperature, pressure );
            densityIncrement += density.computeDerivativeP( phase ) * 1.0e-3 * pressure;
            aux = pressure;
            pressure += 1.0e-3 * pressure;
         }

         pressure = aux;
         
         phase.set( temperature, pressure );
         double densityValue = density.get( phase );

         EXPECT_NEAR( densityIncrement, densityValue, 1.0e-2 * densityValue );
      }
   }
}


/// Testing the exact values for density derivatives wrt pressure with vector interface.
TEST ( BrineDensity, testing_derivative_pressure_exact_values )
{
   const double salinity = 0.2;
   GeoPhysics::Brine::Density density;
   GeoPhysics::Brine::PhaseStateScalar phase( salinity );

   phase.set( 666.0, 271.0 );
   EXPECT_NEAR( density.computeDerivativeP(phase), 3.362115108579896e+00, 1.0e-11 ); // vapor

   phase.set( 130.0, GeoPhysics::Brine::s_MinPressure );
   EXPECT_NEAR( density.computeDerivativeP(phase), 7.833811630165068e+00, 1.0e-11 ); // vapor

   phase.set( 600.0, 100.0 );
   EXPECT_NEAR( density.computeDerivativeP(phase), 3.616295632252603e+00, 1.0e-11 ); // vapor

   phase.set( GeoPhysics::Brine::s_MinTemperature, 127.0 );
   EXPECT_NEAR( density.computeDerivativeP(phase), 3.68418e-01, 1.0e-11 ); // aqueousBatzleWang

   phase.set( GeoPhysics::Brine::s_MinTemperature, GeoPhysics::Brine::s_MaxPressure );
   EXPECT_NEAR( density.computeDerivativeP(phase), 3.198e-01, 1.0e-11 ); // aqueousBatzleWang

   phase.set( 130.0, GeoPhysics::Brine::s_MaxPressure );
   EXPECT_NEAR( density.computeDerivativeP(phase), 1.04039e-01, 1.0e-11 ); // aqueousBatzleWang

   phase.set( 50.0, 100.0 );
   EXPECT_NEAR( density.computeDerivativeP(phase), 2.68775e-01, 1.0e-11 ); // aqueousBatzleWang

   phase.set( 392.0, 60.0 );
   EXPECT_NEAR( density.computeDerivativeP(phase), 3.785747537481778e+00, 1.0e-11 ); // transition with Finite Difference
}


/// Testing the exact values for density derivatives wrt pressure with scalar interface.
TEST ( BrineDensity, testing_derivative_pressure_exact_values_vec )
{
   const int n = 8;
   const double salinity = 0.2;
   GeoPhysics::Brine::Density density;
   GeoPhysics::Brine::PhaseStateVec phase( n, salinity );

   ArrayDefs::Real_ptr temp = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( n );
   temp[0] = 666.0;
   temp[1] = 130.0;
   temp[2] = 600.0;
   temp[3] = GeoPhysics::Brine::s_MinTemperature;
   temp[4] = GeoPhysics::Brine::s_MinTemperature;
   temp[5] = 130.0;
   temp[6] = 50.0;
   temp[7] = 392.0;
   ArrayDefs::Real_ptr pres = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( n );
   pres[0] = 271.0;
   pres[1] = GeoPhysics::Brine::s_MinPressure;
   pres[2] = 100.0;
   pres[3] = 127.0;
   pres[4] = GeoPhysics::Brine::s_MaxPressure;
   pres[5] = GeoPhysics::Brine::s_MaxPressure;
   pres[6] = 100.0;
   pres[7] = 60.0;
   ArrayDefs::Real_ptr densDers = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( n );
   for( int k=0; k<n; ++k ) densDers[k] = 0.0;

   phase.set( n, temp, pres );
   density.computeDerivativeP( phase, densDers );

   EXPECT_NEAR( densDers[0], 3.362115108579896e+00, 1.0e-11 ); // vapor
   EXPECT_NEAR( densDers[1], 7.833811630165068e+00, 1.0e-11 ); // vapor
   EXPECT_NEAR( densDers[2], 3.616295632252603e+00, 1.0e-11 ); // vapor
   EXPECT_NEAR( densDers[3], 3.68418e-01          , 1.0e-11 ); // aqueousBatzleWang
   EXPECT_NEAR( densDers[4], 3.198e-01            , 1.0e-11 ); // aqueousBatzleWang
   EXPECT_NEAR( densDers[5], 1.04039e-01          , 1.0e-11 ); // aqueousBatzleWang
   EXPECT_NEAR( densDers[6], 2.68775e-01          , 1.0e-11 ); // aqueousBatzleWang
   EXPECT_NEAR( densDers[7], 3.785747537481778e+00, 1.0e-11 ); // transition with Finite Difference

   AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::free( temp );
   AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::free( pres );
   AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::free( densDers );
}

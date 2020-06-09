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
#include "../src/BrineViscosity.h"
#include "ArrayDefinitions.h"
#include "AlignedMemoryAllocator.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>



/// NOTE: Unit tests that follow depend on the correctness of the
///       hard-coded parameter ranges. If those change, tests will
///       either fail or no longer cover the entirety of the
///       parameter space.


/// Testing that viscosity values across (but also outside the
/// allowed range of) the parameter space are not negative.
TEST ( BrineViscosity, testing_non_negative )
{
   for ( int j=0; j<=4; ++j )
   {
      GeoPhysics::Brine::PhaseStateScalar phase(0.1*double(j));
      GeoPhysics::Brine::Viscosity visc(phase.getSalinity());
      for ( int i=0; i<=4000; ++i )
      {
         phase.set( -1000.0, 0.1*double(i) );
         EXPECT_GT( visc.get(phase), 0.0 );
         phase.set(  -100.0, 0.1*double(i) );
         EXPECT_GT( visc.get(phase), 0.0 );
         phase.set(     0.0, 0.1*double(i) );
         EXPECT_GT( visc.get(phase), 0.0 );
         phase.set(    80.0, 0.1*double(i) );
         EXPECT_GT( visc.get(phase), 0.0 );
         phase.set(   150.0, 0.1*double(i) );
         EXPECT_GT( visc.get(phase), 0.0 );
         phase.set(   280.0, 0.1*double(i) );
         EXPECT_GT( visc.get(phase), 0.0 );
         phase.set(   450.0, 0.1*double(i) );
         EXPECT_GT( visc.get(phase), 0.0 );
         phase.set(   680.0, 0.1*double(i) );
         EXPECT_GT( visc.get(phase), 0.0 );
         phase.set(   900.0, 0.1*double(i) );
         EXPECT_GT( visc.get(phase), 0.0 );
         phase.set(  1200.0, 0.1*double(i) );
         EXPECT_GT( visc.get(phase), 0.0 );
         phase.set(  2000.0, 0.1*double(i) );
         EXPECT_GT( visc.get(phase), 0.0 );
         phase.set(  6800.0, 0.1*double(i) );
         EXPECT_GT( visc.get(phase), 0.0 );
      }
   }
}


/// Testing that viscosity values across (but also outside the
/// allowed range of) the parameter space are not negative.
TEST ( BrineViscosity, testing_non_negative_vector )
{
   const int n = 12;
   ArrayDefs::Real_ptr temp = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( n );
   temp[ 0] = -1000.0;
   temp[ 1] = -100.0;
   temp[ 2] = 0.0;
   temp[ 3] = 80.0;
   temp[ 4] = 150.0;
   temp[ 5] = 280.0;
   temp[ 6] = 450.0;
   temp[ 7] = 680.0;
   temp[ 8] = 900.0;
   temp[ 9] = 1200.0;
   temp[10] = 2000.0;
   temp[11] = 68000.0;
   ArrayDefs::Real_ptr pres = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( n );
   ArrayDefs::Real_ptr visc = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( n );
   for( int k=0; k<n; ++k )
      visc[k] = 0.0;

   for ( int j=0; j<=4; ++j )
   {
      GeoPhysics::Brine::PhaseStateVec phase(n,0.1*double(j));
      GeoPhysics::Brine::Viscosity viscosity(phase.getSalinity());
      EXPECT_EQ( phase.getVectorSize(), n );
      for ( int i=0; i<=4000; ++i )
      {
         for ( int k=0; k<n; ++k )
            pres[k] = 0.1*double(i);
         phase.set( n, temp, pres );
         viscosity.get( phase, visc );
         for ( int k=0; k<n; ++k ) 
            EXPECT_GT( visc[k], 0.0 );
      }
   }

   AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::free( temp );
   AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::free( pres );
   AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::free( visc );
}


/// Testing that viscosity values grow with growing salinity.
/// Note that this is true up to moderate salinity values.
/// The viscosity curve for S=0.35, for example, goes partially
/// below lower-salinity curves. Hence j<3 in the for loop.
TEST ( BrineViscosity, testing_ordering_salinity )
{
   for ( int j=0; j<3; ++j )
   {
      GeoPhysics::Brine::PhaseStateScalar phase1( 0.1*double(j+1) );
      GeoPhysics::Brine::Viscosity viscosity1(phase1.getSalinity());
      GeoPhysics::Brine::PhaseStateScalar phase2( 0.1*double(j) );
      GeoPhysics::Brine::Viscosity viscosity2(phase2.getSalinity());
      for ( int i=0; i<=100; ++i )
      {
         double pressure = 0.1*double(20*i);
         if ( pressure < 0.1 )
         {
            pressure = 0.1;
         }
         for ( int k=0; k<=50; ++k )
         {
            // findT2Test does not depend on salinity so we can use one or the other
            if ( 30.0*double(k) >= phase1.findT2( pressure ) )
            {
               break;
            }
            phase1.set( 30.0*double(k), pressure );
            phase2.set( 30.0*double(k), pressure );
            EXPECT_GT( viscosity1.get( phase1 ), viscosity2.get( phase1 ) );
         }
      }
   }
}

/// Testing that viscosity values grow with decreasing temperature
/// up until the constant-viscosity vapour phase.
TEST ( BrineViscosity, testing_ordering_temperature )
{
   for ( int j=0; j<4; ++j )
   {
      GeoPhysics::Brine::PhaseStateScalar phase( 0.1*double(j) );
      GeoPhysics::Brine::Viscosity viscosity(phase.getSalinity());
      for ( int i=0; i<100; ++i )
      {
         double pressure = 0.1*double(20*i);
         if ( pressure < 0.1 )
         {
            pressure = 0.1;
         }
         for ( int k=0; k<150; ++k )
         {
            if ( 10.0*double(k+1) >= phase.findT2( pressure ) )
            {
               break;
            }
            phase.set( 10.0*double(k), pressure );
            double visc1 = viscosity.get( phase );
            phase.set( 10.0*double(k+1), pressure );
            double visc2 = viscosity.get( phase );
            EXPECT_GT( visc1, visc2 );
         }
      }
   }
}


/// Testing continuity across T1 and T2.
TEST ( BrineViscosity, testing_viscosity_continuity )
{
   const double epsilon = 1.0e-15;

   for ( int j=0; j<8; ++j )
   {
      GeoPhysics::Brine::PhaseStateScalar phase( 0.1 * 0.5 * double(j) );
      GeoPhysics::Brine::Viscosity viscosity(phase.getSalinity());
      for ( int i=0; i<100; ++i ) 
      {
         double pressure = 0.1 * double(20*i);

         if ( pressure < 0.1 )
         {
            pressure = 0.1;
         }
     
         double highTemp = phase.findT2( pressure );
         double lowTemp = phase.findT1( highTemp );
         
         phase.set( lowTemp * (1.0 - epsilon), pressure );
         double visc1 = viscosity.get( phase );
         phase.set( lowTemp * (1.0 + epsilon), pressure );
         double visc2 = viscosity.get( phase );
         EXPECT_NEAR( visc1, visc2, 1.0e-10 );

         phase.set( highTemp * (1.0 - epsilon), pressure );
         visc1 = viscosity.get( phase );
         phase.set( highTemp * (1.0 + epsilon), pressure );
         visc2 = viscosity.get( phase );
         EXPECT_NEAR( visc1, visc2, 1.0e-10 );
      }
   }
}


/// Testing continuity across T1 and T2.
TEST ( BrineViscosity, testing_viscosity_continuity_vector )
{
   const double epsilon = 1.0e-15;
   const int n = 100;
   ArrayDefs::Real_ptr pres  = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( n );
   ArrayDefs::Real_ptr visc1 = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( n );
   ArrayDefs::Real_ptr visc2 = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( n );
   ArrayDefs::Real_ptr visc3 = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( n );
   ArrayDefs::Real_ptr visc4 = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( n );
   ArrayDefs::Real_ptr temp1 = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( n );
   ArrayDefs::Real_ptr temp2 = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( n );
   ArrayDefs::Real_ptr temp3 = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( n );
   ArrayDefs::Real_ptr temp4 = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( n );

   for ( int j=0; j<8; ++j )
   {
      GeoPhysics::Brine::PhaseStateVec phase( n, 0.1 * 0.5 * double(j) );
      GeoPhysics::Brine::Viscosity viscosity(phase.getSalinity());

      for ( int i=0; i<n; ++i ) 
      {
         pres[i] = 0.1 * double(20*i);
         if ( pres[i] < 0.1 )
         {
            pres[i] = 0.1;
         }
     
         double highTemp = GeoPhysics::Brine::PhaseStateBase::findT2( pres[i] );
         double lowTemp = GeoPhysics::Brine::PhaseStateBase::findT1( highTemp );
         temp1[i] = lowTemp * (1.0 - epsilon);
         temp2[i] = lowTemp * (1.0 + epsilon);
         temp3[i] = highTemp * (1.0 - epsilon);
         temp4[i] = highTemp * (1.0 + epsilon);
      }

      phase.set( n, temp1, pres );
      viscosity.get( phase, visc1 );
      phase.set( n, temp2, pres );
      viscosity.get( phase, visc2 );
      phase.set( n, temp3, pres );
      viscosity.get( phase, visc3 );
      phase.set( n, temp4, pres );
      viscosity.get( phase, visc4 );
      
      for ( int i=0; i<n; ++i ) 
      {
         EXPECT_NEAR( visc1[i], visc2[i], 1.0e-10 );
         EXPECT_NEAR( visc3[i], visc4[i], 1.0e-10 );
      }
   }

   AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::free( pres );
   AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::free( visc1 );
   AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::free( visc2 );
   AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::free( visc3 );
   AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::free( visc4 );
   AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::free( temp1 );
   AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::free( temp2 );
   AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::free( temp3 );
   AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::free( temp4 );
}


/// Testing region selection of viscosity.
TEST ( BrineViscosity, testing_viscosity_vector_exact_val )
{
   const int n = 8;
   const double salinity = 0.2;
   GeoPhysics::Brine::PhaseStateVec phase( n, salinity );
   GeoPhysics::Brine::Viscosity viscosity( phase.getSalinity() );

   ArrayDefs::Real_ptr temp = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( n );
   temp[0] = 130.0;
   temp[1] = 130.0;
   temp[2] = 600.0;
   temp[3] = 50.0;
   temp[4] = 392.0;
   temp[5] = GeoPhysics::Brine::s_MinTemperature;
   temp[6] = GeoPhysics::Brine::s_MinTemperature;
   temp[7] = 666.0;
   ArrayDefs::Real_ptr pres = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( n );
   pres[0] = GeoPhysics::Brine::s_MinPressure;
   pres[1] = GeoPhysics::Brine::s_MaxPressure;
   pres[2] = 100.0;
   pres[3] = 100.0;
   pres[4] = 60.0;
   pres[5] = 127.0;
   pres[6] = GeoPhysics::Brine::s_MaxPressure;
   pres[7] = 271;
   ArrayDefs::Real_ptr visc = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( n );
   for( int k=0; k<n; ++k ) visc[k] = 0.0;

   phase.set( n, temp, pres );
   viscosity.get( phase, visc );

   EXPECT_NEAR( visc[0], 2.5e-05,               1.e-15 ); // vapor
   EXPECT_NEAR( visc[1], 0.0003742085594019073, 1.e-15 ); // aqueousBatzleWang
   EXPECT_NEAR( visc[2], 2.5e-05,               1.e-15 ); // vapor
   EXPECT_NEAR( visc[3], 0.000931930731491726,  1.e-15 ); // aqueousBatzleWang
   EXPECT_NEAR( visc[4], 0.0001110191130450718, 1.e-15 ); // transition
   EXPECT_NEAR( visc[5], 0.0025518,             1.e-15 );
   EXPECT_NEAR( visc[6], 0.0025518,             1.e-15 );
   EXPECT_NEAR( visc[7], 2.5e-05,               1.e-15 );

   AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::free( temp );
   AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::free( pres );
   AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::free( visc );
}


/// Testing region selection of viscosity.
TEST ( BrineViscosity, testing_viscosity_vector_exact_val_different_phases )
{
   const int n = 8;
   const double salinity = 0.2;
   GeoPhysics::Brine::PhaseStateVec phase( n, salinity );
   GeoPhysics::Brine::Viscosity viscosity( phase.getSalinity() );

   ArrayDefs::Real_ptr temp = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( n );
   temp[0] = 130.0; // water
   temp[1] = 130.0; // water
   temp[2] = 130.0; // water
   temp[3] = 392.0; // transition
   temp[4] = 392.0; // transition
   temp[5] = 392.0; // transition
   temp[6] = 392.0; // vapor
   temp[7] = 392.0; // vapor

   ArrayDefs::Real_ptr pres = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( n );
   pres[0] = GeoPhysics::Brine::s_MaxPressure; // water
   pres[1] = GeoPhysics::Brine::s_MaxPressure; // water
   pres[2] = GeoPhysics::Brine::s_MaxPressure; // water
   pres[3] = 60.0; // transition
   pres[4] = 60.0; // transition
   pres[5] = 60.0; // transition
   pres[6] = GeoPhysics::Brine::s_MinPressure; // vapor
   pres[7] = GeoPhysics::Brine::s_MinPressure; // vapor

   ArrayDefs::Real_ptr visc = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( n );
   for( int i = 0; i < n; ++i )
   {
      visc[i] = 0.0;
   }
   
   phase.set( n, temp, pres );
   viscosity.get( phase, visc );

   EXPECT_NEAR( visc[0], 0.0003742085594019073, 1.e-15 ); // water
   EXPECT_NEAR( visc[1], 0.0003742085594019073, 1.e-15 ); // water
   EXPECT_NEAR( visc[2], 0.0003742085594019073, 1.e-15 ); // water
   EXPECT_NEAR( visc[3], 0.0001110191130450718, 1.e-15 ); // transition
   EXPECT_NEAR( visc[4], 0.0001110191130450718, 1.e-15 ); // transition
   EXPECT_NEAR( visc[5], 0.0001110191130450718, 1.e-15 ); // transition
   EXPECT_NEAR( visc[6], 2.5e-05,               1.e-15 ); // vapor
   EXPECT_NEAR( visc[7], 2.5e-05,               1.e-15 ); // vapor

   AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::free( temp );
   AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::free( pres );
   AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::free( visc );
}


/// Testing region selection of viscosity.
TEST ( BrineViscosity, testing_viscosity_vector_exact_val_same_phase )
{
   const int n = 8;
   const double salinity = 0.2;
   GeoPhysics::Brine::PhaseStateVec phase( n, salinity );
   GeoPhysics::Brine::Viscosity viscosity( phase.getSalinity() );

   ArrayDefs::Real_ptr temp = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( n );
   ArrayDefs::Real_ptr pres = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( n );
   ArrayDefs::Real_ptr visc = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( n );

   // All water
   for( int i = 0; i < n; ++i )
   {
      temp[i] = 130.0;
      pres[i] = GeoPhysics::Brine::s_MaxPressure;
      visc[i] = 0.0;
   }
   phase.set( n, temp, pres );
   viscosity.get( phase, visc );
   for( int i = 0; i < n; ++i )
   {
      EXPECT_NEAR( visc[i], 0.0003742085594019073, 1.e-15 );
   }

   // All transition
   for( int i = 0; i < n; ++i )
   {
      temp[i] = 392.0;
      pres[i] = 60.0;
      visc[i] = 0.0;
   }
   phase.set( n, temp, pres );
   viscosity.get( phase, visc );
   for( int i = 0; i < n; ++i )
   {
      EXPECT_NEAR( visc[i], 0.0001110191130450718, 1.e-15 );
   }

   // All vapor
   for( int i = 0; i < n; ++i )
   {
      temp[i] = 392.0;
      pres[i] = GeoPhysics::Brine::s_MinPressure;
      visc[i] = 0.0;
   }
   phase.set( n, temp, pres );
   viscosity.get( phase, visc );
   for( int i = 0; i < n; ++i )
   {
      EXPECT_NEAR( visc[i], 2.5e-05, 1.e-15 );
   }

   AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::free( temp );
   AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::free( pres );
   AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::free( visc );
}

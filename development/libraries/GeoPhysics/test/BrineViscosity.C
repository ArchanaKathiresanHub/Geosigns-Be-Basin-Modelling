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
#include "ArrayDefinitions.h"
#include "AlignedMemoryAllocator.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>



/// NOTE: Unit tests that follow depend on the correctness of the
///       hard-coded parameter ranges. If those change, tests will
///       either fail or no longer cover the entirety of the
///       parameter space.


class BrineViscosityTest: public GeoPhysics::Brine::Viscosity
{
   BrineViscosityTest() = delete;
public:
   explicit BrineViscosityTest( const double sal ) : GeoPhysics::Brine::Viscosity(sal) {}
   double findT2Test( const double pressure ) const
   {
      return findT2 (pressure);
   }

   double findT1Test( const double higherTemperature ) const
   {
      return findT1 (higherTemperature);
   }

   double aqueousBatzleWangTest( const double temperature )const
   {
      return aqueousBatzleWang (temperature);
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
   for ( int j=0; j<=4; ++j )
   {
      BrineViscosityTest valuesCheck(0.1*double(j));
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
      BrineViscosityTest valuesCheck(0.1*double(j));
      valuesCheck.setVectorSize(n);
      EXPECT_EQ( valuesCheck.getVectorSize(), n );
      for ( int i=0; i<=4000; ++i )
      {
         for ( int k=0; k<n; ++k )
            pres[k] = 0.1*double(i);
         valuesCheck.phaseChange( n, temp, pres, visc );
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
      BrineViscosityTest valuesCheck1(0.1*double(j+1));
      BrineViscosityTest valuesCheck2(0.1*double(j));
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
            if ( 30.0*double(k) >= valuesCheck1.findT2Test( pressure ) )
            {
               break;
            }
            EXPECT_GT( valuesCheck1.phaseChange( 30.0*double(k), pressure ), valuesCheck2.phaseChange( 30.0*double(k), pressure ) );
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
      BrineViscosityTest valuesCheck(0.1*double(j));
      for ( int i=0; i<100; ++i )
      {
         double pressure = 0.1*double(20*i);
         if ( pressure < 0.1 )
         {
            pressure = 0.1;
         }
         for ( int k=0; k<150; ++k )
         {
            if ( 10.0*double(k+1) >= valuesCheck.findT2Test( pressure ) )
            {
               break;
            }
            EXPECT_GT( valuesCheck.phaseChange( 10.0*double(k), pressure ), valuesCheck.phaseChange( 10.0*double(k+1), pressure ) );
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
      BrineViscosityTest valuesCheck(0.1 * 0.5 * double(j));
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
      BrineViscosityTest valuesCheck(0.1 * 0.5 * double(j));

      for ( int i=0; i<n; ++i ) 
      {
         pres[i] = 0.1 * double(20*i);
         if ( pres[i] < 0.1 )
         {
            pres[i] = 0.1;
         }
     
         double highTemp = valuesCheck.findT2Test( pres[i] );
         double lowTemp = valuesCheck.findT1Test( highTemp );
         temp1[i] = lowTemp * (1.0 - epsilon);
         temp2[i] = lowTemp * (1.0 + epsilon);
         temp3[i] = highTemp * (1.0 - epsilon);
         temp4[i] = highTemp * (1.0 + epsilon);
      }

      valuesCheck.phaseChange( n, temp1, pres, visc1 );
      valuesCheck.phaseChange( n, temp2, pres, visc2 );
      valuesCheck.phaseChange( n, temp3, pres, visc3 );
      valuesCheck.phaseChange( n, temp4, pres, visc4 );
      
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
TEST ( BrineViscosity, testing_viscosity_region )
{
   const double epsilon = 1.0e-1;

   for ( int j=0; j<8; ++j )
   {
      BrineViscosityTest valuesCheck(0.1 * 0.5 * double(j));
      for ( int i=0; i<=100; ++i )
      {
         double pressure = 0.0;
         double highTemp = 0.0;
         double lowTemp = 0.0;
         for ( int k=1; k<4; ++k )
         {
            pressure = 0.1 * double(20*i);
            if ( pressure < 0.1 )
            {
               pressure = 0.1;
            }
     
            highTemp = valuesCheck.findT2Test( pressure );
            lowTemp = valuesCheck.findT1Test( highTemp );

            EXPECT_NEAR( valuesCheck.phaseChange( ( lowTemp - epsilon) / double(k), pressure ), valuesCheck.aqueousBatzleWangTest( ( lowTemp - epsilon ) / double(k) ), 1.0e-10 );
         
            EXPECT_NEAR( valuesCheck.phaseChange( ( highTemp + epsilon) * double(k), pressure ), valuesCheck.vapourConstantTest(), 1.0e-10 );
         }

         EXPECT_NEAR( valuesCheck.phaseChange( ( highTemp + lowTemp) * 0.5, pressure ),
                      0.5 * ( valuesCheck.vapourConstantTest() + valuesCheck.aqueousBatzleWangTest( lowTemp ) ), 1.0e-10 );

      }
   }
}


/// Testing region selection of viscosity.
TEST ( BrineViscosity, testing_viscosity_vector_exact_val )
{
   const double salinity = 0.2;
   BrineViscosityTest valuesCheck( salinity );

   const int n = 8;
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

   valuesCheck.phaseChange( n, temp, pres, visc );

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
   const double salinity = 0.2;
   BrineViscosityTest valuesCheck( salinity );

   const int n = 8;
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

   valuesCheck.phaseChange( n, temp, pres, visc );

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
   const double salinity = 0.2;
   BrineViscosityTest valuesCheck( salinity );

   const int n = 8;
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
   valuesCheck.phaseChange( n, temp, pres, visc );   
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
   valuesCheck.phaseChange( n, temp, pres, visc );   
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
   valuesCheck.phaseChange( n, temp, pres, visc );   
   for( int i = 0; i < n; ++i )
   {
      EXPECT_NEAR( visc[i], 2.5e-05, 1.e-15 );
   }

   AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::free( temp );
   AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::free( pres );
   AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::free( visc );
}

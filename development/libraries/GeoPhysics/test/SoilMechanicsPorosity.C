//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "../src/SoilMechanicsPorosity.h"
#include "AlignedMemoryAllocator.h"

#include <iostream>
#include <iomanip>
#include <cstring>
#include <cassert>
#include <cmath>
#include <limits>

#include <gtest/gtest.h>

using namespace GeoPhysics;


TEST( Mudstone, Mudstone )
{
   EXPECT_NEAR(4.992470160211E-01, soilMechanicsPorosity( 0.6, 0.03, 0.222,1.50E+00).calculate( 1.0E+5, 1.0E+6, false,0),  1E-10);

   EXPECT_NEAR(5.632000000000E-01, soilMechanicsPorosity( 0.5632, 0.03, 0.199,1.289377289377E+00).calculate( 1.0E+5, 1.0E+5, false,0),  1E-10);
     
   EXPECT_NEAR(3.150631892440E-01, soilMechanicsPorosity( 0.62, 0.03, 0.257,1.631578947368E+00).calculate( 1.0E+6, 1.0E+7, false,0),  1E-10);

   EXPECT_NEAR(3.579932495799E-01, soilMechanicsPorosity( 0.6685, 0.03, 0.322,2.016591251885E+00).calculate( 1.0E+7, 1.0E+7, true,0.01),  1E-10);
}


TEST( soilMechanicsPorosity, derivatives_testDepoPoro )
{
   // For different depositional porosities
   soilMechanicsPorosity porosity1(0.0, 0.03, 0.222, 1.50E+00 );
   EXPECT_NEAR( porosity1.calculateDerivative( 1.0E+07, 1.0E+07, false, 0.0 ), 0.0, 1e-18);

   soilMechanicsPorosity porosity2(0.2, 0.03, 0.222, 1.50E+00 );
   EXPECT_NEAR( porosity2.calculateDerivative( 1.0E+07, 1.0E+07, false, 0.0 ), 0.0, 1e-18);

   soilMechanicsPorosity porosity3(0.4, 0.03, 0.222, 1.50E+00 );
   EXPECT_NEAR( porosity3.calculateDerivative( 1.0E+07, 1.0E+07, false, 0.0 ), -1.016736732597E-08, 1e-18);
}


TEST( soilMechanicsPorosity, derivatives_testVesLoading )
{
   // For different ves (loading)
   soilMechanicsPorosity porosity(0.4, 0.03, 0.222, 1.50E+00 );
   EXPECT_NEAR( porosity.calculateDerivative( 0.0    , 0.0    , false, 0.0 ), 0.0                , 1e-18);
   EXPECT_NEAR( porosity.calculateDerivative( 1.0E+02, 1.0E+02, false, 0.0 ), 0.0                , 1e-18);
   EXPECT_NEAR( porosity.calculateDerivative( 1.0E+06, 1.0E+06, false, 0.0 ), 0.0                , 1e-18);
   EXPECT_NEAR( porosity.calculateDerivative( 1.0E+07, 1.0E+07, false, 0.0 ), -1.016736732597E-08, 1e-18);
}


TEST( soilMechanicsPorosity, derivatives_testVesUnloading )
{
   // For different ves (unloading)
   soilMechanicsPorosity porosity(0.6, 0.03, 0.222, 1.50E+00 );
   EXPECT_NEAR( porosity.calculateDerivative( 1.0E+01, 2.0E+01, false, 0.0 ), 0.0                , 1e-18);
   EXPECT_NEAR( porosity.calculateDerivative( 1.0E+03, 2.0E+03, false, 0.0 ), 0.0                , 1e-18);
   EXPECT_NEAR( porosity.calculateDerivative( 1.0E+05, 2.0E+05, false, 0.0 ), -5.247083256421E-09, 1e-18);
   EXPECT_NEAR( porosity.calculateDerivative( 1.0E+06, 2.0E+06, false, 0.0 ), -1.526049886416E-09, 1e-18);
   EXPECT_NEAR( porosity.calculateDerivative( 1.0E+07, 2.0E+07, false, 0.0 ), -3.572022256761E-10, 1e-18);
}


TEST( soilMechanicsPorosity, derivatives_testChemCompLoading )
{
   // For different chemical compaction terms (loading phase)
   soilMechanicsPorosity porosity(0.3, 0.03, 0.222, 1.50E+00 );
   EXPECT_NEAR( porosity.calculateDerivative( 1.0E+05, 1.0E+05, true, -1.00E-01 ), -1.420800000000E-06, 1e-18);
   EXPECT_NEAR( porosity.calculateDerivative( 1.0E+05, 1.0E+05, true, -2.00E-01 ), -1.798200000000E-06, 1e-18);
   EXPECT_NEAR( porosity.calculateDerivative( 1.0E+05, 1.0E+05, true, -3.00E-01 ), 0.0                , 1e-18);
   EXPECT_NEAR( porosity.calculateDerivative( 1.0E+05, 1.0E+05, true, -4.00E-01 ), 0.0                , 1e-18);
}


TEST( soilMechanicsPorosity, derivatives_testChemCompUnloading )
{
   // For different chemical compaction terms (unloading phase)
   soilMechanicsPorosity porosity(0.3, 0.03, 0.222, 1.50E+00 );
   EXPECT_NEAR( porosity.calculateDerivative( 1.0E+05, 2.0E+05, true, -1.00E-01 ), -5.247083256421E-09, 1e-18);
   EXPECT_NEAR( porosity.calculateDerivative( 1.0E+05, 2.0E+05, true, -2.00E-01 ), -5.247083256421E-09, 1e-18);
   EXPECT_NEAR( porosity.calculateDerivative( 1.0E+05, 2.0E+05, true, -3.00E-01 ), 0.0                , 1e-18);
   EXPECT_NEAR( porosity.calculateDerivative( 1.0E+05, 2.0E+05, true, -4.00E-01 ), 0.0                , 1e-18);
}



#include <chrono>
TEST( soilMechanicsPorosity, derivativesVec_testVesLoading )
{
   soilMechanicsPorosity porosity(0.4, 0.03, 0.222, 1.50E+00 );
   const unsigned int N = 8;
   double* ves = AlignedMemoryAllocator<double, 32>::allocate ( N );
   ves[0] = 0.0;
   ves[1] = 1.0E+02;
   ves[2] = 1.0E+06;
   ves[3] = 1.0E+07;
   ves[4] = 0.0;
   ves[5] = 1.0E+02;
   ves[6] = 1.0E+06;
   ves[7] = 1.0E+07;
   double* maxVes = AlignedMemoryAllocator<double, 32>::allocate ( N );
   maxVes[0] = 0.0;
   maxVes[1] = 1.0E+02;
   maxVes[2] = 1.0E+06;
   maxVes[3] = 1.0E+07;
   maxVes[4] = 0.0;
   maxVes[5] = 1.0E+02;
   maxVes[6] = 1.0E+06;
   maxVes[7] = 1.0E+07;
   double* chemComp = AlignedMemoryAllocator<double, 32>::allocate ( N );
   chemComp[0] = 0.0;
   chemComp[1] = 0.0;
   chemComp[2] = 0.0;
   chemComp[3] = 0.0;
   chemComp[4] = 0.0;
   chemComp[5] = 0.0;
   chemComp[6] = 0.0;
   chemComp[7] = 0.0;
   double* poro = AlignedMemoryAllocator<double, 32>::allocate ( N );
   double* poroDer = AlignedMemoryAllocator<double, 32>::allocate ( N );

   porosity.calculate( N, ves, maxVes, false, chemComp, poro, poroDer );
   EXPECT_NEAR( 0.0                , poroDer[0], 1.0E-18 );
   EXPECT_NEAR( 0.0                , poroDer[1], 1.0E-18 );
   EXPECT_NEAR( 0.0                , poroDer[2], 1.0E-18 );
   EXPECT_NEAR( -1.016736732597E-08, poroDer[3], 1.0E-18 );
   EXPECT_NEAR( 0.0                , poroDer[4], 1.0E-18 );
   EXPECT_NEAR( 0.0                , poroDer[5], 1.0E-18 );
   EXPECT_NEAR( 0.0                , poroDer[6], 1.0E-18 );
   EXPECT_NEAR( -1.016736732597E-08, poroDer[7], 1.0E-18 );

   AlignedMemoryAllocator<double, 32>::free( ves );
   AlignedMemoryAllocator<double, 32>::free( maxVes );
   AlignedMemoryAllocator<double, 32>::free( chemComp );
   AlignedMemoryAllocator<double, 32>::free( poro );
   AlignedMemoryAllocator<double, 32>::free( poroDer );
}


TEST( soilMechanicsPorosity, derivativesVec_testVesUnLoading )
{
   soilMechanicsPorosity porosity(0.6, 0.03, 0.222, 1.50E+00 );
   const unsigned int N = 4;
   double* ves = AlignedMemoryAllocator<double, 32>::allocate ( N );
   ves[0] = 1.0E+01;
   ves[1] = 1.0E+03;
   ves[2] = 1.0E+05;
   ves[3] = 1.0E+06;
   double* maxVes = AlignedMemoryAllocator<double, 32>::allocate ( N );
   maxVes[0] = 2.0E+01;
   maxVes[1] = 2.0E+03;
   maxVes[2] = 2.0E+05;
   maxVes[3] = 2.0E+06;
   double* chemComp = AlignedMemoryAllocator<double, 32>::allocate ( N );
   chemComp[0] = 0.0;
   chemComp[1] = 0.0;
   chemComp[2] = 0.0;
   chemComp[3] = 0.0;
   double* poro = AlignedMemoryAllocator<double, 32>::allocate ( N );
   double* poroDer = AlignedMemoryAllocator<double, 32>::allocate ( N );

   porosity.calculate( N, ves, maxVes, false, chemComp, poro, poroDer );
   EXPECT_NEAR( 0.0                , poroDer[0], 1.0E-18 );
   EXPECT_NEAR( 0.0                , poroDer[1], 1.0E-18 );
   EXPECT_NEAR( -5.247083256421E-09, poroDer[2], 1.0E-18 );
   EXPECT_NEAR( -1.526049886416E-09, poroDer[3], 1.0E-18 );

   AlignedMemoryAllocator<double, 32>::free( ves );
   AlignedMemoryAllocator<double, 32>::free( maxVes );
   AlignedMemoryAllocator<double, 32>::free( chemComp );
   AlignedMemoryAllocator<double, 32>::free( poro );
   AlignedMemoryAllocator<double, 32>::free( poroDer );
}


TEST( soilMechanicsPorosity, derivativesVec_testChemCompLoading )
{
   soilMechanicsPorosity porosity(0.6, 0.03, 0.222, 1.50E+00 );
   const unsigned int N = 4;
   double* ves = AlignedMemoryAllocator<double, 32>::allocate ( N );
   ves[0] = 1.0E+01;
   ves[1] = 1.0E+03;
   ves[2] = 1.0E+05;
   ves[3] = 1.0E+06;
   double* maxVes = AlignedMemoryAllocator<double, 32>::allocate ( N );
   maxVes[0] = 2.0E+01;
   maxVes[1] = 2.0E+03;
   maxVes[2] = 2.0E+05;
   maxVes[3] = 2.0E+06;
   double* chemComp = AlignedMemoryAllocator<double, 32>::allocate ( N );
   chemComp[0] = 0.0;
   chemComp[1] = 0.0;
   chemComp[2] = 0.0;
   chemComp[3] = 0.0;
   double* poro = AlignedMemoryAllocator<double, 32>::allocate ( N );
   double* poroDer = AlignedMemoryAllocator<double, 32>::allocate ( N );

   porosity.calculate( N, ves, maxVes, false, chemComp, poro, poroDer );
   EXPECT_NEAR( 0.0                , poroDer[0], 1.0E-18 );
   EXPECT_NEAR( 0.0                , poroDer[1], 1.0E-18 );
   EXPECT_NEAR( -5.247083256421E-09, poroDer[2], 1.0E-18 );
   EXPECT_NEAR( -1.526049886416E-09, poroDer[3], 1.0E-18 );

   AlignedMemoryAllocator<double, 32>::free( ves );
   AlignedMemoryAllocator<double, 32>::free( maxVes );
   AlignedMemoryAllocator<double, 32>::free( chemComp );
   AlignedMemoryAllocator<double, 32>::free( poro );
   AlignedMemoryAllocator<double, 32>::free( poroDer );
}




TEST( soilMechanicsPorosity, derivativesVec_testChemCompUnLoading )
{
   soilMechanicsPorosity porosity(0.3, 0.03, 0.222, 1.50E+00 );
   const unsigned int N = 4;
   double* ves = AlignedMemoryAllocator<double, 32>::allocate ( N );
   ves[0] = 1.0E+05;
   ves[1] = 1.0E+05;
   ves[2] = 1.0E+05;
   ves[3] = 1.0E+05;
   double* maxVes = AlignedMemoryAllocator<double, 32>::allocate ( N );
   maxVes[0] = 2.0E+05;
   maxVes[1] = 2.0E+05;
   maxVes[2] = 2.0E+05;
   maxVes[3] = 2.0E+05;
   double* chemComp = AlignedMemoryAllocator<double, 32>::allocate ( N );
   chemComp[0] = -1.00E-01;
   chemComp[1] = -2.00E-01;
   chemComp[2] = -3.00E-01;
   chemComp[3] = -4.00E-01;
   double* poro = AlignedMemoryAllocator<double, 32>::allocate ( N );
   double* poroDer = AlignedMemoryAllocator<double, 32>::allocate ( N );

   porosity.calculate( N, ves, maxVes, true, chemComp, poro, poroDer );
   EXPECT_NEAR( -5.247083256421E-09, poroDer[0], 1.0E-18 );
   EXPECT_NEAR( -5.247083256421E-09, poroDer[1], 1.0E-18 );
   EXPECT_NEAR( 0.0                , poroDer[2], 1.0E-18 );
   EXPECT_NEAR( 0.0                , poroDer[3], 1.0E-18 );

   AlignedMemoryAllocator<double, 32>::free( ves );
   AlignedMemoryAllocator<double, 32>::free( maxVes );
   AlignedMemoryAllocator<double, 32>::free( chemComp );
   AlignedMemoryAllocator<double, 32>::free( poro );
   AlignedMemoryAllocator<double, 32>::free( poroDer );
}
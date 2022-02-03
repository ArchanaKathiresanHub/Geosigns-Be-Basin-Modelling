//
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "../src/ExponentialPorosity.h"
#include "AlignedMemoryAllocator.h"

#include <iostream>
#include <iomanip>
#include <cstring>
#include <cassert>
#include <cmath>
#include <limits>

#include <gtest/gtest.h>

using namespace GeoPhysics;

TEST( ExponentialPorosity, Sandstone )
{

   EXPECT_NEAR( 3.12603267213E-01, ExponentialPorosity( 0.39, 0.03, 2.66E-07, 2.66E-08, false ).calculate( 1E+5, 1.0E+6, false, 0 ), 1E-10 );
   
   EXPECT_NEAR( 4.68187798755E-01, ExponentialPorosity( 0.48, 0.03, 2.66E-07, 2.66E-08, false ).calculate( 1E+5, 1.0E+5, false, 0 ), 1E-10 );
   
   EXPECT_NEAR( 8.06549198802E-02, ExponentialPorosity( 0.60, 0.03, 2.66E-07, 2.66E-08, false ).calculate( 1E+6, 1.0E+7, false, 0 ), 1E-10 );
   
   EXPECT_NEAR( 6.98704863944E-02, ExponentialPorosity( 0.60, 0.03, 2.66E-07, 2.66E-08, false ).calculate( 1E+7, 1.0E+7, true, 0 ),  1E-10 );
   
}

TEST( ExponentialPorosity, Shale )
{

   EXPECT_NEAR( 6.438799650567E-01, ExponentialPorosity( 0.7, 0.03, 9.61300E-08, 9.61300E-09, false ).calculate( 1E+5, 1.0E+6, false, 0 ), 1E-10 );
   
   EXPECT_NEAR( 6.444456492845E-01, ExponentialPorosity( 0.65, 0.03, 8.99900E-08, 8.99900E-09, false ).calculate( 1E+5, 1.0E+5, false, 0 ), 1E-10 );

   EXPECT_NEAR( 3.063140763029E-01, ExponentialPorosity( 0.67, 0.03, 9.23000E-08, 9.23000E-09, false ).calculate( 1.00E+06, 1.0E+7, false, 0 ), 1E-10 );
   
   EXPECT_NEAR( 2.545678104070E-01, ExponentialPorosity( 0.7, 0.03, 1.09310E-07, 1.09310E-08, false ).calculate( 1E+7, 1.0E+7, true, 0 ), 1E-10 );
   
}


TEST( ExponentialPorosity, DerivativesExactValues )
{
   // loading phase, no chemical compaction, variations wrt ves, compactionIncr = 2.66E-07, compactionDecr = 2.66E-08
   EXPECT_NEAR( -9.842000000000E-08, ExponentialPorosity( 0.4, 0.03, 2.66E-07, 2.66E-08, false ).calculateDerivative( 0.0, 0.0, false, 0 ), 1.0E-18 );
   EXPECT_NEAR( -9.841973820314E-08, ExponentialPorosity( 0.4, 0.03, 2.66E-07, 2.66E-08, false ).calculateDerivative( 1.00E+01, 1.00E+01, false, 0 ), 1.0E-18 );
   EXPECT_NEAR( -9.841738206281E-08, ExponentialPorosity( 0.4, 0.03, 2.66E-07, 2.66E-08, false ).calculateDerivative( 1.00E+02, 1.00E+02, false, 0 ), 1.0E-18 );
   EXPECT_NEAR( -9.839382376159E-08, ExponentialPorosity( 0.4, 0.03, 2.66E-07, 2.66E-08, false ).calculateDerivative( 1.00E+03, 1.00E+03, false, 0 ), 1.0E-18 );
   EXPECT_NEAR( -9.815855068175E-08, ExponentialPorosity( 0.4, 0.03, 2.66E-07, 2.66E-08, false ).calculateDerivative( 1.00E+04, 1.00E+04, false, 0 ), 1.0E-18 );
   EXPECT_NEAR( -9.583654034106E-08, ExponentialPorosity( 0.4, 0.03, 2.66E-07, 2.66E-08, false ).calculateDerivative( 1.00E+05, 1.00E+05, false, 0 ), 1.0E-18 );
   EXPECT_NEAR( -7.543293892865E-08, ExponentialPorosity( 0.4, 0.03, 2.66E-07, 2.66E-08, false ).calculateDerivative( 1.00E+06, 1.00E+06, false, 0 ), 1.0E-18 );
   EXPECT_NEAR( -6.884303984108E-09, ExponentialPorosity( 0.4, 0.03, 2.66E-07, 2.66E-08, false ).calculateDerivative( 1.00E+07, 1.00E+07, false, 0 ), 1.0E-18 );
   
   // unloading phase, no chemical compaction, variations wrt ves, compactionIncr = 2.66E-07, compactionDecr = 2.66E-08
   EXPECT_NEAR( -9.841950258657E-09, ExponentialPorosity( 0.4, 0.03, 2.66E-07, 2.66E-08, false ).calculateDerivative( 1.00E+01, 2.00E+01, false, 0 ), 1.0E-18 );
   EXPECT_NEAR( -9.841502597889E-09, ExponentialPorosity( 0.4, 0.03, 2.66E-07, 2.66E-08, false ).calculateDerivative( 1.00E+02, 2.00E+02, false, 0 ), 1.0E-18 );
   EXPECT_NEAR( -9.837027109955E-09, ExponentialPorosity( 0.4, 0.03, 2.66E-07, 2.66E-08, false ).calculateDerivative( 1.00E+03, 2.00E+03, false, 0 ), 1.0E-18 );
   EXPECT_NEAR( -9.792384017199E-09, ExponentialPorosity( 0.4, 0.03, 2.66E-07, 2.66E-08, false ).calculateDerivative( 1.00E+04, 2.00E+04, false, 0 ), 1.0E-18 );
   EXPECT_NEAR( -9.356945880672E-09, ExponentialPorosity( 0.4, 0.03, 2.66E-07, 2.66E-08, false ).calculateDerivative( 1.00E+05, 2.00E+05, false, 0 ), 1.0E-18 );
   EXPECT_NEAR( -5.937326467716E-09, ExponentialPorosity( 0.4, 0.03, 2.66E-07, 2.66E-08, false ).calculateDerivative( 1.00E+06, 2.00E+06, false, 0 ), 1.0E-18 );
   EXPECT_NEAR( -6.282884111203E-11, ExponentialPorosity( 0.4, 0.03, 2.66E-07, 2.66E-08, false ).calculateDerivative( 1.00E+07, 2.00E+07, false, 0 ), 1.0E-18 );

   // loading phase, no chemical compaction, variations wrt ves, compactionIncr = 9.23E-08, compactionDecr = 9.23E-09
   EXPECT_NEAR( -3.415099999999E-08, ExponentialPorosity( 0.4, 0.03, 9.23E-08, 9.23E-09, false ).calculateDerivative( 0.0, 0.0, false, 0 ), 1.0E-18 );
   EXPECT_NEAR( -3.415096847864E-08, ExponentialPorosity( 0.4, 0.03, 9.23E-08, 9.23E-09, false ).calculateDerivative( 1.00E+01, 1.00E+01, false, 0 ), 1.0E-18 );
   EXPECT_NEAR( -3.415068478772E-08, ExponentialPorosity( 0.4, 0.03, 9.23E-08, 9.23E-09, false ).calculateDerivative( 1.00E+02, 1.00E+02, false, 0 ), 1.0E-18 );
   EXPECT_NEAR( -3.414784800816E-08, ExponentialPorosity( 0.4, 0.03, 9.23E-08, 9.23E-09, false ).calculateDerivative( 1.00E+03, 1.00E+03, false, 0 ), 1.0E-18 );
   EXPECT_NEAR( -3.411949316963E-08, ExponentialPorosity( 0.4, 0.03, 9.23E-08, 9.23E-09, false ).calculateDerivative( 1.00E+04, 1.00E+04, false, 0 ), 1.0E-18 );
   EXPECT_NEAR( -3.383723651601E-08, ExponentialPorosity( 0.4, 0.03, 9.23E-08, 9.23E-09, false ).calculateDerivative( 1.00E+05, 1.00E+05, false, 0 ), 1.0E-18 );
   EXPECT_NEAR( -3.113995957280E-08, ExponentialPorosity( 0.4, 0.03, 9.23E-08, 9.23E-09, false ).calculateDerivative( 1.00E+06, 1.00E+06, false, 0 ), 1.0E-18 );
   EXPECT_NEAR( -1.356905548376E-08, ExponentialPorosity( 0.4, 0.03, 9.23E-08, 9.23E-09, false ).calculateDerivative( 1.00E+07, 1.00E+07, false, 0 ), 1.0E-18 );
   
   // unloading phase, no chemical compaction, variations wrt ves, compactionIncr = 9.23000E-08, compactionDecr = 9.23000E-09
   EXPECT_NEAR( -3.415094010944E-09, ExponentialPorosity( 0.4, 0.03, 9.23E-08, 9.23E-09, false ).calculateDerivative( 1.00E+01, 2.00E+01, false, 0 ), 1.0E-18 );
   EXPECT_NEAR( -3.415040109916E-09, ExponentialPorosity( 0.4, 0.03, 9.23E-08, 9.23E-09, false ).calculateDerivative( 1.00E+02, 2.00E+02, false, 0 ), 1.0E-18 );
   EXPECT_NEAR( -3.414501146425E-09, ExponentialPorosity( 0.4, 0.03, 9.23E-08, 9.23E-09, false ).calculateDerivative( 1.00E+03, 2.00E+03, false, 0 ), 1.0E-18 );
   EXPECT_NEAR( -3.409116187569E-09, ExponentialPorosity( 0.4, 0.03, 9.23E-08, 9.23E-09, false ).calculateDerivative( 1.00E+04, 2.00E+04, false, 0 ), 1.0E-18 );
   EXPECT_NEAR( -3.355731485657E-09, ExponentialPorosity( 0.4, 0.03, 9.23E-08, 9.23E-09, false ).calculateDerivative( 1.00E+05, 2.00E+05, false, 0 ), 1.0E-18 );
   EXPECT_NEAR( -2.865769140860E-09, ExponentialPorosity( 0.4, 0.03, 9.23E-08, 9.23E-09, false ).calculateDerivative( 1.00E+06, 2.00E+06, false, 0 ), 1.0E-18 );
   EXPECT_NEAR( -5.912636665155E-10, ExponentialPorosity( 0.4, 0.03, 9.23E-08, 9.23E-09, false ).calculateDerivative( 1.00E+07, 2.00E+07, false, 0 ), 1.0E-18 );

   // loading phase, chemical compaction, variations wrt chemical compaction, compactionIncr = 2.66E-07, compactionDecr = 2.66E-08
   EXPECT_NEAR( -9.839382376159E-08, ExponentialPorosity( 0.4, 0.03, 2.66E-07, 2.66E-08, false ).calculateDerivative( 1.00E+03, 1.00E+03, true, 0. ), 1.0E-18 );
   EXPECT_NEAR( -9.839382376159E-08, ExponentialPorosity( 0.4, 0.03, 2.66E-07, 2.66E-08, false ).calculateDerivative( 1.00E+03, 1.00E+03, true, -5.0E-2 ), 1.0E-18 );
   EXPECT_NEAR( -9.839382376159E-08, ExponentialPorosity( 0.4, 0.03, 2.66E-07, 2.66E-08, false ).calculateDerivative( 1.00E+03, 1.00E+03, true, -1.0E-1 ), 1.0E-18 );
   EXPECT_NEAR( -9.839382376159E-08, ExponentialPorosity( 0.4, 0.03, 2.66E-07, 2.66E-08, false ).calculateDerivative( 1.00E+03, 1.00E+03, true, -1.5E-1 ), 1.0E-18 );
   EXPECT_NEAR( -9.839382376159E-08, ExponentialPorosity( 0.4, 0.03, 2.66E-07, 2.66E-08, false ).calculateDerivative( 1.00E+03, 1.00E+03, true, -2.0E-1 ), 1.0E-18 );
   EXPECT_NEAR( -9.839382376159E-08, ExponentialPorosity( 0.4, 0.03, 2.66E-07, 2.66E-08, false ).calculateDerivative( 1.00E+03, 1.00E+03, true, -2.5E-1 ), 1.0E-18 );
   EXPECT_NEAR( -9.839382376159E-08, ExponentialPorosity( 0.4, 0.03, 2.66E-07, 2.66E-08, false ).calculateDerivative( 1.00E+03, 1.00E+03, true, -3.0E-1 ), 1.0E-18 );
   EXPECT_NEAR( -9.839382376159E-08, ExponentialPorosity( 0.4, 0.03, 2.66E-07, 2.66E-08, false ).calculateDerivative( 1.00E+03, 1.00E+03, true, -3.5E-1 ), 1.0E-18 );
   EXPECT_NEAR( 0., ExponentialPorosity( 0.4, 0.03, 2.66E-07, 2.66E-08, false ).calculateDerivative( 1.00E+03, 1.00E+03, true, -4.0E-1 ), 1.0E-18 );
}


#include <chrono>
TEST( ExponentialPorosity, DerivativesLoadingVec )
{
   // loading phase, no chemical compaction, variations wrt ves, compactionIncr = 2.66E-07, compactionDecr = 2.66E-08
   Porosity porosity( Porosity::create(DataAccess::Interface::EXPONENTIAL_POROSITY, 0.4, 0.03, 2.66E-07, 0.0, 0.0, 2.66E-08, 0.0, 0.0, 0.0, 0.0, false ));
   const unsigned int N = 12;
   double* ves = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( N );
   ves[0 ] = 0.0;
   ves[1 ] = 1.00E+01;
   ves[2 ] = 1.00E+02;
   ves[3 ] = 1.00E+03;
   ves[4 ] = 1.00E+04;
   ves[5 ] = 1.00E+05;
   ves[6 ] = 1.00E+06;
   ves[7 ] = 1.00E+07;
   ves[8 ] = 0.0;
   ves[9 ] = 1.00E+01;
   ves[10] = 1.00E+02;
   ves[11] = 1.00E+03;
   double* maxVes = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( N );
   maxVes[0 ] = 0.0;
   maxVes[1 ] = 1.00E+01;
   maxVes[2 ] = 1.00E+02;
   maxVes[3 ] = 1.00E+03;
   maxVes[4 ] = 1.00E+04;
   maxVes[5 ] = 1.00E+05;
   maxVes[6 ] = 1.00E+06;
   maxVes[7 ] = 1.00E+07;
   maxVes[8 ] = 0.0;
   maxVes[9 ] = 1.00E+01;
   maxVes[10] = 1.00E+02;
   maxVes[11] = 1.00E+03;
   double* chemComp = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( N );
   chemComp[0 ] = 0.0;
   chemComp[1 ] = 0.0;
   chemComp[2 ] = 0.0;
   chemComp[3 ] = 0.0;
   chemComp[4 ] = 0.0;
   chemComp[5 ] = 0.0;
   chemComp[6 ] = 0.0;
   chemComp[7 ] = 0.0;
   chemComp[8 ] = 0.0;
   chemComp[9 ] = 0.0;
   chemComp[10] = 0.0;
   chemComp[11] = 0.0;
   double* poro = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( N );
   double* poroDer = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( N );

   porosity.calculate( N, ves, maxVes, false, chemComp, poro, poroDer );
   EXPECT_NEAR( -9.842000000000E-08, poroDer[0 ], 1.0E-18 );
   EXPECT_NEAR( -9.841973820314E-08, poroDer[1 ], 1.0E-18 );
   EXPECT_NEAR( -9.841738206281E-08, poroDer[2 ], 1.0E-18 );
   EXPECT_NEAR( -9.839382376159E-08, poroDer[3 ], 1.0E-18 );
   EXPECT_NEAR( -9.815855068175E-08, poroDer[4 ], 1.0E-18 );
   EXPECT_NEAR( -9.583654034106E-08, poroDer[5 ], 1.0E-18 );
   EXPECT_NEAR( -7.543293892865E-08, poroDer[6 ], 1.0E-18 );
   EXPECT_NEAR( -6.884303984108E-09, poroDer[7 ], 1.0E-18 );
   EXPECT_NEAR( -9.842000000000E-08, poroDer[8 ], 1.0E-18 );
   EXPECT_NEAR( -9.841973820314E-08, poroDer[9 ], 1.0E-18 );
   EXPECT_NEAR( -9.841738206281E-08, poroDer[10], 1.0E-18 );
   EXPECT_NEAR( -9.839382376159E-08, poroDer[11], 1.0E-18 );

   AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::free( ves );
   AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::free( maxVes );
   AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::free( chemComp );
   AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::free( poro );
   AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::free( poroDer );
}


TEST( ExponentialPorosity, DerivativesUnLoadingVec )
{
   // unloading phase, no chemical compaction, variations wrt ves, compactionIncr = 2.66E-07, compactionDecr = 2.66E-08
   Porosity porosity( Porosity::create(DataAccess::Interface::EXPONENTIAL_POROSITY, 0.4, 0.03, 2.66E-07, 0.0, 0.0, 2.66E-08, 0.0, 0.0, 0.0, 0.0, false ));
   const unsigned int N = 6;
   double* ves = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( N );
   ves[0] = 1.00E+01;
   ves[1] = 1.00E+02;
   ves[2] = 1.00E+03;
   ves[3] = 1.00E+04;
   ves[4] = 1.00E+05;
   ves[5] = 1.00E+06;
   double* maxVes = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( N );
   maxVes[0] = 2.00E+01;
   maxVes[1] = 2.00E+02;
   maxVes[2] = 2.00E+03;
   maxVes[3] = 2.00E+04;
   maxVes[4] = 2.00E+05;
   maxVes[5] = 2.00E+06;
   double* chemComp = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( N );
   chemComp[0] = 0.0;
   chemComp[1] = 0.0;
   chemComp[2] = 0.0;
   chemComp[3] = 0.0;
   chemComp[4] = 0.0;
   chemComp[5] = 0.0;
   double* poro = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( N );
   double* poroDer = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( N );

   porosity.calculate( N, ves, maxVes, false, chemComp, poro, poroDer );
   EXPECT_NEAR( -9.841950258657E-09, poroDer[0], 1.0E-18 );
   EXPECT_NEAR( -9.841502597889E-09, poroDer[1], 1.0E-18 );
   EXPECT_NEAR( -9.837027109955E-09, poroDer[2], 1.0E-18 );
   EXPECT_NEAR( -9.792384017199E-09, poroDer[3], 1.0E-18 );
   EXPECT_NEAR( -9.356945880672E-09, poroDer[4], 1.0E-18 );
   EXPECT_NEAR( -5.937326467716E-09, poroDer[5], 1.0E-18 );

   AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::free( ves );
   AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::free( maxVes );
   AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::free( chemComp );
   AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::free( poro );
   AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::free( poroDer );
}


TEST( ExponentialPorosity, DerivativesChemCompVec )
{
   Porosity porosity( Porosity::create(DataAccess::Interface::EXPONENTIAL_POROSITY, 0.4, 0.03, 2.66E-07, 0.0, 0.0, 2.66E-08, 0.0, 0.0, 0.0, 0.0, false ));
   const unsigned int N = 9;
   double* ves = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( N );
   ves[0] = 1.00E+03;
   ves[1] = 1.00E+03;
   ves[2] = 1.00E+03;
   ves[3] = 1.00E+03;
   ves[4] = 1.00E+03;
   ves[5] = 1.00E+03;
   ves[6] = 1.00E+03;
   ves[7] = 1.00E+03;
   ves[8] = 1.00E+03;
   double* maxVes = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( N );
   maxVes[0] = 1.00E+03;
   maxVes[1] = 1.00E+03;
   maxVes[2] = 1.00E+03;
   maxVes[3] = 1.00E+03;
   maxVes[4] = 1.00E+03;
   maxVes[5] = 1.00E+03;
   maxVes[6] = 1.00E+03;
   maxVes[7] = 1.00E+03;
   maxVes[8] = 1.00E+03;
   double* chemComp = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( N );
   chemComp[0] = 0.0;
   chemComp[1] = -5.0E-2;
   chemComp[2] = -1.0E-1;
   chemComp[3] = -1.5E-1;
   chemComp[4] = -2.0E-1;
   chemComp[5] = -2.5E-1;
   chemComp[6] = -3.0E-1;
   chemComp[7] = -3.5E-1;
   chemComp[8] = -4.0E-1;
   double* poro = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( N );
   double* poroDer = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( N );

   porosity.calculate( N, ves, maxVes, true, chemComp, poro, poroDer );
   EXPECT_NEAR( -9.839382376159E-08, poroDer[0], 1.0E-18 );
   EXPECT_NEAR( -9.839382376159E-08, poroDer[1], 1.0E-18 );
   EXPECT_NEAR( -9.839382376159E-08, poroDer[2], 1.0E-18 );
   EXPECT_NEAR( -9.839382376159E-08, poroDer[3], 1.0E-18 );
   EXPECT_NEAR( -9.839382376159E-08, poroDer[4], 1.0E-18 );
   EXPECT_NEAR( -9.839382376159E-08, poroDer[5], 1.0E-18 );
   EXPECT_NEAR( -9.839382376159E-08, poroDer[6], 1.0E-18 );
   EXPECT_NEAR( -9.839382376159E-08, poroDer[7], 1.0E-18 );
   EXPECT_NEAR( 0.0                , poroDer[8], 1.0E-18 );

   AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::free( ves );
   AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::free( maxVes );
   AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::free( chemComp );
   AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::free( poro );
   AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::free( poroDer );
}

TEST(ExponentialPorosity, near_zero_porosity_and_non_zero_compaction_coefficient)
{
   double compactionIncr = 0.04e-6;
   double compactionDecr = 0.04e-7;
   double depoPoro = 1.e-8;

   {//non legacy mode    
      Porosity porExp(Porosity::create(DataAccess::Interface::EXPONENTIAL_POROSITY,
         depoPoro, 0.0, compactionIncr, 0.0, 0.0, compactionDecr, 0.0, 0.0, 0.0, 0.0, false));
      double maxVes = 50.0e6;
      double ves = maxVes;       //for loading
      EXPECT_NEAR(0.0, porExp.calculate(ves, maxVes, false, 0.0), 1e-10);
      ves = maxVes / 2.0;          //for unloading
      EXPECT_NEAR(0.0, porExp.calculate(ves, maxVes, false, 0.0), 1e-10);
   }

   {//legacy mode    
      Porosity porExp(Porosity::create(DataAccess::Interface::EXPONENTIAL_POROSITY,
         depoPoro, 0.0, compactionIncr, 0.0, 0.0, compactionDecr, 0.0, 0.0, 0.0, 0.0, true));
      double maxVes = 50.0e6;
      double ves = maxVes;       //for loading
      EXPECT_NEAR(0.0, porExp.calculate(ves, maxVes, false, 0.0), 1e-10);
      ves = maxVes / 2.0;          //for unloading
      EXPECT_NEAR(0.0, porExp.calculate(ves, maxVes, false, 0.0), 1e-10);
   }

}

TEST(ExponentialPorosity, fullCompThickness){
    //Considering the properties for Sandstone,typical from BPA2 Lithology Ref. catalog
    double compactionIncr = 3.25e-08;
    double compactionDecr = 3.25e-09;
    double depoPoro = 0.45;//Surface porosity
    double minMechPor = 0.05;
    bool isLegacy = false;
    double fctvalueNonLegacy1, fctvalueLegacy1, fctvalueNonLegacy2, fctvalueLegacy2, fctvalueNonLegacy3, fctvalueLegacy3, maxVes, thickness, densityDifference;
    maxVes = 50.0e6;
    thickness = 300.0;
    densityDifference = 1250.0;
    //NonLegacy mode
    {
        Porosity porExp(Porosity::create(DataAccess::Interface::EXPONENTIAL_POROSITY,
            depoPoro, minMechPor, compactionIncr, 0.0, 0.0, compactionDecr, 0.0, 0.0, 0.0, 0.0, isLegacy));
        
        fctvalueNonLegacy1 = porExp.getFullCompThickness(maxVes, thickness, densityDifference, 0.5, false);//for dendityDifference>0
        EXPECT_NEAR(262.56311895391127, fctvalueNonLegacy1, 1e-10);

        densityDifference = 0.0;
        fctvalueNonLegacy2 = porExp.getFullCompThickness(maxVes, thickness, densityDifference, 0.5, false);//for dendityDifference==0
        EXPECT_NEAR(165.0, fctvalueNonLegacy2, 1e-10);

        Porosity porExp1(Porosity::create(DataAccess::Interface::EXPONENTIAL_POROSITY,
            depoPoro, 0.0, compactionIncr, 0.0, 0.0, compactionDecr, 0.0, 0.0, 0.0, 0.0, isLegacy));//With MinMechPor=0.0

        fctvalueNonLegacy3 = porExp1.getFullCompThickness(maxVes, thickness, 1250.0, 0.5, false);//for dendityDifference>0
        EXPECT_NEAR(274.57339596513577, fctvalueNonLegacy3, 1e-10);
    }
    //LegacyMode
    isLegacy = true;
    {
        Porosity porExp(Porosity::create(DataAccess::Interface::EXPONENTIAL_POROSITY,
            depoPoro, minMechPor, compactionIncr, 0.0, 0.0, compactionDecr, 0.0, 0.0, 0.0, 0.0, isLegacy));
   
        densityDifference = 1250.0;
        fctvalueLegacy1 = porExp.getFullCompThickness(maxVes, thickness, densityDifference, 0.5, false);//for dendityDifference>0
        EXPECT_NEAR(274.81863578217718, fctvalueLegacy1, 1e-10);

        Porosity porExp1(Porosity::create(DataAccess::Interface::EXPONENTIAL_POROSITY,
            depoPoro, 0.0, compactionIncr, 0.0, 0.0, compactionDecr, 0.0, 0.0, 0.0, 0.0, isLegacy));//With MinMechPor=0.0
        fctvalueLegacy3 = porExp1.getFullCompThickness(maxVes, thickness, densityDifference, 0.5, false);
        EXPECT_NEAR(274.81863578217718, fctvalueLegacy3, 1e-10);

        densityDifference = 0.0;
        fctvalueLegacy2 = porExp.getFullCompThickness(maxVes, thickness, densityDifference, 0.5, false);//for dendityDifference==0
        EXPECT_NEAR(165.0, fctvalueLegacy2, 1e-10);
    }
    EXPECT_GT( fctvalueLegacy1, fctvalueNonLegacy1);
    EXPECT_DOUBLE_EQ(fctvalueLegacy2, fctvalueNonLegacy2);
    EXPECT_NE(fctvalueNonLegacy3, fctvalueLegacy3);//Even though the mi_MechPor=0.0 (from the P3d file is same) for both the two cases
}

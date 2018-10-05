//
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include "../src/GeoPhysicalConstants.h"
#include "../src/DoubleExponentialPorosity.h"
#include "../src/ExponentialPorosity.h"
#include "AlignedMemoryAllocator.h"

#include <gtest/gtest.h>
#include <algorithm>
#include <cmath>
#include <iostream>


using namespace GeoPhysics;

TEST( DoubleExponentialPorosity, sub_arkose )
{
   // with a typical sub-arkose
   Porosity porosity( Porosity::create(DataAccess::Interface::DOUBLE_EXPONENTIAL_POROSITY, 0.41, MinimumPorosityNonLegacy, 0.0, 1.631e-8, 3.874e-8, 0.0, 0.1631e-8, 0.3874e-8, 0.5, 0.0, false ));

   //For different values of ves
   EXPECT_NEAR( 0.41 , porosity.calculate( 0.0e6, 0.0e6,false, 0.0 ), 1e-10);
   EXPECT_NEAR( 0.35797349509959642, porosity.calculate( 5.0e6, 5.0e6, false, 0.0 ), 1e-10 );
   EXPECT_NEAR( 0.12095024636588726, porosity.calculate( 5.0e7, 5.0e7, false, 0.0 ), 1e-10 );
   EXPECT_NEAR( 0.0010587526774138807, porosity.calculate( 5.0e8, 5.0e8, false, 0.0 ), 1e-10 );
   
   //For different values of chemical compaction
   EXPECT_NEAR( 0.12095024636588726, porosity.calculate( 5.0e7, 5.0e7, true, 0.0 ), 1e-10 );
   EXPECT_NEAR(MinimumPorosityNonLegacy, porosity.calculate( 5.0e7, 5.0e7, true, -1.0 ), 1e-10);

   //If there is a minimumMechanicalPorosity
   Porosity porosity2( Porosity::create(DataAccess::Interface::DOUBLE_EXPONENTIAL_POROSITY, 0.41, 0.05, 0.0, 1.631e-8, 3.874e-8, 0.0, 0.1631e-8, 0.3874e-8, 0.5, 0.0, false ));

   EXPECT_NEAR( 0.05 , porosity2.calculate( 5.0e10, 5.0e10,false, 0.0 ), 1e-10);
   //If there is both mechanical and chemical => chemical only is taken into account
   EXPECT_NEAR(MinimumPorosityNonLegacy, porosity2.calculate( 5.0e7, 5.0e7,true, -1.0 ), 1e-10);
   
}

TEST( DoubleExponentialPorosity, extreme_coefficients_and_porosity )
{
   // with a typical chalk
   Porosity typicalChalk( Porosity::create(DataAccess::Interface::DOUBLE_EXPONENTIAL_POROSITY, 0.70, MinimumPorosityNonLegacy, 0.0, 5.000e-6, 0.045e-6, 0.0, 5.000e-7, 0.045e-7, 0.5, 0.0, false ));
   EXPECT_NEAR( 0.037837028984371586, typicalChalk.calculate( 50.0e6, 50.0e6, false, 0.0 ), 1e-10 );
   
   // with an ooid grainstone limestone
   Porosity ooidGraistone( Porosity::create(DataAccess::Interface::DOUBLE_EXPONENTIAL_POROSITY, 0.35, MinimumPorosityNonLegacy, 0.0, 0.001e-6, 0.001e-6, 0.0, 0.001e-7, 0.001e-7, 0.5, 0.0, false ));
   EXPECT_NEAR( 0.34825935523824608, ooidGraistone.calculate( 5.0e6, 5.0e6, false, 0.0 ), 1e-10 );
   
   //with nul deposition porosity
   Porosity nulPorosity( Porosity::create(DataAccess::Interface::DOUBLE_EXPONENTIAL_POROSITY, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 5.000e-7, 0.045e-7, 0.5, 0.0, false ));
   EXPECT_NEAR( 0.0 , nulPorosity.calculate( 50.0e6, 50.0e6,false, 0.0 ), 1e-10);
      
   //with nul coefficients => no compaction
   Porosity nulCoefficients( Porosity::create(DataAccess::Interface::DOUBLE_EXPONENTIAL_POROSITY, 0.70, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.5, 0.0, false ));
   EXPECT_NEAR( 0.70 , nulCoefficients.calculate( 50.0e6, 50.0e6,false, 0.0 ), 1e-10);
   
   //with extreme coefficients => porosity = minimum porosity
   Porosity extremeCoefficients( Porosity::create(DataAccess::Interface::DOUBLE_EXPONENTIAL_POROSITY, 0.70, 0.05, 0.0, 1.0, 1.0, 0.0, 0.1, 0.1, 0.5, 0.0, false ));
   EXPECT_NEAR( 0.05 , extremeCoefficients.calculate( 50.0e6, 50.0e6,false, 0.0 ), 1e-10);
      
      
}


TEST( DoubleExponentialPorosity, elastic_rebound )
{
   // with a typical sub-arkose
   Porosity elasticRebound( Porosity::create(DataAccess::Interface::DOUBLE_EXPONENTIAL_POROSITY, 0.41, MinimumPorosityNonLegacy, 0.0, 1.631e-8, 3.874e-8, 0.0, 0.1631e-8, 0.3874e-8, 0.5, 0.0, false ));
   double withoutRebound = elasticRebound.calculate( 40.0e6, 40.0e6, false, 0.0 );
   double withoutRebound2 = elasticRebound.calculate( 50.0e6, 50.0e6, false, 0.0 );
   double withRebound = elasticRebound.calculate( 40.0e6, 50.0e6, false, 0.0 );
   
   //test the relations
   EXPECT_GT(withoutRebound, withRebound);
   EXPECT_GT(withRebound, withoutRebound2);
   
   //and test the values
   EXPECT_NEAR( 0.15092452450331825, withoutRebound, 1e-10 );
   EXPECT_NEAR( 0.12095024636588726, withoutRebound2, 1e-10 );
   EXPECT_NEAR( 0.12360228237210288, withRebound, 1e-10 );
}

TEST( DoubleExponentialPorosity, single_is_special_case_double )
{
   //typical sandstone
   Porosity singleExp( Porosity::create(DataAccess::Interface::EXPONENTIAL_POROSITY, 0.41, 0.0, 0.0266e-6, 0.0, 0.0, 0.0266e-7, 0.0, 0.0, 0.0, 0.0, false ));
   Porosity doubleExp( Porosity::create(DataAccess::Interface::DOUBLE_EXPONENTIAL_POROSITY, 0.41, 0.0, 0.0, 0.0266e-6, 0.0266e-6, 0.0, 0.0266e-7, 0.0266e-7, 0.5, 0.0, false ));
   
   EXPECT_NEAR( doubleExp.calculate( 50.0e6, 50.0e6,false, 0.0 ), singleExp.calculate( 50.0e6, 50.0e6,false, 0.0 ), 1e-10);
   EXPECT_NEAR( doubleExp.calculate( 40.0e6, 50.0e6,false, 0.0 ), singleExp.calculate( 40.0e6, 50.0e6,false, 0.0 ), 1e-10);
}


TEST( DoubleExponentialPorosity, derivatives_testDepoPoro )
{
   // For different depositional porosities
   DoubleExponentialPorosity doubleExp1(0.0, 0.0, 2.66E-07, 1.0E-07, 2.66E-08, 1.0E-08, 0.5, false );
   EXPECT_NEAR( doubleExp1.calculateDerivative( 1.0E+04, 1.0E+04, false, 0.0 ), 0.0, 1e-18);

   DoubleExponentialPorosity doubleExp2(0.2, MinimumPorosityNonLegacy, 2.66E-07, 1.0E-07, 2.66E-08, 1.0E-08, 0.5, false );
   EXPECT_NEAR( doubleExp2.calculateDerivative( 1.0E+04, 1.0E+04, false, 0.0 ), -3.6336746305326871E-08, 1e-18 );

   DoubleExponentialPorosity doubleExp3(0.6, MinimumPorosityNonLegacy, 2.66E-07, 1.0E-07, 2.66E-08, 1.0E-08, 0.5, false );
   EXPECT_NEAR( doubleExp3.calculateDerivative( 1.0E+04, 1.0E+04, false, 0.0 ), -1.0937543234618491E-07, 1e-18 );
}


TEST( DoubleExponentialPorosity, derivatives_testMinMecPoro )
{
   // For different minimum mechanical porosities
   DoubleExponentialPorosity doubleExp1(0.4, MinimumPorosityNonLegacy, 2.66E-07, 1.0E-07, 2.66E-08, 1.0E-08, 0.5, false );
   EXPECT_NEAR( doubleExp1.calculateDerivative( 1.0E+04, 1.0E+04, false, 0.0 ), -7.2856089325755894E-08, 1e-18 );

   DoubleExponentialPorosity doubleExp2(0.4, 0.4, 2.66E-07, 1.0E-07, 2.66E-08, 1.0E-08, 0.5, false );
   EXPECT_NEAR( doubleExp2.calculateDerivative( 1.0E+04, 1.0E+04, false, 0.0 ), 0.0, 1e-18);

   DoubleExponentialPorosity doubleExp3(0.4, 0.6, 2.66E-07, 1.0E-07, 2.66E-08, 1.0E-08, 0.5, false );
   EXPECT_NEAR( doubleExp3.calculateDerivative( 1.0E+04, 1.0E+04, false, 0.0 ), 0.0, 1e-18);
}


TEST( DoubleExponentialPorosity, derivatives_testLoading )
{
   // For different ves (loading)
   DoubleExponentialPorosity doubleExp(0.4, 0.3, 2.66E-07, 1.0E-07, 2.66E-08, 1.0E-08, 0.5, false );
   EXPECT_NEAR( doubleExp.calculateDerivative( 0.0    , 0.0    , false, 0.0 ), -1.830000000000E-08, 1e-18);
   EXPECT_NEAR( doubleExp.calculateDerivative( 1.0E+01, 1.0E+01, false, 0.0 ), -1.829995962205E-08, 1e-18);
   EXPECT_NEAR( doubleExp.calculateDerivative( 1.0E+02, 1.0E+02, false, 0.0 ), -1.829959622496E-08, 1e-18);
   EXPECT_NEAR( doubleExp.calculateDerivative( 1.0E+03, 1.0E+03, false, 0.0 ), -1.829596269548E-08, 1e-18);
   EXPECT_NEAR( doubleExp.calculateDerivative( 1.0E+04, 1.0E+04, false, 0.0 ), -1.825967151021E-08, 1e-18);
   EXPECT_NEAR( doubleExp.calculateDerivative( 1.0E+05, 1.0E+05, false, 0.0 ), -1.790113299862E-08, 1e-18);
   EXPECT_NEAR( doubleExp.calculateDerivative( 1.0E+06, 1.0E+06, false, 0.0 ), -1.471782748594E-08, 1e-18);
}


TEST( DoubleExponentialPorosity, derivatives_testUnloading )
{
   // For different ves (unloading)
   DoubleExponentialPorosity doubleExp(0.4, 0.3, 2.66E-07, 1.0E-07, 2.66E-08, 1.0E-08, 0.5, false );
   EXPECT_NEAR( doubleExp.calculateDerivative( 1.0E+01, 2.0E+01, false, 0.0 ), -1.829992328198E-09, 1e-18);
   EXPECT_NEAR( doubleExp.calculateDerivative( 1.0E+02, 2.0E+02, false, 0.0 ), -1.829923283589E-09, 1e-18);
   EXPECT_NEAR( doubleExp.calculateDerivative( 1.0E+03, 2.0E+03, false, 0.0 ), -1.829232996856E-09, 1e-18);
   EXPECT_NEAR( doubleExp.calculateDerivative( 1.0E+04, 2.0E+04, false, 0.0 ), -1.822346039388E-09, 1e-18);
   EXPECT_NEAR( doubleExp.calculateDerivative( 1.0E+05, 2.0E+05, false, 0.0 ), -1.755041827158E-09, 1e-18);
   EXPECT_NEAR( doubleExp.calculateDerivative( 1.0E+06, 2.0E+06, false, 0.0 ), -1.215820981528E-09, 1e-18);
}


TEST( DoubleExponentialPorosity, derivatives_testChemComp )
{
   // For different chemical compaction terms
   DoubleExponentialPorosity doubleExp(0.4, 0.3, 2.66E-07, 1.0E-07, 2.66E-08, 1.0E-08, 0.5, false );
   EXPECT_NEAR( doubleExp.calculateDerivative( 1.0E+03, 1.0E+03, true, -1.00E-01 ), -3.6591925390969709E-08, 1e-18 );
   EXPECT_NEAR( doubleExp.calculateDerivative( 1.0E+03, 1.0E+03, true, -4.00E-01 ), 0.0                    , 1e-18);
}

#include <chrono>
TEST( DoubleExponentialPorosity, derivativesVec_testLoading )
{
   // For different ves (loading)
   Porosity porosity( Porosity::create(DataAccess::Interface::DOUBLE_EXPONENTIAL_POROSITY, 0.4, 0.3, 0.0, 2.66E-07, 1.0E-07, 0.0, 2.66E-08, 1.0E-08, 0.5, 0.0, false ));
   const unsigned int N = 6;
   double* ves = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( N );
   ves[0] = 0.0;
   ves[1] = 1.00E+01;
   ves[2] = 1.00E+02;
   ves[3] = 1.00E+03;
   ves[4] = 1.00E+04;
   ves[5] = 1.00E+05;
   double* maxVes = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( N );
   maxVes[0] = 0.0;
   maxVes[1] = 1.00E+01;
   maxVes[2] = 1.00E+02;
   maxVes[3] = 1.00E+03;
   maxVes[4] = 1.00E+04;
   maxVes[5] = 1.00E+05;
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
   EXPECT_NEAR( poroDer[0], -1.830000000000E-08, 1e-18);
   EXPECT_NEAR( poroDer[1], -1.829995962205E-08, 1e-18);
   EXPECT_NEAR( poroDer[2], -1.829959622496E-08, 1e-18);
   EXPECT_NEAR( poroDer[3], -1.829596269548E-08, 1e-18);
   EXPECT_NEAR( poroDer[4], -1.825967151021E-08, 1e-18);
   EXPECT_NEAR( poroDer[5], -1.790113299862E-08, 1e-18);

   AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::free( ves );
   AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::free( maxVes );
   AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::free( chemComp );
   AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::free( poro );
   AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::free( poroDer );
}


TEST( DoubleExponentialPorosity, derivativesVec_testUnLoading )
{
   // For different ves (unloading)
   Porosity porosity( Porosity::create(DataAccess::Interface::DOUBLE_EXPONENTIAL_POROSITY, 0.4, 0.3, 0.0, 2.66E-07, 1.0E-07, 0.0, 2.66E-08, 1.0E-08, 0.5, 0.0, false ));
   const unsigned int N = 6;
   double* ves = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( N );
   ves[0] = 1.0E+01;
   ves[1] = 1.0E+02;
   ves[2] = 1.0E+03;
   ves[3] = 1.0E+04;
   ves[4] = 1.0E+05;
   ves[5] = 1.0E+06;
   double* maxVes = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( N );
   maxVes[0] = 2.0E+01;
   maxVes[1] = 2.0E+02;
   maxVes[2] = 2.0E+03;
   maxVes[3] = 2.0E+04;
   maxVes[4] = 2.0E+05;
   maxVes[5] = 2.0E+06;
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
   EXPECT_NEAR( poroDer[0], -1.829992328198E-09, 1e-18);
   EXPECT_NEAR( poroDer[1], -1.829923283589E-09, 1e-18);
   EXPECT_NEAR( poroDer[2], -1.829232996856E-09, 1e-18);
   EXPECT_NEAR( poroDer[3], -1.822346039388E-09, 1e-18);
   EXPECT_NEAR( poroDer[4], -1.755041827158E-09, 1e-18);
   EXPECT_NEAR( poroDer[5], -1.215820981528E-09, 1e-18);

   AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::free( ves );
   AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::free( maxVes );
   AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::free( chemComp );
   AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::free( poro );
   AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::free( poroDer );
}




TEST( DoubleExponentialPorosity, derivativesVec_testChemComp )
{
   // For different chemical compaction terms
   DoubleExponentialPorosity doubleExp(0.4, 0.3, 2.66E-07, 1.0E-07, 2.66E-08, 1.0E-08, 0.5, false );
   EXPECT_NEAR( doubleExp.calculateDerivative( 1.0E+03, 1.0E+03, true, -1.00E-01 ), -3.6591925390969709E-08, 1e-18 );
   EXPECT_NEAR( doubleExp.calculateDerivative( 1.0E+03, 1.0E+03, true, -4.00E-01 ), 0.0                    , 1e-18);

   Porosity porosity( Porosity::create(DataAccess::Interface::DOUBLE_EXPONENTIAL_POROSITY, 0.4, 0.3, 0.0, 2.66E-07, 1.0E-07, 0.0, 2.66E-08, 1.0E-08, 0.5, 0.0, false ));
   const unsigned int N = 2;
   double* ves = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( N );
   ves[0] = 1.0E+03;
   ves[1] = 1.0E+03;
   double* maxVes = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( N );
   maxVes[0] = 1.0E+03;
   maxVes[1] = 1.0E+03;
   double* chemComp = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( N );
   chemComp[0] = -1.00E-01;
   chemComp[1] = -4.00E-01;
   double* poro = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( N );
   double* poroDer = AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::allocate ( N );

   porosity.calculate( N, ves, maxVes, true, chemComp, poro, poroDer );
   EXPECT_NEAR( poroDer[0], -3.6591925390969709E-08, 1e-18);
   EXPECT_NEAR( poroDer[1], 0.0                    , 1e-18);

   AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::free( ves );
   AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::free( maxVes );
   AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::free( chemComp );
   AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::free( poro );
   AlignedMemoryAllocator<double, ARRAY_ALIGNMENT>::free( poroDer );
}

TEST(DoubleExponentialPorosity, doubleExponential_is_weightedAverage_of_exponentialModel)
{
   double compactionIncrA = 0.04e-6;
   double compactionIncrB = 0.08e-6;
   double compactionDecrA = 0.04e-7;
   double compactionDecrB = 0.08e-7;
   double depoPoro = 0.5;

    Porosity singleExpA(Porosity::create(DataAccess::Interface::EXPONENTIAL_POROSITY,
       depoPoro, 0.0, compactionIncrA, 0.0, 0.0, compactionDecrA, 0.0, 0.0, 0.0, 0.0, false));
    Porosity singleExpB(Porosity::create(DataAccess::Interface::EXPONENTIAL_POROSITY,
       depoPoro, 0.0, compactionIncrB, 0.0, 0.0, compactionDecrB, 0.0, 0.0, 0.0, 0.0, false));

    double maxVes = 50.0e6;

    for (size_t i = 0; i < 11; i++)
    {
       double compacRatio = static_cast<double>(i) / 10.0;

       Porosity doubleExp(Porosity::create(DataAccess::Interface::DOUBLE_EXPONENTIAL_POROSITY,
          depoPoro, 0.0, 0.0, compactionIncrA, compactionIncrB, 0.0, compactionDecrA, compactionDecrB, compacRatio, 0.0, false));
       //check whether double exponential model is the weighted average of exponential models
       double ves = maxVes;       //for loading
       EXPECT_NEAR(((compacRatio*singleExpA.calculate(ves, maxVes, false, 0.0)) +
          ((1.0 - compacRatio)*singleExpB.calculate(ves, maxVes, false, 0.0))),
          doubleExp.calculate(ves, maxVes, false, 0.0), 1e-10);
       EXPECT_NEAR(((compacRatio*singleExpA.calculateDerivative(ves, maxVes, false, 0.0)) +
          ((1.0 - compacRatio)*singleExpB.calculateDerivative(ves, maxVes, false, 0.0))),
          doubleExp.calculateDerivative(ves, maxVes, false, 0.0), 1e-18);

       ves = maxVes / 2.0;          //for unloading
       EXPECT_NEAR(((compacRatio*singleExpA.calculate(ves, maxVes, false, 0.0)) +
          ((1.0 - compacRatio)*singleExpB.calculate(ves, maxVes, false, 0.0))),
          doubleExp.calculate(ves, maxVes, false, 0.0), 1e-10);
       EXPECT_NEAR(((compacRatio*singleExpA.calculateDerivative(ves, maxVes, false, 0.0)) +
          ((1.0 - compacRatio)*singleExpB.calculateDerivative(ves, maxVes, false, 0.0))),
          doubleExp.calculateDerivative(ves, maxVes, false, 0.0), 1e-18);
    }

}
//
// Copyright (C) 2012-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#ifdef _WIN32
#define _CRT_RAND_S 1
#define RandData unsigned int
#else
#define RandData struct drand48_data
#endif

#include <stdlib.h>

#include "../src/EosPackCAPI.h"
#include "../src/EosPack.h"
#include "PVTCfgFileMgr.h"

#include <fstream>
#include <iostream>
#include <numeric>

#include <cmath>


#include <gtest/gtest.h>

using namespace CBMGenerics;
using namespace pvtFlash;

static const int g_NumOfThreads = 10;
static const int g_NumFlashes = 100;

// Test of EosPack CAPI in multithred environments
class EosPackMTTest : public ::testing::Test
{
public:
   EosPackMTTest()  { PVTCfgFileMgr::instance("./PVT_properties_EPMT.cfg"); } // write cfg file
   ~EosPackMTTest() {;}

   void initializeCompositionMasses( double masses[] );
   double myRand( RandData * data );
};

TEST_F( EosPackMTTest, FlashVapour)
{  
   bool expVap = true;
   bool expLiq = true;

   // Start an OpenMP thread pool
   #pragma omp parallel num_threads(g_NumOfThreads) shared(expLiq, expVap)
   {
      ComputeStruct computeStruct;
      // Cricondentherm point plus some delta must give pure vapour phase
      computeStruct.pressure    = 1e6 * 3.12139 ;  // in Pa
      computeStruct.temperature = 682.255 + 1;     // in K

      initializeCompositionMasses( computeStruct.compMasses );

      computeStruct.isGormPrescribed = false;

      EosPackComputeWithLumping( &computeStruct );

      double sumVapour = std::accumulate( computeStruct.phaseCompMasses + PhaseId::VAPOUR       * ComponentId::NUMBER_OF_SPECIES,
                                          computeStruct.phaseCompMasses + (PhaseId::VAPOUR + 1 )* ComponentId::NUMBER_OF_SPECIES,
                                          0.0 );
      double sumLiquid = std::accumulate( computeStruct.phaseCompMasses + PhaseId::LIQUID       * ComponentId::NUMBER_OF_SPECIES,
                                          computeStruct.phaseCompMasses + (PhaseId::LIQUID + 1 )* ComponentId::NUMBER_OF_SPECIES,
                                          0.0 );
      // collect results for each thread
      #pragma omp critical
      {
         expVap = expVap && ((std::abs(sumVapour - 7426542) < 1e-6) ? true : false); // Vapour mass not as expected
         expLiq = expLiq && ((std::abs(sumLiquid -       0) < 1e-6) ? true : false); // Liquid mass not as expected
      }
   }
   EXPECT_TRUE( expVap && expLiq ); // check all resuts in one go
}


TEST_F( EosPackMTTest, FlashLiquid )
{
   bool expVap = true;
   bool expLiq = true;

   // Start an OpenMP thread pool
   #pragma omp parallel num_threads(g_NumOfThreads) shared(expLiq, expVap)
   {
      ComputeStruct computeStruct;
      // Cricondenbar point plus some delta must give pure liquid phase
      computeStruct.pressure    = 1e6 * (8.0467 + 0.001);  // in Pa
      computeStruct.temperature = 470.578;                 // in K

      initializeCompositionMasses( computeStruct.compMasses );

      computeStruct.isGormPrescribed = false;
      EosPackComputeWithLumping( &computeStruct);
    
      double sumVapour = std::accumulate( computeStruct.phaseCompMasses + PhaseId::VAPOUR       * ComponentId::NUMBER_OF_SPECIES,
                                          computeStruct.phaseCompMasses + (PhaseId::VAPOUR + 1 )* ComponentId::NUMBER_OF_SPECIES,
                                          0.0 );
      double sumLiquid = std::accumulate( computeStruct.phaseCompMasses + PhaseId::LIQUID       * ComponentId::NUMBER_OF_SPECIES,
                                          computeStruct.phaseCompMasses + (PhaseId::LIQUID + 1 )* ComponentId::NUMBER_OF_SPECIES,
                                          0.0 );
      // collect results for each thread
      #pragma omp critical
      {
         expVap = expVap && ((std::abs(sumVapour -      0 ) < 1e-6) ? true : false); // Vapour mass not as expected
         expLiq = expLiq && ((std::abs(sumLiquid - 7426542) < 1e-6) ? true : false); // Liquid mass not as expected
      }
   }
   EXPECT_TRUE( expVap && expLiq ); // check all resuts in one go
}

TEST_F( EosPackMTTest, StressTest )
{
   #pragma omp parallel num_threads(g_NumOfThreads)
   {
      ComputeStruct computeStruct;
      RandData buff;

      for ( int lp = 0; lp < g_NumFlashes; ++lp )
      {
         computeStruct.pressure    = 1.0e6 + myRand( &buff ) * 9.0e6;  // in Pa 1-10 MPa
         computeStruct.temperature = 400   + myRand( &buff ) * 600 ;   // in K 400-1000

         for ( size_t i = ComponentId::FIRST_COMPONENT; i < ComponentId::LAST_COMPONENT; ++i )
         {
            computeStruct.compMasses[i] = myRand( &buff );
         }
         EosPackComputeWithLumping( &computeStruct);
      }
   }
}

///////////////////////////////////////////////////////////
// Axillary functions
///////////////////////////////////////////////////////////
void EosPackMTTest::initializeCompositionMasses( double masses[] )
{
   masses[ ComponentId::ASPHALTENE       ] = 1158;
   masses[ ComponentId::RESIN            ] = 21116;
   masses[ ComponentId::C15_PLUS_ARO     ] = 2021;
   masses[ ComponentId::C15_PLUS_SAT     ] = 19731;
   masses[ ComponentId::C6_MINUS_14ARO   ] = 339;
   masses[ ComponentId::C6_MINUS_14SAT   ] = 6328815;
   masses[ ComponentId::C5               ] = 103238;
   masses[ ComponentId::C4               ] = 187596;
   masses[ ComponentId::C3               ] = 215881;
   masses[ ComponentId::C2               ] = 232280;
   masses[ ComponentId::C1               ] = 308969;
   masses[ ComponentId::COX              ] = 0;
   masses[ ComponentId::N2               ] = 5398;
   masses[ ComponentId::H2S              ] = 0;
   masses[ ComponentId::LSC              ] = 0;
   masses[ ComponentId::C15_PLUS_AT      ] = 0;
   masses[ ComponentId::C6_MINUS_14BT    ] = 0;
   masses[ ComponentId::C6_MINUS_14DBT   ] = 0;
   masses[ ComponentId::C6_MINUS_14BP    ] = 0;
   masses[ ComponentId::C15_PLUS_ARO_S   ] = 0;
   masses[ ComponentId::C15_PLUS_SAT_S   ] = 0;
   masses[ ComponentId::C6_MINUS_14SAT_S ] = 0;
   masses[ ComponentId::C6_MINUS_14ARO_S ] = 0;
}

double EosPackMTTest::myRand( RandData * data )
{
   double res;
#ifdef _WIN32
   unsigned int ddd = *data;
   rand_s( &ddd );
   res = static_cast<double>( *data )/static_cast<double>(UINT_MAX);
#else
   drand48_r( data, &res );
#endif
   return res;
}


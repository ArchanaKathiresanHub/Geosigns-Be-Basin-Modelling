#include "../src/EosPackCAPI.h"
#include "../src/EosPack.h"
#include "PVTCfgFileMgr.h"

#include <numeric>
#include <cmath>

#include <gtest/gtest.h>

using namespace CBMGenerics;
using namespace pvtFlash;

class EosPackTest : public ::testing::Test
{
public:
   EosPackTest()  { PVTCfgFileMgr::instance(); } // write cfg file
   ~EosPackTest() {;}

   void initializeCompositionMasses( double masses[] );
};
  

TEST_F( EosPackTest, InitialisationOfKValues )
{
   const double ComparisonTolerance = 1.0e-10;

   double pressure    = 39958145.0;  // in Pa
   double temperature = 353.50111;   // in K
   double compMasses[CBMGenerics::ComponentManager::NumberOfOutputSpecies];

   compMasses [ ASPHALTENES ] = 5.9322774e-05;
   compMasses [      RESINS ] = 0.00010834321;
   compMasses [     C15_ARO ] = 0.0046930211;
   compMasses [     C15_SAT ] = 0.015339124;
   compMasses [    C6_14ARO ] = 0.053148382;
   compMasses [    C6_14SAT ] = 0.073486113;
   compMasses [          C5 ] = 0.016210614;
   compMasses [          C4 ] = 0.02930958;
   compMasses [          C3 ] = 0.044801003;
   compMasses [          C2 ] = 0.066723203;
   compMasses [          C1 ] = 0.67665664;
   compMasses [         COX ] = 0;
   compMasses [          N2 ] = 0.019464658;
   compMasses [         H2S ] = 0;
   compMasses [         LSC ] = 0;
   compMasses [      C15_AT ] = 0;
   compMasses [     C6_14BT ] = 0;
   compMasses [    C6_14DBT ] = 0;
   compMasses [     C6_14BP ] = 0;
   compMasses [    C15_AROS ] = 0;
   compMasses [    C15_SATS ] = 0;
   compMasses [   C6_14SATS ] = 0;
   compMasses [   C6_14AROS ] = 0;


   double phaseMasses[CBMGenerics::ComponentManager::NumberOfPhases][CBMGenerics::ComponentManager::NumberOfOutputSpecies];

   double phaseDensity[CBMGenerics::ComponentManager::NumberOfPhases];
   double phaseViscosity[CBMGenerics::ComponentManager::NumberOfPhases];

   double kValuesComputed  [ CBMGenerics::ComponentManager::NumberOfSpeciesToFlash ];
   double kValuesExpected [ CBMGenerics::ComponentManager::NumberOfSpeciesToFlash ];
   double gorm = 0.0; // Initialised to stop the compiler from complaining.
                      // This is okay because isGormPrescribed is defined false, any gorm value will not be used.
   bool isGormPrescribed = false;

   // Indicate to EosPack that saving of k-values is required,
   kValuesComputed [ 0 ] = -1.0;

   pvtFlash::EosPack& instance = pvtFlash::EosPack::getInstance();
         
   bool result = instance.computeWithLumping ( temperature, 
                                               pressure, 
                                               compMasses,
                                               phaseMasses,
                                               phaseDensity, 
                                               phaseViscosity, 
                                               isGormPrescribed, 
                                               gorm,
                                               kValuesComputed );
    
   kValuesExpected [  0 ] = 6.60273155042658345e+03;
   kValuesExpected [  1 ] = 1.30765460986769649e+04;
   kValuesExpected [  2 ] = 1.73153010603771850e+03;
   kValuesExpected [  3 ] = 3.73738426515636775e+01;
   kValuesExpected [  4 ] = 5.20409170312865932e+00;
   kValuesExpected [  5 ] = 2.59238570551743397e+00;
   kValuesExpected [  6 ] = 1.99224092565726085e+00;
   kValuesExpected [  7 ] = 1.59796701407822672e+00;
   kValuesExpected [  8 ] = 1.21088348479272834e+00;
   kValuesExpected [  9 ] = 9.07181261374219283e-01;
   kValuesExpected [ 10 ] = 5.21685329290231370e-01;
   kValuesExpected [ 11 ] = 9.94000165289730897e-01;
   kValuesExpected [ 12 ] = 3.24734491437953743e-01;
   kValuesExpected [ 13 ] = 1.53531376868468827e+00;

   // The kvalues computed should be equal to those in the expected array.
   for ( int i = 0; i < CBMGenerics::ComponentManager::NumberOfSpeciesToFlash; ++i ) {
      EXPECT_NEAR( kValuesExpected [ i ], kValuesComputed [ i ],  kValuesExpected [ i ] * ComparisonTolerance );
   }
}
      
TEST_F( EosPackTest, FlashVapour)
{
    ComputeStruct computeStruct;
    // Cricondentherm point plus some delta must give pure vapour phase
    computeStruct.pressure    = 1e6 * 3.12139 ;  // in Pa
    computeStruct.temperature = 682.255 + 1;     // in K

    initializeCompositionMasses( computeStruct.compMasses );

    computeStruct.isGormPrescribed = false;
    EosPackComputeWithLumping( &computeStruct );

    double sumVapour = std::accumulate( computeStruct.phaseCompMasses + VAPOUR_PHASE       * NUM_COMPONENTS,
                                        computeStruct.phaseCompMasses + (VAPOUR_PHASE + 1 )* NUM_COMPONENTS,
                                        0.0 );
    double sumLiquid = std::accumulate( computeStruct.phaseCompMasses + LIQUID_PHASE       * NUM_COMPONENTS,
                                        computeStruct.phaseCompMasses + (LIQUID_PHASE + 1 )* NUM_COMPONENTS,
                                        0.0 );
    EXPECT_NEAR( sumVapour, 7426542, 1e-6 ); // Vapour mass not as expected
    EXPECT_NEAR( sumLiquid, 0, 1e-6);        // Liquid mass not as expected
}

TEST_F( EosPackTest, FlashLiquid )
{
    ComputeStruct computeStruct;
    // Cricondenbar point plus some delta must give pure liquid phase
    computeStruct.pressure    = 1e6 * (8.0467 + 0.001);  // in Pa
    computeStruct.temperature = 470.578;                 // in K

    initializeCompositionMasses( computeStruct.compMasses );

    computeStruct.isGormPrescribed = false;
    EosPackComputeWithLumping( &computeStruct);
    
    double sumVapour = std::accumulate( computeStruct.phaseCompMasses + VAPOUR_PHASE       * NUM_COMPONENTS,
                                        computeStruct.phaseCompMasses + (VAPOUR_PHASE + 1 )* NUM_COMPONENTS,
                                        0.0 );
    double sumLiquid = std::accumulate( computeStruct.phaseCompMasses + LIQUID_PHASE       * NUM_COMPONENTS,
                                        computeStruct.phaseCompMasses + (LIQUID_PHASE + 1 )* NUM_COMPONENTS,
                                        0.0 );
    EXPECT_NEAR( sumVapour, 0, 1e-6 );       // Vapour mass not as expected
    EXPECT_NEAR( sumLiquid, 7426542, 1e-6 ); // Liquid mass not as expected
}

TEST_F( EosPackTest, FlashVapourLiquidWarmer)
{
   ComputeStruct computeStruct;
   computeStruct.pressure    = 1e6 * 1;       // in Pa
   computeStruct.temperature = 273.15 + 290;  // in K

   initializeCompositionMasses( computeStruct.compMasses );

   computeStruct.isGormPrescribed = false;
   EosPackComputeWithLumping( &computeStruct );

   double sumVapour = std::accumulate( computeStruct.phaseCompMasses + VAPOUR_PHASE       * NUM_COMPONENTS,
                                       computeStruct.phaseCompMasses + (VAPOUR_PHASE + 1 )* NUM_COMPONENTS,
                                       0.0 );
   double sumLiquid = std::accumulate( computeStruct.phaseCompMasses + LIQUID_PHASE       * NUM_COMPONENTS,
                                       computeStruct.phaseCompMasses + (LIQUID_PHASE + 1 )* NUM_COMPONENTS,
                                       0.0 );
   EXPECT_NEAR( sumVapour,  7400659.1, 1e-1 );   // Vapour mass not as expected
   EXPECT_NEAR( sumLiquid ,  25882.915, 1e-3 ); // Liquid mass not as expected
}

TEST_F( EosPackTest, FlashVapourLiquid)
{
   ComputeStruct computeStruct;
   computeStruct.pressure    = 1e6 * 1;       // in Pa
   computeStruct.temperature = 273.15 + 100;  // in K

   initializeCompositionMasses( computeStruct.compMasses );

   computeStruct.isGormPrescribed = false;
   EosPackComputeWithLumping( &computeStruct );
    
   double sumVapour = std::accumulate( computeStruct.phaseCompMasses + VAPOUR_PHASE       * NUM_COMPONENTS,
                                       computeStruct.phaseCompMasses + (VAPOUR_PHASE + 1 )* NUM_COMPONENTS,
                                       0.0 );
   double sumLiquid = std::accumulate( computeStruct.phaseCompMasses + LIQUID_PHASE       * NUM_COMPONENTS,
                                       computeStruct.phaseCompMasses + (LIQUID_PHASE + 1 )* NUM_COMPONENTS,
                                       0.0 );
   EXPECT_NEAR( sumVapour, 851393.6564469377, 1e-6  ); // Vapour mass not as expected
   EXPECT_NEAR( sumLiquid, 6575148.3435530625, 1e-6 ); // Liquid mass not as expected
}

TEST_F( EosPackTest, FlashVapourLiquidUsingArrays )
{
   double masses[NUM_COMPONENTS];
   double phaseMasses[NUM_COMPONENTS * N_PHASES];
   double phaseDensity[N_PHASES];
   double phaseViscosity[N_PHASES];

   initializeCompositionMasses( masses );

   EosPackComputeWithLumpingArr( 373.15, 1e6, masses, false, 0.0, phaseMasses, phaseDensity, phaseViscosity );

   double totPhaseMass[N_PHASES];

   for ( int i = 0; i < N_PHASES; ++i )
   {
      totPhaseMass[i] = 0.0;
      for ( int j = 0; j < NUM_COMPONENTS; ++j )
      {
         totPhaseMass[i] += phaseMasses[i * NUM_COMPONENTS + j];
      }
   }
   double sumVapour = totPhaseMass[VAPOUR_PHASE];
   double sumLiquid = totPhaseMass[LIQUID_PHASE];

   EXPECT_NEAR( sumVapour, 851393.6564469377, 1e-6 );  // Vapour mass not as expected
   EXPECT_NEAR( sumLiquid, 6575148.3435530625, 1e-6 ); // Liquid mass not as expected
}

TEST_F( EosPackTest, GormCalculation)
{
   double compos[NUM_COMPONENTS];
   for (int i = 0; i < NUM_COMPONENTS; ++i )
   {
      compos[i] = 1;
   }
   double gorm = Gorm(compos);
   EXPECT_NEAR( gorm, 6.0/15.0, 1e-6 ); // Gorm not as expected
}


TEST_F( EosPackTest, GetMolWeight  )
{
   EXPECT_NEAR( GetMolWeight( ASPHALTENES, 0.0 ),   795.12, 1.e-5 );
   EXPECT_NEAR( GetMolWeight( RESINS     , 0.0 ),   618.35, 1.e-5 );
   EXPECT_NEAR( GetMolWeight( C15_ARO    , 0.0 ),   474.52, 1.e-5 );
   EXPECT_NEAR( GetMolWeight( C15_SAT    , 0.0 ),   281.85, 1.e-5 );
   EXPECT_NEAR( GetMolWeight( C6_14ARO   , 0.0 ),   158.47, 1.e-5 );
   EXPECT_NEAR( GetMolWeight( C6_14SAT   , 0.0 ),   103.09, 1.e-5 );
   EXPECT_NEAR( GetMolWeight( C5         , 0.0 ), 72.15064, 1.e-5 );
   EXPECT_NEAR( GetMolWeight( C4         , 0.0 ),  58.1237, 1.e-5 );
   EXPECT_NEAR( GetMolWeight( C3         , 0.0 ), 44.09676, 1.e-5 );
   EXPECT_NEAR( GetMolWeight( C2         , 0.0 ), 30.06982, 1.e-5 );
   EXPECT_NEAR( GetMolWeight( C1         , 0.0 ), 16.04288, 1.e-5 );
   EXPECT_NEAR( GetMolWeight( COX        , 0.0 ),  44.0098, 1.e-5 );
   EXPECT_NEAR( GetMolWeight( N2         , 0.0 ), 28.01352, 1.e-5 );
   EXPECT_NEAR( GetMolWeight( H2S        , 0.0 ),    34.08, 1.e-5 );
   EXPECT_NEAR( GetMolWeight( LSC        , 0.0 ),   281.85, 1.e-5 );
   EXPECT_NEAR( GetMolWeight( C15_AT     , 0.0 ),   281.85, 1.e-5 );
   EXPECT_NEAR( GetMolWeight( C6_14BT    , 0.0 ),   158.47, 1.e-5 );
   EXPECT_NEAR( GetMolWeight( C6_14DBT   , 0.0 ),   158.47, 1.e-5 );
   EXPECT_NEAR( GetMolWeight( C6_14BP    , 0.0 ),   158.47, 1.e-5 );
   EXPECT_NEAR( GetMolWeight( C15_AROS   , 0.0 ),   281.85, 1.e-5 );
   EXPECT_NEAR( GetMolWeight( C15_SATS   , 0.0 ),   281.85, 1.e-5 );
   EXPECT_NEAR( GetMolWeight( C6_14SATS  , 0.0 ),   158.47, 1.e-5 );
   EXPECT_NEAR( GetMolWeight( C6_14AROS  , 0.0 ),   158.47, 1.e-5 );
   
   EXPECT_NEAR( GetMolWeight( ASPHALTENES, 2.3 ),  801.5255, 1.e-5 );
   EXPECT_NEAR( GetMolWeight( RESINS     , 2.3 ), 600.43116, 1.e-5 );
   EXPECT_NEAR( GetMolWeight( C15_ARO    , 2.3 ),  448.9233, 1.e-5 );
   EXPECT_NEAR( GetMolWeight( C15_SAT    , 2.3 ),  242.3038, 1.e-5 );
   EXPECT_NEAR( GetMolWeight( C6_14ARO   , 2.3 ), 153.74304, 1.e-5 );
   EXPECT_NEAR( GetMolWeight( C6_14SAT   , 2.3 ),   101.779, 1.e-5 );
   EXPECT_NEAR( GetMolWeight( C5         , 2.3 ),  72.15064, 1.e-5 );
   EXPECT_NEAR( GetMolWeight( C4         , 2.3 ),   58.1237, 1.e-5 );
   EXPECT_NEAR( GetMolWeight( C3         , 2.3 ),  44.09676, 1.e-5 );
   EXPECT_NEAR( GetMolWeight( C2         , 2.3 ),  30.06982, 1.e-5 );
   EXPECT_NEAR( GetMolWeight( C1         , 2.3 ),  16.04288, 1.e-5 );
   EXPECT_NEAR( GetMolWeight( COX        , 2.3 ),   44.0098, 1.e-5 );
   EXPECT_NEAR( GetMolWeight( N2         , 2.3 ),  28.01352, 1.e-5 );
   EXPECT_NEAR( GetMolWeight( H2S        , 2.3 ),     34.08, 1.e-5 );
   EXPECT_NEAR( GetMolWeight( LSC        , 2.3 ),  242.3038, 1.e-5 );
   EXPECT_NEAR( GetMolWeight( C15_AT     , 2.3 ),  242.3038, 1.e-5 );
   EXPECT_NEAR( GetMolWeight( C6_14BT    , 2.3 ), 153.74304, 1.e-5 );
   EXPECT_NEAR( GetMolWeight( C6_14DBT   , 2.3 ), 153.74304, 1.e-5 );
   EXPECT_NEAR( GetMolWeight( C6_14BP    , 2.3 ), 153.74304, 1.e-5 );
   EXPECT_NEAR( GetMolWeight( C15_AROS   , 2.3 ),  242.3038, 1.e-5 );
   EXPECT_NEAR( GetMolWeight( C15_SATS   , 2.3 ),  242.3038, 1.e-5 );
   EXPECT_NEAR( GetMolWeight( C6_14SATS  , 2.3 ), 153.74304, 1.e-5 );
   EXPECT_NEAR( GetMolWeight( C6_14AROS  , 2.3 ), 153.74304, 1.e-5 );
}

///////////////////////////////////////////////////////////
// Axillary functions
///////////////////////////////////////////////////////////
void EosPackTest::initializeCompositionMasses( double masses[] )
{
   masses[ASPHALTENES] = 1158;
   masses[RESINS     ] = 21116;
   masses[C15_ARO    ] = 2021;
   masses[C15_SAT    ] = 19731;
   masses[C6_14ARO   ] = 339;
   masses[C6_14SAT   ] = 6328815;
   masses[C5         ] = 103238;
   masses[C4         ] = 187596;
   masses[C3         ] = 215881;
   masses[C2         ] = 232280;
   masses[C1         ] = 308969;
   masses[COX        ] = 0;
   masses[N2         ] = 5398;
   masses[H2S        ] = 0;
   masses[LSC        ] = 0;
   masses[C15_AT     ] = 0;
   masses[C6_14BT    ] = 0;
   masses[C6_14DBT   ] = 0;
   masses[C6_14BP    ] = 0;
   masses[C15_AROS   ] = 0;
   masses[C15_SATS   ] = 0;
   masses[C6_14SATS  ] = 0;
   masses[C6_14AROS  ] = 0;
}



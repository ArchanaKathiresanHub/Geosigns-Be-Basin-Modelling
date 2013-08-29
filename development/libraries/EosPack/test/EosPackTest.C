#include "../src/EosPackCAPI.h"
#include "../src/EosPack.h"

#include <fstream>
#include <iostream>
#include <numeric>

#include <cmath>

#include <gtest/gtest.h>

using namespace CBMGenerics;
using namespace pvtFlash;

class EosPackTest : public ::testing::Test
{
public:
   EosPackTest()
   {
      PVTPropertiesCfgFile::getInstance();               // Write configuration file
      pvtFlash::SetPvtPropertiesConfigFile( s_CfgFile ); // Set configuration file name
   }

   void initializeCompositionMasses( double masses[] );

private:
   static char * const s_CfgFile ;
   
   class PVTPropertiesCfgFile
   {
   public:
      static PVTPropertiesCfgFile & getInstance()
      {
         static PVTPropertiesCfgFile o;
         return o;
      }

   private:
      PVTPropertiesCfgFile();
      ~PVTPropertiesCfgFile(){ std::remove( s_CfgFile ); }
   };
};
  
char * const EosPackTest:: s_CfgFile = "./PVT_properties.cfg" ;


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

// Creates PVT_properties.cfg file in current folder

EosPackTest::PVTPropertiesCfgFile::PVTPropertiesCfgFile()
{
   std::ofstream ofs( s_CfgFile, std::ios_base::out | std::ios_base::trunc );
   ofs << "///component based and general data for PVT" << "\n";
   ofs << "///" << "\n";
   ofs << "///This file contains tables describing 6 COMPONENT-based properties and additionally GENERAL properties " << "\n";
   ofs << "///for PVT (see table headers)." << "\n";
   ofs << "///" << "\n";
   ofs << "///All tables have comma-separated columns. " << "\n";
   ofs << "///" << "\n";
   ofs << "///COMPONENT TABLES:" << "\n";
   ofs << "///" << "\n";
   ofs << "///Every table with component data contains the component names in the first column followed by the property columns." << "\n";
   ofs << "///" << "\n";
   ofs << "///The order of the 6 component property columns must be maintained. " << "\n";
   ofs << "///However, it is possible to split the component tables, i.e. one can use one table containing all component property columns," << "\n";
   ofs << "///or use up to 6 separate tables." << "\n";
   ofs << "///" << "\n";
   ofs << "///GENERAL TABLES:" << "\n";
   ofs << "///" << "\n";
   ofs << "///General data must be contained in one table." << "\n";
   ofs << "///" << "\n";
   ofs << "///All (numerical) data entries to be prescribed (either component or general data) " << "\n";
   ofs << "///are generally interpreted as a piecewise polynomial depending on the " << "\n";
   ofs << "///GORM (mass based gas/oil ratio), i.e. every data entry may be either: " << "\n";
   ofs << "///a constant value," << "\n";
   ofs << "///a polynomial (e.g. 2.0 + 4.0x^3 + 3.4x)," << "\n";
   ofs << "///a piecewise polynomial with an arbitrary number of semicolon separated pieces, each consisting " << "\n";
   ofs << "///of a range [x_a:x_b] followed by a constant or polynomial." << "\n";
   ofs << "///Example:" << "\n";
   ofs << "///[*:1.2]   1.7e-05 ; [1.2:1.8]   -2.7E-05x + 5.1E-05 ;   [1.8:*]   0.0" << "\n";
   ofs << "///(in the above example, the '*'s indicate positive/negative infinity)" << "\n";
   ofs << "" << "\n";
   ofs << "Table:[PVT-Component-Properties]" << "\n";
   ofs << "ComponentName,MolecularWeight,AcentricFactor,VCrit" << "\n";
   ofs << "N2,         28.01352	         ,   0.04000   ,   8.980e-02" << "\n";
   ofs << "COx,        44.00980	         ,   0.22500   ,   9.400e-02" << "\n";
   ofs << "C1,         16.04288	         ,   0.00800   ,   9.900e-02" << "\n";
   ofs << "C2,         30.06982	         ,   0.09800   ,   1.480e-01" << "\n";
   ofs << "C3,         44.09676	         ,   0.15200   ,   2.030e-01" << "\n";
   ofs << "C4,         58.12370	         ,   0.19300   ,   2.550e-01" << "\n";
   ofs << "C5,         72.15064	         ,   0.25100   ,   3.040e-01" << "\n";
   ofs << "H2S,        34.08000	         ,   0.10000   ,   9.850e-02" << "\n";
   ofs << "C6-14Sat,   [0.0:2.5]   103.09 -   0.57x  ;   [2.5:*] 101.665,   0.37272   ,   4.7060e-01" << "\n";
   ofs << "C6-14Aro,   [0.0:2.5]   158.47 - 2.0552x  ;   [2.5:*] 153.332,   0.54832   ,   6.7260e-01" << "\n";
   ofs << "C15+Sat,    [0.0:2.5]   281.85 - 17.194x  ;   [2.5:*] 238.865,   0.86828   ,   1.19620e+00" << "\n";
   ofs << "C15+Aro,    [0.0:2.5]   474.52 - 11.129x  ;   [2.5:*] 446.6975,   1.14380   ,   2.12800e+00" << "\n";
   ofs << "resins,     [0.0:2.5]   618.35 - 7.7908x  ;   [2.5:*] 598.873,   1.16772   ,   2.89200e+00" << "\n";
   ofs << "asphaltenes,[0.0:2.5]   795.12 +  2.785x  ;   [2.5:*] 802.0825,   0.89142   ,   3.84400e+00" << "\n";
   ofs << "EndOfTable" << "\n";
   ofs << "" << "\n";
   ofs << "Table:[PVT-Component-Properties]" << "\n";
   ofs << "ComponentName,VolumeShift" << "\n";
   ofs << "N2,                           -4.2300000E-03" << "\n";
   ofs << "COx,                          -1.6412000E-03" << "\n";
   ofs << "C1,                           -5.2000000E-03" << "\n";
   ofs << "C2,                           -5.7900010E-03" << "\n";
   ofs << "C3,                           -6.3499980E-03" << "\n";
   ofs << "C4,                           -6.4900000E-03" << "\n";
   ofs << "C5,                           -5.1200000E-03" << "\n";
   ofs << "H2S,                          -3.840000E-03" << "\n";
   ofs << "C6-14Sat,    [0.0:1.242]   1.737999554e-02 ; [1.242:1.8694]   -2.770163E-02x + 5.178542E-02 ;   [1.8694:*]   0.0   " << "\n";
   ofs << "C6-14Aro,    [0.0:1.242]   2.214000542e-02 ; [1.242:1.8694]   -3.528849E-02x + 6.596831E-02 ;   [1.8694:*]   0.0  " << "\n";
   ofs << "C15+Sat,     [0.0:1.242]   2.573999668e-02 ; [1.242:1.8694]   -4.102646E-02x + 7.669486E-02 ;   [1.8694:*]   0.0  " << "\n";
   ofs << "C15+Aro,                                     [0.0:2.5]        -3.687332E-02x + 1.244384E-03 ;   [2.5:*] -9.09389E-02" << "\n";
   ofs << "resins,                                      [0.0:2.5]         -4.229062E-02x - 3.814919E-02;   [2.5:*] -1.43876E-01" << "\n";
   ofs << "asphaltenes,                                 [0.0:2.5]        -5.542006E-02x - 9.001542E-02 ;   [2.5:*] -2.28566E-01" << "\n";
   ofs << "EndOfTable" << "\n";
   ofs << "" << "\n";
   ofs << "Table:[PVT-Component-Properties]" << "\n";
   ofs << "ComponentName,CritPressure" << "\n";
   ofs << "N2,            3394387" << "\n";
   ofs << "COx,           7376462" << "\n";
   ofs << "C1,            4600155" << "\n";
   ofs << "C2,            4883865" << "\n";
   ofs << "C3,            4245519" << "\n";
   ofs << "C4,            3799688" << "\n";
   ofs << "C5,            3374122" << "\n";
   ofs << "H2S,           8936864" << "\n";
   ofs << "C6-14Sat,      [0.0:2.0]   2.81e+06 + 8.71e+04x  +  8.24e+05x^2  -  1.15e+06x^3 +  3.68e+05x^4; [2.0:*]  2.97e+06" << "\n";
   ofs << "C6-14Aro,      [0.0:2.0]   2.07e+06 + 2.51e+05x  +  5.79e+05x^2  -  1.07e+06x^3 +  3.85e+05x^4; [2.0:*]  2.49e+06" << "\n";
   ofs << "C15+Sat,       [0.0:2.0]   1.64e+06 - 2.66e+05x  +  1.27e+06x^2  -  1.48e+06x^3 +  4.83e+05x^4; [2.0:*]  2.08e+06" << "\n";
   ofs << "C15+Aro,       [0.0:2.0]   1.34e+06 - 1.85e+05x  +  1.26e+06x^2  -  1.57e+06x^3 +  5.22e+05x^4; [2.0:*]  1.80e+06" << "\n";
   ofs << "resins,        [0.0:2.0]   1.29e+06 - 2.98e+05x  +  1.43e+06x^2  -  1.67e+06x^3 +  5.43e+05x^4; [2.0:*]  1.74e+06" << "\n";
   ofs << "asphaltenes,   [0.0:2.0]   1.25e+06 - 3.43e+05x  +  1.45e+06x^2  -  1.67e+06x^3 +  5.42e+05x^4; [2.0:*]  1.68e+06" << "\n";
   ofs << "EndOfTable" << "\n";
   ofs << "" << "\n";
   ofs << "Table:[PVT-Component-Properties]" << "\n";
   ofs << "ComponentName,CritTemperature" << "\n";
   ofs << "N2,             126.200" << "\n";
   ofs << "COx,            304.200" << "\n";
   ofs << "C1,             190.600" << "\n";
   ofs << "C2,             305.400" << "\n";
   ofs << "C3,             369.800" << "\n";
   ofs << "C4,             425.200" << "\n";
   ofs << "C5,             469.600" << "\n";
   ofs << "H2S,            373.200" << "\n";
   ofs << "C6-14Sat,       [0.0:2.0]    582.13 +  15.00x -	160.73x^2 + 189.96x^3 -  61.28x^4 ; [2.0:*]	 508.426" << "\n";
   ofs << "C6-14Aro,       [0.0:2.0]    686.69 -  41.72x -	101.39x^2 + 179.72x^3 -  65.72x^4 ; [2.0:*]	 583.956" << "\n";
   ofs << "C15+Sat,        [0.0:2.0]    857.09 - 121.02x -	 56.60x^2 + 197.60x^3 -  79.33x^4 ; [2.0:*]	 700.218" << "\n";
   ofs << "C15+Aro,        [0.0:2.0]   1037.50 -  81.42x -	208.72x^2 + 351.63x^3 - 123.74x^4 ; [2.0:*]	 872.986" << "\n";
   ofs << "resins,         [0.0:2.0]   1160.30 -  41.11x -	319.82x^2 + 444.11x^3 - 148.68x^4 ; [2.0:*]	 972.8" << "\n";
   ofs << "asphaltenes,    [0.0:2.0]   1351.30 - 206.25x -	105.01x^2 + 341.23x^3 - 136.23x^4 ; [2.0:*]	 1068.92" << "\n";
   ofs << "EndOfTable" << "\n";
   ofs << "" << "\n";
   ofs << "Table:[PVT-General-Properties]" << "\n";
   ofs << "EOS,OmegaA,OmegaB,LBC0,LBC1,LBC2,LBC3,LBC4" << "\n";
   ofs << "PR,   4.57240000E-01,   7.77960000E-02,   1.02300000E-01,   2.33640000E-02,   5.85330000E-02,   -4.07580000E-02,   9.33240000E-03" << std::endl;
   ofs << "EndOfTable" << std::endl;
   ofs.close();

   EXPECT_TRUE( ofs ) << "Could not write configuration file '" << s_CfgFile << "'";
}


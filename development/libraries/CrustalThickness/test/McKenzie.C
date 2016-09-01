//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include "../src/McKenzieCrustCalculator.h"

// CrustalThickness library test utilities
#include "MockConfigFileParameterCtc.h"
#include "MockInterfaceInput.h"
#include "MockInterfaceOutput.h"
#include "MockValidator.h"

// CrustalThickness library
#include "../src/LinearFunction.h"

// DataAccess libraries
#include "../../SerialDataAccess/src/Interface/SerialGrid.h"
#include "../../SerialDataAccess/src/Interface/SerialGridMap.h"
#include "../../DataAccess/src/Interface/Grid.h"

// Derived Properties library test utilities
#include "../../DerivedProperties/test/MockDerivedSurfaceProperty.h"

// Google test library
#include <gtest/gtest.h>

using namespace CrustalThicknessInterface;
using namespace CrustalThickness;

typedef formattingexception::GeneralException McKenzieException;
typedef std::shared_ptr<McKenzieCrustCalculator> McKenzieCrustCalculatorPtr;

/// @class McKenzieTest Creates some data for the tests and define some fuctions to easily create the calculators
class McKenzieTest : public ::testing::Test
{
public:
   McKenzieTest() :
      m_firstI( 0 ), m_firstJ( 0 ), m_lastI( 2 ), m_lastJ( 2 ),
      m_minI( 0.0 ), m_minJ( 0.0 ), m_maxI( 4.0 ), m_maxJ( 4.0 ),
      m_numI( 3 ), m_numJ( 3 ),
      m_constants(),
      m_validator(),
      m_inputData(),
      m_outputData( this->m_firstI, this->m_firstJ, this->m_lastI, this->m_lastJ )
   {
      m_grid    = new DataAccess::Interface::SerialGrid( this->m_minI, this->m_minJ, this->m_maxI, this->m_maxJ, this->m_numI, this->m_numJ );
      //   - basement depth is 5000m
      m_depthBasement = new const DataModel::MockDerivedSurfaceProperty( this->m_firstI, this->m_firstJ, this->m_firstI, this->m_firstJ,
         this->m_lastI, this->m_lastJ, this->m_lastI, this->m_lastJ, "depthBasement", "Depth", 10, 5000 );
      //   - initial continental crustal thickness is 40Km
      m_HCuMap  = new DataAccess::Interface::SerialGridMap( 0, 0, this->m_grid, 40000, 1  );
      //   - maximum oceanic crustal thickness is 4Km
      m_HBuMap  = new DataAccess::Interface::SerialGridMap( 0, 0, this->m_grid, 4000, 1   );
      //   - initial lithospheric mantle thickness is 115Km
      m_HLMuMap = new DataAccess::Interface::SerialGridMap( 0, 0, this->m_grid, 115000, 1 );
      //   - Rifting start is 100Ma (m_gridMap)
      m_T0Map   = new DataAccess::Interface::SerialGridMap( 0, 0, this->m_grid, 100, 1    );
      //   - Rifting end is 80 Ma
      m_TRMap   = new DataAccess::Interface::SerialGridMap( 0, 0, this->m_grid, 60, 1     );
   }

   ~McKenzieTest()
   {
      delete m_grid;
      delete m_depthBasement;
      delete m_HCuMap;
      delete m_HBuMap;
      delete m_HLMuMap;
      delete m_T0Map;
      delete m_TRMap;
   }

   /// @brief Initialise the constants (to mimic configuration file)
   void initConstants( MockConfigFileParameterCtc& constants ) const
   {
      constants.setWaterDensity               ( 1030    );
      constants.setBackstrippingMantleDensity ( 3300    );
      constants.setModelTotalLithoThickness   ( 92500   );
      constants.setDecayConstant              ( 10000   );
      constants.setTau                        ( 42.9    );
      constants.setLithoMantleDensity         ( 3360    );
      constants.setCoeffThermExpansion        ( 3.45e-5 );
      constants.setBaseLithosphericTemperature( 1330    );
      constants.setReferenceCrustThickness    ( 35000   );
      constants.setReferenceCrustDensity      ( 2875    );
      constants.setA                          ( 2.9881  );
      constants.setB                          ( 1050    );
      constants.setC                          ( 280     );
      constants.setD                          ( -294000 );
      constants.setE                          ( 2830    );
      constants.setF                          ( 2840    );
      constants.setInitialSubsidenceMax       ( 4055    );
      constants.setE0                         ( 2656    );
   }

   /// @brief Create a McKenzie calculator
   /// @details The outputData is filled with zeros
   ///   The constants are defined by the initialisation function but can still be modified afterward if needed
   /// @return A smart pointer to a McKenzieCrustCalculator object
   const McKenzieCrustCalculatorPtr createMcKenzieCalculator()
   {
      m_outputData.clear();
      initConstants( m_constants );
      m_inputData.setConstants( m_constants );
      m_inputData.setT0Map  ( m_T0Map   );
      m_inputData.setTRMap  ( m_TRMap   );
      m_inputData.setHCuMap ( m_HCuMap  );
      m_inputData.setHBuMap ( m_HBuMap  );
      m_inputData.setHLMuMap( m_HLMuMap );

      m_inputData.setDepthBasement( m_depthBasement->getMockderivedSurfacePropertyPtr() );
      return McKenzieCrustCalculatorPtr( new McKenzieCrustCalculator( m_inputData, m_outputData, m_validator ) );
   }

   // Global grid size variables (no gosth nodes)
   const unsigned int m_firstI;
   const unsigned int m_firstJ;
   const unsigned int m_lastI;
   const unsigned int m_lastJ;
   const double m_minI;
   const double m_minJ;
   const double m_maxI;
   const double m_maxJ;
   const unsigned int m_numI; 
   const unsigned int m_numJ;

   // grids
   const DataAccess::Interface::SerialGrid* m_grid;
   DataAccess::Interface::SerialGridMap* m_HCuMap;  ///< The map used to define the initial continental crustal thickness (40Km by default)
   DataAccess::Interface::SerialGridMap* m_HBuMap;  ///< The map used to define the maximum oceanic crustal thickness (4Km by default)
   DataAccess::Interface::SerialGridMap* m_HLMuMap; ///< The map used to define initial lithospheric mantle thickness (115Km by default)
   DataAccess::Interface::SerialGridMap* m_T0Map;   ///< The map used to define the end of rifting (100Ma by default)
   DataAccess::Interface::SerialGridMap* m_TRMap;   ///< The map used to define the end of rifting (60Ma by default)


   // derived properties
   const DataModel::MockDerivedSurfaceProperty* m_depthBasement; ///< Depth of the basement at 10Ma (5Km by default)

   // configuration file constants
   MockConfigFileParameterCtc m_constants;

   // input data
   MockInterfaceInput m_inputData;

   // ouput data array
   MockInterfaceOutput m_outputData;

   // global validator
   MockValidator m_validator;

};

TEST_F( McKenzieTest, exceptions )
{
   McKenzieCrustCalculatorPtr mcKenzieCalculator = createMcKenzieCalculator();
   m_constants.setWaterDensity( 2000 );
   m_constants.setBackstrippingMantleDensity( 2000 );
   m_inputData.setConstants( m_constants );

   // 1. Test that the constructor throws an exception when the mantle and water densities are equal
   //test if the exception is thrown
   EXPECT_THROW( McKenzieCrustCalculator mcKenzieCalculator1( m_inputData, m_outputData, m_validator ), std::invalid_argument );
   //test if the good exception is thrown
   try{
      McKenzieCrustCalculator mcKenzieCalculator1( m_inputData, m_outputData, m_validator );
      FAIL() << "Expected 'Mantle density is equal to the water density (both provided by the interface input), this will lead to divisions by 0' exception";
   }
   catch (const std::invalid_argument& ex) {
      EXPECT_EQ( "Mantle density is equal to the water density (both provided by the interface input), this will lead to divisions by 0",
         std::string( ex.what() ) );
   }
   catch (...) {
      FAIL() << "Expected 'Mantle density is equal to the water density (both provided by the interface input), this will lead to divisions by 0' exception";
   }

   // 2. Test that the constructor throws an exception when dpeht of the basement is a null pointer
   m_constants.setWaterDensity( 1030 );
   m_constants.setBackstrippingMantleDensity( 3300 );
   m_inputData.setConstants( m_constants );
   m_inputData.setDepthBasement( nullptr );
   //test if the exception is thrown
   EXPECT_THROW( McKenzieCrustCalculator mcKenzieCalculator2( m_inputData, m_outputData, m_validator ), std::invalid_argument );
   //test if the good exception is thrown
   try{
      McKenzieCrustCalculator mcKenzieCalculator2( m_inputData, m_outputData, m_validator );
      FAIL() << "Expected 'Basement depth provided by the interface input is a null pointer' exception";
   }
   catch (const std::invalid_argument& ex) {
      EXPECT_EQ( "Basement depth provided by the interface input is a null pointer",
         std::string( ex.what() ) );
   }
   catch (...) {
      FAIL() << "Expected 'Basement depth provided by the interface input is a null pointer' exception";
   }

   // 3. Test that the constructor throws an exception when the model total lithospheric thickness is 0
   m_inputData.setDepthBasement( m_depthBasement->getMockderivedSurfacePropertyPtr() );
   m_constants.setModelTotalLithoThickness( 0 );
   m_inputData.setConstants( m_constants );
   //test if the exception is thrown
   EXPECT_THROW( McKenzieCrustCalculator mcKenzieCalculator3( m_inputData, m_outputData, m_validator ), std::invalid_argument );
   //test if the good exception is thrown
   try{
      McKenzieCrustCalculator mcKenzieCalculator3( m_inputData, m_outputData, m_validator );
      FAIL() << "Expected 'Total lithosphere thickness provided by the interface input is equal to 0 and will lead to divisions by 0' exception";
   }
   catch (const std::invalid_argument& ex) {
      EXPECT_EQ( "Total lithosphere thickness provided by the interface input is equal to 0 and will lead to divisions by 0",
         std::string( ex.what() ) );
   }
   catch (...) {
      FAIL() << "Expected 'Total lithosphere thickness provided by the interface input is equal to 0 and will lead to divisions by 0' exception";
   }

   // 4. Test that the constructor throws an exception when the decay constant is 0
   //test if the exception is thrown
   m_constants.setModelTotalLithoThickness( 92500 );
   m_constants.setDecayConstant( 0 );
   m_inputData.setConstants( m_constants );
   EXPECT_THROW( McKenzieCrustCalculator mcKenzieCalculator4( m_inputData, m_outputData, m_validator ), std::invalid_argument );
   //test if the good exception is thrown
   try{
      McKenzieCrustCalculator mcKenzieCalculator4( m_inputData, m_outputData, m_validator );
      FAIL() << "Expected 'Decay constant provided by the interface input is 0 and will lead to divisions by 0' exception";
   }
   catch (const std::invalid_argument& ex) {
      EXPECT_EQ( "Decay constant provided by the interface input is 0 and will lead to divisions by 0",
         std::string( ex.what() ) );
   }
   catch (...) {
      FAIL() << "Expected 'Decay constant provided by the interface input is 0 and will lead to divisions by 0' exception";
   }

   // 5. Test that the constructor throws an exception when the McKenzie equation Tau variable is 0
   m_constants.setDecayConstant( 10000 );
   m_constants.setTau( 0 );
   m_inputData.setConstants( m_constants );
   //test if the exception is thrown
   EXPECT_THROW( McKenzieCrustCalculator mcKenzieCalculator5( m_inputData, m_outputData, m_validator ), std::invalid_argument );
   //test if the good exception is thrown
   try{
      McKenzieCrustCalculator mcKenzieCalculator5( m_inputData, m_outputData, m_validator );
      FAIL() << "Expected 'Tau provided by the interface input is 0 and will lead to divisions by 0' exception";
   }
   catch (const std::invalid_argument& ex) {
      EXPECT_EQ( "Tau provided by the interface input is 0 and will lead to divisions by 0",
         std::string( ex.what() ) );
   }
   catch (...) {
      FAIL() << "Expected 'Tau provided by the interface input is 0 and will lead to divisions by 0' exception";
   }

}


TEST_F( McKenzieTest, calculates_crustproperties )
{
   McKenzieCrustCalculatorPtr mcKenzieCalculator = createMcKenzieCalculator();

   // 1. continental crust density
   //::testing::FLAGS_gtest_death_test_style = "threadsafe";
   //ASSERT_DEATH( mcKenzieCalculator1.calculateContinentalCrustDensity( 0 ), "Assertion.*initialLithosphericThickness!=0" );
   EXPECT_NEAR( 2927.81214864865, mcKenzieCalculator->calculateContinentalCrustDensity( 40000 ), 1e-11 );
   EXPECT_NEAR( 2803.19485045045, mcKenzieCalculator->calculateContinentalCrustDensity( 30000 ), 1e-11 );

   // 2. asthenosphere potential temperature
   EXPECT_NEAR( 1238.98403752698, mcKenzieCalculator->calculateAstenospherePotentialTemperature( 4000 ), 1e-11 );
   EXPECT_NEAR( 1281.45723073605, mcKenzieCalculator->calculateAstenospherePotentialTemperature( 6000 ), 1e-11 );

   // 3. magma density
   EXPECT_NEAR( 2833.29679953964, mcKenzieCalculator->calculateMagmaDensity( 4000 ), 1e-11 );
   EXPECT_NEAR( 2834.51188363906, mcKenzieCalculator->calculateMagmaDensity( 6000 ), 1e-11 );
   m_constants.setE( 3300 );
   m_constants.setF( 3300 );
   m_inputData.setConstants( m_constants );

   //test if the exception is thrown
   EXPECT_THROW( mcKenzieCalculator->calculateMagmaDensity( 4000 ), McKenzieException );
   //test if the good exception is thrown
   try{
      mcKenzieCalculator->calculateMagmaDensity( 4000 );
      FAIL() << "Expected 'Your mantle density (from configuration file) is equal to your magmatic density (from computation)' exception";
   }
   catch (const McKenzieException& ex) {
      EXPECT_EQ( "Your mantle density (from configuration file) is equal to your magmatic density (from computation)",
         std::string( ex.what() ) );
   }
   catch (...) {
      FAIL() << "Expected 'Your mantle density (from configuration file) is equal to your magmatic density (from computation)' exception";
   }
}

TEST_F( McKenzieTest, calculates_thinningfactors )
{
   McKenzieCrustCalculatorPtr mcKenzieCalculator = createMcKenzieCalculator();

   // 1. thinning factor at melt onset
   //::testing::FLAGS_gtest_death_test_style = "threadsafe";
   //ASSERT_DEATH( mcKenzieCalculator.calculateThinningFactorOnset( 1238.98403752698, 0 ), "Assertion.*initialLithosphericThickness != 0" );
   EXPECT_NEAR( 0.65860948060, mcKenzieCalculator->calculateThinningFactorOnset( 1238.98403752698, 155000 ), 1e-11 );
   EXPECT_NEAR( 0.50147673380, mcKenzieCalculator->calculateThinningFactorOnset( 1281.45723073605, 130000 ), 1e-11 );

   //2. thinning factor at melt onset linearized
   EXPECT_NEAR( 0.77240632040, mcKenzieCalculator->calculateThinningFactorOnsetLinearized( 0.65860948060 ), 1e-11 );
   EXPECT_NEAR( 0.66765115587, mcKenzieCalculator->calculateThinningFactorOnsetLinearized( 0.50147673380 ), 1e-11 );

   //3. thinning factor at melt for maximum basalt thickness
   EXPECT_EQ( 0.6586094806, mcKenzieCalculator->calculateThinningFactorOnsetAtMaxBasalt( 0, 0.6586094806 ) );
   EXPECT_NEAR( 0.90000903190, mcKenzieCalculator->calculateThinningFactorOnsetAtMaxBasalt( 4000, 0.65860948060 ), 1e-11 );
   EXPECT_NEAR( 0.78929927574, mcKenzieCalculator->calculateThinningFactorOnsetAtMaxBasalt( 6000, 0.50147673380 ), 1e-11 );

}

TEST_F( McKenzieTest, calculates_ttsendmembers )
{
   McKenzieCrustCalculatorPtr mcKenzieCalculator = createMcKenzieCalculator();

   // 1. TTS at exhume point
   EXPECT_NEAR( 6920.22280595189, mcKenzieCalculator->calculateTTSexhume( 80  ), 1e-11 );
   EXPECT_NEAR( 7323.88564105350, mcKenzieCalculator->calculateTTSexhume( 250 ), 1e-11 );

   // 2. TTS at critical point
   EXPECT_NEAR( 6097.83831174862, mcKenzieCalculator->calculateTTScritical( 6920.22280595189, 4000, 2833.29679953964 ), 1e-11 );
   EXPECT_NEAR( 6093.52057578229, mcKenzieCalculator->calculateTTScritical( 7323.88564105350, 6000, 2834.51188363906 ), 1e-11 );

   // 3. TTS at melting point linearized
   //ASSERT_DEATH( mcKenzieCalculator.calculateTTSOnsetLinearized( 80, 0 ), "" );
   EXPECT_NEAR( 5285.74782707973, mcKenzieCalculator->calculateTTSOnsetLinearized( 80 , 0.77240632040 ), 1e-07 );
   EXPECT_NEAR( 4888.54662998943, mcKenzieCalculator->calculateTTSOnsetLinearized( 250, 0.66765115587 ), 1e-07 );
   EXPECT_NEAR( 7323.88564105350, mcKenzieCalculator->calculateTTSOnsetLinearized( 250, 1             ), 1e-11 );

   // 4. TTS at exhume point with serpentinization of the mantle
   EXPECT_NEAR( 6238.58340595189, mcKenzieCalculator->calculateTTSexhumeSerpentinized( 6920.22280595189 ), 1e-11 );
   EXPECT_NEAR( 6642.24624105350, mcKenzieCalculator->calculateTTSexhumeSerpentinized( 7323.88564105350 ), 1e-11 );

}

TEST_F( McKenzieTest, define_linearfunction )
{
   McKenzieCrustCalculatorPtr mcKenzieCalculator = createMcKenzieCalculator();
   LinearFunction linearFunction;

   //ASSERT_DEATH( mcKenzieCalculator.defineLinearFunction( linearFunction, 4000, 3300,             0.77240632040, 5190.24172179322, 6097.83831174862 ), "" );
   //ASSERT_DEATH( mcKenzieCalculator.defineLinearFunction( linearFunction, 4000, 2833.29679953964, 0.77240632040, 0,                6097.83831174862 ), "" );

   mcKenzieCalculator->defineLinearFunction( linearFunction, 4000, 2833.29679953964, 0.77240632040, 5285.74782707973, 6097.83831174862 );
   EXPECT_EQ( 6097.83831174862, linearFunction.getWLS_crit()                  );
   EXPECT_EQ( 5285.74782707973, linearFunction.getWLS_onset()                 );
   EXPECT_EQ( 4000            , linearFunction.getMaxBasalticCrustThickness() );
   EXPECT_NEAR( 0.00014613000 , linearFunction.getM1()                 , 1e-09 );
   EXPECT_NEAR( 0.00028025655 , linearFunction.getM2()                 , 1e-09 );
   EXPECT_NEAR( -0.70895914334, linearFunction.getC2()                 , 1e-09 );
   EXPECT_NEAR( 4.86390493522 , linearFunction.getMagmaThicknessCoeff(), 1e-09 );
   EXPECT_NEAR( 0.97258017204 , linearFunction.getCrustTF( 6000 )      , 1e-09 );

   mcKenzieCalculator->defineLinearFunction( linearFunction, 6000, 2834.51188363906, 0.66765115587, 4888.54662998943, 6093.52057578229 );
   EXPECT_EQ( 6093.52057578229, linearFunction.getWLS_crit()                  );
   EXPECT_EQ( 4888.54662998943, linearFunction.getWLS_onset()                 );
   EXPECT_EQ( 6000            , linearFunction.getMaxBasalticCrustThickness() );
   EXPECT_NEAR( 0.00013657457 , linearFunction.getM1()                 , 1e-09 );
   EXPECT_NEAR( 0.00027581413 , linearFunction.getM2()                 , 1e-09 );
   EXPECT_NEAR( -0.68067909446, linearFunction.getC2()                 , 1e-09 );
   EXPECT_NEAR( 4.87660140015 , linearFunction.getMagmaThicknessCoeff(), 1e-09 );
   EXPECT_EQ( 1, linearFunction.getCrustTF( 6500 ) );

}

TEST_F( McKenzieTest, calculates_crustthicknesses )
{
   McKenzieCrustCalculatorPtr mcKenzieCalculator = createMcKenzieCalculator();

   // 1. continental crustal thickness at melting point
   EXPECT_NEAR( 9103.74718409551, mcKenzieCalculator->calculateContinentalCrustThicknessOnset( 40000, 0.77240632040 ), 1e-06 );
   EXPECT_NEAR( 9970.46532401437, mcKenzieCalculator->calculateContinentalCrustThicknessOnset( 30000, 0.66765115587 ), 1e-06 );

   // 2. residual depth anomaly
   EXPECT_NEAR( 97.83831174862,   mcKenzieCalculator->calculateResidualDepthAnomaly( 6097.83831174862, 6000 ), 1e-11 );
   EXPECT_NEAR( -406.47942421771, mcKenzieCalculator->calculateResidualDepthAnomaly( 6093.52057578229, 6500 ), 1e-11 );

   // 3. continental crustal thickness
   EXPECT_NEAR( 1096.79311836947, mcKenzieCalculator->calculateContinentalCrusltalThickness( 0.97258017204, 40000 ), 1e-7 );
   EXPECT_EQ( 0, mcKenzieCalculator->calculateContinentalCrusltalThickness( 1 , 30000 ) );
   EXPECT_EQ( 0, mcKenzieCalculator->calculateContinentalCrusltalThickness( 10, 30000 ) );

   // 4. oceanic crustal thickness
   LinearFunction linearFunction;
   mcKenzieCalculator->defineLinearFunction( linearFunction, 4000, 2833.29679953964, 0.77240632040, 5285.74782707973, 6097.83831174862 );
   EXPECT_NEAR( 3518.09157429785, mcKenzieCalculator->calculateOceanicCrustalThickness( 5500, 6920.22280595189, 6000, linearFunction ), 1e-10 );
   mcKenzieCalculator->defineLinearFunction( linearFunction, 6000, 2834.51188363906, 0.66765115587, 4888.54662998943, 6093.52057578229 );
   EXPECT_NEAR( 4017.76187072684, mcKenzieCalculator->calculateOceanicCrustalThickness( 6000, 7323.88564105350, 6500, linearFunction ), 1e-10 );
   EXPECT_EQ( 0, mcKenzieCalculator->calculateOceanicCrustalThickness( 7500, 7323.88564105350, 6500, linearFunction ) );

   // 4. top oceanic crust
   EXPECT_NEAR( 6096.79311836947, mcKenzieCalculator->calculateTopOceanicCrust( 1096.79311836947, 5000 ), 1e-11 );
   EXPECT_EQ( 3500, mcKenzieCalculator->calculateTopOceanicCrust( 0, 3500 ) );

   // 5. moho
   EXPECT_NEAR( 9614.88469266732, mcKenzieCalculator->calculateMoho( 6096.79311836947, 3518.09157429785 ), 1e-11 );
   EXPECT_NEAR( 7517.76187072684, mcKenzieCalculator->calculateMoho( 3500            , 4017.76187072684 ), 1e-11 );

   // 6. effective crustal thickness
   //ASSERT_DEATH( mcKenzieCalculator->calculateEffectiveCrustalThickness( 1096.79311836947, 3518.09157429785, 40000, 0 ), "" );
   EXPECT_NEAR( 2320.47714421220, mcKenzieCalculator->calculateEffectiveCrustalThickness( 1096.79311836947, 3518.09157429785, 40000, 115000 ), 1e-11 );
   EXPECT_NEAR( 1205.32856121805, mcKenzieCalculator->calculateEffectiveCrustalThickness( 0               , 4017.76187072684, 30000, 100000 ), 1e-11 );

}

TEST_F( McKenzieTest, compute )
{

   // 1. With all valid input values
   McKenzieCrustCalculatorPtr mcKenzieCalculator = createMcKenzieCalculator();
   m_outputData.setMapValues( WLSMap, 5500 );
   m_outputData.setMapValues( WLSadjustedMap, 6000 );
   const DataModel::MockDerivedSurfaceProperty depthBasement( this->m_firstI, this->m_firstJ, this->m_firstI, this->m_firstJ,
      this->m_lastI, this->m_lastJ, this->m_lastI, this->m_lastJ, "depthBasement", "Depth", 10, 5000 );
   mcKenzieCalculator->compute();

   for (unsigned int i = m_firstI; i <= m_lastI; i++)
   {
      for (unsigned int j = m_firstJ; j <= m_lastJ; j++)
      {
         EXPECT_NEAR( 2927.81214864865, m_outputData.getMapValue( estimatedCrustDensityMap, i, j ), 1e-11 );
         EXPECT_NEAR( 2833.29679953964, m_outputData.getMapValue( basaltDensityMap        , i, j ), 1e-11 );
         EXPECT_NEAR( 1238.98403752698, m_outputData.getMapValue( PTaMap                  , i, j ), 1e-11 );
         EXPECT_NEAR( 0.65860948060   , m_outputData.getMapValue( TFOnsetMap              , i, j ), 1e-11 );
         EXPECT_NEAR( 0.90000903190   , m_outputData.getMapValue( TFOnsetMigMap           , i, j ), 1e-11 );
         EXPECT_NEAR( 0.77240632040   , m_outputData.getMapValue( TFOnsetLinMap           , i, j ), 1e-07 );
         EXPECT_NEAR( 6920.22280595189, m_outputData.getMapValue( WLSExhumeMap            , i, j ), 1e-11 );
         EXPECT_NEAR( 6097.83831174862, m_outputData.getMapValue( WLSCritMap              , i, j ), 1e-11 );
         EXPECT_NEAR( 5285.74782707973, m_outputData.getMapValue( WLSOnsetMap             , i, j ), 1e-07 );
         EXPECT_NEAR( 6238.58340595189, m_outputData.getMapValue( WLSExhumeSerpMap        , i, j ), 1e-11 );
         EXPECT_NEAR( 9103.74718409551, m_outputData.getMapValue( thicknessCrustMeltOnset , i, j ), 1e-06 );
         EXPECT_NEAR( 0.00014613000   , m_outputData.getMapValue( slopePreMelt            , i, j ), 1e-09 );
         EXPECT_NEAR( 0.00028025655   , m_outputData.getMapValue( slopePostMelt           , i, j ), 1e-09 );
         EXPECT_NEAR( -0.70895914334  , m_outputData.getMapValue( interceptPostMelt       , i, j ), 1e-09 );
         EXPECT_NEAR( 97.83831174862  , m_outputData.getMapValue( RDAadjustedMap          , i, j ), 1e-11 );
         EXPECT_NEAR( 0.97258017204   , m_outputData.getMapValue( TFMap                   , i, j ), 1e-09 );
         EXPECT_NEAR( 1096.79311836947, m_outputData.getMapValue( thicknessCrustMap       , i, j ), 1e-07 );
         EXPECT_NEAR( 3518.09157429785, m_outputData.getMapValue( thicknessBasaltMap      , i, j ), 1e-10 );
         EXPECT_NEAR( 6096.79311836947, m_outputData.getMapValue( topBasaltMap            , i, j ), 1e-11 );
         EXPECT_NEAR( 9614.88469266732, m_outputData.getMapValue( mohoMap                 , i, j ), 1e-11 );
         EXPECT_NEAR( 2320.47714421220, m_outputData.getMapValue( ECTMap                  , i, j ), 1e-11 );
      }
   }

   // 2. TTS or TTS adjusted are NDV
   m_outputData.clear();
   m_outputData.setMapValues( WLSMap, 5500 );
   m_outputData.setMapValue ( WLSMap, 0, 0, Interface::DefaultUndefinedMapValue );
   m_outputData.setMapValues( WLSadjustedMap, 6000 );
   m_outputData.setMapValue( WLSadjustedMap, 0, 1, Interface::DefaultUndefinedMapValue );
   mcKenzieCalculator->compute();

   for (unsigned int i = m_firstI; i <= m_lastI; i++)
   {
      for (unsigned int j = m_firstJ; j <= m_lastJ; j++)
      {
         EXPECT_NEAR( 2927.81214864865, m_outputData.getMapValue( estimatedCrustDensityMap, i, j ), 1e-11 );
         EXPECT_NEAR( 2833.29679953964, m_outputData.getMapValue( basaltDensityMap        , i, j ), 1e-11 );
         EXPECT_NEAR( 1238.98403752698, m_outputData.getMapValue( PTaMap                  , i, j ), 1e-11 );
         EXPECT_NEAR( 0.65860948060   , m_outputData.getMapValue( TFOnsetMap              , i, j ), 1e-11 );
         EXPECT_NEAR( 0.90000903190   , m_outputData.getMapValue( TFOnsetMigMap           , i, j ), 1e-11 );
         EXPECT_NEAR( 0.77240632040   , m_outputData.getMapValue( TFOnsetLinMap           , i, j ), 1e-07 );
         EXPECT_NEAR( 6920.22280595189, m_outputData.getMapValue( WLSExhumeMap            , i, j ), 1e-11 );
         EXPECT_NEAR( 6097.83831174862, m_outputData.getMapValue( WLSCritMap              , i, j ), 1e-11 );
         EXPECT_NEAR( 5285.74782707973, m_outputData.getMapValue( WLSOnsetMap             , i, j ), 1e-07 );
         EXPECT_NEAR( 6238.58340595189, m_outputData.getMapValue( WLSExhumeSerpMap        , i, j ), 1e-11 );
         EXPECT_NEAR( 9103.74718409551, m_outputData.getMapValue( thicknessCrustMeltOnset , i, j ), 1e-06 );
         EXPECT_NEAR( 0.00014613000   , m_outputData.getMapValue( slopePreMelt            , i, j ), 1e-09 );
         EXPECT_NEAR( 0.00028025655   , m_outputData.getMapValue( slopePostMelt           , i, j ), 1e-09 );
         EXPECT_NEAR( -0.70895914334  , m_outputData.getMapValue( interceptPostMelt       , i, j ), 1e-09 );
         if ( i == 0 and ( j == 0 or j == 1 ) )
         {
            EXPECT_NEAR( Interface::DefaultUndefinedMapValue, m_outputData.getMapValue( RDAadjustedMap          , i, j ), 1e-11 );
            EXPECT_NEAR( Interface::DefaultUndefinedMapValue, m_outputData.getMapValue( TFMap                   , i, j ), 1e-09 );
            EXPECT_NEAR( Interface::DefaultUndefinedMapValue, m_outputData.getMapValue( thicknessCrustMap       , i, j ), 1e-07 );
            EXPECT_NEAR( Interface::DefaultUndefinedMapValue, m_outputData.getMapValue( thicknessBasaltMap      , i, j ), 1e-10 );
            EXPECT_NEAR( Interface::DefaultUndefinedMapValue, m_outputData.getMapValue( topBasaltMap            , i, j ), 1e-11 );
            EXPECT_NEAR( Interface::DefaultUndefinedMapValue, m_outputData.getMapValue( mohoMap                 , i, j ), 1e-11 );
            EXPECT_NEAR( Interface::DefaultUndefinedMapValue, m_outputData.getMapValue( ECTMap                  , i, j ), 1e-11 );
         }
         else
         {
            EXPECT_NEAR( 97.83831174862  , m_outputData.getMapValue( RDAadjustedMap          , i, j ), 1e-11 );
            EXPECT_NEAR( 0.97258017204   , m_outputData.getMapValue( TFMap                   , i, j ), 1e-09 );
            EXPECT_NEAR( 1096.79311836947, m_outputData.getMapValue( thicknessCrustMap       , i, j ), 1e-07 );
            EXPECT_NEAR( 3518.09157429785, m_outputData.getMapValue( thicknessBasaltMap      , i, j ), 1e-10 );
            EXPECT_NEAR( 6096.79311836947, m_outputData.getMapValue( topBasaltMap            , i, j ), 1e-11 );
            EXPECT_NEAR( 9614.88469266732, m_outputData.getMapValue( mohoMap                 , i, j ), 1e-11 );
            EXPECT_NEAR( 2320.47714421220, m_outputData.getMapValue( ECTMap                  , i, j ), 1e-11 );
         }
      }
   }



   // 3. The node is not valid
   m_validator.setIsValid( false );
   m_outputData.clear();
   m_outputData.setMapValues( WLSMap, 5500 );
   m_outputData.setMapValues( WLSadjustedMap, 6000 );
   mcKenzieCalculator->compute();

   for (unsigned int i = m_firstI; i <= m_lastI; i++)
   {
      for (unsigned int j = m_firstJ; j <= m_lastJ; j++)
      {
         EXPECT_EQ( Interface::DefaultUndefinedMapValue, m_outputData.getMapValue( estimatedCrustDensityMap, i, j ) );
         EXPECT_EQ( Interface::DefaultUndefinedMapValue, m_outputData.getMapValue( basaltDensityMap        , i, j ) );
         EXPECT_EQ( Interface::DefaultUndefinedMapValue, m_outputData.getMapValue( PTaMap                  , i, j ) );
         EXPECT_EQ( Interface::DefaultUndefinedMapValue, m_outputData.getMapValue( TFOnsetMap              , i, j ) );
         EXPECT_EQ( Interface::DefaultUndefinedMapValue, m_outputData.getMapValue( TFOnsetMigMap           , i, j ) );
         EXPECT_EQ( Interface::DefaultUndefinedMapValue, m_outputData.getMapValue( TFOnsetLinMap           , i, j ) );
         EXPECT_EQ( Interface::DefaultUndefinedMapValue, m_outputData.getMapValue( WLSExhumeMap            , i, j ) );
         EXPECT_EQ( Interface::DefaultUndefinedMapValue, m_outputData.getMapValue( WLSCritMap              , i, j ) );
         EXPECT_EQ( Interface::DefaultUndefinedMapValue, m_outputData.getMapValue( WLSOnsetMap             , i, j ) );
         EXPECT_EQ( Interface::DefaultUndefinedMapValue, m_outputData.getMapValue( WLSExhumeSerpMap        , i, j ) );
         EXPECT_EQ( Interface::DefaultUndefinedMapValue, m_outputData.getMapValue( thicknessCrustMeltOnset , i, j ) );
         EXPECT_EQ( Interface::DefaultUndefinedMapValue, m_outputData.getMapValue( slopePreMelt            , i, j ) );
         EXPECT_EQ( Interface::DefaultUndefinedMapValue, m_outputData.getMapValue( slopePostMelt           , i, j ) );
         EXPECT_EQ( Interface::DefaultUndefinedMapValue, m_outputData.getMapValue( interceptPostMelt       , i, j ) );
         EXPECT_EQ( Interface::DefaultUndefinedMapValue, m_outputData.getMapValue( RDAadjustedMap          , i, j ) );
         EXPECT_EQ( Interface::DefaultUndefinedMapValue, m_outputData.getMapValue( TFMap                   , i, j ) );
         EXPECT_EQ( Interface::DefaultUndefinedMapValue, m_outputData.getMapValue( thicknessCrustMap       , i, j ) );
         EXPECT_EQ( Interface::DefaultUndefinedMapValue, m_outputData.getMapValue( thicknessBasaltMap      , i, j ) );
         EXPECT_EQ( Interface::DefaultUndefinedMapValue, m_outputData.getMapValue( topBasaltMap            , i, j ) );
         EXPECT_EQ( Interface::DefaultUndefinedMapValue, m_outputData.getMapValue( mohoMap                 , i, j ) );
         EXPECT_EQ( Interface::DefaultUndefinedMapValue, m_outputData.getMapValue( ECTMap                  , i, j ) );
      }
   }

   // 4. The input map are NDV
   m_outputData.clear();
   m_outputData.setMapValues( WLSMap, 5500 );
   m_outputData.setMapValues( WLSadjustedMap, 6000 );
   m_T0Map  ->setValue( 0, 0, Interface::DefaultUndefinedMapValue );
   m_TRMap  ->setValue( 0, 1, Interface::DefaultUndefinedMapValue );
   m_HCuMap ->setValue( 0, 2, Interface::DefaultUndefinedMapValue );
   m_HBuMap ->setValue( 1, 0, Interface::DefaultUndefinedMapValue );
   m_HLMuMap->setValue( 1, 1, Interface::DefaultUndefinedMapValue );

   m_validator.setIsValid( true );
   mcKenzieCalculator->compute();

   for (unsigned int i = m_firstI; i <= m_lastI; i++)
   {
      for (unsigned int j = m_firstJ; j <= m_lastJ; j++)
      {

         if ( (i == 0 and ( j == 0 or j == 1 or j ==2 ) )
            or ( i == 1 and ( j == 0 or j == 1 ) ) )
         {
            EXPECT_EQ( Interface::DefaultUndefinedMapValue, m_outputData.getMapValue( estimatedCrustDensityMap, i, j ) );
            EXPECT_EQ( Interface::DefaultUndefinedMapValue, m_outputData.getMapValue( basaltDensityMap        , i, j ) );
            EXPECT_EQ( Interface::DefaultUndefinedMapValue, m_outputData.getMapValue( PTaMap                  , i, j ) );
            EXPECT_EQ( Interface::DefaultUndefinedMapValue, m_outputData.getMapValue( TFOnsetMap              , i, j ) );
            EXPECT_EQ( Interface::DefaultUndefinedMapValue, m_outputData.getMapValue( TFOnsetMigMap           , i, j ) );
            EXPECT_EQ( Interface::DefaultUndefinedMapValue, m_outputData.getMapValue( TFOnsetLinMap           , i, j ) );
            EXPECT_EQ( Interface::DefaultUndefinedMapValue, m_outputData.getMapValue( WLSExhumeMap            , i, j ) );
            EXPECT_EQ( Interface::DefaultUndefinedMapValue, m_outputData.getMapValue( WLSCritMap              , i, j ) );
            EXPECT_EQ( Interface::DefaultUndefinedMapValue, m_outputData.getMapValue( WLSOnsetMap             , i, j ) );
            EXPECT_EQ( Interface::DefaultUndefinedMapValue, m_outputData.getMapValue( WLSExhumeSerpMap        , i, j ) );
            EXPECT_EQ( Interface::DefaultUndefinedMapValue, m_outputData.getMapValue( thicknessCrustMeltOnset , i, j ) );
            EXPECT_EQ( Interface::DefaultUndefinedMapValue, m_outputData.getMapValue( slopePreMelt            , i, j ) );
            EXPECT_EQ( Interface::DefaultUndefinedMapValue, m_outputData.getMapValue( slopePostMelt           , i, j ) );
            EXPECT_EQ( Interface::DefaultUndefinedMapValue, m_outputData.getMapValue( interceptPostMelt       , i, j ) );
            EXPECT_EQ( Interface::DefaultUndefinedMapValue, m_outputData.getMapValue( RDAadjustedMap          , i, j ) );
            EXPECT_EQ( Interface::DefaultUndefinedMapValue, m_outputData.getMapValue( TFMap                   , i, j ) );
            EXPECT_EQ( Interface::DefaultUndefinedMapValue, m_outputData.getMapValue( thicknessCrustMap       , i, j ) );
            EXPECT_EQ( Interface::DefaultUndefinedMapValue, m_outputData.getMapValue( thicknessBasaltMap      , i, j ) );
            EXPECT_EQ( Interface::DefaultUndefinedMapValue, m_outputData.getMapValue( topBasaltMap            , i, j ) );
            EXPECT_EQ( Interface::DefaultUndefinedMapValue, m_outputData.getMapValue( mohoMap                 , i, j ) );
            EXPECT_EQ( Interface::DefaultUndefinedMapValue, m_outputData.getMapValue( ECTMap                  , i, j ) );
         }
         else
         {
            EXPECT_NEAR( 2927.81214864865, m_outputData.getMapValue( estimatedCrustDensityMap, i, j ), 1e-11 );
            EXPECT_NEAR( 2833.29679953964, m_outputData.getMapValue( basaltDensityMap        , i, j ), 1e-11 );
            EXPECT_NEAR( 1238.98403752698, m_outputData.getMapValue( PTaMap                  , i, j ), 1e-11 );
            EXPECT_NEAR( 0.65860948060   , m_outputData.getMapValue( TFOnsetMap              , i, j ), 1e-11 );
            EXPECT_NEAR( 0.90000903190   , m_outputData.getMapValue( TFOnsetMigMap           , i, j ), 1e-11 );
            EXPECT_NEAR( 0.77240632040   , m_outputData.getMapValue( TFOnsetLinMap           , i, j ), 1e-07 );
            EXPECT_NEAR( 6920.22280595189, m_outputData.getMapValue( WLSExhumeMap            , i, j ), 1e-11 );
            EXPECT_NEAR( 6097.83831174862, m_outputData.getMapValue( WLSCritMap              , i, j ), 1e-11 );
            EXPECT_NEAR( 5285.74782707973, m_outputData.getMapValue( WLSOnsetMap             , i, j ), 1e-07 );
            EXPECT_NEAR( 6238.58340595189, m_outputData.getMapValue( WLSExhumeSerpMap        , i, j ), 1e-11 );
            EXPECT_NEAR( 9103.74718409551, m_outputData.getMapValue( thicknessCrustMeltOnset , i, j ), 1e-06 );
            EXPECT_NEAR( 0.00014613000   , m_outputData.getMapValue( slopePreMelt            , i, j ), 1e-09 );
            EXPECT_NEAR( 0.00028025655   , m_outputData.getMapValue( slopePostMelt           , i, j ), 1e-09 );
            EXPECT_NEAR( -0.70895914334  , m_outputData.getMapValue( interceptPostMelt       , i, j ), 1e-09 );
            EXPECT_NEAR( 97.83831174862  , m_outputData.getMapValue( RDAadjustedMap          , i, j ), 1e-11 );
            EXPECT_NEAR( 0.97258017204   , m_outputData.getMapValue( TFMap                   , i, j ), 1e-09 );
            EXPECT_NEAR( 1096.79311836947, m_outputData.getMapValue( thicknessCrustMap       , i, j ), 1e-07 );
            EXPECT_NEAR( 3518.09157429785, m_outputData.getMapValue( thicknessBasaltMap      , i, j ), 1e-10 );
            EXPECT_NEAR( 6096.79311836947, m_outputData.getMapValue( topBasaltMap            , i, j ), 1e-11 );
            EXPECT_NEAR( 9614.88469266732, m_outputData.getMapValue( mohoMap                 , i, j ), 1e-11 );
            EXPECT_NEAR( 2320.47714421220, m_outputData.getMapValue( ECTMap                  , i, j ), 1e-11 );
         }

      }
   }
}


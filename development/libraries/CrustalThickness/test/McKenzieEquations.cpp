//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// CrustalThickness library test utilities
#include "McKenzieTester.h"

// CrustalThickness library
#include "../src/LinearFunction.h"

// utilites library
#include "../../utilities/test/GoogleTestMacros.h"

// Google test library
#include <gtest/gtest.h>

// std library
#include <string>
#include <exception>

using namespace CrustalThicknessInterface;
using namespace CrustalThickness;
using CrustalThickness::UnitTests::McKenzieTester;

typedef formattingexception::GeneralException McKenzieException;

TEST_F( McKenzieTester, exception1 )
{
   // first initialization of all variables
   m_constants.setWaterDensity( 2000 );
   m_constants.setBackstrippingMantleDensity( 2000 );
   m_inputData.setConstants( m_constants );

   // 1. Test that the constructor throws an exception when the mantle and water densities are equal
   std::invalid_argument exception1( "Mantle density is equal to the water density (both provided by the interface input), this will lead to divisions by 0" );
   EXPECT_EXCEPTION_EQ( McKenzieCrustCalculator( m_inputData, m_outputData, m_validator, m_age, nullptr, nullptr, nullptr ), exception1 );
}

TEST_F( McKenzieTester, exception2 )
{
   // 2. Test that the constructor throws an exception when dpeht of the basement is a null pointer
   m_inputData.setDepthBasement( nullptr );
   std::invalid_argument exception2( "Basement depth provided by the interface input is a null pointer" );
   EXPECT_EXCEPTION_EQ( McKenzieCrustCalculator( m_inputData, m_outputData, m_validator, m_age, nullptr, nullptr, nullptr ), exception2 );
}

TEST_F( McKenzieTester, exception3 )
{
   // 3. Test that the constructor throws an exception when the model total lithospheric thickness is 0
   m_constants.setModelTotalLithoThickness( 0 );
   m_inputData.setConstants( m_constants );
   std::invalid_argument exception3( "Total lithosphere thickness provided by the interface input is equal to 0 and will lead to divisions by 0" );
   EXPECT_EXCEPTION_EQ( McKenzieCrustCalculator( m_inputData, m_outputData, m_validator, m_age, nullptr, nullptr, nullptr ), exception3 );
}

TEST_F( McKenzieTester, exception4 )
{
   // 4. Test that the constructor throws an exception when the decay constant is 0
   //test if the exception is thrown
   m_constants.setDecayConstant( 0 );
   m_inputData.setConstants( m_constants );
   std::invalid_argument exception4( "Decay constant provided by the interface input is 0 and will lead to divisions by 0" );
   EXPECT_EXCEPTION_EQ( McKenzieCrustCalculator ( m_inputData, m_outputData, m_validator, m_age, nullptr, nullptr, nullptr ), exception4 );
}

TEST_F( McKenzieTester, exception5 )
{
   // 5. Test that the constructor throws an exception when the McKenzie equation Tau variable is 0
   m_constants.setTau( 0 );
   m_inputData.setConstants( m_constants );
   std::invalid_argument exception5( "Tau provided by the interface input is 0 and will lead to divisions by 0" );
   EXPECT_EXCEPTION_EQ( McKenzieCrustCalculator( m_inputData, m_outputData, m_validator, m_age, nullptr, nullptr, nullptr ), exception5 );
}

TEST_F( McKenzieTester, exception6 )
{
   // 6. Test that the constructor throws an exception when the continental crust ratio is negative
   m_inputData.setContinentalCrustRatio( -10 );
   std::invalid_argument exception6( "The continental crust ratio (which defines the lower and upper continental crust) provided by the interface input is negative" );
   EXPECT_EXCEPTION_EQ( McKenzieCrustCalculator mcKenzieCalculator( m_inputData, m_outputData, m_validator, m_age, nullptr, nullptr, nullptr ), exception6 );
}

TEST_F( McKenzieTester, exception7 )
{
   // 7. Test that the constructor throws an exception when the oceanic crust ratio is negative
   m_inputData.setContinentalCrustRatio( 0.5 );
   m_inputData.setOceanicCrustRatio( -10 );
   std::invalid_argument exception7( "The oceanic crust ratio (which defines the lower and upper oceanic crust) provided by the interface input is negative" );
   EXPECT_EXCEPTION_EQ( McKenzieCrustCalculator( m_inputData, m_outputData, m_validator, m_age, nullptr, nullptr, nullptr ), exception7 );
}

TEST_F( McKenzieTester, exception8_1 )
{
   // 8. Test that the constructor throws an exception when only one of the previous data input is a nullptr
   m_inputData.setOceanicCrustRatio( 0.5 );
   const DataAccess::Interface::SerialGridMap previousGrid( 0, 0, this->m_grid, 50, 1 );
   std::invalid_argument exception8_1( "The previous continental crust thickness provided to the McKenzie calculator is a null pointer but some other previous data are not null" );
   EXPECT_EXCEPTION_EQ( McKenzieCrustCalculator( m_inputData, m_outputData, m_validator, m_age, nullptr, nullptr, &previousGrid ), exception8_1 );
}

TEST_F( McKenzieTester, exception8_2 )
{
   m_inputData.setOceanicCrustRatio( 0.5 );
   const DataAccess::Interface::SerialGridMap previousGrid( 0, 0, this->m_grid, 50, 1 );
   std::invalid_argument exception8_2( "The previous oceanic crust thickness provided to the McKenzie calculator is a null pointer but some other previous data are not null" );
   EXPECT_EXCEPTION_EQ( McKenzieCrustCalculator( m_inputData, m_outputData, m_validator, m_age, nullptr, &previousGrid, nullptr ), exception8_2 );
}

TEST_F( McKenzieTester, exception9 )
{
   // 9. Test that the constructor throws an exception when the rift end age is NDV
   const DataAccess::Interface::SerialGridMap previousGrid( 0, 0, this->m_grid, 50, 1 );
   (m_riftingEvents[m_age])->setStartRiftAge( DataAccess::Interface::DefaultUndefinedScalarValue );
   std::invalid_argument exception9( "The beginning of the rifting event is undefined for age " + std::to_string( m_age ) + "Ma" );
   EXPECT_EXCEPTION_EQ( McKenzieCrustCalculator( m_inputData, m_outputData, m_validator, m_age, nullptr, &previousGrid, &previousGrid ), exception9 );
}

TEST_F( McKenzieTester, exception10 )
{
   // 10. Test that the constructor throws an exception when the rift end age is NDV
   const DataAccess::Interface::SerialGridMap previousGrid( 0, 0, this->m_grid, 50, 1 );
   (m_riftingEvents[m_age])->setStartRiftAge( m_startAge );
   (m_riftingEvents[m_age])->setEndRiftAge( DataAccess::Interface::DefaultUndefinedScalarValue );
   std::invalid_argument exception10( "The end of the rifting event is undefined for age " + std::to_string( m_age ) + "Ma" );
   EXPECT_EXCEPTION_EQ( McKenzieCrustCalculator( m_inputData, m_outputData, m_validator, m_age, nullptr, &previousGrid, &previousGrid ), exception10 );
}

TEST_F( McKenzieTester, calculates_crustproperties )
{
   McKenzieCrustCalculatorPtr mcKenzieCalculator = createMcKenzieCalculator();

   // 1. continental crust density
   EXPECT_NEAR( 2927.81214864865, mcKenzieCalculator->calculateContinentalCrustDensity( 40000 ), 1e-11 );
   EXPECT_NEAR( 2803.19485045045, mcKenzieCalculator->calculateContinentalCrustDensity( 30000 ), 1e-11 );

   // 2. asthenosphere potential temperature
   EXPECT_NEAR( 1238.98403752698, mcKenzieCalculator->calculateAstenospherePotentialTemperature( 4000 ), 1e-11 );
   EXPECT_NEAR( 1281.45723073605, mcKenzieCalculator->calculateAstenospherePotentialTemperature( 6000 ), 1e-11 );
   EXPECT_EQ( 1050, mcKenzieCalculator->calculateAstenospherePotentialTemperature( 0 ) );

   // 3. magma density
   EXPECT_NEAR( 2833.29679953964, mcKenzieCalculator->calculateMagmaDensity( 4000 ), 1e-11 );
   EXPECT_NEAR( 2834.51188363906, mcKenzieCalculator->calculateMagmaDensity( 6000 ), 1e-11 );
   EXPECT_EQ( 2830, mcKenzieCalculator->calculateMagmaDensity( 0 ) );

   m_constants.setE( 3300 );
   m_constants.setF( 3300 );
   m_inputData.setConstants( m_constants );
   McKenzieException exception;
   exception << "Your mantle density (from configuration file) is equal to your magmatic density (from computation)";
   EXPECT_EXCEPTION_EQ( mcKenzieCalculator->calculateMagmaDensity( 4000 ), exception );
}


#ifndef _WIN32
// TEST_F is not working on windows because of some strange
// behaviors of the projecthandle and the data access library
// Once issues in projecthandle and the data access library are fixed
// then we will be able to enable the death test on windows
// However this is not an easy problem as it seems that the memory is corrupted
#ifndef NDEBUG
TEST_F( McKenzieTester, death_tests ){
   ::testing::FLAGS_gtest_death_test_style = "threadsafe";
   McKenzieCrustCalculatorPtr mcKenzieCalculator = createMcKenzieCalculator();
   LinearFunction linearFunction;
   ASSERT_DEATH( mcKenzieCalculator->calculateContinentalCrustDensity( 0 ), "" );
   ASSERT_DEATH( mcKenzieCalculator->calculateThinningFactorOnset( 1238.98403752698, 0 ), "" );
   ASSERT_DEATH( mcKenzieCalculator->defineLinearFunction( linearFunction, 4000, 3300,             0.77240632040, 5190.24172179322, 6097.83831174862 ), "" );
   ASSERT_DEATH( mcKenzieCalculator->defineLinearFunction( linearFunction, 4000, 2833.29679953964, 0.77240632040, 0,                6097.83831174862 ), "" );
   ASSERT_DEATH( mcKenzieCalculator->calculateEffectiveCrustalThickness( 1096.79311836947, 3518.09157429785, 40000, 0 ), "" );
}
#endif
#endif

TEST_F( McKenzieTester, calculates_thinningfactors )
{
   McKenzieCrustCalculatorPtr mcKenzieCalculator = createMcKenzieCalculator();

   // 1. thinning factor at melt onset
   EXPECT_NEAR( 0.65860948060, mcKenzieCalculator->calculateThinningFactorOnset( 1238.98403752698, 155000 ), 1e-11 );
   EXPECT_NEAR( 0.50147673380, mcKenzieCalculator->calculateThinningFactorOnset( 1281.45723073605, 130000 ), 1e-11 );
   EXPECT_EQ( 1, mcKenzieCalculator->calculateThinningFactorOnset( 1050, 130000 ) );

   //2. thinning factor at melt onset linearized
   EXPECT_NEAR( 0.77240632040, mcKenzieCalculator->calculateThinningFactorOnsetLinearized( 0.65860948060 ), 1e-11 );
   EXPECT_NEAR( 0.66765115587, mcKenzieCalculator->calculateThinningFactorOnsetLinearized( 0.50147673380 ), 1e-11 );
   EXPECT_EQ( 1, mcKenzieCalculator->calculateThinningFactorOnsetLinearized( 1 ) );

   //3. thinning factor at melt for maximum basalt thickness
   EXPECT_NEAR( 0.90000903190, mcKenzieCalculator->calculateThinningFactorOnsetAtMaxBasalt( 4000, 0.65860948060 ), 1e-11 );
   EXPECT_NEAR( 0.78929927574, mcKenzieCalculator->calculateThinningFactorOnsetAtMaxBasalt( 6000, 0.50147673380 ), 1e-11 );
   EXPECT_EQ( 1, mcKenzieCalculator->calculateThinningFactorOnsetAtMaxBasalt( 0, 1 ) );
}

TEST_F( McKenzieTester, calculates_ttsendmembers )
{
   McKenzieCrustCalculatorPtr mcKenzieCalculator1 = createMcKenzieCalculator();

   // 1. TTS at exhume point
   EXPECT_NEAR( 6920.22280595189, mcKenzieCalculator1->calculateTTSexhume( 80  ), 1e-11 );
   EXPECT_NEAR( 7323.88564105350, mcKenzieCalculator1->calculateTTSexhume( 250 ), 1e-11 );

   // 2. TTS at critical point
   EXPECT_NEAR( 6097.83831174862, mcKenzieCalculator1->calculateTTScritical( 6920.22280595189, 4000, 2833.29679953964 ), 1e-11 );
   EXPECT_NEAR( 6093.52057578229, mcKenzieCalculator1->calculateTTScritical( 7323.88564105350, 6000, 2834.51188363906 ), 1e-11 );
   EXPECT_NEAR( 6920.22280595189, mcKenzieCalculator1->calculateTTScritical( 6920.22280595189, 0,    2833.29679953964 ), 1e-11 );

   // 3. TTS at melting point linearized
   EXPECT_NEAR( 5285.74782707973, mcKenzieCalculator1->calculateTTSOnsetLinearized( 80 , 0.77240632040 ), 1e-07 );
   EXPECT_NEAR( 4888.54662998943, mcKenzieCalculator1->calculateTTSOnsetLinearized( 250, 0.66765115587 ), 1e-07 );
   EXPECT_NEAR( 6920.22280595189, mcKenzieCalculator1->calculateTTSOnsetLinearized( 80,  1             ), 1e-11 );

   // 4. TTS at exhume point with serpentinization of the mantle
   EXPECT_NEAR( 6238.58340595189, mcKenzieCalculator1->calculateTTSexhumeSerpentinized( 6920.22280595189 ), 1e-11 );
   EXPECT_NEAR( 6642.24624105350, mcKenzieCalculator1->calculateTTSexhumeSerpentinized( 7323.88564105350 ), 1e-11 );

   // 5. ITS adjusted by the previous thinning factor
   EXPECT_EQ( 350, mcKenzieCalculator1->calculateITScorrected(350, 0) );
   EXPECT_NEAR( 100, mcKenzieCalculator1->calculateITScorrected(200, 0.5), 1e-11 );

   // 6. ITS stacked for the rift
   // there is no previous rift
   auto save = m_previousRiftITS;
   m_previousRiftITS = nullptr;
   McKenzieCrustCalculatorPtr mcKenzieCalculator2 = createMcKenzieCalculator();
   m_previousRiftITS = save;

   for (unsigned int i = static_cast< unsigned int>( m_grid->firstI() ); i <= static_cast< unsigned int>( m_grid->lastI() ); i++) {
      for (unsigned int j = static_cast< unsigned int>( m_grid->firstJ() ); j <= static_cast< unsigned int>( m_grid->lastJ() ); j++) {
         // both values are valid (previous rift as TTS of 1Km)
         EXPECT_EQ( 1050, mcKenzieCalculator1->calculateRiftITS( i, j, 2050 ) );
         // there is no previous rift
         EXPECT_EQ( 2050, mcKenzieCalculator2->calculateRiftITS( i, j, 2050 ) );
         // the previous rift TTS is a ndv
         m_previousRiftITS->setValues( Interface::DefaultUndefinedMapValue );
         EXPECT_EQ( Interface::DefaultUndefinedMapValue, mcKenzieCalculator1->calculateRiftITS( i, j, 2050 ) );
         // the current rift TTS is a ndv
         m_previousRiftITS->setValues( 1000 );
         EXPECT_EQ( Interface::DefaultUndefinedMapValue, mcKenzieCalculator1->calculateRiftITS( i, j, Interface::DefaultUndefinedMapValue ) );
      }
   }

}

TEST_F( McKenzieTester, define_linearfunction )
{
   McKenzieCrustCalculatorPtr mcKenzieCalculator = createMcKenzieCalculator();
   LinearFunction linearFunction;

   mcKenzieCalculator->defineLinearFunction( linearFunction, 4000, 2833.29679953964, 0.77240632040, 5285.74782707973, 6097.83831174862 );
   EXPECT_EQ( 6097.83831174862, linearFunction.getTTS_crit()                   );
   EXPECT_EQ( 5285.74782707973, linearFunction.getTTS_onset()                  );
   EXPECT_EQ( 4000            , linearFunction.getMaxBasalticCrustThickness()  );
   EXPECT_NEAR( 0.00014613000 , linearFunction.getM1()                 , 1e-09 );
   EXPECT_NEAR( 0.00028025655 , linearFunction.getM2()                 , 1e-09 );
   EXPECT_NEAR( -0.70895914334, linearFunction.getC2()                 , 1e-09 );
   EXPECT_NEAR( 4.86390493522 , linearFunction.getMagmaThicknessCoeff(), 1e-09 );
   EXPECT_NEAR( 0.97258017204 , linearFunction.getCrustTF( 6000 )      , 1e-09 );
   EXPECT_NEAR( 0.03196593710 , linearFunction.getCrustTF( 218.75 )    , 1e-09 );

   mcKenzieCalculator->defineLinearFunction( linearFunction, 6000, 2834.51188363906, 0.66765115587, 4888.54662998943, 6093.52057578229 );
   EXPECT_EQ( 6093.52057578229, linearFunction.getTTS_crit()                   );
   EXPECT_EQ( 4888.54662998943, linearFunction.getTTS_onset()                  );
   EXPECT_EQ( 6000            , linearFunction.getMaxBasalticCrustThickness()  );
   EXPECT_NEAR( 0.00013657457 , linearFunction.getM1()                 , 1e-09 );
   EXPECT_NEAR( 0.00027581413 , linearFunction.getM2()                 , 1e-09 );
   EXPECT_NEAR( -0.68067909446, linearFunction.getC2()                 , 1e-09 );
   EXPECT_NEAR( 4.87660140015 , linearFunction.getMagmaThicknessCoeff(), 1e-09 );
   EXPECT_EQ( 1               , linearFunction.getCrustTF( 6500 )              );

   mcKenzieCalculator->defineLinearFunction( linearFunction, 0, 2830, 1, 6920.22280595189, 6920.22280595189 );
   EXPECT_EQ( 6920.22280595189, linearFunction.getTTS_crit()                   );
   EXPECT_EQ( 6920.22280595189, linearFunction.getTTS_onset()                  );
   EXPECT_EQ( 0               , linearFunction.getMaxBasalticCrustThickness()  );
   EXPECT_NEAR( 0.00014450402 , linearFunction.getM1()                 , 1e-09 );
   EXPECT_EQ( 0               , linearFunction.getM2()                         );
   EXPECT_EQ( 1               , linearFunction.getC2()                         );
   EXPECT_NEAR( 4.82978723404, linearFunction.getMagmaThicknessCoeff(), 1e-09  );
   EXPECT_NEAR( 0.20411192524, linearFunction.getCrustTF( 1412.5 ),     1e-09  );
}

TEST_F( McKenzieTester, calculates_crustthicknesses )
{
   McKenzieCrustCalculatorPtr mcKenzieCalculator = createMcKenzieCalculator();

   // 1. continental crustal thickness at melting point
   EXPECT_NEAR( 9103.74718409551, mcKenzieCalculator->calculateContinentalCrustThicknessOnset( 40000, 0.77240632040 ), 1e-06 );
   EXPECT_NEAR( 9970.46532401437, mcKenzieCalculator->calculateContinentalCrustThicknessOnset( 30000, 0.66765115587 ), 1e-06 );
   EXPECT_EQ( 0, mcKenzieCalculator->calculateContinentalCrustThicknessOnset( 40000, 1 ) );

   // 2. residual depth anomaly
   EXPECT_NEAR( 97.83831174862,   mcKenzieCalculator->calculateResidualDepthAnomaly( 6097.83831174862, 6000 ), 1e-11 );
   EXPECT_NEAR( -406.47942421771, mcKenzieCalculator->calculateResidualDepthAnomaly( 6093.52057578229, 6500 ), 1e-11 );
   EXPECT_NEAR( 1597.83831174862, mcKenzieCalculator->calculateResidualDepthAnomaly( 6097.83831174862, 4500 ), 1e-11 );
   EXPECT_NEAR( 920.22280595189,  mcKenzieCalculator->calculateResidualDepthAnomaly( 6920.22280595189, 6000 ), 1e-11 );

   // 3. continental crustal thickness
   EXPECT_NEAR( 1096.79311836947, mcKenzieCalculator->calculateContinentalCrustalThickness( 0.97258017204, 40000 ), 1e-7 );
   EXPECT_EQ( 0, mcKenzieCalculator->calculateContinentalCrustalThickness( 1 , 30000 ) );
   EXPECT_NEAR( 38721.3625158480,  mcKenzieCalculator->calculateContinentalCrustalThickness( 0.03196593710, 40000 ), 1e-6 );
   EXPECT_NEAR( 31835.52299047280, mcKenzieCalculator->calculateContinentalCrustalThickness( 0.20411192524, 40000 ), 1e-7 );
   EXPECT_EQ( 0, mcKenzieCalculator->calculateContinentalCrustalThickness( 10, 30000 ) );

   // 4. oceanic crustal thickness
   LinearFunction linearFunction;
   mcKenzieCalculator->defineLinearFunction( linearFunction, 4000, 2833.29679953964, 0.77240632040, 5285.74782707973, 6097.83831174862 );
   EXPECT_NEAR( 4518.09157429785, mcKenzieCalculator->calculateOceanicCrustalThickness( 6000,   6920.22280595189, linearFunction, 1000 ), 1e-10 );
   EXPECT_EQ( 150,                mcKenzieCalculator->calculateOceanicCrustalThickness( 218.75, 6920.22280595189, linearFunction, 150  ) );

   mcKenzieCalculator->defineLinearFunction( linearFunction, 6000, 2834.51188363906, 0.66765115587, 4888.54662998943, 6093.52057578229 );
   EXPECT_NEAR( 4017.76187072684, mcKenzieCalculator->calculateOceanicCrustalThickness( 6500, 7323.88564105350, linearFunction, 0 ), 1e-10 );
   EXPECT_EQ( 0,                  mcKenzieCalculator->calculateOceanicCrustalThickness( 7500, 7323.88564105350, linearFunction, 0 ) );

   mcKenzieCalculator->defineLinearFunction( linearFunction, 0, 2830, 1, 6920.22280595189, 6920.22280595189 );
   EXPECT_EQ( 3000, mcKenzieCalculator->calculateOceanicCrustalThickness( 1412.5, 6920.22280595189, linearFunction, 3000 ) );

   // 4. top oceanic crust
   EXPECT_NEAR( 6096.79311836947, mcKenzieCalculator->calculateTopOceanicCrust( 1096.79311836947, 5000 ), 1e-11 );
   EXPECT_EQ( 3500, mcKenzieCalculator->calculateTopOceanicCrust( 0, 3500 ) );
   EXPECT_NEAR( 43721.36251584800, mcKenzieCalculator->calculateTopOceanicCrust( 38721.3625158480, 5000 ), 1e-11 );
   EXPECT_NEAR( 36835.52299047280, mcKenzieCalculator->calculateTopOceanicCrust( 31835.5229904728, 5000 ), 1e-11 );

   // 5. moho
   EXPECT_NEAR( 9614.88469266732, mcKenzieCalculator->calculateMoho( 6096.79311836947, 3518.09157429785 ), 1e-11 );
   EXPECT_NEAR( 7517.76187072684, mcKenzieCalculator->calculateMoho( 3500            , 4017.76187072684 ), 1e-11 );
   EXPECT_NEAR( 43871.3625158480, mcKenzieCalculator->calculateMoho( 43721.36251584800, 150  ), 1e-11 );
   EXPECT_NEAR( 39835.5229904728, mcKenzieCalculator->calculateMoho( 36835.52299047280, 3000 ), 1e-11 );

   // 6. effective crustal thickness
   EXPECT_NEAR( 2320.47714421220, mcKenzieCalculator->calculateEffectiveCrustalThickness( 1096.79311836947, 3518.09157429785, 40000, 115000 ), 1e-11 );
   EXPECT_NEAR( 1205.32856121805, mcKenzieCalculator->calculateEffectiveCrustalThickness( 0               , 4017.76187072684, 30000, 100000 ), 1e-11 );
   EXPECT_NEAR( 38773.5364288915, mcKenzieCalculator->calculateEffectiveCrustalThickness( 38721.3625158480, 150,              40000, 115000 ), 1e-10 );
   EXPECT_NEAR( 32879.0012513424, mcKenzieCalculator->calculateEffectiveCrustalThickness( 31835.5229904728, 3000,             40000, 115000 ), 1e-10 );

   // 7. lower and uper crusts
   double upperContinentalCrust, lowerContinentalCrust;
   mcKenzieCalculator->divideCrust( McKenzieCrustCalculator::CONTINENTAL, 1096.79311836947, upperContinentalCrust, lowerContinentalCrust );
   EXPECT_NEAR( 365.597706123156, upperContinentalCrust, 1e-11 );
   EXPECT_NEAR( 731.195412246313, lowerContinentalCrust, 1e-11 );
   double upperOceanicCrust, lowerOceanicCrust;
   mcKenzieCalculator->divideCrust( McKenzieCrustCalculator::OCEANIC, 4518.09157429785, upperOceanicCrust, lowerOceanicCrust );
   EXPECT_NEAR( 1860.39064824029, upperOceanicCrust, 1e-11 );
   EXPECT_NEAR( 2657.70092605755, lowerOceanicCrust, 1e-11 );
}


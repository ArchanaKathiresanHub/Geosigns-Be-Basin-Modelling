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

// Google test library
#include <gtest/gtest.h>

using namespace CrustalThicknessInterface;
using namespace CrustalThickness;

// CASE 1 : Post-melt (TTSonset<ITS_corrected<TTScritical)
TEST_F( McKenzieTester, computePostMelt )
{
   m_depthBasement->getMockderivedSurfacePropertyPtr()->fill( 5000 );
   m_TRMap  ->setValues( 60     );
   m_T0Map  ->setValues( 100    );
   m_HBuMap ->setValues( 4000   );
   m_HCuMap ->setValues( 40000  );
   m_HLMuMap->setValues( 115000 );
   m_previousContinentalCrustThickness->setValues( 40000 );
   m_previousThinningFactor           ->setValues( 0     );
   m_previousOceanicCrustThickness    ->setValues( 200   );
   McKenzieCrustCalculatorPtr mcKenzieCalculatorPostMelt = createMcKenzieCalculator();
   m_outputData.setMapValues( WLSadjustedMap,                6000 );
   m_outputData.setMapValues( incTectonicSubsidenceAdjusted, 6000 );
   mcKenzieCalculatorPostMelt->compute();

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
         EXPECT_NEAR( 3718.09157429785, m_outputData.getMapValue( thicknessBasaltMap      , i, j ), 1e-10 );
         EXPECT_NEAR( 6096.79311836947, m_outputData.getMapValue( topBasaltMap            , i, j ), 1e-11 );
         EXPECT_NEAR( 9814.88469266732, m_outputData.getMapValue( mohoMap                 , i, j ), 1e-11 );
         EXPECT_NEAR( 2390.04236160351, m_outputData.getMapValue( ECTMap                  , i, j ), 1e-11 );
      }
   }
}

// CASE 2 : Critical (TTS_critical<ITS_corrected)
TEST_F( McKenzieTester, computeCritical )
{
   m_depthBasement->getMockderivedSurfacePropertyPtr()->fill( 3500 );
   m_TRMap  ->setValues( 0      );
   m_T0Map  ->setValues( 500    );
   m_HBuMap ->setValues( 6000   );
   m_HCuMap ->setValues( 30000  );
   m_HLMuMap->setValues( 100000 );
   m_previousContinentalCrustThickness->setValues( 30000 );
   m_previousThinningFactor           ->setValues( 0     );
   m_previousOceanicCrustThickness    ->setValues( 0     );
   McKenzieCrustCalculatorPtr mcKenzieCalculatorCritical = createMcKenzieCalculator();
   m_outputData.setMapValues( WLSadjustedMap,                6500 );
   m_outputData.setMapValues( incTectonicSubsidenceAdjusted, 6500 );
   mcKenzieCalculatorCritical->compute();

   for (unsigned int i = m_firstI; i <= m_lastI; i++)
   {
      for (unsigned int j = m_firstJ; j <= m_lastJ; j++)
      {
         EXPECT_NEAR( 2803.19485045045, m_outputData.getMapValue( estimatedCrustDensityMap, i, j ), 1e-11 );
         EXPECT_NEAR( 2834.51188363906, m_outputData.getMapValue( basaltDensityMap        , i, j ), 1e-11 );
         EXPECT_NEAR( 1281.45723073605, m_outputData.getMapValue( PTaMap                  , i, j ), 1e-11 );
         EXPECT_NEAR( 0.50147673380   , m_outputData.getMapValue( TFOnsetMap              , i, j ), 1e-11 );
         EXPECT_NEAR( 0.78929927574   , m_outputData.getMapValue( TFOnsetMigMap           , i, j ), 1e-11 );
         EXPECT_NEAR( 0.66765115587   , m_outputData.getMapValue( TFOnsetLinMap           , i, j ), 1e-07 );
         EXPECT_NEAR( 7323.88564105350, m_outputData.getMapValue( WLSExhumeMap            , i, j ), 1e-11 );
         EXPECT_NEAR( 6093.52057578229, m_outputData.getMapValue( WLSCritMap              , i, j ), 1e-11 );
         EXPECT_NEAR( 4888.54662998943, m_outputData.getMapValue( WLSOnsetMap             , i, j ), 1e-07 );
         EXPECT_NEAR( 6642.24624105350, m_outputData.getMapValue( WLSExhumeSerpMap        , i, j ), 1e-11 );
         EXPECT_NEAR( 9970.46532401437, m_outputData.getMapValue( thicknessCrustMeltOnset , i, j ), 1e-06 );
         EXPECT_NEAR( 0.00013657457   , m_outputData.getMapValue( slopePreMelt            , i, j ), 1e-09 );
         EXPECT_NEAR( 0.00027581413   , m_outputData.getMapValue( slopePostMelt           , i, j ), 1e-09 );
         EXPECT_NEAR( -0.68067909446  , m_outputData.getMapValue( interceptPostMelt       , i, j ), 1e-09 );
         EXPECT_NEAR( -406.47942421771, m_outputData.getMapValue( RDAadjustedMap          , i, j ), 1e-11 );
         EXPECT_EQ( 1                 , m_outputData.getMapValue( TFMap                   , i, j )        );
         EXPECT_EQ( 0                 , m_outputData.getMapValue( thicknessCrustMap       , i, j )        );
         EXPECT_NEAR( 4017.76187072684, m_outputData.getMapValue( thicknessBasaltMap      , i, j ), 1e-10 );
         EXPECT_EQ( 3500.00000000000  , m_outputData.getMapValue( topBasaltMap            , i, j )        );
         EXPECT_NEAR( 7517.76187072684, m_outputData.getMapValue( mohoMap                 , i, j ), 1e-11 );
         EXPECT_NEAR( 1205.32856121805, m_outputData.getMapValue( ECTMap                  , i, j ), 1e-11 );
      }
   }
}

// CASE 3 : Pre-melt
TEST_F( McKenzieTester, computePreMelt )
{
   m_depthBasement->getMockderivedSurfacePropertyPtr()->fill( 5000 );
   m_TRMap  ->setValues( 60     );
   m_T0Map  ->setValues( 100    );
   m_HBuMap ->setValues( 4000   );
   m_HCuMap ->setValues( 40000  );
   m_HLMuMap->setValues( 115000 );
   m_previousContinentalCrustThickness->setValues( 35000 );
   m_previousThinningFactor           ->setValues( 0.125 );
   m_previousOceanicCrustThickness    ->setValues( 150   );
   McKenzieCrustCalculatorPtr mcKenzieCalculatorPreMelt = createMcKenzieCalculator();
   m_outputData.setMapValues( WLSadjustedMap,                4500 );
   m_outputData.setMapValues( incTectonicSubsidenceAdjusted, 250  );
   mcKenzieCalculatorPreMelt->compute();

   for (unsigned int i = m_firstI; i <= m_lastI; i++)
   {
      for (unsigned int j = m_firstJ; j <= m_lastJ; j++)
      {
         EXPECT_NEAR( 2927.81214864865 , m_outputData.getMapValue( estimatedCrustDensityMap, i, j ), 1e-11 );
         EXPECT_NEAR( 2833.29679953964 , m_outputData.getMapValue( basaltDensityMap        , i, j ), 1e-11 );
         EXPECT_NEAR( 1238.98403752698 , m_outputData.getMapValue( PTaMap                  , i, j ), 1e-11 );
         EXPECT_NEAR( 0.65860948060    , m_outputData.getMapValue( TFOnsetMap              , i, j ), 1e-11 );
         EXPECT_NEAR( 0.90000903190    , m_outputData.getMapValue( TFOnsetMigMap           , i, j ), 1e-11 );
         EXPECT_NEAR( 0.77240632040    , m_outputData.getMapValue( TFOnsetLinMap           , i, j ), 1e-07 );
         EXPECT_NEAR( 6920.22280595189 , m_outputData.getMapValue( WLSExhumeMap            , i, j ), 1e-11 );
         EXPECT_NEAR( 6097.83831174862 , m_outputData.getMapValue( WLSCritMap              , i, j ), 1e-11 );
         EXPECT_NEAR( 5285.74782707973 , m_outputData.getMapValue( WLSOnsetMap             , i, j ), 1e-07 );
         EXPECT_NEAR( 6238.58340595189 , m_outputData.getMapValue( WLSExhumeSerpMap        , i, j ), 1e-11 );
         EXPECT_NEAR( 9103.74718409551 , m_outputData.getMapValue( thicknessCrustMeltOnset , i, j ), 1e-06 );
         EXPECT_NEAR( 0.00014613000    , m_outputData.getMapValue( slopePreMelt            , i, j ), 1e-09 );
         EXPECT_NEAR( 0.00028025655    , m_outputData.getMapValue( slopePostMelt           , i, j ), 1e-09 );
         EXPECT_NEAR( -0.70895914334   , m_outputData.getMapValue( interceptPostMelt       , i, j ), 1e-09 );
         EXPECT_NEAR( 1597.83831174862 , m_outputData.getMapValue( RDAadjustedMap          , i, j ), 1e-11 );
         EXPECT_NEAR( 0.03196593710    , m_outputData.getMapValue( TFMap                   , i, j ), 1e-09 );
         EXPECT_NEAR( 33881.19220136700, m_outputData.getMapValue( thicknessCrustMap       , i, j ), 1e-07 );
         EXPECT_EQ( 150                , m_outputData.getMapValue( thicknessBasaltMap      , i, j )        );
         EXPECT_NEAR( 38881.19220136700, m_outputData.getMapValue( topBasaltMap            , i, j ), 1e-11 );
         EXPECT_NEAR( 39031.19220136700, m_outputData.getMapValue( mohoMap                 , i, j ), 1e-11 );
         EXPECT_NEAR( 33933.36611441050, m_outputData.getMapValue( ECTMap                  , i, j ), 1e-10 );
      }
   }
}

// CASE 4 : No oceanic crust
TEST_F( McKenzieTester, computeNoOceanicCrust )
{
   m_depthBasement->getMockderivedSurfacePropertyPtr()->fill( 5000 );
   m_TRMap  ->setValues( 60     );
   m_T0Map  ->setValues( 100    );
   m_HBuMap ->setValues( 0   );
   m_HCuMap ->setValues( 40000  );
   m_HLMuMap->setValues( 115000 );
   m_previousContinentalCrustThickness->setValues( 10000 );
   m_previousThinningFactor           ->setValues( 0.75  );
   m_previousOceanicCrustThickness    ->setValues( 3000  );
   McKenzieCrustCalculatorPtr mcKenzieCalculatorNoOceanicCrust = createMcKenzieCalculator();
   m_outputData.setMapValues( WLSadjustedMap,                6000 );
   m_outputData.setMapValues( incTectonicSubsidenceAdjusted, 5650 );
   mcKenzieCalculatorNoOceanicCrust->compute();

   for (unsigned int i = m_firstI; i <= m_lastI; i++)
   {
      for (unsigned int j = m_firstJ; j <= m_lastJ; j++)
      {
         EXPECT_NEAR( 2927.81214864865, m_outputData.getMapValue( estimatedCrustDensityMap, i, j ), 1e-11 );
         EXPECT_EQ( 2830              , m_outputData.getMapValue( basaltDensityMap        , i, j )        );
         EXPECT_EQ( 1050              , m_outputData.getMapValue( PTaMap                  , i, j )        );
         EXPECT_EQ( 1                 , m_outputData.getMapValue( TFOnsetMap              , i, j )        );
         EXPECT_EQ( 1                 , m_outputData.getMapValue( TFOnsetMigMap           , i, j )        );
         EXPECT_EQ( 1                 , m_outputData.getMapValue( TFOnsetLinMap           , i, j )        );
         EXPECT_NEAR( 6920.22280595189, m_outputData.getMapValue( WLSExhumeMap            , i, j ), 1e-11 );
         EXPECT_NEAR( 6920.22280595189, m_outputData.getMapValue( WLSCritMap              , i, j ), 1e-11 );
         EXPECT_NEAR( 6920.22280595189, m_outputData.getMapValue( WLSOnsetMap             , i, j ), 1e-07 );
         EXPECT_NEAR( 6238.58340595189, m_outputData.getMapValue( WLSExhumeSerpMap        , i, j ), 1e-11 );
         EXPECT_EQ( 0                 , m_outputData.getMapValue( thicknessCrustMeltOnset , i, j )        );
         EXPECT_NEAR( 0.00014450402   , m_outputData.getMapValue( slopePreMelt            , i, j ), 1e-09 );
         EXPECT_EQ( 0                 , m_outputData.getMapValue( slopePostMelt           , i, j )        );
         EXPECT_EQ( 1                 , m_outputData.getMapValue( interceptPostMelt       , i, j )        );
         EXPECT_NEAR( 920.22280595189 , m_outputData.getMapValue( RDAadjustedMap          , i, j ), 1e-11 );
         EXPECT_NEAR( 0.20411192524   , m_outputData.getMapValue( TFMap                   , i, j ), 1e-09 );
         EXPECT_NEAR( 7958.88074761820, m_outputData.getMapValue( thicknessCrustMap       , i, j ), 1e-07 );
         EXPECT_EQ( 3000.0            , m_outputData.getMapValue( thicknessBasaltMap      , i, j )        );
         EXPECT_NEAR( 12958.8807476182, m_outputData.getMapValue( topBasaltMap            , i, j ), 1e-11 );
         EXPECT_NEAR( 15958.8807476182, m_outputData.getMapValue( mohoMap                 , i, j ), 1e-11 );
         EXPECT_NEAR( 9002.35900848777, m_outputData.getMapValue( ECTMap                  , i, j ), 1e-11 );
      }
   }
}

// CASE 5 : TTS adjusted or ITS adjusted are NDV or basement depth or one
// of the previous data is NDV
TEST_F( McKenzieTester, computeNDV_1 )
{
   m_depthBasement->getMockderivedSurfacePropertyPtr()->fill( 5000 );
   m_depthBasement->getMockderivedSurfacePropertyPtr()->set( 0, 0, Interface::DefaultUndefinedMapValue );
   m_TRMap  ->setValues( 60     );
   m_T0Map  ->setValues( 100    );
   m_HBuMap ->setValues( 4000   );
   m_HCuMap ->setValues( 40000  );
   m_HLMuMap->setValues( 115000 );
   m_previousContinentalCrustThickness->setValues( 40000 );
   m_previousThinningFactor           ->setValues( 0     );
   m_previousOceanicCrustThickness    ->setValues( 200   );
   m_previousContinentalCrustThickness->setValue( 0, 1, Interface::DefaultUndefinedMapValue );
   m_previousThinningFactor           ->setValue( 0, 2, Interface::DefaultUndefinedMapValue );
   m_previousOceanicCrustThickness    ->setValue( 1, 0, Interface::DefaultUndefinedMapValue );
   McKenzieCrustCalculatorPtr mcKenzieCalculatorNDV1 = createMcKenzieCalculator();
   m_outputData.setMapValues( WLSadjustedMap,                6000 );
   m_outputData.setMapValues( incTectonicSubsidenceAdjusted, 6000 );
   m_outputData.setMapValue ( WLSadjustedMap,                1, 1, Interface::DefaultUndefinedMapValue );
   m_outputData.setMapValue ( incTectonicSubsidenceAdjusted, 1, 2, Interface::DefaultUndefinedMapValue );
   mcKenzieCalculatorNDV1->compute();

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
         if ( i == 0 or i == 1 )
         {
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
            EXPECT_NEAR( 97.83831174862  , m_outputData.getMapValue( RDAadjustedMap          , i, j ), 1e-11 );
            EXPECT_NEAR( 0.97258017204   , m_outputData.getMapValue( TFMap                   , i, j ), 1e-09 );
            EXPECT_NEAR( 1096.79311836947, m_outputData.getMapValue( thicknessCrustMap       , i, j ), 1e-07 );
            EXPECT_NEAR( 3718.09157429785, m_outputData.getMapValue( thicknessBasaltMap      , i, j ), 1e-10 );
            EXPECT_NEAR( 6096.79311836947, m_outputData.getMapValue( topBasaltMap            , i, j ), 1e-11 );
            EXPECT_NEAR( 9814.88469266732, m_outputData.getMapValue( mohoMap                 , i, j ), 1e-11 );
            EXPECT_NEAR( 2390.04236160351, m_outputData.getMapValue( ECTMap                  , i, j ), 1e-11 );
         }
      }
   }
}

// CASE 6 : The node is not valid
TEST_F( McKenzieTester, computeNDV_2 )
{
   m_validator.setIsValid( false );
   m_depthBasement->getMockderivedSurfacePropertyPtr()->fill( 5000 );
   m_TRMap  ->setValues( 60     );
   m_T0Map  ->setValues( 100    );
   m_HBuMap ->setValues( 4000   );
   m_HCuMap ->setValues( 40000  );
   m_HLMuMap->setValues( 115000 );
   m_previousContinentalCrustThickness->setValues( 40000 );
   m_previousThinningFactor           ->setValues( 0     );
   m_previousOceanicCrustThickness    ->setValues( 200   );
   McKenzieCrustCalculatorPtr mcKenzieCalculatorNDV2 = createMcKenzieCalculator();
   m_outputData.setMapValues( WLSadjustedMap,                6000 );
   m_outputData.setMapValues( incTectonicSubsidenceAdjusted, 6000 );
   mcKenzieCalculatorNDV2->compute();

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

   m_validator.setIsValid( true );
}

// CASE 7 : The input maps are NDV
TEST_F( McKenzieTester, computeNDV_3 )
{
   m_depthBasement->getMockderivedSurfacePropertyPtr()->fill( 5000 );
   m_TRMap  ->setValues( 60     );
   m_T0Map  ->setValues( 100    );
   m_HBuMap ->setValues( 4000   );
   m_HCuMap ->setValues( 40000  );
   m_HLMuMap->setValues( 115000 );
   m_T0Map  ->setValue( 0, 0, Interface::DefaultUndefinedMapValue );
   m_TRMap  ->setValue( 0, 1, Interface::DefaultUndefinedMapValue );
   m_HCuMap ->setValue( 0, 2, Interface::DefaultUndefinedMapValue );
   m_HBuMap ->setValue( 1, 0, Interface::DefaultUndefinedMapValue );
   m_HLMuMap->setValue( 1, 1, Interface::DefaultUndefinedMapValue );
   m_previousContinentalCrustThickness->setValues( 40000 );
   m_previousThinningFactor           ->setValues( 0     );
   m_previousOceanicCrustThickness    ->setValues( 200   );
   McKenzieCrustCalculatorPtr mcKenzieCalculatorNDV3 = createMcKenzieCalculator();
   m_outputData.setMapValues( WLSadjustedMap,                6000 );
   m_outputData.setMapValues( incTectonicSubsidenceAdjusted, 6000 );
   mcKenzieCalculatorNDV3->compute();

   for (unsigned int i = m_firstI; i <= m_lastI; i++)
   {
      for (unsigned int j = m_firstJ; j <= m_lastJ; j++)
      {

         if ( i == 0 or ( i == 1 and ( j == 0 or j == 1 ) ) )
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
            EXPECT_NEAR( 3718.09157429785, m_outputData.getMapValue( thicknessBasaltMap      , i, j ), 1e-10 );
            EXPECT_NEAR( 6096.79311836947, m_outputData.getMapValue( topBasaltMap            , i, j ), 1e-11 );
            EXPECT_NEAR( 9814.88469266732, m_outputData.getMapValue( mohoMap                 , i, j ), 1e-11 );
            EXPECT_NEAR( 2390.04236160351, m_outputData.getMapValue( ECTMap                  , i, j ), 1e-11 );
         }

      }
   }
}


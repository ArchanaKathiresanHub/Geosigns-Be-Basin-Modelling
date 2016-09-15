//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include "../src/TotalTectonicSubsidenceCalculator.h"

// CrustalThickness library test utilities
#include "MockInterfaceInput.h"
#include "MockInterfaceOutput.h"
#include "MockValidator.h"

// DataAccess libraries
#include "../../SerialDataAccess/src/Interface/SerialGrid.h"
#include "../../SerialDataAccess/src/Interface/SerialGridMap.h"
#include "../../DataAccess/src/Interface/Grid.h"

#include <gtest/gtest.h>

typedef GeoPhysics::Local2DArray <CBMGenerics::Polyfunction> PolyFunction2DArray;

using namespace CrustalThicknessInterface;
using namespace CrustalThickness;

// Global validator
MockValidator validator = MockValidator();

// Create some data before tests
class TTSCalculatorTest : public ::testing::Test
{
public:
   TTSCalculatorTest() :
      m_firstI( 0 ), m_firstJ( 0 ), m_lastI( 1 ), m_lastJ( 1 ),
      m_minI( 0.0 ), m_minJ( 0.0 ), m_maxI( 4.0 ), m_maxJ( 4.0 ),
      m_numI( 2 ), m_numJ( 2 )
   {
      m_grid    = new DataAccess::Interface::SerialGrid( this->m_minI, this->m_minJ, this->m_maxI, this->m_maxJ, this->m_numI, this->m_numJ );
      m_gridMap = new DataAccess::Interface::SerialGridMap( 0, 0, this->m_grid, 0, 1 );
      m_previousTTS           = new DataAccess::Interface::SerialGridMap( 0, 0, this->m_grid, 300 );
      m_previousTTSNDV        = new DataAccess::Interface::SerialGridMap( 0, 0, this->m_grid, Interface::DefaultUndefinedMapValue );
      m_seeLevelAdjustment    = new DataAccess::Interface::SerialGridMap( 0, 0, this->m_grid, 20 );
      m_seeLevelAdjustmentNDV = new DataAccess::Interface::SerialGridMap( 0, 0, this->m_grid, Interface::DefaultUndefinedMapValue );
   }

   ~TTSCalculatorTest()
   {
      delete m_grid;
      delete m_gridMap;
      delete m_previousTTS;
      delete m_previousTTSNDV;
      delete m_seeLevelAdjustment;
      delete m_seeLevelAdjustmentNDV;
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
   const DataAccess::Interface::SerialGridMap* m_gridMap;
   const DataAccess::Interface::SerialGridMap* m_previousTTS;
   const DataAccess::Interface::SerialGridMap* m_previousTTSNDV;
   const DataAccess::Interface::SerialGridMap* m_seeLevelAdjustment;
   const DataAccess::Interface::SerialGridMap* m_seeLevelAdjustmentNDV;

};

///1. Test the total tectonic subsidence calculation
TEST_F( TTSCalculatorTest, total_tectonic_subsidence )
{

   PolyFunction2DArray surfaceDepthHistory;
   surfaceDepthHistory.reallocate( m_grid );
   for (unsigned int i = m_firstI; i <= m_lastI; i++){
      for (unsigned int j = m_firstJ; j <= m_lastJ; j++){
         //age is 0Ma, value is 200m
         surfaceDepthHistory( i, j ).AddPoint( 10, 500 );
      }
   }
   MockInterfaceOutput outputData = MockInterfaceOutput( m_firstI, m_firstJ, m_lastI, m_lastJ );
   MockInterfaceInput inputData = MockInterfaceInput();
   inputData.setT0Map( m_gridMap );
   inputData.setDeltaSLMap( m_seeLevelAdjustment );

   //aircorrection=1.5
   TotalTectonicSubsidenceCalculator ttsCalculator( inputData,
      outputData,
      validator,
      10.0,
      1.5,
      m_previousTTS,
      surfaceDepthHistory );
   ///1. Test the TTS
   //under see level
   EXPECT_EQ( 0,   ttsCalculator.calculateTTS( 0,     0  ) );
   EXPECT_EQ( 400, ttsCalculator.calculateTTS( 500, -100 ) );
   EXPECT_EQ( 200, ttsCalculator.calculateTTS( 600, -400 ) );
   //above see level
   EXPECT_EQ( 5,  ttsCalculator.calculateTTS( -10, 20 ) );
   EXPECT_EQ( 10, ttsCalculator.calculateTTS( -20, 40 ) );
   //tts<0
   EXPECT_EQ( 0, ttsCalculator.calculateTTS( -10, -10 )  );
   EXPECT_EQ( 0, ttsCalculator.calculateTTS( 0,   -10 )  );
   EXPECT_EQ( 0, ttsCalculator.calculateTTS( 20,  -100 ) );
   EXPECT_EQ( 0, ttsCalculator.calculateTTS( 40,  -200 ) );

   ///2. Test the ITS
   EXPECT_EQ( 80,   ttsCalculator.calculateIncrementalTTS( 100, 20    ) );
   EXPECT_EQ( -500, ttsCalculator.calculateIncrementalTTS( 2000, 2500 ) );

   ///3. Test the adjusted TTS ot ITS
   EXPECT_EQ( 80,   ttsCalculator.calculateTSadjusted( 100,  20   ) );
   EXPECT_EQ( 1500, ttsCalculator.calculateTSadjusted( 2000, 500  ) );
   EXPECT_EQ( 0,    ttsCalculator.calculateTSadjusted( 2000, 3000 ) );
}

///2. Test the general calculator computation
TEST_F( TTSCalculatorTest, compute )
{
   MockInterfaceOutput outputData = MockInterfaceOutput( m_firstI, m_firstJ, m_lastI, m_lastJ );
   PolyFunction2DArray surfaceDepthHistory;
   surfaceDepthHistory.reallocate( m_grid );
   for (unsigned int i = m_firstI; i <= m_lastI; i++){
      for (unsigned int j = m_firstJ; j <= m_lastJ; j++){
         //age is 0Ma, value is 200m
         surfaceDepthHistory( i, j ).AddPoint( 10, 500 );
         surfaceDepthHistory( i, j ).AddPoint( 20, Interface::DefaultUndefinedMapValue );
      }
   }
   MockInterfaceInput inputData = MockInterfaceInput();
   inputData.setDeltaSLMap( m_seeLevelAdjustment );
   inputData.setT0Map( m_gridMap );
   outputData.setMapValues( cumSedimentBackstrip, -100 );

   // 1. Test for real backstripvalues
   //aircorrection=1.5
   TotalTectonicSubsidenceCalculator ttsCalculator1( inputData,
      outputData,
      validator,
      10.0,
      1.5,
      m_previousTTS,
      surfaceDepthHistory);
   ttsCalculator1.compute();
   EXPECT_EQ( 400, outputData.getMapValue( WLSMap, 0, 0 ) );
   EXPECT_EQ( 400, outputData.getMapValue( WLSMap, 0, 1 ) );
   EXPECT_EQ( 400, outputData.getMapValue( WLSMap, 1, 0 ) );
   EXPECT_EQ( 400, outputData.getMapValue( WLSMap, 1, 1 ) );
   EXPECT_EQ( 100, outputData.getMapValue( incTectonicSubsidence, 0, 0 ) );
   EXPECT_EQ( 100, outputData.getMapValue( incTectonicSubsidence, 0, 1 ) );
   EXPECT_EQ( 100, outputData.getMapValue( incTectonicSubsidence, 1, 0 ) );
   EXPECT_EQ( 100, outputData.getMapValue( incTectonicSubsidence, 1, 1 ) );
   EXPECT_EQ( 380, outputData.getMapValue( WLSadjustedMap, 0, 0 ) );
   EXPECT_EQ( 380, outputData.getMapValue( WLSadjustedMap, 0, 1 ) );
   EXPECT_EQ( 380, outputData.getMapValue( WLSadjustedMap, 1, 0 ) );
   EXPECT_EQ( 380, outputData.getMapValue( WLSadjustedMap, 1, 1 ) );
   EXPECT_EQ( 100,  outputData.getMapValue( incTectonicSubsidenceAdjusted, 0, 0 ) );
   EXPECT_EQ( 100,  outputData.getMapValue( incTectonicSubsidenceAdjusted, 0, 1 ) );
   EXPECT_EQ( 100,  outputData.getMapValue( incTectonicSubsidenceAdjusted, 1, 0 ) );
   EXPECT_EQ( 100,  outputData.getMapValue( incTectonicSubsidenceAdjusted, 1, 1 ) );

   // 2. Test that the incremental tectonic subsidence output is equal to the first TTS when there is not yet any previous TTS
   //aircorrection=1.5
   TotalTectonicSubsidenceCalculator ttsCalculator2( inputData,
      outputData,
      validator,
      10.0,
      1.5,
      nullptr,
      surfaceDepthHistory);
   ttsCalculator2.compute();
   EXPECT_EQ( 400, outputData.getMapValue( WLSMap, 0, 0 ) );
   EXPECT_EQ( 400, outputData.getMapValue( WLSMap, 0, 1 ) );
   EXPECT_EQ( 400, outputData.getMapValue( WLSMap, 1, 0 ) );
   EXPECT_EQ( 400, outputData.getMapValue( WLSMap, 1, 1 ) );
   EXPECT_EQ( 400, outputData.getMapValue( incTectonicSubsidence, 0, 0 ) );
   EXPECT_EQ( 400, outputData.getMapValue( incTectonicSubsidence, 0, 1 ) );
   EXPECT_EQ( 400, outputData.getMapValue( incTectonicSubsidence, 1, 0 ) );
   EXPECT_EQ( 400, outputData.getMapValue( incTectonicSubsidence, 1, 1 ) );
   EXPECT_EQ( 380, outputData.getMapValue( WLSadjustedMap, 0, 0 ) );
   EXPECT_EQ( 380, outputData.getMapValue( WLSadjustedMap, 0, 1 ) );
   EXPECT_EQ( 380, outputData.getMapValue( WLSadjustedMap, 1, 0 ) );
   EXPECT_EQ( 380, outputData.getMapValue( WLSadjustedMap, 1, 1 ) );
   EXPECT_EQ( 400, outputData.getMapValue( incTectonicSubsidenceAdjusted, 0, 0 ) );
   EXPECT_EQ( 400, outputData.getMapValue( incTectonicSubsidenceAdjusted, 0, 1 ) );
   EXPECT_EQ( 400, outputData.getMapValue( incTectonicSubsidenceAdjusted, 1, 0 ) );
   EXPECT_EQ( 400, outputData.getMapValue( incTectonicSubsidenceAdjusted, 1, 1 ) );

   // 3. Test that the incremental tectonic subsidence are NDV if the previous TTS is undefined
   //aircorrection=1.5
   TotalTectonicSubsidenceCalculator ttsCalculator3( inputData,
      outputData,
      validator,
      10.0,
      1.5,
      m_previousTTSNDV,
      surfaceDepthHistory );
   ttsCalculator3.compute();
   EXPECT_EQ( 400, outputData.getMapValue( WLSMap, 0, 0 ) );
   EXPECT_EQ( 400, outputData.getMapValue( WLSMap, 0, 1 ) );
   EXPECT_EQ( 400, outputData.getMapValue( WLSMap, 1, 0 ) );
   EXPECT_EQ( 400, outputData.getMapValue( WLSMap, 1, 1 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( incTectonicSubsidence, 0, 0 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( incTectonicSubsidence, 0, 1 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( incTectonicSubsidence, 1, 0 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( incTectonicSubsidence, 1, 1 ) );
   EXPECT_EQ( 380, outputData.getMapValue( WLSadjustedMap, 0, 0 ) );
   EXPECT_EQ( 380, outputData.getMapValue( WLSadjustedMap, 0, 1 ) );
   EXPECT_EQ( 380, outputData.getMapValue( WLSadjustedMap, 1, 0 ) );
   EXPECT_EQ( 380, outputData.getMapValue( WLSadjustedMap, 1, 1 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( incTectonicSubsidenceAdjusted, 0, 0 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( incTectonicSubsidenceAdjusted, 0, 1 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( incTectonicSubsidenceAdjusted, 1, 0 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( incTectonicSubsidenceAdjusted, 1, 1 ) );

   // 4. Test that the outputs are NDV when the backstrip is undefined
   //aircorrection=1.5
   outputData.setMapValues( cumSedimentBackstrip,Interface::DefaultUndefinedMapValue );
   ttsCalculator1.compute();
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( WLSMap, 0, 0 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( WLSMap, 0, 1 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( WLSMap, 1, 0 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( WLSMap, 1, 1 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( incTectonicSubsidence, 0, 0 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( incTectonicSubsidence, 0, 1 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( incTectonicSubsidence, 1, 0 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( incTectonicSubsidence, 1, 1 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( WLSadjustedMap, 0, 0 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( WLSadjustedMap, 0, 1 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( WLSadjustedMap, 1, 0 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( WLSadjustedMap, 1, 1 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( incTectonicSubsidenceAdjusted, 0, 0 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( incTectonicSubsidenceAdjusted, 0, 1 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( incTectonicSubsidenceAdjusted, 1, 0 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( incTectonicSubsidenceAdjusted, 1, 1 ) );

   // 4. Test that the outputs are NDV when the nodes are not valid
   //aircorrection=1.5
   validator.setIsValid( false );
   outputData.setMapValues( cumSedimentBackstrip, -100 );
   ttsCalculator1.compute();
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( WLSMap, 0, 0 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( WLSMap, 0, 1 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( WLSMap, 1, 0 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( WLSMap, 1, 1 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( incTectonicSubsidence, 0, 0 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( incTectonicSubsidence, 0, 1 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( incTectonicSubsidence, 1, 0 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( incTectonicSubsidence, 1, 1 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( incTectonicSubsidence, 1, 1 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( WLSadjustedMap, 0, 0 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( WLSadjustedMap, 0, 1 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( WLSadjustedMap, 1, 0 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( WLSadjustedMap, 1, 1 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( incTectonicSubsidenceAdjusted, 0, 0 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( incTectonicSubsidenceAdjusted, 0, 1 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( incTectonicSubsidenceAdjusted, 1, 0 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( incTectonicSubsidenceAdjusted, 1, 1 ) );

   // 5. Test that the outputs are NDV when the depth water bottom is undefined
   //aircorrection=1.5
   validator.setIsValid( true );
   TotalTectonicSubsidenceCalculator ttsCalculator4( inputData,
      outputData,
      validator,
      20.0,
      1.5,
      m_previousTTS,
      surfaceDepthHistory);
   ttsCalculator4.compute();
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( WLSMap, 0, 0 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( WLSMap, 0, 1 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( WLSMap, 1, 0 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( WLSMap, 1, 1 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( incTectonicSubsidence, 0, 0 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( incTectonicSubsidence, 0, 1 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( incTectonicSubsidence, 1, 0 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( incTectonicSubsidence, 1, 1 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( incTectonicSubsidence, 1, 1 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( WLSadjustedMap, 0, 0 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( WLSadjustedMap, 0, 1 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( WLSadjustedMap, 1, 0 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( WLSadjustedMap, 1, 1 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( incTectonicSubsidenceAdjusted, 0, 0 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( incTectonicSubsidenceAdjusted, 0, 1 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( incTectonicSubsidenceAdjusted, 1, 0 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( incTectonicSubsidenceAdjusted, 1, 1 ) );


   // 6. Test that the adjusted TTS are NDV when the see level adjustment is undefined
   //aircorrection=1.5
   inputData.setDeltaSLMap( m_seeLevelAdjustmentNDV );
   TotalTectonicSubsidenceCalculator ttsCalculator5( inputData,
      outputData,
      validator,
      10.0,
      1.5,
      m_previousTTS,
      surfaceDepthHistory );
   ttsCalculator5.compute();
   EXPECT_EQ( 400, outputData.getMapValue( WLSMap, 0, 0 ) );
   EXPECT_EQ( 400, outputData.getMapValue( WLSMap, 0, 1 ) );
   EXPECT_EQ( 400, outputData.getMapValue( WLSMap, 1, 0 ) );
   EXPECT_EQ( 400, outputData.getMapValue( WLSMap, 1, 1 ) );
   EXPECT_EQ( 100, outputData.getMapValue( incTectonicSubsidence, 0, 0 ) );
   EXPECT_EQ( 100, outputData.getMapValue( incTectonicSubsidence, 0, 1 ) );
   EXPECT_EQ( 100, outputData.getMapValue( incTectonicSubsidence, 1, 0 ) );
   EXPECT_EQ( 100, outputData.getMapValue( incTectonicSubsidence, 1, 1 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( WLSadjustedMap, 0, 0 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( WLSadjustedMap, 0, 1 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( WLSadjustedMap, 1, 0 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( WLSadjustedMap, 1, 1 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( incTectonicSubsidenceAdjusted, 0, 0 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( incTectonicSubsidenceAdjusted, 0, 1 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( incTectonicSubsidenceAdjusted, 1, 0 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( incTectonicSubsidenceAdjusted, 1, 1 ) );

}


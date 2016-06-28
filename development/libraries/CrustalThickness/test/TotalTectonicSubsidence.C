//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include "MockInterfaceOutput.h"
#include "MockValidator.h"
#include "../src/TotalTectonicSubsidenceCalculator.h"

// DataAccess libraries
#include "../../SerialDataAccess/src/Interface/SerialGrid.h"
#include "../../SerialDataAccess/src/Interface/SerialGridMap.h"
#include "../../DataAccess/src/Interface/Grid.h"

#include <gtest/gtest.h>

typedef GeoPhysics::Local2DArray <CBMGenerics::Polyfunction> PolyFunction2DArray;

using namespace CrustalThicknessInterface;

// Global validator
MockValidator validator = MockValidator();

// Global grid size variables (no gosth nodes)
unsigned int firstI = 0;
unsigned int firstJ = 0;
unsigned int lastI = 1;
unsigned int lastJ = 1;

double minI = 0.0;
double minJ = 0.0;
double maxI = 4.0;
double maxJ = 4.0;
unsigned int numI = 2;
unsigned int numJ = 2;

// Define grid
const DataAccess::Interface::SerialGrid grid( minI, minJ, maxI, maxJ, numI, numJ );
DataAccess::Interface::SerialGridMap previousTTS ( 0, 0, &grid, 300 );

#include <gtest/gtest.h>

///1. Test the total tectonic subsidence calculation
TEST( TotalTectonicSubsidenceCalculator, total_tectonic_subsidence )
{

   PolyFunction2DArray surfaceDepthHistory;
   surfaceDepthHistory.reallocate( &grid );
   for (unsigned int i = firstI; i <= lastI; i++){
      for (unsigned int j = firstJ; j <= lastJ; j++){
         //age is 0Ma, value is 200m
         surfaceDepthHistory( i, j ).AddPoint( 10, 500 );
      }
   }

   //aircorrection=1.5
   MockInterfaceOutput outputData = MockInterfaceOutput( firstI, firstJ, lastI, lastJ );
   TotalTectonicSubsidenceCalculator ttsCalculator( firstI,
                                                    firstJ,
                                                    lastI,
                                                    lastJ,
                                                    10.0,
                                                    1.5,
                                                    &previousTTS,
                                                    surfaceDepthHistory,
                                                    outputData,
                                                    validator );
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

   ///2. Test the incremental TTS
   EXPECT_EQ( 80,   ttsCalculator.calculateIncrementalTTS( 100, 20    ) );
   EXPECT_EQ( -500, ttsCalculator.calculateIncrementalTTS( 2000, 2500 ) );

}

///2. Test the general calculator computation
TEST( TotalTectonicSubsidenceCalculator, compute )
{
   MockInterfaceOutput outputData = MockInterfaceOutput( firstI, firstJ, lastI, lastJ );
   PolyFunction2DArray surfaceDepthHistory;
   surfaceDepthHistory.reallocate( &grid );
   for (unsigned int i = firstI; i <= lastI; i++){
      for (unsigned int j = firstJ; j <= lastJ; j++){
         //age is 0Ma, value is 200m
         surfaceDepthHistory( i, j ).AddPoint( 10, 500 );
         surfaceDepthHistory( i, j ).AddPoint( 20, Interface::DefaultUndefinedMapValue );
      }
   }

   // 1. Test for real backstripvalues
   //aircorrection=1.5
   outputData.setMapValues( cumSedimentBackstrip, -100 );
   TotalTectonicSubsidenceCalculator ttsCalculator1( firstI,
                                                     firstJ,
                                                     lastI,
                                                     lastJ,
                                                     10.0,
                                                     1.5,
                                                     &previousTTS,
                                                     surfaceDepthHistory,
                                                     outputData,
                                                     validator );
   ttsCalculator1.compute();
   EXPECT_EQ( 400, outputData.getMapValue( WLSMap, 0, 0 ) );
   EXPECT_EQ( 400, outputData.getMapValue( WLSMap, 0, 1 ) );
   EXPECT_EQ( 400, outputData.getMapValue( WLSMap, 1, 0 ) );
   EXPECT_EQ( 400, outputData.getMapValue( WLSMap, 1, 1 ) );
   EXPECT_EQ( 100, outputData.getMapValue( incTectonicSubsidence, 0, 0 ) );
   EXPECT_EQ( 100, outputData.getMapValue( incTectonicSubsidence, 0, 1 ) );
   EXPECT_EQ( 100, outputData.getMapValue( incTectonicSubsidence, 1, 0 ) );
   EXPECT_EQ( 100, outputData.getMapValue( incTectonicSubsidence, 1, 1 ) );

   // 2. Test that the incremental tectonic subsidence output is equal to 0  when there is not yet any previous TTS
   //aircorrection=1.5
   outputData.setMapValues( cumSedimentBackstrip, -100 );
   TotalTectonicSubsidenceCalculator ttsCalculator2( firstI,
                                                     firstJ,
                                                     lastI,
                                                     lastJ,
                                                     10.0,
                                                     1.5,
                                                     nullptr,
                                                     surfaceDepthHistory,
                                                     outputData,
                                                     validator );
   ttsCalculator2.compute();
   EXPECT_EQ( 400, outputData.getMapValue( WLSMap, 0, 0 ) );
   EXPECT_EQ( 400, outputData.getMapValue( WLSMap, 0, 1 ) );
   EXPECT_EQ( 400, outputData.getMapValue( WLSMap, 1, 0 ) );
   EXPECT_EQ( 400, outputData.getMapValue( WLSMap, 1, 1 ) );
   EXPECT_EQ( 0, outputData.getMapValue( incTectonicSubsidence, 0, 0 ) );
   EXPECT_EQ( 0, outputData.getMapValue( incTectonicSubsidence, 0, 1 ) );
   EXPECT_EQ( 0, outputData.getMapValue( incTectonicSubsidence, 1, 0 ) );
   EXPECT_EQ( 0, outputData.getMapValue( incTectonicSubsidence, 1, 1 ) );

   // 3. Test that the outputs are NDV when the backstrip is undefined
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

   // 5. Test that the outputs are NDV when the depth water bottom is undefined
   //aircorrection=1.5
   outputData.setMapValues( cumSedimentBackstrip, -100 ); 
   TotalTectonicSubsidenceCalculator ttsCalculator3( firstI,
                                                     firstJ,
                                                     lastI,
                                                     lastJ,
                                                     20.0,
                                                     1.5,
                                                     &previousTTS,
                                                     surfaceDepthHistory,
                                                     outputData,
                                                     validator );
   ttsCalculator3.compute();
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( WLSMap, 0, 0 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( WLSMap, 0, 1 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( WLSMap, 1, 0 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( WLSMap, 1, 1 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( incTectonicSubsidence, 0, 0 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( incTectonicSubsidence, 0, 1 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( incTectonicSubsidence, 1, 0 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( incTectonicSubsidence, 1, 1 ) );

}


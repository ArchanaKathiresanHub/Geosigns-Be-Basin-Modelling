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

// Derived Properties library
#include "../../DerivedProperties/test/MockDerivedSurfaceProperty.h"

#include <gtest/gtest.h>

using namespace CrustalThicknessInterface;

// Global validator
MockValidator validator = MockValidator();

// Global grid size variables (no gosth nodes)
unsigned int firstI = 2;
unsigned int firstJ = 2;
unsigned int lastI = 3;
unsigned int lastJ = 3;

// Define derived property
const DataModel::MockDerivedSurfaceProperty depthWaterBottom( firstI, firstJ, firstI, firstJ, lastI, lastJ, lastI, lastJ, "depthWaterBottom", "Depth", 10, 500 );

#include <gtest/gtest.h>

///1. Test the total tectonic subsidence calculation
TEST( TotalTectonicSubsidenceCalculator, total_tectonic_subsidence )
{
   //aircorrection=1.5
   MockInterfaceOutput outputData = MockInterfaceOutput( firstI, firstJ, lastI, lastJ );
   TotalTectonicSubsidenceCalculator ttsCalculator( firstI,
                                                    firstJ,
                                                    lastI,
                                                    lastJ,
                                                    1.5,
                                                    depthWaterBottom.getMockderivedSurfacePropertyPtr(),
                                                    outputData,
                                                    validator );

   //under see level
   EXPECT_EQ( 0,   ttsCalculator.calculateTTS( 0,     0  ) );
   EXPECT_EQ( 400, ttsCalculator.calculateTTS( 500, -100 ) );
   EXPECT_EQ( 200, ttsCalculator.calculateTTS( 600, -400 ) );
   //above see level
   EXPECT_EQ( 5,  ttsCalculator.calculateTTS( -10, 20 ) );
   EXPECT_EQ( 10, ttsCalculator.calculateTTS( -20, 40 ) );
   //tts<0
   EXPECT_EQ( 0, ttsCalculator.calculateTTS( -10, -10 ) );
   EXPECT_EQ( 0, ttsCalculator.calculateTTS( 0,   -10 ) );
   EXPECT_EQ( 0, ttsCalculator.calculateTTS( 20,  -100 ) );
   EXPECT_EQ( 0, ttsCalculator.calculateTTS( 40,  -200 ) );

}

///2. Test the general calculator computation
TEST( TotalTectonicSubsidenceCalculator, compute )
{
   MockInterfaceOutput outputData = MockInterfaceOutput( firstI, firstJ, lastI, lastJ );

   // 1. Test for real backstripvalues
   //aircorrection=1.5
   outputData.setMapValues( cumSedimentBackstrip, -100 );
   TotalTectonicSubsidenceCalculator ttsCalculator1( firstI,
                                                     firstJ,
                                                     lastI,
                                                     lastJ,
                                                     1.5,
                                                     depthWaterBottom.getMockderivedSurfacePropertyPtr(),
                                                     outputData,
                                                     validator );
   ttsCalculator1.compute();
   EXPECT_EQ( 400, outputData.getMapValue(WLSMap, 2, 2) );
   EXPECT_EQ( 400, outputData.getMapValue(WLSMap, 2, 3) );
   EXPECT_EQ( 400, outputData.getMapValue(WLSMap, 3, 2) );
   EXPECT_EQ( 400, outputData.getMapValue(WLSMap, 3, 3) );

   // 2. Test that the outputs are NDV when the backstrip is undefined
   //aircorrection=1.5
   outputData.setMapValues( cumSedimentBackstrip,Interface::DefaultUndefinedMapValue );
   ttsCalculator1.compute();
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( WLSMap, 2, 2 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( WLSMap, 2, 3 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( WLSMap, 3, 2 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( WLSMap, 3, 3 ) );

   // 3. Test that the outputs are NDV when the nodes are not valid
   //aircorrection=1.5
   validator.setIsValid( false );
   outputData.setMapValues( cumSedimentBackstrip, -100 );
   ttsCalculator1.compute();
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( WLSMap, 2, 2 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( WLSMap, 2, 3 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( WLSMap, 3, 2 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( WLSMap, 3, 3 ) );

   // 4. Test that the outputs are NDV when the depth water bottom is undefined
   //aircorrection=1.5
   const DataModel::MockDerivedSurfaceProperty depthWaterBottomNDV( firstI, firstJ, firstI, firstJ, lastI, lastJ, lastI, lastJ, "depthWaterBottom", "Depth", 10, Interface::DefaultUndefinedMapValue );
   outputData.setMapValues( cumSedimentBackstrip, -100 ); 
   TotalTectonicSubsidenceCalculator ttsCalculator2( firstI,
                                                     firstJ,
                                                     lastI,
                                                     lastJ,
                                                     1.5,
                                                     depthWaterBottomNDV.getMockderivedSurfacePropertyPtr(),
                                                     outputData,
                                                     validator );
   ttsCalculator2.compute();
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( WLSMap, 2, 2 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( WLSMap, 2, 3 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( WLSMap, 3, 2 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( WLSMap, 3, 3 ) );

}


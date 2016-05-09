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
#include "../src/PaleowaterdepthResidualCalculator.h"

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
unsigned int lastI  = 1;
unsigned int lastJ  = 1;

double minI = 0.0;
double minJ = 0.0;
double maxI = 4.0;
double maxJ = 4.0;
unsigned int numI = 2;
unsigned int numJ = 2;

 // Define grid
const DataAccess::Interface::SerialGrid grid( minI, minJ, maxI, maxJ, numI, numJ );

///1. Test the paleowaterdepth residual computation
TEST( PaleowaterdepthResidualCalculator, paleowaterdepth_residual )
{
   MockInterfaceOutput outputData = MockInterfaceOutput( firstI, firstJ, lastI, lastJ );
   PolyFunction2DArray surfaceDepthHistory;
   surfaceDepthHistory.reallocate( &grid );
   for (unsigned int i = firstI; i <= lastI; i++){
      for (unsigned int j = firstJ; j <= lastJ; j++){
         //age is 0Ma, value is 200m
         surfaceDepthHistory( i, j ).AddPoint( 0, 200 );
      }
   }

   //age is 0Ma
   PaleowaterdepthResidualCalculator pwdrCalculator( firstI,
                                                     firstJ,
                                                     lastI,
                                                     lastJ,
                                                     0,
                                                     surfaceDepthHistory,
                                                     outputData,
                                                     validator );
   EXPECT_EQ( 200,  pwdrCalculator.calculatePWDR( 500, 300 ) );
   EXPECT_EQ( -200, pwdrCalculator.calculatePWDR( 300, 500 ) );
   EXPECT_EQ( 0,    pwdrCalculator.calculatePWDR( 50,  50  ) );

}

///2. Test the general calculator computation
TEST( PaleowaterdepthResidualCalculator, compute )
{
   MockInterfaceOutput outputData = MockInterfaceOutput( firstI, firstJ, lastI, lastJ );
   outputData.setMapValues( isostaticBathymetry, 500 );
   PolyFunction2DArray surfaceDepthHistory;
   surfaceDepthHistory.reallocate( &grid );
   for (unsigned int i = firstI; i <= lastI; i++){
      for (unsigned int j = firstJ; j <= lastJ; j++){
         //age is 0Ma, value is 200m
         surfaceDepthHistory( i, j ).AddPoint( 0, 200 );
      }
   }

   //Regular values (age is 0Ma)
   PaleowaterdepthResidualCalculator pwdrCalculator( firstI,
                                                     firstJ,
                                                     lastI,
                                                     lastJ,
                                                     0,
                                                     surfaceDepthHistory,
                                                     outputData,
                                                     validator );
   pwdrCalculator.compute();
   EXPECT_EQ( 300, outputData.getMapValue( PaleowaterdepthResidual, 0, 0 ) );
   EXPECT_EQ( 300, outputData.getMapValue( PaleowaterdepthResidual, 0, 1 ) );
   EXPECT_EQ( 300, outputData.getMapValue( PaleowaterdepthResidual, 1, 0 ) );
   EXPECT_EQ( 300, outputData.getMapValue( PaleowaterdepthResidual, 0, 0 ) );

   //NDV (age is 0Ma)
   // unvalid PWD
   outputData.setMapValues( isostaticBathymetry, Interface::DefaultUndefinedMapValue );
   pwdrCalculator.compute();
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( PaleowaterdepthResidual, 0, 0 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( PaleowaterdepthResidual, 0, 1 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( PaleowaterdepthResidual, 1, 0 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( PaleowaterdepthResidual, 0, 0 ) );
   // unvalid nodes
   validator.setIsValid( false );
   pwdrCalculator.compute();
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( PaleowaterdepthResidual, 0, 0 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( PaleowaterdepthResidual, 0, 1 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( PaleowaterdepthResidual, 1, 0 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( PaleowaterdepthResidual, 0, 0 ) );

}


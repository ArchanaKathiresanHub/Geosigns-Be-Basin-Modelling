//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include "../src/PaleowaterdepthResidualCalculator.h"

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

// Global validator
MockValidator validator = MockValidator();

// Create some data before TEST_Fs
class PWDRCalculatorTest : public ::testing::Test
{
public:
   PWDRCalculatorTest() :
      m_firstI( 0 ), m_firstJ( 0 ), m_lastI( 1 ), m_lastJ( 1 ),
      m_minI( 0.0 ), m_minJ( 0.0 ), m_maxI( 4.0 ), m_maxJ( 4.0 ),
      m_numI( 2 ), m_numJ( 2 )
   {
      m_grid    = new DataAccess::Interface::SerialGrid( this->m_minI, this->m_minJ, this->m_maxI, this->m_maxJ, this->m_numI, this->m_numJ );
      m_gridMap = new DataAccess::Interface::SerialGridMap( 0, 0, this->m_grid, 0, 1 );
   }

   ~PWDRCalculatorTest()
   {
      delete m_grid;
      delete m_gridMap;
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

};

///1. Test the paleowaterdepth residual computation
TEST_F( PWDRCalculatorTest, paleowaterdepth_residual )
{
   MockInterfaceOutput outputData = MockInterfaceOutput( m_firstI, m_firstJ, m_lastI, m_lastJ );
   PolyFunction2DArray surfaceDepthHistory;
   surfaceDepthHistory.reallocate( m_grid );
   for (unsigned int i = m_firstI; i <= m_lastI; i++){
      for (unsigned int j = m_firstJ; j <= m_lastJ; j++){
         //age is 0Ma, value is 200m
         surfaceDepthHistory( i, j ).AddPoint( 0, 200 );
      }
   }
   MockInterfaceInput inputData = MockInterfaceInput();
   inputData.setT0Map( m_gridMap );

   //age is 0Ma
   PaleowaterdepthResidualCalculator pwdrCalculator( inputData,
      outputData,
      validator,
      0,
      surfaceDepthHistory);
   EXPECT_EQ( 200,  pwdrCalculator.calculatePWDR( 500, 300 ) );
   EXPECT_EQ( -200, pwdrCalculator.calculatePWDR( 300, 500 ) );
   EXPECT_EQ( 0,    pwdrCalculator.calculatePWDR( 50,  50  ) );

}

///2. Test the general calculator computation
TEST_F( PWDRCalculatorTest, compute )
{
   MockInterfaceOutput outputData = MockInterfaceOutput( m_firstI, m_firstJ, m_lastI, m_lastJ );
   outputData.setMapValues( isostaticBathymetry, 500 );
   PolyFunction2DArray surfaceDepthHistory;
   surfaceDepthHistory.reallocate( m_grid );
   for (unsigned int i = m_firstI; i <= m_lastI; i++){
      for (unsigned int j = m_firstJ; j <= m_lastJ; j++){
         //age is 0Ma, value is 200m
         surfaceDepthHistory( i, j ).AddPoint( 0, 200 );
      }
   }

   MockInterfaceInput inputData = MockInterfaceInput();
   inputData.setT0Map( m_gridMap );

   //Regular values (age is 0Ma)
   PaleowaterdepthResidualCalculator pwdrCalculator( inputData,
      outputData,
      validator,
      0,
      surfaceDepthHistory );
   pwdrCalculator.compute();
   EXPECT_EQ( 300, outputData.getMapValue( PaleowaterdepthResidual, 0, 0 ) );
   EXPECT_EQ( 300, outputData.getMapValue( PaleowaterdepthResidual, 0, 1 ) );
   EXPECT_EQ( 300, outputData.getMapValue( PaleowaterdepthResidual, 1, 0 ) );
   EXPECT_EQ( 300, outputData.getMapValue( PaleowaterdepthResidual, 1, 1 ) );

   //NDV (age is 0Ma)
   // unvalid PWD
   outputData.setMapValues( isostaticBathymetry, Interface::DefaultUndefinedMapValue );
   pwdrCalculator.compute();
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( PaleowaterdepthResidual, 0, 0 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( PaleowaterdepthResidual, 0, 1 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( PaleowaterdepthResidual, 1, 0 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( PaleowaterdepthResidual, 1, 1 ) );
   // unvalid nodes
   validator.setIsValid( false );
   outputData.setMapValues( isostaticBathymetry, 500 );
   pwdrCalculator.compute();
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( PaleowaterdepthResidual, 0, 0 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( PaleowaterdepthResidual, 0, 1 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( PaleowaterdepthResidual, 1, 0 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( PaleowaterdepthResidual, 1, 1 ) );

}


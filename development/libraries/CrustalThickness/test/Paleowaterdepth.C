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
#include "../src/PaleowaterdepthCalculator.h"

// Derived Properties library
#include "../../DerivedProperties/test/MockDerivedSurfaceProperty.h"

// utility library
#include "../../utilities/src/FormattingException.h"

// DataAccess libraries
#include "../../SerialDataAccess/src/Interface/SerialGrid.h"
#include "../../SerialDataAccess/src/Interface/SerialGridMap.h"
#include "../../DataAccess/src/Interface/Grid.h"

#include <gtest/gtest.h>

typedef formattingexception::GeneralException PaleowaterdepthException;

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

// Define grids and derived properties
DataModel::MockDerivedSurfaceProperty currentPressureMantle   ( firstI, firstJ, firstI, firstJ, lastI, lastJ, lastI, lastJ, "Mantle", "Pressure", 10, 10000 );
DataModel::MockDerivedSurfaceProperty presentDayPressureMantle( firstI, firstJ, firstI, firstJ, lastI, lastJ, lastI, lastJ, "Mantle", "Pressure", 0,  5000  );
DataAccess::Interface::SerialGrid grid( minI, minJ, maxI, maxJ, numI, numJ );
DataAccess::Interface::SerialGridMap gridMapPresentDayPressureTTS   ( 0, 0, &grid, 1000 );
DataAccess::Interface::SerialGridMap gridMapPresentDayPressureTTSNDV( 0, 0, &grid, Interface::DefaultUndefinedMapValue );
DataAccess::Interface::SerialGridMap gridMapCurrentPressureTTS      ( 0, 0, &grid, 2000 );

///1. Test the paleowaterdepth calculation
TEST( PaleowaterdepthCalculator, paleowaterdepth )
{

   MockInterfaceOutput outputData = MockInterfaceOutput( firstI, firstJ, lastI, lastJ );

   //With pressure equlibrium
   PaleowaterdepthCalculator pwdCalculator1( firstI,
                                             firstJ,
                                             lastI,
                                             lastJ,
                                             3000,
                                             1000,
                                             outputData,
                                             validator,
                                             presentDayPressureMantle.getMockderivedSurfacePropertyPtr(),
                                             currentPressureMantle.getMockderivedSurfacePropertyPtr(),
                                             &gridMapPresentDayPressureTTS,
                                             &gridMapCurrentPressureTTS );
   EXPECT_NEAR( 800.203873598369,  pwdCalculator1.calculatePWD( 1000, 200,   5000, 1000, 10000, 2000 ), 1e-12 );
   EXPECT_NEAR( 3000.183486238530, pwdCalculator1.calculatePWD( 1000, -2000, 6000, 700,  9000,  100 ),  1e-11 );
   EXPECT_NEAR( -0.089194699286,   pwdCalculator1.calculatePWD( 0,    0,     4000, 350,  2000,  100 ),  1e-12 );
   
   //Without pressure equilibirium
   PaleowaterdepthCalculator pwdCalculator2( firstI,
                                             firstJ,
                                             lastI,
                                             lastJ,
                                             3000,
                                             1000,
                                             outputData,
                                             validator );
   EXPECT_EQ( 700,   pwdCalculator2.calculatePWD( 500,  -200  ) );
   EXPECT_EQ( 800  , pwdCalculator2.calculatePWD( 1000, 200   ) );
   EXPECT_EQ( 0,     pwdCalculator2.calculatePWD( 0,    0     ) );
   
   //Unvalid constructor
   try{
      PaleowaterdepthCalculator pwdCalculator3( firstI,
                                                firstJ,
                                                lastI,
                                                lastJ,
                                                3000,
                                                3000,
                                                outputData,
                                                validator );
      FAIL() << "Expected 'The water density is equal to the mantle density in the Paleowaterdepth calculator but they should be different.' exception";
   }
   catch (const PaleowaterdepthException& ex) {
      EXPECT_EQ( "The water density is equal to the mantle density in the Paleowaterdepth calculator but they should be different.", std::string( ex.what() ) );
   }
   catch (...) {
      FAIL() << "Expected 'The water density is equal to the mantle density in the Paleowaterdepth calculator but they should be different.' exception";
   }

}

///2. Test the general calculator computation
TEST( PaleowaterdepthCalculator, compute )
{
   //Without pressure equlibrium
   MockInterfaceOutput outputData = MockInterfaceOutput( firstI, firstJ, lastI, lastJ );
   outputData.setMapValues( WLSMap, 1000 );
   outputData.setMapValues( cumSedimentBackstrip, 200 );
   PaleowaterdepthCalculator pwdCalculator1( firstI,
                                             firstJ,
                                             lastI,
                                             lastJ,
                                             3000,
                                             1000,
                                             outputData,
                                             validator );
   pwdCalculator1.compute();
   EXPECT_EQ( 800, outputData.getMapValue( isostaticBathymetry, 0, 0 ) );
   EXPECT_EQ( 800, outputData.getMapValue( isostaticBathymetry, 0, 1 ) );
   EXPECT_EQ( 800, outputData.getMapValue( isostaticBathymetry, 1, 0 ) );
   EXPECT_EQ( 800, outputData.getMapValue( isostaticBathymetry, 0, 0 ) );

   //Without pressure equilibirium
   // and defined data
   PaleowaterdepthCalculator pwdCalculator2( firstI,
                                             firstJ,
                                             lastI,
                                             lastJ,
                                             3000,
                                             1000,
                                             outputData,
                                             validator,
                                             presentDayPressureMantle.getMockderivedSurfacePropertyPtr(),
                                             currentPressureMantle.getMockderivedSurfacePropertyPtr(),
                                             &gridMapPresentDayPressureTTS,
                                             &gridMapCurrentPressureTTS );
   pwdCalculator2.compute();
   EXPECT_NEAR( 800.203873598369, outputData.getMapValue( isostaticBathymetry, 0, 0 ), 1e-12 );
   EXPECT_NEAR( 800.203873598369, outputData.getMapValue( isostaticBathymetry, 0, 1 ), 1e-12 );
   EXPECT_NEAR( 800.203873598369, outputData.getMapValue( isostaticBathymetry, 1, 0 ), 1e-12 );
   EXPECT_NEAR( 800.203873598369, outputData.getMapValue( isostaticBathymetry, 0, 0 ), 1e-12 );
   // and undefined data
   PaleowaterdepthCalculator pwdCalculator3( firstI,
                                             firstJ,
                                             lastI,
                                             lastJ,
                                             3000,
                                             1000,
                                             outputData,
                                             validator,
                                             presentDayPressureMantle.getMockderivedSurfacePropertyPtr(),
                                             currentPressureMantle.getMockderivedSurfacePropertyPtr(),
                                             &gridMapPresentDayPressureTTSNDV,
                                             &gridMapCurrentPressureTTS );
   pwdCalculator3.compute();
   EXPECT_EQ( 800, outputData.getMapValue( isostaticBathymetry, 0, 0 ) );
   EXPECT_EQ( 800, outputData.getMapValue( isostaticBathymetry, 0, 1 ) );
   EXPECT_EQ( 800, outputData.getMapValue( isostaticBathymetry, 1, 0 ) );
   EXPECT_EQ( 800, outputData.getMapValue( isostaticBathymetry, 0, 0 ) );

   //Undefined values
   // for TTS
   outputData.setMapValues( WLSMap, Interface::DefaultUndefinedMapValue );
   PaleowaterdepthCalculator pwdCalculator4( firstI,
                                             firstJ,
                                             lastI,
                                             lastJ,
                                             3000,
                                             1000,
                                             outputData,
                                             validator );
   pwdCalculator4.compute();
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( isostaticBathymetry, 0, 0 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( isostaticBathymetry, 0, 1 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( isostaticBathymetry, 1, 0 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( isostaticBathymetry, 0, 0 ) );
   // for backstrip
   outputData.setMapValues( WLSMap, 200 );
   outputData.setMapValues( cumSedimentBackstrip, Interface::DefaultUndefinedMapValue );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( isostaticBathymetry, 0, 0 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( isostaticBathymetry, 0, 1 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( isostaticBathymetry, 1, 0 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( isostaticBathymetry, 0, 0 ) );
   //for valid node
   outputData.setMapValues( WLSMap, 250 );
   outputData.setMapValues( cumSedimentBackstrip, -300 );
   validator.setIsValid( false );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( isostaticBathymetry, 0, 0 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( isostaticBathymetry, 0, 1 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( isostaticBathymetry, 1, 0 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( isostaticBathymetry, 0, 0 ) );
}
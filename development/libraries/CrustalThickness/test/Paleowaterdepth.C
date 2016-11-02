//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include "../src/PaleowaterdepthCalculator.h"

// CrustalThickness library test utilities
#include "MockConfigFileParameterCtc.h"
#include "MockInterfaceInput.h"
#include "MockInterfaceOutput.h"
#include "MockValidator.h"

// Derived Properties library test utilities
#include "../../DerivedProperties/test/MockDerivedSurfaceProperty.h"

// utility library
#include "../../utilities/src/FormattingException.h"

// DataAccess libraries
#include "../../SerialDataAccess/src/Interface/SerialGrid.h"
#include "../../SerialDataAccess/src/Interface/SerialGridMap.h"

#include <gtest/gtest.h>

typedef formattingexception::GeneralException PaleowaterdepthException;

using namespace CrustalThicknessInterface;
using namespace CrustalThickness;

// Global validator
MockValidator validator = MockValidator();

// Create some data before tests
class PWDCalculatorTest : public ::testing::Test
{
public:
   PWDCalculatorTest() :
      m_firstI( 0 ), m_firstJ( 0 ), m_lastI( 1 ), m_lastJ( 1 ),
      m_minI( 0.0 ), m_minJ( 0.0 ), m_maxI( 4.0 ), m_maxJ( 4.0 ),
      m_numI( 2 ), m_numJ( 2 )
   {
      m_grid = new DataAccess::Interface::SerialGrid( this->m_minI, this->m_minJ, this->m_maxI, this->m_maxJ, this->m_numI, this->m_numJ );
      m_gridMap                         = new DataAccess::Interface::SerialGridMap( 0, 0, this->m_grid, 0, 1 );
      m_gridMapPresentDayTTS            = new DataAccess::Interface::SerialGridMap( 0, 0, this->m_grid, 1000 );
      m_gridMapPresentDayTTSNDV         = new DataAccess::Interface::SerialGridMap( 0, 0, this->m_grid, Interface::DefaultUndefinedMapValue );
      
      m_currentPressureBasement       = new DataModel::MockDerivedSurfaceProperty( this->m_firstI, this->m_firstJ, this->m_firstI, this->m_firstJ, this->m_lastI, this->m_lastJ, this->m_lastI, this->m_lastJ, "Mantle", "Pressure", 10, 2000  );
      m_presentDayPressureBasement    = new DataModel::MockDerivedSurfaceProperty( this->m_firstI, this->m_firstJ, this->m_firstI, this->m_firstJ, this->m_lastI, this->m_lastJ, this->m_lastI, this->m_lastJ, "Mantle", "Pressure", 0,  1000  );
      m_presentDayPressureBasementNDV = new DataModel::MockDerivedSurfaceProperty( this->m_firstI, this->m_firstJ, this->m_firstI, this->m_firstJ, this->m_lastI, this->m_lastJ, this->m_lastI, this->m_lastJ, "Mantle", "Pressure", 0, Interface::DefaultUndefinedMapValue );
      m_currentPressureMantle         = new DataModel::MockDerivedSurfaceProperty( this->m_firstI, this->m_firstJ, this->m_firstI, this->m_firstJ, this->m_lastI, this->m_lastJ, this->m_lastI, this->m_lastJ, "Mantle", "Pressure", 10, 10000 );
      m_presentDayPressureMantle      = new DataModel::MockDerivedSurfaceProperty( this->m_firstI, this->m_firstJ, this->m_firstI, this->m_firstJ, this->m_lastI, this->m_lastJ, this->m_lastI, this->m_lastJ, "Mantle", "Pressure", 0,  5000  );
   }

   ~PWDCalculatorTest()
   {
      delete m_grid;
      delete m_gridMap;
      delete m_gridMapPresentDayTTS;
      delete m_gridMapPresentDayTTSNDV;
      delete m_currentPressureBasement;
      delete m_presentDayPressureBasement;
      delete m_presentDayPressureBasementNDV;
      delete m_currentPressureMantle;
      delete m_presentDayPressureMantle;
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
   const DataAccess::Interface::SerialGridMap* m_gridMapPresentDayTTS;
   const DataAccess::Interface::SerialGridMap* m_gridMapPresentDayTTSNDV;

   //derived properties
   const DataModel::MockDerivedSurfaceProperty* m_currentPressureBasement;
   const DataModel::MockDerivedSurfaceProperty* m_presentDayPressureBasement;
   const DataModel::MockDerivedSurfaceProperty* m_presentDayPressureBasementNDV;
   const DataModel::MockDerivedSurfaceProperty* m_currentPressureMantle;
   const DataModel::MockDerivedSurfaceProperty* m_presentDayPressureMantle;

};

///0. Exception handling by the constructor
TEST_F( PWDCalculatorTest, exceptions ){

   MockInterfaceOutput outputData = MockInterfaceOutput( m_firstI, m_firstJ, m_lastI, m_lastJ );
   MockInterfaceInput inputData = MockInterfaceInput();
   MockConfigFileParameterCtc constants = MockConfigFileParameterCtc();
   constants.setWaterDensity( 3000 );
   constants.setBackstrippingMantleDensity( 3000 );
   inputData.setHCuMap( m_gridMap );
   inputData.setConstants( constants );
   inputData.setPressureBasementAtPresentDay( m_presentDayPressureBasement->getMockderivedSurfacePropertyPtr() );
   inputData.setPressureBasement( m_currentPressureBasement->getMockderivedSurfacePropertyPtr() );
   inputData.setPressureMantleAtPresentDay( m_presentDayPressureMantle->getMockderivedSurfacePropertyPtr() );
   inputData.setPressureMantle( m_currentPressureMantle->getMockderivedSurfacePropertyPtr() );

   // with wrong density values
   try{
      PaleowaterdepthCalculator pwdCalculator1( inputData,
         outputData,
         validator,
         m_gridMapPresentDayTTS );
      FAIL() << "Expected 'The water density is equal to the mantle density in the Paleowaterdepth calculator but they should be different' exception";
   }
   catch (const PaleowaterdepthException& ex) {
      EXPECT_EQ( "The water density is equal to the mantle density in the Paleowaterdepth calculator but they should be different", std::string( ex.what() ) );
   }
   catch (...) {
      FAIL() << "Expected 'The water density is equal to the mantle density in the Paleowaterdepth calculator but they should be different' exception";
   }
   // with present day TTS = nullptr
   constants.setWaterDensity( 1000 );
   constants.setBackstrippingMantleDensity( 3000 );
   inputData.setConstants( constants );
   try{
      PaleowaterdepthCalculator pwdCalculator2( inputData,
         outputData,
         validator,
         nullptr );
      FAIL() << "Expected 'The present day total tectonic subsidence is a null pointer and is required by the Paleowaterdepth calculator' exception";
   }
   catch (const PaleowaterdepthException& ex) {
      EXPECT_EQ( "The present day total tectonic subsidence is a null pointer and is required by the Paleowaterdepth calculator", std::string( ex.what() ) );
   }
   catch (...) {
      FAIL() << "Expected 'The present day total tectonic subsidence is a null pointer and is required by the Paleowaterdepth calculator' exception";
   }
}

///1. Test the paleowaterdepth calculation
TEST_F( PWDCalculatorTest, paleowaterdepth )
{

   MockInterfaceOutput outputData = MockInterfaceOutput( m_firstI, m_firstJ, m_lastI, m_lastJ );
   MockInterfaceInput inputData = MockInterfaceInput();
   MockConfigFileParameterCtc constants = MockConfigFileParameterCtc();
   constants.setWaterDensity( 1000 );
   constants.setBackstrippingMantleDensity( 3000 );
   inputData.setHCuMap( m_gridMap );
   inputData.setConstants( constants );
   inputData.setPressureBasementAtPresentDay( m_presentDayPressureBasement->getMockderivedSurfacePropertyPtr() );
   inputData.setPressureBasement( m_currentPressureBasement->getMockderivedSurfacePropertyPtr() );
   inputData.setPressureMantleAtPresentDay( m_presentDayPressureMantle->getMockderivedSurfacePropertyPtr() );
   inputData.setPressureMantle( m_currentPressureMantle->getMockderivedSurfacePropertyPtr() );

   //With pressure equlibrium
   PaleowaterdepthCalculator pwdCalculator1( inputData,
      outputData,
      validator,
      m_gridMapPresentDayTTS );
   EXPECT_NEAR( 800.203873598369, pwdCalculator1.calculatePWD( 1000, 200, 5000, 1000, 10000, 2000 ), 1e-12 );
   EXPECT_NEAR( 3000.183486238530, pwdCalculator1.calculatePWD( 1000, -2000, 6000, 700, 9000, 100 ), 1e-11 );
   EXPECT_NEAR( -0.089194699286, pwdCalculator1.calculatePWD( 0, 0, 4000, 350, 2000, 100 ), 1e-12 );

   //Without pressure equilibirium
   inputData.setPressureBasementAtPresentDay( nullptr );
   inputData.setPressureBasement( nullptr );
   PaleowaterdepthCalculator pwdCalculator2( inputData,
      outputData,
      validator,
      m_gridMapPresentDayTTS );
   EXPECT_EQ( 700, pwdCalculator2.calculatePWD( 500, -200 ) );
   EXPECT_EQ( 800, pwdCalculator2.calculatePWD( 1000, 200 ) );
   EXPECT_EQ( 0, pwdCalculator2.calculatePWD( 0, 0 ) );

}

///2. Test the general calculator computation
TEST_F( PWDCalculatorTest, compute )
{

   MockInterfaceOutput outputData = MockInterfaceOutput( m_firstI, m_firstJ, m_lastI, m_lastJ );
   MockInterfaceInput inputData = MockInterfaceInput();
   MockConfigFileParameterCtc constants = MockConfigFileParameterCtc();
   constants.setWaterDensity( 1000 );
   constants.setBackstrippingMantleDensity( 3000 );
   inputData.setHCuMap( m_gridMap );
   inputData.setConstants( constants );
   inputData.setPressureBasementAtPresentDay( nullptr );
   inputData.setPressureBasement( nullptr );
   inputData.setPressureMantleAtPresentDay( m_presentDayPressureMantle->getMockderivedSurfacePropertyPtr() );
   inputData.setPressureMantle( m_currentPressureMantle->getMockderivedSurfacePropertyPtr() );

   //Without pressure equlibrium
   outputData.setMapValues( cumSedimentBackstrip, 200 );
   PaleowaterdepthCalculator pwdCalculator1( inputData,
                                             outputData,
                                             validator,
                                             m_gridMapPresentDayTTS );
   pwdCalculator1.compute();
   EXPECT_EQ( 800, outputData.getMapValue( isostaticBathymetry, 0, 0 ) );
   EXPECT_EQ( 800, outputData.getMapValue( isostaticBathymetry, 0, 1 ) );
   EXPECT_EQ( 800, outputData.getMapValue( isostaticBathymetry, 1, 0 ) );
   EXPECT_EQ( 800, outputData.getMapValue( isostaticBathymetry, 1, 1 ) );

   //With pressure equilibirium
   // and defined data
   inputData.setPressureBasementAtPresentDay( m_presentDayPressureBasement->getMockderivedSurfacePropertyPtr() );
   inputData.setPressureBasement( m_currentPressureBasement->getMockderivedSurfacePropertyPtr() );
   PaleowaterdepthCalculator pwdCalculator2( inputData,
                                             outputData,
                                             validator,
                                             m_gridMapPresentDayTTS );
   pwdCalculator2.compute();
   EXPECT_NEAR( 800.203873598369, outputData.getMapValue( isostaticBathymetry, 0, 0 ), 1e-12 );
   EXPECT_NEAR( 800.203873598369, outputData.getMapValue( isostaticBathymetry, 0, 1 ), 1e-12 );
   EXPECT_NEAR( 800.203873598369, outputData.getMapValue( isostaticBathymetry, 1, 0 ), 1e-12 );
   EXPECT_NEAR( 800.203873598369, outputData.getMapValue( isostaticBathymetry, 1, 1 ), 1e-12 );
   // and undefined data
   inputData.setPressureBasementAtPresentDay( m_presentDayPressureBasementNDV->getMockderivedSurfacePropertyPtr() );
   PaleowaterdepthCalculator pwdCalculator3( inputData,
                                             outputData,
                                             validator,
                                             m_gridMapPresentDayTTS );
   pwdCalculator3.compute();
   EXPECT_EQ( 800, outputData.getMapValue( isostaticBathymetry, 0, 0 ) );
   EXPECT_EQ( 800, outputData.getMapValue( isostaticBathymetry, 0, 1 ) );
   EXPECT_EQ( 800, outputData.getMapValue( isostaticBathymetry, 1, 0 ) );
   EXPECT_EQ( 800, outputData.getMapValue( isostaticBathymetry, 1, 1 ) );

   //Undefined values
   // for TTS
   inputData.setPressureBasementAtPresentDay( m_presentDayPressureBasement->getMockderivedSurfacePropertyPtr() );
   PaleowaterdepthCalculator pwdCalculator4( inputData,
                                             outputData,
                                             validator,
                                             m_gridMapPresentDayTTSNDV );
   pwdCalculator4.compute();
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( isostaticBathymetry, 0, 0 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( isostaticBathymetry, 0, 1 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( isostaticBathymetry, 1, 0 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( isostaticBathymetry, 1, 1 ) );
   // for backstrip
   PaleowaterdepthCalculator pwdCalculator5( inputData,
                                             outputData,
                                             validator,
                                             m_gridMapPresentDayTTS );
   outputData.setMapValues( cumSedimentBackstrip, Interface::DefaultUndefinedMapValue );
   pwdCalculator5.compute();
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( isostaticBathymetry, 0, 0 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( isostaticBathymetry, 0, 1 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( isostaticBathymetry, 1, 0 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( isostaticBathymetry, 1, 1 ) );
   //for valid node
   outputData.setMapValues( cumSedimentBackstrip, -300 );
   validator.setIsValid( false );
   pwdCalculator5.compute();
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( isostaticBathymetry, 0, 0 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( isostaticBathymetry, 0, 1 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( isostaticBathymetry, 1, 0 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData.getMapValue( isostaticBathymetry, 1, 1 ) );
}
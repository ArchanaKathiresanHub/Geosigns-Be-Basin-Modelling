// 
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include "../src/DensityCalculator.h"

// CrustalThickness library test utilities
#include "MockConfigFileParameterCtc.h"
#include "MockInterfaceInput.h"
#include "MockInterfaceOutput.h"

// DataModel library test utilities
#include "../../DataModel/test/MockValidator.h"

// Derived Properties library test utilities
#include "../../DerivedProperties/test/MockDerivedSurfaceProperty.h"

// DataAccess libraries
#include "../../SerialDataAccess/src/Interface/SerialGrid.h"
#include "../../SerialDataAccess/src/Interface/SerialGridMap.h"

// utility library
#include "../../utilities/src/FormattingException.h"

#include <gtest/gtest.h>

typedef formattingexception::GeneralException DensityException;

using namespace CrustalThicknessInterface;
using namespace CrustalThickness;

// Global validator
MockValidator validator;

// Create some data before tests
class DensityCalculatorTest : public ::testing::Test
{
public:
   DensityCalculatorTest() :
      m_firstI( 0 ), m_firstJ( 0 ), m_lastI( 1 ), m_lastJ(1),
      m_minI(0.0), m_minJ(0.0), m_maxI(4.0), m_maxJ(4.0),
      m_numI(2), m_numJ(2)
   { 
      m_grid    = new DataAccess::Interface::SerialGrid( this->m_minI, this->m_minJ, this->m_maxI, this->m_maxJ, this->m_numI, this->m_numJ );
      m_gridMap = new DataAccess::Interface::SerialGridMap( 0, 0, this->m_grid, 0, 1 );
      m_pressureBasement    = new DataModel::MockDerivedSurfaceProperty( this->m_firstI, this->m_firstJ, this->m_firstI, this->m_firstJ, this->m_lastI, this->m_lastJ, this->m_lastI, this->m_lastJ, "pressureBasement", "Pressure", 10, 1000 );
      m_pressureWaterBottom = new DataModel::MockDerivedSurfaceProperty( this->m_firstI, this->m_firstJ, this->m_firstI, this->m_firstJ, this->m_lastI, this->m_lastJ, this->m_lastI, this->m_lastJ, "pressureWaterBottom", "Pressure", 10, 500 );
      m_depthBasement       = new DataModel::MockDerivedSurfaceProperty( this->m_firstI, this->m_firstJ, this->m_firstI, this->m_firstJ, this->m_lastI, this->m_lastJ, this->m_lastI, this->m_lastJ, "depthBasement", "Depth", 10, 100 );
      m_depthWaterBottom    = new DataModel::MockDerivedSurfaceProperty( this->m_firstI, this->m_firstJ, this->m_firstI, this->m_firstJ, this->m_lastI, this->m_lastJ, this->m_lastI, this->m_lastJ, "depthWaterBottom", "Depth", 10, 20 );
   }

   ~DensityCalculatorTest()
   { 
      delete m_gridMap;
      delete m_grid;
      delete m_pressureBasement;
      delete m_pressureWaterBottom;
      delete m_depthBasement;
      delete m_depthWaterBottom;
   }

   // global grid size variables (no gosth nodes)
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

   // derived properties
   const DataModel::MockDerivedSurfaceProperty* m_pressureBasement;
   const DataModel::MockDerivedSurfaceProperty* m_pressureWaterBottom;
   const DataModel::MockDerivedSurfaceProperty* m_depthBasement;
   const DataModel::MockDerivedSurfaceProperty* m_depthWaterBottom;

};


///0. Test that exceptions are handled correctly by the constructor
TEST_F( DensityCalculatorTest, exceptions )
{
   MockInterfaceOutput outputData( m_firstI, m_firstJ, m_lastI, m_lastJ );
   MockInterfaceInput inputData;
   MockConfigFileParameterCtc constants;
   constants.setWaterDensity( 2000 );
   constants.setBackstrippingMantleDensity( 2000 );
   inputData.setHCuMap( m_gridMap );
   inputData.setConstants( constants );
   inputData.setPressureBasement   ( m_pressureBasement->getMockderivedSurfacePropertyPtr()    );
   inputData.setPressureWaterBottom( m_pressureWaterBottom->getMockderivedSurfacePropertyPtr() );
   inputData.setDepthBasement      ( m_depthBasement->getMockderivedSurfacePropertyPtr()       );
   inputData.setDepthWaterBottom   ( m_depthWaterBottom->getMockderivedSurfacePropertyPtr()    );

   // 1. Test that the constructor throws an exception when the mantle and water densities are equal
   //test if the exception is thrown
   EXPECT_THROW( DensityCalculator densityCalculator1( inputData, outputData, validator ), std::invalid_argument );
   //test if the good exception is thrown
   try{
      DensityCalculator densityCalculator1( inputData, outputData, validator );
      FAIL() << "Expected 'Mantle density is equal to the water density (both provided by the interface input), this will lead to divisions by 0' exception";
   }
   catch (const std::invalid_argument& ex) {
      EXPECT_EQ( "Mantle density is equal to the water density (both provided by the interface input), this will lead to divisions by 0",
         std::string( ex.what() ) );
   }
   catch (...) {
      FAIL() << "Expected 'Mantle density is equal to the water density (both provided by the interface input), this will lead to divisions by 0' exception";
   }

   // 2. Test that the constructor throws an exception when the basement pressure is a null pointer
   constants.setBackstrippingMantleDensity( 3000 );
   inputData.setPressureBasement( nullptr );
   //test if the exception is thrown
   EXPECT_THROW( DensityCalculator densityCalculator2( inputData, outputData, validator ), std::invalid_argument );
   //test if the good exception is thrown
   try{
      DensityCalculator densityCalculator2( inputData, outputData, validator );
      FAIL() << "Expected 'Basement pressure provided by the interface input is a null pointer' exception";
   }
   catch (const std::invalid_argument& ex) {
      EXPECT_EQ( "Basement pressure provided by the interface input is a null pointer",
         std::string( ex.what() ) );
   }
   catch (...) {
      FAIL() << "Expected 'Basement pressure provided by the interface input is a null pointer' exception";
   }

   // 3. Test that the constructor throws an exception when the watter bottom pressure is a null pointer
   inputData.setPressureBasement( m_pressureBasement->getMockderivedSurfacePropertyPtr() );
   inputData.setPressureWaterBottom( nullptr );
   //test if the exception is thrown
   EXPECT_THROW( DensityCalculator densityCalculator3( inputData, outputData, validator ), std::invalid_argument );
   //test if the good exception is thrown
   try{
      DensityCalculator densityCalculator3( inputData, outputData, validator );
      FAIL() << "Expected 'Water bottom pressure provided by the interface input is a null pointer' exception";
   }
   catch (const std::invalid_argument& ex) {
      EXPECT_EQ( "Water bottom pressure provided by the interface input is a null pointer",
         std::string( ex.what() ) );
   }
   catch (...) {
      FAIL() << "Expected 'Water bottom pressure provided by the interface input is a null pointer' exception";
   }

   // 4. Test that the constructor throws an exception when the basement depth is a null pointer
   inputData.setPressureWaterBottom( m_pressureWaterBottom->getMockderivedSurfacePropertyPtr() );
   inputData.setDepthBasement( nullptr );
   //test if the exception is thrown
   EXPECT_THROW( DensityCalculator densityCalculator4( inputData, outputData, validator ), std::invalid_argument );
   //test if the good exception is thrown
   try{
      DensityCalculator densityCalculator4( inputData, outputData, validator );
      FAIL() << "Expected 'Basement depth provided by the interface input is a null pointer";
   }
   catch (const std::invalid_argument& ex) {
      EXPECT_EQ( "Basement depth provided by the interface input is a null pointer",
         std::string( ex.what() ) );
   }
   catch (...) {
      FAIL() << "Expected 'Basement depth provided by the interface input is a null pointer' exception";
   }

   // 5. Test that the constructor throws an exception when the watter bottom depth is anull pointer
   inputData.setDepthBasement( m_depthBasement->getMockderivedSurfacePropertyPtr() );
   inputData.setDepthWaterBottom( nullptr );
   //test if the exception is thrown
   EXPECT_THROW( DensityCalculator densityCalculator5( inputData, outputData, validator ), std::invalid_argument );
   //test if the good exception is thrown
   try{
      DensityCalculator densityCalculator5( inputData, outputData, validator );
      FAIL() << "Expected 'Water bottom depth provided by the interface input is a null pointer' exception";
   }
   catch (const std::invalid_argument& ex) {
      EXPECT_EQ( "Water bottom depth provided by the interface input is a null pointer",
         std::string( ex.what() ) );
   }
   catch (...) {
      FAIL() << "Expected 'Water bottom depth provided by the interface input is a null pointer' exception";
   }

}

///1. Test the air correction and density term
TEST_F( DensityCalculatorTest, density_terms )
{
   MockInterfaceOutput outputData( m_firstI, m_firstJ, m_lastI, m_lastJ );
   MockInterfaceInput inputData;
   MockConfigFileParameterCtc constants;
   constants.setWaterDensity( 1030 );
   constants.setBackstrippingMantleDensity( 3300 );
   inputData.setHCuMap( m_gridMap );
   inputData.setConstants( constants );
   inputData.setPressureBasement   ( m_pressureBasement->getMockderivedSurfacePropertyPtr()    );
   inputData.setPressureWaterBottom( m_pressureWaterBottom->getMockderivedSurfacePropertyPtr() );
   inputData.setDepthBasement      ( m_depthBasement->getMockderivedSurfacePropertyPtr()       );
   inputData.setDepthWaterBottom   ( m_depthWaterBottom->getMockderivedSurfacePropertyPtr()    );

   // 1. Test for real density values
   //dm=3300,dw=1030
   DensityCalculator densityCalculator1( inputData, outputData, validator );
   EXPECT_NEAR( 1.453744493392070,    densityCalculator1.getAirCorrection(), 1e-14 );
   EXPECT_NEAR( 4.405286343612335e-4, densityCalculator1.getDensityTerm(),   1e-14 );

   //dm=3000,dw=1000
   constants.setWaterDensity( 1000 );
   constants.setBackstrippingMantleDensity( 3000 );
   inputData.setConstants( constants );
   DensityCalculator densityCalculator2( inputData, outputData, validator );
   EXPECT_NEAR( 1.5, densityCalculator2.getAirCorrection(), 1e-14 );
   EXPECT_NEAR( 5e-4, densityCalculator2.getDensityTerm(),  1e-10 );
}

///2. Test the backstripping
TEST_F( DensityCalculatorTest, backstriping )
{
   MockInterfaceOutput outputData = MockInterfaceOutput( m_firstI, m_firstJ, m_lastI, m_lastJ );
   MockInterfaceInput inputData;
   MockConfigFileParameterCtc constants;
   constants.setWaterDensity( 1030 );
   constants.setBackstrippingMantleDensity( 3300 );
   inputData.setHCuMap( m_gridMap );
   inputData.setConstants( constants );
   inputData.setPressureBasement   ( m_pressureBasement->getMockderivedSurfacePropertyPtr()    );
   inputData.setPressureWaterBottom( m_pressureWaterBottom->getMockderivedSurfacePropertyPtr() );
   inputData.setDepthBasement      ( m_depthBasement->getMockderivedSurfacePropertyPtr()       );
   inputData.setDepthWaterBottom   ( m_depthWaterBottom->getMockderivedSurfacePropertyPtr()    );

   // 1. Test for real density values
   //dm=3300,dw=1030
   DensityCalculator densityCalculator1( inputData, outputData, validator );
   densityCalculator1.calculate( 100, 20, 1000, 500 );
   EXPECT_EQ( 80, densityCalculator1.getSedimentThickness() );
   EXPECT_NEAR( 637104.994903160,    densityCalculator1.getSedimentDensity(), 1e-10 );
   EXPECT_NEAR( -22336.739908481400, densityCalculator1.getBackstrip(),       1e-10 );
   EXPECT_NEAR( 22416.739908481400,  densityCalculator1.getCompensation(),    1e-10 );

   //dm=3000,dw=1000
   constants.setWaterDensity( 1000 );
   constants.setBackstrippingMantleDensity( 3000 );
   inputData.setConstants( constants );
   DensityCalculator densityCalculator2( inputData, outputData, validator );
   densityCalculator2.calculate( 500, 100, 5000, 1000 );
   EXPECT_EQ( 400, densityCalculator2.getSedimentThickness() );
   EXPECT_NEAR( 1019367.9918450600,  densityCalculator2.getSedimentDensity(), 1e-08 );
   EXPECT_NEAR( -203273.59836901100, densityCalculator2.getBackstrip(),       1e-09 );
   EXPECT_NEAR( 203673.59836901100,  densityCalculator2.getCompensation(),    1e-09 );

   // 2. Test that the outputs are 0 and NDV when the sediment thickness is null
   densityCalculator2.calculate( 100, 100, 1000, 500 );
   EXPECT_EQ( 0, densityCalculator2.getSedimentThickness() );
   EXPECT_EQ( DataAccess::Interface::DefaultUndefinedMapValue, densityCalculator2.getSedimentDensity()   );
   EXPECT_EQ( 0, densityCalculator2.getBackstrip()    );
   EXPECT_EQ( 0, densityCalculator2.getCompensation() );

   //3. Test that the outputs are NDV when the sediment thickness is negative
   densityCalculator2.calculate( 20, 100, 1000, 500 );
   EXPECT_EQ( -80, densityCalculator2.getSedimentThickness() );
   EXPECT_EQ( DataAccess::Interface::DefaultUndefinedMapValue, densityCalculator2.getSedimentDensity() );
   EXPECT_EQ( DataAccess::Interface::DefaultUndefinedMapValue, densityCalculator2.getBackstrip()       );
   EXPECT_EQ( DataAccess::Interface::DefaultUndefinedMapValue, densityCalculator2.getCompensation()    );
}


///3. Test the general calculator computation
TEST_F( DensityCalculatorTest, compute )
{
   MockInterfaceOutput outputData( m_firstI, m_firstJ, m_lastI, m_lastJ );
   MockInterfaceInput inputData;
   MockConfigFileParameterCtc constants;
   constants.setWaterDensity( 1030 );
   constants.setBackstrippingMantleDensity( 3300 );
   inputData.setHCuMap( m_gridMap );
   inputData.setConstants( constants );
   inputData.setPressureBasement   ( m_pressureBasement->getMockderivedSurfacePropertyPtr()    );
   inputData.setPressureWaterBottom( m_pressureWaterBottom->getMockderivedSurfacePropertyPtr() );
   inputData.setDepthBasement      ( m_depthBasement->getMockderivedSurfacePropertyPtr()       );
   inputData.setDepthWaterBottom   ( m_depthWaterBottom->getMockderivedSurfacePropertyPtr()    );

   // 1. Test for real density values
   //dm=3300,dw=1030
   MockInterfaceOutput outputData1( m_firstI, m_firstJ, m_lastI, m_lastJ );
   DensityCalculator densityCalculator1( inputData, outputData1, validator );
   densityCalculator1.compute();
   EXPECT_EQ( 80, outputData1.getMapValue( cumSedimentThickness, 0, 0 ) );
   EXPECT_EQ( 80, outputData1.getMapValue( cumSedimentThickness, 0, 1 ) );
   EXPECT_EQ( 80, outputData1.getMapValue( cumSedimentThickness, 1, 0 ) );
   EXPECT_EQ( 80, outputData1.getMapValue( cumSedimentThickness, 1, 1 ) );
   EXPECT_NEAR( 637104.994903160,    outputData1.getMapValue( sedimentDensityMap,      0, 0 ), 1e-10 );
   EXPECT_NEAR( 637104.994903160,    outputData1.getMapValue( sedimentDensityMap,      0, 1 ), 1e-10 );
   EXPECT_NEAR( 637104.994903160,    outputData1.getMapValue( sedimentDensityMap,      1, 0 ), 1e-10 );
   EXPECT_NEAR( 637104.994903160,    outputData1.getMapValue( sedimentDensityMap,      1, 1 ), 1e-10 );
   EXPECT_NEAR( -22336.739908481400, outputData1.getMapValue( cumSedimentBackstrip,    0, 0 ), 1e-10 );
   EXPECT_NEAR( -22336.739908481400, outputData1.getMapValue( cumSedimentBackstrip,    0, 1 ), 1e-10 );
   EXPECT_NEAR( -22336.739908481400, outputData1.getMapValue( cumSedimentBackstrip,    1, 0 ), 1e-10 );
   EXPECT_NEAR( -22336.739908481400, outputData1.getMapValue( cumSedimentBackstrip,    1, 1 ), 1e-10 );
   EXPECT_NEAR( 22416.739908481400,  outputData1.getMapValue( cumBasementCompensation, 0, 0 ), 1e-10 );
   EXPECT_NEAR( 22416.739908481400,  outputData1.getMapValue( cumBasementCompensation, 0, 1 ), 1e-10 );
   EXPECT_NEAR( 22416.739908481400,  outputData1.getMapValue( cumBasementCompensation, 1, 0 ), 1e-10 );
   EXPECT_NEAR( 22416.739908481400,  outputData1.getMapValue( cumBasementCompensation, 1, 1 ), 1e-10 );

   //dm=3000,dw=1000
   constants.setWaterDensity( 1000 );
   constants.setBackstrippingMantleDensity( 3000 );
   const DataModel::MockDerivedSurfaceProperty pressureBasement2   ( m_firstI, m_firstJ, m_firstI, m_firstJ, m_lastI, m_lastJ, m_lastI, m_lastJ, "m_pressureBasement",    "Pressure", 10, 5000 );
   const DataModel::MockDerivedSurfaceProperty pressureWaterBottom2( m_firstI, m_firstJ, m_firstI, m_firstJ, m_lastI, m_lastJ, m_lastI, m_lastJ, "m_pressureWaterBottom", "Pressure", 10, 1000 );
   const DataModel::MockDerivedSurfaceProperty depthBasement2      ( m_firstI, m_firstJ, m_firstI, m_firstJ, m_lastI, m_lastJ, m_lastI, m_lastJ, "m_depthBasement",       "Depth",    10, 500  );
   const DataModel::MockDerivedSurfaceProperty depthWaterBottom2   ( m_firstI, m_firstJ, m_firstI, m_firstJ, m_lastI, m_lastJ, m_lastI, m_lastJ, "m_depthWaterBottom",    "Depth",    10, 100  );
   inputData.setConstants( constants );
   inputData.setPressureBasement   ( pressureBasement2.getMockderivedSurfacePropertyPtr()    );
   inputData.setPressureWaterBottom( pressureWaterBottom2.getMockderivedSurfacePropertyPtr() );
   inputData.setDepthBasement      ( depthBasement2.getMockderivedSurfacePropertyPtr()       );
   inputData.setDepthWaterBottom   ( depthWaterBottom2.getMockderivedSurfacePropertyPtr()    );
   MockInterfaceOutput outputData2( m_firstI, m_firstJ, m_lastI, m_lastJ );
   DensityCalculator densityCalculator2( inputData, outputData2, validator );
   densityCalculator2.compute();
   EXPECT_EQ( 400, outputData2.getMapValue( cumSedimentThickness, 0, 0 ) );
   EXPECT_EQ( 400, outputData2.getMapValue( cumSedimentThickness, 0, 1 ) );
   EXPECT_EQ( 400, outputData2.getMapValue( cumSedimentThickness, 1, 0 ) );
   EXPECT_EQ( 400, outputData2.getMapValue( cumSedimentThickness, 1, 1 ) );
   EXPECT_NEAR( 1019367.9918450600,  outputData2.getMapValue( sedimentDensityMap,      0, 0 ), 1e-8 );
   EXPECT_NEAR( 1019367.9918450600,  outputData2.getMapValue( sedimentDensityMap,      0, 1 ), 1e-8 );
   EXPECT_NEAR( 1019367.9918450600,  outputData2.getMapValue( sedimentDensityMap,      1, 0 ), 1e-8 );
   EXPECT_NEAR( 1019367.9918450600,  outputData2.getMapValue( sedimentDensityMap,      1, 1 ), 1e-8 );
   EXPECT_NEAR( -203273.59836901100, outputData2.getMapValue( cumSedimentBackstrip,    0, 0 ), 1e-9 );
   EXPECT_NEAR( -203273.59836901100, outputData2.getMapValue( cumSedimentBackstrip,    0, 1 ), 1e-9 );
   EXPECT_NEAR( -203273.59836901100, outputData2.getMapValue( cumSedimentBackstrip,    1, 0 ), 1e-9 );
   EXPECT_NEAR( -203273.59836901100, outputData2.getMapValue( cumSedimentBackstrip,    1, 1 ), 1e-9 );
   EXPECT_NEAR( 203673.59836901100,  outputData2.getMapValue( cumBasementCompensation, 0, 0 ), 1e-9 );
   EXPECT_NEAR( 203673.59836901100,  outputData2.getMapValue( cumBasementCompensation, 0, 1 ), 1e-9 );
   EXPECT_NEAR( 203673.59836901100,  outputData2.getMapValue( cumBasementCompensation, 1, 0 ), 1e-9 );
   EXPECT_NEAR( 203673.59836901100,  outputData2.getMapValue( cumBasementCompensation, 1, 1 ), 1e-9 );

   // 2. Test that the outputs are NDV when the nodes are not valid
   MockInterfaceOutput outputData3( m_firstI, m_firstJ, m_lastI, m_lastJ );
   validator.setIsValid( false );
   DensityCalculator densityCalculator3( inputData, outputData3, validator );
   densityCalculator3.compute();
   EXPECT_EQ( DataAccess::Interface::DefaultUndefinedMapValue, outputData3.getMapValue( cumSedimentBackstrip,    0, 0 ) );
   EXPECT_EQ( DataAccess::Interface::DefaultUndefinedMapValue, outputData3.getMapValue( cumSedimentBackstrip,    0, 1 ) );
   EXPECT_EQ( DataAccess::Interface::DefaultUndefinedMapValue, outputData3.getMapValue( cumSedimentBackstrip,    1, 0 ) );
   EXPECT_EQ( DataAccess::Interface::DefaultUndefinedMapValue, outputData3.getMapValue( cumSedimentBackstrip,    1, 1 ) );
   EXPECT_EQ( DataAccess::Interface::DefaultUndefinedMapValue, outputData3.getMapValue( sedimentDensityMap,      0, 0 ) );
   EXPECT_EQ( DataAccess::Interface::DefaultUndefinedMapValue, outputData3.getMapValue( sedimentDensityMap,      0, 1 ) );
   EXPECT_EQ( DataAccess::Interface::DefaultUndefinedMapValue, outputData3.getMapValue( sedimentDensityMap,      1, 0 ) );
   EXPECT_EQ( DataAccess::Interface::DefaultUndefinedMapValue, outputData3.getMapValue( sedimentDensityMap,      1, 1 ) );
   EXPECT_EQ( DataAccess::Interface::DefaultUndefinedMapValue, outputData3.getMapValue( cumSedimentThickness,    0, 0 ) );
   EXPECT_EQ( DataAccess::Interface::DefaultUndefinedMapValue, outputData3.getMapValue( cumSedimentThickness,    0, 1 ) );
   EXPECT_EQ( DataAccess::Interface::DefaultUndefinedMapValue, outputData3.getMapValue( cumSedimentThickness,    1, 0 ) );
   EXPECT_EQ( DataAccess::Interface::DefaultUndefinedMapValue, outputData3.getMapValue( cumSedimentThickness,    1, 1 ) );
   EXPECT_EQ( DataAccess::Interface::DefaultUndefinedMapValue, outputData3.getMapValue( cumBasementCompensation, 0, 0 ) );
   EXPECT_EQ( DataAccess::Interface::DefaultUndefinedMapValue, outputData3.getMapValue( cumBasementCompensation, 0, 1 ) );
   EXPECT_EQ( DataAccess::Interface::DefaultUndefinedMapValue, outputData3.getMapValue( cumBasementCompensation, 1, 0 ) );
   EXPECT_EQ( DataAccess::Interface::DefaultUndefinedMapValue, outputData3.getMapValue( cumBasementCompensation, 1, 1 ) );


}
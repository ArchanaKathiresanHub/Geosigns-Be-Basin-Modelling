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
#include "../src/DensityCalculator.h"

// Derived Properties library
#include "../../DerivedProperties/test/MockDerivedSurfaceProperty.h"

// utility library
#include "../../utilities/src/FormattingException.h"

#include <gtest/gtest.h>

typedef formattingexception::GeneralException DensityException;

using namespace CrustalThicknessInterface;

// Global validator
MockValidator validator = MockValidator();

// Global grid size variables (no gosth nodes)
unsigned int firstI = 2;
unsigned int firstJ = 2;
unsigned int lastI  = 3;
unsigned int lastJ  = 3;

// Define derived properties
const DataModel::MockDerivedSurfaceProperty pressureBasement   ( firstI, firstJ, firstI, firstJ, lastI, lastJ, lastI, lastJ, "pressureBasement",    "Pressure", 10, 1000 );
const DataModel::MockDerivedSurfaceProperty pressureWaterBottom( firstI, firstJ, firstI, firstJ, lastI, lastJ, lastI, lastJ, "pressureWaterBottom", "Pressure", 10, 500  );
const DataModel::MockDerivedSurfaceProperty depthBasement      ( firstI, firstJ, firstI, firstJ, lastI, lastJ, lastI, lastJ, "depthBasement",       "Depth",    10, 100  );
const DataModel::MockDerivedSurfaceProperty depthWaterBottom   ( firstI, firstJ, firstI, firstJ, lastI, lastJ, lastI, lastJ, "depthWaterBottom",    "Depth",    10, 20   );

///1. Test the air correction and density term
TEST( DensityCalculatorTest, density_terms )
{
   MockInterfaceOutput outputData = MockInterfaceOutput( firstI, firstJ, lastI, lastJ );

   // 1. Test for real density values
   //dm=3300,dw=1030
   DensityCalculator densityCalculator1( firstI,
                                         firstJ,
                                         lastI,
                                         lastJ,
                                         3300,
                                         1030,
                                         pressureBasement.getMockderivedSurfacePropertyPtr(),
                                         pressureWaterBottom.getMockderivedSurfacePropertyPtr(),
                                         depthBasement.getMockderivedSurfacePropertyPtr(),
                                         depthWaterBottom.getMockderivedSurfacePropertyPtr(),
                                         outputData,
                                         validator );
   EXPECT_NEAR( 1.453744493392070,    densityCalculator1.getAirCorrection(), 1e-14 );
   EXPECT_NEAR( 4.405286343612335e-4, densityCalculator1.getDensityTerm(),   1e-14 );

   //dm=3000,dw=1000
   DensityCalculator densityCalculator2( firstI,
                                         firstJ,
                                         lastI,
                                         lastJ,
                                         3000,
                                         1000,
                                         pressureBasement.getMockderivedSurfacePropertyPtr(),
                                         pressureWaterBottom.getMockderivedSurfacePropertyPtr(),
                                         depthBasement.getMockderivedSurfacePropertyPtr(),
                                         depthWaterBottom.getMockderivedSurfacePropertyPtr(),
                                         outputData,
                                         validator );
   EXPECT_NEAR( 1.5, densityCalculator2.getAirCorrection(), 1e-14 );
   EXPECT_NEAR( 5e-4, densityCalculator2.getDensityTerm(),  1e-10 );

   // 2. Test that the function return false when the mantle and water densities are equal
   //test if the exception is thrown
   EXPECT_THROW( DensityCalculator densityCalculator3( firstI,
                                                       firstJ,
                                                       lastI,
                                                       lastJ,
                                                       2000,
                                                       2000,
                                                       pressureBasement.getMockderivedSurfacePropertyPtr(),
                                                       pressureWaterBottom.getMockderivedSurfacePropertyPtr(),
                                                       depthBasement.getMockderivedSurfacePropertyPtr(),
                                                       depthWaterBottom.getMockderivedSurfacePropertyPtr(),
                                                       outputData,
                                                       validator ),
                 DensityException );

   //test if the good exception is thrown
   try{
      DensityCalculator densityCalculator4( firstI,
                                            firstJ,
                                            lastI,
                                            lastJ,
                                            2000,
                                            2000,
                                            pressureBasement.getMockderivedSurfacePropertyPtr(),
                                            pressureWaterBottom.getMockderivedSurfacePropertyPtr(),
                                            depthBasement.getMockderivedSurfacePropertyPtr(),
                                            depthWaterBottom.getMockderivedSurfacePropertyPtr(),
                                            outputData,
                                            validator );
      FAIL() << "Expected 'BackstrippingMantleDensity = WaterDensity. Check the constants in the configuration file.' exception";
   }
   catch (const DensityException& ex) {
      EXPECT_EQ( "BackstrippingMantleDensity = WaterDensity. Check the constants in the configuration file.", std::string( ex.what() ) );
   }
   catch (...) {
      FAIL() << "Expected 'BackstrippingMantleDensity = WaterDensity. Check the constants in the configuration file.' exception";
   }
}

///2. Test the backstripping
TEST( DensityCalculatorTest, backstriping )
{
   MockInterfaceOutput outputData = MockInterfaceOutput( firstI, firstJ, lastI, lastJ );

   // 1. Test for real density values
   //dm=3300,dw=1030
   DensityCalculator densityCalculator1( firstI,
                                         firstJ,
                                         lastI,
                                         lastJ,
                                         3300,
                                         1030,
                                         pressureBasement.getMockderivedSurfacePropertyPtr(),
                                         pressureWaterBottom.getMockderivedSurfacePropertyPtr(),
                                         depthBasement.getMockderivedSurfacePropertyPtr(),
                                         depthWaterBottom.getMockderivedSurfacePropertyPtr(),
                                         outputData,
                                         validator );
   densityCalculator1.calculate( 100, 20, 1000, 500 );
   EXPECT_EQ( 80, densityCalculator1.getSedimentThickness() );
   EXPECT_NEAR( 637104.994903160,    densityCalculator1.getSedimentDensity(), 1e-10 );
   EXPECT_NEAR( -22336.739908481400, densityCalculator1.getBackstrip(),       1e-10 );
   EXPECT_NEAR( 22416.739908481400,  densityCalculator1.getCompensation(),    1e-10 );

   //dm=3000,dw=1000
   DensityCalculator densityCalculator2( firstI,
                                         firstJ,
                                         lastI,
                                         lastJ,
                                         3000,
                                         1000,
                                         pressureBasement.getMockderivedSurfacePropertyPtr(),
                                         pressureWaterBottom.getMockderivedSurfacePropertyPtr(),
                                         depthBasement.getMockderivedSurfacePropertyPtr(),
                                         depthWaterBottom.getMockderivedSurfacePropertyPtr(),
                                         outputData,
                                         validator );
   densityCalculator2.calculate( 500, 100, 5000, 1000 );
   EXPECT_EQ( 400, densityCalculator2.getSedimentThickness() );
   EXPECT_NEAR( 1019367.9918450600,  densityCalculator2.getSedimentDensity(), 1e-08 );
   EXPECT_NEAR( -203273.59836901100, densityCalculator2.getBackstrip(),       1e-09 );
   EXPECT_NEAR( 203673.59836901100,  densityCalculator2.getCompensation(),    1e-09 );

   // 2. Test that the outputs are NDV when the sediment thickness is null
   densityCalculator2.calculate( 100, 100, 1000, 500 );
   EXPECT_EQ( 0, densityCalculator2.getSedimentThickness() );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, densityCalculator2.getSedimentDensity() );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, densityCalculator2.getBackstrip()       );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, densityCalculator2.getCompensation()    );

   //3. Test that the outputs are NDV when the sediment thickness is negative
   densityCalculator2.calculate( 20, 100, 1000, 500 );
   EXPECT_EQ( -80, densityCalculator2.getSedimentThickness() );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, densityCalculator2.getSedimentDensity() );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, densityCalculator2.getBackstrip()       );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, densityCalculator2.getCompensation()    );
}

///3. Test the general calculator computation
TEST( DensityCalculatorTest, compute )
{
   // 1. Test for real density values
   //dm=3300,dw=1030
   MockInterfaceOutput outputData1 = MockInterfaceOutput( firstI, firstJ, lastI, lastJ );
   DensityCalculator densityCalculator1( firstI,
                                         firstJ,
                                         lastI,
                                         lastJ,
                                         3300,
                                         1030,
                                         pressureBasement.getMockderivedSurfacePropertyPtr(),
                                         pressureWaterBottom.getMockderivedSurfacePropertyPtr(),
                                         depthBasement.getMockderivedSurfacePropertyPtr(),
                                         depthWaterBottom.getMockderivedSurfacePropertyPtr(),
                                         outputData1,
                                         validator );
   densityCalculator1.compute();
   EXPECT_EQ( 80, outputData1.getMapValue( cumSedimentThickness, 2, 2 ) );
   EXPECT_EQ( 80, outputData1.getMapValue( cumSedimentThickness, 2, 3 ) );
   EXPECT_EQ( 80, outputData1.getMapValue( cumSedimentThickness, 3, 2 ) );
   EXPECT_EQ( 80, outputData1.getMapValue( cumSedimentThickness, 3, 3 ) );
   EXPECT_NEAR( 637104.994903160,    outputData1.getMapValue( sedimentDensityMap,      2, 2 ), 1e-10 );
   EXPECT_NEAR( 637104.994903160,    outputData1.getMapValue( sedimentDensityMap,      2, 3 ), 1e-10 );
   EXPECT_NEAR( 637104.994903160,    outputData1.getMapValue( sedimentDensityMap,      3, 2 ), 1e-10 );
   EXPECT_NEAR( 637104.994903160,    outputData1.getMapValue( sedimentDensityMap,      3, 3 ), 1e-10 );
   EXPECT_NEAR( -22336.739908481400, outputData1.getMapValue( cumSedimentBackstrip,    2, 2 ), 1e-10 );
   EXPECT_NEAR( -22336.739908481400, outputData1.getMapValue( cumSedimentBackstrip,    2, 3 ), 1e-10 );
   EXPECT_NEAR( -22336.739908481400, outputData1.getMapValue( cumSedimentBackstrip,    3, 2 ), 1e-10 );
   EXPECT_NEAR( -22336.739908481400, outputData1.getMapValue( cumSedimentBackstrip,    3, 3 ), 1e-10 );
   EXPECT_NEAR( 22416.739908481400,  outputData1.getMapValue( cumBasementCompensation, 2, 2 ), 1e-10 );
   EXPECT_NEAR( 22416.739908481400,  outputData1.getMapValue( cumBasementCompensation, 2, 3 ), 1e-10 );
   EXPECT_NEAR( 22416.739908481400,  outputData1.getMapValue( cumBasementCompensation, 3, 2 ), 1e-10 );
   EXPECT_NEAR( 22416.739908481400,  outputData1.getMapValue( cumBasementCompensation, 3, 3 ), 1e-10 );

   //dm=3000,dw=1000
   const DataModel::MockDerivedSurfaceProperty pressureBasement2   ( firstI, firstJ, firstI, firstJ, lastI, lastJ, lastI, lastJ, "pressureBasement",    "Pressure", 10, 5000 );
   const DataModel::MockDerivedSurfaceProperty pressureWaterBottom2( firstI, firstJ, firstI, firstJ, lastI, lastJ, lastI, lastJ, "pressureWaterBottom", "Pressure", 10, 1000 );
   const DataModel::MockDerivedSurfaceProperty depthBasement2      ( firstI, firstJ, firstI, firstJ, lastI, lastJ, lastI, lastJ, "depthBasement",       "Depth",    10, 500  );
   const DataModel::MockDerivedSurfaceProperty depthWaterBottom2   ( firstI, firstJ, firstI, firstJ, lastI, lastJ, lastI, lastJ, "depthWaterBottom",    "Depth",    10, 100  );
   MockInterfaceOutput outputData2 = MockInterfaceOutput( firstI, firstJ, lastI, lastJ );
   DensityCalculator densityCalculator2( firstI,
                                         firstJ,
                                         lastI,
                                         lastJ,
                                         3000,
                                         1000,
                                         pressureBasement2.getMockderivedSurfacePropertyPtr(),
                                         pressureWaterBottom2.getMockderivedSurfacePropertyPtr(),
                                         depthBasement2.getMockderivedSurfacePropertyPtr(),
                                         depthWaterBottom2.getMockderivedSurfacePropertyPtr(),
                                         outputData2,
                                         validator );
   densityCalculator2.compute();
   EXPECT_EQ( 400, outputData2.getMapValue( cumSedimentThickness, 2, 2 ) );
   EXPECT_EQ( 400, outputData2.getMapValue( cumSedimentThickness, 2, 3 ) );
   EXPECT_EQ( 400, outputData2.getMapValue( cumSedimentThickness, 3, 2 ) );
   EXPECT_EQ( 400, outputData2.getMapValue( cumSedimentThickness, 3, 3 ) );
   EXPECT_NEAR( 1019367.9918450600,  outputData2.getMapValue( sedimentDensityMap,      2, 2 ), 1e-8  );
   EXPECT_NEAR( 1019367.9918450600,  outputData2.getMapValue( sedimentDensityMap,      2, 3 ), 1e-8  );
   EXPECT_NEAR( 1019367.9918450600,  outputData2.getMapValue( sedimentDensityMap,      3, 2 ), 1e-8  );
   EXPECT_NEAR( 1019367.9918450600,  outputData2.getMapValue( sedimentDensityMap,      3, 3 ), 1e-8  );
   EXPECT_NEAR( -203273.59836901100, outputData2.getMapValue( cumSedimentBackstrip,    2, 2 ), 1e-9  );
   EXPECT_NEAR( -203273.59836901100, outputData2.getMapValue( cumSedimentBackstrip,    2, 3 ), 1e-9  );
   EXPECT_NEAR( -203273.59836901100, outputData2.getMapValue( cumSedimentBackstrip,    3, 2 ), 1e-9  );
   EXPECT_NEAR( -203273.59836901100, outputData2.getMapValue( cumSedimentBackstrip,    3, 3 ), 1e-9  );
   EXPECT_NEAR( 203673.59836901100,  outputData2.getMapValue( cumBasementCompensation, 2, 2 ), 1e-9  );
   EXPECT_NEAR( 203673.59836901100,  outputData2.getMapValue( cumBasementCompensation, 2, 3 ), 1e-9  );
   EXPECT_NEAR( 203673.59836901100,  outputData2.getMapValue( cumBasementCompensation, 3, 2 ), 1e-9  );
   EXPECT_NEAR( 203673.59836901100,  outputData2.getMapValue( cumBasementCompensation, 3, 3 ), 1e-9  );

   // 2. Test that the outputs are NDV when the nodes are not valid
   MockInterfaceOutput outputData3 = MockInterfaceOutput( firstI, firstJ, lastI, lastJ );
   validator.setIsValid( false );
   DensityCalculator densityCalculator3( firstI,
                                         firstJ,
                                         lastI,
                                         lastJ,
                                         3300,
                                         1030,
                                         pressureBasement.getMockderivedSurfacePropertyPtr(),
                                         pressureWaterBottom.getMockderivedSurfacePropertyPtr(),
                                         depthBasement.getMockderivedSurfacePropertyPtr(),
                                         depthWaterBottom.getMockderivedSurfacePropertyPtr(),
                                         outputData3,
                                         validator );
   densityCalculator3.compute();
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData3.getMapValue( cumSedimentBackstrip,    2, 2 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData3.getMapValue( cumSedimentBackstrip,    2, 3 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData3.getMapValue( cumSedimentBackstrip,    3, 2 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData3.getMapValue( cumSedimentBackstrip,    3, 3 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData3.getMapValue( sedimentDensityMap,      2, 2 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData3.getMapValue( sedimentDensityMap,      2, 3 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData3.getMapValue( sedimentDensityMap,      3, 2 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData3.getMapValue( sedimentDensityMap,      3, 3 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData3.getMapValue( cumSedimentThickness,    2, 2 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData3.getMapValue( cumSedimentThickness,    2, 3 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData3.getMapValue( cumSedimentThickness,    3, 2 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData3.getMapValue( cumSedimentThickness,    3, 3 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData3.getMapValue( cumBasementCompensation, 2, 2 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData3.getMapValue( cumBasementCompensation, 2, 3 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData3.getMapValue( cumBasementCompensation, 3, 2 ) );
   EXPECT_EQ( Interface::DefaultUndefinedMapValue, outputData3.getMapValue( cumBasementCompensation, 3, 3 ) );

}
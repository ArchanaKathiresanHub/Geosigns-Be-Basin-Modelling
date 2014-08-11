/**
 * \file WalderhaugCompactionCalculatorTest.C
 * \brief Test the WalderhaugCompactionCalculator class
 * \author Magali Cougnenc magali.cougnenc@pds.nl
 * \date June 2014
 *
 * Test of the chemical compaction through Walderhaug model
 */

#include "../src/WalderhaugCompactionCalculator.h"
#include "../src/GeoPhysicalConstants.h"
#include "MockGrid.h"
#include <gtest/gtest.h>
#include <iostream>
#include <cmath>

using namespace GeoPhysics;

class testWalderhaugCompactionCalculator : public ::testing::Test
{
public:

   testWalderhaugCompactionCalculator() {;}
   ~testWalderhaugCompactionCalculator() {;}

   double computeTestTimeStep(
         double coatingClay,
         double fractionQuartz,
         double grainSize,
         double time0,
         double time1,
         double temp0,
         double temp1 )
   {
      static const double ln10 = std::log( 10.0 );
      double result = - ( ( 1 - coatingClay ) * 6 * fractionQuartz / grainSize );
      result *= ( time0 - time1 ) * Secs_IN_MA * MolarMassQuartz * 1.98e-22 / DensityQuartz;
      result *= (exp( 0.022 * ( temp1 ) * ln10 ) - exp( 0.022 * ( temp0 ) * ln10 ) );
      result /= ( 0.022 * ln10 * ( temp1 - temp0 ) );
      return result;
   }
};


/*!
 * \brief Tests the input of short and long time steps
 */
TEST_F( testWalderhaugCompactionCalculator, test_extreme_time_values )
{
   WalderhaugCompactionCalculator * my_Object = dynamic_cast < WalderhaugCompactionCalculator* > ( ChemicalCompactionCalculator::create ( "Walderhaug" ) );

   const int size           = 1;
   const int sizeValidNodes = size;

   const double currentTime   = 20.0     ; //[ma]
   const double previousTime1 = 21.0     ; //[ma]
   const double previousTime2 = 100020.0 ; //[ma]

   double       chemicalCompaction1[size] = {0.0}; //[fraction of unit volume]
   double       chemicalCompaction2[size] = {0.0}; //[fraction of unit volume]
   const double porosity[size]            = {0.3}; //[fraction of unit volume]

   const double previousTemperature[size] = {100.0}; //[K]
   const double currentTemperature[size]  = {102.0}; //[K]

   const int validNodes[sizeValidNodes] = {0};
   const int numberLithologies          = 1;
   const int lithoId[size]              = {0};

   const double fractionQuartz [numberLithologies] = {0.65}; //[fraction of unit volume]
   const double grainSize      [numberLithologies] = {0.02}; //[cm]
   const double coatingClay    [numberLithologies] = {0.1};  //[fraction of available quartz surface]

   MockGrid previousGrid( size, currentTime, currentTime, chemicalCompaction1, porosity, previousTemperature, currentTemperature, sizeValidNodes, validNodes,
         lithoId, numberLithologies, fractionQuartz, grainSize, coatingClay );

   MockGrid currentGrid1( size, previousTime1, currentTime, chemicalCompaction1, porosity, previousTemperature, currentTemperature, sizeValidNodes, validNodes,
         lithoId, numberLithologies, fractionQuartz, grainSize, coatingClay );

   MockGrid currentGrid2( size, previousTime2, currentTime, chemicalCompaction1, porosity, previousTemperature, currentTemperature, sizeValidNodes, validNodes,
         lithoId, numberLithologies, fractionQuartz, grainSize, coatingClay );

   //Time step = 0 => no change in the chemical compaction
   my_Object -> computeOnTimeStep ( previousGrid );
   ASSERT_EQ( chemicalCompaction1[0], previousGrid.setChemicalCompaction()[0] );

   //Time step = 1 My => normal change in the chemical compaction
   my_Object -> computeOnTimeStep ( currentGrid1 );
   double result = chemicalCompaction2[0] + computeTestTimeStep( coatingClay[0], fractionQuartz[0], grainSize[0],
         previousTime1, currentTime,
         previousTemperature[0], currentTemperature[0] );
   ASSERT_DOUBLE_EQ ( result, currentGrid1.setChemicalCompaction()[0] );

   //Time step = 100 000 My => chemical compaction still <= -1.0
   my_Object -> computeOnTimeStep ( currentGrid2 );
   ASSERT_LE( -1.0, currentGrid2.setChemicalCompaction()[0] );

}

/*!
 * \brief Tests the input of extreme chemical compaction, porosity and temperature values
 */
TEST_F( testWalderhaugCompactionCalculator, test_extreme_parameters_values )
{
   WalderhaugCompactionCalculator * my_Object = dynamic_cast < WalderhaugCompactionCalculator* > ( ChemicalCompactionCalculator::create ( "Walderhaug" ) );

   const int size           = 36;
   const int sizeValidNodes = size;

   const double previousTime = 21.0; //[s]
   const double currentTime  = 20.0; //[s]

   double chemicalCompaction1[size]; //[fraction of unit volume]
   double chemicalCompaction2[size]; //[fraction of unit volume]
   double porosity           [size]; //[fraction of unit volume]

   std::fill_n( chemicalCompaction1,  size,  0.0 );
   std::fill_n( chemicalCompaction2,  size,  0.0 );

   std::fill_n( porosity,      12,      0.03 );
   std::fill_n( porosity + 12, 12,      0.25 );
   std::fill_n( porosity + 24, size-24, 1.0  );

   //We don't want to test the aglorithm without temperature variation for now
   //that is why the temperatures are never equals here
   double previousTemperature [size]; //[K]
   double currentTemperature  [size]; //[K]

   for ( int index = 0; index < size; index += 3 )
   {
      switch( index % 12 )
      {
      case 0:
         previousTemperature[index    ] = 0.0;
         previousTemperature[index + 1] = 0.0;
         previousTemperature[index + 2] = 0.0;
         currentTemperature [index    ] = 80.0;
         currentTemperature [index + 1] = 120.0;
         currentTemperature [index + 2] = 14000.0;
         break;

      case 3:
         previousTemperature[index    ] = 80.0;
         previousTemperature[index + 1] = 80.0;
         previousTemperature[index + 2] = 80.0;
         currentTemperature [index    ] = 0.0;
         currentTemperature [index + 1] = 120.0;
         currentTemperature [index + 2] = 14000.0;
         break;

      case 6:
         previousTemperature[index    ] = 120.0;
         previousTemperature[index + 1] = 120.0;
         previousTemperature[index + 2] = 120.0;
         currentTemperature [index    ] = 0.0;
         currentTemperature [index + 1] = 80.0;
         currentTemperature [index + 2] = 14000.0;
         break;

      case 9:
         previousTemperature[index    ] = 14000.0;
         previousTemperature[index + 1] = 14000.0;
         previousTemperature[index + 2] = 14000.0;
         currentTemperature [index    ] = 0.0;
         currentTemperature [index + 1] = 80.0;
         currentTemperature [index + 2] = 120.0;
         break;
      }
   }


   int validNodes    [sizeValidNodes];
   for ( int i = 0; i< sizeValidNodes; ++i ){ validNodes[i] = i; }
   const int lithoId [size]     = {0};
   const int numberLithologies  = 1;

   const double fractionQuartz [numberLithologies] = {0.65}; //[fraction of unit volume]
   const double grainSize      [numberLithologies] = {0.02}; //[cm]
   const double coatingClay    [numberLithologies] = {0.1};  //[fraction of available quartz surface]


   MockGrid grid( size, previousTime, currentTime, chemicalCompaction1, porosity, previousTemperature, currentTemperature, sizeValidNodes, validNodes,
         lithoId, numberLithologies, fractionQuartz, grainSize, coatingClay );

   my_Object -> computeOnTimeStep( grid );

   for ( int i=0 ; i<size; ++i )
   {
      double expectedResult = 0;
      if ( currentTemperature[i] > 80.0 )
      {
         expectedResult += computeTestTimeStep( coatingClay[0], fractionQuartz[0], grainSize[0],
               previousTime, currentTime,
               previousTemperature[i], currentTemperature[i] );
      }
      expectedResult += chemicalCompaction2[i];
      if ( expectedResult <= -1.0 ){ expectedResult = -1.0; }

      double result = grid.setChemicalCompaction()[i];

      ASSERT_GE( 0.0, result );                    // chemical compaction >= 0
      ASSERT_LE( -1.0, result );                    // chemical compaction <= 1
      ASSERT_GE( chemicalCompaction1[i], result ); // chemical compaction (t + deltat) <= chemical compaction (t)
      ASSERT_DOUBLE_EQ( expectedResult, result );  // chemical compaction = expected chemical compaction
   }
}

/*!
 * \brief Tests the input of differnet lithologies with extreme parameters values
 */
TEST_F( testWalderhaugCompactionCalculator, test_extreme_lithologies_values )
{
   WalderhaugCompactionCalculator * my_Object = dynamic_cast < WalderhaugCompactionCalculator* > ( ChemicalCompactionCalculator::create ( "Walderhaug" ) );

   const int size           = 27;
   const int sizeValidNodes = size;

   const double previousTime = 21.0; //[s]
   const double currentTime  = 20.0; //[s]

   double chemicalCompaction1 [size]; //[fraction of unit volume]
   double chemicalCompaction2 [size]; //[fraction of unit volume]
   double porosity            [size]; //[fraction of unit volume]

   std::fill_n( chemicalCompaction1,      size,      0.0 ); //[fraction of unit volume]
   std::fill_n( chemicalCompaction2,      size,      0.0 ); //[fraction of unit volume]
   std::fill_n( porosity,                 size,      0.25 ); //[fraction of unit volume]

   double previousTemperature [size]; //[K]
   double currentTemperature  [size]; //[K]

   std::fill_n( previousTemperature,   size,    100.0 ); //[K]
   std::fill_n( currentTemperature,    size,    102.0 ); //[K]

   int validNodes [sizeValidNodes];
   int lithoId    [size];
   const int numberLithologies = size;

   for ( int index = 0; index < size; ++index )
   {
      validNodes [index] = index;
      lithoId    [index] = index;
   }

   double fractionQuartz [numberLithologies]; //[fraction of unit volume]
   double grainSize      [numberLithologies]; //[cm]
   double coatingClay    [numberLithologies]; //[fraction of available quartz surface]

   std::fill_n( fractionQuartz,          9,      0.0    );
   std::fill_n( fractionQuartz + 9,      9,      0.65   );
   std::fill_n( fractionQuartz + 18,     9,      1.0    );

   for ( int index = 0; index < size; ++index )
   {
      grainSize [index    ] = 0.000000001;
      grainSize [index + 1] = 0.02;
      grainSize [index + 2] = 10.0;

      int remainder = index % 9;
      if (      remainder <= 2 ) { coatingClay [index] = 0.0; }
      else if ( remainder >= 6 ) { coatingClay [index] = 1.0; }
      else                       { coatingClay [index] = 0.1; }
   }

   MockGrid grid( size, previousTime, currentTime, chemicalCompaction1, porosity, previousTemperature, currentTemperature, sizeValidNodes, validNodes,
         lithoId, numberLithologies, fractionQuartz, grainSize, coatingClay );

   my_Object -> computeOnTimeStep ( grid );

   for ( int i=0 ; i<size; ++i )
   {
      double expectedResult = 0;
      if ( currentTemperature[i] > 80.0 )
      {
         expectedResult += computeTestTimeStep( coatingClay[i], fractionQuartz[i], grainSize[i],
               previousTime, currentTime,
               previousTemperature[i], currentTemperature[i] );
      }
      expectedResult += chemicalCompaction2[i];
      if ( expectedResult <= -1.0 ){expectedResult = -1.0;}

      double result = grid.setChemicalCompaction()[i];

      ASSERT_GE(  0.0, result );                       // chemical compaction >= 0
      ASSERT_LE( -1.0, result );                       // chemical compaction <= 1
      ASSERT_GE( chemicalCompaction1[i], result );  // chemical compaction (t + deltat) <= chemical compaction (t)
      ASSERT_DOUBLE_EQ( expectedResult, result );   // chemical compaction = expected chemical compaction
   }
}

/*!
 * \brief Tests that initial porosity is stored only at the first time step where T > 80 [celsius degree]
 */
TEST_F( testWalderhaugCompactionCalculator, test_initial_porosity )
{
   WalderhaugCompactionCalculator * my_Object = dynamic_cast < WalderhaugCompactionCalculator* > ( ChemicalCompactionCalculator::create ( "Walderhaug" ) );

   const int size           = 1;
   const int sizeValidNodes = size;

   const double currentTime = 20.0; //[s]
   const double previousTime1 = 21.0; //[s]
   const double previousTime2 = 22.0; //[s]
   const double previousTime3 = 23.0; //[s]

   double chemicalCompaction1 [size] = {0.0}; //[fraction of unit volume]
   double chemicalCompaction2 [size] = {0.0}; //[fraction of unit volume]
   double chemicalCompaction3 [size] = {0.0}; //[fraction of unit volume]
   double chemicalCompaction4 [size] = {0.0}; //[fraction of unit volume]
   const double porosity1     [size] = {0.5}; //[fraction of unit volume]
   const double porosity2     [size] = {0.3}; //[fraction of unit volume]

   const double previousTemperature [size] = {80.0}; //[K]
   const double currentTemperature1 [size] = {81.0}; //[K]
   const double currentTemperature2 [size] = {82.0}; //[K]

   const int validNodes [sizeValidNodes] = {0};
   int lithoId          [size]           = {0};
   const int numberLithologies           = 1;

   const double fractionQuartz [numberLithologies] = {0.65}; //[fraction of unit volume]
   const double grainSize      [numberLithologies] = {0.02}; //[cm]
   const double coatingClay    [numberLithologies] = {0.1};  //[fraction of available quartz surface]

   MockGrid previousGrid( size, previousTime3, previousTime2, chemicalCompaction1, porosity1, previousTemperature, previousTemperature, sizeValidNodes, validNodes,
         lithoId, numberLithologies, fractionQuartz, grainSize, coatingClay );

   MockGrid currentGrid1( size, previousTime2, previousTime1, chemicalCompaction2, porosity1, previousTemperature, currentTemperature1, sizeValidNodes, validNodes,
         lithoId, numberLithologies, fractionQuartz, grainSize, coatingClay );

   MockGrid currentGrid2( size, previousTime1, currentTime, chemicalCompaction3, porosity2, currentTemperature1, currentTemperature2, sizeValidNodes, validNodes,
         lithoId, numberLithologies, fractionQuartz, grainSize, coatingClay );

   // T < 80 [C degree]  => no change in chemical compaction
   my_Object -> computeOnTimeStep ( previousGrid );
   double result1         = previousGrid.setChemicalCompaction()[0];
   double expectedResult1 = chemicalCompaction1[0];
   ASSERT_DOUBLE_EQ( expectedResult1, result1 );
   ASSERT_DOUBLE_EQ( 0.0, result1 );

   // T > 80 [C degree]  for the first time => chemical compaction happens and initial porosity is stored
   double expectedResult2 = computeTestTimeStep( coatingClay[0], fractionQuartz[0], grainSize[0],
		   previousTime2, previousTime1,
         previousTemperature[0], currentTemperature1[0] );
   expectedResult2 += chemicalCompaction1[0];
   my_Object -> computeOnTimeStep ( currentGrid1 );
   double result2         = currentGrid1.setChemicalCompaction()[0];
   ASSERT_DOUBLE_EQ( expectedResult2, result2 );

   // T > 80 [degree C]  => chemical compaction happens and initial porosity stays constant
   double expectedResult3 = computeTestTimeStep( coatingClay[0], fractionQuartz[0], grainSize[0],
		   previousTime1, currentTime,
         currentTemperature1[0], currentTemperature2[0] );
   expectedResult3 *= porosity2[0]/porosity1[0];
   expectedResult3 += chemicalCompaction1[0];
   my_Object -> computeOnTimeStep ( currentGrid2 );
   double result3 = currentGrid2.setChemicalCompaction()[0];
   ASSERT_DOUBLE_EQ( expectedResult3, result3 );
}

/*!
 * \brief Tests current and previous temperatures interaction
 */
TEST_F( testWalderhaugCompactionCalculator, test_interaction_temperatures )
{
   WalderhaugCompactionCalculator * my_Object = dynamic_cast < WalderhaugCompactionCalculator* > ( ChemicalCompactionCalculator::create ( "Walderhaug" ) );

   const int size           = 3;
   const int sizeValidNodes = size;

   const double previousTime = 21.0; //[s]
   const double currentTime  = 20.0;//[s]

   double chemicalCompaction1 [size] = {0.0,0.0,0.0}; //[fraction of unit volume]
   const double porosity      [size] = {0.5,0.5,0.5}; //[fraction of unit volume]

   const double previousTemperature [size] = {100.0,100.0,100.0};                  //[K]
   const double currentTemperature  [size] = {100.0,100.0000000001,99.9999999999}; //[K]

   const int validNodes [sizeValidNodes] = {0,1,2};
   int lithoId          [size]           = {0};
   const int numberLithologies           = 1;

   const double fractionQuartz [numberLithologies] = {0.65}; //[fraction of unit volume]
   const double grainSize      [numberLithologies] = {0.02}; //[cm]
   const double coatingClay    [numberLithologies] = {0.1};  //[fraction of available quartz surface]

   MockGrid previousGrid( size, previousTime, currentTime, chemicalCompaction1, porosity, previousTemperature, currentTemperature, sizeValidNodes, validNodes,
         lithoId, numberLithologies, fractionQuartz, grainSize, coatingClay );


   my_Object -> computeOnTimeStep ( previousGrid );

   for ( int i=0 ; i<size; ++i )
   {
      double result = previousGrid.setChemicalCompaction()[i];
      ASSERT_GE( 0.0, result );                       // chemical compaction >= 0
      ASSERT_LE( -1.0, result );                      // chemical compaction <= 1
      ASSERT_GE( chemicalCompaction1[i], result );    // chemical compaction (t + deltat) <= chemical compaction (t)
   }
}


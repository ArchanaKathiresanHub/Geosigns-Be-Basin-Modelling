//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "../src/MapSmoother.h"

// DataAccess libraries
#include "../../SerialDataAccess/src/Interface/SerialGrid.h"
#include "../../SerialDataAccess/src/Interface/SerialGridMap.h"

#include <gtest/gtest.h>

double minI = 0.0;
double minJ = 0.0;
double maxI = 4.0;
double maxJ = 4.0;
unsigned int numI = 3;
unsigned int numJ = 3;

using namespace CrustalThicknessInterface;
using namespace DataAccess;
using namespace Interface;

void setValuesToMap( GridMap* mapToSmooth ){
   assert( mapToSmooth != nullptr );
   // Set new values to the map to smooth
   //--------------
   //|2  |6  |10.6|
   //|7  |3  |4.4 |
   //|2  |4  |6   |
   //--------------
   mapToSmooth->setValue( 0, 0, 2    );
   mapToSmooth->setValue( 0, 1, 6    );
   mapToSmooth->setValue( 0, 2, 10.6 );
   mapToSmooth->setValue( 1, 0, 7    );
   mapToSmooth->setValue( 1, 1, 3    );
   mapToSmooth->setValue( 1, 2, 4.4  );
   mapToSmooth->setValue( 2, 0, 2    );
   mapToSmooth->setValue( 2, 1, 4    );
   mapToSmooth->setValue( 2, 2, 6    );
}


///1. SerialTest
TEST( MapSmoother, SerialSmoothing )
{
   // Define grids and derived properties
   SerialGrid grid          ( minI, minJ, maxI, maxJ, numI, numJ );
   SerialGridMap mapToSmooth( 0, 0, &grid, 500 );
   bool status;

   // Wrong inputs tests
   {
      MapSmoother smoother(5);
      status = smoother.averageSmoothing( nullptr );
      EXPECT_FALSE( status );
   };

   // Simple average over the same value (500)
   {
      MapSmoother smoother(10);
      status = smoother.averageSmoothing( &mapToSmooth );
      EXPECT_TRUE( status );
      EXPECT_EQ( 500, mapToSmooth.getValue( 0, 0 ) );
      EXPECT_EQ( 500, mapToSmooth.getValue( 0, 1 ) );
      EXPECT_EQ( 500, mapToSmooth.getValue( 0, 2 ) );
      EXPECT_EQ( 500, mapToSmooth.getValue( 1, 0 ) );
      EXPECT_EQ( 500, mapToSmooth.getValue( 1, 1 ) );
      EXPECT_EQ( 500, mapToSmooth.getValue( 1, 2 ) );
      EXPECT_EQ( 500, mapToSmooth.getValue( 2, 0 ) );
      EXPECT_EQ( 500, mapToSmooth.getValue( 2, 1 ) );
      EXPECT_EQ( 500, mapToSmooth.getValue( 2, 2 ) );
   };


   // Set new values to the map to smooth
   //--------------
   //|2  |6  |10.6|
   //|7  |3  |4.4 |
   //|2  |4  |6   |
   //--------------
   setValuesToMap( &mapToSmooth );

   // Smoothing radius = 10 (>mapSize=3x3)
   // Expected result for each cell is the mean of the map (5)
   {
      MapSmoother smoother(10);
      status = smoother.averageSmoothing( &mapToSmooth );
      EXPECT_TRUE( status );
      EXPECT_EQ( 5, mapToSmooth.getValue( 0, 0 ) );
      EXPECT_EQ( 5, mapToSmooth.getValue( 0, 1 ) );
      EXPECT_EQ( 5, mapToSmooth.getValue( 0, 2 ) );
      EXPECT_EQ( 5, mapToSmooth.getValue( 1, 0 ) );
      EXPECT_EQ( 5, mapToSmooth.getValue( 1, 1 ) );
      EXPECT_EQ( 5, mapToSmooth.getValue( 1, 2 ) );
      EXPECT_EQ( 5, mapToSmooth.getValue( 2, 0 ) );
      EXPECT_EQ( 5, mapToSmooth.getValue( 2, 1 ) );
      EXPECT_EQ( 5, mapToSmooth.getValue( 2, 2 ) );
      setValuesToMap( &mapToSmooth ); //reset values for next test
   };
   // Smoothing radius = 1 (<mapSize=3x3)
   // Expected result for each cell
   //---------------------------
   //|4.5     |5.5     |6      |
   //|4       |5       |5.666~ |
   //|4       |4.4     |4.35   |
   //---------------------------
   {
      MapSmoother smoother(1);
      status = smoother.averageSmoothing( &mapToSmooth );
      EXPECT_TRUE( status );
      EXPECT_NEAR( 4.5,             mapToSmooth.getValue( 0, 0 ) , 1e-12 );
      EXPECT_NEAR( 5.5,             mapToSmooth.getValue( 0, 1 ) , 1e-12 );
      EXPECT_NEAR( 6,               mapToSmooth.getValue( 0, 2 ) , 1e-12 );
      EXPECT_NEAR( 4,               mapToSmooth.getValue( 1, 0 ) , 1e-12 );
      EXPECT_NEAR( 5,               mapToSmooth.getValue( 1, 1 ) , 1e-12 );
      EXPECT_NEAR( 5.6666666666666, mapToSmooth.getValue( 1, 2 ),  1e-12 );
      EXPECT_NEAR( 4,               mapToSmooth.getValue( 2, 0 ) , 1e-12 );
      EXPECT_NEAR( 4.4,             mapToSmooth.getValue( 2, 1 ) , 1e-12 );
      EXPECT_NEAR( 4.35,            mapToSmooth.getValue( 2, 2 ) , 1e-12 );
      setValuesToMap( &mapToSmooth ); //reset values for next test
   };
   //Smoothing radius = 0 (no smoothing)
   // Values do not change
   {
      MapSmoother smoother(0);
      status = smoother.averageSmoothing( &mapToSmooth );
      EXPECT_TRUE( status );
      EXPECT_NEAR( 2,    mapToSmooth.getValue( 0, 0 ) , 1e-12 );
      EXPECT_NEAR( 6,    mapToSmooth.getValue( 0, 1 ) , 1e-12 );
      EXPECT_NEAR( 10.6, mapToSmooth.getValue( 0, 2 ) , 1e-12 );
      EXPECT_NEAR( 7,    mapToSmooth.getValue( 1, 0 ) , 1e-12 );
      EXPECT_NEAR( 3,    mapToSmooth.getValue( 1, 1 ) , 1e-12 );
      EXPECT_NEAR( 4.4,  mapToSmooth.getValue( 1, 2 ) , 1e-12 );
      EXPECT_NEAR( 2,    mapToSmooth.getValue( 2, 0 ) , 1e-12 );
      EXPECT_NEAR( 4,    mapToSmooth.getValue( 2, 1 ) , 1e-12 );
      EXPECT_NEAR( 6,    mapToSmooth.getValue( 2, 2 ) , 1e-12 );
      setValuesToMap( &mapToSmooth ); //reset values for next test
   };

   // Set one value to NDV
   //--------------
   //|2  |6  |10.6|
   //|7  |3  |4.4 |
   //|2  |4  |NDV |
   //--------------
   mapToSmooth.setValue( 2, 2, Interface::DefaultUndefinedMapValue );
   // Smoothing radius = 1 (<mapSize=3x3)
   // Expected result for each cell
   //--------------------
   //|4.5  |5.5   |6    |
   //|4    |4.875 |5.6  |
   //|4    |4.08  |NDV  |
   //--------------------
   {
      MapSmoother smoother(1);
      status = smoother.averageSmoothing( &mapToSmooth );
      EXPECT_TRUE( status );
      EXPECT_NEAR( 4.5,   mapToSmooth.getValue( 0, 0 ) , 1e-12 );
      EXPECT_NEAR( 5.5,   mapToSmooth.getValue( 0, 1 ) , 1e-12 );
      EXPECT_NEAR( 6,     mapToSmooth.getValue( 0, 2 ) , 1e-12 );
      EXPECT_NEAR( 4,     mapToSmooth.getValue( 1, 0 ) , 1e-12 );
      EXPECT_NEAR( 4.875, mapToSmooth.getValue( 1, 1 ) , 1e-12 );
      EXPECT_NEAR( 5.6,   mapToSmooth.getValue( 1, 2 ) , 1e-12 );
      EXPECT_NEAR( 4,     mapToSmooth.getValue( 2, 0 ) , 1e-12 );
      EXPECT_NEAR( 4.08,  mapToSmooth.getValue( 2, 1 ) , 1e-12 );
      EXPECT_EQ( Interface::DefaultUndefinedMapValue, mapToSmooth.getValue( 2, 2 ) );
      setValuesToMap( &mapToSmooth ); //reset values for next test
   };
}
//
// Copyright (C) 2017 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include <cmath>
#include <memory>

#include "../../DataAccess/src/Grid.h"
#include "../../DataAccess/src/Interface.h"
#include "../../SerialDataAccess/src/SerialGrid.h"
#include "../../SerialDataAccess/src/SerialGridMap.h"
#include "../../utilities/src/array.h"

#include <gtest/gtest.h>

static const double       s_minI = 0.0;
static const double       s_minJ = -2.0;
static const double       s_maxI = 10.0;
static const double       s_maxJ = 2.0;
static const int          s_numI = 11;
static const int          s_numJ = 5;
static const unsigned int s_depth = 3;
static const double       s_value = 23.0;
static const double       s_subValue = 3.0;

// 3D SerialGridMap. Sizes (X,Y,Z) = (11,5,3)
//
//    |
// 2  | X X X X X X X X X X X
//    |
// 1  | X X X X X X X X X X X
//    |
// 0  | X X X X X X X X X X X
//    |
// -1 | X X X X X X X X X X X
//    |
// -2 | X X X X X X X X X X X
//    *------------------------>
//      0 1 2 3 4 5 6 7 8 9 10


// This function tests the set up of each constructor
void checkSerialGridMap( DataAccess::Interface::SerialGridMap * sGridMap )
{
   EXPECT_EQ( sGridMap->minJ(), s_minJ );
   EXPECT_EQ( sGridMap->retrieved(), true );
   EXPECT_EQ( sGridMap->retrieveData( false ), true );
   EXPECT_EQ( sGridMap->retrieveData( false ), true );
   EXPECT_EQ( sGridMap->retrieveGhostedData(), true );
   EXPECT_EQ( sGridMap->retrieved(), true );
   EXPECT_EQ( sGridMap->minI(), 0.0 );
   EXPECT_EQ( sGridMap->firstI(), 0 );
   EXPECT_EQ( sGridMap->lastI(), s_numI-1 );
   EXPECT_EQ( sGridMap->numI(), s_numI );
   EXPECT_EQ( sGridMap->firstJ(), 0 );
   EXPECT_EQ( sGridMap->lastJ(), s_numJ-1 );
   EXPECT_EQ( sGridMap->numJ(), s_numJ );
   EXPECT_EQ( sGridMap->firstK(), 0 );
   EXPECT_EQ( sGridMap->lastK(), s_depth-1 );
   EXPECT_EQ( sGridMap->getDepth(), s_depth );
   EXPECT_EQ( sGridMap->getUndefinedValue(), DataAccess::Interface::DefaultUndefinedMapValue );
   EXPECT_EQ( sGridMap->restoreData(false,false), true );
   EXPECT_EQ( sGridMap->restoreData(false,false), true );
   EXPECT_TRUE( sGridMap->getValues() != nullptr );
   EXPECT_FALSE( sGridMap->isAscendingOrder() );
   EXPECT_DOUBLE_EQ( sGridMap->deltaI(), (s_maxI-s_minI)/(s_numI-1) );
   EXPECT_DOUBLE_EQ( sGridMap->deltaJ(), (s_maxJ-s_minJ)/(s_numJ-1) );
}


// Constructor #1 testing
TEST( SerialGridMap, Constructor1 )
{
   std::unique_ptr<DataAccess::Interface::Grid> grid( new DataAccess::Interface::SerialGrid( s_minI, s_minJ, s_maxI, s_maxJ, s_numI, s_numJ ) );
   float *** array = Array < float >::create3d (grid->numIGlobal(), grid->numJGlobal(), s_depth);
   for (int i = 0; i < grid->numIGlobal(); ++i)
   {
      for (int j = 0; j < grid->numJGlobal(); ++j)
      {
         for (unsigned int k = 0; k < s_depth; ++k)
         {
            array[i][j][k] = s_value;
         }
      }
   }
   const unsigned int arraySize = grid->numIGlobal()*grid->numJGlobal()*s_depth;
   std::unique_ptr<DataAccess::Interface::SerialGridMap> sGridMap(
      new DataAccess::Interface::SerialGridMap(nullptr, 0, grid.get(), DataAccess::Interface::DefaultUndefinedMapValue, s_depth, array) );

   checkSerialGridMap( sGridMap.get() );
   EXPECT_DOUBLE_EQ( sGridMap->getConstantValue(), s_value );
   EXPECT_DOUBLE_EQ( sGridMap->getAverageValue(), s_value );
   EXPECT_EQ( sGridMap->getNumberOfDefinedValues(), arraySize );
   for (int i = 0; i < grid->numIGlobal(); ++i)
   {
      for (int j = 0; j < grid->numJGlobal(); ++j)
      {
         for (unsigned int k = 0; k < s_depth; ++k)
         {
            EXPECT_DOUBLE_EQ( sGridMap->getValue(i, j, k), s_value );
         }
      }
   }

   Array < float >::delete3d (array);
}


// Constructor #2 testing
TEST( SerialGridMap, Constructor2 )
{
   std::unique_ptr<DataAccess::Interface::Grid> grid( new DataAccess::Interface::SerialGrid( s_minI, s_minJ, s_maxI, s_maxJ, s_numI, s_numJ ) );
   std::unique_ptr<DataAccess::Interface::SerialGridMap> sGridMap(
      new DataAccess::Interface::SerialGridMap(nullptr, 0, grid.get(), DataAccess::Interface::DefaultUndefinedMapValue, s_depth) );

   checkSerialGridMap( sGridMap.get() );
   EXPECT_DOUBLE_EQ( sGridMap->getConstantValue(), DataAccess::Interface::DefaultUndefinedMapValue );
   EXPECT_EQ( sGridMap->getNumberOfDefinedValues(), 0 );
   EXPECT_DOUBLE_EQ( sGridMap->getAverageValue(), DataAccess::Interface::DefaultUndefinedMapValue );
}


// Constructor #3 testing
TEST( SerialGridMap, Constructor3 )
{
   std::unique_ptr<DataAccess::Interface::Grid> grid( new DataAccess::Interface::SerialGrid( s_minI, s_minJ, s_maxI, s_maxJ, s_numI, s_numJ ) );

   std::unique_ptr<DataAccess::Interface::SerialGridMap> sGridMap1( new DataAccess::Interface::SerialGridMap(0, 0, grid.get(), s_value, s_depth) );
   std::unique_ptr<DataAccess::Interface::SerialGridMap> sGridMap2( new DataAccess::Interface::SerialGridMap(0, 0, grid.get(), s_value, s_depth) );
   std::unique_ptr<DataAccess::Interface::SerialGridMap> sGridMap(
      new DataAccess::Interface::SerialGridMap(0, 0, sGridMap1.get(), sGridMap2.get(), [](double a, double b){return a-b;} ) );

   checkSerialGridMap( sGridMap.get() );
   EXPECT_DOUBLE_EQ( sGridMap->getConstantValue(), 0 );
   EXPECT_EQ( sGridMap->getNumberOfDefinedValues(), grid->numIGlobal()*grid->numJGlobal()*s_depth );
   EXPECT_DOUBLE_EQ( sGridMap->getAverageValue(), 0 );
}


// Constructor #4 testing
TEST( SerialGridMap, Constructor4 )
{
   std::unique_ptr<DataAccess::Interface::Grid> grid( new DataAccess::Interface::SerialGrid( s_minI, s_minJ, s_maxI, s_maxJ, s_numI, s_numJ ) );
   std::unique_ptr<DataAccess::Interface::SerialGridMap> sGridMap2( new DataAccess::Interface::SerialGridMap(0, 0, grid.get(), s_value, s_depth) );
   std::unique_ptr<DataAccess::Interface::SerialGridMap> sGridMap( new DataAccess::Interface::SerialGridMap(0, 0, sGridMap2.get(), [](double a){return 2*a;} ) );

   checkSerialGridMap( sGridMap.get() );
   EXPECT_DOUBLE_EQ( sGridMap->getConstantValue(), 2*s_value );
   EXPECT_EQ( sGridMap->getNumberOfDefinedValues(), grid->numIGlobal()*grid->numJGlobal()*s_depth );
   EXPECT_DOUBLE_EQ( sGridMap->getAverageValue(), 2*s_value );
}


// Constructor #5 testing
TEST( SerialGridMap, Constructor5 )
{
   DataAccess::Interface::AdditionFunctor addFct;

   std::unique_ptr<DataAccess::Interface::Grid> grid( new DataAccess::Interface::SerialGrid( s_minI, s_minJ, s_maxI, s_maxJ, s_numI, s_numJ ) );

   std::unique_ptr<DataAccess::Interface::SerialGridMap> sGridMap1( new DataAccess::Interface::SerialGridMap(0, 0, grid.get(), s_value, s_depth) );
   std::unique_ptr<DataAccess::Interface::SerialGridMap> sGridMap2( new DataAccess::Interface::SerialGridMap(0, 0, grid.get(), s_value, s_depth) );
   std::unique_ptr<DataAccess::Interface::SerialGridMap> sGridMap ( new DataAccess::Interface::SerialGridMap(0, 0, sGridMap1.get(), sGridMap2.get(), addFct) );

   checkSerialGridMap( sGridMap.get() );
   EXPECT_DOUBLE_EQ( sGridMap->getConstantValue(), 2*s_value );
   EXPECT_EQ( sGridMap->getNumberOfDefinedValues(), grid->numIGlobal()*grid->numJGlobal()*s_depth );
   EXPECT_DOUBLE_EQ( sGridMap->getAverageValue(), 2*s_value );
}


// Constructor #6 testing
TEST( SerialGridMap, Constructor6 )
{
   DataAccess::Interface::SubtractConstant subFct( s_subValue );

   std::unique_ptr<DataAccess::Interface::Grid> grid( new DataAccess::Interface::SerialGrid( s_minI, s_minJ, s_maxI, s_maxJ, s_numI, s_numJ ) );
   std::unique_ptr<DataAccess::Interface::SerialGridMap> sGridMap2( new DataAccess::Interface::SerialGridMap(0, 0, grid.get(), s_value, s_depth) );
   std::unique_ptr<DataAccess::Interface::SerialGridMap> sGridMap ( new DataAccess::Interface::SerialGridMap(0, 0, sGridMap2.get(), subFct) );

   checkSerialGridMap( sGridMap.get() );
   EXPECT_DOUBLE_EQ( sGridMap->getConstantValue(), s_value - s_subValue );
   EXPECT_EQ( sGridMap->getNumberOfDefinedValues(), grid->numIGlobal()*grid->numJGlobal()*s_depth );
   EXPECT_DOUBLE_EQ( sGridMap->getAverageValue(), s_value - s_subValue );
}


// Get and set
TEST( SerialGridMap, GetAndSet )
{
   std::unique_ptr<DataAccess::Interface::Grid> grid( new DataAccess::Interface::SerialGrid( s_minI, s_minJ, s_maxI, s_maxJ, s_numI, s_numJ ) );
   float *** array = Array < float >::create3d (grid->numIGlobal(), grid->numJGlobal(), s_depth);
   for (int i = 0; i < grid->numIGlobal(); ++i)
   {
      for (int j = 0; j < grid->numJGlobal(); ++j)
      {
         for (unsigned int k = 0; k < s_depth; ++k)
         {
            array[i][j][k] = s_value;
         }
      }
   }
   std::unique_ptr<DataAccess::Interface::SerialGridMap> sGridMap(
      new DataAccess::Interface::SerialGridMap(nullptr, 0, grid.get(), DataAccess::Interface::DefaultUndefinedMapValue, s_depth, array) );

   EXPECT_TRUE( sGridMap->isConstant() );

   EXPECT_DOUBLE_EQ( sGridMap->getValue((unsigned int)0,(unsigned int)0), s_value );
   EXPECT_DOUBLE_EQ( sGridMap->getValue((unsigned int)0,(unsigned int)0,(unsigned int)0), s_value );
   EXPECT_DOUBLE_EQ( sGridMap->getValue((unsigned int)0,(unsigned int)0,.0), s_value );
   EXPECT_DOUBLE_EQ( sGridMap->getValue(.0,.0,.0), s_value );

   EXPECT_FALSE( sGridMap->setValue(2*s_numI,2*s_numI,2*s_depth, 2*s_value) );
   EXPECT_TRUE( sGridMap->setValue((unsigned int)0,(unsigned int)0,(unsigned int)0, 2*s_value) );
   EXPECT_FALSE( sGridMap->isConstant() );
   EXPECT_DOUBLE_EQ( sGridMap->getValue((unsigned int)0,(unsigned int)0,(unsigned int)0), 2*s_value );

   EXPECT_DOUBLE_EQ( sGridMap->getFractionalValue(1.,(unsigned int)2,(unsigned int)2,(unsigned int)2), s_value );
   EXPECT_DOUBLE_EQ( sGridMap->getFractionalValue(.5,(unsigned int)2,(unsigned int)2,(unsigned int)2), .5*s_value );
   EXPECT_DOUBLE_EQ( sGridMap->getFractionalValue(0.,(unsigned int)2,(unsigned int)2,(unsigned int)2), 0. );

   Array < float >::delete3d (array);
}


// Min/max
TEST( SerialGridMap, MinMax )
{
   std::unique_ptr<DataAccess::Interface::Grid> grid( new DataAccess::Interface::SerialGrid( s_minI, s_minJ, s_maxI, s_maxJ, s_numI, s_numJ ) );
   float *** array = Array < float >::create3d (grid->numIGlobal(), grid->numJGlobal(), s_depth);
   for (int i = 0; i < grid->numIGlobal(); ++i)
   {
      for (int j = 0; j < grid->numJGlobal(); ++j)
      {
         for (unsigned int k = 0; k < s_depth; ++k)
         {
            array[i][j][k] = s_value;
         }
      }
   }
   std::unique_ptr<DataAccess::Interface::SerialGridMap> sGridMap(
      new DataAccess::Interface::SerialGridMap(nullptr, 0, grid.get(), DataAccess::Interface::DefaultUndefinedMapValue, s_depth, array) );

   double min, max;
   sGridMap->getMinMaxValue(min,max);

   EXPECT_TRUE( sGridMap->isConstant() );
   EXPECT_DOUBLE_EQ( min, s_value );
   EXPECT_TRUE( min == max );

   EXPECT_TRUE( sGridMap->setValue((unsigned int)0,(unsigned int)0,(unsigned int)0, 2*s_value) );

   sGridMap->getMinMaxValue(min,max);
   EXPECT_FALSE( sGridMap->isConstant() );
   EXPECT_FALSE( min == max );
   EXPECT_DOUBLE_EQ( min, s_value );
   EXPECT_DOUBLE_EQ( max, 2*s_value );

   Array < float >::delete3d (array);
}


// Average value
TEST( SerialGridMap, Average )
{
   const int numJ = 4;
   const unsigned int depth = 2;
   std::unique_ptr<DataAccess::Interface::Grid> grid( new DataAccess::Interface::SerialGrid( s_minI, s_minJ, s_maxI, s_maxJ, s_numI, numJ ) );
   float *** array = Array < float >::create3d (grid->numIGlobal(), grid->numJGlobal(), depth);
   for (int i = 0; i < grid->numIGlobal(); ++i)
   {
      int j = 0;
      for (; j < numJ/2; ++j)
      {
         for (unsigned int k = 0; k < depth; ++k)
         {
            array[i][j][k] = static_cast<float>(s_value + std::pow(-1.,k));
         }
      }
      for (; j < grid->numJGlobal(); ++j)
      {
         for (unsigned int k = 0; k < depth; ++k)
         {
            array[i][j][k] = static_cast<float>(s_value + std::pow(-1.,k));
         }
      }
   }
   std::unique_ptr<DataAccess::Interface::SerialGridMap> sGridMap(
      new DataAccess::Interface::SerialGridMap(nullptr, 0, grid.get(), DataAccess::Interface::DefaultUndefinedMapValue, depth, array) );

   double min, max;
   sGridMap->getMinMaxValue(min,max);

   EXPECT_FALSE( sGridMap->isConstant() );
   EXPECT_DOUBLE_EQ( min, s_value - 1 );
   EXPECT_DOUBLE_EQ( max, s_value + 1 );
   EXPECT_DOUBLE_EQ( sGridMap->getAverageValue(), s_value );

   Array < float >::delete3d (array);
}


// Sum of values
TEST( SerialGridMap, Sum )
{
   float sum = 0.f;
   float sumSquared = 0.f;
   std::unique_ptr<DataAccess::Interface::Grid> grid( new DataAccess::Interface::SerialGrid( s_minI, s_minJ, s_maxI, s_maxJ, s_numI, s_numJ ) );
   float *** array = Array < float >::create3d (grid->numIGlobal(), grid->numJGlobal(), s_depth);
   for (int i = 0; i < grid->numIGlobal(); ++i)
   {
      for (int j = 0; j < grid->numJGlobal(); ++j)
      {
         for (unsigned int k = 0; k < s_depth; ++k)
         {
            float val = static_cast<float>(i + j + k);
            array[i][j][k] = val;
            sum += val;
            sumSquared += val*val;
         }
      }
   }
   std::unique_ptr<DataAccess::Interface::SerialGridMap> sGridMap(
      new DataAccess::Interface::SerialGridMap(nullptr, 0, grid.get(), DataAccess::Interface::DefaultUndefinedMapValue, s_depth, array) );

   EXPECT_DOUBLE_EQ( sGridMap->getSumOfValues(), sum );
   EXPECT_DOUBLE_EQ( sGridMap->getSumOfSquaredValues(), sumSquared );

   Array < float >::delete3d (array);
}


// HighRes2LowRes
TEST( SerialGridMap, HighRes2LowRes )
{
   const int size = 4;
   const unsigned int depth = 1;
   std::unique_ptr<DataAccess::Interface::Grid> gridHighRes( new DataAccess::Interface::SerialGrid( s_minI, s_minJ, s_maxI, s_maxJ, size, size ) );
   float *** array = Array < float >::create3d (gridHighRes->numIGlobal(), gridHighRes->numJGlobal(), depth);
   for (int i = 0; i < gridHighRes->numIGlobal(); ++i)
   {
      for (int j = 0; j < gridHighRes->numJGlobal(); ++j)
      {
         for (unsigned int k = 0; k < depth; ++k)
         {
            array[i][j][k] = s_value;
         }
      }
   }
   std::unique_ptr<DataAccess::Interface::SerialGridMap> sGridMapHighRes(
      new DataAccess::Interface::SerialGridMap(nullptr, 0, gridHighRes.get(), DataAccess::Interface::DefaultUndefinedMapValue, depth, array) );

   std::unique_ptr<DataAccess::Interface::Grid> gridLowRes( new DataAccess::Interface::SerialGrid( s_minI, s_minJ, s_maxI, s_maxJ, size/2, size/2 ) );
   std::unique_ptr<DataAccess::Interface::SerialGridMap> sGridMapLowRes(
      new DataAccess::Interface::SerialGridMap(nullptr, 0, gridLowRes.get(), 0.) );

   EXPECT_TRUE( sGridMapHighRes->transformHighRes2LowRes( sGridMapLowRes.get() ) );

   EXPECT_DOUBLE_EQ( sGridMapLowRes->getValue(sGridMapLowRes->firstI(),sGridMapLowRes->firstJ()),
                     sGridMapHighRes->getValue(sGridMapHighRes->firstI(),sGridMapHighRes->firstJ()) );
   EXPECT_DOUBLE_EQ( sGridMapLowRes->getValue(sGridMapLowRes->firstI(),sGridMapLowRes->firstJ()+sGridMapLowRes->numJ()-1),
                     sGridMapHighRes->getValue(sGridMapHighRes->firstI(),sGridMapHighRes->firstJ()+sGridMapLowRes->numJ()-1) );
   EXPECT_DOUBLE_EQ( sGridMapLowRes->getValue(sGridMapLowRes->firstI()+sGridMapLowRes->numI()-1,sGridMapLowRes->firstJ()),
                     sGridMapHighRes->getValue(sGridMapHighRes->firstI()+sGridMapLowRes->numI()-1,sGridMapHighRes->firstJ()) );
   EXPECT_DOUBLE_EQ( sGridMapLowRes->getValue(sGridMapLowRes->firstI()+sGridMapLowRes->numI()-1,sGridMapLowRes->firstJ()+sGridMapLowRes->numJ()-1),
                     sGridMapHighRes->getValue(sGridMapHighRes->firstI()+sGridMapLowRes->numI()-1,sGridMapHighRes->firstJ()+sGridMapLowRes->numJ()-1) );

   Array < float >::delete3d (array);
}


// LowRes2HighRes
TEST( SerialGridMap, LowRes2HighRes )
{
   const int size = 4;
   const unsigned int depth = 1;
   std::unique_ptr<DataAccess::Interface::Grid> gridLowRes( new DataAccess::Interface::SerialGrid( s_minI, s_minJ, s_maxI, s_maxJ, size/2, size/2 ) );
   float *** array = Array < float >::create3d (gridLowRes->numIGlobal(), gridLowRes->numJGlobal(), depth);
   for (int i = 0; i < gridLowRes->numIGlobal(); ++i)
   {
      for (int j = 0; j < gridLowRes->numJGlobal(); ++j)
      {
         for (unsigned int k = 0; k < depth; ++k)
         {
            array[i][j][k] = s_value;
         }
      }
   }
   std::unique_ptr<DataAccess::Interface::SerialGridMap> sGridMapLowRes(
      new DataAccess::Interface::SerialGridMap(nullptr, 0, gridLowRes.get(), DataAccess::Interface::DefaultUndefinedMapValue, depth, array) );

   std::unique_ptr<DataAccess::Interface::Grid> gridHighRes( new DataAccess::Interface::SerialGrid( s_minI, s_minJ, s_maxI, s_maxJ, size/2, size/2 ) );
   std::unique_ptr<DataAccess::Interface::SerialGridMap> sGridMapHighRes(
      new DataAccess::Interface::SerialGridMap(nullptr, 0, gridHighRes.get(), 0.) );

   EXPECT_TRUE( sGridMapLowRes->transformLowRes2HighRes( sGridMapHighRes.get() ) );

   EXPECT_DOUBLE_EQ( sGridMapLowRes->getValue(sGridMapLowRes->firstI(),sGridMapLowRes->firstJ()),
                     sGridMapHighRes->getValue(sGridMapHighRes->firstI(),sGridMapHighRes->firstJ()) );
   EXPECT_DOUBLE_EQ( sGridMapLowRes->getValue(sGridMapLowRes->firstI(),sGridMapLowRes->firstJ()+sGridMapLowRes->numJ()-1),
                     sGridMapHighRes->getValue(sGridMapHighRes->firstI(),sGridMapHighRes->firstJ()+sGridMapLowRes->numJ()-1) );
   EXPECT_DOUBLE_EQ( sGridMapLowRes->getValue(sGridMapLowRes->firstI()+sGridMapLowRes->numI()-1,sGridMapLowRes->firstJ()),
                     sGridMapHighRes->getValue(sGridMapHighRes->firstI()+sGridMapLowRes->numI()-1,sGridMapHighRes->firstJ()) );
   EXPECT_DOUBLE_EQ( sGridMapLowRes->getValue(sGridMapLowRes->firstI()+sGridMapLowRes->numI()-1,sGridMapLowRes->firstJ()+sGridMapLowRes->numJ()-1),
                     sGridMapHighRes->getValue(sGridMapHighRes->firstI()+sGridMapLowRes->numI()-1,sGridMapHighRes->firstJ()+sGridMapLowRes->numJ()-1) );

   Array < float >::delete3d (array);
}

// SetValues
TEST( SerialGridMap, SetValues )
{
   const int size = 4;
   const unsigned int depth = 1;
   std::unique_ptr<DataAccess::Interface::SerialGrid> grid( new DataAccess::Interface::SerialGrid( s_minI, s_minJ, s_maxI, s_maxJ, size/2, size/2 ) );
   float *** array = Array < float >::create3d (grid->numIGlobal(), grid->numJGlobal(), depth);
   for (int i = 0; i < grid->numIGlobal(); ++i)
   {
      for (int j = 0; j < grid->numJGlobal(); ++j)
      {
         for (unsigned int k = 0; k < depth; ++k)
         {
            array[i][j][k] = s_value;
         }
      }
   }
   std::unique_ptr<DataAccess::Interface::SerialGridMap> sGridMap(
      new DataAccess::Interface::SerialGridMap(nullptr, 0, grid.get(), DataAccess::Interface::DefaultUndefinedMapValue, depth, array) );
   sGridMap->setValues( 123.456 );
   EXPECT_TRUE( sGridMap->isConstant() );
   EXPECT_DOUBLE_EQ( sGridMap->getConstantValue(), 123.456 );
}

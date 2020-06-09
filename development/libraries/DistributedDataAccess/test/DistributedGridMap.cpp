//
// Copyright (C) 2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include <assert.h>
#include <iostream>
#include <memory>
#include "petsc.h"

#include "../../utilities/src/FormattingException.h"
#include "../src/DistributedGrid.h"
#include "../src/DistributedGridMap.h"
#include "../../DataAccess/src/Grid.h"
#include "../../DataAccess/src/Interface.h"
#include "../../SerialDataAccess/src/SerialGrid.h"
#include "../../utilities/src/array.h"

#include <gtest/gtest.h>

static const double       minI = 0.0;
static const double       minJ = -2.0;
static const double       maxI = 10.0;
static const double       maxJ = 2.0;
static const int          numI = 11;
static const int          numJ = 5;
static const unsigned int depth = 3;
static const double       value = 23.0;
static const double       subValue = 3.0;

// 3D DistributedGridMap. Sizes (X,Y,Z) = (11,5,3)
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
//
// 2 MPI processes split
//
//    |
// 2  | 1 1 1 1 1 1 G1
//    |
// 1  | 1 1 1 1 1 1 G1
//    |
// 0  | 1 1 1 1 1 1 G1
//    |
// -1 | 1 1 1 1 1 1 G1
//    |
// -2 | 1 1 1 1 1 1 G1
//    *------------------------>
//      0 1 2 3 4 5 6 7 8 9 10
//
//    |
// 2  |           G2 2 2 2 2 2
//    |
// 1  |           G2 2 2 2 2 2
//    |
// 0  |           G2 2 2 2 2 2
//    |
// -1 |           G2 2 2 2 2 2
//    |
// -2 |           G2 2 2 2 2 2
//    *------------------------>
//      0 1 2 3 4 5 6 7 8 9 10
//
//
//
// 4 MPI processes split
//
//    |
// 2  | 1 1 1 G1
//    |
// 1  | 1 1 1 G1
//    |
// 0  | 1 1 1 G1
//    |
// -1 | 1 1 1 G1
//    |
// -2 | 1 1 1 G1
//    *------------------------>
//      0 1 2 3 4 5 6 7 8 9 10
//
//    |
// 2  |    G2 2 2 2 G2
//    |
// 1  |    G2 2 2 2 G2
//    |
// 0  |    G2 2 2 2 G2
//    |
// -1 |    G2 2 2 2 G2
//    |
// -2 |    G2 2 2 2 G2
//    *------------------------>
//      0 1 2 3 4 5 6 7 8 9 10
//
//    |
// 2  |          G3 3 3 3 G3
//    |
// 1  |          G3 3 3 3 G3
//    |
// 0  |          G3 3 3 3 G3
//    |
// -1 |          G3 3 3 3 G3
//    |
// -2 |          G3 3 3 3 G3
//    *------------------------>
//      0 1 2 3 4 5 6 7 8 9 10
//
//    |
// 2  |                G4 4 4
//    |
// 1  |                G4 4 4
//    |
// 0  |                G4 4 4
//    |
// -1 |                G4 4 4
//    |
// -2 |                G4 4 4
//    *------------------------>
//      0 1 2 3 4 5 6 7 8 9 10

namespace TestingOperator
{
   // Binary operator for testing constructor #4
   static double minus (double a, double b)
   {
      return a - b;
   }
#ifdef NO_ASSERT_DEATH
   // Unary operator for testing constructor #5
   static double doubleVal (double a)
   {
      return 2.0 * a;
   }
#endif
}

struct MPIHelper
{
   MPIHelper()
   {
      PetscInitialize(0, 0, 0, 0);
   }

   ~MPIHelper()
   {
      MPI_Barrier(PETSC_COMM_WORLD);
      PetscFinalize();
   }

   static MPIHelper & instance()
   {
      static MPIHelper object;
      return object;
   }

   static int rank()
   {
      instance();

      int rank;
      MPI_Comm_rank( PETSC_COMM_WORLD, &rank);
      return rank;
   }

   static int size()
   {
      instance();

      int size;
      MPI_Comm_size( PETSC_COMM_WORLD, &size);
      return size;
   }

   static void barrier()
   {
      instance();
      MPI_Barrier(PETSC_COMM_WORLD);
   }
};


#ifdef NO_ASSERT_DEATH
// This function tests the set up of each constructor defined by the version index
void checkDistributedGridMap( DataAccess::Interface::DistributedGridMap * dGridMap,
                              const unsigned int ctrVer )
{
   const int mpiSize = MPIHelper::size();
   if( (mpiSize != 2) and (mpiSize != 4) )
   {
      EXPECT_EQ( "", "This test can be run only with 2 or 4 MPI processes" );
   }
   else
   {
      EXPECT_EQ( dGridMap->minJ(), minJ );
      EXPECT_EQ( dGridMap->retrieved(), false );
      EXPECT_EQ( dGridMap->retrieveData( false ), true );
      EXPECT_EQ( dGridMap->retrieveData( false ), false ); // It won't do anything because the map is already retrieved
      EXPECT_EQ( dGridMap->retrieveGhostedData(), false ); // It won't do anything because the map is already retrieved
      EXPECT_EQ( dGridMap->retrieved(), true );
      EXPECT_EQ( dGridMap->firstJ(), 0 );
      EXPECT_EQ( dGridMap->firstJ(true), 0 );
      EXPECT_EQ( dGridMap->lastJ(), 4 );
      EXPECT_EQ( dGridMap->lastJ(true), 4 );
      EXPECT_EQ( dGridMap->numJ(), numJ );
      EXPECT_EQ( dGridMap->firstK(), 0 );
      EXPECT_EQ( dGridMap->lastK(), depth-1 );
      EXPECT_EQ( dGridMap->getDepth(), depth );
      EXPECT_EQ( dGridMap->getUndefinedValue(), DataAccess::Interface::DefaultUndefinedMapValue );
      if( ctrVer == 1 ) EXPECT_EQ( dGridMap->getConstantValue(), DataAccess::Interface::DefaultUndefinedMapValue );
      else if( ctrVer == 4 ) EXPECT_EQ( dGridMap->getConstantValue(), 0 );
      else if( ctrVer == 5 or ctrVer == 6 ) EXPECT_EQ( dGridMap->getConstantValue(), 2.0 * value );
      else if( ctrVer == 7 ) EXPECT_EQ( dGridMap->getConstantValue(), value - subValue );
      else EXPECT_EQ( dGridMap->getConstantValue(), value );

      const int myRank = MPIHelper::rank();
      if (mpiSize == 2)
      {
         if( myRank == 0 )
         {
            EXPECT_EQ( dGridMap->minI(), 0.0 );
            EXPECT_EQ( dGridMap->firstI(), 0 );
            EXPECT_EQ( dGridMap->firstI(true), 0 );
            EXPECT_EQ( dGridMap->lastI(), 5 );
            EXPECT_EQ( dGridMap->lastI(true), 6 );
            EXPECT_EQ( dGridMap->numI(), 6 );
         }
         if( myRank == 1 )
         {
            EXPECT_EQ( dGridMap->minI(), 6.0 );
            EXPECT_EQ( dGridMap->firstI(), 6 );
            EXPECT_EQ( dGridMap->firstI(true), 5 );
            EXPECT_EQ( dGridMap->lastI(), numI-1 );
            EXPECT_EQ( dGridMap->lastI(true), numI-1 );
            EXPECT_EQ( dGridMap->numI(), 5 );
         }
      }
      else if (mpiSize == 4)
      {
         if( myRank == 0 )
         {
            EXPECT_EQ( dGridMap->minI(), 0.0 );
            EXPECT_EQ( dGridMap->firstI(), 0 );
            EXPECT_EQ( dGridMap->firstI(true), 0 );
            EXPECT_EQ( dGridMap->lastI(), 2 );
            EXPECT_EQ( dGridMap->lastI(true), 3 );
            EXPECT_EQ( dGridMap->numI(), 3 );
         }
         if( myRank == 1 )
         {
            EXPECT_EQ( dGridMap->minI(), 3.0 );
            EXPECT_EQ( dGridMap->firstI(), 3 );
            EXPECT_EQ( dGridMap->firstI(true), 2 );
            EXPECT_EQ( dGridMap->lastI(), 5 );
            EXPECT_EQ( dGridMap->lastI(true), 6 );
            EXPECT_EQ( dGridMap->numI(), 3 );
         }
         if( myRank == 2 )
         {
            EXPECT_EQ( dGridMap->minI(), 6.0 );
            EXPECT_EQ( dGridMap->firstI(), 6 );
            EXPECT_EQ( dGridMap->firstI(true), 5 );
            EXPECT_EQ( dGridMap->lastI(), 8 );
            EXPECT_EQ( dGridMap->lastI(true), 9 );
            EXPECT_EQ( dGridMap->numI(), 3 );
         }
         if( myRank == 3 )
         {
            EXPECT_EQ( dGridMap->minI(), 9.0 );
            EXPECT_EQ( dGridMap->firstI(), 9 );
            EXPECT_EQ( dGridMap->firstI(true), 8 );
            EXPECT_EQ( dGridMap->lastI(), numI-1 );
            EXPECT_EQ( dGridMap->lastI(true), numI-1 );
            EXPECT_EQ( dGridMap->numI(), 2 );
         }
      }

      EXPECT_EQ( dGridMap->restoreData(false,false), true );
      EXPECT_EQ( dGridMap->restoreData(false,false), false ); // It won't do anything because the map is not retrieved anymore
   }
}


// Constructor #1 testing
TEST( DistributedGridMap, Constructor1 )
{
   MPIHelper::barrier();

   DataAccess::Interface::Grid * grid = new DataAccess::Interface::DistributedGrid( minI, minJ, maxI, maxJ, numI, numJ, numI, numJ );
   DataAccess::Interface::DistributedGridMap * dGridMap = 0;
   dGridMap = new DataAccess::Interface::DistributedGridMap(grid, depth, DataAccess::Interface::DefaultUndefinedMapValue);

   checkDistributedGridMap( dGridMap, 1 );
   EXPECT_EQ( dGridMap->getNumberOfDefinedValues(), 0 );

   delete dGridMap; dGridMap = 0;
   delete grid; grid = 0;
}


// Constructor #2 testing
TEST( DistributedGridMap, Constructor2 )
{
   MPIHelper::barrier();

   DataAccess::Interface::Grid * grid = new DataAccess::Interface::DistributedGrid( minI, minJ, maxI, maxJ, numI, numJ, numI, numJ );
   DataAccess::Interface::DistributedGridMap * dGridMap = 0;
   dGridMap = new DataAccess::Interface::DistributedGridMap(0, 0, grid, value, depth);

   checkDistributedGridMap( dGridMap, 2 );

   delete dGridMap; dGridMap = 0;
   delete grid; grid = 0;
}


// Constructor #3 testing
TEST( DistributedGridMap, Constructor3 )
{
   MPIHelper::barrier();

   DataAccess::Interface::Grid * grid = new DataAccess::Interface::DistributedGrid( minI, minJ, maxI, maxJ, numI, numJ, numI, numJ );
   DataAccess::Interface::DistributedGridMap * dGridMap = 0;

   float *** array = Array < float >::create3d (grid->numIGlobal(), grid->numJGlobal(), depth);
   for (int i = 0; i < grid->numIGlobal(); ++i)
   {
      for (int j = 0; j < grid->numJGlobal(); ++j)
      {
         for (unsigned int k = 0; k < depth; ++k)
         {
            {
               array[i][j][k] = value;
            }
         }
      }
   }
   dGridMap = new DataAccess::Interface::DistributedGridMap(0, 0, grid, DataAccess::Interface::DefaultUndefinedMapValue, depth, array);
   Array < float >::delete3d (array);

   checkDistributedGridMap( dGridMap, 3 );

   delete dGridMap; dGridMap = 0;
   delete grid; grid = 0;
}


// Constructor #4 testing
TEST( DistributedGridMap, Constructor4 )
{
   MPIHelper::barrier();

   DataAccess::Interface::Grid * grid = new DataAccess::Interface::DistributedGrid( minI, minJ, maxI, maxJ, numI, numJ, numI, numJ );
   DataAccess::Interface::DistributedGridMap * dGridMap = 0;

   DataAccess::Interface::DistributedGridMap * dGridMap1 = new DataAccess::Interface::DistributedGridMap(0, 0, grid, value, depth);
   DataAccess::Interface::DistributedGridMap * dGridMap2 = new DataAccess::Interface::DistributedGridMap(0, 0, grid, value, depth);
   dGridMap = new DataAccess::Interface::DistributedGridMap(0, 0, dGridMap1, dGridMap2, TestingOperator::minus);
   delete dGridMap1; dGridMap1 = 0;
   delete dGridMap2; dGridMap2 = 0;

   checkDistributedGridMap( dGridMap, 4 );

   delete dGridMap; dGridMap = 0;
   delete grid; grid = 0;
}


// Constructor #5 testing
TEST( DistributedGridMap, Constructor5 )
{
   MPIHelper::barrier();

   DataAccess::Interface::Grid * grid = new DataAccess::Interface::DistributedGrid( minI, minJ, maxI, maxJ, numI, numJ, numI, numJ );
   DataAccess::Interface::DistributedGridMap * dGridMap = 0;
   DataAccess::Interface::DistributedGridMap * dGridMap2;
   dGridMap2 = new DataAccess::Interface::DistributedGridMap(0, 0, grid, value, depth);
   dGridMap = new DataAccess::Interface::DistributedGridMap(0, 0, dGridMap2, TestingOperator::doubleVal);
   delete dGridMap2; dGridMap2 = 0;

   checkDistributedGridMap( dGridMap, 5 );

   delete dGridMap; dGridMap = 0;
   delete grid; grid = 0;
}


// Constructor #6 testing
TEST( DistributedGridMap, Constructor6 )
{
   MPIHelper::barrier();

   DataAccess::Interface::AdditionFunctor addFct;

   DataAccess::Interface::Grid * grid = new DataAccess::Interface::DistributedGrid( minI, minJ, maxI, maxJ, numI, numJ, numI, numJ );

   DataAccess::Interface::DistributedGridMap * dGridMap1 = new DataAccess::Interface::DistributedGridMap(0, 0, grid, value, depth);
   DataAccess::Interface::DistributedGridMap * dGridMap2 = new DataAccess::Interface::DistributedGridMap(0, 0, grid, value, depth);
   DataAccess::Interface::DistributedGridMap * dGridMap =  new DataAccess::Interface::DistributedGridMap(0, 0, dGridMap1, dGridMap2, addFct);

   delete dGridMap1; dGridMap1 = 0;
   delete dGridMap2; dGridMap2 = 0;

   checkDistributedGridMap( dGridMap, 6 );

   delete dGridMap; dGridMap = 0;
   delete grid; grid = 0;
}


// Constructor #7 testing
TEST( DistributedGridMap, Constructor7 )
{
   MPIHelper::barrier();

   DataAccess::Interface::SubtractConstant subFct( subValue );

   DataAccess::Interface::Grid * grid = new DataAccess::Interface::DistributedGrid( minI, minJ, maxI, maxJ, numI, numJ, numI, numJ );
   DataAccess::Interface::DistributedGridMap * dGridMap = 0;
   DataAccess::Interface::DistributedGridMap * dGridMap2;
   dGridMap2 = new DataAccess::Interface::DistributedGridMap(0, 0, grid, value, depth);
   dGridMap = new DataAccess::Interface::DistributedGridMap(0, 0, dGridMap2, subFct);
   delete dGridMap2; dGridMap2 = 0;

   checkDistributedGridMap( dGridMap, 7 );

   delete dGridMap; dGridMap = 0;
   delete grid; grid = 0;
}



TEST( DistributedGridMap, RetrieveAndRestore )
{
   const int mpiSize = MPIHelper::size();
   if( (mpiSize != 2) and (mpiSize != 4) )
   {
      EXPECT_EQ( "", "This test can be run only with 2 or 4 MPI processes" );
   }
   else
   {
      const int myRank = MPIHelper::rank();

      DataAccess::Interface::Grid * grid = new DataAccess::Interface::DistributedGrid( minI, minJ, maxI, maxJ, numI, numJ, numI, numJ );
      const unsigned int firstIgh = grid->firstI( true );
      const unsigned int lastIgh  = grid->lastI ( true );
      const unsigned int firstJgh = grid->firstJ( true );
      const unsigned int lastJgh  = grid->lastJ ( true );

      DataAccess::Interface::DistributedGridMap * dGridMap = new DataAccess::Interface::DistributedGridMap(0, 0, grid, 0, depth);

      dGridMap->retrieveGhostedData();
      // Setting each local map with the current MPI rank
      for( unsigned int i = firstIgh; i <= lastIgh; ++i )
      {
         for( unsigned int j = firstJgh; j <= lastJgh; ++j )
         {
            for (unsigned int k = 0; k < depth; ++k)
            {
               dGridMap->setValue( i, j, k, static_cast<double>(myRank+1) );
            }
         }
      }
      dGridMap->restoreData(true,true); // Restoring the data with ghost nodes, ghost values have to be summed

      dGridMap->retrieveData(true);
      if (mpiSize == 2)
      {
         // Expected data values after restore
         //
         //    |
         // 2  | 1 1 1 1 1 3 3 2 2 2 2
         //    |
         // 1  | 1 1 1 1 1 3 3 2 2 2 2
         //    |
         // 0  | 1 1 1 1 1 3 3 2 2 2 2
         //    |
         // -1 | 1 1 1 1 1 3 3 2 2 2 2
         //    |
         // -2 | 1 1 1 1 1 3 3 2 2 2 2
         //    *------------------------>
         //      0 1 2 3 4 5 6 7 8 9 10
         if( myRank == 0 )
         {
            EXPECT_EQ( dGridMap->getValue( (unsigned int)0,(unsigned int)0,(unsigned int)0), 1.0 );  // Non ghost node on processor #1
            EXPECT_EQ( dGridMap->getValue( (unsigned int)0,(unsigned int)0,depth-1), 1.0 );          // Non ghost node on processor #1
            EXPECT_EQ( dGridMap->getValue( dGridMap->lastI(true),(unsigned int)0,depth-1), 3.0 );    // Ghost node between processor #1 and #2 : 2+1
         }
         if( myRank == 1 )
         {
            EXPECT_EQ( dGridMap->getValue( dGridMap->lastI(),(unsigned int)0,(unsigned int)0), 2.0 ); // Non ghost node on processor #2
            EXPECT_EQ( dGridMap->getValue( dGridMap->lastI(),(unsigned int)0,depth-1), 2.0 );         // Non ghost node on processor #2
            EXPECT_EQ( dGridMap->getValue( dGridMap->firstI(true),(unsigned int)0,depth-1), 3.0 );    // Ghost node between processor #1 and #2 : 2+1
         }
         // Check min and max value
         double min, max;
         dGridMap->getMinMaxValue( min, max );
         EXPECT_EQ( min, 1.0 ); // Non ghost node on processor #1
         EXPECT_EQ( max, 3.0 ); // Ghost node between processor #1 and #2 : 2+1
         EXPECT_EQ(dGridMap->getAverageValue(), static_cast<double>(19) / 11 );
      }
      else if (mpiSize == 4)
      {
         // Expected data values after restore
         //
         //    |
         // 2  | 1 1 3 3 2 5 5 3 7 7 4
         //    |
         // 1  | 1 1 3 3 2 5 5 3 7 7 4
         //    |
         // 0  | 1 1 3 3 2 5 5 3 7 7 4
         //    |
         // -1 | 1 1 3 3 2 5 5 3 7 7 4
         //    |
         // -2 | 1 1 3 3 2 5 5 3 7 7 4
         //    *------------------------>
         //      0 1 2 3 4 5 6 7 8 9 10
         if( myRank == 0 )
         {
            EXPECT_EQ( dGridMap->getValue( (unsigned int)0,(unsigned int)0,(unsigned int)0), 1.0 );  // Non ghost node on processor #1
            EXPECT_EQ( dGridMap->getValue( (unsigned int)0,(unsigned int)0,depth-1), 1.0 );          // Non ghost node on processor #1
            EXPECT_EQ( dGridMap->getValue( dGridMap->lastI(true),(unsigned int)0,depth-1), 3.0 );    // Ghost node on processor #1 : 2+1
         }
         if( myRank == 1 )
         {
            EXPECT_EQ( dGridMap->getValue( dGridMap->firstI(false),(unsigned int)0,(unsigned int)0), 3.0 );   // Ghost node between processor #1 and #2: 2+1
            EXPECT_EQ( dGridMap->getValue( dGridMap->lastI(false)-1,(unsigned int)0,depth-1), 2.0 );          // Non ghost node on processor #2
            EXPECT_EQ( dGridMap->getValue( dGridMap->lastI(true),(unsigned int)0,depth-1), 5.0 );             // Ghost node on processor #2 and #3 : 3+2
         }
         if( myRank == 2 )
         {
            EXPECT_EQ( dGridMap->getValue( dGridMap->firstI(false),(unsigned int)0,(unsigned int)0), 5.0 );   // Ghost node between processor #2 and #3: 2+3
            EXPECT_EQ( dGridMap->getValue( dGridMap->lastI(false)-1,(unsigned int)0,depth-1), 3.0 );          // Non ghost node on processor #3
            EXPECT_EQ( dGridMap->getValue( dGridMap->lastI(true),(unsigned int)0,depth-1), 7.0 );             // Ghost node on processor #3 and #4 : 3+4
         }
         if( myRank == 3 )
         {
            EXPECT_EQ( dGridMap->getValue( dGridMap->lastI(),(unsigned int)0,(unsigned int)0), 4.0 ); // Non ghost node on processor #4
            EXPECT_EQ( dGridMap->getValue( dGridMap->lastI(),(unsigned int)0,depth-1), 4.0 );         // Non ghost node on processor #4
            EXPECT_EQ( dGridMap->getValue( dGridMap->firstI(true),(unsigned int)0,depth-1), 7.0 );    // Ghost node between processor #3 and #4 : 3+4
         }
         // Check min and max value
         double min, max;
         dGridMap->getMinMaxValue( min, max );
         EXPECT_EQ( min, 1.0 ); // Non ghost node on processor #1
         EXPECT_EQ( max, 7.0 ); // Ghost node between processor #1 and #2 : 2+1
         EXPECT_EQ(dGridMap->getAverageValue(), static_cast<double>(41) / 11 );
      }
      dGridMap->restoreData(false,false);

      delete dGridMap; dGridMap = 0;
      delete grid; grid = 0;
   }
}
#endif


#ifdef ASSERT_DEATH_CTR_1
// This test checks the assertion fail on unconforming grid X-dimension
TEST( DistributedGridMap, AssertDeath1 )
{
   MPIHelper::barrier();

   DataAccess::Interface::DistributedGrid grid( minI, minJ, maxI, maxJ, numI, numJ, numI, numJ );

   DataAccess::Interface::DistributedGridMap dGridMap1(0, 0, &grid, value, depth);

   // Death test for different I sizes
   DataAccess::Interface::DistributedGrid grid2( minI, minJ, maxI, maxJ, numI-5, numJ, numI-5, numJ );
   DataAccess::Interface::DistributedGridMap dGridMap2(0, 0, &grid2, value, depth);

   try { DataAccess::Interface::DistributedGridMap(0, 0, &dGridMap1, &dGridMap2, TestingOperator::minus); }
   catch( formattingexception::GeneralException & exc ) {
      EXPECT_EQ( exc.what(), std::string("DistributedGridMap: numI() != operand2->numI()") );
   }
}
#endif


#ifdef ASSERT_DEATH_CTR_2
// This test checks the assertion fail on unconforming grid Y-dimension
TEST( DistributedGridMap, AssertDeath2 )
{
   MPIHelper::barrier();

   DataAccess::Interface::DistributedGrid grid( minI, minJ, maxI, maxJ, numI, numJ, numI, numJ );

   DataAccess::Interface::DistributedGridMap dGridMap1(0, 0, &grid, value, depth);

   // Death test for different J sizes
   DataAccess::Interface::DistributedGrid grid2( minI, minJ, maxI, maxJ, numI, numJ-1, numI, numJ-1 );
   DataAccess::Interface::DistributedGridMap dGridMap2(0, 0, &grid2, value, depth);

   try { DataAccess::Interface::DistributedGridMap(0, 0, &dGridMap1, &dGridMap2, TestingOperator::minus); }
   catch( formattingexception::GeneralException & exc ) {
      EXPECT_EQ( exc.what(), std::string("DistributedGridMap: numJ() != operand2->numJ()") );
   }
}
#endif


#ifdef ASSERT_DEATH_CTR_3
// This test checks the assertion fail on unconforming grid Z-dimension
TEST( DistributedGridMap, AssertDeath3 )
{
   MPIHelper::barrier();

   DataAccess::Interface::DistributedGrid grid( minI, minJ, maxI, maxJ, numI, numJ, numI, numJ );

   DataAccess::Interface::DistributedGridMap dGridMap1(0, 0, &grid, value, depth);

   // Death test for different depths
   DataAccess::Interface::DistributedGrid grid2( minI, minJ, maxI, maxJ, numI, numJ, numI, numJ );
   DataAccess::Interface::DistributedGridMap dGridMap2(0, 0, &grid2, value, depth+7);

   try { DataAccess::Interface::DistributedGridMap(0, 0, &dGridMap1, &dGridMap2, TestingOperator::minus); }
   catch( formattingexception::GeneralException & exc ) {
      EXPECT_EQ( exc.what(), std::string("DistributedGridMap: getDepth() != operand2->getDepth()") );
   }
}
#endif


#ifdef ASSERT_DEATH_NOT_RETRIEVED
// This test checks the assertion fail on retrieved flag when calling
// DistributedGridMap::numI() on maps not yet restored
TEST( DistributedGridMap, AssertDeathNotRetrieved )
{
   MPIHelper::barrier();

   DataAccess::Interface::DistributedGrid grid( minI, minJ, maxI, maxJ, numI, numJ, numI, numJ );
   DataAccess::Interface::DistributedGridMap dGridMap(&grid, depth, DataAccess::Interface::DefaultUndefinedMapValue);

   try { dGridMap.numI(); }
   catch( formattingexception::GeneralException & exc ) {
      EXPECT_EQ( exc.what(), std::string("DistributedGridMap::numI() map not retrieved") );
   }
}
#endif


#ifdef ASSERT_THROW_ON_RESTORE
// This test checks the exception thrown on retrieve/restore workflow causing MPI communication hanging
TEST( DistributedGridMap, HangingCommunication )
{
   MPIHelper::barrier();
   if( MPIHelper::size() > 1 )
   {
      const int myRank = MPIHelper::rank();

      DataAccess::Interface::Grid * grid = new DataAccess::Interface::DistributedGrid( minI, minJ, maxI, maxJ, numI, numJ, numI, numJ );
      const unsigned int firstIgh = grid->firstI( true );
      const unsigned int lastIgh  = grid->lastI ( true );
      const unsigned int firstJgh = grid->firstJ( true );
      const unsigned int lastJgh  = grid->lastJ ( true );

      const unsigned int l_depth = 1;
      DataAccess::Interface::DistributedGridMap dGridMap(0, 0, grid, value, l_depth);

      dGridMap.retrieveData( true );
      for( unsigned int i = firstIgh; i <= lastIgh; ++i )
      {
         for( unsigned int j = firstJgh; j <= lastJgh; ++j )
         {
            if( myRank == 0 ) dGridMap.setValue( i, j, 1.0 );
         }
      }

      try { dGridMap.restoreData(true,true); }
      catch( formattingexception::GeneralException & exc ) {
         delete grid; grid = 0;
         EXPECT_EQ( exc.what(), std::string("DistributedGridMap::restoreData is causing hanging MPI communications.") );
      }
   }
}
#endif


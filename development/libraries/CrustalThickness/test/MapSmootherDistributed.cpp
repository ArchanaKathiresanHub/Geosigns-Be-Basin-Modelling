//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "../src/CTCMapSmoother.h"

// DataAccess libraries
#include "Grid.h"
#include "GlobalGrid.h"
#include "DistributedGrid.h"
#include "DistributedGridMap.h"

// utilities library
#include "../../utilities/src/array.h"

// 3rdparty
#include "petsc.h"

// std
#include <memory>

#include <gtest/gtest.h>

double minI = 0.0;
double minJ = 0.0;
double maxI = 4.0;
double maxJ = 4.0;
unsigned int numI = 10;
unsigned int numJ = 10;
unsigned int depth = 1;

using namespace CrustalThicknessInterface;

struct MPIHelper {
   MPIHelper() {
      PetscInitialize( 0, 0, 0, 0 );
   }

   ~MPIHelper() {
      MPI_Barrier( PETSC_COMM_WORLD );
      PetscFinalize();
   }

   static MPIHelper & instance() {
      static MPIHelper object;
      return object;
   }

   static void barrier() {
      instance();
      MPI_Barrier( PETSC_COMM_WORLD );
   }
};

TEST( MapSmoother, DistributedSmoothingConstantMap ) {
   // Define grids and derived properties
   MPIHelper::barrier();

   std::shared_ptr<DataAccess::Interface::Grid> grid(new DataAccess::Interface::DistributedGrid( minI, minJ, maxI, maxJ, numI, numJ, numI, numJ ));
   DataAccess::Interface::DistributedGridMap mapToSmooth( grid.get(), depth, 500 );

   // Simple average over the same value (500)
   MapSmoother smoother( 10 );
   bool status = smoother.averageSmoothing( &mapToSmooth );
   EXPECT_TRUE( status );
   mapToSmooth.retrieveGhostedData();
   for (unsigned int i = mapToSmooth.firstI( true ); i <= mapToSmooth.lastI( true ); ++i) {
      for (unsigned int j = mapToSmooth.firstJ( true ); j <= mapToSmooth.firstJ( true ); ++j) {
         EXPECT_EQ( 500, mapToSmooth.getValue( i, j ) );
      }
   }
   mapToSmooth.restoreData( true, true ); // Restoring the data with ghost nodes, ghost values have to be summed
}

TEST( MapSmoother, DistributedNoSmoothing ) {
   // Define grids and derived properties
   MPIHelper::barrier();

   std::shared_ptr<DataAccess::Interface::Grid> grid(new DataAccess::Interface::DistributedGrid( minI, minJ, maxI, maxJ, numI, numJ, numI, numJ ));
   // set map values to i index (value[i,j,k]=i)
   float *** array = Array < float >::create3d( grid->numIGlobal(), grid->numJGlobal(), depth );
   for (int i = 0; i < grid->numIGlobal(); ++i) {
      for (int j = 0; j < grid->numJGlobal(); ++j) {
         for (unsigned int k = 0; k < depth; ++k) {
            {
               array[i][j][k] = i;
            }
         }
      }
   }
   DataAccess::Interface::DistributedGridMap mapToSmooth( 0, 0, grid.get(), DataAccess::Interface::DefaultUndefinedMapValue, depth, array );

   // No smoothing (smoothing radius is zero)
   MapSmoother smoother( 0 );
   bool status = smoother.averageSmoothing( &mapToSmooth );
   EXPECT_TRUE( status );
   mapToSmooth.retrieveGhostedData();
   // Check that values are the same as before the smoothing
   for (unsigned int i = mapToSmooth.firstI( true ); i <= mapToSmooth.lastI( true ); ++i) {
      for (unsigned int j = mapToSmooth.firstJ( true ); j <= mapToSmooth.firstJ( true ); ++j) {
         for (unsigned int k = mapToSmooth.firstK(); k <= mapToSmooth.lastK(); ++k) {
            EXPECT_EQ( array[i][j][k], mapToSmooth.getValue( i, j ) );
         }
      }
   }
   mapToSmooth.restoreData( true, true ); // Restoring the data with ghost nodes, ghost values have to be summed

   Array < float >::delete3d( array );
}
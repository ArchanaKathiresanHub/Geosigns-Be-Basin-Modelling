//                                                                      
// Copyright (C) 2015 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 
#include "MatrixNonZeroCalculator.h"

// Access to STL library.
#include <iostream>
#include <sstream>
#include <algorithm>

// Access to PETSc library
#include "petsc.h"

// Access to fastcauldron application code.
#include "FastcauldronSimulator.h"
#include "NodalGrid.h"
#include "NodalVolumeGrid.h"
#include "PetscBlockVector.h"

void MatrixNonZeroCalculator::compute ( const ComputationalDomain& domain,
                                        IntegerArray&              localNumberOfNonZerosPerRow,
                                        IntegerArray&              ghostNumberOfNonZerosPerRow,
                                        const bool                 verbose ) const {

   const FastcauldronSimulator& fc = FastcauldronSimulator::getInstance ();
   const LocalIntegerArray3D& depthIndex = domain.getDepthIndices ();
   const LocalBooleanArray3D& nodeIsActive = domain.getActiveNodes ();
   const NodalVolumeGrid&     scalarNodeGrid = domain.getStratigraphicGrids ().getNodeGrid ();

   PetscBlockVector<double> dof;

   // Need get only dof number for this process.
   dof.setVector ( scalarNodeGrid, domain.getDofVector (), INSERT_VALUES );

   unsigned int activeAbove;
   unsigned int activeBelow;
   int localDofNumber;

   localNumberOfNonZerosPerRow.resize ( domain.getLocalNumberOfActiveNodes (), 0 );
   ghostNumberOfNonZerosPerRow.resize ( domain.getLocalNumberOfActiveNodes (), 0 );

   // Loop over the nodes that are local to the rank.
   for ( unsigned int i = depthIndex.first ( 0 ); i <= depthIndex.last ( 0 ); ++i ) {

      for ( unsigned int j = depthIndex.first ( 1 ); j <= depthIndex.last ( 1 ); ++j ) {

         if ( fc.nodeIsDefined ( i, j )) {

            for ( unsigned int k = depthIndex.first ( 2 ); k <= depthIndex.last ( 2 ); ++k ) {

               // For each active node compute the number of connecting nodes.
               // These connecting nodes may be local or non-local (i.e. ghost).
               if ( nodeIsActive ( i, j, k )) {
                  findColumnActivityRange ( depthIndex, i, j, k, activeAbove, activeBelow  );
                  localDofNumber = static_cast<int>(dof ( k, j, i )) - domain.getLocalStartDof ();
                  countNumberOfActiveDofs ( depthIndex, localDofNumber, i, j, k, activeAbove, activeBelow, localNumberOfNonZerosPerRow, ghostNumberOfNonZerosPerRow );
               }

            }

         }

      }

   }

   dof.restoreVector ( NO_UPDATE );

   if ( verbose ) {

      std::stringstream buffer;
      int rank = FastcauldronSimulator::getInstance ().getRank ();

      buffer << "local nonzeros for rank " << rank << std::endl;

      for ( size_t i = 0; i < localNumberOfNonZerosPerRow.size (); ++i ) {
         buffer <<  " LNZ " << rank << " " << i << " " << localNumberOfNonZerosPerRow [ i ] << std::endl;
      }

      buffer << "ghost nonzeros " << std::endl;

      for ( size_t i = 0; i < ghostNumberOfNonZerosPerRow.size (); ++i ) {
         buffer << " GNZ " << rank << " " << i << " " << ghostNumberOfNonZerosPerRow [ i ] << std::endl;
      }

      buffer << "--------------------------------" << std::endl << std::endl;

      PetscSynchronizedPrintf ( PETSC_COMM_WORLD, buffer.str ().c_str ());
   }

}

void MatrixNonZeroCalculator::findColumnActivityRange ( const LocalIntegerArray3D& depthIndex,
                                                        const unsigned int         localI,
                                                        const unsigned int         localJ,
                                                        const unsigned int         localK, 
                                                        unsigned int&              activeAbove,
                                                        unsigned int&              activeBelow ) const {

   activeAbove = localK;

   for ( unsigned int k = localK; k <= depthIndex.last ( 2 ); ++k ) {

      if ( depthIndex ( localI, localJ, k ) != depthIndex ( localI, localJ, localK )) {
         activeAbove = k;
         break;
      }

   }

   activeBelow = localK;

   for ( int k = localK; k >= static_cast<int>(depthIndex.first ( 2 )); --k ) { 

      if ( depthIndex ( localI, localJ, k ) != depthIndex ( localI, localJ, localK )) {
         activeBelow = k;
         break;
      }

   }

}

void MatrixNonZeroCalculator::countNumberOfActiveDofs ( const LocalIntegerArray3D& depthIndex,
                                                        const int                  localDofNumber,
                                                        const unsigned int         localI,
                                                        const unsigned int         localJ,
                                                        const unsigned int         localK, 
                                                        const unsigned int         activeAbove,
                                                        const unsigned int         activeBelow,
                                                        IntegerArray&              localNumberOfNonZerosPerRow,
                                                        IntegerArray&              ghostNumberOfNonZerosPerRow ) const {

   const FastcauldronSimulator& fc = FastcauldronSimulator::getInstance ();

   // Find the start and end i and j for the current dof.
   unsigned int startI = ( localI > depthIndex.first ( 0 ) ? localI - 1 : depthIndex.first ( 0 ));
   unsigned int endI   = ( localI < depthIndex.last  ( 0 ) ? localI + 1 : depthIndex.last  ( 0 ));
   unsigned int startJ = ( localJ > depthIndex.first ( 1 ) ? localJ - 1 : depthIndex.first ( 1 ));
   unsigned int endJ   = ( localJ < depthIndex.last  ( 1 ) ? localJ + 1 : depthIndex.last  ( 1 ));

   for ( unsigned int i = startI; i <= endI; ++i ) {

      for ( unsigned int j = startJ; j <= endJ; ++j ) {

         if ( fc.nodeIsDefined ( i, j )) {
            countNumberOfActiveDofsForColumn ( depthIndex, localDofNumber, i, j, localK, activeAbove, activeBelow, localNumberOfNonZerosPerRow, ghostNumberOfNonZerosPerRow );
         }

      }

   }

}

void MatrixNonZeroCalculator::countNumberOfActiveDofsForColumn ( const LocalIntegerArray3D& depthIndex,
                                                                 const int                  localDofNumber,
                                                                 const unsigned int         columnI,
                                                                 const unsigned int         columnJ,
                                                                 const unsigned int         localK, 
                                                                 const unsigned int         activeAbove,
                                                                 const unsigned int         activeBelow,
                                                                 IntegerArray&              localNumberOfNonZerosPerRow,
                                                                 IntegerArray&              ghostNumberOfNonZerosPerRow ) const {

   const NodalGrid& nodelGrid = FastcauldronSimulator::getInstance ().getNodalGrid ();

   // We need to include the start node, so the count is initialised at 1.
   int activeCount = 1;
   bool isLocalColumn = nodelGrid.isPartOfStencil ( columnI, columnJ, false );

   for ( unsigned int k = localK; k <= activeAbove; ++k ) {

      if ( depthIndex ( columnI, columnJ, k ) != depthIndex ( columnI, columnJ, localK )) {
         ++activeCount;
      }

   }

   for ( int k = localK; k >= static_cast<int>(activeBelow); --k ) { 

      if ( depthIndex ( columnI, columnJ, k ) != depthIndex ( columnI, columnJ, localK )) {
         ++activeCount;
      }

   }

   if ( isLocalColumn ) {
      localNumberOfNonZerosPerRow [ localDofNumber ] += activeCount;
   } else {
      ghostNumberOfNonZerosPerRow [ localDofNumber ] += activeCount;
   }

}

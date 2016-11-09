//                                                                      
// Copyright (C) 2015 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 
#ifndef FASTCAULDRON__MATRIX_NON_ZERO_CALCULATOR__H
#define FASTCAULDRON__MATRIX_NON_ZERO_CALCULATOR__H

// Access to fastcauldron application code.
#include "ComputationalDomain.h"
#include "ConstantsFastcauldron.h"

/// \brief Class used to compute the number of non-zeros in the sparse matrix.
///
/// The non-zeros are split into those that are on the local diagonal block and 
/// those that are in the off-diagonal block(s). 
/// The local diagonal block comes from the interaction of the dofs local to the process
/// with those that are local to the process.
/// The ghost off-diagonal block(s) comes from the interaction of the dofs local to the process
/// with those that are not local to the process, PETSc calls these ghost nodes.
///
/// In PETSc if the following matrix were to be distributed on 3 processes:
///
/// [ 1 2 0 0 3 0 0 4 0 0 5 ]
/// [ 0 6 7 8 0 0 9 1 0 1 0 ]
/// [ 0 0 2 0 3 0 4 0 0 0 5 ]
/// [ 0 6 0 7 0 0 0 8 0 0 0 ]
/// [ 0 9 0 1 2 0 3 0 0 0 4 ]
/// [ 5 0 6 0 0 0 7 0 0 8 9 ]
/// [ 0 1 0 2 0 3 4 0 0 0 0 ]
/// [ 0 5 0 6 7 0 0 0 0 8 0 ]
/// [ 9 0 1 0 0 2 3 0 4 0 5 ]
/// [ 0 0 6 0 0 7 0 8 0 0 9 ]
/// [ 1 0 2 3 0 0 0 0 4 0 0 ]
///
/// then we might get:
///
/// [ 1 2 0 0 | 3 0 0 | 4 0 0 5 ]  
/// [ 0 6 7 8 | 0 0 9 | 1 0 1 0 ]  Process 0
/// [ 0 0 2 0 | 3 0 4 | 0 0 0 5 ]  
/// [ 0 6 0 7 | 0 0 0 | 8 0 0 0 ]  
/// ----------+-------+----------  
/// [ 0 9 0 1 | 2 0 3 | 0 0 0 4 ]  
/// [ 5 0 6 0 | 0 0 7 | 0 0 8 9 ]  Process 1
/// [ 0 1 0 2 | 0 3 4 | 0 0 0 0 ]  
/// ----------+-------+----------  
/// [ 0 5 0 6 | 7 0 0 | 0 0 8 0 ]  
/// [ 9 0 1 0 | 0 2 3 | 0 4 0 5 ]  
/// [ 0 0 6 0 | 0 7 0 | 8 0 0 9 ]  Process 2
/// [ 1 0 2 3 | 0 0 0 | 0 4 0 0 ]  
///
/// This can be considered as a block matrix like:
///
/// [A][B][C]
/// [D][E][F]
/// [G][H][J]
///
/// Block A, E and J are the diagonal blocks, the remaining blocks are off-diagonal.
/// PETSc stores the matrix so that values for blocks A, B and C are all stored on
/// process 1. The values for D, E and F are on process 2, and values for G, H and J
/// are on process 3.
///
/// So the number of non-zeros for this matrix are:
///
/// On process 0
///   localNonZerosPerRow = {2, 3, 1, 2}
///   corresponds to block [A]
///   ghostNonZerosPerRow = {3, 3, 3, 1}
///   corresponds to block [B,C]
///
/// On process 1
///   localNonZerosPerRow = {2, 1, 2}
///   corresponds to block [E]
///   ghostNonZerosPerRow = {3, 4, 2}
///   corresponds to block [D,F]
///
/// On process 2
///   localNonZerosPerRow = {1, 2, 2, 1}
///   corresponds to block [J]
///   ghostNonZerosPerRow = {3, 4, 2, 3}
///   corresponds to block [G,H]
///
class MatrixNonZeroCalculator {

public :


   /// \brief Compute the number of non-zeros per row for the local diagonal block and the off-diagonal block.
   ///
   /// \param [in]  domain                      The computational domain for which the number of non-zeros is sought.
   /// \param [out] localNumberOfNonZerosPerRow On exit this will contain the number of number of non-zeros
   ///                                          per row of the diagonal block part of the matrix
   /// \param [out] ghostNumberOfNonZerosPerRow On exit this will contain the number of number of non-zeros
   ///                                          per row of the off-diagonal block part of the matrix.
   /// \param [in]  verbose                     Indicate if the number of non-zeros should be output.
   void compute ( const ComputationalDomain& domain,
                  IntegerArray&              localNumberOfNonZerosPerRow,
                  IntegerArray&              ghostNumberOfNonZerosPerRow,
                  const bool                 verbose = false ) const;


private :


   /// \brief Find the first active nodes above and below the current node.
   void findColumnActivityRange ( const LocalIntegerArray3D& depthIndex,
                                  const unsigned int         localI,
                                  const unsigned int         localJ,
                                  const unsigned int         localK, 
                                  unsigned int&              activeAbove,
                                  unsigned int&              activeBelow ) const;

   /// \brief Count the total number of dofs that are in the support of the basis function at (i,j,k).
   void countNumberOfActiveDofs ( const LocalIntegerArray3D& depthIndex,
                                  const int                  localDofNumber,
                                  const unsigned int         localI,
                                  const unsigned int         localJ,
                                  const unsigned int         localK, 
                                  const unsigned int         activeAbove,
                                  const unsigned int         activeBelow,
                                  IntegerArray&              localNumberOfNonZerosPerRow,
                                  IntegerArray&              ghostNumberOfNonZerosPerRow ) const;

   /// \brief Count the number of dofs that are contained in a section of the column.
   ///
   /// The column section is located at some i,j position and is bounded by the activeAbove and activeBelow indices.
   void countNumberOfActiveDofsForColumn ( const LocalIntegerArray3D& depthIndex,
                                           const int                  localDofNumber,
                                           const unsigned int         columnI,
                                           const unsigned int         columnJ,
                                           const unsigned int         localK, 
                                           const unsigned int         activeAbove,
                                           const unsigned int         activeBelow,
                                           IntegerArray&              localNumberOfNonZerosPerRow,
                                           IntegerArray&              ghostNumberOfNonZerosPerRow ) const;



};

#endif // FASTCAULDRON__MATRIX_NON_ZERO_CALCULATOR__H

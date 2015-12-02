//                                                                      
// Copyright (C) 2015 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 
#include "PetscObjectAllocator.h"

// STL library.
#include <vector>

// Access to fastcauldron application code.
#include "MatrixNonZeroCalculator.h"

Mat PetscObjectAllocator::allocateMatrix ( const ComputationalDomain& domain, 
                                           const bool                 verbose ) {


  MatrixNonZeroCalculator msp;
  std::vector<int> localNzs;
  std::vector<int> ghostNzs;
  Mat matrix;

  msp.compute ( domain, localNzs, ghostNzs, verbose );

  MatCreate ( PETSC_COMM_WORLD, &matrix );
  MatSetSizes ( matrix,
                domain.getLocalNumberOfActiveNodes (),
                domain.getLocalNumberOfActiveNodes (),
                PETSC_DECIDE, PETSC_DECIDE );
  MatSetFromOptions ( matrix );
  MatSeqAIJSetPreallocation ( matrix, 0, localNzs.data ());
  MatMPIAIJSetPreallocation ( matrix, 0, localNzs.data (), 0, ghostNzs.data ());
  MatSetLocalToGlobalMapping ( matrix,
                               domain.getLocalToGlobalMapping (),
                               domain.getLocalToGlobalMapping ());

  return matrix;

}

Vec PetscObjectAllocator::allocateVector ( const ComputationalDomain& domain ) {


  Vec vector;

  VecCreate ( PETSC_COMM_WORLD, &vector );
  VecSetSizes ( vector,
                domain.getLocalNumberOfActiveNodes (),
                domain.getGlobalNumberOfActiveNodes ());
  VecSetFromOptions ( vector );
  VecSetLocalToGlobalMapping ( vector, domain.getLocalToGlobalMapping ());

  return vector;
}

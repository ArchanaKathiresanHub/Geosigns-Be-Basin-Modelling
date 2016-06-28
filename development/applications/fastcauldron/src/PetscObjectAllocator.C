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
   int mpiSize = 0;
   MPI_Comm_size( PETSC_COMM_WORLD, &mpiSize );
   
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

  // To avoid memory leaks within PETSc
  // http://lists.mcs.anl.gov/pipermail/petsc-users/2014-September/022866.html
  if( mpiSize == 1 )
  {
     MatSetType( matrix, MATSEQAIJ );
     MatSeqAIJSetPreallocation ( matrix, 0, localNzs.data ());
  }
  else
  {
     MatSetType( matrix, MATMPIAIJ );
     MatMPIAIJSetPreallocation ( matrix, 0, localNzs.data (), 0, ghostNzs.data ());
  }
  MatSetLocalToGlobalMapping ( matrix,
                               domain.getLocalToGlobalMapping (),
                               domain.getLocalToGlobalMapping ());

  return matrix;

}

Vec PetscObjectAllocator::allocateVector ( const ComputationalDomain& domain ) {
 
  int mpiSize = 0;
  MPI_Comm_size( PETSC_COMM_WORLD, &mpiSize );

  Vec vector;

  VecCreate ( PETSC_COMM_WORLD, &vector );
  VecSetSizes ( vector,
                domain.getLocalNumberOfActiveNodes (),
                domain.getGlobalNumberOfActiveNodes ());
  VecSetFromOptions ( vector );

  // To avoid memory leaks within PETSc
  // http://lists.mcs.anl.gov/pipermail/petsc-users/2014-September/022866.html
  if( mpiSize == 1 )
  {
     VecSetType( vector, VECSEQ );
  }
  else
  {
     VecSetType( vector, VECMPI );
  }
  VecSetLocalToGlobalMapping ( vector, domain.getLocalToGlobalMapping ());

  return vector;
}

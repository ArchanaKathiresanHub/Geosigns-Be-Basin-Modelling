#ifndef HPC_MKLCSRMATRIX_H
#define HPC_MKLCSRMATRIX_H

namespace hpc
{
  class PetscMatrix;
  class PetscVector;
  
  void MklCsrMultiply( double alpha, const PetscMatrix & A, const PetscVector & x, double beta, PetscVector & y );



}

#endif

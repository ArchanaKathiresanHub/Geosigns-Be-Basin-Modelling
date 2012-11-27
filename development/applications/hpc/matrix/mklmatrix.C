#include "mklmatrix.h"
#include "petscmatrix.h"
#include "petscvector.h"

#include <cassert>

#define MKL_INT hpc::PetscMatrix::SizeType
#include <mkl.h>
#include <mkl_spblas.h>

namespace hpc
{

  void MklCsrMultiply( double alpha, const PetscMatrix & A, const PetscVector & x, double beta, PetscVector & y)
  {
    assert( x.rows() == A.columns() );
    assert( y.rows() == A.rows() );

    char transa = 'T';
    char matdescra[6] = "GLNC";
    PetscMatrix::SizeType rows = A.rows();
    PetscMatrix::SizeType columns = A.columns();
    mkl_dcsrmv( &transa,
	& rows,
	& columns,
	&alpha,
	matdescra,
	const_cast<double *>(A.nonZeros()),
	const_cast<uint32_t *>(A.columnIndices()),
	const_cast<uint32_t *>(A.rowOffsets()),
	const_cast<uint32_t *>(A.rowOffsets()) + 1,
	const_cast<double *>(&x[0]),
	&beta,
	&y[0]
    );
  }




}

#include "ilu.h"
#include "cudaexception.h"
#include "cudasparse.h"

#include <cassert>
#include <mkl_rci.h>
#include <mkl_spblas.h>

namespace hpc
{


ILU< CudaCSRMatrix, CudaVector>
  :: ILU(const PetscMatrix & matrix)
  : m_matrix( ILU<PetscMatrix, PetscVector>(matrix).m_matrix )
  , m_tmpVector(PetscVector())
  , m_lMatDescr( CudaMatrixDescription().isLowerTriangular().hasUnitDiagonal().hasIndexBaseZero())
  , m_uMatDescr( CudaMatrixDescription().isUpperTriangular().hasNonUnitDiagonal().hasIndexBaseZero())
  , m_lSolveInfo()
  , m_uSolveInfo()
{
  // Analyse lower triangular matrix 'L'
  CuSparseException::check(
      cusparseDcsrsv_analysis(
       	CudaSparse::instance().handle(),
	CUSPARSE_OPERATION_NON_TRANSPOSE,
	m_matrix.rows(),
	m_matrix.m_nonZeros.size(),
	m_lMatDescr.matdescr(),
	m_matrix.m_nonZeros.devicePointer(),
	m_matrix.m_rowOffsets.devicePointer(),
	m_matrix.m_columnIndices.devicePointer(),
	m_lSolveInfo.handle()
      )
    );

  // Analyse upper triangular matrix 'U'
  CuSparseException::check(
      cusparseDcsrsv_analysis(
       	CudaSparse::instance().handle(),
	CUSPARSE_OPERATION_NON_TRANSPOSE,
	m_matrix.rows(),
	m_matrix.m_nonZeros.size(),
	m_uMatDescr.matdescr(),
	m_matrix.m_nonZeros.devicePointer(),
	m_matrix.m_rowOffsets.devicePointer(),
	m_matrix.m_columnIndices.devicePointer(),
	m_uSolveInfo.handle()
      )
    );
}

ILU< CudaHYBMatrix, CudaVector>
  :: ILU(const PetscMatrix & matrix)
  : m_matrix( ILU<PetscMatrix, PetscVector>(matrix).m_matrix )
  , m_tmpVector(PetscVector())
  , m_lMatDescr( CudaMatrixDescription().isLowerTriangular().hasUnitDiagonal().hasIndexBaseZero())
  , m_uMatDescr( CudaMatrixDescription().isUpperTriangular().hasNonUnitDiagonal().hasIndexBaseZero())
  , m_lSolveInfo()
  , m_uSolveInfo()
{
  // Analyse lower triangular matrix L
  CuSparseException::check(
      cusparseDhybsv_analysis(
       	CudaSparse::instance().handle(),
	CUSPARSE_OPERATION_NON_TRANSPOSE,
	m_lMatDescr.matdescr(),
	m_matrix.m_hybmatrix,
	m_lSolveInfo.handle()
      )
    );

  // Analyse lower triangular matrix L
  CuSparseException::check(
      cusparseDhybsv_analysis(
       	CudaSparse::instance().handle(),
	CUSPARSE_OPERATION_NON_TRANSPOSE,
	m_uMatDescr.matdescr(),
	m_matrix.m_hybmatrix,
	m_uSolveInfo.handle()
      )
    );

}

ILU<PetscMatrix, PetscVector>
  :: ILU(const PetscMatrix & matrix)
  : m_matrix(matrix)
  , m_tmpVector()
{
  assert( m_matrix.columns() == m_matrix.rows() && "The matrix must be square");

  std::vector<SizeType> ipar(128, 0);
  ipar[31] = 0; // "stop when a diagonal element is '0'"

  std::vector<ValueType> dpar(128, 0.0);

  // convert the rowOffsets to one-base index
  std::vector< SizeType > rowOffsets( m_matrix.rowOffsets(),
      m_matrix.rowOffsets() + m_matrix.rows() + 1
      );

  for (unsigned i = 0; i < rowOffsets.size(); ++i)
    rowOffsets[i]++;

  // convert the columnIndices to one-base index
  std::vector< SizeType > columnIndices( m_matrix.columnIndices(),
      m_matrix.columnIndices() + m_matrix.numberNonZeros()
      );

  for (unsigned i = 0; i < columnIndices.size(); ++i)
    columnIndices[i]++;
 
  // Call MKL's ILU
  int error = 0;
  SizeType dim = m_matrix.rows();
  dcsrilu0( 
      & dim,
      const_cast<double *>(matrix.nonZeros()),
      &rowOffsets[0],
      &columnIndices[0],
      const_cast<double *>(m_matrix.nonZeros()),
      &ipar[0],
      &dpar[0],
      & error
      );

  switch(error)
  {
    case 0: // normal completion
      break;

    case -101: 
      throw ILUException() << "At least one diagonal element is omitted from the matrix";

    case -102:
      throw ILUException() << "Matrix contains a diagonal element that is zero.";

    case -103:
      throw ILUException() << "Matrix contains a diagonal element that is too small.";

    case -104:
      throw ILUException() << "Insufficient memory for the work array.";

    case -105:
      throw ILUException() << "Empty input matrix";

    case -106:
      throw ILUException() << "Column indices are not in ascending order.";

    default:
      throw ILUException() << "Unknown dcsrilu0 error '" << error << "'.";
  }
}

void
ILU<PetscMatrix, PetscVector>
  :: apply( const PetscVector & x, PetscVector & y)
{
  assert( m_matrix.rows() == x.rows() );

  if (m_tmpVector.rows() != x.rows() )
    m_tmpVector = x;

  if (y.rows() != x.rows())
    y = x;

  char transL = 'N', transU = 'N';
  char matdescrL[6] = "TLUC"; // lower triangle, unit diaginal, zero based indexing
  char matdescrU[6] = "TUNC"; // upper triangle, non-unit diagonal, zero based indexing
  SizeType dim = m_matrix.rows();
  ValueType alpha = 1.0;

  mkl_dcsrsv( 
      & transL,
      & dim,
      & alpha,
      matdescrL,
      const_cast<double *>(m_matrix.nonZeros()),
      const_cast<int *>(m_matrix.columnIndices()),
      const_cast<int *>(m_matrix.rowOffsets()),
      const_cast<int *>(m_matrix.rowOffsets() + 1),
      const_cast<double *>(&x[0]),
      &m_tmpVector[0]
      );

  mkl_dcsrsv( 
      & transU,
      & dim,
      & alpha,
      matdescrU,
      const_cast<double *>(m_matrix.nonZeros()),
      const_cast<int *>(m_matrix.columnIndices()),
      const_cast<int *>(m_matrix.rowOffsets()),
      const_cast<int *>(m_matrix.rowOffsets() + 1),
      const_cast<double *>(&m_tmpVector[0]),
      &y[0]
      );
}

void
ILU<CudaCSRMatrix, CudaVector>
  :: apply( const CudaVector & x, CudaVector & y)
{
  assert( m_matrix.rows() == x.rows() );

  if (m_tmpVector.rows() != x.rows() )
    m_tmpVector = x;

  if (y.rows() != x.rows())
    y = x;

  SizeType dim = m_matrix.rows();
  ValueType alpha = 1.0;

  CuSparseException :: check( cusparseDcsrsv_solve(
      CudaSparse::instance().handle(),
      CUSPARSE_OPERATION_NON_TRANSPOSE,
      dim,
      & alpha,
      m_lMatDescr.matdescr(),
      m_matrix.m_nonZeros.devicePointer(),
      m_matrix.m_rowOffsets.devicePointer(),
      m_matrix.m_columnIndices.devicePointer(),
      m_lSolveInfo.handle(),
      x.devicePointer(),
      m_tmpVector.devicePointer()
      ) 
    );

  CuSparseException :: check( cusparseDcsrsv_solve(
      CudaSparse::instance().handle(),
      CUSPARSE_OPERATION_NON_TRANSPOSE,
      dim,
      & alpha,
      m_uMatDescr.matdescr(),
      m_matrix.m_nonZeros.devicePointer(),
      m_matrix.m_rowOffsets.devicePointer(),
      m_matrix.m_columnIndices.devicePointer(),
      m_uSolveInfo.handle(),
      m_tmpVector.devicePointer(),
      y.devicePointer()
      ) 
    );
}

void
ILU<CudaHYBMatrix, CudaVector>
  :: apply( const CudaVector & x, CudaVector & y)
{
  assert( m_matrix.rows() == x.rows() );

  if (m_tmpVector.rows() != x.rows() )
    m_tmpVector = x;

  if (y.rows() != x.rows())
    y = x;

  ValueType alpha = 1.0;

  CuSparseException :: check( cusparseDhybsv_solve(
      CudaSparse::instance().handle(),
      CUSPARSE_OPERATION_NON_TRANSPOSE,
      & alpha,
      m_lMatDescr.matdescr(),
      m_matrix.m_hybmatrix,
      m_lSolveInfo.handle(),
      x.devicePointer(),
      m_tmpVector.devicePointer()
      ) 
    );

  CuSparseException :: check( cusparseDhybsv_solve(
      CudaSparse::instance().handle(),
      CUSPARSE_OPERATION_NON_TRANSPOSE,
      & alpha,
      m_uMatDescr.matdescr(),
      m_matrix.m_hybmatrix,
      m_uSolveInfo.handle(),
      m_tmpVector.devicePointer(),
      y.devicePointer()
      ) 
    );
}



}

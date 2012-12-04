#include "cudamatdescr.h"
#include "cudaexception.h"

namespace hpc
{

CudaMatrixDescription
  :: CudaMatrixDescription()
  : m_matdescr()
{
  CuSparseException::check( cusparseCreateMatDescr(&m_matdescr) );
}

CudaMatrixDescription
  :: CudaMatrixDescription( const CudaMatrixDescription & other)
  : m_matdescr()
{
  CuSparseException::check( cusparseCreateMatDescr(&m_matdescr) );
  CuSparseException :: check( cusparseSetMatDiagType( m_matdescr, cusparseGetMatDiagType(other.m_matdescr)) );
  CuSparseException :: check( cusparseSetMatFillMode( m_matdescr, cusparseGetMatFillMode(other.m_matdescr)) );
  CuSparseException :: check( cusparseSetMatIndexBase( m_matdescr, cusparseGetMatIndexBase(other.m_matdescr)) );
  CuSparseException :: check( cusparseSetMatType( m_matdescr, cusparseGetMatType(other.m_matdescr)) );
}

CudaMatrixDescription
  :: ~CudaMatrixDescription()
{
  // destroy the m_matdescr. Ignore any errors, because it is evil to throw
  // any exception
  // TODO: log any errors
  cusparseDestroyMatDescr( m_matdescr);
}

CudaMatrixDescription &
CudaMatrixDescription
  :: isGeneral()
{
  CuSparseException :: check( cusparseSetMatType( m_matdescr, CUSPARSE_MATRIX_TYPE_GENERAL) );
  return *this;
}

CudaMatrixDescription &
CudaMatrixDescription
  :: isSymmetric()
{
  CuSparseException :: check( cusparseSetMatType( m_matdescr, CUSPARSE_MATRIX_TYPE_SYMMETRIC) );
  return *this;
}

CudaMatrixDescription &
CudaMatrixDescription
  :: isHermitian()
{
  CuSparseException :: check( cusparseSetMatType( m_matdescr, CUSPARSE_MATRIX_TYPE_HERMITIAN) );
  return *this;
}

CudaMatrixDescription &
CudaMatrixDescription
  :: isUpperTriangular()
{
  CuSparseException :: check( cusparseSetMatType( m_matdescr, CUSPARSE_MATRIX_TYPE_TRIANGULAR) );
  CuSparseException :: check( cusparseSetMatFillMode( m_matdescr, CUSPARSE_FILL_MODE_UPPER) );
  return *this;
}

CudaMatrixDescription &
CudaMatrixDescription
  :: isLowerTriangular()
{
  CuSparseException :: check( cusparseSetMatType( m_matdescr, CUSPARSE_MATRIX_TYPE_TRIANGULAR) );
  CuSparseException :: check( cusparseSetMatFillMode( m_matdescr, CUSPARSE_FILL_MODE_LOWER) );
  return *this;
}

CudaMatrixDescription &
CudaMatrixDescription
  :: hasUnitDiagonal()
{
  CuSparseException :: check( cusparseSetMatDiagType( m_matdescr, CUSPARSE_DIAG_TYPE_UNIT) );
  return *this;
}

CudaMatrixDescription &
CudaMatrixDescription
  :: hasNonUnitDiagonal()
{
  CuSparseException :: check( cusparseSetMatDiagType( m_matdescr, CUSPARSE_DIAG_TYPE_NON_UNIT) );
  return *this;
}

CudaMatrixDescription &
CudaMatrixDescription
  :: hasIndexBaseZero()
{
  CuSparseException :: check( cusparseSetMatIndexBase( m_matdescr, CUSPARSE_INDEX_BASE_ZERO) );
  return *this;
}

CudaMatrixDescription &
CudaMatrixDescription
  :: hasIndexBaseOne()
{
  CuSparseException :: check( cusparseSetMatIndexBase( m_matdescr, CUSPARSE_INDEX_BASE_ONE) );
  return *this;
}

}

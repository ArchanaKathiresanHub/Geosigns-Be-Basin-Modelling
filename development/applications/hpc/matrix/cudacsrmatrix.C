#include "cudacsrmatrix.h"
#include "petscmatrix.h"
#include "cudaexception.h"
#include "cudasparse.h"
#include "cudavector.h"

#include <cusparse_v2.h>

namespace hpc
{


CudaCSRMatrix
  :: CudaCSRMatrix( const PetscMatrix & matrix)
  : m_rows(matrix.rows()), m_columns(matrix.columns())
  , m_matdescr( CudaMatrixDescription().isGeneral().hasIndexBaseZero() )
  , m_rowOffsets( matrix.rowOffsets(), matrix.rowOffsets() + matrix.rows() + 1)
  , m_columnIndices( matrix.columnIndices(), matrix.columnIndices() + matrix.numberNonZeros())
  , m_nonZeros( matrix.nonZeros(), matrix.nonZeros() + matrix.numberNonZeros())
{
}										 

void 
CudaCSRMatrix
  :: multiply( ValueType alpha, const CudaVector & x, ValueType beta, CudaVector & y) const
{
  cusparseOperation_t transa = CUSPARSE_OPERATION_NON_TRANSPOSE;

  CuSparseException::check(
    cusparseDcsrmv( CudaSparse::instance().handle() 
      , transa
      , rows(), columns(), m_nonZeros.size()
      , & alpha, m_matdescr.matdescr()
      , m_nonZeros.devicePointer()
      , m_rowOffsets.devicePointer()
      , m_columnIndices.devicePointer()
      , x.devicePointer()
      , & beta, y.devicePointer()
     )
   );
}

CudaCSRMatrix::ValueType
CudaCSRMatrix::Row
  :: operator[]( SizeType column) const
{
  const SizeType nnz = m_matrix->m_columnIndices.size();
  const std::vector<SizeType> columns = m_matrix->m_columnIndices.get( m_rowBegin, m_rowEnd);
  const SizeType index 
    = std::distance( columns.begin(), std::lower_bound( columns.begin(),columns.end(), column));

  if ( index < columns.size() && columns[index] == column)
    return m_matrix->m_nonZeros[m_rowBegin + index];
  else
    return 0.0;
}

}

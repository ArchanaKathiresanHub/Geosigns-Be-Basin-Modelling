#include "cudahybmatrix.h"
#include "cudacsrmatrix.h"
#include "cudavector.h"
#include "cudaexception.h"
#include "cudasparse.h"

namespace hpc
{

CudaHYBMatrix
  :: CudaHYBMatrix( const CudaCSRMatrix & matrix, SizeType ellWidth)
  : m_rows( matrix.rows() )
  , m_columns(matrix.columns())
  , m_matdescr( matrix.m_matdescr )
  , m_hybmatrix()
{
  CuSparseException::check( cusparseCreateHybMat( &m_hybmatrix ) );  

  CuSparseException::check( cusparseDcsr2hyb( CudaSparse::instance().handle()
    , m_rows, m_columns, m_matdescr.matdescr()
    , matrix.m_nonZeros.devicePointer()
    , matrix.m_rowOffsets.devicePointer()
    , matrix.m_columnIndices.devicePointer()
    , m_hybmatrix
    , ellWidth
    , ellWidth == -1 ? CUSPARSE_HYB_PARTITION_AUTO : CUSPARSE_HYB_PARTITION_USER
    )
  );
}

CudaHYBMatrix
  :: ~CudaHYBMatrix()
{
  // TODO: log errors
  cusparseDestroyHybMat( m_hybmatrix );
}

void
CudaHYBMatrix
  :: multiply( ValueType alpha, const CudaVector & x, ValueType beta, CudaVector & y ) const
{
  cusparseOperation_t transa = CUSPARSE_OPERATION_NON_TRANSPOSE;
  CuSparseException :: check( cusparseDhybmv( CudaSparse::instance().handle()
      , transa
      , &alpha
      , m_matdescr.matdescr()
      , m_hybmatrix
      , x.devicePointer()
      , & beta, y.devicePointer()
      )
  );
}


}

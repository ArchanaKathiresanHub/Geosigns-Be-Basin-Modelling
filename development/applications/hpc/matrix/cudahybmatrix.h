#ifndef HPC_CUDAHYBMATRIX_H
#define HPC_CUDAHYBMATRIX_H

#include "cudamatdescr.h"

namespace hpc
{

class CudaVector;
class CudaCSRMatrix;

class CudaHYBMatrix
{
public:
  typedef int SizeType;
  typedef double ValueType;

  CudaHYBMatrix( const CudaCSRMatrix & matrix, SizeType ellWidth = -1);
  ~CudaHYBMatrix();

  SizeType rows() const
  { return m_rows; }

  SizeType columns() const
  { return m_columns; }

  void multiply( ValueType alpha, const CudaVector & x, ValueType beta, CudaVector & y) const;


private:
  CudaHYBMatrix(const CudaHYBMatrix & other); // copying prohibited

  SizeType m_rows, m_columns;
  CudaMatrixDescription m_matdescr;
  cusparseHybMat_t m_hybmatrix;
};



}

#endif

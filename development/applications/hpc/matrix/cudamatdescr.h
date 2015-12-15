#ifndef HPC_CUDAMATDESCR_H
#define HPC_CUDAMATDESCR_H

#include <cusparse_v2.h>

namespace hpc
{


class CudaMatrixDescription
{
public:
  CudaMatrixDescription();
  CudaMatrixDescription( const CudaMatrixDescription & ); 
  ~CudaMatrixDescription();
  
  CudaMatrixDescription & isGeneral();
  CudaMatrixDescription & isSymmetric();
  CudaMatrixDescription & isHermitian();
  CudaMatrixDescription & isUpperTriangular();
  CudaMatrixDescription & isLowerTriangular();

  CudaMatrixDescription & hasUnitDiagonal();
  CudaMatrixDescription & hasNonUnitDiagonal();

  CudaMatrixDescription & hasIndexBaseZero();
  CudaMatrixDescription & hasIndexBaseOne();

  cusparseMatDescr_t matdescr() const
  { return m_matdescr; }

private:
  CudaMatrixDescription & operator=(const CudaMatrixDescription & other); // assignment prohibited

  cusparseMatDescr_t m_matdescr;
};

}

#endif

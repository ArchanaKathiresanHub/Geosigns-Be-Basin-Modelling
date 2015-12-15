#ifndef HPC_CUDAEXCEPTION_H
#define HPC_CUDAEXCETPION_H

#include <cusparse_v2.h>

#include "generalexception.h"

namespace hpc
{

struct CudaException : BaseException< CudaException > 
{
  CudaException();
  static void check();
  static void check(cudaError_t e);
};

struct CuSparseException : BaseException< CuSparseException >
{
  CuSparseException();
  static void check(cusparseStatus_t e);
};

}

#endif

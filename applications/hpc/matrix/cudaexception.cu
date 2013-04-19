#include "cudaexception.h"


namespace hpc
{

CudaException
  :: CudaException()
{
  (*this) << "CUDA Error";
}
 
void
CudaException
  :: check()
{
  cudaError_t e = cudaGetLastError();
  if (e != cudaSuccess)
    throw CudaException() << ": " << cudaGetErrorString(e) ;
}

void
CudaException
  :: check(cudaError_t e)
{
  if (e != cudaSuccess)
    throw CudaException() << ": " << cudaGetErrorString(e);
}

CuSparseException
  :: CuSparseException()
{
  (*this) << "CUSPARSE Library Error";
}

void
CuSparseException
  :: check(cusparseStatus_t e)
{
  switch(e)
  {
    case CUSPARSE_STATUS_SUCCESS:
      break;
    
    case CUSPARSE_STATUS_NOT_INITIALIZED:
      throw CuSparseException() << ": Library wasn't initialized.";
    case CUSPARSE_STATUS_ALLOC_FAILED:
      throw CuSparseException() << ": Could not allocated resource.";
    case CUSPARSE_STATUS_INVALID_VALUE:
      throw CuSparseException() << ": Invalid parameter";
    case CUSPARSE_STATUS_ARCH_MISMATCH:
      throw CuSparseException() << ": Device architecture mismatch";
    case CUSPARSE_STATUS_MAPPING_ERROR:
      throw CuSparseException() << ": Memory error";
    case CUSPARSE_STATUS_EXECUTION_FAILED:
      throw CuSparseException() << ": GPU program failed to execute";
    case CUSPARSE_STATUS_INTERNAL_ERROR:
      throw CuSparseException() << ": Internal error";
    case CUSPARSE_STATUS_MATRIX_TYPE_NOT_SUPPORTED:
      throw CuSparseException() << ": Matrix type is not supported.";

    default:
      throw CuSparseException() << ": Unknown error";
  }
}

}

#include "cudasparse.h"
#include "cudaexception.h"
#include <iostream>

#include <cusparse_v2.h>

#if HAVE_MPI
  #include <mpi.h>
#endif

namespace hpc
{

CudaSparse
  :: CudaSparse()
   : m_deviceCount(1)
   , m_deviceRank(0)
   , m_handle(0)
{
  CudaException :: check(   cudaGetDeviceCount(&m_deviceCount) );
 
  int rank = 0 ;
  int procs = 1;

#if HAVE_MPI
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &procs);
#endif

  m_deviceRank = rank % m_deviceCount;
  CudaException :: check(   cudaSetDevice( m_deviceRank ) );

  CuSparseException::check( cusparseCreate( & m_handle ));
  CuSparseException::check( cusparseSetPointerMode(m_handle, CUSPARSE_POINTER_MODE_HOST) );
}

CudaSparse
  :: ~CudaSparse()
{
  cusparseDestroy(m_handle);
}

CudaSparse &
CudaSparse
  :: instance() 
{
  static CudaSparse object;
  return object;
}

}

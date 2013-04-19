#include "cudaarray.h"

#include "cudaexception.h"

namespace hpc
{
 
  
CudaArray<void>
  :: CudaArray( const CudaArray & other)
  : m_size(0)
  , m_array(0)
{
  this->resize( other.size() );

  try
  {
    cudaMemcpy( this->m_array, other.m_array, m_size, cudaMemcpyDeviceToDevice);
    CudaException::check();
  }
  catch(...)
  {
    clear();
    throw;
  }
}

void
CudaArray<void>
  ::  resize(SizeType size)
{
  clear();

  cudaMalloc( &m_array, size);
  CudaException::check();
  m_size = size;
}

void
CudaArray<void>
  :: clear()
{
  if (m_array)
  {
    cudaFree(m_array);
    CudaException::check();
    m_size = 0;
    m_array = 0;
  }
}

void
CudaArray<void>
  :: getData(void * dst, SizeType srcOffset, SizeType length) const
{
  cudaMemcpy(dst, static_cast<const char *>(m_array) + srcOffset, length
    , cudaMemcpyDeviceToHost);
  CudaException::check();
  cudaDeviceSynchronize();
  CudaException::check();
}

void
CudaArray<void>
  :: setData(const void * src, SizeType dstOffset, SizeType length) 
{
  cudaMemcpy(static_cast<char *>(m_array) + dstOffset, src, length
    , cudaMemcpyHostToDevice);
  CudaException::check();
  cudaDeviceSynchronize();
  CudaException::check();
}


}



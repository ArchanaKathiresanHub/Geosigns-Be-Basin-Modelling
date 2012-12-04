#ifndef HPC_CUDASPARSE_H
#define HPC_CUDASPARSE_H

#include <cusparse_v2.h>

namespace hpc 
{


class CudaSparse
{
public:
  ~CudaSparse();

  static CudaSparse & instance();

  cusparseHandle_t handle() const
  { return m_handle; }

  int deviceRank() const
  { return m_deviceRank; }

  int deviceCount() const
  { return m_deviceCount; }

private:
  CudaSparse();
  CudaSparse(const CudaSparse & ); // prohibit copying
  CudaSparse & operator=(const CudaSparse & ); // prohibit assignment

  int m_deviceCount;
  int m_deviceRank;
  cusparseHandle_t m_handle;
};

}

#endif


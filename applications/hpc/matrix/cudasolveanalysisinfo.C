#include "cudasolveanalysisinfo.h"
#include "cudaexception.h"

#include <cusparse_v2.h>

namespace hpc
{

CudaSolveAnalysisInfo
  :: CudaSolveAnalysisInfo()
  : m_solveInfo()
{ 
  CuSparseException::check(
      cusparseCreateSolveAnalysisInfo(&m_solveInfo)
    );
}


CudaSolveAnalysisInfo
  :: ~CudaSolveAnalysisInfo()
{
  // FIXME: log errors
  cusparseDestroySolveAnalysisInfo(m_solveInfo);
}

}

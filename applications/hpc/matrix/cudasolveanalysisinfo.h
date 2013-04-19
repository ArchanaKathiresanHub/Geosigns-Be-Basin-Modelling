#ifndef HPC_CUDASOLVEANALYSISINFO_H
#define HPC_CUDASOLVEANALYSISINFO_H

#include <cusparse_v2.h>

namespace hpc
{

class CudaSolveAnalysisInfo
{
public:
  CudaSolveAnalysisInfo();
  ~CudaSolveAnalysisInfo();

  cusparseSolveAnalysisInfo_t  handle() const
  { return m_solveInfo; }

private:
  CudaSolveAnalysisInfo(const CudaSolveAnalysisInfo & ); // copying prohibited
  CudaSolveAnalysisInfo & operator=(const CudaSolveAnalysisInfo & ); // assignment prohibited
  cusparseSolveAnalysisInfo_t m_solveInfo;
};


class CudaSolveAnalysis
{



};

}

#endif

#ifndef HPC_ILU_H
#define HPC_ILU_H

#include "generalexception.h"
#include "cudasolveanalysisinfo.h"
#include "cudamatdescr.h"
#include "cudacsrmatrix.h"
#include "cudahybmatrix.h"
#include "petscvector.h"
#include "petscmatrix.h"
#include "cudavector.h"

#include <cusparse_v2.h>

namespace hpc
{

struct ILUException : BaseException<ILUException> {};


template <typename Matrix, typename Vector>
class ILU;

template <>
class ILU<PetscMatrix, PetscVector>
{ 
public:
  template <typename M, typename V> friend class ILU;

  typedef int SizeType;
  typedef double ValueType;

  ILU( const PetscMatrix & matrix);
  
  void apply( const PetscVector &, PetscVector & y );

private:
  PetscMatrix m_matrix;
  PetscVector m_tmpVector;
};

template <>
class ILU<CudaCSRMatrix, CudaVector>
{ 
public:
  template <typename M, typename V> friend class ILU;

  typedef int SizeType;
  typedef double ValueType;

  ILU( const PetscMatrix & matrix);
  
  void apply( const CudaVector &, CudaVector & y );

private:
  CudaCSRMatrix m_matrix;
  CudaVector m_tmpVector;
  CudaMatrixDescription m_lMatDescr, m_uMatDescr;
  CudaSolveAnalysisInfo m_lSolveInfo, m_uSolveInfo;
};

template <>
class ILU<CudaHYBMatrix, CudaVector>
{ 
public:
  template <typename M, typename V> friend class ILU;

  typedef int SizeType;
  typedef double ValueType;

  ILU( const PetscMatrix & matrix);
  
  void apply( const CudaVector &, CudaVector & y );

private:
  CudaHYBMatrix m_matrix;
  CudaVector m_tmpVector;
  CudaMatrixDescription m_lMatDescr, m_uMatDescr;
  CudaSolveAnalysisInfo m_lSolveInfo, m_uSolveInfo;
};

}

#endif

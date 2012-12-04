#include <iostream>
#include <fstream>
#include <cassert>

#include "petscmatrix.h"
#include "petscvector.h"
#include "cudaexception.h"
#include "gettime.h"
#include "cudaarray.h"

#include <cusparse_v2.h>

struct RuntimeException : hpc::BaseException< RuntimeException > {};

int main(int argc, char ** argv)
{
  using namespace hpc;

  // setup cuSPARSE
  cusparseHandle_t cusparse ;
  cusparseCreate(&cusparse);
  CudaException::check();

  cusparseSetPointerMode(cusparse, CUSPARSE_POINTER_MODE_DEVICE);
  CudaException::check();

  // read matrix
  if (argc < 2)
    throw RuntimeException() << "Not enough parameters.";

  std::ifstream input( argv[1] );

  if (!input)
    throw RuntimeException() << "Could not open file '" << argv[1] << "'.";

  PetscMatrix A = PetscMatrix::load(input);
  PetscVector b = PetscVector::load(input);
  PetscVector x = PetscVector::load(input);
  PetscVector r = PetscVector::load(input);

  input.close();

  const int dim = A.rows();
  const int nnz = A.numberNonZeros();
  const double alpha = 1.0;
  const double beta = 1.0;

  std::cout << "Matrix: " << A.rows() << " x " << A.columns() << " - " <<
    A.numberNonZeros() << " non-zeros" << std::endl;
  
  // copy matrices and vector to GPU
  CudaArray<double> d_A_NonZeros( A.nonZeros(), A.nonZeros() + nnz );
  CudaArray<int> d_A_RowOffsets( A.rowOffsets(), A.rowOffsets() + dim + 1);
  CudaArray<int> d_A_ColumnIndices( A.columnIndices(), A.columnIndices() + nnz);

  CudaArray<double> d_b_values( &b[0], &b[0] + dim );
  CudaArray<double> d_x_values( &x[0], &x[0] + dim );

  CudaArray<double> d_alpha( &alpha, &alpha + 1);
  CudaArray<double> d_beta( &beta, &beta + 1);
  
      
  cusparseMatDescr_t matdescra;
  cusparseCreateMatDescr(&matdescra); CudaException::check();
  cusparseSetMatType(matdescra, CUSPARSE_MATRIX_TYPE_GENERAL);
  cusparseSetMatIndexBase(matdescra, CUSPARSE_INDEX_BASE_ZERO);

  // measure a multiplication with CSR matrix format
  { 
    int N = 0;
    const double duration = 10.0;
    double t0 = getTime(), t1 = 0.0;
    while (t0 + duration > (t1=getTime()))
    {
      for (int i = 0; i < 100; ++i, ++N)
      {
	cusparseOperation_t transa = CUSPARSE_OPERATION_NON_TRANSPOSE;
	cusparseStatus_t result = cusparseDcsrmv( cusparse
	    , transa
	    , dim, dim, nnz, d_alpha.devicePointer(), matdescra
	    , d_A_NonZeros.devicePointer()
	    , d_A_RowOffsets.devicePointer()
	    , d_A_ColumnIndices.devicePointer()
	    , d_b_values.devicePointer()
	    , d_beta.devicePointer(), d_x_values.devicePointer()
	    );

	switch(result)
	{
	  case CUSPARSE_STATUS_SUCCESS:
	    break;
	  
	  case CUSPARSE_STATUS_NOT_INITIALIZED:
	    throw RuntimeException() << "CUSPARSE library wasn't initialized";

	  case CUSPARSE_STATUS_ALLOC_FAILED:
	    throw RuntimeException() << "Resource allocation failed in CUSPARSE library";
	  case CUSPARSE_STATUS_INVALID_VALUE:
	    throw RuntimeException() << "Invalid parameter";
	  case CUSPARSE_STATUS_ARCH_MISMATCH:
	    throw RuntimeException() << "Device architecture mismatch";
	  case CUSPARSE_STATUS_MAPPING_ERROR:
	    throw RuntimeException() << "memory mapping error";
	  case CUSPARSE_STATUS_EXECUTION_FAILED:
	    throw RuntimeException() << "GPU program failed to execute";
	  case CUSPARSE_STATUS_INTERNAL_ERROR:
	    throw RuntimeException() << "Internal error";
	  case CUSPARSE_STATUS_MATRIX_TYPE_NOT_SUPPORTED:
	    throw RuntimeException() << "Matrix type is not supported.";
	 }
      }
      cudaDeviceSynchronize();
      CudaException::check();
    }
    std::cout << "\n=== CSR Format ===\n";
    std::cout << "Time/matrix = " << (t1-t0)/N << " seconds." << std::endl;
    double flops = 2.0 * N * A.numberNonZeros() / (t1 - t0);
    std::cout << "FLOPS = " << flops << std::endl;
  }

  // Convert to HYB
  cusparseHybMat_t d_A_hyb;
  if ( cusparseCreateHybMat(&d_A_hyb) != CUSPARSE_STATUS_SUCCESS)
    throw RuntimeException() << "Error creating Hyb matrix";

  if ( cusparseDcsr2hyb(cusparse, dim, dim, matdescra, d_A_NonZeros.devicePointer()
      , d_A_RowOffsets.devicePointer(), d_A_ColumnIndices.devicePointer()
      , d_A_hyb, 27, CUSPARSE_HYB_PARTITION_AUTO) != CUSPARSE_STATUS_SUCCESS)
    throw RuntimeException() << "Error converting matrix to Hyb";

  {  // Measure Hyb multiplication
    int N = 0;
    const double duration = 10.0;
    double t0 = getTime(), t1 = 0.0;
    while (t0 + duration > (t1=getTime()))
    {
      for (int i = 0; i < 100; ++i, ++N)
      {
	cusparseOperation_t transa = CUSPARSE_OPERATION_NON_TRANSPOSE;
	cusparseStatus_t result = cusparseDhybmv( cusparse
	    , transa
	    , d_alpha.devicePointer(), matdescra
	    , d_A_hyb
	    , d_b_values.devicePointer()
	    , d_beta.devicePointer(), d_x_values.devicePointer()
	    );

	switch(result)
	{
	  case CUSPARSE_STATUS_SUCCESS:
	    break;
	  
	  case CUSPARSE_STATUS_NOT_INITIALIZED:
	    throw RuntimeException() << "CUSPARSE library wasn't initialized";

	  case CUSPARSE_STATUS_ALLOC_FAILED:
	    throw RuntimeException() << "Resource allocation failed in CUSPARSE library";
	  case CUSPARSE_STATUS_INVALID_VALUE:
	    throw RuntimeException() << "Invalid parameter";
	  case CUSPARSE_STATUS_ARCH_MISMATCH:
	    throw RuntimeException() << "Device architecture mismatch";
	  case CUSPARSE_STATUS_MAPPING_ERROR:
	    throw RuntimeException() << "memory mapping error";
	  case CUSPARSE_STATUS_EXECUTION_FAILED:
	    throw RuntimeException() << "GPU program failed to execute";
	  case CUSPARSE_STATUS_INTERNAL_ERROR:
	    throw RuntimeException() << "Internal error";
	  case CUSPARSE_STATUS_MATRIX_TYPE_NOT_SUPPORTED:
	    throw RuntimeException() << "Matrix type is not supported.";
	 }
      }
      cudaDeviceSynchronize();
      CudaException::check();
    }
    std::cout << "\n=== HYB Format ===\n";
    std::cout << "Time/matrix = " << (t1-t0)/N << " seconds." << std::endl;
    double flops = 2.0 * N * A.numberNonZeros() / (t1 - t0);
    std::cout << "FLOPS = " << flops << std::endl;
  }

  



  cusparseDestroyHybMat(d_A_hyb);
 

  cusparseDestroyMatDescr(matdescra);
  cusparseDestroy(cusparse);
  return 0;
}


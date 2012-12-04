#if HAVE_MPI
  #include <mpi.h>
#endif

#include "petscmatrix.h"
#include "petscvector.h"
#include "diacoomatrix.h"
#include "cudacsrmatrix.h"
#include "cudahybmatrix.h"
#include "cudavector.h"
#include "matrix.h"
#include "gettime.h"
#include "cudaexception.h"
#include "cudasparse.h"

#include <fstream>
#include <iostream>
#include <iomanip>
#include <cassert>


using namespace hpc;

struct RuntimeException : hpc::BaseException< RuntimeException > {};


template <typename Matrix>
struct MatrixTypeTraits
{
  static const char * s_name;
};

template <> const char * MatrixTypeTraits< hpc::PetscMatrix > :: s_name = "CSR";
template <> const char * MatrixTypeTraits< hpc::DIAMatrix > :: s_name = "DIA";
template <> const char * MatrixTypeTraits< hpc::COOMatrix > :: s_name = "COO";
template <> const char * MatrixTypeTraits< hpc::DIACOOMatrix > :: s_name = "DIA+COO";
template <> const char * MatrixTypeTraits< hpc::CudaCSRMatrix > :: s_name = "CUDA-CSR";
template <> const char * MatrixTypeTraits< hpc::CudaHYBMatrix > :: s_name = "CUDA-HYB";


template <typename Matrix, typename Vector>
void benchmark(int ITER, int procs, int rank, double flops, const Matrix & A, const Vector & b, const Vector & x)
{ 
  Vector y = x;

#if HAVE_MPI
  MPI_Barrier(MPI_COMM_WORLD);
#endif
  double t0 = hpc::getTime();
  for (int i = 0 ; i < ITER; ++i)
    A.multiply( 1.0, b, 1.0, y);

  // read an element to force synchronization
  (void) y[0];

  double t1 = hpc::getTime();

#if HAVE_MPI  
  MPI_Barrier(MPI_COMM_WORLD);
#endif
  double t2 = hpc::getTime();

  double t = t1-t0;
  double maxT = t, minT = t, sumT = t;
#if HAVE_MPI
  MPI_Reduce( &t, &maxT, 1, MPI_DOUBLE, MPI_MAX , 0, MPI_COMM_WORLD);
  MPI_Reduce( &t, &minT, 1, MPI_DOUBLE, MPI_MIN , 0, MPI_COMM_WORLD);
  MPI_Reduce( &t, &sumT, 1, MPI_DOUBLE, MPI_SUM , 0, MPI_COMM_WORLD);
#endif
  const double avgT = sumT / procs;
  
  flops *= ITER;
  const double maxFlops = flops / minT;
  const double minFlops = flops / maxT;
  const double aggFlops = procs * flops / (t2 - t0) ;
  if (rank == 0)
  {
    std::cout << std::setw(15) << MatrixTypeTraits<Matrix>::s_name
              << std::setw(10) << procs
	      << std::setw(10) << CudaSparse::instance().deviceCount()
              << std::setw(15) << maxT
	      << std::setw(15) << minT
	      << std::setw(15) << avgT
	      << std::setw(15) << maxT/ITER
	      << std::setw(20) << maxFlops
	      << std::setw(20) << minFlops
	      << std::setw(20) << aggFlops
	      << '\n';
  }
}

template <typename Matrix, typename Vector>
void check( const PetscMatrix & origMatrix, const PetscVector & origVector
    , const Matrix & matrix, const Vector & x, Vector y
    , double tolerance)
{
  assert( origVector.rows() == y.rows() );

  if (! matrixEqual( origMatrix, matrix))
     throw RuntimeException() << MatrixTypeTraits<Matrix>::s_name << " matrix is not equal to original";

  matrix.multiply( 1.0, x, 1.0, y);

  const double machineEps = std::numeric_limits<double>::epsilon();

  if (! std::equal( y.begin(), y.end(), origVector.begin(), RelativlyEqual(tolerance * machineEps) ))
     throw RuntimeException() 
       << MatrixTypeTraits<Matrix>::s_name 
       << " matrix multiplication does not give the same results as the original";
}

void check( const PetscMatrix & origMatrix, const PetscVector & origVector
    , const CudaHYBMatrix & matrix, const CudaVector & x, CudaVector y
    , double tolerance)
{
  assert( origVector.rows() == y.rows() );

  matrix.multiply( 1.0, x, 1.0, y);

  const double machineEps = std::numeric_limits<double>::epsilon();

  if (! std::equal( y.begin(), y.end(), origVector.begin(), RelativlyEqual(tolerance * machineEps) ))
     throw RuntimeException() 
       << MatrixTypeTraits<CudaHYBMatrix>::s_name 
       << " matrix multiplication does not give the same results as the original";
}



int main(int argc, char ** argv)
{
#if HAVE_MPI
  struct MPILib
  {
    MPILib(int & argc, char ** & argv) { MPI_Init(&argc, &argv); }
    ~MPILib() { MPI_Finalize(); }

    int procs() const
    { 
      int p = 0;
      MPI_Comm_size(MPI_COMM_WORLD, &p);
      return p;
    }

    int rank() const
    { 
      int r = 0;
      MPI_Comm_rank(MPI_COMM_WORLD, &r);
      return r;
    }

  } mpilib(argc, argv);

 const int rank = mpilib.rank();
 const int procs = mpilib.procs();
#else

 const int rank = 0;
 const int procs = 1;
#endif

 // init CUDA
 CudaSparse::instance();

 // parse parameter
  if (argc < 2)
    throw RuntimeException() << "Not enough parameters.";

  const bool needCheck = argc > 2; // Supplying a second parameter enables checks

  std::ifstream input( argv[1] );

  if (!input)
    throw RuntimeException() << "Could not open file '" << argv[1] << "'.";

  PetscMatrix A = PetscMatrix::load(input);
  PetscVector b = PetscVector::load(input);
  PetscVector x = PetscVector::load(input);
  PetscVector r = PetscVector::load(input);

  input.close();

  if (rank == 0)
    std::cout   << std::setw(15) << "Matrix format"
                << std::setw(10) << "CPU Cores"
		<< std::setw(10) << "GPUs"
                << std::setw(15) << "Max Time" 
	        << std::setw(15) << "Min Time" 
		<< std::setw(15) << "Avg. Time"
		<< std::setw(15) << "Sec/Multiply"
		<< std::setw(20) << "Max FLOPS/sec"
		<< std::setw(20) << "Min FLOPS/sec"
		<< std::setw(20) << "Agg. FLOPS/sec"
		<< '\n';

  const double flops = 2.0 * A.numberNonZeros();
  
  PetscVector y = x;
  {
    A.multiply( 1.0, b, 1.0, y);
    benchmark( 1000, procs, rank, flops, A, b, x);
  }

#if 0
  {
    DIAMatrix D( A.rows(), A.columns() );
    D.insert( A.begin(), A.end() );
    PetscVector z = x;

    if (needCheck)
      check( A, y, D, b, z, 1e+8);

    benchmark( 1000, procs, rank, flops, D, b, x);
  }
#endif

  {
    COOMatrix E( A.rows(), A.columns(), A.begin(), A.end() );
    PetscVector z = x;
    
    if (needCheck)
      check( A, y, E, b, z, 1e+8);
    
    benchmark( 1000, procs, rank, flops, E, b, x);
  }

  
  {
    DIACOOMatrix F = DIACOOMatrix::create(A.rows(), A.columns(), 27, A.begin(),A.end());
    PetscVector z = x;
    
    if (needCheck)
      check( A, y, F, b, z, 1e+8);

    benchmark( 1000, procs, rank, flops, F, b, x);
  }

 
  try
  {
    CudaCSRMatrix G(A);
    CudaHYBMatrix H(G);
    {
      CudaVector c = b;
      CudaVector z = x;

      if (needCheck)
        check( A, y, G, c, z, 1e+8);

      benchmark( 1000, procs, rank, flops, G, c, z);
    }
    
    {
      CudaVector c = b;
      CudaVector z = x;

      if (needCheck)
        check( A, y, H, c, z, 1e+8);

      benchmark( 1000, procs, rank, flops, H, c, z);
    }
  }
  catch( CuSparseException & e)
  {
    if (rank == 0)
      std::cerr << "CUDA benchmarks failed:\n\t" << e.what() << std::endl ;
  }

  return 0;
}

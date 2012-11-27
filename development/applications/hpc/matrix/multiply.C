#include "mklmatrix.h"
#include "petscmatrix.h"
#include "petscvector.h"
#include "generalexception.h"
#include "gettime.h"

#include <fstream>
#include <iostream>

struct RuntimeException : hpc::BaseException< RuntimeException > {};

int main(int argc, char ** argv)
{
  using namespace hpc;
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

  std::cout << "Matrix: " << A.rows() << " x " << A.columns() << " - " <<
    A.numberNonZeros() << " non-zeros" << std::endl;

  int N = 0;
  const double duration = 10.0;
  double t0 = getTime(), t1 = 0.0;
  while ( t0 + duration > (t1=getTime()) )
      for (int i = 0; i < 100; ++i, ++N)
        MklCsrMultiply(1.0, A, b, 0.0, x);

  std::cout << "Time/matrix = " << (t1-t0)/N << " seconds." << std::endl;
  double flops = 2.0 * N * A.numberNonZeros() / (t1 - t0);
  std::cout << "FLOPS = " << flops << std::endl;

  return 0;
}


#include <iostream>
#include <fstream>
#include <vector>
#include <tr1/unordered_set>
#include <tr1/unordered_map>

#include "petscmatrix.h"
#include "petscvector.h"
#include "generalexception.h"

using namespace hpc;

struct RuntimeException : hpc::BaseException< RuntimeException > {};

int main(int argc, char ** argv)
{
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


  std::vector<int> sizeLevelSet(A.rows(), 0);

  for (int i = 0; i < A.rows(); ++i)
  {
    const int * dependenciesBegin = A.columnIndices() + A.rowOffsets()[i];
    const int * dependenciesEnd = A.columnIndices() + A.rowOffsets()[i+1];

    int maxSizeLevelSet = 0;
    for (const int * dep = dependenciesBegin; dep != dependenciesEnd && *dep < i ; ++dep)
      maxSizeLevelSet = std::max(sizeLevelSet[*dep], maxSizeLevelSet);

    sizeLevelSet[i] = maxSizeLevelSet+1;
  }

  int maxSizeLevelSet = 0;
  for (int i = 0; i < A.rows(); ++i)
    maxSizeLevelSet = std::max( maxSizeLevelSet, sizeLevelSet[i] );

  int biggestRow = 0;
  for (int i = 0; i < A.rows(); ++i)
    biggestRow = std::max( biggestRow, A.rowOffsets()[i+1]-A.rowOffsets()[i]);

  std::cout << "Dimension: " << A.rows() << std::endl;
  std::cout << "Number of nonzeros: " << A.numberNonZeros() << std::endl;
  std::cout << "Max entries per row: " << biggestRow << std::endl;
  std::cout << "Level lower triangle from ILU(0) = " << maxSizeLevelSet << std::endl;


  return 0;
}

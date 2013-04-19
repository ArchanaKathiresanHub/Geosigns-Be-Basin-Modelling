#include <iostream>
#include <fstream>
#include <cassert>

#include "petscmatrix.h"
#include "petscvector.h"
#include "generalexception.h"
#include "gettime.h"
#include "diacoomatrix.h"

#include <mkl.h>
#include <mkl_spblas.h>

struct RuntimeException : hpc::BaseException< RuntimeException > {};


int main( int argc, char ** argv)
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

  // determine which diagonals are occupied
  std::vector<int> diagonals( 2 * A.rows() - 1, 0);
  for (PetscMatrix::Iterator i = A.begin(); i != A.end(); ++i)
    diagonals[ A.rows() - (*i).row + (*i).column - 1] ++;

  
  // count number of diagonals
  int32_t count = 0;
  for (unsigned i = 0; i < diagonals.size(); ++i)
    count +=  ( diagonals[i] > 0 ? 1 : 0) ;

  // compute storage requirements
  size_t diagonalStorage = (count * A.rows() * sizeof(double)) + (count+2) * sizeof(int32_t);
  size_t csrStorage = (count + 2) * sizeof(uint32_t) + A.numberNonZeros() * (sizeof(int32_t)+sizeof(double));

  // compute histogram: how many diagonals have a certain  fill percentage
  std::vector< int > histogram(11); 
  std::vector< int > mainDiags, sparseDiags;
  for (unsigned i = 0; i < diagonals.size(); ++i)
  {
    if (diagonals[i] > 0)
    {
      size_t numberOfAllDiagonalElements = i < unsigned(A.rows()) ? (i+1) : (2*A.rows() - i - 1) ;
      double fill = 1.0 * diagonals[i] / numberOfAllDiagonalElements ;
      histogram[ unsigned(10.0 * fill) ]++;
      
      if (fill > 0.1)
	mainDiags.push_back( i );
      else
	sparseDiags.push_back( i);
    }
  }

/*  std::cout << "Histogram\n";
  for (int i = 0; i < 10; ++i)
  {
    std::cout << 10*i << "% - " << 10*(i+1) << "%: " << histogram[i] << std::endl;
  } 
*/

  // compute hybride storage: Diagonal storage format + COO for sparse diags
  size_t mainDiagStorage = (mainDiags.size() * A.rows() * sizeof(double)) + (mainDiags.size() + 2) * sizeof(int32_t);
  size_t sparseDiagStorage = 0;
  for (unsigned i = 0; i < sparseDiags.size(); ++i)
    sparseDiagStorage += (2*sizeof(int32_t) + sizeof(double)) * diagonals[ sparseDiags[i] ];

  std::cout << "Matrix: " << A.rows() << " x " << A.columns() 
    << "\t- " << A.numberNonZeros() << " non-zeros\n" 
    << "\t- " << count << " diagonals: " << mainDiags.size() << " main diagonals + " << sparseDiags.size() << " sparse diagonals\n"
    << "\t- " << diagonalStorage*1e-6 << " MB in diagonal storage\n"
    << "\t- " << csrStorage*1e-6 << " MB in CSR\n"
    << "\t- " << mainDiagStorage*1e-6 << " MB main diagonals + " << sparseDiagStorage*1e-6 << " MB sparse diagonals = " << (mainDiagStorage + sparseDiagStorage)*1e-6 << " MB hybrid." << std::endl;


  // let's execute a matrix multiplication
  
  // step 1: convert the main diagonals to diagonal format
  int dim = A.rows();
  int nDiags = mainDiags.size();
  std::vector< MKL_INT > distance(nDiags);
  std::vector< double > diagValues( nDiags * dim );
  int spNz=0;
  std::vector<int> spRows, spColumns;
  std::vector<double> spValues;
  {
    std::vector< int > diagonalToDiagNr(2*dim - 1, -1);
    for (int i = 0; i < nDiags; ++i)
    {
      distance[i] = mainDiags[i] + 1 - dim;
      diagonalToDiagNr[ mainDiags[i] ] = i;
    }

    for (PetscMatrix::Iterator i = A.begin(); i != A.end(); ++i)
    {
      int row = (*i).row;
      int column = (*i).column;
      int diagonal = dim -1 - row + column;
      
      int diagNr=diagonalToDiagNr[diagonal];
      if (diagNr != -1)
      { // the in one of the main diagonals
        diagValues[ diagNr * dim + row ] = (*i).value;
      }
      else
      {
	spRows.push_back( row);
	spColumns.push_back( column );
	spValues.push_back( (*i).value );
	++spNz;
      }
    }
  }

  // measure a multiplication
  int N = 0;
  const double duration = 10.0;
  double t0 = getTime(), t1 = 0.0;
  while (t0 + duration > (t1=getTime()))
  {
    for (int i = 0; i < 100; ++i, ++N)
    {
      char transa = 'N';
      char matdescra[6] = "GLNC";
      double alpha = 1.0;
      double beta = 1.0;
      mkl_ddiamv( & transa, &dim, &dim, &alpha, matdescra, &diagValues[0], &dim, &distance[0], &nDiags
	  , &b[0]
	  , &beta, &x[0]
	  );

      mkl_dcoomv( &transa, &dim, &dim, &alpha, matdescra, &spValues[0], &spRows[0], &spColumns[0], &spNz
	  , &b[0]
	  , &beta, &x[0]
	  );
    }
  }
  std::cout << "Time/matrix = " << (t1-t0)/N << " seconds." << std::endl;
  double flops = 2.0 * N * A.numberNonZeros() / (t1 - t0);
  std::cout << "FLOPS = " << flops << std::endl;


  return 0;
}

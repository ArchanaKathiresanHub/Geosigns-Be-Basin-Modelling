#include <iostream>
#include <sstream>
#include "test.h"
#include "petscmatrix.h"



using namespace hpc;



// Test: PetscMatrix::PetscMatrix
// Input: Number of rows (M) and columns (N)
// Equivalence classes
//   - Valid:
//      * M=0, M >= 1
//      * N = 0, N >= 1
void testPetscMatrixEmptyMatrixConstruction()
{
  {
    PetscMatrix matrix(0, 0);
    ASSERT( matrix.rows() == 0 );
    ASSERT( matrix.columns() == 0);
    ASSERT( matrix.m_rowOffsets.size() == 0);
    ASSERT( matrix.m_columnIndices.empty() );
    ASSERT( matrix.m_nonZeros.empty() );
  }

  {
    PetscMatrix matrix(1, 1);
    ASSERT( matrix.rows() == 1);
    ASSERT( matrix.columns() == 1);
    ASSERT( matrix.m_rowOffsets.size() == 0);
    ASSERT( matrix.m_columnIndices.empty() );
    ASSERT( matrix.m_nonZeros.empty() );
  }
}

// Test: PetscMatrix::load
// Input: Input stream 
// - Valid: 
//     * Input stream with matrix
// - Invalid:
//     * Input stream without matrix, with dense mstrix
void testPetscMatrixLoad()
{
  // Test case 1: 1x1 matrix, with element = 1.0

  char a[] = { 0x00, 0x12, 0x7b, 0x50 // header
             , 0x00, 0x00, 0x00, 0x01 // number of rows
             , 0x00, 0x00, 0x00, 0x01 // number of columns
             , 0x00, 0x00, 0x00, 0x01 // number of non-zeros
	     , 0x00, 0x00, 0x00, 0x01 // number of non-zeros in row 0
	     , 0x00, 0x00, 0x00, 0x00 // columns index of nonzero 0
             , 0x3F, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 // nonzero 0:  1.0
  };

  std::istringstream input( std::string(a, sizeof(a)));

  PetscMatrix matrix = PetscMatrix :: load( input );

  ASSERT( matrix.rows() == 1);
  ASSERT( matrix.columns() == 1);
  ASSERT( matrix.m_rowOffsets.size() == 2);
  ASSERT( matrix.m_rowOffsets[0] == 0);
  ASSERT( matrix.m_rowOffsets[1] == 1);
  ASSERT( matrix.m_columnIndices.size() == 1);
  ASSERT( matrix.m_columnIndices[0] == 0);
  ASSERT( matrix.m_nonZeros.size() == 1);
  ASSERT( matrix.m_nonZeros[0] == 1.0 );


  // TODO: Test invalid equivalence classes
}


// Test: PetscMatrix::load
// Input: matrix
// - Valid: 
//     * matrix
void testPetscMatrixSave()
{
  char a[] = { 0x00, 0x12, 0x7b, 0x50 // header
             , 0x00, 0x00, 0x00, 0x01 // number of rows
             , 0x00, 0x00, 0x00, 0x01 // number of columns
             , 0x00, 0x00, 0x00, 0x01 // number of non-zeros
	     , 0x00, 0x00, 0x00, 0x01 // number of non-zeros in row 0
	     , 0x00, 0x00, 0x00, 0x00 // columns index of nonzero 0
             , 0x3F, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 // nonzero 0:  1.0
  };


  // Test case 1: 1x1 matrix, with element = 1.0
  PetscMatrix matrix(1, 1);
  matrix.m_rowOffsets.resize(2);
  matrix.m_rowOffsets[0] = 0;
  matrix.m_rowOffsets[1] = 1;
  matrix.m_columnIndices.resize(1, 0u);
  matrix.m_nonZeros.resize(1, 1.0);

  ASSERT( matrix.rows() == 1);
  ASSERT( matrix.columns() == 1);
  ASSERT( matrix.m_rowOffsets.size() == 2);
  ASSERT( matrix.m_rowOffsets[0] == 0);
  ASSERT( matrix.m_rowOffsets[1] == 1);
  ASSERT( matrix.m_columnIndices.size() == 1);
  ASSERT( matrix.m_columnIndices[0] == 0);
  ASSERT( matrix.m_nonZeros.size() == 1);
  ASSERT( matrix.m_nonZeros[0] == 1.0 );

  std::ostringstream output;
  matrix.save(output);

  ASSERT( output.str() == std::string( a, sizeof(a)));
}

// Test: PetscIterator
// Input: matrix
void testPetscMatrixIterator()
{
  PetscMatrix matrix(1, 1);
  matrix.m_rowOffsets.resize(2);
  matrix.m_rowOffsets[0] = 0;
  matrix.m_rowOffsets[1] = 1;
  matrix.m_columnIndices.resize(1, 0u);
  matrix.m_nonZeros.resize(1, 1.0);

  ASSERT( matrix.rows() == 1);
  ASSERT( matrix.columns() == 1);
  ASSERT( matrix.m_rowOffsets.size() == 2);
  ASSERT( matrix.m_rowOffsets[0] == 0);
  ASSERT( matrix.m_rowOffsets[1] == 1);
  ASSERT( matrix.m_columnIndices.size() == 1);
  ASSERT( matrix.m_columnIndices[0] == 0);
  ASSERT( matrix.m_nonZeros.size() == 1);
  ASSERT( matrix.m_nonZeros[0] == 1.0 );

  PetscMatrix::Iterator i = matrix.begin();
  ASSERT( i != matrix.end() );
  ASSERT( (*i).column == 0 );
  ASSERT( (*i).row == 0 );
  ASSERT( (*i).value == 1.0 );

  ++i;

  ASSERT( i == matrix.end() );

  --i;
  ASSERT( i == matrix.begin() );

  PetscMatrix::Iterator j = i++;
  ASSERT( j == matrix.begin());
  ASSERT( i == matrix.end() );

  j = i--;
  ASSERT( j == matrix.end() );
  ASSERT( i == matrix.begin() );
}

// Test: PetscMatrix::insert
// Input: Matrix, row, column, value
// Equivalence classes
//   - Matrix: empty matrix, non-empty matrix
//   - row at the end of the matrix, row in the middle of the matrix
//   - column at the end of the matrix of the last row, column in the middle of the row
//   - value: any

void testPetscMatrixInsert()
{
  // test case: empty matrix
  { PetscMatrix matrix(1,1);
    matrix.insert( 0, 0, 1.0);

    ASSERT( matrix.rows() == 1);
    ASSERT( matrix.columns() == 1);
    ASSERT( matrix.m_rowOffsets.size() == 2);
    ASSERT( matrix.m_rowOffsets[0] == 0);
    ASSERT( matrix.m_rowOffsets[1] == 1);
    ASSERT( matrix.m_columnIndices.size() == 1);
    ASSERT( matrix.m_columnIndices[0] == 0);
    ASSERT( matrix.m_nonZeros.size() == 1);
    ASSERT( matrix.m_nonZeros[0] == 1.0 );
  }



  { // test case: non empty matrix
    PetscMatrix matrix(3, 3);
    matrix.m_rowOffsets.resize(2);
    matrix.m_rowOffsets[0] = 0;
    matrix.m_rowOffsets[1] = 1;
    matrix.m_columnIndices.resize(1, 0u);
    matrix.m_nonZeros.resize(1, 1.0);

    ASSERT( matrix.rows() == 3);
    ASSERT( matrix.columns() == 3);
    ASSERT( matrix.m_rowOffsets.size() == 2);
    ASSERT( matrix.m_rowOffsets[0] == 0);
    ASSERT( matrix.m_rowOffsets[1] == 1);
    ASSERT( matrix.m_columnIndices.size() == 1);
    ASSERT( matrix.m_columnIndices[0] == 0);
    ASSERT( matrix.m_nonZeros.size() == 1);
    ASSERT( matrix.m_nonZeros[0] == 1.0 );

    // insert at the end of the non-empty matrix
    matrix.insert(1, 1, 2.0);
  
    ASSERT( matrix.rows() == 3);
    ASSERT( matrix.columns() == 3);
    ASSERT( matrix.m_rowOffsets.size() == 3);
    ASSERT( matrix.m_rowOffsets[0] == 0);
    ASSERT( matrix.m_rowOffsets[1] == 1);
    ASSERT( matrix.m_rowOffsets[2] == 2);
    ASSERT( matrix.m_columnIndices.size() == 2);
    ASSERT( matrix.m_columnIndices[0] == 0);
    ASSERT( matrix.m_columnIndices[1] == 1);
    ASSERT( matrix.m_nonZeros.size() == 2);
    ASSERT( matrix.m_nonZeros[0] == 1.0 );
    ASSERT( matrix.m_nonZeros[1] == 2.0 );

    // insert in the middle of the non-empty matrix
    matrix.insert(1,0, 3.0);
    ASSERT( matrix.rows() == 3);
    ASSERT( matrix.columns() == 3);
    ASSERT( matrix.m_rowOffsets.size() == 3);
    ASSERT( matrix.m_rowOffsets[0] == 0);
    ASSERT( matrix.m_rowOffsets[1] == 1);
    ASSERT( matrix.m_rowOffsets[2] == 3);
    ASSERT( matrix.m_columnIndices.size() == 3);
    ASSERT( matrix.m_columnIndices[0] == 0);
    ASSERT( matrix.m_columnIndices[1] == 0);
    ASSERT( matrix.m_columnIndices[2] == 1);
    ASSERT( matrix.m_nonZeros.size() == 3);
    ASSERT( matrix.m_nonZeros[0] == 1.0 );
    ASSERT( matrix.m_nonZeros[1] == 3.0 );
    ASSERT( matrix.m_nonZeros[2] == 2.0 );


    // insert a new row
    matrix.insert(2,2, 4.0);
    ASSERT( matrix.rows() == 3);
    ASSERT( matrix.columns() == 3);
    ASSERT( matrix.m_rowOffsets.size() == 4);
    ASSERT( matrix.m_rowOffsets[0] == 0);
    ASSERT( matrix.m_rowOffsets[1] == 1);
    ASSERT( matrix.m_rowOffsets[2] == 3);
    ASSERT( matrix.m_rowOffsets[3] == 4);
    ASSERT( matrix.m_columnIndices.size() == 4);
    ASSERT( matrix.m_columnIndices[0] == 0);
    ASSERT( matrix.m_columnIndices[1] == 0);
    ASSERT( matrix.m_columnIndices[2] == 1);
    ASSERT( matrix.m_columnIndices[3] == 2);
    ASSERT( matrix.m_nonZeros.size() == 4);
    ASSERT( matrix.m_nonZeros[0] == 1.0 );
    ASSERT( matrix.m_nonZeros[1] == 3.0 );
    ASSERT( matrix.m_nonZeros[2] == 2.0 );
    ASSERT( matrix.m_nonZeros[3] == 4.0 );

    // replacing a value
    matrix.insert(1,1, -2.0);
    ASSERT( matrix.rows() == 3);
    ASSERT( matrix.columns() == 3);
    ASSERT( matrix.m_rowOffsets.size() == 4);
    ASSERT( matrix.m_rowOffsets[0] == 0);
    ASSERT( matrix.m_rowOffsets[1] == 1);
    ASSERT( matrix.m_rowOffsets[2] == 3);
    ASSERT( matrix.m_rowOffsets[3] == 4);
    ASSERT( matrix.m_columnIndices.size() == 4);
    ASSERT( matrix.m_columnIndices[0] == 0);
    ASSERT( matrix.m_columnIndices[1] == 0);
    ASSERT( matrix.m_columnIndices[2] == 1);
    ASSERT( matrix.m_columnIndices[3] == 2);
    ASSERT( matrix.m_nonZeros.size() == 4);
    ASSERT( matrix.m_nonZeros[0] == 1.0 );
    ASSERT( matrix.m_nonZeros[1] == 3.0 );
    ASSERT( matrix.m_nonZeros[2] == -2.0 );
    ASSERT( matrix.m_nonZeros[3] == 4.0 );
  }
}



int main(int argc, char ** argv)
{
  if (argc < 2)
    FAIL("Missing test driver parameter");

  try
  {
    if (strcmp(argv[1], "emptyMatrixConstruction") == 0)
      testPetscMatrixEmptyMatrixConstruction();
    else if (strcmp(argv[1], "load") == 0)
      testPetscMatrixLoad();
    else if (strcmp(argv[1], "save") == 0)
      testPetscMatrixSave();
    else if (strcmp(argv[1], "iterator") == 0)
      testPetscMatrixIterator();
    else if (strcmp(argv[1], "insert") == 0)
      testPetscMatrixInsert();
    else 
      FAIL("Unrecognized parameter to test driver");
  }
  catch(TestFailure & e)
  {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

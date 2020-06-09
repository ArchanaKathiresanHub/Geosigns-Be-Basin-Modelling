#include <iostream>

#include "petscio.h"
#include "test.h"


using namespace hpc::petscio;



// Tests: convertHostToNetwork
// input: Type, Value
// equivalence classes:
//   - Valid ranges:
//     * Endianness: Little-endian (x86), Big-endian
//     * Type: byte, integral, double
//     * Value: positive, negative
//   - Invalid ranges:
//     * Type: non POD (plain-old-data)
//     * Value: NA
void testPetscMatrixHostToNetwork()
{
  ASSERT( convertHostToNetwork( char(0x01) ) == char(0x01) );
  ASSERT( isBigEndian() || 
      convertHostToNetwork( int32_t(0x01020304)) == int32_t(0x04030201)
  );
  ASSERT( isBigEndian() || 
      convertHostToNetwork( int32_t(-0x01020304)) == int32_t(0xFCFCFDFEu)
  );

  int64_t v = 0x3FF0000000000000ull;
  ASSERT( double(1.0) == * (const double *) &v);

  v = 0xF03F;
  ASSERT( isBigEndian() || 
      convertHostToNetwork( double(1.0)) == * (const double * ) & v
  );
  
  v = 0xF0BF;
  ASSERT( isBigEndian() || 
      convertHostToNetwork( double(-1.0)) == * (const double * ) & v
  );

  ASSERT( !isBigEndian() || 
      convertHostToNetwork( int32_t(-0x01020304))==int32_t(-0x01020304)
  );

  ASSERT( !isBigEndian() || 
      convertHostToNetwork( int32_t(0x01020304)) == int32_t(0x01020304)
  );
 
  ASSERT( !isBigEndian() || 
      convertHostToNetwork( double(1.0) ) == 1.0
  );
 
  ASSERT( !isBigEndian() || 
      convertHostToNetwork( double(-1.0)) == double(-1.0)
  );
}

// Tests: convertNetworkToHost
// input: Type, Value
// equivalence classes:
//   - Valid ranges:
//     * Endianness: Little-endian (x86), Big-endian
//     * Type: byte, integral, double
//     * Value: positive, negative
//   - Invalid ranges:
//     * Type: non POD (plain-old-data)
//     * Value: NA
void testPetscMatrixNetworkToHost()
{
  ASSERT( convertNetworkToHost( char(0x01) ) == char(0x01) );
  ASSERT( isBigEndian() || 
      convertNetworkToHost( int32_t(0x01020304)) == int32_t(0x04030201)
  );
  ASSERT( isBigEndian() || 
      convertNetworkToHost( int32_t(-0x01020304)) == int32_t(0xFCFCFDFEu)
  );

  int64_t v = 0x3FF0000000000000ull;
  ASSERT( double(1.0) == * (const double *) &v);

  v = 0xF03F;
  ASSERT( isBigEndian() || 
      convertNetworkToHost( double(1.0)) == * (const double * ) & v
  );
  
  v = 0xF0BF;
  ASSERT( isBigEndian() || 
      convertNetworkToHost( double(-1.0)) == * (const double * ) & v
  );

  ASSERT( !isBigEndian() || 
      convertNetworkToHost( int32_t(-0x01020304))==int32_t(-0x01020304)
  );

  ASSERT( !isBigEndian() || 
      convertNetworkToHost( int32_t(0x01020304)) == int32_t(0x01020304)
  );
 
  ASSERT( !isBigEndian() || 
      convertNetworkToHost( double(1.0) ) == 1.0
  );
 
  ASSERT( !isBigEndian() || 
      convertNetworkToHost( double(-1.0)) == double(-1.0)
  );
}

// Test: readIndices
// Input: Input stream, An array reference
// Equivalence classes:
//    - Valid: 
//       * an input stream with enough available indices to read
//       * an empty array, an non-empty array
//    - Invalid
//       * An input stream with non enough available indices
void testPetscMatrixReadIndices()
{
  char a[] = { 0x01, 0x02, 0x03, 0x04 };
  std::istringstream input( a );
  SizeType index = 0;

  readIndices(input, &index, 0);
  ASSERT( input.gcount() == 0 );
  ASSERT( index == 0 );

  readIndices(input, &index, 1);
  ASSERT( input.gcount() == sizeof(index) );
  ASSERT( index == 0x01020304 );

  bool caught = false;
  try
  {
    readIndices(input, &index, 1);
  }
  catch(ReadException & e)
  {
    caught = true;
  }

  ASSERT( caught );
  
  ASSERT( input.eof() );
}

// Test: readReal
// Input: Input stream, An array reference
// Equivalence classes:
//    - Valid: 
//       * an input stream with enough available real to read
//       * an empty array, an non-empty array
//    - Invalid
//       * An input stream with non enough available reals
void testPetscMatrixReadReal()
{
  char a[] = {0x3F, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  std::string s(a, sizeof(a));
  std::istringstream input( s );
  ValueType real = 0.0;

  readReal(input, &real, 0);
  ASSERT( input.gcount() == 0 );
  ASSERT( real == 0.0 );

  readReal(input, &real, 1);
  ASSERT( input.gcount() == sizeof(real) );
  ASSERT( real == 1.0 );

  bool caught = false;
  try
  {
    readReal(input, &real, 1);
  }
  catch(ReadException & e)
  {
    caught = true;
  }

  ASSERT( caught );
  
  ASSERT( input.eof() );
}

// Test: writeReal
// Input: Output stream, a real value
// Equivalence classes:
//   - Valid: 
//     * An output stream 
//     * A real value
//   - Invalid:
//     * None
void testPetscMatrixWriteReal()
{
  std::ostringstream output;

  writeReal(output, 1.0);

  char a[] = {0x3F, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  std::string s(a, sizeof(a));
  ASSERT( output.str() == s );
}

// Test: writeIndices
// Input: Output stream, an array with indices
// Equivalence classes
//   - Valid:
//     * An output stream
//     * An array with indices: empty, non-empty
//   - Invalid:
//     * None
void testPetscMatrixWriteIndices()
{
  std::ostringstream output;

  ASSERT( output.str().empty() );

  SizeType empty[] = {};
  writeIndices(output, empty, 0 );

  ASSERT( output.str().empty() );

  SizeType nonEmpty[] = { 1 };
  writeIndices(output, nonEmpty, 1);

  char a[] = { 0x00, 0x00, 0x00, 0x01};
  ASSERT( output.str() == std::string(a, sizeof(a)) );

}


int main(int argc, char ** argv)
{
  if (argc < 2)
    FAIL("Missing test driver parameter");

  try
  {
    if (strcmp(argv[1], "convertHostToNetwork")==0)
      testPetscMatrixHostToNetwork();
    else if (strcmp(argv[1], "convertNetworkToHost")==0)
      testPetscMatrixNetworkToHost();
    else if (strcmp(argv[1], "readIndices") == 0)
      testPetscMatrixReadIndices();
    else if (strcmp(argv[1], "readReal") == 0)
      testPetscMatrixReadReal();
    else if (strcmp(argv[1], "writeReal") == 0)
      testPetscMatrixWriteReal();
    else if (strcmp(argv[1], "writeIndices") == 0)
      testPetscMatrixWriteIndices();
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

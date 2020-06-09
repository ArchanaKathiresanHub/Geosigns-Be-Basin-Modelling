#include <iostream>
#include <string>
#include <algorithm>
#include "test.h"
#include "diamatrix.h"


using namespace hpc;


void testDIAMatrixConstructSlow()
{
  DIAMatrix::Element elements[]
    = { { 0, 0, 1.0 }
      , { 1, 0, 2.0 }
      , { 2, 2, 5.0 }
      , { 2, 1, 3.0 }
      , { 3, 2, 4.0 }
      };

  int nnz = sizeof(elements)/sizeof(DIAMatrix::Element);
  DIAMatrix m( 4, 3);
  m.insert( elements, elements+nnz);

  ASSERT( m.rows() == 4 );
  ASSERT( m.columns() == 3);

  ASSERT( m.m_distances.size() == 2);
  ASSERT( m.m_distances[0] == -1);
  ASSERT( m.m_distances[1] == 0);
  ASSERT( m.m_values.size() == 8);

  double values[] = { 0, 2, 3, 4, 1, 0, 5, 0};
  ASSERT( m.m_values.size() == 8);
  ASSERT( std::equal( values, values + 8, m.m_values.begin()) );
  
  int di[] = { -1, -1, 0, 1, -1, -1, -1};
  ASSERT( m.m_distanceIndex.size() == 6);
  ASSERT( std::equal( di,di+6, m.m_distanceIndex.begin() ));
}

void testDIAMatrixForwardIterator()
{
  DIAMatrix::Element elements[]
    = { { 0, 0, 1.0 }
      , { 1, 0, 2.0 }
      , { 2, 2, 5.0 }
      , { 2, 1, 3.0 }
      , { 3, 2, 4.0 }
      };

  DIAMatrix::Element result[]
    = { { 1, 0, 2.0 }
      , { 2, 1, 3.0 }
      , { 3, 2, 4.0 }
      , { 0, 0, 1.0 }
      , { 1, 1, 0.0 }
      , { 2, 2, 5.0 }
      };

  int nnz = sizeof(elements)/sizeof(DIAMatrix::Element);
  int n = sizeof(result)/sizeof(DIAMatrix::Element);

  DIAMatrix m( 4, 3);
  m.insert( elements, elements+nnz);

  ASSERT( m.rows() == 4 );
  ASSERT( m.columns() == 3);

  int i = 0;
  for (DIAMatrix::Iterator j = m.begin(); j != m.end(); ++j, ++i)
  {
    ASSERT( (*j).row == result[i].row );
    ASSERT( (*j).column == result[i].column );
    ASSERT( (*j).value == result[i].value );
  }
  
  ASSERT( i == n);
}

void testDIAMatrixReverseIterator()
{
  DIAMatrix::Element elements[]
    = { { 0, 0, 1.0 }
      , { 1, 0, 2.0 }
      , { 2, 2, 5.0 }
      , { 2, 1, 3.0 }
      , { 3, 2, 4.0 }
      };

  DIAMatrix::Element result[]
    = { { 1, 0, 2.0 }
      , { 2, 1, 3.0 }
      , { 3, 2, 4.0 }
      , { 0, 0, 1.0 }
      , { 1, 1, 0.0 }
      , { 2, 2, 5.0 }
      };

  int nnz = sizeof(elements)/sizeof(DIAMatrix::Element);
  int n = sizeof(result)/sizeof(DIAMatrix::Element);

  DIAMatrix m( 4, 3);
  m.insert( elements, elements+nnz);

  int i = n;
  DIAMatrix::Iterator j = m.end(); 
  do
  {
    --j;
    --i;

    ASSERT( (*j).row == result[i].row );
    ASSERT( (*j).column == result[i].column );
    ASSERT( (*j).value == result[i].value );
  }
  while (j != m.begin() );

  ASSERT( i == 0);
}


int main(int argc, char ** argv)
{
  if (argc < 2)
    FAIL("Missing test driver parameter");

  std::string arg = argv[1];

  try
  {
    if (arg == "creationSlow")
      testDIAMatrixConstructSlow();
    else if (arg == "forwardIterator")
      testDIAMatrixForwardIterator();
    else if (arg == "reverseIterator")
      testDIAMatrixReverseIterator();
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

#ifndef HPC_TEST_H
#define HPC_TEST_H

#include "generalexception.h"

#define FAIL( msg ) \
  do { \
    throw TestFailure() \
              << "Failure in " << __FILE__ << ":" << __LINE__ << '\n' \
              << '\t' << msg ; \
  } while(0)

#define ASSERT( statement ) \
  do { \
    if ( ! (statement) ) \
      FAIL( "Assertion failed: '" #statement "'" ) ; \
  } while (0) 


// redefine access qualifiers 'private' and 'protected' to 'public' so that 
// unit tests can access private and protected members
#define private public
#define protected public


struct TestFailure : hpc::BaseException< TestFailure > {};


#endif

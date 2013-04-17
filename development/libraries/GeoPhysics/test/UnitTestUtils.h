#ifndef GEOPHYSICS_UNITTESTUTILS_H
#define GEOPHYSICS_UNITTESTUTILS_H

#include <cassert>
#include <cmath>

//#define SHOW_UNIT_TEST_RESULTS

#ifndef SHOW_UNIT_TEST_RESULTS
#define ASSERT_ALMOST_EQUAL( a, b, epsilon )\
   assert( std::fabs( (a) - (b) ) / std::fabs(b) < (epsilon) )
#else
#define ASSERT_ALMOST_EQUAL( a, b, epsilon )\
   std::cout << "ASSERT_ALMOST_EQUAL: " << __FILE__ << ":" << __LINE__ << ":\n"\
       << '\t' << #a << '\n' \
       << std::setprecision(20) << std::scientific \
       << "\tResult = " << (a) \
       << ", Expected = " << (b) \
       << ", Error = " << std::fabs( (a) - (b) ) / std::fabs(b) / (epsilon) << " epsilon\n" << std::endl ;
#endif

#endif

#ifndef GEOPHYSICS_UNITTESTUTILS_H
#define GEOPHYSICS_UNITTESTUTILS_H

#include <cassert>
#include <cmath>


#if 1
#define ASSERT_ALMOST_EQUAL( a, b, epsilon )\
   assert( std::fabs( (a) - (b) ) < (epsilon) )
#endif

#if 0
#define ASSERT_ALMOST_EQUAL( a, b, epsilon )\
   std::cout << "ASSERT_ALMOST_EQUAL: " << __FILE__ << ":" << __LINE__ << ":\n"\
       << '\t' << #a << '\n' \
       << std::setprecision(20) << std::scientific \
       << "\tResult = " << (a) \
       << ", Expected = " << (b) << '\n' << std::endl ;
#endif

#endif

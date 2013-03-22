#ifndef GEOPHYSICS_UNITTESTUTILS_H
#define GEOPHYSICS_UNITTESTUTILS_H

#include <cassert>
#include <cmath>

#define ASSERT_ALMOST_EQUAL( a, b, epsilon )\
   assert( std::fabs( (a) - (b) ) < (epsilon) )

//#define ASSERT_ALMOST_EQUAL( a, b, epsilon )\
//   std::cout << std::setprecision(20) << std::scientific \
//       << "Result = " << (a) \
//       << " Expected = " << (b) << std::endl ;

#endif

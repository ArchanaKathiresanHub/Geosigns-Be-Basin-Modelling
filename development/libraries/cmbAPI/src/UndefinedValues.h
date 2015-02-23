//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file UndefinedValues.h
/// @brief This file keeps set of macroses which define undevined numerical values

#ifndef CMB_UNDEFINED_VALUES
#define CMB_UNDEFINED_VALUES

#define UndefinedDoubleValue  -9999      ///< Undefined value for float point numbers
#define UndefinedIntegerValue -1         ///< Undefined value for integer numbers
#define UndefinedStringValue  "undef"    ///< Undefined value for strings

#include <cmath>

#ifdef _WIN32
#include <float.h>
#else
#include <values.h>
#endif

/// @brief Check is given value is equal to "undefined double value" 
/// @param val float point number to check
/// @return true if given value is exact as "undefined value", false otherwise
inline bool IsValueUndefined( double              val ) { return val == UndefinedDoubleValue; }

/// @brief Check is given value is equal to "undefined string value" 
/// @param val string to check
/// @return true if given value is exact as "undefined string value", false otherwise
inline bool IsValueUndefined( const std::string & val ) { return val == UndefinedStringValue; }

/// @brief Check is given value is equal to "undefined integer value" 
/// @param val integer value to check
/// @return true if given value is exact as "undefined integer value", false otherwise
inline bool IsValueUndefined( int                 val ) { return val == UndefinedIntegerValue; }

/// @brief Check if given values are the same with relative given tolerance: @f$ \frac{|v_1 - v_2|}{|v_1+v_2|} < \epsilon  @f$
/// @param val1 the first value to compare
/// @param val2 the second value to compare
/// @param eps relative tolerance
/// @return true if values relatively the same, false otherwise
inline bool NearlyEqual( double val1, double val2, double eps ) 
{
   if ( val1 == val2 ) { return true; } // shortcut, handles infinities

   double diff = std::fabs( val1 - val2 );

   // val1 or val2 is zero or both are extremely close to it, relative error is less meaningful here
   if ( val1 == 0 || val2 == 0 || diff < FLT_MIN ) { return diff < ( eps * FLT_MIN ); }

   // use relative error
   return diff < eps * ( std::fabs( val1 ) + std::fabs( val2 ) );
}
#endif // CMB_API

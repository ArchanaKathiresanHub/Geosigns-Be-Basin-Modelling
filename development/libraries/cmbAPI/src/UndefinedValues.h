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

#define UndefinedDoubleValue  -9999.0    ///< Undefined value for float point numbers
#define UndefinedIntegerValue -1         ///< Undefined value for integer numbers
#define UndefinedIDValue       65535     ///< Undefined value for size_t/ID type numbers
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

/// @brief Check is given value is equal to "undefined size_t/ID-type value" 
/// @param val integer value to check
/// @return true if given value is exact as "undefined ID value", false otherwise
inline bool IsValueUndefined( size_t              val ) { return val == UndefinedIDValue; }

#endif // CMB_API

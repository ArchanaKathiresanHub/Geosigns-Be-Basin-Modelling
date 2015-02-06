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

#define UndefinedDoubleValue  -9999      ///< Undefined value for floating point numbers
#define UndefinedIntegerValue -1         ///< Undefined value for integer numbers
#define UndefinedStringValue  "undef"    ///< Undefined value for strings

#include <cmath>

inline bool IsValueUndefined( double              val ) { return std::fabs( val - UndefinedDoubleValue ) < 1e-5; }
inline bool IsValueUndefined( const std::string & val ) { return val == UndefinedStringValue; }
inline bool IsValueUndefined( int                 val ) { return val == UndefinedIntegerValue; }

#endif // CMB_API

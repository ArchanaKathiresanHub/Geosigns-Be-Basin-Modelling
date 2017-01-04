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

#include "ConstantsNumerical.h"

#ifdef _WIN32
#include <float.h>
#else
#include <values.h>
#endif

/// @brief Do check if the given value is equal to one of the "no data values" 
/// @param val float point number to check
/// @return true if given value is exact as one of the "no data values", false otherwise
inline bool IsValueUndefined( double val )
{ 
   return val == Utilities::Numerical::IbsNoDataValue or
          val == Utilities::Numerical::CauldronNoDataValue;
}

/// @brief Do check is given value is equal to "no data integer value" 
/// @param val integerf value to check
/// @return true if given value is exact as "no data integer value", false otherwise
inline bool IsValueUndefined( int val ) { return val == Utilities::Numerical::NoDataIntValue; }

/// @brief Do check is given value is equal to "undefined string value" 
/// @param val string to check
/// @return true if given value is exact as "undefined string value", false otherwise
inline bool IsValueUndefined( const std::string & val ) { return val == Utilities::Numerical::NoDataStringValue; }

/// \brief Do check is given value is equal to "undefined size_t/ID-type value" 
/// \param val integer value to check
/// \return true if given value is exact as "undefined ID value", false otherwise
inline bool IsValueUndefined( size_t val ) { return val == Utilities::Numerical::NoDataIDValue; }

#endif // CMB_API

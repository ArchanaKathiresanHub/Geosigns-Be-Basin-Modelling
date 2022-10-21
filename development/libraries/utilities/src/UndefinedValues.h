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

#pragma once

#include "ConstantsNumerical.h"
#include <cmath>

namespace Utilities
{

/// @brief Do check if the given value is equal to one of the "no data values" 
/// @param val float point number to check
/// @return true if given value is exact as one of the "no data values", false otherwise
inline bool isValueUndefined( double val )
{ 
   using namespace Numerical;
   return std::abs(val - IbsNoDataValue) < DefaultNumericalTolerance ||
          std::abs(val - CauldronNoDataValue) < DefaultNumericalTolerance;
}

/// @brief Do check is given value is equal to "no data integer value" 
/// @param val integerf value to check
/// @return true if given value is exact as "no data integer value", false otherwise
inline bool isValueUndefined( int val )
{
   using namespace Numerical;
   return val == NoDataIntValue ||
         val == IbsNoDataValueInt ||
         val == CauldronNoDataValueInt;
}

/// @brief Do check is given value is equal to "undefined string value" 
/// @param val string to check
/// @return true if given value is exact as "undefined string value", false otherwise
inline bool isValueUndefined( const std::string & val ) { return val == Numerical::NoDataStringValue; }

/// \brief Do check is given value is equal to "undefined size_t/ID-type value" 
/// \param val integer value to check
/// \return true if given value is exact as "undefined ID value", false otherwise
inline bool isValueUndefined( size_t val ) { return val == Numerical::NoDataIDValue; }

}

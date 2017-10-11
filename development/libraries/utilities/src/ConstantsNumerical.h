//                                                                      
// Copyright (C) 2015-2017 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file ConstantsNumerical.h
/// @brief This file stores the numerical constants used by all IBS applications and libraries

#ifndef UTILITIES_CONSTANTSNUMERICAL_H
#define UTILITIES_CONSTANTSNUMERICAL_H

#include <limits>

namespace Utilities
{
   namespace Numerical {
      /// \brief Unsigned int no data value used accross all ibs libraries and applications
      constexpr unsigned int UnsignedIntNoDataValue = std::numeric_limits<unsigned int>::max();
      /// \brief Negative no data value used accross all ibs libraries and applications
      constexpr double IbsNoDataValue         = -9999;
      /// \brief Positive no data value used accross all ibs libraries and applications
      constexpr double CauldronNoDataValue    = 99999;
      /// \brief The maximum number of characters allowed for a command line options
      constexpr int    MaxLineSize            = 512;
      /// \brief Genex positive no data value used accross all ibs libraries and applications
      constexpr double GenexNoDataValue       = 9999.0;
      /// \brief No valid value for size_t (or CASA IDs) type numbers 
      constexpr size_t NoDataIDValue     = std::numeric_limits<size_t>::max();
      /// \brief Undefined value for strings
      const std::string NoDataStringValue = "undef";
      /// \brief No valid value for integer type numbers
      constexpr int     NoDataIntValue    = std::numeric_limits<int>::max();
   }
}
#endif

//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
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

namespace Utilities
{
   namespace Numerical {

      /// \brief Negative no data value used accross all ibs libraries and applications
      constexpr double IbsNoDataValue      = -9999;
      /// \brief Positive no data value used accross all ibs libraries and applications
      constexpr double CauldronNoDataValue = 99999;
      /// \brief The maximum number of characters allowed for a command line options
      constexpr int    MaxLineSize         = 512;
      /// \brief Genex positive no data value used accross all ibs libraries and applications
      constexpr double GenexNoDataValue    = 9999.0;
	  /// \brief Positive value to convert kilobytes to megabytes
	  constexpr double KiloBytesToMegaBytes = 1024;
   }
}
#endif

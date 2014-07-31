//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file CurveRange.h
/// @brief This file keeps declaration of the class for handling range for 1D curve profile

#ifndef CASA_API_CURVE_RANGE_H
#define CASA_API_CURVE_RANGE_H

#include "Range.h"

/// @page CASA_CurveRangePage 1D curve profile range
/// 
/// @link casa::CurveRange 1D curve profile range @endlink is used for variable parameter which value can be 1D curve profile
/// For variation of 1D curve profiles the linear interpolation between profiles is used. 
/// User should specify curve profiles for minimal and maximal range values. This range is mapped to 
/// [-1:1] range for easy integration with DoE algorithms. On variation request in range [-1:1] 
/// CurveRange will linear interpolate between two 1D curve profiles:
///
/// @image html Curve1DRange.png "1D function variation through linear interpolation between min/max 1D functions"
namespace casa
{
/// @brief Base class for continuous range of 1D curve profile. It allows to interpolate between two\n
   ///        1D curve profiles corresponded to min and max values
   class CurveRange : public Range
   {
   public:
   protected:
      CurveRange() {;}
      virtual ~CurveRange() {;}
   };
}

#endif // CASA_API_PARAMETER_H
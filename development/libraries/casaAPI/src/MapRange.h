//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file MapRange.h
/// @brief This file keeps declaration of the class for handling range for 2D map

#ifndef CASA_API_MAP_RANGE_H
#define CASA_API_MAP_RANGE_H

#include "Range.h"

/// @page CASA_MapRangePage 2D Map Range
/// 
/// @link casa::MapRange 2D Map range @endlink is used for variable parameter which value can be 2D map
/// For variation of 2D maps the linear interpolation between 2 maps is be used. 
/// User should specify 2 maps for minimal and maximal range values. This range will be mapped 
/// to [-1:1] range for easy integration with DoE algorithms. On variation request in range [-1:1] 
/// MapRange will linear interpolate between two maps:
///
/// @image html MapRange0.png "-1.0 (left range boundary value)"
/// @image html MapRange1.png "0.3 range value"
/// @image html MapRange2.png "0.7 range value"
/// @image html MapRange3.png "1.0 (right range boundary value)"



namespace casa
{
   /// @brief Base class for continuous range of 2D map. It allows to interpolate between two\n
   ///        2D maps corresponded to min and max values
   class MapRange : public Range
   {
   public:
   protected:
      MapRange() {;}
      virtual ~MapRange() {;}
   };
}

#endif // CASA_API_PARAMETER_H
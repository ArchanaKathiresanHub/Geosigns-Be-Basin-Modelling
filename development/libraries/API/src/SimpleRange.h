//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file SimpleRange.h
/// @brief This file keeps declaration of the class for handling range of float continuous variable parameter

#ifndef CASA_API_SIMPLE_RANGE_H
#define CASA_API_SIMPLE_RANGE_H

#include "Range.h"

/// @page CASA_SimpleRangePage Simple range variable parameter
/// 
/// @link casa::SimpleRange Simple range @endlink is used for variable parameter which value can be one double value
/// SimpleRange defines minimal and maximal double values for the parameter

namespace casa
{
   /// @brief Class for handling continuous variable parameter range represented by min and max float values
   class SimpleRange : public Range
   {
   public:
      SimpleRange( double minVal, double maxVal ) : m_min( minVal ), m_max( maxVal ) {;}
      virtual ~SimpleRange() {;}

      virtual double minRangeValueAsDouble() const { return m_min; }
 
      virtual double maxRangeValueAsDouble() const { return m_max; }

      virtual bool isValInRange( double val ) const { return (val >= m_min && val <= m_max); }

   protected:
      double m_min; ///< minimal (left bound) value of the range
      double m_max; ///< maximal (right bound) value of the range
   };
}

#endif // CASA_API_SIMPLE_RANGE_H

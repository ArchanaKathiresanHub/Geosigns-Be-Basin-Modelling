//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file Range.h
/// @brief This file keeps declaration of top level class for handling variable parameter range


#ifndef CASA_API_RANGE_H
#define CASA_API_RANGE_H

#include <vector>

namespace casa
{
   /// @brief Base class handling ranges of variable continuous parameters
   class Range
   {
   public:
      /// @brief Range destructor
      virtual ~Range() {;}
       
      /// @brief Get minimal range value as double
      /// @return left (min) range boundary value as double
      virtual std::vector<double> minRangeValue() const = 0;
 
      /// @brief Get maximal range value as double
      /// @return right (max) range boundary value as double
      virtual std::vector<double> maxRangeValue() const = 0;
      
      /// @brief Check if given value inside the range
      /// @param val value for checking
      /// @return true if given value between range boundaries, false otherwise
      virtual bool isValInRange( const std::vector<double> & val ) const = 0;

   protected:
      Range() {;}
   };
}
#endif // CASA_API_RANGE_H

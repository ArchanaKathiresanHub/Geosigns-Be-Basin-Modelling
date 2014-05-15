//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file VarParameter.h
/// @brief This file keeps base class declaration for handling variable parameters. 

#ifndef CASA_API_VARIABLE_PARAMETER_H
#define CASA_API_VARIABLE_PARAMETER_H

#include <memory>

/// @page CASA_VarParameterPage Variable parameter
///
/// Variable parameter - parameter which value can be in varied in some range.
/// Variable parameter could be a @link CASA_ContinuousParameterPage continuous parameter @endlink,
/// or @link CASA_CategoricalParameterPage categorical parameter @endlink 

namespace casa
{
   /// @brief 
   class VarParameter
   {
   public:
      /// @brief Copy operator
      /// @param otherPrm ContinuousParameter object to be copied
      /// @return reference to this object 
      virtual VarParameter & operator = ( const VarParameter & otherPrm ) = 0;
      
      /// @brief get minimal range value
      /// @return left (minimal) range boundary value
      virtual const Parameter & minimalValue() const = 0;

      /// @brief get maximal range value
      /// @return right (maximal) range boundary value
      virtual const Parameter & maximalValue() const = 0;

   protected:
      VarParameter() {;}
      virtual ~VarParameter() {;}
   };

}

#endif // CASA_API_VARIABLE_PARAMETER_H

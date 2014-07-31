//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file VarPrmCategorical.h
/// @brief This file keeps API declaration for handling categorical parameters. 

#ifndef CASA_API_VAR_PRM_CATEGORICAL_H
#define CASA_API_VAR_PRM_CATEGORICAL_H

#include "Parameter.h"
#include "Enumeration.h"

#include <memory>
#include <vector>

/// @page CASA_VarPrmCategoricalPage Variable parameter for categorical values
///
/// Categorical parameter - a parameter that can take some unordered discrete values each of which referring to a
/// different category (for instance, a PVT parameter that can distinguish between a few, non related PVT models)

namespace casa
{

   /// @brief Class to manage categorical variable parameter type
   class VarPrmCategorical
   {
   public:
      /// @brief Destructor
      virtual ~VarPrmCategorical() {;}

      /// @brief Get list of categorical parameter values as sorted list of unsigned integers
      virtual std::vector< unsigned int> valuesAsUnsignedIntSortedSet() const = 0;

      /// @brief Create a copy of the parameter and assign to the given value. If value is not in var. parameter values set,\n
      ///        the method will return a zero pointer
      /// @param val new value for parameter
      /// @return the new parameter object which should be deleted by the caller itself
      virtual Parameter * createNewParameterFromUnsignedInt( unsigned int val ) const = 0;

   protected:
      VarPrmCategorical() {;}

      std::auto_ptr<Enumeration> m_valueSet;
   };

}

#endif // CASA_API_VAR_PRM_CATEGORICAL_H
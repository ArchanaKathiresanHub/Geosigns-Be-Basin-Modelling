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
/// @brief This file keeps API declaration for handling variable parameters. 

#ifndef CASA_API_VAR_PARAMETER_H
#define CASA_API_VAR_PARAMETER_H

/// @page CASA_VarParameterPage Variable parameter
///  
/// For the given variable parameter a DoE algorithm generates several fixed values of this parameter 
/// when generating Cases set
/// 
/// CASA API supports the following types of variable parameters:
/// - @subpage CASA_VarPrmContinuousPage
/// - @subpage CASA_VarPrmDiscretePage
/// - @subpage CASA_VarPrmCategoricalPage
///
/// Set of variable parameters for scenario analysis is managed by a @ref CASA_VarSpacePage "VarSpace manager"

// CASA
#include "CasaSerializer.h"
#include "Parameter.h"

// STL
#include <vector>

namespace casa
{
   /// @brief Base class for variable parameters. It keeps the common part of any variable parameter 
   class VarParameter : public CasaSerializable
   {
   public:
      /// @brief Defines possible types of parameter variation
      enum Type
      {
         Continuous,  ///< A parameter that can take any value between certain bounds (for instance, a fault-sealing factor that varies in the [0,1] range)
         Discrete,    ///< A parameter that can take ordered discrete values between certain bounds (for instance, a diameter of a pipe, available in some pre-defined sizes). 
         Categorical  ///< A parameter that can take some unordered discrete values each of which referring to a different category
                      /// (for instance, a PVT parameter that can distinguish between a few, unrelated PVT models) 
      };

      /// @brief Destructor
      virtual ~VarParameter() {;}

      /// @brief Get name of variable parameter in short form
      /// @return array of names for each subparameter
      virtual std::vector<std::string> name() const = 0;

      /// @brief Get number of subparameters if it is more than one
      /// @return dimension of variable parameter
      virtual size_t dimension() const = 0;

      /// @brief Get type of variable parameter
      /// @return type of parameter variation
      virtual Type variationType() const = 0;

      /// @brief A parameter which corresponds the base value of the variable parameter 
      /// @return the parameter object which should not be deleted by a caller
      virtual const SharedParameterPtr baseValue() const = 0;

      /// @brief A parameter which corresponds the minimal range value of the variable parameter 
      /// @return the parameter object which should not be deleted by a caller
      virtual const SharedParameterPtr minValue() const = 0;

      /// @brief A parameter which corresponds the maximal range value of the variable parameter 
      /// @return the parameter object should be deleted by a caller
      virtual const SharedParameterPtr maxValue() const = 0;

      /// @brief Create parameter by reading the values stored in the project file
      /// @param[in, out] mdl the model where the parameters values should be read
      /// @param[in] an input vector (e.g. spatial/temporal) coordinates 
      /// @return the new parameter read from the model
      virtual SharedParameterPtr newParameterFromModel( mbapi::Model & mdl, const std::vector<double> & vin ) const = 0;

   protected:
      // version of serialized object representation
      virtual unsigned int version() const = 0;
   
      std::string       m_name;  ///< user specified parameter name
  };
}
#endif // CASA_API_VAR_PARAMETER_H



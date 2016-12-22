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
/// @brief This file keeps API declaration for handling influential parameters. 

#ifndef CASA_API_VAR_PARAMETER_H
#define CASA_API_VAR_PARAMETER_H

/// @page CASA_VarParameterPage Influential parameter
///  
/// The influential parameter is a parameter which the exact value is unknown for the current scenario.
/// User has some suggestions about it uncertainty by providing to scenario the min/max/most probable values 
/// for this parameter. If there is no most probable parameter value, it could be chosen as a middle value of the range.
///
/// For the given influential parameter a DoE algorithm generates several fixed values of this parameter/
///
/// Each influential parameter has a probability distribution function (PDF) which is used by casa::MonteCarloSolver. 
/// The following types of PDF are supported:
///
/// - "Block" - uniform probability distribution inside min/max value range.
///   @image html "Uniform_Distribution_PDF.png"
/// - "Triangle" - a continuous probability distribution with lower parameter range value @f$ a @f$, upper parameter range 
/// value @f$ b  @f$ and most probable parameter value @f$ c @f$, where @f$ a < b @f$ and @f$ a \leq c \leq b @f$
///   @image html "Triangular_distribution_PDF.png"
/// - "Normal" - the normal (or Gaussian) distribution: 
///   @f$ f(x,\mu,\sigma) = \frac{1}{{\sigma \sqrt {2\pi } }}e^{{{ - \left( {x - \mu } \right)^2 } \mathord{\left/ {\vphantom {{ - \left( {x - \mu } \right)^2 } {2\sigma ^2 }}} \right. \kern-\nulldelimiterspace} {2\sigma ^2 }}} @f$
///   where @f$ \mu @f$ is the most probable parameter value and @f$ \sigma = \frac{max - min}{\sqrt{12}} @f$. @image html "Normal_Distribution_PDF.png"
///
/// 
/// CASA API supports the following types of influential parameters:
/// - @subpage CASA_VarPrmContinuousPage
/// - @subpage CASA_VarPrmDiscretePage (Note: No one is implemented yet for this IP type)
/// - @subpage CASA_VarPrmCategoricalPage
///
/// The set of influential parameters for the scenario analysis object is managed by a @ref CASA_VarSpacePage "VarSpace manager"

// CASA
#include "CasaSerializer.h"
#include "Parameter.h"

// STL
#include <vector>

namespace casa
{
   /// @brief Base class for influential parameters. It keeps the common part of any influential parameter 
   class VarParameter : public CasaSerializable
   {
   public:
      /// @brief Defines possible types of parameter variation
      enum Type
      {
         Continuous,  ///< A parameter that can take any value between certain bounds (for instance, a fault-sealing factor that varies in the [0,1] range)
         Discrete,    ///< A parameter that can take ordered discrete values between certain bounds (for instance, a diameter of a pipe, available in some predefined sizes)
         Categorical  ///< A parameter that can take some unordered discrete values each of which referring to a different category
                      /// (for instance, a PVT parameter that can distinguish between a few, unrelated PVT models) 
      };

      /// @brief Destructor
      virtual ~VarParameter() {;}

      /// @brief Get name of influential parameter in short form
      /// @return array of names for each subparameter
      virtual std::vector<std::string> name() const = 0;

      /// @brief Get number of subparameters if it is more than one
      /// @return dimension of influential parameter
      virtual size_t dimension() const = 0;

      /// @brief Get type of influential parameter
      /// @return type of parameter variation
      virtual Type variationType() const = 0;

      /// @brief A parameter which corresponds the base value of the influential parameter 
      /// @return the parameter object which should not be deleted by a caller
      virtual const SharedParameterPtr baseValue() const = 0;

      /// @brief A parameter which corresponds the minimal range value of the influential parameter 
      /// @return the parameter object which should not be deleted by a caller
      virtual const SharedParameterPtr minValue() const = 0;

      /// @brief A parameter which corresponds the maximal range value of the influential parameter 
      /// @return the parameter object should be deleted by a caller
      virtual const SharedParameterPtr maxValue() const = 0;

      /// @brief Create parameter by reading the values stored in the project file
      /// @param[in, out] mdl the model where the parameters values should be read
      /// @param[in] vin an input vector with parameter cpecific values if they needed to extract parameter value from the model
      /// @return the new parameter read from the model
      virtual SharedParameterPtr newParameterFromModel( mbapi::Model & mdl, const std::vector<double> & vin ) const = 0;

   protected:
      // version of serialized object representation
      virtual unsigned int version() const = 0;
   
      std::string       m_name;  ///< user specified parameter name
  };
}
#endif // CASA_API_VAR_PARAMETER_H



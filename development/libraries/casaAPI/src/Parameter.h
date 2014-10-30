//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file Parameter.h
/// @brief This file keeps declaration of top level class for handling Cauldron model parameters

#ifndef CASA_API_PARAMETER_H
#define CASA_API_PARAMETER_H

#include "ErrorHandler.h"

#include <vector>
#include <boost/shared_ptr.hpp>

/// @page CASA_ParameterPage Cauldron project parameter
/// Parameter is some value (or possibly set of values) in project file which has an influence on simulation.
/// Any change of the parameter value, will change in some way the simulation results.
///

namespace mbapi
{
   class Model;
}

namespace casa
{
   class VarParameter;

   /// @brief Base class for all types of Cauldron model parameters used in CASA
   class Parameter
   {
   public:
      /// @brief Parameter destructor
      virtual ~Parameter() {;}
      
      /// @brief Get name of the parameter
      /// @return parameter name
      virtual const char * name() const = 0;

      /// @brief Get variable parameter which was used to create this parameter
      /// @return Pointer to the variable parameter
      virtual const VarParameter * parent() const = 0;

      /// @brief Set this parameter value in Cauldron model
      /// @param caldModel reference to Cauldron model
      /// @return ErrorHandler::NoError in success, or error code otherwise
      virtual ErrorHandler::ReturnCode setInModel( mbapi::Model & caldModel ) = 0;
      
      /// @brief Validate parameter value if it is in range of allowed values
      /// @param caldModel reference to Cauldron model
      /// @return empty string on success, or error message if validation fail
      virtual std::string validate( mbapi::Model & caldModel ) = 0;

      // The following methods are used for converting between CASA RunCase and SUMLib::Case objects

      /// @brief Get parameter value as an array of doubles
      /// @return parameter value represented as set of doubles
      ///
      /// @pre parent()->variationType() must returns VarParameter::Continuous
      /// @post return set of doubles which represent value of this parameter if parameter is continuous
      ///       or empty array otherwise
      virtual std::vector<double> asDoubleArray() const = 0;

      /// @brief Get parameter value as integer
      /// @return parameter value represented as integer
      ///
      /// @pre parent()->variationType() must returns VarParameter::Categorical
      /// @post return one non negative integer number which represents a value of this parameter if
      ///       parameter is categorical, or -1 otherwise
      virtual int asInteger() const = 0;

   protected:
      Parameter() {;}

   private:
      Parameter( const Parameter & );
      Parameter & operator = ( const Parameter & );
   };
}

typedef boost::shared_ptr<casa::Parameter> SharedParameterPtr;

#endif // CASA_API_PARAMETER_H

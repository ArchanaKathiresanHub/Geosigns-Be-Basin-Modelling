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

namespace mbapi
{
   class Model;
}

/// @page CASA_VarParameterPage Variable Parameters
/// 
/// For Variable parameter a DoE algorithm generates several fixed values of this Parameter when generating
/// a Case set for building a response surface proxy.
/// 
/// CASA API supports the following types of variable parameters:
/// - \subpage CASA_VarPrmContinuousPage
/// - \subpage CASA_VarPrmCategoricalPage
///
/// Set of variable parameters for scenario analysis is managed by a @ref CASA_VarSpacePage "VarSpace manager"

namespace casa
{
   /// @brief Base class for all types of Cauldron model parameters used in CASA
   class Parameter
   {
   public:
      /// @brief Parameter destructor
      virtual ~Parameter() {;}
      
      /// @brief Set this parameter value in Cauldron model
      /// @param caldModel reference to Cauldron model
      /// @return ErrorHandler::NoError in success, or error code otherwise
      virtual ErrorHandler::ReturnCode setInModel( mbapi::Model & caldModel ) = 0;
      
      // The following methods are used for testing  
      virtual bool isDouble() = 0;
      virtual double doubleValue() = 0;

   protected:
      Parameter() {;}
   };
}

#endif // CASA_API_PARAMETER_H
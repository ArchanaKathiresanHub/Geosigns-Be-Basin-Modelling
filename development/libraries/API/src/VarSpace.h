//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file VarSpace.h
/// @brief This file keeps API declaration for variable parameters set manager

#ifndef CASA_API_VAR_SPACE_H
#define CASA_API_VAR_SPACE_H

#include "ErrorHandler.h"

/// @page CASA_VarSpacePage Variable parameters set manager
/// @link casa::VarSpace A collection of variable parameters @endlink defined for scenario analysis 

namespace casa
{
   class VarPrmCategorical;
   class VarPrmContinuous;

   /// @class VarSpace VarSpace.h "VarSpace.h"
   /// @brief Variable parameters set manager. It keeps a unique set of variable parameters
   class VarSpace : public ErrorHandler
   {
   public:
      /// @brief Add a new categorical parameter
      /// @param prm a new parameter to be added to the set
      /// @return NoError on success or error code if such parameter is already added to the set
      virtual ErrorHandler::ReturnCode addParameter( VarPrmCategorical * prm ) = 0;

      /// @brief Add a new continuous parameter
      /// @param prm a new parameter to be added to the set
      /// @return NoError on success or error code if such parameter is already added to the set
      virtual ErrorHandler::ReturnCode addParameter( VarPrmContinuous * prm ) = 0;

      /// @brief Get number of variable parameters defined in VarSpace
      /// @return total number of parameters in set
      virtual size_t size() const = 0;

      /// @brief Get number of continuous parameters defined in VarSpace
      /// @return number of continuous parameters
      virtual size_t numberOfContPrms() const = 0;

      /// @brief Get number of categorical parameters defined in VarSpace
      /// @return number of categorical parameters
      virtual size_t numberOfCategPrms() const = 0;

   protected:
      VarSpace() {;}
      virtual ~VarSpace() {;}

   };
}

#endif // CASA_API_VAR_SPACE_H

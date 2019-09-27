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
/// @brief This file keeps API declaration for influential parameters set manager

#ifndef CASA_API_VAR_SPACE_H
#define CASA_API_VAR_SPACE_H

#include "ErrorHandler.h"
#include "CasaSerializer.h"

/// @page CASA_VarSpacePage Container for influential parameters
/// @link casa::VarSpace A collection of influential parameters @endlink defined for scenario analysis

namespace casa
{
   class VarParameter;
   class VarPrmCategorical;
   class VarPrmContinuous;
   class VarPrmDiscrete;

   /// @class VarSpace VarSpace.h "VarSpace.h"
   /// @brief Influential parameters set manager. It keeps a unique set of influential parameters
   class VarSpace : public ErrorHandler, public CasaSerializable
   {
   public:
      /// @brief Add a new influential parameter
      /// @param prm a new parameter to be added to the set
      /// @return NoError on success or error code if such parameter is already added to the set
      virtual ErrorHandler::ReturnCode addParameter( VarParameter * prm ) = 0;

      /// @brief Get number of influential parameters defined in VarSpace. Some parameters could have
      ///        dimension more then 1
      /// @return total number of parameters in set
      virtual size_t size() const = 0;

      /// @brief Dimension of parameter space
      /// @return total parameters space dimension
      virtual size_t dimension() const = 0;

      /// @brief Get number of continuous parameters defined in VarSpace
      /// @return number of continuous parameters
      virtual size_t numberOfContPrms() const = 0;

      /// @brief Get number of discrete parameters defined in VarSpace
      /// @return number of discrete parameters
      virtual size_t numberOfDiscrPrms() const = 0;

      /// @brief Get number of categorical parameters defined in VarSpace
      /// @return number of categorical parameters
      virtual size_t numberOfCategPrms() const = 0;

      /// @brief Get i-th parameter (numeration is first continuous, then discrete and then categorical)
      /// @param i parameter number
      /// @return pointer to the requested parameter on success or zero pointer otherwise
      virtual const VarParameter * parameter( size_t i ) const = 0;

      /// @brief Get i-th parameter (numeration is first continuous, then discrete and then categorical)
      /// @param i parameter number
      /// @return pointer to the requested parameter on success or zero pointer otherwise
      virtual VarParameter * operator[] ( size_t i ) const = 0;

      /// @brief Get i-th continuous parameter from the list
      /// @param i continuous parameter number
      /// @return pointer to the requested continuous parameter on success or zero pointer otherwise
      virtual const VarPrmContinuous * continuousParameter( size_t i ) const = 0;

      /// @brief Get i-th discrete parameter from the list
      /// @param i discrete parameter number
      /// @return pointer to the requested discrete parameter on success or zero pointer otherwise
      virtual const VarPrmDiscrete * discreteParameter( size_t i ) const = 0;

      /// @brief Get i-th categorical parameter from the list
      /// @param i categorical parameter number
      /// @return pointer to the requested categorical parameter on success or zero pointer otherwise
      virtual const VarPrmCategorical * categoricalParameter( size_t i ) const = 0;

      virtual ~VarSpace() { ; }
   protected:
      VarSpace() {;}
   };
}

#endif // CASA_API_VAR_SPACE_H

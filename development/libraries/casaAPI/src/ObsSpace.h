//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file ObsSpace.h
/// @brief This file keeps API declaration for observables set manager

#ifndef CASA_OBS_VAR_SPACE_H
#define CASA_OBS_VAR_SPACE_H

#include "ErrorHandler.h"
#include "Observable.h"

#include <vector>

/// @page CASA_ObsSpacePage Container for observables description
/// @link casa::ObsSpace A collection of observables @endlink defined for scenario analysis 
/// @link casa::ObsSpace @endlink also provides a set of API functions to create various types of @link casa::Observable Observable @endlink objects.
namespace casa
{
   /// @class ObsSpace ObsSpace.h "ObsSpace.h"
   /// @brief Observables set manager. It keeps a set of observables
   class ObsSpace : public ErrorHandler
   {
   public:
      /// @brief Add a new observable
      /// @param obs a new observable to be added to the set
      /// @return NoError on success or error code if such parameter is already added to the set
      virtual ErrorHandler::ReturnCode addObservable( Observable * obs ) = 0;

      /// @brief Get number of observables defined in ObsSpace
      /// @return total number of observables in set
      virtual size_t size() const = 0;

      /// @brief Get i-th observable
      /// @param i observable number
      /// @return i-th observable pointer on success, 0 pointer otherwise
      virtual const Observable * observable( size_t i ) const = 0;

   protected:
      ObsSpace() {;}
      virtual ~ObsSpace() {;}
   };
}

#endif // CASA_API_OBS_SPACE_H

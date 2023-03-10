//
// Copyright (C) 2012-2016 Shell International Exploration & Production.
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

#include "CasaSerializer.h"
#include "ErrorHandler.h"
#include "Observable.h"

// STL
#include <vector>

/// @page CASA_ObsSpacePage Container for observables description
/// @link casa::ObsSpace A collection of observables @endlink defined for scenario analysis
/// @link casa::ObsSpace @endlink also provides a set of API functions to create various types of @link casa::Observable Observable @endlink objects.
namespace casa
{
   /// @class ObsSpace ObsSpace.h "ObsSpace.h"
   /// @brief Observables set manager. It keeps a set of observables
   class ObsSpace : public ErrorHandler, public CasaSerializable
   {
   public:
      /// @brief Add a new observable
      /// @param obs a new observable to be added to the set
      /// @return NoError on success or error code if such parameter is already added to the set
      virtual ErrorHandler::ReturnCode addObservable( Observable * obs ) = 0;

      /// @brief Get number of observables defined in ObsSpace
      /// @return total number of observables in set
      virtual size_t size() const = 0;

      /// @brief Dimension of observables space
      /// @return total observables space dimension
      virtual size_t dimension() const = 0;

      /// @brief Get i-th observable
      /// @param i observable number (starting from 0)
      /// @return i-th observable pointer on success, 0 pointer otherwise
      virtual Observable * operator[] ( size_t i ) const = 0;

      /// @brief Get i-th observable
      /// @param i observable number (starting from 0)
      /// @return i-th observable pointer on success, 0 pointer otherwise
      virtual const Observable * observable( size_t i ) const = 0;

      /// @brief Is the given observable valid at least for one case?
      /// @param obId observable id
      /// @param obSubId observable sub id for observables with dimension more than 1
      /// @return true if requested observable value is valid at least for one run case
      virtual bool isValid( size_t obId, size_t obSubId ) const = 0;

      /// @brief Add observable validity status. This function also is called for each run
      ///        case on the stage of extracting observables in data digger.
      /// @param ob observable number in observables space
      /// @param valFlags list the same size as transformed observable dimension with valid/invalid flags
      virtual void updateObsValueValidateStatus( size_t ob, const std::vector<bool> & valFlags ) = 0;

      virtual ~ObsSpace() { ; }
   protected:
      ObsSpace() { ; }

   private:
   };
}

#endif // CASA_API_OBS_SPACE_H

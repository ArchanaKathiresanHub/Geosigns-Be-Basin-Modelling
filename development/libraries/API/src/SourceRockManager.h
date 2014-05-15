//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file SourceRockManager.h
/// @brief This file keeps API declaration for manipulating source rocks in Cauldron model

#ifndef CMB_SOURCE_ROCK_MANAGER_API
#define CMB_SOURCE_ROCK_MANAGER_API

#include <vector>
#include "ErrorHandler.h"

/// @page SourceRockManagerPage Source Rock Manager
/// @link mbapi::SourceRockManager Source rock manager @endlink provides set of interfaces to create/delete/edit
/// list various source rocks in the data model. Also it has set of interfaces to get/set property of any source
/// rock from the list

namespace mbapi {
   /// @class SourceRockManager SourceRockManager.h "SourceRockManager.h"
   /// @brief Class SourceRockManager keeps a list of source rocks in Cauldron model and allows to add/delete/edit source rock
   class SourceRockManager : public ErrorHandler
   {
   public:
      
      /// @{
      /// Set of interfaces for interacting with a Cauldron model

      typedef size_t SourceRockID;  ///< unique ID for source rock

      /// @brief Get list of source rocks in the model
      /// @return array with IDs of different source rocks defined in the model
      virtual std::vector<SourceRockID> getSourceRocksID() const = 0; 

      /// @brief Create new source rock
      /// @return ID of the new SourceRock
      virtual SourceRockID createNewSourceRock() = 0;

      /// @brief Get source rock name for
      /// @param[in] id source rock ID
      /// @param[out] srName on success has a source rock name, or empty string otherwise
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode getSourceRockName( SourceRockID id, std::string & srName ) const = 0;
      /// @}


   protected:
      /// @name Constructors/destructor
      /// @{
      /// @brief Constructor which creates an empty model
      SourceRockManager() {;}
      
      /// @brief Destructor, no any actual work is needed here, all is done in the implementation part
      virtual ~SourceRockManager() {;}

   private:
      /// @{
      /// Copy constructor and operator are disabled, use the copyTo method instead
      SourceRockManager( const SourceRockManager & otherSourceRockManager );
      SourceRockManager & operator = ( const SourceRockManager & otherSourceRockManager );
      /// @}
   };
}

#endif // CMB_SOURCE_ROCK_MANAGER_API

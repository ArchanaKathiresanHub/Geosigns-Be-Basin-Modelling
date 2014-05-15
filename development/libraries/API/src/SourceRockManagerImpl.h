//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file SourceRockManagerImpl.h
/// @brief This file keeps API implementation for manipulating source rocks in Cauldron model

#ifndef CMB_SOURCE_ROCK_MANAGER_IMPL_API
#define CMB_SOURCE_ROCK_MANAGER_IMPL_API

#include "SourceRockManager.h"

namespace mbapi {

   // Class SourceRockManager keeps a list of source rocks in Cauldron model and allows to add/delete/edit source rock
   class SourceRockManagerImpl : public SourceRockManager
   {
   public:     
      // Constructors/destructor
      // brief Constructor which creates an SourceRockManager
      SourceRockManagerImpl();
      
      // Destructor
      virtual ~SourceRockManagerImpl() {;}

      // Copy operator
      SourceRockManagerImpl & operator = ( const SourceRockManagerImpl & otherSrRockMgr );

      // Set of interfaces for interacting with a Cauldron model

      // Get list of source rocks in the model
      // return array with IDs of different lythologies defined in the model
      virtual std::vector<SourceRockID> getSourceRocksID() const; 

      // Create new source rock
      // return ID of the new SourceRock
      virtual SourceRockID createNewSourceRock();

      // Get source rock name for
      // [in] id source rock ID
      // [out] srName on success has a source rock name, or empty string otherwise
      // return NoError on success or NonexistingID on error
      virtual ReturnCode getSourceRockName( SourceRockID id, std::string & srName ) const;
      
   private:
      // Copy constructor is disabled, use the copy operator instead
      SourceRockManagerImpl( const SourceRockManager & );
   };
}

#endif // CMB_SOURCE_ROCK_MANAGER_IMPL_API
//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file SourceRockManagerImpl.C
/// @brief This file keeps API implementation for manipulating source rocks in Cauldron model

#include "SourceRockManagerImpl.h"

#include <exception>
#include <string>

namespace mbapi
{

// Constructor
SourceRockManagerImpl::SourceRockManagerImpl()
{
   //throw std::runtime_error( "Not implemented yet" );
}

// Copy operator
SourceRockManagerImpl & SourceRockManagerImpl::operator = ( const SourceRockManagerImpl & otherSrRockMgr )
{
   throw std::runtime_error( "Not implemented yet" );
   return *this;
}

// Get list of source rocks in the model
// return array with IDs of different source rocks defined in the model
std::vector<SourceRockManager::SourceRockID> SourceRockManagerImpl::getSourceRocksID() const
{
   throw std::runtime_error( "Not implemented yet" );
   return std::vector<SourceRockID>();
}

// Create new source rock
// return ID of the new SourceRock
SourceRockManager::SourceRockID SourceRockManagerImpl::createNewSourceRock()
{
   throw std::runtime_error( "Not implemented yet" );
}

// Get source rock name for
// [in] id source rock ID
// [out] srName on success has a source rock name, or empty string otherwise
// return NoError on success or NonexistingID on error
ErrorHandler::ReturnCode SourceRockManagerImpl::getSourceRockName( SourceRockID id, std::string & srName ) const
{
   throw std::runtime_error( "Not implemented yet" );
   return NotImplementedAPI;
}

}
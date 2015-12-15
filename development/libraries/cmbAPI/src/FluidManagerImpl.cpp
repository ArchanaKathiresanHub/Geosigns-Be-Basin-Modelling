//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file FluidManagerImpl.C
/// @brief This file keeps API implementation for manipulating fluids in Cauldron model

#include "FluidManagerImpl.h"

#include <stdexcept>
#include <string>

namespace mbapi
{

// Constructor
FluidManagerImpl::FluidManagerImpl()
{
   //throw std::runtime_error( "Not implemented yet" );
}

// Copy operator
FluidManagerImpl & FluidManagerImpl::operator = ( const FluidManagerImpl & /*otherFluidMgr*/ )
{
   throw std::runtime_error( "Not implemented yet" );
   return *this;
}

// Get list of fluids in the model
// return array with IDs of different fluids defined in the model
std::vector<FluidManager::FluidID> FluidManagerImpl::getFluidsID() const
{
   throw std::runtime_error( "Not implemented yet" );
   return std::vector<FluidID>();
}

// Create new fluid
// return ID of the new Fluid
FluidManager::FluidID FluidManagerImpl::createNewFluid()
{
   throw std::runtime_error( "Not implemented yet" );
}

// Get fluid name for
// [in] id fluid ID
// [out] fluidName on succes has a fluid name, or empty string otherwise
// return NoError on success or NonexistingID on error
ErrorHandler::ReturnCode FluidManagerImpl::getFluidName( FluidID /*id*/, std::string & /*fluidName*/ ) const
{
   throw std::runtime_error( "Not implemented yet" );
   return NotImplementedAPI;
}

}
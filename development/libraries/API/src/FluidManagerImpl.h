//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file FluidManagerImpl.h
/// @brief This file keeps API implementation for manipulating Fluids in Cauldron model

#ifndef CMB_FLUID_MANAGER_IMPL_API
#define CMB_FLUID_MANAGER_IMPL_API

#include <memory>

#include "FluidManager.h"

namespace mbapi {

   // Class FluidManager keeps a list of fluids in Cauldron model and allows to add/delete/edit fluid
   class FluidManagerImpl : public FluidManager
   {
   public:     
      // Constructors/destructor
      // brief Constructor which creates an FluidManager
      FluidManagerImpl();
      
      // Destructor
      virtual ~FluidManagerImpl() {;}

      // Copy operator
      FluidManagerImpl & operator = ( const FluidManagerImpl & otherFluidMgr );

      // Set of interfaces for interacting with a Cauldron model

      // Get list of fluids in the model
      // return array with IDs of different lygthologies defined in the model
      virtual std::vector<FluidID> getFluidsID() const; 

      // Create new fluid
      // return ID of the new Fluid
      virtual FluidID createNewFluid();

      // Get fluid name for
      // [in] id fluid ID
      // [out] fluidName on success has a fluid name, or empty string otherwise
      // return NoError on success or NonexistingID on error
      virtual ReturnCode getFluidName( FluidID id, std::string & fluidName ) const;
      
   private:
      // Copy constructor is disabled, use the copy operator instead
      FluidManagerImpl( const FluidManager & );
   };
}

#endif // CMB_FLUID_MANAGER_IMPL_API
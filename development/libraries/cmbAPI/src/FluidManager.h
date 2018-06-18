//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file FluidManager.h
/// @brief This file keeps API declaration for manipulating fluids in Cauldron model

#ifndef CMB_FLUID_MANAGER_API
#define CMB_FLUID_MANAGER_API

#include <vector>
#include "ErrorHandler.h"

/// @page FluidManagerPage Fluid Manager
/// @link mbapi::FluidManager Fluid manager @endlink provides set of interfaces to create/delete/edit list fluids
/// in the data model. Also it has set of interfaces to get/set property of any fluid from the list
///

namespace mbapi {
   /// @class FluidManager FluidManager.h "FluidManager.h" 
   /// @brief Class FluidManager keeps a list of fluids in Cauldron model and allows to add/delete/edit fluid
   class FluidManager : public ErrorHandler
   {
   public:
      
      /// @{
      /// Set of interfaces for interacting with a Cauldron model
      typedef size_t FluidID;  ///< unique ID for fluid

	  enum FluidDensityModel 
	  {
		  Calculated,
		  Constant,
		  Table
	  };

	  enum CalculationModel 
	  { 
		  ConstantModel, 
		  CalculatedModel, 
		  TableModel, 
		  StandardModel, 
		  LowcondModel, 
		  HighcondModel 
	  };

      /// @brief Get list of fluids in the model
      /// @return array with IDs of different lygthologies defined in the model
      virtual std::vector<FluidID> getFluidsID() const = 0; 

      /// @brief Create new fluid
      /// @return ID of the new Fluid
      virtual FluidID createNewFluid() = 0;

      /// @brief Get fluid name for
      /// @param[in] id fluid ID
      /// @param[out] fluidName on success has a fluid name, or empty string otherwise
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode getFluidName( FluidID id, std::string & fluidName ) = 0;

      /// @}

	  // Density model definition
	  /// @{

	  /// @brief Get fluid density model
	  /// @param[in] id fluid ID
	  /// @param[out] densModel type of density model set for the given fluid
	  /// @param[out] refDens reference density value.
	  /// @return NoError on success or OutOfRangeValue or NonexistingID on error
	  virtual ReturnCode densityModel(FluidID id, FluidDensityModel & densModel, double & refDens) = 0;

	  /// @brief Set fluid density model
	  /// @param[in] id fluid ID
	  /// @param[in] densModel type of density model set for the given fluid
	  /// @param[in] refDens reference density value.
	  /// @return NoError on success or OutOfRangeValue or NonexistingID on error
	  virtual ReturnCode setDensityModel(FluidID id, const FluidDensityModel & densModel, const double & refDens) = 0;

	  /// @}

	  // Seismic Velocity model definition
	  /// @{

	  /// @brief Get fluid seismic velocity model
	  /// @param[in] id fluid ID
	  /// @param[out] seisVelModel type of seismic velocity model set for the given fluid
	  /// @param[out] refSeisVel reference seismic velocity value.
	  /// @return NoError on success or OutOfRangeValue or NonexistingID on error
	  virtual ReturnCode seismicVelocityModel(FluidID id, CalculationModel & seisVelModel, double & refSeisVel) = 0;

	  /// @brief Get fluid seismic velocity model
	  /// @param[in] id fluid ID
	  /// @param[in] seisVelModel type of seismic velocity model set for the given fluid
	  /// @param[in] refSeisVel reference seismic velocity value.
	  /// @return NoError on success or OutOfRangeValue or NonexistingID on error
	  virtual ReturnCode setSeismicVelocityModel(FluidID id, const CalculationModel & seisVelModel, const double & refSeisVel) = 0;

	  /// @}
   protected:
      /// @{
      /// Constructors/destructor

      /// @brief Constructor which creates an empty model
	   FluidManager() = default;
      
      /// @brief Destructor, no any actual work is needed here, all is done in the implementation part
	   virtual ~FluidManager() = default;

      /// @}

   private:
      /// @{
      /// Copy constructor and copy operator are disabled
      FluidManager( const FluidManager & otherFluidManager );
      FluidManager & operator = ( const FluidManager & otherFluidManager );
      /// @}
   };
}

#endif // CMB_FLUID_MANAGER_API

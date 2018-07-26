//                                                                      
// Copyright (C) 2012-2018 Shell International Exploration & Production.
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

#include "ProjectFileHandler.h"

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

      // Set project database. Reset all
      void setDatabase(database::ProjectFileHandlerPtr pfh);

      // Get list of fluids in the model
      // return array with IDs of different fluids defined in the model
      virtual std::vector<FluidID> getFluidsID() const; 

      // Create new fluid
      // return ID of the new Fluid
      virtual FluidID createNewFluid();

      // Get fluid name for
      // [in] id fluid ID
      // [out] fluidName on success has a fluid name, or empty string otherwise
      // return NoError on success or NonexistingID on error
      virtual ReturnCode getFluidName( const FluidID id, std::string & fluidName );

      /// @brief Set description for
      /// @param[in] id fluid ID
      /// @param[in] myDescription contains fluid description
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode getDescription(const FluidID id, std::string & myDescription);

      /// @brief Set description for
      /// @param[in] id fluid ID
      /// @param[in] myDescription contains fluid description
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode setDescription(const FluidID id, const std::string & myDescription);

      /// @brief Get UserDefined flag for
      /// @param[in] id fluid ID
      /// @param[out] if myUserDefined is 1, the fluid type is user defined, else system generated
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode getUserDefined(const FluidID id, int & myUserDefined);

      /// @brief Set UserDefined flag for
      /// @param[in] id fluid ID
      /// @param[in] if myUserDefined is 1, the fluid type is user defined, else system generated
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode setUserDefined(const FluidID id, const int myUserDefined);

      // Density model definition
      /// @{

      /// @brief Get fluid density model
      /// @param[in] id fluid ID
      /// @param[out] densModel type of density model set for the given fluid
      /// @param[out] refDens reference density value.
      /// @return NoError on success or OutOfRangeValue or NonexistingID on error
      virtual ReturnCode densityModel(const FluidID id, FluidDensityModel & densModel, double & refDens);

      /// @brief Set fluid density model
      /// @param[in] id fluid ID
      /// @param[in] densModel type of density model set for the given fluid
      /// @param[in] refDens reference density value.
      /// @return NoError on success or OutOfRangeValue or NonexistingID on error
      virtual ReturnCode setDensityModel(const FluidID id, const FluidDensityModel densModel, const double refDens);

      /// @}

      // Seismic Velocity model definition
      /// @{

      /// @brief Get fluid seismic velocity model
      /// @param[in] id fluid ID
      /// @param[out] seisVelModel type of seismic velocity model set for the given fluid
      /// @param[out] refSeisVel reference seismic velocity value.
      /// @return NoError on success or OutOfRangeValue or NonexistingID on error
      virtual ReturnCode seismicVelocityModel(const FluidID id, CalculationModel & seisVelModel, double & refSeisVel);

      /// @brief Get fluid seismic velocity model
      /// @param[in] id fluid ID
      /// @param[in] seisVelModel type of seismic velocity model set for the given fluid
      /// @param[in] refSeisVel reference seismic velocity value.
      /// @return NoError on success or OutOfRangeValue or NonexistingID on error
      virtual ReturnCode setSeismicVelocityModel(const FluidID id, const CalculationModel seisVelModel, const double refSeisVel);

      /// @}
      
   private:
      // Copy constructor is disabled, use the copy operator instead
      FluidManagerImpl( const FluidManager & );

      static const char * s_fluidTypesTableName;
      database::ProjectFileHandlerPtr m_db;                 // cauldron project database
      database::Table               * m_fluidIoTbl;         // Fluid Type Io table
      static const char * s_fluidTypeFieldName;		        // column name for fluid name
      static const char * s_descriptionFieldName;		     // column name for fluid description
      static const char * s_userDefinedFieldName;           // column name for user defined flag	  
      static const char * s_densityModelFieldName;          // column name for density model
      static const char * s_densityFieldName;               // column name for reference density
      static const char * s_seismicVelocityModelFieldName;  // column name for seismic velocity model
      static const char * s_seismicVelocityFieldName;       // column name for reference seismic velocity
   };
}

#endif // CMB_FLUID_MANAGER_IMPL_API
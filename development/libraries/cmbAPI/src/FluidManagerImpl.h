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

      /// @brief Get list of stratigraphic layers defined in the StratIoTbl 
      /// @return array with IDs of different layers in the model
      virtual std::vector<LayerID> getLayerID() const;

      /// @brief Get fluid type from the StartIoTbl
      /// @param[in] id Stratigraphic layer ID
      /// @param[out] brine specified for particular ID
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode getBrineType(LayerID id, std::string & brine);

      /// @brief Get HeatCaptype for
      /// @param[in] id fluid ID
      /// @param[out] HeatCapType specified for a particular fluid
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode getHeatCapType(FluidID id, std::string & HeatCapType);

      /// @brief Set HeatCaptype for
      /// @param[in] id fluid ID
      /// @param[in] HeatCapType required to be set for a particular fluid
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode setHeatCapType(FluidID id, const std::string & HeatCapType);

      /// @brief Get ThermCondType for
      /// @param[in] id fluid ID
      /// @param[out] ThermCondType specified for a particular fluid
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode getThermCondType(FluidID id, std::string & ThermCondType);

      /// @brief Set ThermCondType for
      /// @param[in] id fluid ID
      /// @param[in] ThermCondType required to be set for a particular fluid
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode setThermCondType(FluidID id, const std::string & ThermCondType);

      /// @brief Get list of ids for each row in the FltThCondIoTbl 
      /// @return array with IDs defined in the FltThCondIoTbl 
      virtual std::vector<FluidThCondID> getFluidThCondID() const;

      /// @brief Get TempIndex from FltThCondIoTbl 
      /// @param[in] id of FltThCondIoTbl
      /// @param[out] Temperature value specified for the particular id
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode getThermalCondTblTempIndex(FluidThCondID id, double & Temperature);

      /// @brief Set TempIndex in FltThCondIoTbl 
      /// @param[in] id fluid ID
      /// @param[in] Temperature value specified for the particular id
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode setThermalCondTblTempIndex(FluidThCondID id, const double & Temperature);

      /// @brief Get Pressure from FltThCondIoTbl 
      /// @param[in] id of FltThCondIoTbl
      /// @param[out] Pressure value specified for the particular id
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode getThermalCondTblPressure(FluidThCondID id, double & Pressure);

      /// @brief Set Pressure in FltThCondIoTbl 
      /// @param[in] id fluid ID
      /// @param[in] Pressure value specified for the particular id
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode setThermalCondTblPressure(FluidThCondID id, const double & Pressure);

      /// @brief Get ThCond from FltThCondIoTbl 
      /// @param[in] id of FltThCondIoTbl
      /// @param[out] ThCond value specified for the particular id
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode getThermalCond(FluidThCondID id, double & ThCond);

      /// @brief Get list of ids for each row in the FltHeatCapIoTbl 
      /// @return array with IDs defined in the FltHeatCapIoTbl 
      virtual std::vector<FluidHeatCapID> getFluidHeatCapID() const;

      /// @brief Get TempIndex from FltHeatCapIoTbl 
      /// @param[in] id of FltHeatCapIoTbl
      /// @param[out] Temperature value specified for the particular id
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode getHeatCapTblTempIndex(FluidHeatCapID id, double & Temperature);

      /// @brief Get Pressure from FltHeatCapIoTbl 
      /// @param[in] id of FltHeatCapIoTbl
      /// @param[out] Pressure value specified for the particular id
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode getHeatCapTblPressure(FluidHeatCapID id, double & Pressure);

      /// @brief Get HeatCapacity from FltHeatCapIoTbl 
      /// @param[in] id of FltHeatCapIoTbl
      /// @param[out] HeatCap value specified for the particular id
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode getHeatCap(FluidHeatCapID id, double & HeatCap);

      /// @brief Get DefinedBy field from FluidTypeIoTbl 
      /// @param[in] id unique id of each record in FluidTypeIoTbl
      /// @param[out] DefinedBy brine type specified for the particular id
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode getDefinedBy(FluidID id, std::string & DefinedBy);

      /// @brief Set DefinedBy field for each record in the FluidTypeIoTbl
      /// @param[in] id fluid ID for each record in FluidTypeIoTbl
      /// @param[in] DefinedBy brine type specified for the particular id
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode setDefinedBy(FluidID id, const std::string & DefinedBy);

      /// @}
      
   private:
      // Copy constructor is disabled, use the copy operator instead
      FluidManagerImpl( const FluidManager & );

      static const char * s_fluidTypesTableName;
      static const char * s_startLayersTableName;
      static const char * s_fltThCondTableName;
      static const char * s_fltHeatCapTableName;
      database::ProjectFileHandlerPtr m_db;                 // cauldron project database
      database::Table               * m_fluidIoTbl;         // Fluid Type Io table
      database::Table               * m_stratIoTbl;         // start Io table
      database::Table               * m_fltThCondIoTbl;     // Fluid Thermal Conductivity Io table
      database::Table               * m_fltHeatCapIoTbl;     // Fluid heat capacity Io table
      static const char * s_fluidTypeFieldName;		        // column name for fluid name in FluidTypeIoTbl
      static const char * s_descriptionFieldName;		     // column name for fluid description
      static const char * s_userDefinedFieldName;           // column name for user defined flag	  
      static const char * s_densityModelFieldName;          // column name for density model
      static const char * s_densityFieldName;               // column name for reference density
      static const char * s_seismicVelocityModelFieldName;  // column name for seismic velocity model
      static const char * s_seismicVelocityFieldName;       // column name for reference seismic velocity
      static const char * s_stratIoTblFluidTypeFieldName;       // column name for fluid type in stratIoTbl
      static const char * s_fluidIoTblHeatCapTypeFieldName;   // column name for heat capacity type specified in FluidTypeIoTbl
      static const char * s_fluidIoTblThermCondTypeFieldName; // column name for thermal conductivity type specified in FluidTypeIoTbl
      static const char * s_fltThCondTableTempIndexFieldName; // column name for TempIndex field of FltThCondIoTbl
      static const char * s_fltThCondTablePressureFieldName; // column name for Pressure field of FltThCondIoTbl
      static const char * s_fltThCondFieldName; // column name for ThCond field of FltThCondIoTbl
      static const char * s_fltHeatCapTableTempIndexFieldName; // column name for TempIndex field of FltHeatCapIoTbl
      static const char * s_fltHeatCapTablePressureFieldName; // column name for Pressure field of FltHeatCapIoTbl
      static const char * s_fltHeatCapFieldName; // column name for ThCond field of FltHeatCapIoTbl
      static const char * s_definedByFieldName; // column name for DefinedBy field of FluidTypeIoTbl
   };
}

#endif // CMB_FLUID_MANAGER_IMPL_API
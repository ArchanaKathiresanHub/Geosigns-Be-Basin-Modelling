//                                                                      
// Copyright (C) 2012-2018 Shell International Exploration & Production.
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
      typedef size_t LayerID;  ///< unique ID for stratigraphic layers
      typedef size_t FluidThCondID;  ///< unique ID for FltThCondIoTbl
      typedef size_t FluidHeatCapID;  ///< unique ID for FltHeatCapIoTbl

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
      virtual ReturnCode getFluidName( const FluidID id, std::string & fluidName ) = 0;

      /// @brief Set description for
      /// @param[in] id fluid ID
      /// @param[in] myDescription contains fluid description
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode getDescription(const FluidID id, std::string & myDescription) = 0;

      /// @brief Set description for
      /// @param[in] id fluid ID
      /// @param[in] myDescription contains fluid description
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode setDescription(const FluidID id, const std::string & myDescription) = 0;

      /// @brief Get UserDefined flag for
      /// @param[in] id fluid ID
      /// @param[out] if myUserDefined is 1, the fluid type is user defined, else system generated
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode getUserDefined(const FluidID id, int & myUserDefined) = 0;

      /// @brief Set UserDefined flag for
      /// @param[in] id fluid ID
      /// @param[in] if myUserDefined is 1, the fluid type is user defined, else system generated
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode setUserDefined(const FluidID id, const int myUserDefined) = 0;
      /// @}

	  // Density model definition
     /// @{

     /// @brief Get fluid density model
     /// @param[in] id fluid ID
     /// @param[out] densModel type of density model set for the given fluid
     /// @param[out] refDens reference density value.
     /// @return NoError on success or OutOfRangeValue or NonexistingID on error
      virtual ReturnCode densityModel(const FluidID id, FluidDensityModel & densModel, double & refDens) = 0;

      /// @brief Set fluid density model
      /// @param[in] id fluid ID
      /// @param[in] densModel type of density model set for the given fluid
      /// @param[in] refDens reference density value.
      /// @return NoError on success or OutOfRangeValue or NonexistingID on error
      virtual ReturnCode setDensityModel(const FluidID id, const FluidDensityModel densModel, const double refDens) = 0;

      /// @}

      // Seismic Velocity model definition
      /// @{

      /// @brief Get fluid seismic velocity model
      /// @param[in] id fluid ID
      /// @param[out] seisVelModel type of seismic velocity model set for the given fluid
      /// @param[out] refSeisVel reference seismic velocity value.
      /// @return NoError on success or OutOfRangeValue or NonexistingID on error
      virtual ReturnCode seismicVelocityModel(const FluidID id, CalculationModel & seisVelModel, double & refSeisVel) = 0;

      /// @brief Get fluid seismic velocity model
      /// @param[in] id fluid ID
      /// @param[in] seisVelModel type of seismic velocity model set for the given fluid
      /// @param[in] refSeisVel reference seismic velocity value.
      /// @return NoError on success or OutOfRangeValue or NonexistingID on error
      virtual ReturnCode setSeismicVelocityModel(const FluidID id, const CalculationModel seisVelModel, const double refSeisVel) = 0;

      /// @brief Get list of stratigraphic layers defined in the StratIoTbl 
      /// @return array with IDs of different layers in the model
      virtual std::vector<LayerID> getLayerID() const = 0;

      /// @brief Get fluid type from the StartIoTbl
      /// @param[in] id Stratigraphic layer ID
      /// @param[out] brine specified for particular ID
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode getBrineType(LayerID id, std::string & brine) = 0;

      /// @brief Get HeatCaptype for
      /// @param[in] id fluid ID
      /// @param[out] HeatCapType specified for a particular fluid
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode getHeatCapType(FluidID id, std::string & HeatCapType) = 0;

      /// @brief Set HeatCaptype for
      /// @param[in] id fluid ID
      /// @param[in] HeatCapType required to be set for a particular fluid
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode setHeatCapType(FluidID id, const std::string & HeatCapType) = 0;

      /// @brief Get ThermCondType for
      /// @param[in] id fluid ID
      /// @param[out] ThermCondType specified for a particular fluid
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode getThermCondType(FluidID id, std::string & ThermCondType) = 0;

      /// @brief Set ThermCondType for
      /// @param[in] id fluid ID
      /// @param[in] ThermCondType required to be set for a particular fluid
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode setThermCondType(FluidID id, const std::string & ThermCondType) = 0;

      /// @brief Get list of ids for each row in the FltThCondIoTbl 
      /// @return array with IDs defined in the FltThCondIoTbl 
      virtual std::vector<FluidThCondID> getFluidThCondID() const = 0;

      /// @brief Get TempIndex from FltThCondIoTbl 
      /// @param[in] id of FltThCondIoTbl
      /// @param[out] Temperature value specified for the particular id
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode getThermalCondTblTempIndex(FluidThCondID id, double & Temperature) = 0;

      /// @brief Set TempIndex in FltThCondIoTbl 
      /// @param[in] id fluid ID
      /// @param[in] Temperature value specified for the particular id
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode setThermalCondTblTempIndex(FluidThCondID id, const double & Temperature) = 0;

      /// @brief Get Pressure from FltThCondIoTbl 
      /// @param[in] id of FltThCondIoTbl
      /// @param[out] Pressure value specified for the particular id
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode getThermalCondTblPressure(FluidThCondID id, double & Pressure) = 0;
      
      /// @brief Set Pressure in FltThCondIoTbl 
      /// @param[in] id fluid ID
      /// @param[in] Pressure value specified for the particular id
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode setThermalCondTblPressure(FluidThCondID id, const double & Pressure) = 0;

      /// @brief Get ThCond from FltThCondIoTbl 
      /// @param[in] id of FltThCondIoTbl
      /// @param[out] ThCond value specified for the particular id
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode getThermalCond(FluidThCondID id, double & ThCond) = 0;

      /// @brief Get list of ids for each row in the FltHeatCapIoTbl 
      /// @return array with IDs defined in the FltHeatCapIoTbl 
      virtual std::vector<FluidHeatCapID> getFluidHeatCapID() const = 0;

      /// @brief Get TempIndex from FltHeatCapIoTbl 
      /// @param[in] id of FltHeatCapIoTbl
      /// @param[out] Temperature value specified for the particular id
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode getHeatCapTblTempIndex(FluidHeatCapID id, double & Temperature) = 0;

      /// @brief Get Pressure from FltHeatCapIoTbl 
      /// @param[in] id of FltHeatCapIoTbl
      /// @param[out] Pressure value specified for the particular id
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode getHeatCapTblPressure(FluidHeatCapID id, double & Pressure) = 0;

      /// @brief Get HeatCapacity from FltHeatCapIoTbl 
      /// @param[in] id of FltHeatCapIoTbl
      /// @param[out] HeatCap value specified for the particular id
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode getHeatCap(FluidHeatCapID id, double & HeatCap) = 0;

      /// @brief Get DefinedBy field from FluidTypeIoTbl 
      /// @param[in] id unique id of each record in FluidTypeIoTbl
      /// @param[out] DefinedBy brine type specified for the particular id
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode getDefinedBy(FluidID id, std::string & DefinedBy) = 0;

      /// @brief Set DefinedBy field for each record in the FluidTypeIoTbl
      /// @param[in] id fluid ID for each record in FluidTypeIoTbl
      /// @param[in] DefinedBy brine type specified for the particular id
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode setDefinedBy(FluidID id, const std::string & DefinedBy) = 0;

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

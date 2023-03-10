//                                                                      
// Copyright (C) 2012-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file LithologyManager.h
/// @brief This file keeps API declaration for manipulating lithologies in Cauldron model

#ifndef CMB_LITHOLOGY_MANAGER_API
#define CMB_LITHOLOGY_MANAGER_API

#include <vector>
#include "ErrorHandler.h"

// DataAccess Lib
#include "Interface.h"

/// @page LithologyManagerPage Lithology Manager
/// @link mbapi::LithologyManager Lithology manager @endlink provides set of interfaces to create/delete/edit 
/// list of lithologies defined in data model. Also it has set of interfaces to get/set property of lithology from the list
///
/// @section PorosityModelsSec Porosity models
/// Lithology must have defined a porosity model. The current implementation allows to choose from:
///   -# Exponential porosity model
///   -# Soil mechanics porosity model
///   -# Double exponential porosity model
///
/// @subsection ExpPorosityModelSubSec Exponential porosity model 
/// in <b>Exponential</b> porosity model the porosity is calculated according to this formula:
/// @f[ \phi = \phi_0 exp(-c_{ef} \sigma) @f]
/// The model has 2 parameters:
///   -# @f$ \phi_0 @f$ - surface porosity [%]
///   -# @f$ c_{ef} @f$ - compaction coefficient (effective stress) @f$ [10^{-8} Pa^{-1}] @f$
///   -# @f$ \phi_min @f$ - minimal porosity [%]
///
/// @subsection SoilMechPorosityModelSubSec Soil mechanics porosity model
/// in <b>Soil Mechanics</b> porosity model the porosity is calculated according to this formula:
/// @f[ \phi = \frac{\psi}{1+\psi} @f] where
/// @f[ \psi = \psi_0 - \beta log( \frac{\sigma}{\sigma_0} ), \psi_0 = \frac{\phi_0}{1 - \phi_0} @f]
/// The model has 2 parameters:
///   -# @f$ \phi_0 @f$ - surface porosity [%]
///   -# @f$ \beta @f$ - compaction coefficient  @f$ [unitless] @f$
///
/// @subsection DoubleExpPorosityModelSubSec Double-Exponential porosity model 
/// in <b>DoubleExponential</b> porosity model the porosity is calculated according to this formula:
/// @f[ \phi = \phi_m + \phi_0 / 2 exp(-c1_{ef} \sigma) + \phi_0 / 2 exp(-c2_{ef} \sigma) @f]
/// The model has 4 parameters:
///   -# @f$ \phi_0 @f$ - surface porosity
///   -# @f$ \phi_min @f$ - minimal porosity
///   -# @f$ c1_{ef} @f$ - compaction coefficient (effective stress) @f$ [10^{-8} Pa^{-1}] @f$
///   -# @f$ c2_{ef} @f$ - compaction coefficient (effective stress) @f$ [10^{-8} Pa^{-1}] @f$
///
///
/// @section PermModelsSpec Permeability models
/// Lithology must have defined a permeability model. The current implementation allows to choose from:
///   -# Nonpermeable layer, layer with tiny or zero permeability which does not depends on porosity
///   -# Mudstone
///   -# Sandstone
///   -# Multipoint permeability model
///
/// @subsection NonePermModelSubSec None model means tiny constant permeability
/// Permeability is set to @f$ 1.0^{-9} @f$ mD
///
/// @subsection SandPermModelSubSec Permeability for sandstone
/// in <b>Sandstone</b> permeability model the permeability is calculated according to this formula:
/// @f[ k_{sand}=k_{0}\cdot10^{(\phi-\phi_{0})\cdot(0.12+0.02\cdot C)} @f]
/// The model has 3 parameters:
///  -# @f$ k_{0} @f$ - depositional permeability [mD]
///  -# permeability anisotropy [kh/kv] which is used to scale lateral permeability
///  -# @f$ C @f$ clay percentage of the sand [%]
///
/// @subsection MudPermModelSubSec Permeability for mudstone
/// in <b>Mudstone</b> permeability model the permeability is calculated according to this formula:
/// @f[ k_{shale}=k_{0}\cdot\left(\left[\frac{VES+\sigma_{ref}}{\sigma_{ref}}\right]^{-C_{sensitivity}}\cdot
///       \left[\frac{VES+\sigma_{ref}}{MaxVES+\sigma_{ref}}\right]^{-C_{recovery}}\right)
/// @f]
/// The model has 4 parameters
///  -# @f$ k_{0} @f$ - depositional permeability [mD]
///  -# permeability anisotropy [kh/kv] which is used to scale lateral permeability
///  -# @f$ C_{sensitivity} @f$ permeability sensitivity coefficient []
///  -# @f$ C_{recovery} @f$ permeability recovery coefficient []
///
/// @subsection MultiPointPermModelSubSec
/// in <b>Multipoint</b> permeability model is a simple table of porosity and @f$ (log10) @f$ permeability values.
/// This allows the user to input any porosity-permeability relationship 
/// The model has 4 parameters
///  -# permeability anisotropy [kh/kv] which is used to scale lateral permeability
///  -# array of porosity values [%]
///  -# array of permeability values [log10(mD)] which has the same size as an array of porosity values
/// 

namespace mbapi {
   /// @class LithologyManager LithologyManager.h "LithologyManager.h"
   /// @brief Class LithologyManager keeps a list of lithologies in Cauldron model and allows to add/delete/edit lithology
   class LithologyManager : public ErrorHandler
   {
   public:

      /// @{
      /// Set of interfaces for interacting with a Cauldron model
      typedef size_t LithologyID;         ///< unique ID for lithology
      typedef size_t AllochtLithologyID;  ///< unique ID for allochton lithology
	  typedef size_t LitThCondTblID;	///< unique ID for thermalConductivity values of lithotypes
	  typedef size_t LitHeatCapTblID;	///< unique ID for thermalConductivity values of lithotypes

      typedef enum
      {
         PorExponential,           ///< Exponential porosity model
         PorSoilMechanics,         ///< Soil mechanics porosity model
         PorDoubleExponential,     ///< Double Exponential porosity model 
         PorUnknown           = -1 ///< Not any model was defined
      } PorosityModel;

      enum PorModelParameters
      {
         PhiSurf,
         CompactionCoef,
         PhiMin,
         CompactionCoefB,
         CompactionRatio
      };

      // Define permeability models in the same name as in DataAccess interface library
      typedef enum
      {
         PermSandstone,       ///< permeability model for sandstones
         PermMudstone,        ///< permeability model for shales
         PermNone,            ///< non permeable layer, layer with tiny permeability ~1e-9
         PermImpermeable,     ///< mostly the same as None
         PermMultipoint,      ///< permeability depends on porosity as 1D function
         PermUnknown     = -1 ///< Not any model was defined
      } PermeabilityModel;

      enum PermSandstoneParameters
      {
         AnisotropySand,
         PermSurfSand,
         SandClayPercentage
      };

      enum PermMudstoneParameters
      {
         AnisotropyMud,
         PermSurfMud,
         SensitivityCoef,
         RecoveryCoeff
      };

      /// @brief Get list of lithologies in the model
      /// @return array with IDs of different lygthologies defined in the model
      virtual std::vector<LithologyID> lithologiesIDs() const = 0; 

      /// @brief Create new lithology
      /// @return ID of the new Lithology
      virtual LithologyID createNewLithology() = 0;

      /// @brief Get lithology type name for the given ID
      /// @param[in] id lithology ID
      /// @return lithology type name for given lithology ID or empty string in case of error
      virtual std::string lithologyName( LithologyID id ) = 0;

	  // Set lithology name for the given ID
	  // [in] id lithology ID
	  // [in] newName contains the lithology name as per BPA2 standard
	  // return NoError on success or NonexistingID on error
	  virtual ReturnCode setLithologyName(const LithologyID id, const std::string & newName) = 0;

	  /// @brief get user defined flag value for the given lithology ID
	  /// @param[in] id lithology ID
	  /// @param[out] flag user defined flag value for the particular lithology id
	  /// @return NoError on success or NonexistingID on error
	  virtual ReturnCode getUserDefinedFlagForLithology(const LithologyID id, int & flag) = 0;

	  /// @brief get reference lithology for the given lithology ID
	  /// @param[in] id lithology ID
	  /// @param[out] referenceLithology reference lithology for the particular lithology id
	  /// @return NoError on success or NonexistingID on error
	  virtual ReturnCode getReferenceLithology(const LithologyID id, std::string & referenceLithology) = 0;

     /// @brief get the Density
     /// @param[in] id lithology ID
     /// @return Density
     virtual double  getLitPropDensity(const LithologyID id) = 0;

     /// @brief get the Heat Production
     /// @param[in] id lithology ID
     /// @return Heat Production
     virtual double  getLitPropHeatProduction(const LithologyID id) = 0;

     /// @brief get the Thermal Conductivity
     /// @param[in] id lithology ID
     /// @return Thermal Conductivity
     virtual double  getLitPropThrConductivity(const LithologyID id) = 0;

     /// @brief get the Thermal Conductivity Anistropy
     /// @param[in] id lithology ID
     /// @return Thermal Conductivity Anistropy
     virtual double  getLitPropThrCondAnistropy(const LithologyID id) = 0;

     /// @brief get the Permeability Anistropy
     /// @param[in] id lithology ID
     /// @return Permeability Anistropy
     virtual double  getLitPropPermAnistropy(const LithologyID id) = 0;

     /// @brief get the Seismic Velocity
     /// @param[in] id lithology ID
     /// @return Seismic Velocity
     virtual double  getLitPropSeisVelocity(const LithologyID id) = 0;

     /// @brief get the Seismic Velocity Exponent
     /// @param[in] id lithology ID
     /// @return Seismic Velocity Exponent
     virtual double  getLitPropSeisVeloExponent(const LithologyID id) = 0;

     /// @brief get the Entry Pressure Coefficient 1
     /// @param[in] id lithology ID
     /// @return Entry Pressure Coefficient 1
     virtual double  getLitPropEntryPresCoeff1(const LithologyID id) = 0;

     /// @brief get the Entry Pressure Coefficient 2
     /// @param[in] id lithology ID
     /// @return Entry Pressure Coefficient 2
     virtual double  getLitPropEntryPresCoeff2(const LithologyID id) = 0;

     /// @brief get the Hydraulic Fracturing
     /// @param[in] id lithology ID
     /// @return Hydraulic Fracturing
     virtual double  getLitPropHydFracturing(const LithologyID id) = 0;

     /// @brief get the ReferenceSolidViscosity
     /// @param[in] id lithology ID
     /// @return ReferenceSolidViscosity
     virtual double  getLitPropRefSoldViscosity(const LithologyID id) = 0;

     /// @brief get the Intrusion Temperature
     /// @param[in] id lithology ID
     /// @return Intrusion Temperature
     virtual double  getLitPropIntrTemperature(const LithologyID id) = 0;

     /// @brief set the Density
     /// @param[in] id lithology ID
     /// @param[in] value Density 
     // return NoError on success or NonexistingID on error
     virtual ReturnCode  setLitPropDensity(const LithologyID id, double & value) = 0;

     /// @brief set the Heat Production
     /// @param[in] id lithology ID
     /// @param[in] value Heat Production
     // return NoError on success or NonexistingID on error
     virtual ReturnCode  setLitPropHeatProduction(const LithologyID id, double & value) = 0;

     /// @brief set the Thermal Conductivity
     /// @param[in] id lithology ID
     /// @param[in] value Thermal Conductivity
     // return NoError on success or NonexistingID on error
     virtual ReturnCode  setLitPropThrConductivity(const LithologyID id, double & value) = 0;

     /// @brief set the Thermal Conductivity Anistropy
     /// @param[in] id lithology ID
     /// @param[in] value Thermal Conductivity Anistropy
     // return NoError on success or NonexistingID on error
     virtual ReturnCode  setLitPropThrCondAnistropy(const LithologyID id, double & value) = 0;

     /// @brief set the Permeability Anistropy
     /// @param[in] id lithology ID
     /// @param[in] value Permeability Anistropy
     // return NoError on success or NonexistingID on error
     virtual ReturnCode  setLitPropPermAnistropy(const LithologyID id, double & value) = 0;

     /// @brief set the Seismic Velocity
     /// @param[in] id lithology ID
     /// @param[in] value Seismic Velocity
     // return NoError on success or NonexistingID on error
     virtual ReturnCode  setLitPropSeisVelocity(const LithologyID id, double & value) = 0;

     /// @brief set the Seismic Velocity Exponent
     /// @param[in] id lithology ID
     /// @param[in] value Seismic Velocity Exponent
     // return NoError on success or NonexistingID on error
     virtual ReturnCode  setLitPropSeisVeloExponent(const LithologyID id, double & value) = 0;

     /// @brief set the Entry Pressure Coefficient 1
     /// @param[in] id lithology ID
     /// @param[in] value Entry Pressure Coefficient 1
     // return NoError on success or NonexistingID on error
     virtual ReturnCode  setLitPropEntryPresCoeff1(const LithologyID id, double & value) = 0;

     /// @brief set the Entry Pressure Coefficient 2
     /// @param[in] id lithology ID
     /// @param[in] value Entry Pressure Coefficient 2
     // return NoError on success or NonexistingID on error
     virtual ReturnCode  setLitPropEntryPresCoeff2(const LithologyID id, double & value) = 0;

     /// @brief set the Hydraulic Fracturing
     /// @param[in] id lithology ID
     /// @param[in] value Hydraulic Fracturing
     // return NoError on success or NonexistingID on error
     virtual ReturnCode  setLitPropHydFracturing(const LithologyID id, double & value) = 0;

     /// @brief set the ReferenceSolidViscosity
     /// @param[in] id lithology ID
     /// @param[in] value ReferenceSolidViscosity
     // return NoError on success or NonexistingID on error
     virtual ReturnCode  setLitPropRefSoldViscosity(const LithologyID id, double & value) = 0;

     /// @brief set the Intrusion Temperature
     /// @param[in] id lithology ID
     /// @param[in] value Intrusion Temperature
     // return NoError on success or NonexistingID on error
     virtual ReturnCode  setLitPropIntrTemperature(const LithologyID id, double & value) = 0;

      /// @brief Get lithology description for the given ID
      /// @param[in] id lithology ID
      /// @return lithology description for given lithology ID or empty string in case of error
      virtual std::string getDescription(const LithologyID id) = 0;

      /// @brief Set lithology description for the given ID
      /// @param[in] id lithology ID
      /// @param[in] myDescription contains lithology description
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode setDescription(const LithologyID id, const std::string & myDescription) = 0;

      /// @brief Search for lithology record which has given lithology name 
      /// @param lName lithology name
      /// @return ID of found lithology on success or UndefinedIDValue otherwise
      virtual LithologyID findID( const std::string & lName ) = 0;

      /// @brief Make a copy of the given lithology. Also makes a new set of records in table [LitThCondIoTbl] for the new litholog
      ///        If there is another lithology with the same as given new name, method will fail.
      /// @param[in] id lithology ID
      /// @param[in] newLithoName new name for the lithology
      /// @return new lithology ID on success or UndefinedIDValue on error
      virtual LithologyID copyLithology( LithologyID id, const std::string & newLithoName ) = 0;

      /// @brief Check does this lithology has references in FaultCut and AlochtLith tables, then delete
      /// this lithology from the Lithology table and delete all records from ThermoCond/ThermoCapacity tables
      /// @param id lithology ID
      /// @return NoError on success or error code if this lithology is referenced in other tables
      virtual ReturnCode deleteLithology( LithologyID id ) = 0;

      /// @brief Scan lithology table for duplicated lithologies and delete them updating references
      /// @return NoError on success or error code otherwise
      virtual ReturnCode cleanDuplicatedLithologies() = 0;

      // Alochton lithology methods
      /// @{ 

      /// @brief Get list of allochton lithologies in the model
      /// @return array with IDs of allochton lygthologies defined in the model
      virtual std::vector<AllochtLithologyID> allochtonLithologiesIDs() const = 0; 

      /// @brief Search in AllochthonLithoIoTbl table for the given layer name
      /// @param layerName layer name for allochton lithology
      /// @return AllochthonLithologyID for the found lithology on success, UndefinedIDValue otherwise
      virtual AllochtLithologyID findAllochtID( const std::string & layerName ) = 0;

      /// @brief Get lithlogy name for the allochton lithology
      /// @return Name of the allochton lithology
      virtual std::string allochtonLithology( AllochtLithologyID alID ) = 0;

      /// @brief Get layer name for the allochton lithology
      /// @return Name of the layer for allochton lithology
      virtual std::string allochtonLithologyLayerName( AllochtLithologyID alID ) = 0;

      /// @brief Set new allochton lithology for the layer
      /// @param alID layer id in AllochthonLithoIoTbl
      /// @param newLithoName new lithology name
      /// @return ErrorHandler::NoError on success, error code otherwise
      virtual ReturnCode setAllochtonLithology( AllochtLithologyID alID, const std::string & newLithoName ) = 0;
      /// @}
      
      // Porosity model definition
      /// @{

      /// @brief Get lithology porosity model
      /// @param[in] id lithology ID
      /// @param[out] porModel type of porosity model set for the given lithology
      /// @param[out] porModelPrms array with porosity model parameters. The order of parameters depends on the model and 
      ///             the same as described \ref PorosityModelsSec here.
      /// @return NoError on success or OutOfRangeValue or NonexistingID on error
      virtual ReturnCode porosityModel( LithologyID id, PorosityModel & porModel, std::vector<double> & porModelPrms ) = 0;

      /// @brief Set lithology porosity model
      /// @param[in] id lithology ID
      /// @param[in] porModel new type of porosity model for the given lithology
      /// @param[in] porModelPrms porosity model parameters. The order of parameters depends on the model and 
      ///             the same as described \ref PorosityModelsSec here.
      /// @return NoError on success or OutOfRangeValue or NonexistingID on error
      virtual ReturnCode setPorosityModel( LithologyID id, PorosityModel porModel, const std::vector<double> & porModelPrms ) = 0;

      /// @}

      // Permeability model definition
      /// @{

      /// @brief Get lithology permeability model
      /// @return NoError on success or error code otherwise
      virtual ReturnCode permeabilityModel( LithologyID           id         ///< lithology ID
                                          , PermeabilityModel   & prmModel   ///< permeability calculation model
                                          , std::vector<double> & modelPrms  ///< model parameters, depends on the given model
                                          , std::vector<double> & mpPor      ///< for multi-point perm. model the porosity values vector. Empty for other models
                                          , std::vector<double> & mpPerm     ///< for multi-point perm. model the log. of perm values vector. Empty for other models.
                                          ) = 0;

      // @brief Get lithology permeability model parameters
      // @param [in] id          lithology ID
      // @param[out] prmModel   permeability calculation model
      // @param[out] mpPor      for multi-point perm. model the porosity values vector. Empty for other models
      // @param[out] mpPerm     for multi-point perm. model the log. of perm values vector. Empty for other models.
      // @param[out] numPts      for multi-point perm. Number_Of_Data_Points
      // @return NoError on success or error code otherwise
      virtual ReturnCode getPermeabilityModel(LithologyID id, PermeabilityModel & prmModel, std::vector<double> & mpPor, std::vector<double> & mpPerm, int & numPts) = 0;

      /// @brief Set lithology permeability model with parameters
      /// @return NoError on success or error code otherwise
      virtual ReturnCode setPermeabilityModel( LithologyID                 id       ///< lithology ID
                                             , PermeabilityModel           prmModel ///< permeability calculation model
                                             , const std::vector<double> & modelPrms///< model parameters, depends on the given model
                                             , const std::vector<double> & mpPor    ///< for multi-point perm. model the porosity values vector. Empty for other models
                                             , const std::vector<double> & mpPerm   ///< for multi-point perm. model the log. of perm values vector. Empty for other models.
                                             ) = 0;
      /// @}

      /// @brief Set lithology permeability model with parameters
      // @param [in]    id          lithology ID
      // @param[in/out] prmModel    permeability calculation model
      // @param[out] mpPor     for multi-point perm. model the porosity values vector
      // @param[out] mpPerm    for multi-point perm. model the log. of perm values vector
      // @param[in] numPts     for multi-point perm. Number_Of_Data_Points
      /// @param[in]          flag user defined flag value for the particular lithology id
      /// @return NoError on success or error code otherwise
      virtual ReturnCode setPermeabilityModel(LithologyID id, PermeabilityModel prmModel, std::vector<double> & mpPor, std::vector<double> & mpPerm, int & numPts, int & flag) = 0;
      /// @}

      // Seismic velocity
      /// @{


      /// @brief For a referenced lithology, get the value of the seismic velocity
      /// @return the value of the seismic velocity
      virtual double seisVelocity( LithologyID id ) = 0;

      /// @}

      // Thermal conductivity model parameters
      /// @{

      /// @brief Get lithology STP (Standart Temperature Pressure) thermal conductivity [W/m/K]
      /// @param[in] id lithology ID
      /// @return the STP thermal conductivity value on success or UndefinedDoubleValue on error
      virtual double stpThermalConductivityCoeff( LithologyID id ) = 0;

      /// @brief Set lithology STP (Standart Temperature Pressure) thermal conductivity [W/m/K]
      /// @param[in] id lithology ID
      /// @param[in] stpThermCond the new value for STP thermal conductivity
      /// @return NoError on success, NonexistingID on unknown lithology ID or OutOfRangeValue if the value not in an allowed range
      virtual ReturnCode setSTPThermalConductivityCoeff( LithologyID id, double stpThermCond ) = 0;

	  /// @brief Get list of thermal conductiviries for all the lithotypes used in the model from LitThCondIoTbl
	  /// @return array with IDs of different lithologies defined in the model
	  virtual std::vector<LitThCondTblID> thermCondLithologiesIDs() const = 0;

	  /// @brief Get lithology name from LitThCondIoTbl
	  /// @param[in] id lithology ID
	  /// @param[out] LithoName on success has a lithology name
	  /// @return NoError on success or NonexistingID on error
	  virtual ReturnCode getThermCondTableLithoName(const LitThCondTblID id, std::string & LithoName) = 0;

	  /// @brief Set lithotype name in the LitThCondIoTbl
	  /// @param[in] id lithology ID
	  /// @param[in] LithoName new name of the lithology to set for the lithology ID
	  /// @return NoError on success or NonexistingID on error
	  virtual ReturnCode setThermCondTableLithoName(const LitThCondTblID id, const std::string & LithoName) = 0;

	  /// @brief Get list of heat capacities for all the lithotypes used in the model from LitHeatCapIoTbl
	  /// @return array with IDs of different lithologies defined in the model
	  virtual std::vector<LitHeatCapTblID> heatCapLithologiesIDs() const = 0;

	  /// @brief Get lithology name from LitHeatCapIoTbl
	  /// @param[in] id lithology ID
	  /// @param[out] LithoName on success has a lithology name
	  /// @return NoError on success or NonexistingID on error
	  virtual ReturnCode getHeatCapTableLithoName(const LitThCondTblID id, std::string & LithoName) = 0;

	  /// @brief Set lithotype name in the LitHeatCapIoTbl
	  /// @param[in] id lithology ID
	  /// @param[in] LithoName new name of the lithology to set for the lithology ID
	  /// @return NoError on success or NonexistingID on error
	  virtual ReturnCode setHeatCapTableLithoName(const LitThCondTblID id, const std::string & LithoName) = 0;

      /// @}
      /// @}

   protected:
      /// @name Constructors/destructor
      /// @{
      /// @brief Constructor which creates an empty model
      LithologyManager() {;}
      
      /// @brief Destructor, no any actual work is needed here, all is done in the implementation part
      virtual ~LithologyManager() {;}
      /// @}

   private:
      /// @{
      /// Copy constructor and operator are disabled, use the copyTo method instead
      LithologyManager( const LithologyManager & otherLithologyManager );
      LithologyManager & operator = ( const LithologyManager & otherLithologyManager );
      /// @}
   };
}

#endif // CMB_LITHOLOGY_MANAGER_API

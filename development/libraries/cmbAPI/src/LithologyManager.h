//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
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
#include "Interface/Interface.h"

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
      typedef size_t LithologyID;  ///< unique ID for lithology

      typedef enum
      {
         PorExponential       = DataAccess::Interface::EXPONENTIAL_POROSITY,        ///< Exponential porosity model
         PorSoilMechanics     = DataAccess::Interface::SOIL_MECHANICS_POROSITY,     ///< Soil mechanics porosity model
         PorDoubleExponential = DataAccess::Interface::DOUBLE_EXPONENTIAL_POROSITY, ///< Double Exponential porosity model 
         PorUnknown           = -1                                                  ///< Not any model was defined
      } PorosityModel;


      // Define permeability models in the same name as in DataAccess interface library
      typedef enum
      {
         PermSandstone   = DataAccess::Interface::SANDSTONE_PERMEABILITY,   ///< permeability model for sandstones
         PermMudstone    = DataAccess::Interface::MUDSTONE_PERMEABILITY,    ///< permeability model for shales
         PermNone        = DataAccess::Interface::NONE_PERMEABILITY,        ///< non permeable layer, layer with tiny permeability ~1e-9
         PermImpermeable = DataAccess::Interface::IMPERMEABLE_PERMEABILITY, ///< mostly the same as None
         PermMultipoint  = DataAccess::Interface::MULTIPOINT_PERMEABILITY,  ///< permeability depends on porosity as 1D function
         PermUnknown     = -1                                               ///< Not any model was defined
      } PermeabilityModel;

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

      /// @brief Set lithology permeability model with parameters
      /// @return NoError on success or error code otherwise
      virtual ReturnCode setPermeabilityModel( LithologyID                 id         ///< lithology ID
                                             , PermeabilityModel           prmModel   ///< permeability calculation model
                                             , const std::vector<double> & modelPrms  ///< model parameters, depends on the given model
                                             , const std::vector<double> & mpPor      ///< for multi-point perm. model the porosity values vector. Empty for other models
                                             , const std::vector<double> & mpPerm     ///< for multi-point perm. model the log. of perm values vector. Empty for other models.
                                             ) = 0;
      /// @}

      // Thermal conductivity model parameters
      /// @{

      /// @brief Get lithology STP (Standart Temperature Pressure) thermal conductivity [W/m/K]
      /// @param[in] id lithology ID
      /// @param[out] stpThermCond 
      /// @return the STP thermal conductivity value on success or UndefinedDoubleValue on error
      virtual double stpThermalConductivityCoeff( LithologyID id ) = 0;

      /// @brief Set lithology STP (Standart Temperature Pressure) thermal conductivity [W/m/K]
      /// @param[in] id lithology ID
      /// @param[in] stpThermCond the new value for STP thermal conductivity
      /// @return NoError on success, NonexistingID on unknown lithology ID or OutOfRangeValue if the value not in an allowed range
      virtual ReturnCode setSTPThermalConductivityCoeff( LithologyID id, double stpThermCond ) = 0;

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

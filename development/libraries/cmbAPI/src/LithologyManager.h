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
//// The model has 4 parameters:
///   -# @f$ \phi_0 @f$ - surface porosity
///   -# @f$ \phi_min @f$ - minimal porosity
///   -# @f$ c1_{ef} @f$ - compaction coefficient (effective stress) @f$ [10^{-8} Pa^{-1}] @f$
///   -# @f$ c2_{ef} @f$ - compaction coefficient (effective stress) @f$ [10^{-8} Pa^{-1}] @f$
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
         Exponential,   ///< Exponential porosity model
         SoilMechanics, ///< Soil mechanics porosity model
         DoubleExponential, ///< Double Exponential porosity model 
         Unknown        ///< Not any model was defined
      } PorosityModel;


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

      /// @{
      /// Porosity model definition

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

      /// @{
      /// Thermal conductivity model parameters

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

   private:
      /// @{
      /// Copy constructor and operator are disabled, use the copyTo method instead
      LithologyManager( const LithologyManager & otherLithologyManager );
      LithologyManager & operator = ( const LithologyManager & otherLithologyManager );
      /// @}
   };
}

#endif // CMB_LITHOLOGY_MANAGER_API

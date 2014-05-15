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

#ifndef CMB_LYTHOLOGY_MANAGER_API
#define CMB_LYTHOLOGY_MANAGER_API

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
///
/// @subsection ExpPorosityModelSubSec Exponential porosity model 
/// in <b>Exponential</b> porosity model the porosity is calculated according to this formula:
/// @f[ \phi = \phi_0 exp(-c_{ef} \sigma) @f]
/// The model has 2 parameters:
///   -# @f$ \phi_0 @f$ - surface porosity
///   -# @f$ c_{ef} @f$ - inverse effective stress value @f$ [10^{-8} Pa^-1] @f$
///
/// @subsection SoilMechPorosityModelSubSec Soil mechanics porosity model
/// in <b>Soil Mechanics</b> porosity model the porosity is calculated according to this formula:
/// @f[ \phi = \frac{\psi}{1+\psi} @f] where
/// @f[ \psi = \psi_0 - \beta log( \frac{\sigma}{\sigma_0} ), \psi_0 = \frac{\phi_0}{1 - \phi_0} @f]
/// The model has 2 parameters:
///   -# @f$ \phi_0 @f$ - surface porosity
///   -# @f$ \beta @f$ - compaction coefficient

namespace mbapi {
   /// @class LithologyManager LithologyManager.h "LithologyManager.h"
   /// @brief Class LithologyManager keeps a list of lithologies in Cauldron model and allows to add/delete/edit lithology
   class LithologyManager : public ErrorHandler
   {
   public:
      
      /// @{
      /// Set of interfaces for interacting with a Cauldron model
      typedef size_t LithologyID;  ///< unique ID for lithology

      typedef enum {
         Exponential,   ///< Exponential porosity model
         SoilMechanics, ///< Soil mechanics porosity model
         Unknown        ///< Not any model was defined
      } PorosityModel;



      /// @brief Get list of lithologies in the model
      /// @return array with IDs of different lygthologies defined in the model
      virtual std::vector<LithologyID> getLithologiesID() const = 0; 

      /// @brief Create new lithology
      /// @return ID of the new Lithology
      virtual LithologyID createNewLithology() = 0;

      /// @brief Get lithology name for
      /// @param[in] id lithology ID
      /// @param[out] lythName on success has a lithology name, or empty string otherwise
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode getLithologyName( LithologyID id, std::string & lythName ) = 0;

      /// @{
      /// Porosity model definition

      /// @brief Get lithology porosity model
      /// @param[in] id lithology ID
      /// @param[out] porModel type of porosity model set for the given lithology
      /// @return NoError on success or OutOfRangeValue or NonexistingID on error
      virtual ReturnCode getPorosityModel( LithologyID id, PorosityModel & porModel ) = 0;

      /// @brief Set lithology porosity model
      /// @param[in] id lithology ID
      /// @param[out] porModel new type of porosity model for the given lithology
      /// @return NoError on success or OutOfRangeValue or NonexistingID on error
      virtual ReturnCode setPorosityModel( LithologyID id, PorosityModel porModel ) = 0;

      /// @brief Get surface porosity for Exponential or Soil Mechanic lithology porosity model
      /// @param[in] id lithology ID
      /// @param[out] surfPor Surface porosity @f$ \phi_0 @f$
      /// @return NoError on success or UndefinedValue if the value wasn't set before or NonexistingID on error
      virtual ReturnCode getSurfacePorosity( LithologyID id, double & surfPor ) = 0;

      /// @brief Set surface porosity for Exponential or Soil Mechanic lithology porosity model
      /// @param[in] id lithology ID
      /// @param[in] surfPor Surface porosity @f$ \phi_0 @f$
      /// @return NoError on success or OutOfRangeValue or NonexistingID on error
      virtual ReturnCode setSurfacePorosity( LithologyID id, double surfPor ) = 0;

      /// @brief Get compaction coefficient for Exponential @f$(c_{ef})@f$ or Soil Mechanic @f$(\beta)@f$ lithology porosity model
      /// @param[in] id lithology ID
      /// @param[out] compCoeff Compaction coefficient valued
      /// @return NoError on success or UndefinedValue if the value wasn't set before or NonexistingID on error
      virtual ReturnCode getCompactionCoeff( LithologyID id, double & compCoeff ) = 0;

      /// @brief Set compaction coefficient for Exponential @f$(c_{ef})@f$ or Soil Mechanic @f$(\beta)@f$ lithology porosity model
      /// @param[in] id lithology ID
      /// @param[in] compCoeff The new value for compaction coefficient
      /// @return NoError on success or OutOfRangeValue or NonexistingID on error
      virtual ReturnCode setCompactionCoeff( LithologyID id, double compCoeff ) = 0;

      /// @}

      /// @{
      /// Thermal conductivity model parameters

      /// @brief Get lithology STP thermal conductivity coefficient
      /// @param[in] id lithology ID
      /// @param[out] stpThermCond on success has the thermal conductivity coefficient value, or unchanged in case of error
      /// @return NoError on success, NonexistingID on unknown lithology ID or UndefinedValue if the value wasn't set before
      virtual ReturnCode getSTPThermalConductivityCoeff( LithologyID id, double & stpThermCond ) = 0;

      /// @brief Set lithology STP thermal conductivity coefficient
      /// @param[in] id lithology ID
      /// @param[in] stpThermCond the new value for the thermal conductivity coefficient 
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

#endif // CMB_LYTHOLOGY_MANAGER_API

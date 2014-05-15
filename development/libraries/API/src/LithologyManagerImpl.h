//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file LithologyManagerImpl.h
/// @brief This file keeps API implementation for manipulating Lithologies in Cauldron model

#ifndef CMB_LYTHOLOGY_MANAGER_IMPL_API
#define CMB_LYTHOLOGY_MANAGER_IMPL_API

#include "LithologyManager.h"

namespace mbapi {

   // Class LithologyManager keeps a list of lithologies in Cauldron model and allows to add/delete/edit lithology
   class LithologyManagerImpl : public LithologyManager
   {
   public:     
      // Constructors/destructor
      // brief Constructor which creates an LithologyManager
      LithologyManagerImpl();
      
      // Destructor
      virtual ~LithologyManagerImpl() {;}

      // Copy operator
      LithologyManagerImpl & operator = ( const LithologyManagerImpl & otherLythMgr );

      // Set of interfaces for interacting with a Cauldron model

      // Get list of lithologies in the model
      // return array with IDs of different lygthologies defined in the model
      virtual std::vector<LithologyID> getLithologiesID() const; 

      // Create new lithology
      // return ID of the new Lithology
      virtual LithologyID createNewLithology();

      // Get lithology name for
      // [in] id lithology ID
      // [out] lythName on success has a lithology name, or empty string otherwise
      // return NoError on success or NonexistingID on error
      virtual ReturnCode getLithologyName( LithologyID id, std::string & lythName );
      
      /////////////////////////////////////////////////////////////////////////
      // Porosity models

      // Get lithology porosity model
      // [in] id lithology ID
      // [out] porModel type of porosity model set for the given lithology
      // return NoError on success or OutOfRangeValue or NonexistingID on error
      virtual ReturnCode getPorosityModel( LithologyID id, PorosityModel & porModel );

      // Set lithology porosity model
      // [in] id lithology ID
      // [out] porModel new type of porosity model for the given lithology
      // return NoError on success or OutOfRangeValue or NonexistingID on error
      virtual ReturnCode setPorosityModel( LithologyID id, PorosityModel porModel );

      // Get surface porosity for Exponential or Soil Mechanic lithology porosity model
      // [in] id lithology ID
      // [out] surfPorosity Surface porosity @f$ \phi_0 @f$
      // return NoError on success or UndefinedValue if the value wasn't set before or NonexistingID on error
      virtual ReturnCode getSurfacePorosity( LithologyID id, double & surfPor );

      // Set surface porosity for Exponential or Soil Mechanic lithology porosity model
      // [in] id lithology ID
      // [in] surfPor Surface porosity @f$ \phi_0 @f$
      // return NoError on success or OutOfRangeValue or NonexistingID on error
      virtual ReturnCode setSurfacePorosity( LithologyID id, double surfPor );

      // Get compaction coefficient for Exponential @f$(c_{ef})@f$ or Soil Mechanic @f$(\beta)@f$ lithology porosity model
      // [in] id lithology ID
      // [out] compCoeff Compaction coefficient valued
      // return NoError on success or UndefinedValue if the value wasn't set before or NonexistingID on error
      virtual ReturnCode getCompactionCoeff( LithologyID id, double & compCoeff );

      //  Set compaction coefficient for Exponential @f$(c_{ef})@f$ or Soil Mechanic @f$(\beta)@f$ lithology porosity model
      // [in] id lithology ID
      // [in] compCoeff The new value for compaction coefficient
      // return NoError on success or OutOfRangeValue or NonexistingID on error
      virtual ReturnCode setCompactionCoeff( LithologyID id, double compCoeff );
      
      /////////////////////////////////////////////////////////////////////////
      // Thermal conductivity model

      // Set lithology STP thermal conductivity coefficient
      // [in] id lithology ID
      // [out] stpThermCond on success has the thermal conductivity coefficient value, or unchanged in case of error
      // return NoError on success, NonexistingID on unknown lithology ID or UndefinedValue if the value wasn't set before
      virtual ReturnCode getSTPThermalConductivityCoeff( LithologyID id, double & stpThermCond );

      // Set lithology STP thermal conductivity coefficient
      // [in] id lithology ID
      // [in] stpThermCond the new value for the thermal conductivity coefficient 
      // return NoError on success, NonexistingID on unknown lithology ID or OutOfRangeValue if the value not in an allowed range
      virtual ReturnCode setSTPThermalConductivityCoeff( LithologyID id, double stpThermCond );

   private:
      // Copy constructor is disabled, use the copy operator instead
      LithologyManagerImpl( const LithologyManager & );
   };
}

#endif // CMB_LYTHOLOGY_MANAGER_IMPL_API
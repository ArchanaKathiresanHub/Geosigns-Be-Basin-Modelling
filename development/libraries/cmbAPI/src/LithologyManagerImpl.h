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

#ifndef CMB_LITHOLOGY_MANAGER_IMPL_API
#define CMB_LITHOLOGY_MANAGER_IMPL_API

#include "LithologyManager.h"

namespace database
{
   class Database;
}

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

      // Set of interfaces for interacting with a Cauldron model

      // Set project database. Reset all
      void setDatabase( database::Database * db );

      // Get list of lithologies in the model
      // return array with IDs of different lygthologies defined in the model
      virtual std::vector<LithologyID> lithologiesIDs() const; 

      // Create new lithology
      // return ID of the new Lithology
      virtual LithologyID createNewLithology();

      // Get lithology type name for the given ID
      // [in] id lithology ID
      // return lithology type name for given lithology ID or empty string in case of error
      virtual std::string lithologyName( LithologyID id );
     
      /////////////////////////////////////////////////////////////////////////
      // Porosity models

      //  Get lithology porosity model
      // [in] id lithology ID
      // [out] porModel type of porosity model set for the given lithology
      // [out] porModelPrms array with porosity model parameters
      // return NoError on success or OutOfRangeValue or NonexistingID on error
      virtual ReturnCode porosityModel( LithologyID id, PorosityModel & porModel, std::vector<double> & porModelPrms );

      //  Set lithology porosity model
      // [in] id lithology ID
      // [in] porModel new type of porosity model for the given lithology
      // [in] porModelPrms porosity model parameters
      // return NoError on success or OutOfRangeValue or NonexistingID on error
      virtual ReturnCode setPorosityModel( LithologyID id, PorosityModel porModel, const std::vector<double> & porModelPrms );

      /////////////////////////////////////////////////////////////////////////
      // Thermal conductivity model

      // Set lithology STP thermal conductivity 
      // [in] id lithology ID
      // return the STP thermal conductivity value on success or UndefinedDoubleValue on error
      virtual double stpThermalConductivityCoeff( LithologyID id );

      // Set lithology STP thermal conductivity
      // [in] id lithology ID
      // [in] stpThermCond the new value for STP thermal conductivity 
      // return NoError on success, NonexistingID on unknown lithology ID or OutOfRangeValue if the value not in an allowed range
      virtual ReturnCode setSTPThermalConductivityCoeff( LithologyID id, double stpThermCond );

   private:
      // Copy constructor is disabled
      LithologyManagerImpl( const LithologyManager & );

      // Copy operator is disabled
      LithologyManagerImpl & operator = ( const LithologyManagerImpl & otherLithMgr );

      static const char * m_lithoTypesTableName;        // table name for lithologies type in project file
      static const char * m_lithoTypeNameFieldName;     // column name for lithology type name 

      static const char * m_porosityModelFieldName;     // column name for type of porosity model
      static const char * m_surfPorosityFieldName;      // column name for surface porosity (porosity model parameter)
      static const char * m_ccExponentialFieldName;     // column name for compaction coefficient of Exponential porosity model     
      static const char * m_ccaDblExponentialFieldName; // column name for compaction coefficient A of Double Exponential porosity model 
      static const char * m_ccbDblExponentialFieldName; // column name for compaction coefficient B of Double Exponential porosity model 

      static const char * m_ccSoilMechanicsFieldName;   // column name for compaction coefficient of the Soil Model porosity model
      static const char * m_minPorosityFieldName;       // column name for minimal porosity of the Double Exponential porosity model
      static const char * m_stpThermalCondFieldName;    // column name for STP thermal conductivity coefficient

      database::Database * m_db; // cauldron project database
   };
}

#endif // CMB_LITHOLOGY_MANAGER_IMPL_API

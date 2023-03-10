//
// Copyright (C) 2012-2016 Shell International Exploration & Production.
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

#include "ProjectFileHandler.h"

#include "LithologyManager.h"

namespace database
{
   class Database;
   class Table;
}

namespace mbapi {

   class StratigraphyManagerImpl;

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
      void setDatabase( database::ProjectFileHandlerPtr pfh, StratigraphyManagerImpl * stMgr );

      // Get list of lithologies in the model
      // return array with IDs of different lygthologies defined in the model
      virtual std::vector<LithologyID> lithologiesIDs() const;

      // Create new lithology
      // return ID of the new Lithology
      virtual LithologyID createNewLithology();

      // Get lithology type name for the given ID
      // return lithology type name for given lithology ID or empty string in case of error
      virtual std::string lithologyName( LithologyID id );

	  // Set lithology name for the given ID
	  // [in] id lithology ID
	  // [in] newName contains the lithology name as per BPA2 standard
	  // return NoError on success or NonexistingID on error
	  virtual ReturnCode setLithologyName(const LithologyID id, const std::string & newName);

     /// @brief get the Density
     /// @param[in] id lithology ID
     /// @return Density
     virtual double  getLitPropDensity(const LithologyID id);

     /// @brief get the Heat Production
     /// @param[in] id lithology ID
     /// @return Heat Production
     virtual double  getLitPropHeatProduction(const LithologyID id);

     /// @brief get the Thermal Conductivity
     /// @param[in] id lithology ID
     /// @return Thermal Conductivity
     virtual double  getLitPropThrConductivity(const LithologyID id);

     /// @brief get the Thermal Conductivity Anistropy
     /// @param[in] id lithology ID
     /// @return Thermal Conductivity Anistropy
     virtual double  getLitPropThrCondAnistropy(const LithologyID id);

     /// @brief get the Permeability Anistropy
     /// @param[in] id lithology ID
     /// @return Permeability Anistropy
     virtual double  getLitPropPermAnistropy(const LithologyID id);

     /// @brief get the Seismic Velocity
     /// @param[in] id lithology ID
     /// @return Seismic Velocity
     virtual double  getLitPropSeisVelocity(const LithologyID id);

     /// @brief get the Seismic Velocity Exponent
     /// @param[in] id lithology ID
     /// @return Seismic Velocity Exponent
     virtual double  getLitPropSeisVeloExponent(const LithologyID id);

     /// @brief get the Entry Pressure Coefficient 1
     /// @param[in] id lithology ID
     /// @return Entry Pressure Coefficient 1
     virtual double  getLitPropEntryPresCoeff1(const LithologyID id);

     /// @brief get the Entry Pressure Coefficient 2
     /// @param[in] id lithology ID
     /// @return Entry Pressure Coefficient 2
     virtual double  getLitPropEntryPresCoeff2(const LithologyID id);

     /// @brief get the Hydraulic Fracturing
     /// @param[in] id lithology ID
     /// @return Hydraulic Fracturing
     virtual double  getLitPropHydFracturing(const LithologyID id);

     /// @brief get the ReferenceSolidViscosity
     /// @param[in] id lithology ID
     /// @return ReferenceSolidViscosity
     virtual double  getLitPropRefSoldViscosity(const LithologyID id);

     /// @brief get the Intrusion Temperature
     /// @param[in] id lithology ID
     /// @return Intrusion Temperature
     virtual double  getLitPropIntrTemperature(const LithologyID id);

     /// @brief set the Density
     /// @param[in] id lithology ID
     /// @param[in] value Density 
     // return NoError on success or NonexistingID on error
     virtual ReturnCode  setLitPropDensity(const LithologyID id, double & value);

     /// @brief set the Heat Production
     /// @param[in] id lithology ID
     /// @param[in] value Heat Production
     // return NoError on success or NonexistingID on error
     virtual ReturnCode  setLitPropHeatProduction(const LithologyID id, double & value);

     /// @brief set the Thermal Conductivity
     /// @param[in] id lithology ID
     /// @param[in] value Thermal Conductivity
     // return NoError on success or NonexistingID on error
     virtual ReturnCode  setLitPropThrConductivity(const LithologyID id, double & value);

     /// @brief set the Thermal Conductivity Anistropy
     /// @param[in] id lithology ID
     /// @param[in] value Thermal Conductivity Anistropy
     // return NoError on success or NonexistingID on error
     virtual ReturnCode  setLitPropThrCondAnistropy(const LithologyID id, double & value);

     /// @brief set the Permeability Anistropy
     /// @param[in] id lithology ID
     /// @param[in] value Permeability Anistropy
     // return NoError on success or NonexistingID on error
     virtual ReturnCode  setLitPropPermAnistropy(const LithologyID id, double & value);

     /// @brief set the Seismic Velocity
     /// @param[in] id lithology ID
     /// @param[in] value Seismic Velocity
     // return NoError on success or NonexistingID on error
     virtual ReturnCode  setLitPropSeisVelocity(const LithologyID id, double & value);

     /// @brief set the Seismic Velocity Exponent
     /// @param[in] id lithology ID
     /// @param[in] value Seismic Velocity Exponent
     // return NoError on success or NonexistingID on error
     virtual ReturnCode  setLitPropSeisVeloExponent(const LithologyID id, double & value);

     /// @brief set the Entry Pressure Coefficient 1
     /// @param[in] id lithology ID
     /// @param[in] value Entry Pressure Coefficient 1
     // return NoError on success or NonexistingID on error
     virtual ReturnCode  setLitPropEntryPresCoeff1(const LithologyID id, double & value);

     /// @brief set the Entry Pressure Coefficient 2
     /// @param[in] id lithology ID
     /// @param[in] value Entry Pressure Coefficient 2
     // return NoError on success or NonexistingID on error
     virtual ReturnCode  setLitPropEntryPresCoeff2(const LithologyID id, double & value);

     /// @brief set the Hydraulic Fracturing
     /// @param[in] id lithology ID
     /// @param[in] value Hydraulic Fracturing
     // return NoError on success or NonexistingID on error
     virtual ReturnCode  setLitPropHydFracturing(const LithologyID id, double & value);

     /// @brief set the ReferenceSolidViscosity
     /// @param[in] id lithology ID
     /// @param[in] value ReferenceSolidViscosity
     // return NoError on success or NonexistingID on error
     virtual ReturnCode  setLitPropRefSoldViscosity(const LithologyID id, double & value);

     /// @brief set the Intrusion Temperature
     /// @param[in] id lithology ID
     /// @param[in] value Intrusion Temperature
     // return NoError on success or NonexistingID on error
     virtual ReturnCode  setLitPropIntrTemperature(const LithologyID id, double & value);

	  /// @brief get user defined flag value for the given lithology ID
	  /// @param[in] id lithology ID
	  /// @param[out] flag user defined flag value for the particular lithology id
	  /// @return NoError on success or NonexistingID on error
	  virtual ReturnCode getUserDefinedFlagForLithology(const LithologyID id, int & flag);

	  /// @brief get reference lithology name for the given lithology ID
	  /// @param[in] id lithology ID
	  /// @param[out] referenceLithology reference lithology for the particular lithology id
	  /// @return NoError on success or NonexistingID on error
	  virtual ReturnCode getReferenceLithology(const LithologyID id, std::string & referenceLithology);

      // Get lithology description for the given ID
      // return lithology description for given lithology ID or empty string in case of error
      virtual std::string getDescription( const LithologyID id );

      // Set lithology description for the given ID
      // [in] id lithology ID
      // [in] myDescription contains lithology description
      // return NoError on success or NonexistingID on error
      virtual ReturnCode setDescription( const LithologyID id, const std::string & myDescription);

      // Search for lithology record which has given lithology name
      // return ID of found lithology on success or UndefinedIDValue otherwise
      virtual LithologyID findID( const std::string & lName );

      // Make a copy of the given lithology. Also makes a new set of records in table [LitThCondIoTbl] for the new litholog
      //        If there is another lithology with the same as given new name, method will fail.
      // return new lithology ID on success or UndefinedIDValue on error
      virtual LithologyID copyLithology( LithologyID id, const std::string & newLithoName );

      // Check does this lithology has references in StratIoTbl, FaultCut and AlochtLith tables, then delete
      // this lithology from the Lithology table and delete all records from ThermoCond/ThermoCapacity tables
      // return NoError on success or error code if this lithology is referenced in other tables
      virtual ReturnCode deleteLithology( LithologyID id );


      // Scan lithology table for duplicated lithologies and delete them updating references
      // return NoError on success or error code otherwise
      virtual ReturnCode cleanDuplicatedLithologies();

      // Allochton lithology methods

      // Get list of allochton lithologies in the model
      // return array with IDs of allochton lygthologies defined in the model
      virtual std::vector<AllochtLithologyID> allochtonLithologiesIDs() const;

      // Search in AllochthonLithoIoTbl table for the given layer name
      // AllochthonLithologyID for the found lithology on success, UndefinedIDValue otherwise
      virtual AllochtLithologyID findAllochtID( const std::string & layerName );

      // Get lithlogy name for the allochton lithology
      // return Name of the allochton lithology on success or empty string on error.
      virtual std::string allochtonLithology( AllochtLithologyID alID );

      // Get layer name for the allochton lithology
      // return Name of the layer for allochton lithology on success or empty strin on error
      virtual std::string allochtonLithologyLayerName( AllochtLithologyID alID );

      // Set new allochton lithology for the layer
      // return ErrorHandler::NoError on success, error code otherwise
      virtual ReturnCode setAllochtonLithology( AllochtLithologyID alID, const std::string & newLithoName );

	  /// @brief Get list of thermal conductiviries for all the lithotypes used in the model from LitThCondIoTbl
	  /// @return array with IDs of different lithologies defined in the model
	  virtual std::vector<LitThCondTblID> thermCondLithologiesIDs() const;

	  /// @brief Get lithology name from LitThCondIoTbl
	  /// @param[in] id lithology ID
	  /// @param[out] LithoName on success has a lithology name
	  /// @return NoError on success or NonexistingID on error
	  virtual ReturnCode getThermCondTableLithoName(const LitThCondTblID id, std::string & LithoName);

	  /// @brief Set lithotype name in the LitThCondIoTbl
	  /// @param[in] id lithology ID
	  /// @param[in] LithoName new name of the lithology to set for the lithology ID
	  /// @return NoError on success or NonexistingID on error
	  virtual ReturnCode setThermCondTableLithoName(const LitThCondTblID id, const std::string & LithoName);

	  /// @brief Get list of heat capacities for all the lithotypes used in the model from LitHeatCapIoTbl
	  /// @return array with IDs of different lithologies defined in the model
	  virtual std::vector<LitHeatCapTblID> heatCapLithologiesIDs() const;

	  /// @brief Get lithology name from LitHeatCapIoTbl
	  /// @param[in] id lithology ID
	  /// @param[out] LithoName on success has a lithology name
	  /// @return NoError on success or NonexistingID on error
	  virtual ReturnCode getHeatCapTableLithoName(const LitThCondTblID id, std::string & LithoName);

	  /// @brief Set lithotype name in the LitHeatCapIoTbl
	  /// @param[in] id lithology ID
	  /// @param[in] LithoName new name of the lithology to set for the lithology ID
	  /// @return NoError on success or NonexistingID on error
	  virtual ReturnCode setHeatCapTableLithoName(const LitThCondTblID id, const std::string & LithoName);


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
      // Permeability models

      // Get lithology permeability model
      // [in] id          lithology ID
      // [out] prmModel   permeability calculation model
      // [out] modelPrms  model parameters, depends on the given model
      // [out] mpPor      for multi-point perm. model the porosity values vector. Empty for other models
      // [out] mpPerm     for multi-point perm. model the log. of perm values vector. Empty for other models.
      // return NoError on success or error code otherwise
      virtual ReturnCode permeabilityModel( LithologyID           id
                                          , PermeabilityModel   & prmModel
                                          , std::vector<double> & modelPrms
                                          , std::vector<double> & mpPor
                                          , std::vector<double> & mpPerm
                                          );

      // Get lithology permeability model parameters
      // [in] id          lithology ID
      // [out] prmModel   permeability calculation model
      // [out] mpPor      for multi-point perm. model the porosity values vector. Empty for other models
      // [out] mpPerm     for multi-point perm. model the log. of perm values vector. Empty for other models.
      // [out] numPts      for multi-point perm. Number_Of_Data_Points
      // return NoError on success or error code otherwise
      virtual ReturnCode getPermeabilityModel(LithologyID id, PermeabilityModel & prmModel, std::vector<double> & mpPor, std::vector<double> & mpPerm, int & numPts);

      // Set lithology permeability model with parameters
      // id        lithology ID
      // prmModel  permeability calculation model
      // modelPrms model parameters, depends on the given model
      // mpPor     for multi-point perm. model the porosity values vector. Empty for other models
      // mpPerm    for multi-point perm. model the log. of perm values vector. Empty for other models.
      // return NoError on success or error code otherwise
      virtual ReturnCode setPermeabilityModel( LithologyID                 id
                                             , PermeabilityModel           prmModel
                                             , const std::vector<double> & modelPrms
                                             , const std::vector<double> & mpPor
                                             , const std::vector<double> & mpPerm
                                             );
      /// @}

      // Set lithology permeability model with parameters
      // [in] id              lithology ID
      // [in/out] prmModel    permeability calculation model
      ///return NoError on success or error code otherwise
      // [out] mpPor     for multi-point perm. model the porosity values vector
      // [out] mpPerm    for multi-point perm. model the log. of perm values vector
      // [in] numPts     for multi-point perm. Number_Of_Data_Points
      // [in] flag      user defined flag value for the particular lithology id
      virtual ReturnCode setPermeabilityModel(LithologyID id, PermeabilityModel prmModel, std::vector<double> & mpPor, std::vector<double> & mpPerm, int & numPts, int & flag);
      /// @}

      /////////////////////////////////////////////////////////////////////////
      // Seismic velocity

      // Get lithology permeability model
      // [in] id          lithology ID
      // return the value on success or UndefinedDoubleValue otherwise
      virtual double seisVelocity( LithologyID id );

      /// @}

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

      // clean records from thermal conductivity and heat capacity tables for the given lithology
      void cleanHeatCoeffTbls( const char * tblName, const std::string & lithoName );
      // duplicate records in thermal conductivity and heat capacity tables for the given lithology
      void copyRecordsHeatCoeffTbls( const char * tblName, const std::string & origLithoName, const std::string & newLithoName );


      static const char * s_lithoTypesTableName;        // table name for lithologies type in project file
      static const char * s_lithoTypeNameFieldName;     // column name for lithology type name
      static const char * s_descriptionFieldName;		  // column name for lithology description
	  static const char * s_userDefinedFlagFieldName;		  // column name for lithology description
	  static const char * s_definedByFieldName;		  // column name for reference lithology in the LithotypeIoTbl

      static const char * s_porosityModelFieldName;             // column name for type of porosity model
      static const char * s_surfPorosityFieldName;              // column name for surface porosity (porosity model parameter)
      static const char * s_ccExponentialFieldName;             // column name for compaction coefficient of Exponential porosity model
      static const char * s_ccaDblExponentialFieldName;         // column name for compaction coefficient A of Double Exponential porosity model
      static const char * s_ccbDblExponentialFieldName;         // column name for compaction coefficient B of Double Exponential porosity model
      static const char * s_compacRatioDblExponentialFieldName; // column name for compaction ratio of Double Exponential porosity model

      static const char * s_ccSoilMechanicsFieldName;   // column name for compaction coefficient of the Soil Model porosity model
      static const char * s_minPorosityFieldName;       // column name for minimal porosity of the Double Exponential porosity model
      static const char * s_stpThermalCondFieldName;    // column name for STP thermal conductivity coefficient
      static const char * s_seisVelocityFieldName;      // column name for the seismic velocity coefficent

      static const char * s_permeabilityModelFieldName;        // column name for permeability model
      static const char * s_permeabilityAnisotropyFieldName;   // column name for permeability anisotropy coefficient
      static const char * s_DepositionalPermFieldName;         // column name for depositional permeability [mD]
      static const char * s_mpNumberOfDataPointsFieldName;     // column name for the number of data points in perm. vs. porosity profile
      static const char * s_mpPorosityFieldName;               // column name for porosity table values list
      static const char * s_mpPermpeabilityFieldName;          // column name for permeability table values list
      static const char * s_mudPermeabilityRecoveryCoeff;      // column name for permeability recovery coeff
      static const char * s_mudPermeabilitySensitivityCoeff;   // column name for permeability sensitivity coeff
      static const char * s_permSandClayPercentage;            // column name for sand clay percentage

      static const char * s_lithoThCondTableName;       // table name for lithology tabulated thermo conductivity function
      static const char * s_lithoHeatCapTableName;      // table name for lithology tabulated heat capacity function
      static const char * s_LithotypeFieldName;         // column name for lithology name
      static const char * s_TempIndexFieldName;         // column name for temperature index
      static const char * s_ThCondFieldName;            // column name for thermal conductivity value
      static const char * s_HeatCapacityFieldName;      // column name for heat capacity value

      static const char * s_allochtLithTableName;       // table name for the list of allochtonous lithologies
      static const char * s_allochtLayerNameFieldName;  // column name for layer name
      static const char * s_allochtLithotypeFieldName;  // column name for lithology name

      static const char * s_DensityFieldName; // column name for Density value
      static const char * s_HeatProductionFieldName; // column name for Heat Production
      static const char * s_stpThermalCondAnisotrophyFieldName; // column name for Thermal Conductivity Anistropy
      static const char * s_seisVeloExponentFieldName; // column name for Seismic Velocity Exponent
      static const char * s_capEntryPresCoeff1FieldName; // column name for Entry Pressure Coefficient 1
      static const char * s_capEntryPresCoeff2FieldName; // column name for Entry Pressure Coefficient 2
      static const char * s_hrdrFracturingFieldName; // column name for Hydraulic Fracturing
      static const char * s_refSolidViscosityFieldName; // column name for Reference Solid Viscosity
      static const char  * s_IgnIntruTemperatureFieldName; // column name for Intrusion Temperature

      database::ProjectFileHandlerPtr m_db;          // cauldron project database
      database::Table               * m_lithIoTbl;   // lithology Io table
      database::Table               * m_alLithIoTbl; // allochton lithology Io table
	  database::Table               * m_lithoThCondIoTbl; // thermalConductivity Io table for lithologies
	  database::Table               * m_lithoHeatCapIoTbl; // thermalConductivity Io table for lithologies
      StratigraphyManagerImpl       * m_stMgr;       // Stratigraphy manager
   };                                                        }

#endif // CMB_LITHOLOGY_MANAGER_IMPL_API

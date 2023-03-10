//
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

/// @file SourceRockManagerImpl.h
/// @brief This file keeps API implementation for manipulating source rocks in Cauldron model

#ifndef CMB_SOURCE_ROCK_MANAGER_IMPL_API
#define CMB_SOURCE_ROCK_MANAGER_IMPL_API

#include "ProjectFileHandler.h"

#include "SourceRockManager.h"
#include "database.h"

namespace mbapi {

   // Class SourceRockManager keeps a list of source rocks in Cauldron model and allows to add/delete/edit source rock
   class SourceRockManagerImpl : public SourceRockManager
   {
   public:
      // Constructors/destructor
      // brief Constructor which creates an SourceRockManager
      SourceRockManagerImpl();

      // Destructor
      virtual ~SourceRockManagerImpl() {;}

      // Copy operator
      SourceRockManagerImpl & operator = ( const SourceRockManagerImpl & otherSrRockMgr );

      // Set of interfaces for interacting with a Cauldron model

      // Set project database. Reset all
      void setDatabase( database::ProjectFileHandlerPtr pfh );

      // Get list of source rocks in the model
      // return array with IDs of different lithologies defined in the model
      virtual std::vector<SourceRockID> sourceRockIDs( ) const;

      // Search for source rock lithology record which has given layer name and source rock type name
      // return ID of found source rock lithology on success or UndefinedIDValue otherwise
      virtual SourceRockID findID( const std::string & lName, const std::string & srTypeName );

      // Create new source rock
      // return ID of the new source rock lithology
      virtual SourceRockID createNewSourceRockLithology( );

      // Get layer name for given ID
      virtual std::string layerName( SourceRockID id );

      // Get source rock type name for the given ID
      virtual std::string sourceRockType( SourceRockID id );

	  // Get source rock type name for the given ID
	  virtual ReturnCode setSourceRockType(SourceRockID id, const std::string & newSourceRockType);

	  // Delete source for the given ID from the table
	  virtual ReturnCode deleteSourceRockRecord(SourceRockID id, database::Table::iterator& newIt);

	  // Get base source rock type name for the given ID
	  virtual std::string baseSourceRockType(SourceRockID id);

	  // Get source rock type name for the given ID
	  virtual ReturnCode setBaseSourceRockType(SourceRockID id, const std::string & newBaseSourceRockType);

      // Get total organic contents value ( must be in range 0-100 percent) for the given source rock lithology
      virtual double tocIni( SourceRockID id );

      // Get TOC map name for the given source rock lithology
      virtual std::string tocInitMapName( SourceRockID id );

      // Set total organic contents value ( must be in range 0-100 percent) for all source
      // rock lithologies associated with the given layer
      virtual ReturnCode setTOCIni( SourceRockID id, double newTOC );

      // Set TOC map name for the given source rock lithology
      virtual ReturnCode setTOCInitMapName( SourceRockID id, const std::string & mapName );

      // Source rock HI (hydrogen index initial ratio) API
      virtual double hiIni( SourceRockID id );

      //  Set hydrogen index initial ratio value ( must be in range 0-1000 kg/tonne) for all source rock lithologies associated with the given layer
      virtual ReturnCode setHIIni( SourceRockID id, double newHI );

      // Get H/C initial ratio 
      virtual double hcIni( SourceRockID id );

      // Set H/C initial ratio value 
      virtual ReturnCode setHCIni( SourceRockID id, double newHC );

	  // Get pre-asphaltene activation energy [kJ/mol]
	  virtual double scIni(SourceRockID id);

	  // Set pre-asphaltene activation energy 
	  virtual ReturnCode setSCIni(SourceRockID id, double newVal);

      // Get pre-asphaltene activation energy [kJ/mol]
      virtual double preAsphActEnergy( SourceRockID id );

      // Set pre-asphaltene activation energy (must be in range 200-220 kJ/mol)
      virtual ReturnCode setPreAsphActEnergy( SourceRockID id, double newVal );	 

	  // Get asphaltene diffusion energy [kJ/mol]
	  virtual double getAsphalteneDiffusionEnergy(SourceRockID id);

	  // Set asphaltene diffusion energy 
	  virtual ReturnCode setAsphalteneDiffusionEnergy(SourceRockID id, double newVal);

	  // Get resin diffusion energy [kJ/mol]
	  virtual double getResinDiffusionEnergy(SourceRockID id);

	  // Set resin diffusion energy 
	  virtual ReturnCode setResinDiffusionEnergy(SourceRockID id, double newVal);

	  // Get C15Aro diffusion energy [kJ/mol]
	  virtual double getC15AroDiffusionEnergy(SourceRockID id);

	  // Set C15Aro diffusion energy 
	  virtual ReturnCode setC15AroDiffusionEnergy(SourceRockID id, double newVal);

	  // Get C15Sat diffusion energy [kJ/mol]
	  virtual double getC15SatDiffusionEnergy(SourceRockID id);

	  // Set C15Sat diffusion energy 
	  virtual ReturnCode setC15SatDiffusionEnergy(SourceRockID id, double newVal);	

	  // Get VRE optimization string
	  virtual std::string getVREoptimization(SourceRockID id);

	  // Get VRE threshold value
	  virtual double getVREthreshold(SourceRockID id);

	  // Set VRE threshold value
	  virtual ReturnCode setVREthreshold(SourceRockID id, double newVES);

	  // Get VES limit indicator string
	  virtual std::string getVESlimitIndicator(SourceRockID id);

	  // Get VES limit value (MPa)
	  virtual double getVESlimit(SourceRockID id);

	  // Set VES limit value (must be in range 0-70 MPa)
	  virtual ReturnCode setVESlimit(SourceRockID id, double newVES);
	  
	  // Get adsorption related fields
	  virtual ReturnCode getAdsoptionList(SourceRockID id, int & applyAdsorption, int & adsorptionTOCDependent, int & computeOTGC, std::string & adsorptionCapacityFunctionName);

	  // Set adsorption TOC Dependent fields
	  virtual ReturnCode setAdsorptionTOCDependent(SourceRockID id, int adsorptionTOCDependent);

	  // Set adsorption capacity function name fields
	  virtual ReturnCode setAdsorptionCapacityFunctionName(SourceRockID id, const std::string & adsorptionCapacityFunctionName);

	  // Set adsorption simulator name fields
	  virtual ReturnCode setWhichAdsorptionSimulator(SourceRockID id, const std::string & whichAdsorptionSimulator);


   private:
      static const char * s_sourceRockTableName;     // table name for source rock lithologies in project file
      static const char * s_layerNameFieldName;      // name of the field which keeps layer name
      static const char * s_sourceRockTypeFieldName; // name of the field which keeps source rock type name
	  static const char * s_baseSourceRockTypeFieldName; // name of the field which keeps base source rock type name
      static const char * s_tocIni;                  // initial TOC
      static const char * s_tocIniMap;               // initial TOC map name
      static const char * s_hcIni;                   // initial H/C ratio
	  static const char * s_scIni;                   // initial H/C ratio
      static const char * s_PreAsphaltStartAct;      // pre-asphaltene activation energy
	  static const char * s_applyAdsorption;		 // apply adsorption flag
	  static const char * s_adsorptionTOCDependent;   // apply TOC dependent flag 
	  static const char * s_computeOTGC;			  // compute OTGC flag
	  static const char * s_adsorptionCapacityFunctionName; // adsorption capacity function name
	  static const char * s_whichAdsorptionSimulator;	// adsorption simulator name
	  static const char * s_asphalteneDiffusionEnergy;	// asphaltene diffusion energy
	  static const char * s_resinDiffusionEnergy;	// resin diffusion energy
	  static const char * s_C15AroDiffusionEnergy;	// C15Aro diffusion energy
	  static const char * s_C15SatDiffusionEnergy;	// C15Sat diffusion energy
	  static const char * s_vreOptimization;   // VRE optimization
	  static const char * s_vreThreshold;    // VRE threshold
	  static const char * s_vesLimitIndicator;   // VES limit indicator
	  static const char * s_vesLimit;    // VES limit 

      // Copy constructor is disabled, use the copy operator instead
      SourceRockManagerImpl( const SourceRockManager & );

      database::ProjectFileHandlerPtr m_db; // cauldron project database
   };
}

#endif // CMB_SOURCE_ROCK_MANAGER_IMPL_API

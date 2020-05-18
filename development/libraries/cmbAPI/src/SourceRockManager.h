//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file SourceRockManager.h
/// @brief This file keeps API declaration for manipulating source rocks in Cauldron model

#ifndef CMB_SOURCE_ROCK_MANAGER_API
#define CMB_SOURCE_ROCK_MANAGER_API

#include <vector>
#include "ErrorHandler.h"
#include "database.h"
/// @page SourceRockManagerPage Source Rock Manager
/// @link mbapi::SourceRockManager Source rock manager @endlink provides set of interfaces to create/delete/edit
/// list various source rocks in the data model. Also it has set of interfaces to get/set property of any source
/// rock from the list

namespace mbapi {
   /// @class SourceRockManager SourceRockManager.h "SourceRockManager.h"
   /// @brief Class SourceRockManager keeps a list of source rocks in Cauldron model and allows to add/delete/edit source rock
   class SourceRockManager : public ErrorHandler
   {
   public:
      
      /// @{
      /// Set of interfaces for interacting with a source rock lithologies

      typedef size_t SourceRockID;  ///< unique ID for source rock

      /// @brief Get list of source rocks in the model
      /// @return array with IDs of different source rocks defined in the model
      virtual std::vector<SourceRockID> sourceRockIDs() const = 0; 

      /// @brief Search for source rock lithology record which has given layer name and source rock type name
      /// @param lName layer name
      /// @param srTypeName name of source rock lithology
      /// @return ID of found source rock lithology on success or UndefinedIDValue otherwise
      virtual SourceRockID findID( const std::string & lName, const std::string & srTypeName ) = 0;

      /// @brief Create new source rock lithology record
      /// @return ID of the new source rock lithology
      virtual SourceRockID createNewSourceRockLithology() = 0;

      /// @brief Get layer name for given ID
      /// @param id source rock ID
      /// @return layer name for given source rock lithology ID or empty string in case of error
      virtual std::string layerName( SourceRockID id ) = 0;

      /// @brief Get source rock type name for the given ID
      /// @param id source rock ID
      /// @return source rock type name for the given source rock lithology ID or empty string in case of error
      virtual std::string sourceRockType( SourceRockID id ) = 0;
	  
	  // Delete source for the given ID from the table
	  virtual ReturnCode deleteSourceRockRecord(SourceRockID id, database::Table::iterator& newIt)=0;
	  
	  /// @brief Set source rock type name for the given ID
	  /// @param id source rock ID
	  /// @param newSourceRockType new source rock type
	  /// @return ErrorHandler::NoError on success, error code otherwise
	  virtual ReturnCode setSourceRockType(SourceRockID id, const std::string & newSourceRockType) = 0;

	  /// @brief Get base source rock type name for the given ID
	  /// @param id source rock ID
	  /// @return base source rock type name for the given source rock lithology ID or empty string in case of error
	  virtual std::string baseSourceRockType(SourceRockID id) = 0;

	  /// @brief Set source rock type name for the given ID
	  /// @param id source rock ID
	  /// @param newBaseSourceRockType new base source rock type
	  /// @return ErrorHandler::NoError on success, error code otherwise
	  virtual ReturnCode setBaseSourceRockType(SourceRockID id, const std::string & newBaseSourceRockType) = 0;

      /// @}

      /// @{
      /// Source rock TOC (total organic content) API 
      ///
      /// @brief Get total organic contents value ( must be in range 0-100 percent) for the given source rock lithology
      /// @param[in] id source rock ID
      /// @return TOC value for given source rock lithology ID or UndefinedDoubleValue in case of error
      virtual double tocIni( SourceRockID id ) = 0;

      /// @brief Get TOC map name for the given source rock lithology
      /// @param id source rock ID
      /// @return on success TOC map name (if it was defined or empty string otherwise)
      virtual std::string tocInitMapName( SourceRockID id ) = 0;

      /// @brief Set total organic contents value ( must be in range 0-100 percent) for all source
      ///        rock lithologies associated with the given layer
      /// @param id source rock ID
      /// @param newTOC new TOC value
      /// @return ErrorHandler::NoError on success, error code otherwise
      virtual ReturnCode setTOCIni( SourceRockID id, double newTOC ) = 0;

      /// @brief Set TOC map name for the given source rock lithology
      /// @param id source rock ID
      /// @param mapName new TOC map name
      /// @return ErrorHandler::NoError on success, error code otherwise
      virtual ReturnCode setTOCInitMapName( SourceRockID id, const std::string & mapName ) = 0;

      /// @}

      /// @{
      /// Source rock HI (hydrogen index initial ratio) API
      ///
      /// @brief Get HI value for the given source rock lithology
      /// @param id source rock ID
      /// @return HI value for given source rock lithology ID or UndefinedDoubleValue in case of error
      virtual double hiIni( SourceRockID id ) = 0;

      /// @brief Set hydrogen index initial ratio value ( must be in range 0-1000 kg/tonne) for all source
      ///        rock lithologies associated with the given layer
      /// @param id source rock ID
      /// @param newHI new HI value ( must be in range 0-1000 kg/tonne)
      /// @return ErrorHandler::NoError on success, error code otherwise
      virtual ReturnCode setHIIni( SourceRockID id, double newHI ) = 0;
      /// @}	 

      /// @{
      /// Source rock H/C initial ratio API 

      /// @brief Get H/C initial ratio
      /// @param id source rock ID
      /// @return H/C value for given source rock lithology ID or UndefinedDoubleValue in case of error
      virtual double hcIni( SourceRockID id ) = 0;

      /// @brief Set H/C initial ratio value  (must be in range 0-2)
      /// @param id source rock ID
      /// @param newHC new H/C ratio value
      /// @return ErrorHandler::NoError on success, error code otherwise
      virtual ReturnCode setHCIni( SourceRockID id, double newHC ) = 0;
      /// @}

	  /// @{
	  /// Source rock S/C initial ratio API 

	  /// @brief Get S/C initial ratio
	  /// @param id source rock ID
	  /// @return S/C value for given source rock lithology ID or UndefinedDoubleValue in case of error
	  virtual double scIni(SourceRockID id) = 0;

	  /// @brief Set S/C initial ratio value  (must be in range 0-2)
	  /// @param id source rock ID
	  /// @param newHC new S/C ratio value
	  /// @return ErrorHandler::NoError on success, error code otherwise
	  virtual ReturnCode setSCIni(SourceRockID id, double newVal) = 0;
	  /// @}

      /// @{
      /// @Source rock pre-asphaltene activation energy API
      /// @brief Get pre-asphaltene activation energy [kJ/mol]
      /// @param id source rock ID
      /// @return pre-asphaltene activation energy value for given source rock lithology ID or UndefinedDoubleValue in case of error
      virtual double preAsphActEnergy( SourceRockID id ) = 0;

      /// @brief Set pre-asphaltene activation energy (must be in range 200-220 kJ/mol)
      /// @param id source rock ID
      /// @param newVal new pre-asphaltene activation energy value
      /// @return ErrorHandler::NoError on success, error code otherwise
      virtual ReturnCode setPreAsphActEnergy( SourceRockID id, double newVal ) = 0;
      /// @}

	  /// @{
	  /// @Source rock Asphaltene diffusion energy API
	  /// @brief Get asphaltene diffusion energy [kJ/mol]
	  /// @param id source rock ID
	  /// @return asphaltene diffusion energy value for given source rock lithology ID or UndefinedDoubleValue in case of error
	  virtual double getAsphalteneDiffusionEnergy(SourceRockID id) = 0;

	  /// @brief Set asphaltene diffusion energy [kJ/mol]
	  /// @param id source rock ID
	  /// @param newVal new asphaltene diffusion energy value
	  /// @return ErrorHandler::NoError on success, error code otherwise
	  virtual ReturnCode setAsphalteneDiffusionEnergy(SourceRockID id, double newVal) = 0;

	  /// @Source rock Resin diffusion energy API
	  /// @brief Get resin diffusion energy [kJ/mol]
	  /// @param id source rock ID
	  /// @return resin diffusion energy value for given source rock lithology ID or UndefinedDoubleValue in case of error
	  virtual double getResinDiffusionEnergy(SourceRockID id) = 0;

	  /// @brief Set resin diffusion energy [kJ/mol]
	  /// @param id source rock ID
	  /// @param newVal new resin diffusion energy value
	  /// @return ErrorHandler::NoError on success, error code otherwise
	  virtual ReturnCode setResinDiffusionEnergy(SourceRockID id, double newVal) = 0;

	  /// @Source rock C15Aron diffusion energy API
	  /// @brief Get C15Aro diffusion energy [kJ/mol]
	  /// @param id source rock ID
	  /// @return C15Aro diffusion energy value for given source rock lithology ID or UndefinedDoubleValue in case of error
	  virtual double getC15AroDiffusionEnergy(SourceRockID id) = 0;

	  /// @brief Set C15Aro diffusion energy [kJ/mol]
	  /// @param id source rock ID
	  /// @param newVal new C15Aro diffusion energy value
	  /// @return ErrorHandler::NoError on success, error code otherwise
	  virtual ReturnCode setC15AroDiffusionEnergy(SourceRockID id, double newVal) = 0;

	  /// @Source rock C15Sat diffusion energy API
	  /// @brief Get C15Sat diffusion energy [kJ/mol]
	  /// @param id source rock ID
	  /// @return C15Sat diffusion energy value for given source rock lithology ID or UndefinedDoubleValue in case of error
	  virtual double getC15SatDiffusionEnergy(SourceRockID id) = 0;

	  /// @brief Set C15Sat diffusion energy [kJ/mol]
	  /// @param id source rock ID
	  /// @param newVal new C15Sat diffusion energy value
	  /// @return ErrorHandler::NoError on success, error code otherwise
	  virtual ReturnCode setC15SatDiffusionEnergy(SourceRockID id, double newVal) = 0;
	  /// @}

	  /// @{
	  /// @brief Get VRE optimization 
	  /// @param[in] id source rock ID
	  /// @return on success VRE optimization string (if it was defined or empty string otherwise)
	  virtual std::string getVREoptimization(SourceRockID id) = 0;
	  /// @}

	  /// @{
	  /// @brief Get VRE threshold
	  /// @param[in] id source rock ID
	  /// @return VRE threshold value for given source rock lithology ID or UndefinedDoubleValue in case of error
	  virtual double getVREthreshold(SourceRockID id) = 0;
	  /// @}

	  /// @{
	  /// @brief Set VRE threshold value
	  /// @param[in] id source rock ID
	  /// @param[in] newVRE new VRE threshold value
	  /// @return ErrorHandler::NoError on success, error code otherwise
	  virtual ReturnCode setVREthreshold(SourceRockID id, double newVRE) = 0;
	  /// @}

	  /// @{
	  /// @brief Get VES limit indicator 
	  /// @param[in] id source rock ID
	  /// @return on success VES limit indicator string (if it was defined or empty string otherwise)
	  virtual std::string getVESlimitIndicator(SourceRockID id) = 0;
	  /// @}

	  /// @{
	  /// @brief Get VES limit 
	  /// @param[in] id source rock ID
	  /// @return VES limit value for given source rock lithology ID or UndefinedDoubleValue in case of error
	  virtual double getVESlimit(SourceRockID id) = 0;
	  /// @}

	  /// @{
	  /// @brief Set VES limit value  (must be in range 0-70 MPa)
	  /// @param[in] id source rock ID
	  /// @param[in] newVES new VES limit value
	  /// @return ErrorHandler::NoError on success, error code otherwise
	  virtual ReturnCode setVESlimit(SourceRockID id, double newVES) = 0;
	  /// @}

	  /// @{
	  /// @Source rock adsorption related field API
      /// @brief Get adsorption related fields
      /// @param[in] id source rock ID
	  /// @param[out] applyAdsorption source rock adsorption is active or not 
	  /// @param[out] adsorptionTOCDependent source rock adsorption TOC dependent or not
	  /// @param[out] computeOTGC source rock compute OTGC is active or not
	  /// @param[out] adsorptionCapacityFunctionName source rock adsorption capacity function name 
      /// @return ErrorHandler::NoError on success, error code otherwise
	  virtual ReturnCode getAdsoptionList(SourceRockID id, int & applyAdsorption, int & adsorptionTOCDependent, int & computeOTGC, std::string & adsorptionCapacityFunctionName) = 0;
	  /// @}

	  /// @brief Set adsorption TOC dependent
	  /// @param[in] id source rock ID
	  /// @param[in] newAdsorptionTOCDependent new adsorption toc dependent value
	  /// @return ErrorHandler::NoError on success, error code otherwise
	  virtual ReturnCode setAdsorptionTOCDependent(SourceRockID id, int newAdsorptionTOCDependent) = 0;
	  /// @}

	  /// @brief Set adsorption capacity function name 
	  /// @param[in] id source rock ID
	  /// @param[in] newAdsorptionCapacityFunctionName new adsorption capacity function name
	  /// @return ErrorHandler::NoError on success, error code otherwise
	  virtual ReturnCode setAdsorptionCapacityFunctionName(SourceRockID id, const std::string & newAdsorptionCapacityFunctionName) = 0;
	  /// @}

	  /// @brief Set adsorption simulator name
	  /// @param[in] id source rock ID
	  /// @param[in] newWhichAdsorptionSimulator new adsorption simulator name
	  /// @return ErrorHandler::NoError on success, error code otherwise
	  virtual ReturnCode setWhichAdsorptionSimulator(SourceRockID id, const std::string & newWhichAdsorptionSimulator) = 0;
	  /// @}

   protected:
      /// @name Constructors/destructor
      /// @{
      /// @brief Constructor which creates an empty model
      SourceRockManager() {;}
      
      /// @brief Destructor, no any actual work is needed here, all is done in the implementation part
      virtual ~SourceRockManager() {;}

   private:
      /// @{
      /// Copy constructor and operator are disabled, use the copyTo method instead
      SourceRockManager( const SourceRockManager & otherSourceRockManager );
      SourceRockManager & operator = ( const SourceRockManager & otherSourceRockManager );
      /// @}
   };
}

#endif // CMB_SOURCE_ROCK_MANAGER_API

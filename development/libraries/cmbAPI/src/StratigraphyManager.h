//                                                                      
// Copyright (C) 2012-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file StratigraphyManager.h
/// @brief This file keeps API declaration for manipulating layers in Cauldron model

#ifndef CMB_STRATIGRAPHY_MANAGER_API
#define CMB_STRATIGRAPHY_MANAGER_API

#include <vector>
#include "ErrorHandler.h"

/// @page StratigraphyManagerPage Stratigraphy Manager
/// @link mbapi::StratigraphyManager Stratigraphy manager @endlink provides set of interfaces to
/// create/delete/edit list of layers and surfaces in the data model. Also it has set of interfaces
/// to get/set property of any layer from the list

namespace mbapi {

   /// @class StratigraphyManager StratigraphyManager.h "StratigraphyManager.h" 
   /// @brief Class StratigraphyManager keeps a list of layers/surfaces in Cauldron model and allows to add/delete/edit layer/surface
   class StratigraphyManager : public ErrorHandler
   {
   public:
      ///@{
      /// Types definition
      typedef size_t LayerID;      //< unique ID for layer
      typedef size_t SurfaceID;    //< unique ID for surface
      typedef size_t PrFaultCutID; //< uinque ID for fault cut for P/T solver
      ///@}

      /// @{
      /// Set of interfaces for interacting with a Cauldron model

      /// @brief Get list of layers in the model
      /// @return array with IDs of layers defined in the model
      virtual std::vector<LayerID> layersIDs() const = 0; 

      /// @brief Get list of surfaces in the model
      /// @return array with IDs of surfaces defined in the model
      virtual std::vector<SurfaceID> surfacesIDs() const = 0; 

      /// @brief Get the referenced table
      /// @returns the table name as a string
      virtual std::string referenceID( ) const = 0;

      /// @brief Create new layer
      /// @return ID of the new Stratigraphy
      virtual LayerID createNewLayer() = 0;

      /// @brief Create new surface
      /// @return ID of the new surface
      virtual SurfaceID createNewSurface() = 0;

      /// @brief Get layer name for the given ID
      /// @param id layer ID
      /// @return layer name on success or empty string otherwise 
      virtual std::string layerName( LayerID id ) = 0;

      /// @brief Get layer ID for the given name 
      /// @param ln layer name
      /// @return layer ID on success or UndefinedIDValue otherwise
      virtual LayerID layerID( const std::string & ln ) = 0;

      /// @brief Get surface name for the given ID
      /// @param id surface ID
      /// @return surface name on success, or empty string otherwise
      virtual std::string surfaceName( LayerID id ) = 0;

      /// @brief Get depth grid name for the given ID
      /// @param id surface ID
      /// @return depth grid name on success, or empty string otherwise
      virtual std::string depthGridName(StratigraphyManager::LayerID id) = 0;

      /// @brief Get depth grid scalar value for the given ID
      /// @param id surface ID
      /// @return depth grid scalar value on success, or undefined value otherwise
      virtual double depthScalarValue(StratigraphyManager::LayerID id) = 0;

      /// @brief Get age of the eldest layer in stratigraphy
      /// @return age of the eldest (first) layer, UndefinedDoubleValue if no any layer is defined
      virtual double eldestLayerAge() = 0;

      // Layer -> lithology type relation methods

      /// @brief Get all lithologies associated with the given layer and percentage of each lithology in a mix
      /// @return ErrorHandler::NoError on success, or error code otherwise
      virtual ErrorHandler::ReturnCode layerLithologiesList(
              LayerID id                                   ///< [in] id layer ID
            , std::vector<std::string> & lithoList         ///< [out] lithoList on exit it contains the list of lithologies for the given layer
            , std::vector<double> & lithoPercent           ///< [out] lithoPercent on exit it contains percentage of each lithology in a mix
            , std::vector<std::string> & lithoPercMap      ///< [out] lithoPercMap on exit it contains the names of the percentage maps
                                                           ) = 0;
      
      /// @brief set lithologies and their percenatges for the given layer
      /// @return ErrorHandler::NoError on success, or error code otherwise
      virtual ErrorHandler::ReturnCode setLayerLithologiesList( 
            LayerID                          id           ///< layer ID
          , const std::vector<std::string> & lithoList    ///< lithologies name list (max 3)
          , const std::vector<double>      & lithoPercent ///< corresponded lithology percentage
		  , const std::vector<std::string> & lithoPercentGrid ///< corresponded lithology percentageGrid
                                                              ) = 0;

      /// @brief Collect layers where the given lithology is referenced
      /// @param lithoName name of lithology type
      /// @return list of layers ID
      virtual std::vector<LayerID> findLayersForLithology( const std::string & lithoName ) = 0;

      /// @brief For a given layer set the new lithofraction map names in the StratIoTbl
      /// @param[in] id layer ID
      /// @param[in] mapNameFirstLithoPercentage the name of the first lithology percentage map
      /// @param[in] mapNameSecondLithoPercentage the name of the second lithology percentage map
      /// @param[out] firstReplacedMapName the name of the first replaced map - returns empty string if no map was present before setting the new lithologies
      /// @param[out] secondReplacedMapName the name of the second replaced map - returns empty string if no map was present before setting the new lithologies
      /// @return NoError on success or NonexistingID on error
      virtual ErrorHandler::ReturnCode setLayerLithologiesPercentageMaps( LayerID id, const std::string & mapNameFirstLithoPercentage, const std::string mapNameSecondLithoPercentage ,
                                                                          std::string& firstReplacedMapName, std::string& secondReplacedMapName ) = 0;
      
      /// @brief Bind layer with top and bottom surfaces. Layer set itself as top/bottom layer for surface also
      /// @param[in] lid layer ID
      /// @param[in] usid up surface id
      /// @param[in] dsid down surface id
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode setLayerSurfaces( LayerID lid,  SurfaceID usid, SurfaceID dsid ) = 0;

      // Layer -> Source rock type relation methods

      /// @brief Check if the given layer is a source rock layer
      /// @param id layer ID
      /// @return true if the given layer is a source rock layer, false otherwise
      virtual bool isSourceRockActive( LayerID id ) = 0;

      /// @brief Check if for the given layer source rock mixing is enabled
      /// @param id layer ID
      /// @return true if source rock mixing is enabled, false otherwise
      virtual bool isSourceRockMixingEnabled( LayerID id ) = 0;

      /// @brief Enable or disable source rock mixing for the giving layer
      /// @param id layer ID
      /// @param val true - enable source rock mixing/false - disable
      /// @return NoError on success, error code otherwise
      virtual ErrorHandler::ReturnCode setSourceRockMixingEnabled( LayerID id, bool val ) = 0;

      /// @brief Check if layer has active allochton lithology
      /// @param id layer ID
      /// @return true if yes, false otherwise
      virtual bool isAllochtonLithology( LayerID id ) = 0;

      /// @brief Get source rock types associated with given layer ID
      /// @param lid layer ID
      /// @return if layer is not a source rock layer function returns an empty array,
      ///         otherwise it returns one or two (if source rock mixing is enabled for the layer)
      ///         source rock type names which can be used to access source rock type properties.
      virtual std::vector<std::string> sourceRockTypeName( LayerID lid ) = 0;
      
      /// @brief Get HI index for source rocks mix for the given layer
      /// @param lid layer ID
      /// @return if source rock mixing is enabled for the given layer, this function returns
      /// HI value for the mix or 0 otherwise.
      virtual double sourceRockMixHI( LayerID lid ) = 0;

      /// @brief Get H/C index for source rocks mix for the given layer
      /// @param lid layer ID
      /// @return if source rock mixing is enabled for the given layer, this function returns
      /// HI value for the mix or 0 otherwise.
      virtual double sourceRockMixHC( LayerID lid ) = 0;

      /// @brief Set source rock types name for the given layer and enable layer to be layer with source rock 
      /// @param lid layer ID
      /// @param srTypeNames array which can have one or two (in case of mixing) source rock types name. 
      /// @return ErrorHandler::NoError on success, error code otherwise
      virtual ReturnCode setSourceRockTypeName( LayerID lid, const std::vector<std::string> & srTypeNames ) = 0;

      /// @brief Set HI value for source rock mix for the given layer
      /// @param lid layer ID
      /// @param srmHI HI value for source rock mix
      /// @return ErrorHandler::NoError on success or error code if mixing is not turned off or other error happened
      virtual ReturnCode setSourceRockMixHI( LayerID lid, double srmHI ) = 0;

      /// @brief Set HI map name for the source rock mix for the given layer
      /// @param lid layer ID
      /// @param srmHImap HI map name for the source rock mix
      /// @return ErrorHandler::NoError on success or error code if mixing is not turned off or other error happened
      virtual ReturnCode setSourceRockMixHIMapName( LayerID lid, const std::string & srmHImap ) = 0;


      /// @brief Set H/C value for source rock mix for the given layer
      /// @param lid layer ID
      /// @param srmHC H/C value for source rock mix
      /// @return ErrorHandler::NoError on success or error code if mixing is not turned off or other error happened
      virtual ReturnCode setSourceRockMixHC( LayerID lid, double srmHC ) = 0;

      /// @}

      /// @{ Fault cuts methods

      /// @brief Get list of fault cuts from PressureFaultcutIoTbl
      /// @return array with IDs of layers defined in the model
      virtual std::vector<PrFaultCutID> faultCutsIDs() = 0;

      /// @brief Search in PressureFaultcutIoTbl table for the given combination of map name/fault name
      /// @param mapName map name
      /// @param fltName fault cut name 
      /// @return PrFaultCutID for the found fault / map combination on success, UndefinedIDValue otherwise
      virtual PrFaultCutID findFaultCut( const std::string & mapName, const std::string & fltName ) = 0;

      /// @brief Get lithlogy name for the given fault cut ID
      /// @return name of the fault cut lithology on success or emtpy string on error
      virtual std::string faultCutLithology( PrFaultCutID flID ) = 0;

      /// @brief Get fault cut name for the given fault cut ID
      /// @return name of the fault cut on success or empty string on error
      virtual std::string faultCutName( PrFaultCutID flID ) = 0;

      /// @brief Get fault cat map for the given fault cut ID
      /// @return map name on success or empty string on error
      virtual std::string faultCutMapName( PrFaultCutID flID ) = 0;

      /// @brief Set new lithology for the fault cut
      /// @param flID fault cut id in PressureFaultcutIoTbl
      /// @param newLithoName new lithology name
      /// @return ErrorHandler::NoError on success, error code otherwise
      virtual ReturnCode setFaultCutLithology( PrFaultCutID flID, const std::string & newLithoName ) = 0;
      /// @}
 
      /// @{ Get the maps names for the measured twt and the measured twt 

      /// @brief Get the map name of the measured twt at the top surface
      /// @param id layer id
      /// @return the twt map name for the layer on success, empty string otherwise
      virtual std::string twtGridName( LayerID id ) = 0;

      /// @brief Get the value of the measured twt at the top surface
      /// @param id layer id
      /// @return the measured twt value on success, UndefinedDoubleValue otherwise
      virtual double twtValue( LayerID id ) = 0;

	  /// @brief Get depositional age from the StartIoTbl
	  /// @param[in] id Stratigraphic layer ID
	  /// @param[out] DepoAge depositional age of the specified stratigraphic layer ID
	  /// @return NoError on success or NonexistingID on error
	  virtual ReturnCode getDepoAge(const LayerID id, double & DepoAge) = 0;

      /// @}
 
   protected:
      /// @{
      /// Constructors/destructor
      
      /// @brief Constructor which creates an empty model
      StratigraphyManager() {;}
      
      /// @brief Destructor, no any actual work is needed here, all is done in the implementation part
      virtual ~StratigraphyManager() {;}
      /// @}

   private:
      /// @{
      /// Copy constructor and copy operator are disabled
      StratigraphyManager( const StratigraphyManager & otherStratigraphyManager );
      StratigraphyManager & operator = ( const StratigraphyManager & otherStratigraphyManager );
      /// @}
   };
}

#endif // CMB_STRATIGRAPHY_MANAGER_API

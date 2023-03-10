//
// Copyright (C) 2012-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// @file StratigraphyManagerImpl.h
//  This file keeps API implementation for manipulating layers in Cauldron model

#ifndef CMB_STRATIGRAPHY_MANAGER_IMPL_API
#define CMB_STRATIGRAPHY_MANAGER_IMPL_API

#include <memory>

#include "ProjectFileHandler.h"

#include "StratigraphyManager.h"

namespace database
{
   class Database;
   class Table;
}

namespace mbapi {

   // Class StratigraphyManager keeps a list of layers/surfaces in Cauldron model and allows to add/delete/edit layer/surface
   class StratigraphyManagerImpl : public StratigraphyManager
   {
   public:
      // Constructors/destructor
      // brief Constructor which creates an StratigraphyManager
      StratigraphyManagerImpl();

      // Destructor
      virtual ~StratigraphyManagerImpl() {;}

      // Copy operator
      StratigraphyManagerImpl & operator = ( const StratigraphyManagerImpl & otherStratMgr );

      // Set of interfaces for interacting with a Cauldron model

      // Set project database. Reset all
      void setDatabase( database::ProjectFileHandlerPtr pfh );

      // Get list of layers in the model
      // returns an array with IDs of layers defined in the model
      virtual std::vector<LayerID> layersIDs() const;

      // Get list of surfaces in the model
      // returns array with IDs of surfaces defined in the model
      virtual std::vector<SurfaceID> surfacesIDs() const;

      // Get the referenced table
      // returns the table name as a string
      virtual std::string referenceID() const;

      // Create new layer
      // returns ID of the new Stratigraphy
      virtual LayerID createNewLayer();

      // Create new surface
      // returns ID of the new surface
      virtual SurfaceID createNewSurface();

      // Get layer name for the given ID
      // id layer ID
      // return layer name on success or empty string otherwise
      virtual std::string layerName( LayerID id );

      // Get layer ID for the given name
      // ln layer name
      // return layer ID on success or UndefinedIDValue otherwise
      virtual LayerID layerID( const std::string & ln );

      // Get surface name for the given ID
      // id surface ID
      // return surface name on success, or empty string otherwise
      virtual std::string surfaceName( LayerID id );

      // Get age of the eldest layer in stratigraphy
      // return age of the eldest (first) layer, UndefinedDoubleValue if no any layer is defined
      virtual double eldestLayerAge();

      // Layer -> lithology  type relation methods

      // Get all lithologies associated with the given layer and percentage of each lithology in a mix
      // id layer ID
      // lithoList on exit it contains the list of lithologies for the given layer
      // lithoPercent on exit it contains percentage of each lithology in a mix
      // return ErrorHandler::NoError on success, or error code otherwise
      virtual ErrorHandler::ReturnCode layerLithologiesList( LayerID id, std::vector<std::string> & lithoList, std::vector<double> & lithoPercent, std::vector<std::string> & lithoPercMap );

      // Set lithologies and their percenatges for the given layer
      // return ErrorHandler::NoError on success, or error code otherwise
      virtual ErrorHandler::ReturnCode setLayerLithologiesList( LayerID                          id
                                                              , const std::vector<std::string> & lithoList
                                                              , const std::vector<double>      & lithoPercent
															  , const std::vector<std::string> & lithoPercentGrid
                                                              );

      // Bind layer with top and bottom surfaces. Layer set itself as top/bottom layer for surface also
      // [in] lid layer ID
      // [in] usid up surface id
      // [in] dsid down surface id
      // returns NoError on success or NonexistingID on error
      virtual ReturnCode setLayerSurfaces( LayerID lid,  SurfaceID usid, SurfaceID dsid );

      // Collect layers where the given lithology is referenced
      // lithName name of lithology type
      // return list of layers ID
      virtual std::vector<LayerID> findLayersForLithology( const std::string & lithoName );

      /// @brief For a given layer set the new lithofraction map names in the StratIoTbl
      /// @param[in] id layer ID
      /// @param[in] mapNameFirstLithoPercentage the name of the first lithology percentage map
      /// @param[in] mapNameSecondLithoPercentage the name of the second lithology percentage map
      /// @param[out] firstReplacedMapName the name of the first replaced map - returns empty string if no map was present before setting the new lithologies
      /// @param[out] secondReplacedMapName the name of the second replaced map - returns empty string if no map was present before setting the new lithologies
      /// @return NoError on success or NonexistingID on error
      virtual ErrorHandler::ReturnCode setLayerLithologiesPercentageMaps(LayerID id, const std::string & mapNameFirstLithoPercentage, const std::string mapNameSecondLithoPercentage ,
                                                                         std::string& firstReplacedMapName, std::string& secondReplacedMapName) override;


      // Layer -> Source rock type relation methods

      // Check if the given layer is a source rock layer
      // id layer ID
      // return true if the given layer is a source rock layer, false otherwise
      virtual bool isSourceRockActive( LayerID id );

      // Check if for the given layer source rock mixing is enabled
      // id layer ID
      // return true if source rock mixing is enabled, false otherwise
      virtual bool isSourceRockMixingEnabled( LayerID id );

      // Enable or disable source rock mixing for the giving layer
      // id layer ID
      // val true - enable source rock mixing/false - disable
      // return NoError on success, error code otherwise
      virtual ErrorHandler::ReturnCode setSourceRockMixingEnabled( LayerID id, bool val );

      // Check if layer has active allochton lithology
      // id layer ID
      // return true if yes, false otherwise
      virtual bool isAllochtonLithology( LayerID id );

      // Get source rock types associated with given layer ID
      // lid layer ID
      // return if layer is not a source rock layer function returns an empty array,
      //        otherwise it returns one or two (if source rock mixing is enabled for the layer)
      //        source rock type names which can be used to access source rock type properties.
      virtual std::vector<std::string> sourceRockTypeName( LayerID lid );

      // Get HI index for source rocks mix for the given layer
      // lid layer ID
      // return if source rock mixing is enabled for the given layer, this function returns
      //        HI value for the mix or 0 otherwise.
      virtual double sourceRockMixHI( LayerID lid );

      // Get H/C index for source rocks mix for the given layer
      // lid layer ID
      // return if source rock mixing is enabled for the given layer, this function returns
      //        HI value for the mix or 0 otherwise.
      virtual double sourceRockMixHC( LayerID lid );

      // Set source rock types name for the given layer and enable layer to be layer with source rock
      // lid layer ID
      // srTypeNames array which can have one or two (in case of mixing) source rock types name.
      // return ErrorHandler::NoError on success, error code otherwise
      virtual ReturnCode setSourceRockTypeName( LayerID lid, const std::vector<std::string> & srTypeNames );

      // Set HI value for source rock mix for the given layer
      // lid layer ID
      // srmHI HI value for source rock mix
      // return ErrorHandler::NoError on success or error code if mixing is not turned off or other error happened
      virtual ReturnCode setSourceRockMixHI( LayerID lid, double srmHI );

      // Set HI map name for the source rock mix for the given layer
      // lid layer ID
      // srmHImap HI map name for the source rock mix
      // return ErrorHandler::NoError on success or error code if mixing is not turned off or other error happened
      virtual ReturnCode setSourceRockMixHIMapName( LayerID lid, const std::string & srmHImap );


      // Set H/C value for source rock mix for the given layer
      // lid layer ID
      // srmHI HI value for source rock mix
      // return ErrorHandler::NoError on success or error code if mixing is not turned off or other error happened
      virtual ReturnCode setSourceRockMixHC( LayerID lid, double srmHC );

      // Get list of fault cuts from PressureFaultcutIoTbl
      // return array with IDs of layers defined in the model
      virtual std::vector<PrFaultCutID> faultCutsIDs();

      // Search in PressureFaultcutIoTbl table for the given combination of map name/fault name
      // mapName map name
      // fltName fault cut name
      // return PrFaultCutID for the found fault / map combination on success, UndefinedIDValue otherwise
      virtual PrFaultCutID findFaultCut( const std::string & mapName, const std::string & fltName );

      // Get lithlogy name for the given fault cut ID
      // flID fault cut id in PressureFaultcutIoTbl
      // return Name of the fault cut lithology
      virtual std::string faultCutLithology( PrFaultCutID flID );

      // Get fault cut name for the given fault cut ID
      // return Name of the fault cut
      virtual std::string faultCutName( PrFaultCutID flID );

      // Get fault cat map for the given fault cut ID
      // return map name
      virtual std::string faultCutMapName( PrFaultCutID flID );

      // Set new lithology for the fault cut
      // flID fault cut id in PressureFaultcutIoTbl
      // newLithoName new lithology name
      // return ErrorHandler::NoError on success, error code otherwise
      virtual ReturnCode setFaultCutLithology( PrFaultCutID flID, const std::string & newLithoName );

      /// Get the map name of the measured twt at the top surface
      /// id layer id
      /// return the twt map name for the layer on success, empty string otherwise
      virtual std::string twtGridName( LayerID id );

      /// Get the map name of the depth grid
      /// id layer id
      /// return the depth grid name for the layer on success, empty string otherwise
      virtual std::string depthGridName(StratigraphyManager::LayerID id);

      /// Get the scalar value of the depth grid
      /// id layer id
      /// return the depth scalar value for the layer on success, undefined otherwise
      virtual double depthScalarValue(StratigraphyManager::LayerID id);

      /// Get the value of the measured twt at the top surface
      /// id layer id
      /// the measured twt value on success, UndefinedDoubleValue otherwise
      virtual double twtValue( LayerID id );

	  /// @brief Get depositional age from the StartIoTbl
	  /// @param[in] id Stratigraphic layer ID
	  /// @param[out] DepoAge depositional age of the specified stratigraphic layer ID
	  /// @return NoError on success or NonexistingID on error
	  virtual ReturnCode getDepoAge(const LayerID id, double & DepoAge);			

   private:
      static const char * s_stratigraphyTableName;
      static const char * s_layerNameFieldName;
      static const char * s_depoAgeFieldName;
      static const char * s_depoSequence;
      static const char * s_lithoType1FiledName;
      static const char * s_lithoType2FiledName;
      static const char * s_lithoType3FiledName;
      static const char * s_lithoTypePercent1FiledName;
      static const char * s_lithoTypePercent2FiledName;
      static const char * s_lithoTypePercent3FiledName;
      static const char * s_lithoTypePercent1GridFiledName;
      static const char * s_lithoTypePercent2GridFiledName;
      static const char * s_isSourceRockFieldName;
      static const char * s_sourceRockType1FieldName;
      static const char * s_sourceRockType2FieldName;
      static const char * s_sourceRockHIFieldName;
      static const char * s_sourceRockHIMapFieldName;
      static const char * s_sourceRockEnableMixingFieldName;
      static const char * s_isAllochtonLithology;

      static const char * s_pressureFaultCutTableName;
      static const char * s_FaultcutsMapFieldName;
      static const char * s_FaultNameFieldName;
      static const char * s_FaultLithologyFieldName;
      static const char * s_twoWayTimeTableName;
      static const char * s_twoWayTimeGridFiledName;

      static const char * s_twoWayTimeFiledName;
      static const char * s_surfaceNameFieldName;
      static const char * s_depthGridFiledName;
      static const char * s_depthScalarFiledName;


      database::ProjectFileHandlerPtr m_db;         // cauldron project database
      database::Table               * m_stratIoTbl; // stratigraphy table

      // Copy constructor is disabled, use the copy operator instead
      StratigraphyManagerImpl( const StratigraphyManager & );
   };
}

#endif // CMB_STRATIGRAPHY_MANAGER_IMPL_API

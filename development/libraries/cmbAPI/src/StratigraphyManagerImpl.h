//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
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

#include "StratigraphyManager.h"

namespace database
{
   class Database;
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
      void setDatabase( database::Database * db );

      // Get list of layers in the model
      // returns an array with IDs of layers defined in the model
      virtual std::vector<LayerID> getLayersID() const; 

      // Get list of surfaces in the model
      // returns array with IDs of surfaces defined in the model
      virtual std::vector<SurfaceID> geSurfacesID() const; 

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

      // Bind layer with top and bottom surfaces. Layer set itself as top/bottom layer for surface also
      // [in] lid layer ID
      // [in] usid up surface id
      // [in] dsid down surface id
      // returns NoError on success or NonexistingID on error
      virtual ReturnCode setLayerSurfaces( LayerID lid,  SurfaceID usid, SurfaceID dsid );
      

      // Layer -> Source rock type relation methods

      // Check if the given layer is a source rock layer
      // id layer ID
      // return true if the given layer is a source rock layer, false otherwise
      virtual bool isSourceRockActive( LayerID id );

      // Check if for the given layer source rock mixing is enabled
      // id layer ID
      // return true if source rock mixing is enabled, false otherwise
      virtual bool isSourceRockMixingEnabled( LayerID id );

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

      // Set H/C value for source rock mix for the given layer
      // lid layer ID
      // srmHI HI value for source rock mix
      // return ErrorHandler::NoError on success or error code if mixing is not turned off or other error happened
      virtual ReturnCode setSourceRockMixHC( LayerID lid, double srmHC );


   private:

      static const char * m_stratigraphyTableName;
      static const char * m_layerNameFieldName;
      static const char * m_isSourceRockFieldName;
      static const char * m_sourceRockType1FieldName;
      static const char * m_sourceRockType2FieldName;
      static const char * m_sourceRockHIFieldName;
      static const char * m_sourceRockEnableMixintFieldName;

      database::Database * m_db; // cauldron project database

      // Copy constructor is disabled, use the copy operator instead
      StratigraphyManagerImpl( const StratigraphyManager & );
   };
}

#endif // CMB_STRATIGRAPHY_MANAGER_IMPL_API

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
      // [in] id layer ID
      // [out] layerName on success has a layer name, or empty string otherwise
      // returns NoError on success or NonexistingID on error
      virtual ReturnCode getLayerName( LayerID id, std::string & layerName );

      // Get surface name for the given ID
      // [in] id surface ID
      // [out] surfName on success has a surface name, or empty string otherwise
      // returns NoError on success or NonexistingID on error
      virtual ReturnCode getSurfaceName( LayerID id, std::string & surfName );

      // Bind layer with top and bottom surfaces. Layer set itself as top/bottom layer for surface also
      // [in] lid layer ID
      // [in] usid up surface id
      // [in] dsid down surface id
      // returns NoError on success or NonexistingID on error
      virtual ReturnCode setLayerSurfaces( LayerID lid,  SurfaceID usid, SurfaceID dsid );
      
   private:
      // Copy constructor is disabled, use the copy operator instead
      StratigraphyManagerImpl( const StratigraphyManager & );
   };
}

#endif // CMB_STRATIGRAPHY_MANAGER_IMPL_API
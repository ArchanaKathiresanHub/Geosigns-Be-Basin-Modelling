//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
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
      typedef size_t LayerID;    //< unique ID for layer
      typedef size_t SurfaceID;  //< unique ID for surface
      ///@}

      /// @{
      /// Set of interfaces for interacting with a Cauldron model

      /// @brief Get list of layers in the model
      /// @return array with IDs of layers defined in the model
      virtual std::vector<LayerID> getLayersID() const = 0; 

      /// @brief Get list of surfaces in the model
      /// @return array with IDs of surfaces defined in the model
      virtual std::vector<SurfaceID> geSurfacesID() const = 0; 

      /// @brief Create new layer
      /// @return ID of the new Stratigraphy
      virtual LayerID createNewLayer() = 0;

      /// @brief Create new surface
      /// @return ID of the new surface
      virtual SurfaceID createNewSurface() = 0;

      /// @brief Get layer name for the given ID
      /// @param[in] id layer ID
      /// @param[out] layerName on success has a layer name, or empty string otherwise
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode getLayerName( LayerID id, std::string & layerName ) = 0;

      /// @brief Get surface name for the given ID
      /// @param[in] id surface ID
      /// @param[out] surfName on success has a surface name, or empty string otherwise
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode getSurfaceName( LayerID id, std::string & surfName ) = 0;

      /// @brief Bind layer with top and bottom surfaces. Layer set itself as top/bottom layer for surface also
      /// @param[in] lid layer ID
      /// @param[in] usid up surface id
      /// @param[in] dsid down surface id
      /// @return NoError on success or NonexistingID on error
      virtual ReturnCode setLayerSurfaces( LayerID lid,  SurfaceID usid, SurfaceID dsid ) = 0;

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

//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file StratigraphyManagerImpl.C
/// @brief This file keeps API implementation for manipulating layers/surfaces in Cauldron model

#include "StratigraphyManagerImpl.h"

#include <stdexcept>
#include <string>

namespace mbapi
{

// Constructor
StratigraphyManagerImpl::StratigraphyManagerImpl()
{
   //throw std::runtime_error( "Not implemented yet" );
}

// Copy operator
StratigraphyManagerImpl & StratigraphyManagerImpl::operator = ( const StratigraphyManagerImpl & otherLythMgr )
{
   throw std::runtime_error( "Not implemented yet" );
   return *this;
}


// Get list of layers in the model
// returns an array with IDs of layers defined in the model
std::vector<StratigraphyManager::LayerID> StratigraphyManagerImpl::getLayersID() const
{
   throw std::runtime_error( "Not implemented yet" );
   return std::vector<StratigraphyManager::LayerID>();
}

// Get list of surfaces in the model
// returns array with IDs of surfaces defined in the model
std::vector<StratigraphyManager::SurfaceID> StratigraphyManagerImpl::geSurfacesID() const
{
   throw std::runtime_error( "Not implemented yet" );
   return std::vector<SurfaceID>();
}

// Create new layer
// returns ID of the new Stratigraphy
StratigraphyManager::LayerID StratigraphyManagerImpl::createNewLayer()
{
   throw std::runtime_error( "Not implemented yet" );
   return 0;
}

// Create new surface
// returns ID of the new surface
StratigraphyManager::SurfaceID StratigraphyManagerImpl::createNewSurface()
{
   throw std::runtime_error( "Not implemented yet" );
   return 0;
}

// Get layer name for the given ID
// [in] id layer ID
// [out] layerName on succes has a layer name, or empty string otherwise
// returns NoError on success or NonexistingID on error
ErrorHandler::ReturnCode StratigraphyManagerImpl::getLayerName( StratigraphyManager::LayerID id, std::string & layerName )
{
   throw std::runtime_error( "Not implemented yet" );
   layerName = "";
   return NotImplementedAPI;
}

// Get surface name for the given ID
// [in] id surface ID
// [out] surfName on succes has a surface name, or empty string otherwise
// returns NoError on success or NonexistingID on error
ErrorHandler::ReturnCode StratigraphyManagerImpl::getSurfaceName( StratigraphyManager::LayerID id, std::string & surfName )
{
   throw std::runtime_error( "Not implemented yet" );
   surfName = "";
   return NotImplementedAPI;
}

// Bind layer with top and bottom surfaces. Layer set itself as top/bottom layer for surface also
// [in] lid layer ID
// [in] usid up surface id
// [in] dsid down surface id
// returns NoError on success or NonexistingID on error
ErrorHandler::ReturnCode StratigraphyManagerImpl::setLayerSurfaces( StratigraphyManager::LayerID lid
                                                                  , StratigraphyManager::SurfaceID usid
                                                                  , StratigraphyManager::SurfaceID dsid
                                                                  )
{
   throw std::runtime_error( "Not implemented yet" );
   return NotImplementedAPI;
}

}
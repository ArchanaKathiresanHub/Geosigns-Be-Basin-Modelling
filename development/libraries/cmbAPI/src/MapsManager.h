//                                                                      
// Copyright (C) 2015 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file MapsManager.h
/// @brief This file keeps API declaration for manipulating input 2D maps

#ifndef CMB_MAPS_MANAGER_API
#define CMB_MAPS_MANAGER_API

#include "ErrorHandler.h"


/// @page MapsManagerPage Maps Manager
/// @link mbapi::MapsManager Maps manager @endlink provides set of interfaces to load/modify/save input 2D maps in project file 

namespace mbapi
{

   /// @class MapsManager MapsManager.h "MapsManager.h"
   /// @brief Class MapsManager keeps a list of input maps in Cauldron model and allows to load/modify/save 2D input maps
   class MapsManager : public ErrorHandler
   {
   public:
      typedef size_t MapID;  ///< unique ID for lithology

      /// @{
      /// Set of interfaces for interacting with a Cauldron model

      /// @brief Get list of input maps in the model
      /// @return array with IDs of different input maps defined in the model
      virtual std::vector<MapID> mapsIDs() const = 0; 

      /// @brief Search for map record which has given name 
      /// @param name map name
      /// @return ID of found input map on success or UndefinedIDValue otherwise
      virtual MapID findID( const std::string & mName ) = 0;

      /// @brief Make a copy of the given map. Map must be saved in the separate call of MapManager::saveMapToHDF
      /// @param[in] id map ID
      /// @param[in] newMapName new name for the input map
      /// @return new map ID on success or UndefinedIDValue on error
      virtual MapID copyMap( MapID id, const std::string & newMapName ) = 0;

      /// @brief Save input map to the new HDF file. File with the given name should not exist before.
      /// @param id map ID
      /// @param fileName unique file name
      /// @return ErrorHandler::NoError on succes, or error code otherwise
      virtual ErrorHandler::ReturnCode saveMapToHDF( MapID id, const std::string & fileName ) = 0;

      /// @brief Get min/max map values range
      /// @param[in] id map ID
      /// @param[out] minV minimal value in the map
      /// @param[out] maxV maximal value in the map
      /// @return ErrorHandler::NoError on success, or error code otherwise
      virtual ErrorHandler::ReturnCode mapValuesRange( MapID id, double & minV, double & maxV ) = 0;

      /// @brief Linearly rescale input map to the new value range
      /// @param[in] id map ID
      /// @param[out] minV new minimal value in the map
      /// @param[out] maxV new maximal value in the map
      /// @return ErrorHandler::NoError on success, or error code otherwise
      virtual ErrorHandler::ReturnCode rescaleMap( MapID id, double newMinV, double newMaxV ) = 0;

      /// @}

   protected:
      /// @name Constructors/destructor
      /// @{

      /// @brief Constructor which creates an empty model
      MapsManager() {;}
   
      /// @brief Destructor, no any actual work is needed here, all is done in the implementation part
      virtual ~MapsManager() {;}

      /// @}

   private:
      /// @{
      /// Copy constructor and operator are disabled, use the copyTo method instead
      MapsManager( const MapsManager & otherMapsManager );
      MapsManager & operator = ( const MapsManager & otherMapsManager );
      /// @}
   };

}

#endif // CMB_MAPS_MANAGER_API
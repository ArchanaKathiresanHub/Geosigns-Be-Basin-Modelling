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
      /// @param mName map name
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
      /// @param mapSequenceNbr the map sequence number in the project3d
      /// @return ErrorHandler::NoError on succes, or error code otherwise
      virtual ErrorHandler::ReturnCode saveMapToHDF( MapID id, const std::string& filePathName, const int mapSequenceNbr ) = 0;

      /// @brief Get min/max map values range
      /// @param[in] id map ID
      /// @param[out] minV minimal value in the map
      /// @param[out] maxV maximal value in the map
      /// @return ErrorHandler::NoError on success, or error code otherwise
      virtual ErrorHandler::ReturnCode mapValuesRange( MapID id, double & minV, double & maxV ) = 0;

      /// @brief Scale input map to the new value range: [min,max] -> [min*coeff, newMaxV*coeff]
      /// @param[in] id map ID
      /// @param[in] coeff map scaler
      /// @return ErrorHandler::NoError on success, or error code otherwise
      virtual ErrorHandler::ReturnCode scaleMap( MapID id, double coeff ) = 0;

      /// @brief Set the values contained in vin in the map
      /// @param[in] id map ID
      /// @param[in] vin the values to set
      /// @return ErrorHandler::NoError on success, or error code otherwise
      virtual ErrorHandler::ReturnCode mapSetValues( MapID id, const std::vector<double>& vin ) = 0;

      /// @brief Get a value contained in the map
      /// @param[in] id map ID
      /// @param[in] i position
      /// @param[in] j position
      /// @return the parameter value
      virtual double mapGetValue( MapID id, const unsigned int i, const unsigned int j ) = 0;

      /// @brief Get the values contained in vin in the map
      /// @param[in] id map ID
      /// @param[out] vout the local (partitioned) values of the map
      /// @return ErrorHandler::NoError on success, or error code otherwise
      virtual ErrorHandler::ReturnCode mapGetValues( MapID id, std::vector<double>& vout ) = 0;

      /// @brief Create map by interpolating between 2 maps 
      /// @param id new map id
      /// @param minId low range map
      /// @param maxId high range map
      /// @param coeff coefficient in range [0:1]
      /// @return NoError on success or error code otherwise
      virtual ErrorHandler::ReturnCode interpolateMap( MapID id, MapID minId, MapID maxId, double coeff ) = 0;

      /// @brief Interpolate input values using the natural neighbour algorithm
      /// @param[in]  xin x coordinates of the input values
      /// @param[in]  yin y coordinates of the input values
      /// @param[in]  vin input values
      /// @param[in]  xmin minimum x value of the interpolation window
      /// @param[in]  xmax maximum x value of the interpolation window
      /// @param[in]  ymin minimum y value of the interpolation window
      /// @param[in]  ymax maximum y value of the interpolation window
      /// @param[in]  numI number of interpolated points in the x direction
      /// @param[in]  numJ number of interpolated points in the y direction
      /// @param[out] xout x coordinates of the interpolated values
      /// @param[out] yout y coordinates of the interpolated values
      /// @param[out] vout interpolated values
      /// @return NoError on success or error code otherwise
      virtual ErrorHandler::ReturnCode interpolateMap( 
         const std::vector<double>& xin,
         const std::vector<double>& yin,
         const std::vector<double>& vin,
         double xmin,
         double xmax,
         double ymin,
         double ymax,
         int numI,
         int numJ,
         std::vector<double>& xout,
         std::vector<double>& yout,
         std::vector<double>& vout ) = 0;

      /// @brief Generate a new map in the GridMapIoTbl
      /// @param[in] refferedTable the name of the table refferenced this map
      /// @param[in] mapName the name of the map
      /// @param[in] values new map values
      /// @param[in] the file name of the  HDF file storing the map
      /// @return ErrorHandler::NoError on succes, or error code otherwise
      virtual MapID generateMap( const std::string & refferedTable, const std::string mapName, const std::vector<double>& values, int & mapSequenceNbr, const std::string & filePathName ) = 0;

      /// @brief inizialize the map writer to write 2D HDF maps
      /// @param[in] filePathName the file name 
      /// @param[in] append the mode of writing
      /// @return ErrorHandler::NoError on success, or error code otherwise
      virtual ErrorHandler::ReturnCode inizializeMapWriter( const std::string & filePathName, const bool append ) = 0;

      /// @brief finalize the map writer to write 2D maps in the HDF file
      /// @return ErrorHandler::NoError on success, or error code otherwise
      virtual ErrorHandler::ReturnCode finalizeMapWriter( ) = 0;

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

//                                                                      
// Copyright (C) 2012-2015 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file MapsManagerImpl.h
/// @brief This file keeps API implementation for manipulating input 2D maps

#ifndef CMB_MAPS_MANAGER_IMPL_API
#define CMB_MAPS_MANAGER_IMPL_API

#include "ErrorHandler.h"
#include "MapsManager.h"
#include "Interface/Interface.h"

// STL
#include <set>
#include <memory>
#include <map>

namespace database
{
   class Database;
}

namespace DataAccess
{
   namespace Interface
   {
      class ProjectHandle;
      class GridMap;
   }
}

namespace mbapi
{
   // Class MapsManagerImpl keeps a list of input maps in Cauldron model and allows to load/modify/save 2D input maps
   class MapsManagerImpl : public MapsManager 
   {
   public:
      /// @brief Constructor 
      MapsManagerImpl();
   
      // Destructor
      virtual ~MapsManagerImpl();

      // Get list of input maps in the model
      virtual std::vector<MapID> mapsIDs() const; 

      // Search for map record which has given name 
      virtual MapID findID( const std::string & mName );

      // Make a copy of the given map. Map must be saved in the separate call of MapManager::saveMapToHDF
      virtual MapID copyMap( MapID id, const std::string & newMapName );

      // Save input map to the new HDF file. File with the given name should not exist before.
      virtual ErrorHandler::ReturnCode saveMapToHDF( MapID id, const std::string& filePathName );

      // Get min/max map values range
      virtual ErrorHandler::ReturnCode mapValuesRange( MapID id, double & minV, double & maxV );

      // Linearly rescale input map to the new value range
      virtual ErrorHandler::ReturnCode scaleMap( MapID id, double coeff );

      // Set values in the map
      virtual ErrorHandler::ReturnCode mapSetValues( MapID id, const std::vector<double>& vin );

      // Get the values from the map
      virtual ErrorHandler::ReturnCode mapGetValues( MapID id, std::vector<double>& vout );

      // Interpolate between 2 maps, coefficient in range [0:1]
      virtual ErrorHandler::ReturnCode interpolateMap( MapID id, MapID minId, MapID maxId, double coeff );

      // Interpolate input values using the natural neighbour algorithm
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
         std::vector<double>& vout );

      // Generate a new map in the GridMapIoTbl
      virtual MapID generateMap( const std::string & refferedTable, const std::string mapName, const std::vector<double>& values, const std::string & filePathName );

      // Inizialize the map writer
      virtual ErrorHandler::ReturnCode inizializeMapWriter( const std::string & filePathName, const bool append );

      // Finalize the map writer
      virtual ErrorHandler::ReturnCode finalizeMapWriter();
      
      // Set of interfaces for interacting with a Cauldron model
      // Set project database. Reset all
      void setProject( DataAccess::Interface::ProjectHandle * ph, const std::string & projectFileName );

      // Copy input maps file to the given location
      void copyMapFiles( const std::string & newLocation );

   private:
      static const char * s_mapsTableName;      // table name for input maps list in project file
      static const char * s_ReferredByColName;  // Name of the table which refer to this map, e.g. StratIoTbl
      static const char * s_MapNameColName;     // Input map name
      static const char * s_MapTypeColName;     // Type of the grid map, possible values are:DECASCII, DECBINARY, ZYCOR, CPS3, EPIRUS, XYZ
      static const char * s_MapFileNameColName; // Filename of the grid map (with extension)
      static const char * s_MapSeqNbrColName;   // Sequence number of the grid map, within the grid loader (Starting with 0). This attribute
      static const char * s_StratIoTbl;         // Table name reffered in the GridMapIoTbl for the lithofractions
      static const char * s_mapResultFile;      // The name of the HDF file containing the maps 

      // Copy constructor and operator are disabled
      MapsManagerImpl( const MapsManagerImpl & otherMapsManagerImpl );
      MapsManagerImpl & operator = ( const MapsManagerImpl & otherMapsManagerImpl );
   
      database::Database                                  * m_db;              // cauldron project database
      DataAccess::Interface::ProjectHandle                * m_proj;            // project handle, to load/save maps
      std::string                                           m_projectFileName; // project file name
      DataAccess::Interface::MapWriter                    * m_mapPropertyValuesWriter; // MapsManager should have its own map writer to write/append maps to HDF file

      std::vector<std::string>                              m_mapName;
      std::vector<std::string>                              m_mapRefTable;

      std::vector<DataAccess::Interface::GridMap *>         m_mapObj;
      std::map<std::string, std::vector<std::string>>       m_fileMaps;     // for each HDF file, the vector of maps names
   
      std::set<std::string>                                 m_mapsFileList; // unique list of files with project maps

      void loadGridMap( MapID id );
   };
}

#endif // CMB_MAPS_MANAGER_IMPL_API

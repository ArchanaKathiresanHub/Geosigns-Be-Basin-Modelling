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

// STL
#include <set>
#include <memory>

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
      virtual ErrorHandler::ReturnCode saveMapToHDF( MapID id, const std::string & fileName );

      // Get min/max map values range
      virtual ErrorHandler::ReturnCode mapValuesRange( MapID id, double & minV, double & maxV );

      // Linearly rescale input map to the new value range
      virtual ErrorHandler::ReturnCode rescaleMap( MapID id, double newMinV, double newMaxV );
 
      // Set of interfaces for interacting with a Cauldron model
      // Set project database. Reset all
      void setProject( DataAccess::Interface::ProjectHandle * ph, const std::string & projectFileName );

      // Copy input maps file to the given location
      void copyMapFiles( const std::string & newLocation );

   private:
      static const char * m_mapsTableName;      // table name for input maps list in project file
      static const char * m_ReferredByColName;  // Name of the table which refer to this map, e.g. StratIoTbl
      static const char * m_MapNameColName;     // Input map name
      static const char * m_MapTypeColName;     // Type of the grid map, possible values are:DECASCII, DECBINARY, ZYCOR, CPS3, EPIRUS, XYZ
      static const char * m_MapFileNameColName; // Filename of the grid map (with extension)
      static const char * m_MapSeqNbrColName;   // Sequence number of the grid map, within the grid loader (Starting with 0). This attribute

      // Copy constructor and operator are disabled
      MapsManagerImpl( const MapsManagerImpl & otherMapsManagerImpl );
      MapsManagerImpl & operator = ( const MapsManagerImpl & otherMapsManagerImpl );
   
      database::Database                                  * m_db;              // cauldron project database
      DataAccess::Interface::ProjectHandle                * m_proj;            // project handle, to load/save maps
      std::string                                           m_projectFileName; // project file name
      std::vector<std::string> m_mapName;
      std::vector<std::string> m_mapRefTable;

      std::vector<DataAccess::Interface::GridMap *>   m_mapObj;
   
      std::set<std::string>    m_mapsFileList; // unique list of files with project maps

      void loadGridMap( MapID id );
   };
}

#endif // CMB_MAPS_MANAGER_IMPL_API

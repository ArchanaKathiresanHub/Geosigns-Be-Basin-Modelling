//
// Copyright (C) 2012-2018 Shell International Exploration & Production.
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
#include "Interface.h"

// STL
#include <set>
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
      MapsManagerImpl() = default;

      // Destructor
      virtual ~MapsManagerImpl() = default;

      // Copy constructor and operator are disabled
      MapsManagerImpl( const MapsManagerImpl & otherMapsManagerImpl ) = delete;
      MapsManagerImpl & operator = ( const MapsManagerImpl & otherMapsManagerImpl ) = delete;

      // Get list of input maps in the model
      std::vector<MapID> mapsIDs() const final;

      // Search for map record which has given name
      MapID findID( const std::string & mName ) final;

      // Get list of map names
      std::vector<std::string> mapNames() const final;

      // Get hash table of map names to map IDs
      std::unordered_map<std::string, MapID> mapNameIDs() const final;

      // Make a copy of the given map. Map must be saved in the separate call of MapManager::saveMapToHDF
      MapID copyMap(MapID id, const std::string& newMapName, const std::string& newMapFile = "") final;

      // Save input map to the new HDF file. File with the given name should not exist before.
      ErrorHandler::ReturnCode saveMapToHDF(MapID id, const std::string & fileName) final;

      // Get min/max map values range
      ErrorHandler::ReturnCode mapValuesRange( MapID id, double & minV, double & maxV ) final;

      // Linearly rescale input map to the new value range
      ErrorHandler::ReturnCode scaleMap( MapID id, double coeff ) final;

      // Smoothen grid map
      ErrorHandler::ReturnCode smoothenGridMap( MapID id, const int method, const double smoothingRadius, const unsigned int nrOfThreads ) final;

      // Smoothen vectorized map
      ErrorHandler::ReturnCode smoothenVectorizedMap( std::vector<double>& vec, const int method, const unsigned int numI, const unsigned int numJ,
                                                      const double dx, const double dy, const double smoothingRadius,
                                                      const double undefinedValue, const unsigned int nrOfThreads ) const;

      // Scale and shift the input map and correct for well locations
      ErrorHandler::ReturnCode scaleAndShiftMapCorrectedForWells( MapID id, double scale, double shift, double radiusOfInfluence,
                                                                  const std::vector<double>& xWells, const std::vector<double>& yWells ) final;

      // Set values in the map
      ErrorHandler::ReturnCode mapSetValues( MapID id, const std::vector<double> & vin ) final;

      // Get the values from the map
      ErrorHandler::ReturnCode mapGetValues( MapID id, std::vector<double>& vout ) final;

      // Get map dimensions
      ErrorHandler::ReturnCode mapGetDimensions( MapID id, int& i, int& j) final;

      // Get the value from the map
      double mapGetValue(MapID id, unsigned int i, unsigned int j ) final;

      // Get interpolated value from the map
      double mapGetValue( MapID id, double x, double y ) final;

      // Interpolate between 2 maps, coefficient in range [0:1]
      ErrorHandler::ReturnCode interpolateMap( MapID id, MapID minId, MapID maxId, double coeff ) final;

      // Interpolate input values using the natural neighbour algorithm
      ErrorHandler::ReturnCode interpolateMap( const std::vector<double> & xin
                                             , const std::vector<double> & yin
                                             , const std::vector<double> & vin
                                             , double                      xmin
                                             , double                      xmax
                                             , double                      ymin
                                             , double                      ymax
                                             , int                         numI
                                             , int                         numJ
                                             , std::vector<double>       & xout
                                             , std::vector<double>       & yout
                                             , std::vector<double>       & vout
                                             ) final;

      // Generate a new map in the GridMapIoTbl
      MapID generateMap(const std::string         & refferedTable
                       , const std::string         & mapName
                       , const std::vector<double> & values
                       , size_t                    & mapSequenceNbr
                       , const std::string         & filePathName
                       , const bool saveToHDF = true) final;

      // Generate a new map in the GridMapIoTbl
      MapID generateMap( const std::string              & refferedTable
                       , const std::string              & mapName
                       , DataAccess::Interface::GridMap * gridMap
                       , size_t                         & mapSequenceNbr
                       , const std::string              & filePathName
                       ) final;


      // Initialize the map writer
      ErrorHandler::ReturnCode initializeMapWriter( const std::string & filePathName, const bool append ) final;

      // Finalize the map writer
      ErrorHandler::ReturnCode finalizeMapWriter() final;

      // Remove GridMapIOTbl Reference
      ErrorHandler::ReturnCode removeMapReferenceFromGridMapIOTbl(const std::string &mapName, const std::string &referredBy) final;

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

      DataAccess::Interface::ProjectHandle            * m_proj;                    // project handle, to load/save maps
      std::string                                       m_projectFileName;         // project file name
      DataAccess::Interface::MapWriter                * m_mapPropertyValuesWriter; // own map writer to write/append maps to HDF file

      std::vector<std::string>                          m_mapName;
      std::vector<std::string>                          m_mapRefTable;

      std::vector<DataAccess::Interface::GridMap *>     m_mapObj;
      std::map<std::string, std::vector<std::string>>   m_fileMaps;     // for each HDF file, the vector of maps names
      std::map<std::string, size_t>                     m_seqNrMap;     // for each map, the map sequence number

      std::set<std::string>                             m_mapsFileList; // unique list of files with project maps

      void loadGridMap( MapID id );

      /// @brief Creates a map and registers it in the manager
      /// @param[out] id The map id
      void createMap( const std::string& refferedTable, const std::string& mapName, size_t& mapSequenceNbr,
         const std::string& filePathName, MapsManager::MapID& id, DataAccess::Interface::GridMap* gridMap );
   };
}

#endif // CMB_MAPS_MANAGER_IMPL_API

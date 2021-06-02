//
// Copyright (C) 2012-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

/// @file MapsManagerImpl.cpp
/// @brief This file keeps API implementation for manipulating input 2D maps

// CMB API
#include "MapsManagerImpl.h"
#include "UndefinedValues.h"

// MapSmoothing
#include "mapSmootherGridMap.h"
#include "mapSmootherVectorized.h"

// DataAccess
#include "database.h"
#include "GridMap.h"
#include "ProjectHandle.h"
#include "ProjectData.h"
#include "ObjectFactory.h"
#include "MapWriter.h"

// FileSystem library
#include "FilePath.h"
#include "FolderPath.h"

// Utilities lib
#include "NumericFunctions.h"

// NNlib
#include "nn.h"

// STL
#include <string>
#include <cmath>

namespace mbapi
{

const char * MapsManagerImpl::s_mapsTableName      = "GridMapIoTbl";  // table name for input maps list in project file
const char * MapsManagerImpl::s_ReferredByColName  = "ReferredBy";    // Name of the table which refer to this map, e.g. StratIoTbl
const char * MapsManagerImpl::s_MapNameColName     = "MapName";       // Input map name
const char * MapsManagerImpl::s_MapTypeColName     = "MapType";       // Type of the grid map, possible values are:DECASCII, DECBINARY, ZYCOR, CPS3, EPIRUS, XYZ
const char * MapsManagerImpl::s_MapFileNameColName = "MapFileName";   // Filename of the grid map (with extension)
const char * MapsManagerImpl::s_MapSeqNbrColName   = "MapSeqNbr";     // Sequence number of the grid map, within the grid loader (Starting with 0). This attribute
                                                                             // is only relevant for multiple map files.
const char * MapsManagerImpl::s_StratIoTbl         = "StratIoTbl";    // Table name reffered in the GridMapIoTbl for the lithofractions
//const char * MapsManagerImpl::s_mapResultFile      = "CasaModel_Results.HDF";
const char * MapsManagerImpl::s_mapResultFile      = "Inputs.HDF";

// Get list of input maps in the model
std::vector<MapsManager::MapID> MapsManagerImpl::mapsIDs() const
{
   std::vector<MapID> ids;
   if ( !m_proj ) return ids;

   // get pointer to the table
   database::Table * table = m_proj->getTable( s_mapsTableName );

   // if table does not exist - return empty array
   if ( !table ) return ids;

   assert( table->size() == m_mapName.size() );

   // fill IDs array with increasing indexes
   ids.resize( m_mapName.size(), 0 );

   for ( size_t i = 0; i < m_mapName.size(); ++i ) ids[ i ] = static_cast<MapID>( i );

   return ids;
}


// Search for map record which has given name
MapsManager::MapID MapsManagerImpl::findID( const std::string & mName )
{
   if ( errorCode() != NoError ) resetError();
   try
   {
      for ( size_t i = 0; i < m_mapName.size(); ++i )
      {
         if ( m_mapName[i] == mName )
         {
            return static_cast<MapID>( i );
         }
      }
   }
   catch ( const Exception & e ) { reportError( e.errorCode(), e.what() ); }

   return Utilities::Numerical::NoDataIDValue;
}

// Get list of map names
std::vector<std::string> MapsManagerImpl::mapNames() const
{
  return m_mapName;
}

// Get hash table of map names to map IDs
std::unordered_map<std::string, MapsManager::MapID> MapsManagerImpl::mapNameIDs() const
{
  std::unordered_map<std::string, MapsManager::MapID> myMapNameIDs;
  const std::vector<MapsManager::MapID> myMapsIDs = mapsIDs();

  for (const MapsManager::MapID & id : myMapsIDs)
  {
    myMapNameIDs[m_mapName[id]] = id;
  }

  return myMapNameIDs;
}

// Make a copy of the given map. Map must be saved in the separate call of MapManager::saveMapToHDF
MapsManager::MapID MapsManagerImpl::copyMap( MapID id, const std::string& newMapName, const std::string& newMapFile )
{
   if ( errorCode() != NoError ) resetError();

   MapID ret = Utilities::Numerical::NoDataIDValue;

   try
   {
      std::vector<double> vdata;
      mapGetValues( id, vdata );

      size_t sequenceNr;
      const std::string newMapFileName = (newMapFile != "") ? newMapFile : newMapName + ".HDF";
      const std::string referredTable = m_mapRefTable[id];
      const MapID newId = generateMap(referredTable, newMapName, vdata, sequenceNr, newMapFileName, false );

      // get pointer to the GridMapIo table
      database::Table * table = m_proj->getTable( s_mapsTableName );
      if ( !table ) { throw Exception( NonexistingID ) <<  s_mapsTableName << " table could not be found in project " << m_projectFileName; }

      // get record for copy
      database::Record* copyRec = table->getRecord( static_cast<int>( newId ) );
      if ( !copyRec ) { throw Exception( NonexistingID ) << "No input map with such ID: " << id; }

      // change the name
      copyRec->setValue(              s_MapNameColName,     newMapName );
      copyRec->setValue<std::string>( s_MapTypeColName,     "HDF5" );
      copyRec->setValue(              s_MapFileNameColName, newMapFileName );
      copyRec->setValue<int>(         s_MapSeqNbrColName,   sequenceNr );

      ret = newId;
   }
   catch( const Exception & ex ) { reportError( ex.errorCode(), ex.what() ); }

   return ret;
}

void MapsManagerImpl::createMap(const std::string& referredTable, const std::string& mapName, size_t& mapSequenceNbr, const std::string& filePathName,
      MapID& id, DataAccess::Interface::GridMap * gridMap)
   {

      ibs::FilePath mapFullPath( ibs::FilePath( m_projectFileName ).filePath() );

      // if filename is empty
      mapFullPath << ( filePathName.empty() ? s_mapResultFile : filePathName );

      const MapID pos = find( m_mapName.begin(), m_mapName.end(), mapName ) - m_mapName.begin();

      if ( pos == m_mapName.size() )
      {
         // create a new Map
         id = m_mapName.size();
         m_mapObj.push_back( gridMap );
         m_mapName.push_back( mapName );
         m_mapRefTable.push_back( referredTable );

         // get pointer to the GridMapIo table
         database::Table * table = m_proj->getTable( s_mapsTableName );
         if ( !table ) { throw Exception( NonexistingID ) << s_mapsTableName << " table could not be found in project " << m_projectFileName; }

         // create a new record in s_mapsTableName
         database::Record * newRec = table->createRecord();

         // determine the map sequence number (in this case the map is always a new one)
         const std::string mapFile = mapFullPath.path();
         if ( !m_fileMaps.count( mapFile ) )
         {
            // the first map of the file
            mapSequenceNbr = 0;
            m_fileMaps.insert( std::pair<std::string, std::vector<std::string>>( mapFile, std::vector<std::string>( 1, m_mapName[id] ) ) );
         }
         else
         {
            mapSequenceNbr = 0;
            for ( const std::string& name : m_fileMaps[mapFile] )
            {
               mapSequenceNbr = std::max( mapSequenceNbr, m_seqNrMap[name] );
            }
            ++mapSequenceNbr;

            m_fileMaps[mapFile].push_back( mapName ); //add the new name to m_fileMaps
         }

         m_seqNrMap[mapName] = mapSequenceNbr;

         // change names
         newRec->setValue<std::string>( s_ReferredByColName,  referredTable.c_str() );
         newRec->setValue(              s_MapNameColName,     mapName );
         newRec->setValue<std::string>( s_MapTypeColName,     "HDF5" );
         newRec->setValue<std::string>( s_MapFileNameColName, mapFullPath.fileName() );
         newRec->setValue<int>(         s_MapSeqNbrColName,   static_cast<int>( mapSequenceNbr ) );
      }
      else
      {
         m_mapObj[pos] = gridMap;
         m_mapRefTable[pos] = referredTable;
         id = pos;

         // map already exists
         std::vector<std::string>  maps = m_fileMaps[mapFullPath.path()];
         const auto position = std::find( maps.begin(), maps.end(), m_mapName[id] );
         mapSequenceNbr = position - maps.begin();
      }
   }

MapsManager::MapID MapsManagerImpl::generateMap( const std::string         & referredTable
                                               , const std::string         & mapName
                                               , const std::vector<double> & values
                                               , size_t                    & mapSequenceNbr
                                               , const std::string         & filePathName
                                               , const bool                  saveToHDF)
{
   if ( errorCode() != NoError ) resetError();

   MapID ret = Utilities::Numerical::NoDataIDValue;

   try
   {
      DataAccess::Interface::GridMap * gridMap = m_proj->getFactory()->produceGridMap( nullptr, 0, m_proj->getInputGrid(),
         Utilities::Numerical::CauldronNoDataValue, 1 );
      createMap(referredTable, mapName, mapSequenceNbr, filePathName, ret, gridMap);

      // set the values in the map
      mapSetValues( ret, values );

      // save the map to HDF
      if ( saveToHDF && ErrorHandler::ReturnCode::NoError != saveMapToHDF( ret, filePathName ) )
      {
         throw Exception( UnknownError ) << "Can not save HDF5 map";
      }
   }
   catch ( const Exception & ex ) { reportError( ex.errorCode(), ex.what() ); }

   return ret;
}

MapsManager::MapID MapsManagerImpl::generateMap( const std::string              & referredTable
                                               , const std::string              & mapName
                                               , DataAccess::Interface::GridMap * gridMap
                                               , size_t                         & mapSequenceNbr
                                               , const std::string              & filePathName
                                               )
{
   if ( errorCode() != NoError ) resetError();

   MapID ret = Utilities::Numerical::NoDataIDValue;

   try
   {
      createMap( referredTable, mapName, mapSequenceNbr, filePathName, ret, gridMap );

      // save the map to HDF
      if ( ErrorHandler::ReturnCode::NoError != saveMapToHDF( ret, filePathName ) )
      {
         throw Exception( UnknownError ) << "Can not save HDF5 map";
      }
   }
   catch ( const Exception & ex ) { reportError( ex.errorCode(), ex.what() ); }

   return ret;
}

ErrorHandler::ReturnCode MapsManagerImpl::initializeMapWriter( const std::string & filePathName, const bool append )
{
   if ( errorCode() != NoError ) resetError();
   try
   {
      m_mapPropertyValuesWriter = m_proj->getFactory()->produceMapWriter();
      bool status = m_mapPropertyValuesWriter->open( filePathName, append );
      if ( status && !append )
      {
         // Note: we assume that all maps written by MapsManagerImpl are full resolution
         status = m_mapPropertyValuesWriter->saveDescription( m_proj->getInputGrid() );
      }
      if ( !status ) throw Exception( UnknownError ) << "Map writer not instantiated correctly";
   }
   catch ( const Exception & ex ) { reportError( ex.errorCode(), ex.what() ); }

   return NoError;
}

ErrorHandler::ReturnCode MapsManagerImpl::finalizeMapWriter()
{

   if ( errorCode() != NoError ) resetError();
   try
   {
      if ( !m_mapPropertyValuesWriter ) { throw Exception( UnknownError ) << "Map writer not allocated, can not delete "; }

      const bool status = m_mapPropertyValuesWriter->close();
      if ( !status ) { throw Exception( UnknownError ) << "Map writer not closed correctly "; }
      delete m_mapPropertyValuesWriter;
      m_mapPropertyValuesWriter = nullptr;
   }
   catch ( const Exception & ex ) { reportError( ex.errorCode(), ex.what() ); }

   return NoError;
}

ErrorHandler::ReturnCode MapsManagerImpl::removeMapReferenceFromGridMapIOTbl(const string& mapName, const string& referredBy)
{
  // Delete Reference in GridMapIOTbl
  database::Table * table = m_proj->getTable( s_mapsTableName );
  if ( !table ) { throw Exception( NonexistingID ) << s_mapsTableName << " table could not be found in project " << m_projectFileName; }

  database::Record* record = table->findRecord("ReferredBy", referredBy, "MapName", mapName);
  table->removeRecord(record);

  // Delete reference in member containing all the used map names
  const auto pos = std::find( m_mapName.begin(), m_mapName.end(), mapName );
  if (pos != m_mapName.end())
  {
    m_mapName.erase(pos);
  }

  return NoError;
}

ErrorHandler::ReturnCode MapsManagerImpl::mapSetValues( MapID id, const std::vector<double>& vin )
{
   if ( errorCode() != NoError ) resetError();
   try
   {
      loadGridMap( id );
      DataAccess::Interface::GridMap* gridMap = m_mapObj[id];
      gridMap->retrieveData();

      const double nulVal = gridMap->getUndefinedValue();

      int k = 0;
      for ( unsigned int j = gridMap->firstJ(); j <= gridMap->lastJ(); ++j )
      {
         for ( unsigned int i = gridMap->firstI(); i <= gridMap->lastI(); ++i )
         {
            const double v = vin[k++];
            gridMap->setValue( i, j, NumericFunctions::isEqual( v, nulVal, 1e-5 ) ? nulVal : v );
         }
      }
      gridMap->restoreData();
   }
   catch ( const Exception & ex ) { return reportError( ex.errorCode(), ex.what() ); }

   return NoError;
}

ErrorHandler::ReturnCode MapsManagerImpl::mapGetValues( MapID id, std::vector<double> & vout )
{
   if ( errorCode() != NoError ) resetError();
   try
   {
      loadGridMap( id ); // check if map is loaded and load it if not loaded before
      DataAccess::Interface::GridMap* gridMap = m_mapObj[id];
      gridMap->retrieveData();

      const int numI = gridMap->lastI() - gridMap->firstI() + 1;
      const int numJ = gridMap->lastJ() - gridMap->firstJ() + 1;
      vout.resize( numI * numJ );

      int k = 0;
      for ( unsigned int j = gridMap->firstJ(); j <= gridMap->lastJ(); ++j )
      {
         for ( unsigned int i = gridMap->firstI(); i <= gridMap->lastI(); ++i )
         {
            vout[k++] = gridMap->getValue( i, j );
         }
      }
      gridMap->restoreData();
   }
   catch ( const Exception & ex ) { return reportError( ex.errorCode(), ex.what() ); }

   return NoError;
}

double MapsManagerImpl::mapGetValue( MapID id, unsigned int i, unsigned int j )
{
   if ( errorCode() != NoError ) resetError();

   double value = 0.0;
   try
   {
     loadGridMap( id ); // check if map is loaded and load it if not loaded before
     m_mapObj[id]->retrieveData();
     value = m_mapObj[id]->getValue( static_cast<unsigned int>( i ), static_cast<unsigned int>( j ) );
     m_mapObj[id]->restoreData();
   }
   catch ( const Exception & ex ) { reportError( ex.errorCode(), ex.what() ); }

   return value;
}

double MapsManagerImpl::mapGetValue( MapID id, double x, double y )
{
   if ( errorCode() != NoError ) resetError();

   double value = 0.0;
   try
   {
     loadGridMap( id ); // check if map is loaded and load it if not loaded before
     const GridMap* map = m_mapObj[id];
     map->retrieveData();
     const double i = (x - map->minI()) / map->deltaI();
     const double j = (y - map->minJ()) / map->deltaJ();
     value = map->getValue(  i, j );
     map->restoreData();
   }
   catch ( const Exception & ex ) { reportError( ex.errorCode(), ex.what() ); }

   return value;
}

// Save input map to the new HDF file. File with the given name should not exist before.
ErrorHandler::ReturnCode MapsManagerImpl::saveMapToHDF( MapID id, const std::string & fileName )
{
   if ( errorCode() != NoError ) resetError();
   try
   {
      if ( !m_proj ) throw Exception( NonexistingID ) << "MapsManager::copyMap(): No project file was loaded";

      if ( id >= m_mapName.size() ) { throw Exception( NonexistingID ) <<  "No map with given ID: " << id; }

      // get pointer to the GridMapIo table
      database::Table * table = m_proj->getTable( s_mapsTableName );
      if ( !table ) { throw Exception( NonexistingID ) <<  s_mapsTableName << " table could not be found in project " << m_projectFileName; }

      // load source map
      if ( !m_mapObj[id] )
      {
         throw Exception( UndefinedValue ) << "MapManager::saveMapToHDF(): Map " << m_mapName[id] << " wasn't modified. Nothing to save";
      }

      // get record from table for the given map
      database::Record * rec = table->getRecord( static_cast<int>( id ) );
      if ( !rec ) { throw Exception( NonexistingID ) << "No input map with such ID: " << id; }

      // get only the file name from the file path (if given)
      std::string mfName = fileName.empty() ? std::string( "" ) : ibs::FilePath( fileName ).fileName();
      if ( mfName.empty() ) // if no file name is given - take file name from the map record
      {
         mfName = rec->getValue<std::string>( s_MapFileNameColName );
         if ( mfName.empty() ) throw Exception( UndefinedValue ) << "No file name is given to save input map " << m_mapName[id];
      }

      // check if given file name has path and this path is equal with path to project file
      const std::string givenPath = ibs::FilePath( fileName ).filePath();
      if ( !givenPath.empty() && givenPath != ibs::FilePath( m_projectFileName ).filePath() )
      {
         throw Exception( OutOfRangeValue ) << "Given path to save map " << m_mapName[id] << ": " << givenPath <<
            ", is different from the path where project is located: " << ibs::FilePath( m_projectFileName ).filePath();
      }
      ibs::FilePath mapFullPath( givenPath.empty() ? ibs::FilePath( m_projectFileName ).filePath() : givenPath );
      mapFullPath << mfName;

      // write map to HDF using the MapWriter. if the HDF file already exist append
      const bool append = mapFullPath.exists();
      if ( NoError != initializeMapWriter( mapFullPath.path(), append ) )
      {
         throw Exception( OutOfRangeValue ) << "Could not inizialize the map writer ";
      }

      const bool writingSucceed = m_mapPropertyValuesWriter->writeInputMap( m_mapObj[id], static_cast<int>( rec->getValue<int>( s_MapSeqNbrColName ) ) );
      if ( !writingSucceed && !append )
      {
         throw Exception( IoError ) << "Can not write the map " << m_mapName[id] << " to HDF ";
      }
      if ( NoError != finalizeMapWriter() )
      {
         throw Exception( OutOfRangeValue ) << "Could not finilize the map writer ";
      }

      // add new file name to the maps file list if it is not present
      if ( std::find( m_mapsFileList.begin(), m_mapsFileList.end(), mapFullPath.path() ) == m_mapsFileList.end() )
      {
         m_mapsFileList.insert( mapFullPath.path() );
      }
      // set real file name in map record
      rec->setValue( s_MapFileNameColName, mfName );

   }
   catch( const Exception & ex ) { return reportError( ex.errorCode(), ex.what() ); }

   return NoError;
}


// Get min/max map values range
ErrorHandler::ReturnCode MapsManagerImpl::mapValuesRange( MapID id, double & minV, double & maxV )
{
   if ( errorCode() != NoError ) resetError();
   try
   {
      loadGridMap( id ); // check if map is loaded and load it if not loaded before

      // get the first value as initial value for min/max
      bool firstVal = true;
      const double nulVal = m_mapObj[id]->getUndefinedValue();

      for (    unsigned int i = m_mapObj[id]->firstI(); i <= m_mapObj[id]->lastI(); ++i )
      {
         for ( unsigned int j = m_mapObj[id]->firstJ(); j <= m_mapObj[id]->lastJ(); ++j )
         {
            double v = m_mapObj[id]->getValue( i, j );
            if ( NumericFunctions::isEqual( v, nulVal, 1e-5 ) ) { continue; } // skip undefined values

            if ( firstVal )
            {
               minV = maxV = v;
               firstVal = false;
            }
            else
            {
               minV = v < minV ? v : minV;
               maxV = v > maxV ? v : maxV;
            }
         }
      }
   }
   catch( const Exception & ex ) { return reportError( ex.errorCode(), ex.what() ); }

   return NoError;
}

// Linearly rescale input map to the new value range
ErrorHandler::ReturnCode MapsManagerImpl::scaleMap( MapID id, double coeff )
{
   if ( errorCode() != NoError ) resetError();
   try
   {
      double oldMin;
      double oldMax;

      if ( NoError != mapValuesRange( id, oldMin, oldMax ) ) throw ErrorHandler::Exception( errorCode() ) << errorMessage();

      DataAccess::Interface::GridMap* gridmap = m_mapObj[id];
      const double nulVal = gridmap->getUndefinedValue();

      for ( unsigned int i = gridmap->firstI(); i <= gridmap->lastI(); ++i )
      {
         for ( unsigned int j = gridmap->firstJ(); j <= gridmap->lastJ(); ++j )
         {
            const double v = gridmap->getValue( i, j );
            gridmap->setValue( i, j, NumericFunctions::isEqual( v, nulVal, 1e-5 ) ? nulVal : v * coeff );
         }
      }
   }
   catch( const Exception & ex ) { return reportError( ex.errorCode(), ex.what() ); }

   return NoError;
}

ErrorHandler::ReturnCode MapsManagerImpl::smoothenGridMap( MapsManager::MapID id, const int method,
                                                           const double smoothingRadius, const unsigned int nrOfThreads )
{
  if ( errorCode() != NoError ) resetError();
  try
  {
    double oldMin;
    double oldMax;
    if ( NoError != mapValuesRange( id, oldMin, oldMax ) )
    {
      throw ErrorHandler::Exception( errorCode() ) << errorMessage();
    }

    std::unique_ptr<MapSmoothing::MapSmoother> mapSmoother( new MapSmoothing::MapSmootherGridMap( m_mapObj[id], smoothingRadius, nrOfThreads) );
    if ( method == 0 ) mapSmoother->doSmoothing( MapSmoothing::FilterType::Gaussian );
    if ( method == 1 ) mapSmoother->doSmoothing( MapSmoothing::FilterType::MovingAverage );
  }
  catch( const Exception & ex )
  {
    return reportError( ex.errorCode(), ex.what() );
  }

  return NoError;
}

ErrorHandler::ReturnCode MapsManagerImpl::smoothenVectorizedMap( std::vector<double>& vec, const int method,
                                                                 const unsigned int numI, const unsigned int numJ,
                                                                 const double dx, const double dy,
                                                                 const double smoothingRadius, const double undefinedValue,
                                                                 const unsigned int nrOfThreads) const
{
  std::unique_ptr<MapSmoothing::MapSmoother> mapSmoother(
        new MapSmoothing::MapSmootherVectorized( vec, undefinedValue, smoothingRadius,dx , dy, numI, numJ, nrOfThreads ) );
  if ( method == 0 ) mapSmoother->doSmoothing( MapSmoothing::FilterType::Gaussian );
  if ( method == 1 ) mapSmoother->doSmoothing( MapSmoothing::FilterType::MovingAverage );
  return ErrorHandler::NoError;
}

// Shift the input map by a constant value
ErrorHandler::ReturnCode MapsManagerImpl::scaleAndShiftMapCorrectedForWells( MapID id, double scale, double shift, double radiusOfInfluence,
                                                                             const std::vector<double>& xWells, const std::vector<double>& yWells)
{
  if ( errorCode() != NoError ) resetError();
  try
  {
    if ( xWells.size() != yWells.size() )
    {
      throw Exception( IoError ) << "Number of x and y locations should be equal";
    }

    double oldMin;
    double oldMax;
    if ( NoError != mapValuesRange( id, oldMin, oldMax ) )
    {
      throw ErrorHandler::Exception( errorCode() ) << errorMessage();
    }

    DataAccess::Interface::GridMap* gridmap = m_mapObj[id];
    const double nulVal = gridmap->getUndefinedValue();

    const double minI = gridmap->minI();
    const double minJ = gridmap->minJ();
    const double deltaI = gridmap->deltaI();
    const double deltaJ = gridmap->deltaJ();
    const double maxI = minI + deltaI * gridmap->numI();
    const double maxJ = minJ + deltaJ * gridmap->numJ();

    for ( unsigned int w = 0; w<xWells.size(); ++w)
    {
      const double x = xWells[w];
      const double y = yWells[w];

      if ( x < minI || x > maxI || y < minJ || y > maxJ )
      {
        throw Exception( IoError ) << "Well location " << x << ", " << y << " out of map range";
      }
    }

    const double radiusOfInfluenceSquared = radiusOfInfluence*radiusOfInfluence;
    const double maxDistanceSquared = (maxI-minI)*(maxI-minI) + (maxJ-minJ)*(maxJ-minJ);
    for ( unsigned int i = gridmap->firstI(); i <= gridmap->lastI(); ++i )
    {
      for ( unsigned int j = gridmap->firstJ(); j <= gridmap->lastJ(); ++j )
      {
        double wellCorrection = 1.0;
        if (xWells.size() > 0 && radiusOfInfluence > 0.0)
        {
          double distanceSquaredClosestWell = maxDistanceSquared;
          for ( unsigned int w = 0; w<xWells.size(); ++w)
          {
            const double dx = xWells[w] - (minI + deltaI * i);
            const double dy = yWells[w] - (minJ + deltaJ * j);
            const double distanceSquared = dx*dx + dy*dy;
            distanceSquaredClosestWell = (distanceSquared<distanceSquaredClosestWell) ? distanceSquared : distanceSquaredClosestWell;
          }

          wellCorrection -= std::exp(-5.0*distanceSquaredClosestWell/radiusOfInfluenceSquared);
        }
        const double v = gridmap->getValue( i, j );
        gridmap->setValue( i, j, NumericFunctions::isEqual( v, nulVal, 1e-5 ) ?
                           nulVal : (1.0-wellCorrection) * v  +  wellCorrection*(v*scale + shift) );
      }
    }
  }
  catch( const Exception & ex ) { return reportError( ex.errorCode(), ex.what() ); }

  return NoError;
}

ErrorHandler::ReturnCode MapsManagerImpl::interpolateMap( MapID id, MapID minId, MapID maxId, double coeff )
{
   if ( errorCode() != NoError ) resetError();
   try
   {
      const double nulVal = m_mapObj[id]->getUndefinedValue();

      if ( !m_mapObj[minId] ) { loadGridMap( minId ); }
      if ( !m_mapObj[maxId] ) { loadGridMap( maxId ); }

      for (    unsigned int i = m_mapObj[id]->firstI(); i <= m_mapObj[id]->lastI(); ++i )
      {
         for ( unsigned int j = m_mapObj[id]->firstJ(); j <= m_mapObj[id]->lastJ(); ++j )
         {
            double minV = m_mapObj[minId]->getValue( i, j );
            double maxV = m_mapObj[maxId]->getValue( i, j );

            if ( NumericFunctions::isEqual( minV, nulVal, 1e-5 ) || NumericFunctions::isEqual( maxV, nulVal, 1.e-5 ) )
            {
               m_mapObj[id]->setValue( i, j, nulVal );
               continue;
            }

            m_mapObj[id]->setValue( i, j, minV + (maxV - minV) * coeff );
         }
      }
   }
   catch( const Exception & ex ) { return reportError( ex.errorCode(), ex.what() ); }

   return NoError;
}

ErrorHandler::ReturnCode MapsManagerImpl::interpolateMap( const std::vector<double> & xin
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
                                                        )
{
   if ( errorCode() != NoError ) resetError();
   try
   {      
      std::vector<point> pin;
      point * pout = nullptr;
      int     nout;
      const double wmin = -1.e-4;
      const double tolerance = 1.e-10;

      size_t j = 0;
      for ( size_t i = 0; i < xin.size(); ++i )
      {
         bool noDuplicate = true;
         for ( size_t k = 0; k < j; ++k)
         {
            if (std::fabs(xin[i] - pin[k].x) < tolerance && std::fabs(yin[i] - pin[k].y) < tolerance)
            {
               pin[k].z = (pin[k].z + vin[i]) * 0.5;
               noDuplicate = false;
               break;
            }
         }
         if (noDuplicate)
         {
             point p;
             p.x = xin[i];
             p.y = yin[i];
             p.z = vin[i];
             pin.push_back(p);
             j++;
         }
      }
      const int nin = pin.size();

      // generate the points, interpolate with NNlib
      points_getrange( nin, pin.data(), 1, &xmin, &xmax, &ymin, &ymax );
      points_generate( xmin, xmax, ymin, ymax, numI, numJ, &nout, &pout );
      nnpi_interpolate_points( nin, pin.data(), wmin, nout, pout );

      xout.resize( nout );
      yout.resize( nout );
      vout.resize( nout );

      for ( int i = 0; i != nout; ++i )
      {
         xout[i] = pout[i].x;
         yout[i] = pout[i].y;
         vout[i] = pout[i].z;
      }

      free( pout );
   }
   catch ( const Exception & ex ) { return reportError( ex.errorCode(), ex.what() ); }

   return NoError;
}

void MapsManagerImpl::clearMaps()
{
  // clear arrays
  m_mapName.clear();
  m_mapRefTable.clear();
  m_mapObj.clear();

  m_fileMaps.clear();
  m_seqNrMap.clear();

  // clear list of input maps files

  m_mapsFileList.clear();
}

// Set project database. Reset all
void MapsManagerImpl::setProject( DataAccess::Interface::ProjectHandle * ph, const std::string & projectFileName )
{
   m_projectFileName = projectFileName;
   m_proj = ph;

   clearMaps();

   // collecting map names and map files name
   // get pointer to the table
   database::Table * table = m_proj->getTable( s_mapsTableName );

   if ( !table ) return; // no table - no maps

   const size_t recNum = table->size();

   m_mapObj.resize( recNum, nullptr );

   ibs::FilePath projectFile( projectFileName );
   std::string projectPath = projectFile.filePath();
   if ( projectPath.empty() ) projectPath = ".";

   // go over all records in the table and collect names
   for ( size_t i = 0; i < recNum; ++i )
   {
      database::Record * rec = table->getRecord( static_cast<int>( i ) );
      if ( rec )
      {
         const std::string & fname = rec->getValue<std::string>( s_MapFileNameColName );
         if ( !fname.empty() )
         {
            // construct the full file path to the original map file
            ibs::FolderPath mapFilePath( projectPath );
            mapFilePath << fname;
            // add it to the list
            if ( !m_mapsFileList.count( mapFilePath.path() ) )
            {
               m_mapsFileList.insert( mapFilePath.path() );
               m_fileMaps.insert( std::pair<std::string, std::vector<std::string>>( mapFilePath.path(), std::vector<std::string>() ) );
            }
         }

         const std::string & mapName = rec->getValue<std::string>( s_MapNameColName );
         const std::string & refTbl  = rec->getValue<std::string>( s_ReferredByColName );
         if ( !mapName.empty() )
         {
            m_mapName.push_back( mapName );
            m_mapRefTable.push_back( refTbl );
            m_seqNrMap[mapName] = rec->getValue<int>( s_MapSeqNbrColName );
            if ( !fname.empty() )
            {
               // construct the full file path to the original map file
               ibs::FolderPath mapFilePath( projectPath );
               mapFilePath << fname;
               m_fileMaps[mapFilePath.path()].push_back( mapName );
            }
         }
      }
   }
}

// copying maps to the given location
void MapsManagerImpl::copyMapFiles( const std::string & newLocation )
{
   for ( std::set<std::string>::const_iterator it = m_mapsFileList.begin(); it != m_mapsFileList.end(); ++it )
   {
      ibs::FilePath origMapFile( *it );
      ibs::FilePath newMapFile( newLocation );
      newMapFile << origMapFile.fileName();

      if ( !newMapFile.exists() && !origMapFile.copyFile( newMapFile ) )
      {
        throw ErrorHandler::Exception( ErrorHandler::IoError ) << "Model::saveModelToProjectFile(): can not copy map file: " << origMapFile.path();
      }
   }
}

// load grid map with given ID
void MapsManagerImpl::loadGridMap( MapID id )
{
   if ( id >= m_mapName.size() ) { throw Exception( NonexistingID ) <<  "No map with given ID: " << id; }

   if ( m_mapObj[id] ) return; // nothing to do, map is already loaded

   // get pointer to the GridMapIo table
   database::Table * table = m_proj->getTable( s_mapsTableName );
   if ( !table ) { throw Exception( NonexistingID ) <<  s_mapsTableName << " table could not be found in project " << m_projectFileName; }

   // load source map
   if ( !m_mapObj[id] )
   {
      m_mapObj[id] = m_proj->loadInputMap( m_mapRefTable[id], m_mapName[id] );
      if ( !m_mapObj[id] ) throw Exception( IoError ) << "MapsManager::copyMap() failed to load map " << m_mapName[id] << " in " << m_projectFileName;
   }

   // load source map
   if ( !m_mapObj[id] ) { throw Exception( UndefinedValue ) << "MapManager::saveMapToHDF(): Map " << m_mapName[id] << " wasn't modified. Nothing to save"; }
}

ErrorHandler::ReturnCode mbapi::MapsManagerImpl::mapGetDimensions(mbapi::MapsManager::MapID id, int& i, int& j)
{
  try
  {
    loadGridMap( id ); // check if map is loaded and load it if not loaded before

    DataAccess::Interface::GridMap* gridMap = m_mapObj[id];
    i = gridMap->lastI() - gridMap->firstI() + 1;
    j = gridMap->lastJ() - gridMap->firstJ() + 1;
  }
  catch (Exception& ex)
  {
    return reportError( ex.errorCode(), ex.what());
  }
  return NoError;
}

}

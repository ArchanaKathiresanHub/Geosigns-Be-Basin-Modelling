//                                                                      
// Copyright (C) 2012-2015 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file MapsManagerImpl.C
/// @brief This file keeps API implementation for manipulating input 2D maps

// CMB API
#include "MapsManagerImpl.h"
#include "UndefinedValues.h"

// DataAccess
#include "database.h"
#include "Interface/GridMap.h"
#include "Interface/ProjectHandle.h"
#include "Interface/ProjectData.h"
#include "Interface/ObjectFactory.h"
#include "Interface/MapWriter.h"

// FileSystem library
#include "FilePath.h"
#include "FolderPath.h"

// Utilities lib
#include <NumericFunctions.h>

// NNlib
#include "nn.h"

// STL
#include <stdexcept>
#include <string>
#include <cmath>
#include <sstream>

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
const char * MapsManagerImpl::s_mapResultFile      = "CasaModel_Results.HDF";

MapsManagerImpl::MapsManagerImpl()
{
}


MapsManagerImpl::~MapsManagerImpl()
{
}


// Get list of input maps in the model
std::vector<MapsManager::MapID> MapsManagerImpl::mapsIDs() const
{
   std::vector<MapID> ids;
   if ( !m_db ) return ids;

   // get pointer to the table
   database::Table * table = m_db->getTable( s_mapsTableName );

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

   return UndefinedIDValue;
}


// Make a copy of the given map. Map must be saved in the separate call of MapManager::saveMapToHDF
MapsManager::MapID MapsManagerImpl::copyMap( MapID id, const std::string & newMapName )
{
   if ( errorCode() != NoError ) resetError();

   MapID ret = UndefinedIDValue;

   try
   {
      loadGridMap( id );
  
      // create a copy
      MapID newId = m_mapName.size();

      m_mapObj.push_back( m_mapObj[id] );
      m_mapObj[id] = NULL;

      m_mapName.push_back( newMapName );
      m_mapRefTable.push_back( m_mapRefTable[id] );

      std::string newMapFile = newMapName + ".HDF";

      // get pointer to the GridMapIo table
      database::Table * table = m_db->getTable( s_mapsTableName );
      if ( !table ) { throw Exception( NonexistingID ) <<  s_mapsTableName << " table could not be found in project " << m_projectFileName; }

      // get record for copy
      database::Record * origRec = table->getRecord( static_cast<int>( id ) );
      if ( !origRec ) { throw Exception( NonexistingID ) << "No input map with such ID: " << id; }

      // create map copy
      database::Record * copyRec = new database::Record( *origRec );

      // change the name
      copyRec->setValue(              s_MapNameColName,     newMapName );
      copyRec->setValue<std::string>( s_MapTypeColName,     "HDF5" );
      copyRec->setValue(              s_MapFileNameColName, newMapFile ); // not saved yet
      copyRec->setValue<int>(         s_MapSeqNbrColName,   0 );

      // add copy record with new name to the table end
      table->addRecord( copyRec );

      ret = newId;
   }
   catch( const Exception & ex ) { reportError( ex.errorCode(), ex.what() ); }

   return ret;
}

MapsManager::MapID MapsManagerImpl::generateMap( const std::string & refferedTable, const std::string mapName, const std::vector<double>& values, int & mapSequenceNbr, const std::string & filePathName )
{
   if ( errorCode() != NoError ) resetError();

   MapID ret = UndefinedIDValue;

   try
   {
      DataAccess::Interface::GridMap * gridMap = m_proj->getFactory( )->produceGridMap( 0, 0, m_proj->getInputGrid( ), DataAccess::Interface::DefaultUndefinedMapValue, 1 );
      ibs::FilePath mapFullPath(ibs::FilePath( m_projectFileName ).filePath( ));

      // if filename is empty 
      if ( filePathName.empty( ) ) 
         mapFullPath << s_mapResultFile;
      else
         mapFullPath << filePathName;

      MapID pos = find( m_mapName.begin( ), m_mapName.end( ), mapName ) - m_mapName.begin( );

      if ( pos == m_mapName.size( ) )
      {
         // create a new Map
         ret = m_mapName.size();
         m_mapObj.push_back( gridMap );
         m_mapName.push_back( mapName );
         m_mapRefTable.push_back( refferedTable );

         // get pointer to the GridMapIo table
         database::Table * table = m_db->getTable( s_mapsTableName );
         if ( !table ) { throw Exception( NonexistingID ) << s_mapsTableName << " table could not be found in project " << m_projectFileName; }

         // create a new record in s_mapsTableName
         database::Record * newRec = table->createRecord();

         //determine the map sequence number (in this case the map is always a new one)
         std::string mapFile = mapFullPath.path( );
         if ( !m_fileMaps.count( mapFile ) )
         {
            //the first map of the file
            mapSequenceNbr = 0;
         }
         else
         {
            //new map needs to be created
            mapSequenceNbr = m_fileMaps[mapFile].size( );
         }

         // change names
         newRec->setValue<std::string>( s_ReferredByColName,  refferedTable.c_str() );
         newRec->setValue(              s_MapNameColName,     mapName );
         newRec->setValue<std::string>( s_MapTypeColName,     "HDF5" );
         newRec->setValue<std::string>( s_MapFileNameColName, mapFullPath.fileName( ) );
         newRec->setValue<int>(         s_MapSeqNbrColName,   mapSequenceNbr );
      }
      else
      {
         m_mapObj[pos] = gridMap;
         m_mapRefTable[pos] = refferedTable;
         ret = pos;
      }

      // set the values in the map
      mapSetValues( ret, values );

      // save the map to HDF
      saveMapToHDF( ret, filePathName );
   }
   catch ( const Exception & ex ) { reportError( ex.errorCode(), ex.what() ); }

   return ret;
}

ErrorHandler::ReturnCode MapsManagerImpl::inizializeMapWriter( const std::string & filePathName, const bool append )
{
   if ( errorCode( ) != NoError ) resetError( );
   try
   {
      bool status = false;
      m_mapPropertyValuesWriter = m_proj->getFactory()->produceMapWriter();
      status = m_mapPropertyValuesWriter->open( filePathName, append );
      if ( status )
      {
         // Note: we assume that all maps written by MapsManagerImpl are full resolution
         status = m_mapPropertyValuesWriter->saveDescription( m_proj->getInputGrid() );
      }
      if ( !status ) throw Exception( UnknownError ) << "Map writer not instantiated correctly ";
   }
   catch ( const Exception & ex ) { reportError( ex.errorCode( ), ex.what( ) ); }

   return NoError;
}


ErrorHandler::ReturnCode MapsManagerImpl::finalizeMapWriter( )
{
   
   if ( errorCode( ) != NoError ) resetError( );
   try
   {
      bool status = false;
      if ( !m_mapPropertyValuesWriter ) throw Exception( UnknownError ) << "Map writer not allocated, cannot delete ";

      status = m_mapPropertyValuesWriter->close( );
      if ( !status ) throw Exception( UnknownError ) << "Map writer not closed correctly ";
      delete m_mapPropertyValuesWriter;
      m_mapPropertyValuesWriter = 0;
   }
   catch ( const Exception & ex ) { reportError( ex.errorCode( ), ex.what( ) ); }

   return NoError;
}

ErrorHandler::ReturnCode MapsManagerImpl::mapSetValues( MapID id, const std::vector<double>& vin )
{
 
   if ( errorCode() != NoError ) resetError();
   try
   {
      loadGridMap( id );

      m_mapObj[id]->retrieveData();

      const double nulVal = m_mapObj[id]->getUndefinedValue();

      const DataAccess::Interface::ProjectData * pd = m_proj->getProjectData();

      int numI = m_mapObj[id]->lastI( ) + 1;

      for ( unsigned int j = m_mapObj[id]->firstJ( ); j <= m_mapObj[id]->lastJ( ); ++j )
      {
         for ( unsigned int i = m_mapObj[id]->firstI( ); i <= m_mapObj[id]->lastI( ); ++i )
      {
            unsigned int pos = i + j * numI; //values in vin are saved row-wise
            m_mapObj[id]->setValue( i, j, NumericFunctions::isEqual( vin[pos], nulVal, 1e-5 ) ? nulVal : vin[pos] );
         }
      }

      m_mapObj[id]->restoreData( );
   }
   catch ( const Exception & ex ) { return reportError( ex.errorCode( ), ex.what( ) ); }

   return NoError;
}

ErrorHandler::ReturnCode MapsManagerImpl::mapGetValues( MapID id, std::vector<double>& vout )
{
   if ( errorCode( ) != NoError ) resetError( );
   try
   {
      loadGridMap( id ); // check if map is loaded and load it if not loaded before

      m_mapObj[id]->retrieveData();

      const double nulVal = m_mapObj[id]->getUndefinedValue( );

      int numI = m_mapObj[id]->lastI( ) +1;
      int numJ = m_mapObj[id]->lastJ( ) +1;
      vout.resize( numI * numJ );

      for ( unsigned int j = m_mapObj[id]->firstJ(); j <= m_mapObj[id]->lastJ(); ++j )
      {
         for ( unsigned int i = m_mapObj[id]->firstI(); i <= m_mapObj[id]->lastI(); ++i )
         {
               vout[i + j * numI] = m_mapObj[id]->getValue( i, j );
         }
      }

      m_mapObj[id]->restoreData( );
   }
   catch ( const Exception & ex ) { return reportError( ex.errorCode(), ex.what() ); }

   return NoError;
}

// Save input map to the new HDF file. File with the given name should not exist before.
ErrorHandler::ReturnCode MapsManagerImpl::saveMapToHDF( MapID id, const std::string & fileName )
{
   if ( errorCode() != NoError ) resetError();
   try
   {
      if ( !m_db ) throw Exception( NonexistingID ) << "MapsManager::copyMap(): No project file was loaded";

      if ( id >= m_mapName.size() ) { throw Exception( NonexistingID ) <<  "No map with given ID: " << id; }

      // get pointer to the GridMapIo table
      database::Table * table = m_db->getTable( s_mapsTableName );
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
      bool append = mapFullPath.exists() ? true :false;
      if ( NoError != inizializeMapWriter( mapFullPath.path().c_str(), append ) )
      {
         throw Exception( OutOfRangeValue ) << "Could not inizialize the map writer ";
      }

      //determine the map sequence number
      int mapSequenceNbr;
      if ( !m_fileMaps.count(mapFullPath.path( )) )
      {
         mapSequenceNbr = 0;
         m_fileMaps.insert( std::pair<std::string, std::vector<std::string>>( mapFullPath.path( ), std::vector<std::string>( ) ) );
         m_fileMaps[mapFullPath.path( )].push_back( m_mapName[id] );
      }
      else
      {
         // handle also the case of overwriting an exsisting map
         std::vector<std::string>  maps = m_fileMaps[mapFullPath.path()];
         auto pos = std::find( maps.begin( ), maps.end( ), m_mapName[id] );
         if ( pos == maps.end( ) )
         {
            mapSequenceNbr = maps.size( );
            m_fileMaps[mapFullPath.path( )].push_back( m_mapName[id] ); //add the new name to m_fileMaps
         }
         else
      {
            mapSequenceNbr = pos - maps.begin( );                       //map already exist
         }
      }

      bool writingSucceed = m_mapPropertyValuesWriter->writeMapToHDF( m_mapObj[id], 0.0, 0.0, std::to_string( mapSequenceNbr ), m_mapName[id], true );
      if ( !writingSucceed )
      {
         throw Exception( IoError ) << "Can not write the map " << m_mapName[id] << " to HDF ";
      }
      if ( NoError != finalizeMapWriter( ) )
      {
         throw Exception( OutOfRangeValue ) << "Could not finilize the map writer ";
      }

      // add new file name to the maps file list if it is not present
      if ( std::find( m_mapsFileList.begin( ), m_mapsFileList.end( ), mapFullPath.path( ) ) == m_mapsFileList.end( ) )
      {
         m_mapsFileList.insert( mapFullPath.path( ) );
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

      double nulVal = m_mapObj[id]->getUndefinedValue();

      for (    unsigned int i = m_mapObj[id]->firstI(); i <= m_mapObj[id]->lastI(); ++i )
      {
         for ( unsigned int j = m_mapObj[id]->firstJ(); j <= m_mapObj[id]->lastJ(); ++j )
         {
            double v = m_mapObj[id]->getValue( i, j );

            m_mapObj[id]->setValue( i, j, NumericFunctions::isEqual( v, nulVal, 1e-5 ) ? nulVal : v * coeff );
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
      double nulVal = m_mapObj[id]->getUndefinedValue();
      
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

ErrorHandler::ReturnCode MapsManagerImpl::interpolateMap( 
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
   std::vector<double>& vout )
{
   if ( errorCode() != NoError ) resetError();
   try
   {
      int nin = xin.size();
      point * pin = (point *)malloc( nin * sizeof( point ) );
      point*  pout = NULL;
      int     nout;
      double wmin = 0;

      for ( int i = 0; i != nin; ++i )
      {
         pin[i].x = xin[i];
         pin[i].y = yin[i];
         pin[i].z = vin[i];
      }

      // generate the points, interpolate with NNlib
      points_getrange( nin, pin, 1, &xmin, &xmax, &ymin, &ymax );
      points_generate( xmin, xmax, ymin, ymax, numI, numJ, &nout, &pout );  
      nnpi_interpolate_points( nin, pin, wmin, nout, pout );

      xout.resize( nout );
      yout.resize( nout );
      vout.resize( nout );

      for ( int i = 0; i != nout; ++i )
      {
         xout[i] = pout[i].x;
         yout[i] = pout[i].y;
         vout[i] = pout[i].z;
      }

      free( pin );
      free( pout );
   }
   catch ( const Exception & ex ) { return reportError( ex.errorCode(), ex.what() ); }

   return NoError;
}

// Set project database. Reset all
void MapsManagerImpl::setProject( DataAccess::Interface::ProjectHandle * ph, const std::string & projectFileName )
{
   m_projectFileName = projectFileName;
   m_db   = ph->getDataBase();
   m_proj = ph;

   // clear arrays
   m_mapName.clear();
   m_mapRefTable.clear();
   m_mapObj.clear();

   // clear list of input maps files
   m_mapsFileList.clear();

   // collecting map names and map files name
   // get pointer to the table
   database::Table * table = m_db->getTable( s_mapsTableName );

   if ( !table ) return; // no table - no maps

   size_t recNum = table->size();

   m_mapObj.resize( recNum, NULL );

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
            if ( !fname.empty() )
            {
               // construct the full file path to the original map file
               ibs::FolderPath mapFilePath( projectPath );
               mapFilePath << fname;
               m_fileMaps[mapFilePath.path( )].push_back( mapName );
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
   database::Table * table = m_db->getTable( s_mapsTableName );
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

}

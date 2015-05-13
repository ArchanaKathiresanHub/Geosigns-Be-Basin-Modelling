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

// CMB API
#include "StratigraphyManagerImpl.h"
#include "UndefinedValues.h"

// Table IO lib
#include "database.h"

// Genex6_kernel lib
#include "SourceRock.h"

// STL
#include <stdexcept>
#include <string>

namespace mbapi
{

const char * StratigraphyManagerImpl::m_stratigraphyTableName           = "StratIoTbl";
const char * StratigraphyManagerImpl::m_layerNameFieldName              = "LayerName";
const char * StratigraphyManagerImpl::m_lithoType1FiledName             = "Lithotype1";
const char * StratigraphyManagerImpl::m_lithoType2FiledName             = "Lithotype2";
const char * StratigraphyManagerImpl::m_lithoType3FiledName             = "Lithotype3";
const char * StratigraphyManagerImpl::m_lithoTypePercent1FiledName      = "Percent1";
const char * StratigraphyManagerImpl::m_lithoTypePercent2FiledName      = "Percent2";
const char * StratigraphyManagerImpl::m_lithoTypePercent3FiledName      = "Percent3";
const char * StratigraphyManagerImpl::m_isSourceRockFieldName           = "SourceRock";
const char * StratigraphyManagerImpl::m_sourceRockType1FieldName        = "SourceRockType1";
const char * StratigraphyManagerImpl::m_sourceRockType2FieldName        = "SourceRockType2";
const char * StratigraphyManagerImpl::m_sourceRockHIFieldName           = "SourceRockMixingHI";
const char * StratigraphyManagerImpl::m_sourceRockEnableMixintFieldName = "EnableSourceRockMixing";

// Constructor
StratigraphyManagerImpl::StratigraphyManagerImpl()
{
   //throw std::runtime_error( "Not implemented yet" );
   m_db = NULL;
}

// Copy operator
StratigraphyManagerImpl & StratigraphyManagerImpl::operator = ( const StratigraphyManagerImpl & otherLithMgr )
{
   throw std::runtime_error( "Not implemented yet" );
   return *this;
}

// Set project database. Reset all
void StratigraphyManagerImpl::setDatabase( database::Database * db )
{
   m_db = db;
}

// Get list of layers in the model
// returns an array with IDs of layers defined in the model
std::vector<StratigraphyManager::LayerID> StratigraphyManagerImpl::layersIDs() const
{
   std::vector<LayerID> ids;
   if ( !m_db ) return ids;

   // get pointer to the table
   database::Table * table = m_db->getTable( m_stratigraphyTableName );

   // if table does not exist - return empty array
   if ( !table ) return ids;

   // fill IDs array with increasing indexes
   ids.resize( table->size() - 1, 0 ); // last one record - just bottom surface

   for ( size_t i = 0; i < ids.size(); ++i ) ids[i] = static_cast<LayerID>(i);

   return ids;
}

// Get list of surfaces in the model. Numbering from top to bottom
// returns array with IDs of surfaces defined in the model
std::vector<StratigraphyManager::SurfaceID> StratigraphyManagerImpl::surfacesIDs() const
{
   std::vector<SurfaceID> ids;
   if ( !m_db ) return ids;

   // get pointer to the table
   database::Table * table = m_db->getTable( m_stratigraphyTableName );

   // if table does not exist - return empty array
   if ( !table ) return ids;

   // fill IDs array with increasing indexes
   ids.resize( table->size(), 0 );

   for ( size_t i = 0; i < ids.size(); ++i ) ids[i] = static_cast<SurfaceID>(i);

   return ids;
}

// Create new layer
// returns ID of the new Stratigraphy
StratigraphyManager::LayerID StratigraphyManagerImpl::createNewLayer()
{
   throw std::runtime_error( "Not implemented yet" );
   return 0;
}

// Create new surface and returns ID of the new surface
StratigraphyManager::SurfaceID StratigraphyManagerImpl::createNewSurface()
{
   throw std::runtime_error( "Not implemented yet" );
   return 0;
}

// Get layer name for the given ID
std::string StratigraphyManagerImpl::layerName( StratigraphyManager::LayerID id )
{
   std::string layName;
   if ( errorCode() != NoError ) resetError();

   try
   {
      // get pointer to the table
      database::Table * table = m_db->getTable( m_stratigraphyTableName );

      // if table does not exist - report error
      if ( !table )
      {
         throw Exception( NonexistingID ) << m_stratigraphyTableName << " table could not be found in project";
      }

      database::Record * rec = table->getRecord( static_cast<int>(id) );
      if ( !rec )
      {
         throw Exception( NonexistingID ) << "No layer with ID: " << id << " in stratigraphy table";
      }
      layName = rec->getValue<std::string>( m_layerNameFieldName );
   }
   catch ( const Exception & e ) { reportError( e.errorCode(), e.what() ); }
   
   return layName;
}

// Get layer ID for the given name 
StratigraphyManagerImpl::LayerID StratigraphyManagerImpl::layerID( const std::string & ln )
{
   const std::vector<LayerID> & ids = layersIDs();
   for ( size_t i = 0; i < ids.size(); ++i )
   {
      if ( ln == layerName( ids[i] ) ) return ids[i];
   }
   return UndefinedIDValue;
}


// Get surface name for the given ID
std::string StratigraphyManagerImpl::surfaceName( StratigraphyManager::LayerID id )
{
   std::string sfName;
   if ( errorCode() != NoError ) resetError();

   try
   {
      // get pointer to the table
      database::Table * table = m_db->getTable( m_stratigraphyTableName );

      // if table does not exist - report error
      if ( !table )
      {
         throw Exception( NonexistingID ) << m_stratigraphyTableName << " table could not be found in project";
      }

      database::Record * rec = table->getRecord( static_cast<int>(id) );
      if ( !rec )
      {
         throw Exception( NonexistingID ) << "No layer with ID: " << id << " in stratigraphy table";
      }
      sfName = rec->getValue<std::string>( m_layerNameFieldName );
   }
   catch ( const Exception & e ) { reportError( e.errorCode(), e.what() ); }

   return sfName;
}

// Get all lithologies associated with the given layer and percentage of each lithology in a mix
ErrorHandler::ReturnCode StratigraphyManagerImpl::layerLithologiesList( LayerID id, std::vector<std::string> & lithoList, std::vector<double> & lithoPercent )
{
   if ( errorCode() != NoError ) resetError();

   lithoList.clear();
   lithoPercent.clear();

   try
   {
      // get pointer to the table
      database::Table * table = m_db->getTable( m_stratigraphyTableName );

      // if table does not exist - report error
      if ( !table )
      {
         throw Exception( NonexistingID ) << m_stratigraphyTableName << " table could not be found in project";
      }

      database::Record * rec = table->getRecord( static_cast<int>(id) );
      if ( !rec )
      {
         throw Exception( NonexistingID ) << "No layer with ID: " << id << " in stratigraphy table";
      }

      // get 1st lithology
      std::string lithoName = rec->getValue<std::string>( m_lithoType1FiledName        );
      double      perc      = rec->getValue<double>(      m_lithoTypePercent1FiledName );
      if ( !lithoName.empty() && perc > 0.0 )
      {
         lithoList.push_back( lithoName );
         lithoPercent.push_back( perc );
      }

      // get 2nd lithology
      lithoName = rec->getValue<std::string>( m_lithoType2FiledName        );
      perc      = rec->getValue<double>(      m_lithoTypePercent2FiledName );
      if ( !lithoName.empty() && perc > 0.0 )
      {
         lithoList.push_back( lithoName );
         lithoPercent.push_back( perc );
      }

      // get 3d lithology
      lithoName = rec->getValue<std::string>( m_lithoType3FiledName        );
      perc      = rec->getValue<double>(      m_lithoTypePercent3FiledName );
      if ( !lithoName.empty() && perc > 0.0 )
      {
         lithoList.push_back( lithoName );
         lithoPercent.push_back( perc );
      }
   }
   catch ( const Exception & e ) { return reportError( e.errorCode(), e.what() ); }

   return NoError;
}

// Set all lithologies associated with the given layer and percentage of each lithology in a mix
ErrorHandler::ReturnCode StratigraphyManagerImpl::setLayerLithologiesList( LayerID id, const std::vector<std::string> & lithoList, const std::vector<double> & lithoPercent )
{
   if ( errorCode() != NoError ) resetError();

   try
   {
      // get pointer to the table
      database::Table * table = m_db->getTable( m_stratigraphyTableName );

      // if table does not exist - report error
      if ( !table )
      {
         throw Exception( NonexistingID ) << m_stratigraphyTableName << " table could not be found in project";
      }

      database::Record * rec = table->getRecord( static_cast<int>(id) );
      if ( !rec )
      {
         throw Exception( NonexistingID ) << "No layer with ID: " << id << " in stratigraphy table";
      }

      assert( !lithoList.empty() && lithoList.size() == lithoPercent.size() );

      // set 1st lithology
      rec->setValue<std::string>( m_lithoType1FiledName,        lithoList[0]    );
      rec->setValue<double>(      m_lithoTypePercent1FiledName, lithoPercent[0] );

      // set 2nd lithology
      if ( lithoList.size() > 1 )
      {
         rec->setValue<std::string>( m_lithoType2FiledName,        lithoList[1] );
         rec->setValue<double>(      m_lithoTypePercent2FiledName, lithoPercent[1] );
      }
      else
      {
         rec->setValue<std::string>( m_lithoType2FiledName,        ""  );
         rec->setValue<double>(      m_lithoTypePercent2FiledName, 0.0 );
      }

      // set 3d lithology
      if ( lithoList.size() > 2 )
      {
         rec->setValue<std::string>( m_lithoType3FiledName,        lithoList[2]    );
         rec->setValue<double>(      m_lithoTypePercent3FiledName, lithoPercent[2] );
      }
      else
      {
         rec->setValue<std::string>( m_lithoType3FiledName,        ""  );
         rec->setValue<double>(      m_lithoTypePercent3FiledName, 0.0 );
      }
   }
   catch ( const Exception & e ) { return reportError( e.errorCode(), e.what() ); }

   return NoError;
}

// Collect layers where the given lithology is referenced
std::vector<StratigraphyManager::LayerID> StratigraphyManagerImpl::findLayersForLithology( const std::string & lithoName )
{
   const std::vector<LayerID> & ids = layersIDs();
   std::vector<LayerID> foundLayers;

   for ( size_t i = 0; i < ids.size(); ++ i )
   {
      std::vector<std::string> lithNames;
      std::vector<double>      lithPerc;
      
      if ( layerLithologiesList( ids[i], lithNames, lithPerc ) != NoError ) continue;
      
      bool found = false;
      for ( size_t j = 0; j < lithNames.size() && !found; ++j )
      {
         if ( lithNames[j] == lithoName ) // found another layer with the same lithology - needed to copy
         {
            found = true;
            foundLayers.push_back( ids[i] );
         }
      }
   }
   return foundLayers;
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



// Layer -> Source rock type relation methods

// Check if the given layer is a source rock layer
bool StratigraphyManagerImpl::isSourceRockActive( LayerID id )
{
   if ( errorCode() != NoError ) resetError();

   bool isLayerSR = false;

   try
   {
      // get pointer to the table
      database::Table * table = m_db->getTable( m_stratigraphyTableName );

      // if table does not exist - report error
      if ( !table )
      {
         throw Exception( NonexistingID ) << m_stratigraphyTableName << " table could not be found in project";
      }

      database::Record * rec = table->getRecord( static_cast<int>(id) );
      if ( !rec )
      {
         throw Exception( NonexistingID ) << "No layer with ID: " << id << " in stratigraphy table";
      }

      isLayerSR = rec->getValue<int>( m_isSourceRockFieldName ) == 1 ? true : false;
   }
   catch ( const Exception & e ) { reportError( e.errorCode(), e.what() ); }

   return isLayerSR;
}

// Check if for the given layer source rock mixing is enabled
bool StratigraphyManagerImpl::isSourceRockMixingEnabled( LayerID id )
{
   if ( errorCode() != NoError ) resetError();

   bool isLayerSRM = false;

   try
   {
      // get pointer to the table
      database::Table * table = m_db->getTable( m_stratigraphyTableName );

      // if table does not exist - report error
      if ( !table )
      {
         throw Exception( NonexistingID ) << m_stratigraphyTableName << " table could not be found in project";
      }

      database::Record * rec = table->getRecord( static_cast<int>(id) );
      if ( !rec )
      {
         throw Exception( NonexistingID ) << "No layer with ID: " << id << " in stratigraphy table";
      }

      isLayerSRM = rec->getValue<int>( m_sourceRockEnableMixintFieldName ) == 1 ? true : false;
   }
   catch ( const Exception & e ) { reportError( e.errorCode(), e.what() ); }

   return isLayerSRM;
}

// Get source rock types associated with given layer ID
std::vector<std::string> StratigraphyManagerImpl::sourceRockTypeName( LayerID lid )
{
   std::vector<std::string> srtNames;

   if ( errorCode() != NoError ) resetError();

   try
   {
      // get pointer to the table
      database::Table * table = m_db->getTable( m_stratigraphyTableName );

      // if table does not exist - report error
      if ( !table )
      {
         throw Exception( NonexistingID ) << m_stratigraphyTableName << " table could not be found in project";
      }

      database::Record * rec = table->getRecord( static_cast<int>(lid) );
      if ( !rec )
      {
         throw Exception( NonexistingID ) << "No layer with ID: " << lid << " in stratigraphy table";
      }

      int isSourceRock = rec->getValue<int>( m_isSourceRockFieldName );
      
      // check if layer is source rock layer
      if ( 1 == isSourceRock )
      {
         // get first source rock type name
         srtNames.push_back( rec->getValue<std::string>( m_sourceRockType1FieldName ) );

         // check if mixing is enabled
         int isMixingEnabled = rec->getValue<int>( m_sourceRockEnableMixintFieldName );
         if ( 1 == isMixingEnabled )
         {
            srtNames.push_back( rec->getValue<std::string>( m_sourceRockType2FieldName ) );
         }
      }
   }
   catch ( const Exception & e ) { reportError( e.errorCode(), e.what() ); }

   return srtNames;
}

// Get HI index for source rocks mix for the given layer
double StratigraphyManagerImpl::sourceRockMixHI( LayerID lid )
{
   double mixHI = 0.0;

   if ( errorCode() != NoError ) resetError();

   try
   {
      // get pointer to the table
      database::Table * table = m_db->getTable( m_stratigraphyTableName );

      // if table does not exist - report error
      if ( !table )
      {
         throw Exception( NonexistingID ) << m_stratigraphyTableName << " table could not be found in project";
      }

      database::Record * rec = table->getRecord( static_cast<int>(lid) );
      if ( !rec )
      {
         throw Exception( NonexistingID ) << "No layer with ID: " << lid << " in stratigraphy table";
      }

      int isSourceRock = rec->getValue<int>( m_isSourceRockFieldName );

      // check if layer is source rock layer
      if ( 1 == isSourceRock )
      {
         // check if mixing is enabled
         int isMixingEnabled = rec->getValue<int>( m_sourceRockEnableMixintFieldName );
         if ( 1 == isMixingEnabled )
         {
            mixHI = rec->getValue<double>( m_sourceRockHIFieldName );
         }
      }
   }
   catch ( const Exception & e ) { reportError( e.errorCode(), e.what() ); }

   return mixHI;
}

// Get H/C index for source rocks mix for the given layer
double StratigraphyManagerImpl::sourceRockMixHC( LayerID lid )
{
   double mixHC = 0;
   double hi = sourceRockMixHI( lid );
   if ( ErrorHandler::NoError == errorCode() )
   {
      return Genex6::SourceRock::convertHItoHC( hi );
   }
   return mixHC;
}
 
// Set source rock types name for the given layer and enable layer to be layer with source rock 
ErrorHandler::ReturnCode StratigraphyManagerImpl::setSourceRockTypeName( LayerID lid
                                                                       , const std::vector<std::string> & srTypeNames
                                                                       )
{
   if ( errorCode() != NoError ) resetError();
   try
   {
      // get pointer to the table
      database::Table * table = m_db->getTable( m_stratigraphyTableName );

      // if table does not exist - report error
      if ( !table )
      {
         throw Exception( NonexistingID ) << m_stratigraphyTableName << " table could not be found in project";
      }

      database::Record * rec = table->getRecord( static_cast<int>(lid) );
      if ( !rec )
      {
         throw Exception( NonexistingID ) << "No layer with ID: " << lid << " in stratigraphy table";
      }

      if ( srTypeNames.empty() )
      {  // disable source rock for given layer
         rec->setValue( m_isSourceRockFieldName, 0 );
      }
      else
      {  // enable/disable mixing
         rec->setValue( m_sourceRockEnableMixintFieldName, (srTypeNames.size() == 1 ? 0 : 1) );

         rec->setValue( m_sourceRockType1FieldName, srTypeNames[0] );
         if ( srTypeNames.size() > 1 )
         {
            rec->setValue( m_sourceRockType2FieldName, srTypeNames[1] );
         }
      }
   }
   catch ( const Exception & e ) { return reportError( e.errorCode(), e.what() ); }

   return NoError;
}

// Set HI value for source rock mix for the given layer
ErrorHandler::ReturnCode StratigraphyManagerImpl::setSourceRockMixHI( LayerID lid, double srmHI )
{
   if ( errorCode() != NoError ) resetError();
   try
   {
      // get pointer to the table
      database::Table * table = m_db->getTable( m_stratigraphyTableName );

      // if table does not exist - report error
      if ( !table )
      {
         throw Exception( NonexistingID ) << m_stratigraphyTableName << " table could not be found in project";
      }

      database::Record * rec = table->getRecord( static_cast<int>(lid) );
      if ( !rec )
      {
         throw Exception( NonexistingID ) << "No layer with ID: " << lid << " in stratigraphy table";
      }

      int isSourceRock = rec->getValue<int>( m_isSourceRockFieldName );

      // check if layer is source rock layer
      if ( 1 != isSourceRock )
      {
         throw Exception( OutOfRangeValue ) << "Layer with id: " << lid <<
            " is not a source rock layer. Can't set HI for source rock mixing";
      }

      // check if mixing is enabled
      int isMixingEnabled = rec->getValue<int>( m_sourceRockEnableMixintFieldName );
      if ( 1 == isMixingEnabled )
      {
         throw Exception( OutOfRangeValue ) << "Layer with id: " << lid <<
            " has source rock mixing disabled. Can't set HI for source rock mixing";
      }
      // all checks are OK, set HI for mixing
      rec->setValue( m_sourceRockHIFieldName, srmHI );
   }
   catch ( const Exception & e ) { return reportError( e.errorCode(), e.what() ); }

   return NoError;
}

// Set HI value for source rock mix for the given layer
ErrorHandler::ReturnCode StratigraphyManagerImpl::setSourceRockMixHC( LayerID lid, double srmHC )
{
   double hi = Genex6::SourceRock::convertHCtoHI( srmHC );
   return setSourceRockMixHI( lid, srmHC );
}


}


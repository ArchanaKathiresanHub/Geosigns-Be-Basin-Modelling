//                                                                      
// Copyright (C) 2012-2016 Shell International Exploration & Production.
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

const char * StratigraphyManagerImpl::s_stratigraphyTableName           = "StratIoTbl";
const char * StratigraphyManagerImpl::s_layerNameFieldName              = "LayerName";
const char * StratigraphyManagerImpl::s_depoAgeFieldName                = "DepoAge";
const char * StratigraphyManagerImpl::s_lithoType1FiledName             = "Lithotype1";
const char * StratigraphyManagerImpl::s_lithoType2FiledName             = "Lithotype2";
const char * StratigraphyManagerImpl::s_lithoType3FiledName             = "Lithotype3";
const char * StratigraphyManagerImpl::s_lithoTypePercent1FiledName      = "Percent1";
const char * StratigraphyManagerImpl::s_lithoTypePercent2FiledName      = "Percent2";
const char * StratigraphyManagerImpl::s_lithoTypePercent3FiledName      = "Percent3";
const char * StratigraphyManagerImpl::s_lithoTypePercent1GridFiledName  = "Percent1Grid";
const char * StratigraphyManagerImpl::s_lithoTypePercent2GridFiledName =  "Percent2Grid";
const char * StratigraphyManagerImpl::s_isSourceRockFieldName           = "SourceRock";
const char * StratigraphyManagerImpl::s_sourceRockType1FieldName        = "SourceRockType1";
const char * StratigraphyManagerImpl::s_sourceRockType2FieldName        = "SourceRockType2";
const char * StratigraphyManagerImpl::s_sourceRockHIFieldName           = "SourceRockMixingHI";
const char * StratigraphyManagerImpl::s_sourceRockHIMapFieldName        = "SourceRockMixingHIGrid";
const char * StratigraphyManagerImpl::s_sourceRockEnableMixingFieldName = "EnableSourceRockMixing";
const char * StratigraphyManagerImpl::s_isAllochtonLithology            = "HasAllochthonLitho";

const char * StratigraphyManagerImpl::s_pressureFaultCutTableName       = "PressureFaultcutIoTbl";
const char * StratigraphyManagerImpl::s_FaultcutsMapFieldName           = "FaultcutsMap";
const char * StratigraphyManagerImpl::s_FaultNameFieldName              = "FaultName";
const char * StratigraphyManagerImpl::s_FaultLithologyFieldName         = "FaultLithology";
 
// Constructor
StratigraphyManagerImpl::StratigraphyManagerImpl()
{
   //throw std::runtime_error( "Not implemented yet" );
   m_db = NULL;
}

// Copy operator
StratigraphyManagerImpl & StratigraphyManagerImpl::operator = ( const StratigraphyManagerImpl & /*otherLithMgr*/ )
{
   throw std::runtime_error( "Not implemented yet" );
   return *this;
}

// Set project database. Reset all
void StratigraphyManagerImpl::setDatabase( database::Database * db )
{
   m_db = db;
   m_stratIoTbl = m_db->getTable( s_stratigraphyTableName );
}

// Get list of layers in the model
// returns an array with IDs of layers defined in the model
std::vector<StratigraphyManager::LayerID> StratigraphyManagerImpl::layersIDs() const
{
   std::vector<LayerID> ids;
   if ( !m_db ) return ids;

   // if table does not exist - return empty array
   if ( !m_stratIoTbl ) return ids;

   // fill IDs array with increasing indexes
   ids.resize( m_stratIoTbl->size() - 1, 0 ); // last one record - just bottom surface

   for ( size_t i = 0; i < ids.size(); ++i ) ids[i] = static_cast<LayerID>(i);

   return ids;
}

// Get list of surfaces in the model. Numbering from top to bottom
// returns array with IDs of surfaces defined in the model
std::vector<StratigraphyManager::SurfaceID> StratigraphyManagerImpl::surfacesIDs() const
{
   std::vector<SurfaceID> ids;
   if ( !m_db ) return ids;

   // if table does not exist - return empty array
   if ( !m_stratIoTbl ) return ids;

   // fill IDs array with increasing indexes
   ids.resize( m_stratIoTbl->size(), 0 );

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
      // if table does not exist - report error
      if ( !m_stratIoTbl )
      {
         throw Exception( NonexistingID ) << s_stratigraphyTableName << " table could not be found in project";
      }

      database::Record * rec = m_stratIoTbl->getRecord( static_cast<int>(id) );
      if ( !rec )
      {
         throw Exception( NonexistingID ) << "No layer with ID: " << id << " in stratigraphy table";
      }
      layName = rec->getValue<std::string>( s_layerNameFieldName );
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
      // if table does not exist - report error
      if ( !m_stratIoTbl )
      {
         throw Exception( NonexistingID ) << s_stratigraphyTableName << " table could not be found in project";
      }

      database::Record * rec = m_stratIoTbl->getRecord( static_cast<int>(id) );
      if ( !rec )
      {
         throw Exception( NonexistingID ) << "No layer with ID: " << id << " in stratigraphy table";
      }
      sfName = rec->getValue<std::string>( s_layerNameFieldName );
   }
   catch ( const Exception & e ) { reportError( e.errorCode(), e.what() ); }

   return sfName;
}

double StratigraphyManagerImpl::eldestLayerAge()
{
   if ( errorCode() != NoError ) resetError();

   double layerAge = UndefinedDoubleValue;
   try
   {
      // if table does not exist - report error
      if ( !m_stratIoTbl )
      {
         throw Exception( NonexistingID ) << s_stratigraphyTableName << " table could not be found in project";
      }

      size_t tblSize = m_stratIoTbl->size();

      for ( size_t i = 0; i < tblSize; ++i )
      {
         database::Record * rec = m_stratIoTbl->getRecord( static_cast<int>( i ) );
         if ( !rec ) { throw Exception( NonexistingID ) << "No layer with ID: " << i << " in stratigraphy table"; }

         double age = rec->getValue<double>( s_depoAgeFieldName );
         layerAge = i == 0 ? age : ( layerAge < age ? age : layerAge );
      }
   }
   catch ( const Exception & e ) { reportError( e.errorCode(), e.what() ); }

   return layerAge;
}

// Get all lithologies associated with the given layer and percentage of each lithology in a mix
ErrorHandler::ReturnCode StratigraphyManagerImpl::layerLithologiesList( LayerID id, std::vector<std::string> & lithoList, std::vector<double> & lithoPercent )
{
   if ( errorCode() != NoError ) resetError();

   lithoList.clear();
   lithoPercent.clear();

   try
   {
      // if table does not exist - report error
      if ( !m_stratIoTbl )
      {
         throw Exception( NonexistingID ) << s_stratigraphyTableName << " table could not be found in project";
      }

      database::Record * rec = m_stratIoTbl->getRecord( static_cast<int>(id) );
      if ( !rec )
      {
         throw Exception( NonexistingID ) << "No layer with ID: " << id << " in stratigraphy table";
      }

      // get 1st lithology
      std::string lithoName = rec->getValue<std::string>( s_lithoType1FiledName        );
      double      perc      = rec->getValue<double>(      s_lithoTypePercent1FiledName );
      lithoList.push_back( lithoName );
      lithoPercent.push_back( perc );

      // get 2nd lithology
      lithoName = rec->getValue<std::string>( s_lithoType2FiledName        );
      perc      = rec->getValue<double>(      s_lithoTypePercent2FiledName );
      lithoList.push_back( lithoName );
      lithoPercent.push_back( perc );

      // get 3d lithology
      lithoName = rec->getValue<std::string>( s_lithoType3FiledName        );
      perc      = rec->getValue<double>(      s_lithoTypePercent3FiledName );
      lithoList.push_back( lithoName );
      lithoPercent.push_back( perc );
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
      // if table does not exist - report error
      if ( !m_stratIoTbl )
      {
         throw Exception( NonexistingID ) << s_stratigraphyTableName << " table could not be found in project";
      }

      database::Record * rec = m_stratIoTbl->getRecord( static_cast<int>(id) );
      if ( !rec )
      {
         throw Exception( NonexistingID ) << "No layer with ID: " << id << " in stratigraphy table";
      }

      assert( !lithoList.empty() && lithoList.size() == lithoPercent.size() );

      // set 1st lithology
      rec->setValue<std::string>( s_lithoType1FiledName,        lithoList[0]    );
      rec->setValue<double>(      s_lithoTypePercent1FiledName, lithoPercent[0] );

      // set 2nd lithology
      if ( lithoList.size() > 1 )
      {
         rec->setValue<std::string>( s_lithoType2FiledName,        lithoList[1] );
         rec->setValue<double>(      s_lithoTypePercent2FiledName, lithoPercent[1] );
      }
      else
      {
         rec->setValue<std::string>( s_lithoType2FiledName,        ""  );
         rec->setValue<double>(      s_lithoTypePercent2FiledName, 0.0 );
      }

      // set 3d lithology
      if ( lithoList.size() > 2 )
      {
         rec->setValue<std::string>( s_lithoType3FiledName,        lithoList[2]    );
         rec->setValue<double>(      s_lithoTypePercent3FiledName, lithoPercent[2] );
      }
      else
      {
         rec->setValue<std::string>( s_lithoType3FiledName,        ""  );
         rec->setValue<double>(      s_lithoTypePercent3FiledName, 0.0 );
      }
   }
   catch ( const Exception & e ) { return reportError( e.errorCode(), e.what() ); }

   return NoError;
}

// Set the lithology percentages maps
ErrorHandler::ReturnCode StratigraphyManagerImpl::setLayerLithologiesPercentageMaps( LayerID id, const std::string & mapNameFirstLithoPercentage, const std::string mapNameSecondLithoPercentage )
{
   if ( errorCode() != NoError ) resetError();

   try
   {
      // if table does not exist - report error
      if ( !m_stratIoTbl )
      {
         throw Exception( NonexistingID ) << s_stratigraphyTableName << " table could not be found in project";
      }

      database::Record * rec = m_stratIoTbl->getRecord( static_cast<int>( id ) );
      if ( !rec )
      {
         throw Exception( NonexistingID ) << "No layer with ID: " << id << " in stratigraphy table";
      }

      // set the percentage grid maps. all three maps are always defined
      rec->setValue<std::string>( s_lithoTypePercent1GridFiledName, mapNameFirstLithoPercentage );
      rec->setValue<std::string>( s_lithoTypePercent2GridFiledName, mapNameSecondLithoPercentage );
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
ErrorHandler::ReturnCode StratigraphyManagerImpl::setLayerSurfaces( StratigraphyManager::LayerID   /*lid*/
                                                                  , StratigraphyManager::SurfaceID /*usid*/
                                                                  , StratigraphyManager::SurfaceID /*dsid*/
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

   bool flag = false;

   try
   {
      // if table does not exist - report error
      if ( !m_stratIoTbl ) { throw Exception( NonexistingID ) << s_stratigraphyTableName << " table could not be found in project"; }

      database::Record * rec = m_stratIoTbl->getRecord( static_cast<int>(id) );
      if ( !rec ) { throw Exception( NonexistingID ) << "No layer with ID: " << id << " in stratigraphy table"; }

      flag = rec->getValue<int>( s_isSourceRockFieldName ) == 1 ? true : false;
   }
   catch ( const Exception & e ) { reportError( e.errorCode(), e.what() ); }

   return flag;
}

// Check if for the given layer source rock mixing is enabled
bool StratigraphyManagerImpl::isSourceRockMixingEnabled( LayerID id )
{
   if ( errorCode() != NoError ) resetError();

   bool flag = false;

   try
   {
      // if table does not exist - report error
      if ( !m_stratIoTbl ) { throw Exception( NonexistingID ) << s_stratigraphyTableName << " table could not be found in project"; }

      database::Record * rec = m_stratIoTbl->getRecord( static_cast<int>(id) );
      if ( !rec ) { throw Exception( NonexistingID ) << "No layer with ID: " << id << " in stratigraphy table"; }

      flag = rec->getValue<int>( s_sourceRockEnableMixingFieldName ) == 1 ? true : false;
   }
   catch ( const Exception & e ) { reportError( e.errorCode(), e.what() ); }

   return flag;
}

// Enable or disable source rock mixing for the giving layer
ErrorHandler::ReturnCode StratigraphyManagerImpl::setSourceRockMixingEnabled( LayerID id, bool val )
{
   if ( errorCode() != NoError ) resetError();

   try
   {
      // if table does not exist - report error
      if ( !m_stratIoTbl ) { throw Exception( NonexistingID ) << s_stratigraphyTableName << " table could not be found in project"; }

      database::Record * rec = m_stratIoTbl->getRecord( static_cast<int>(id) );
      if ( !rec ) { throw Exception( NonexistingID ) << "No layer with ID: " << id << " in stratigraphy table"; }

      rec->setValue<int>( s_sourceRockEnableMixingFieldName, (val ? 1 : 0) );
   }
   catch ( const Exception & e ) { return reportError( e.errorCode(), e.what() ); }

   return NoError;
}



// Check if layer has active allochton lithology
bool StratigraphyManagerImpl::isAllochtonLithology( LayerID id )
{
   if ( errorCode() != NoError ) resetError();

   bool flag = false;
   try
   {
      // if table does not exist - report error
      if ( !m_stratIoTbl ) { throw Exception( NonexistingID ) << s_stratigraphyTableName << " table could not be found in project"; }

      database::Record * rec = m_stratIoTbl->getRecord( static_cast<int>(id) );
      if ( !rec ) { throw Exception( NonexistingID ) << "No layer with ID: " << id << " in stratigraphy table"; }

      flag = rec->getValue<int>( s_isAllochtonLithology ) == 1 ? true : false;
   }
   catch ( const Exception & e ) { reportError( e.errorCode(), e.what() ); }

   return flag;
}


// Get source rock types associated with given layer ID
std::vector<std::string> StratigraphyManagerImpl::sourceRockTypeName( LayerID lid )
{
   std::vector<std::string> srtNames;

   if ( errorCode() != NoError ) resetError();

   try
   {
      // if table does not exist - report error
      if ( !m_stratIoTbl )
      {
         throw Exception( NonexistingID ) << s_stratigraphyTableName << " table could not be found in project";
      }

      database::Record * rec = m_stratIoTbl->getRecord( static_cast<int>(lid) );
      if ( !rec )
      {
         throw Exception( NonexistingID ) << "No layer with ID: " << lid << " in stratigraphy table";
      }

      int isSourceRock = rec->getValue<int>( s_isSourceRockFieldName );
      
      // check if layer is source rock layer
      if ( 1 == isSourceRock )
      {
         // get first source rock type name
         srtNames.push_back( rec->getValue<std::string>( s_sourceRockType1FieldName ) );

         // check if mixing is enabled
         int isMixingEnabled = rec->getValue<int>( s_sourceRockEnableMixingFieldName );
         if ( 1 == isMixingEnabled )
         {
            srtNames.push_back( rec->getValue<std::string>( s_sourceRockType2FieldName ) );
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
      // if table does not exist - report error
      if ( !m_stratIoTbl )
      {
         throw Exception( NonexistingID ) << s_stratigraphyTableName << " table could not be found in project";
      }

      database::Record * rec = m_stratIoTbl->getRecord( static_cast<int>(lid) );
      if ( !rec )
      {
         throw Exception( NonexistingID ) << "No layer with ID: " << lid << " in stratigraphy table";
      }

      int isSourceRock = rec->getValue<int>( s_isSourceRockFieldName );

      // check if layer is source rock layer
      if ( 1 == isSourceRock )
      {
         // check if mixing is enabled
         int isMixingEnabled = rec->getValue<int>( s_sourceRockEnableMixingFieldName );
         if ( 1 == isMixingEnabled )
         {
            mixHI = rec->getValue<double>( s_sourceRockHIFieldName );
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
      // if table does not exist - report error
      if ( !m_stratIoTbl )
      {
         throw Exception( NonexistingID ) << s_stratigraphyTableName << " table could not be found in project";
      }

      database::Record * rec = m_stratIoTbl->getRecord( static_cast<int>(lid) );
      if ( !rec )
      {
         throw Exception( NonexistingID ) << "No layer with ID: " << lid << " in stratigraphy table";
      }

      if ( srTypeNames.empty() )
      {  // disable source rock for given layer
         rec->setValue( s_isSourceRockFieldName, 0 );
      }
      else
      {  // enable/disable mixing
         int srNum = 0;
         for ( size_t i = 0; i < srTypeNames.size(); ++i ) { srNum += srTypeNames[i].empty() ? 0 : 1; }
         rec->setValue<int>( s_sourceRockEnableMixingFieldName, (srNum == 1 ? 0 : 1) );

         rec->setValue( s_sourceRockType1FieldName, srTypeNames[0] );
         if ( srTypeNames.size() > 1 )
         {
            rec->setValue( s_sourceRockType2FieldName, srTypeNames[1] );
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
      // if table does not exist - report error
      if ( !m_stratIoTbl )
      {
         throw Exception( NonexistingID ) << s_stratigraphyTableName << " table could not be found in project";
      }

      database::Record * rec = m_stratIoTbl->getRecord( static_cast<int>(lid) );
      if ( !rec )
      {
         throw Exception( NonexistingID ) << "No layer with ID: " << lid << " in stratigraphy table";
      }

      int isSourceRock = rec->getValue<int>( s_isSourceRockFieldName );

      // check if layer is source rock layer
      if ( 1 != isSourceRock )
      {
         throw Exception( OutOfRangeValue ) << "Layer with id: " << lid <<
            " is not a source rock layer. Can't set HI for source rock mixing";
      }

      // check if mixing is enabled
      int isMixingEnabled = rec->getValue<int>( s_sourceRockEnableMixingFieldName );
      if ( 0 == isMixingEnabled && srmHI != 0.0 )
      {
         throw Exception( OutOfRangeValue ) << "Layer with id: " << lid <<
            " has source rock mixing disabled. Can't set HI for source rock mixing";
      }
      // all checks are OK, set HI for mixing
      rec->setValue( s_sourceRockHIFieldName, srmHI );
   }
   catch ( const Exception & e ) { return reportError( e.errorCode(), e.what() ); }

   return NoError;
}

// Set HI map name for the source rock mix for the given layer
ErrorHandler::ReturnCode StratigraphyManagerImpl::setSourceRockMixHIMapName( LayerID lid, const std::string & srmHImap )
{
   if ( errorCode() != NoError ) resetError();
   try
   {
      // if table does not exist - report error
      if ( !m_stratIoTbl ) { throw Exception( NonexistingID ) << s_stratigraphyTableName << " table could not be found in project"; }

      database::Record * rec = m_stratIoTbl->getRecord( static_cast<int>(lid) );
      if ( !rec ) { throw Exception( NonexistingID ) << "No layer with ID: " << lid << " in stratigraphy table"; }

      int isSourceRock = rec->getValue<int>( s_isSourceRockFieldName );

      // check if layer is source rock layer
      if ( 1 != isSourceRock )
      {
         throw Exception( OutOfRangeValue ) << "Layer with id: " << lid << " is not a source rock layer. Can't set HI for source rock mixing";
      }

      // check if mixing is enabled
      int isMixingEnabled = rec->getValue<int>( s_sourceRockEnableMixingFieldName );
      if ( 0 == isMixingEnabled && !srmHImap.empty() )
      {
         throw Exception( OutOfRangeValue ) << "Layer with id: " << lid <<
            " has source rock mixing disabled. Can't set HI for source rock mixing";
      }
      // all checks are OK, set HI for mixing
      rec->setValue( s_sourceRockHIMapFieldName, srmHImap );
   }
   catch ( const Exception & e ) { return reportError( e.errorCode(), e.what() ); }

   return NoError;
}

// Set HI value for source rock mix for the given layer
ErrorHandler::ReturnCode StratigraphyManagerImpl::setSourceRockMixHC( LayerID lid, double srmHC )
{
   double hi = Genex6::SourceRock::convertHCtoHI( srmHC );
   return setSourceRockMixHI( lid, hi );
}

// Get list of fault cuts from PressureFaultcutIoTbl
// return array with IDs of layers defined in the model
std::vector<StratigraphyManager::PrFaultCutID> StratigraphyManagerImpl::faultCutsIDs()
{
   std::vector<PrFaultCutID> ids;
   if ( !m_db ) return ids;

   // get pointer to the table
   database::Table * table = m_db->getTable( s_pressureFaultCutTableName );

   // if table does not exist - return empty array
   if ( !table ) return ids;

   // fill IDs array with increasing indexes
   ids.resize( table->size(), 0 );

   for ( size_t i = 0; i < ids.size(); ++i ) ids[i] = static_cast<PrFaultCutID>(i);

   return ids;
}

// Search in PressureFaultcutIoTbl table for the given combination of map name/fault name
StratigraphyManager::PrFaultCutID StratigraphyManagerImpl::findFaultCut( const std::string & mapName, const std::string & fltName )
{
   if ( errorCode() != NoError ) resetError();
   try
   {
      // get pointer to the table
      database::Table * table = m_db->getTable( s_pressureFaultCutTableName );

      // if table does not exist - report error
      if ( !table ) { throw Exception( NonexistingID ) << s_pressureFaultCutTableName << " table could not be found in project"; }

      size_t tblSize = table->size();
      for ( size_t i = 0; i < tblSize; ++i )
      {
         database::Record * rec = table->getRecord( static_cast<unsigned int>( i ) );
         if ( !rec ) { throw Exception( NonexistingID ) << "No fault cat type with such ID: " << i; }

         if ( mapName == rec->getValue<std::string>( s_FaultcutsMapFieldName ) &&
              fltName == rec->getValue<std::string>( s_FaultNameFieldName )
            )
         {
            return static_cast<PrFaultCutID>( i );
         }
      }
   }
   catch ( const Exception & e ) { reportError( e.errorCode(), e.what() ); }

   return UndefinedIDValue;
}

// Get lithlogy name for the given fault cut ID
std::string StratigraphyManagerImpl::faultCutLithology( PrFaultCutID flID )
{
   if ( errorCode() != NoError ) resetError();

   try
   {
      // get pointer to the table
      database::Table * table = m_db->getTable( s_pressureFaultCutTableName );

      // if table does not exist - report error
      if ( !table ) { throw Exception( NonexistingID ) << s_pressureFaultCutTableName << " table could not be found in project"; }

      database::Record * rec = table->getRecord( static_cast<int>( flID ) );
      if ( !rec ) { throw Exception( NonexistingID ) << "No fault cut lithology type with such ID: " << flID; }

      return rec->getValue<std::string>( s_FaultLithologyFieldName );
   }
   catch ( const Exception & e ) { reportError( e.errorCode(), e.what() ); }

   return "";
}

// Get fault cut name for the given fault cut ID
std::string StratigraphyManagerImpl::faultCutName( PrFaultCutID flID )
{
   if ( errorCode() != NoError ) resetError();

   try
   {
      // get pointer to the table
      database::Table * table = m_db->getTable( s_pressureFaultCutTableName );

      // if table does not exist - report error
      if ( !table ) { throw Exception( NonexistingID ) << s_pressureFaultCutTableName << " table could not be found in project"; }

      database::Record * rec = table->getRecord( static_cast<int>( flID ) );
      if ( !rec ) { throw Exception( NonexistingID ) << "No fault cut lithology type with such ID: " << flID; }

      return rec->getValue<std::string>( s_FaultNameFieldName );
   }
   catch ( const Exception & e ) { reportError( e.errorCode(), e.what() ); }

   return "";
}

// Get fault cat map for the given fault cut ID
std::string StratigraphyManagerImpl::faultCutMapName( PrFaultCutID flID )
{
   if ( errorCode() != NoError ) resetError();

   try
   {
      // get pointer to the table
      database::Table * table = m_db->getTable( s_pressureFaultCutTableName );

      // if table does not exist - report error
      if ( !table ) { throw Exception( NonexistingID ) << s_pressureFaultCutTableName << " table could not be found in project"; }

      database::Record * rec = table->getRecord( static_cast<int>( flID ) );
      if ( !rec ) { throw Exception( NonexistingID ) << "No fault cut lithology type with such ID: " << flID; }

      return rec->getValue<std::string>( s_FaultcutsMapFieldName );
   }
   catch ( const Exception & e ) { reportError( e.errorCode(), e.what() ); }

   return "";
}

// Set new lithology for the fault cut
ErrorHandler::ReturnCode StratigraphyManagerImpl::setFaultCutLithology( PrFaultCutID flID, const std::string & newLithoName )
{
   if ( errorCode() != NoError ) resetError();

   try
   {
      // get pointer to the table
      database::Table * table = m_db->getTable( s_pressureFaultCutTableName );

      // if table does not exist - report error
      if ( !table ) { throw Exception( NonexistingID ) << s_pressureFaultCutTableName << " table could not be found in project"; }

      database::Record * rec = table->getRecord( static_cast<int>( flID ) );
      if ( !rec ) { throw Exception( NonexistingID ) << "No fault cut lithology type with such ID: " << flID; }

      rec->setValue( s_FaultLithologyFieldName, newLithoName );
   }
   catch ( const Exception & e ) { return reportError( e.errorCode(), e.what() ); }

   return NoError;
}
    
}


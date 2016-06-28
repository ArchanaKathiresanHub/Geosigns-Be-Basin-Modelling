//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// to do: add the case when the refence is the sealevel and the first depth is calculated with seismic velocity of seawater
#define TWTCONVFACT 0.001 //milliseconds to seconds
#define FULLRES 

#include <assert.h>
#include <stdlib.h>
#include "FastcauldronSimulator.h"
#include "FastcauldronStartup.h"

//DataAccess library
#include "Interface/Interface.h"
#include "Interface/ProjectHandle.h"
#include "Interface/Formation.h"
#include "Interface/GridMap.h"
#include "Interface/Grid.h"
#include "MemoryChecker.h"

//utilities library
#include "LogHandler.h"
#include "FormattingException.h"

//cbmAPI and utilities
#include "cmbAPI.h"
#include "FilePath.h"
#include "FolderPath.h"

// fill the values of the grid map in one array 
static void fillArray( const DataAccess::Interface::GridMap *  grid, std::vector<double>& v, int k, double convFact )
{
   int i, j;
   grid->retrieveData();
   int numI = grid->lastI() + 1;
   for ( unsigned int j = grid->firstJ(); j <= grid->lastJ(); ++j )
   {
      for ( unsigned int i = grid->firstI(); i <= grid->lastI(); ++i )
      {
         v[i + j * numI] = grid->getValue( i, j, (unsigned int)k ) * convFact;
      }
   }
   grid->restoreData();
}

static ErrorHandler::ReturnCode getGridMapDepthValues( mbapi::Model * mdl, const mbapi::StratigraphyManager::LayerID l, std::vector<double> & v )
{

   mbapi::MapsManager & mapsMgr = mdl->mapsManager( );

   // Get the map name and id
   std::string depthMap = mdl->tableValueAsString( "StratIoTbl", l, "DepthGrid" );
   if ( depthMap == UndefinedStringValue )
   {
      return ErrorHandler::UnknownError;
   }
   mbapi::MapsManager::MapID depthMapID = mapsMgr.findID( depthMap );
   if ( depthMapID == UndefinedIDValue )
   {
      return ErrorHandler::UnknownError;
   }

   // Get the values
   if ( ErrorHandler::ReturnCode::NoError != mapsMgr.mapGetValues( depthMapID, v ) )
   {
      return ErrorHandler::UnknownError;
   }

   return ErrorHandler::NoError;
}



static void abortOnBadAlloc( ) {
   cerr << " cannot allocate resources, aborting" << endl;
   MPI_Abort( PETSC_COMM_WORLD, 3 );
}

static ErrorHandler::ReturnCode  prepareProject( mbapi::Model * mdl, const mbapi::Model * refMdl, const mbapi::StratigraphyManager::LayerID startLayer )
{

   ErrorHandler::ReturnCode error( ErrorHandler::ReturnCode::NoError );
   // set full resolution
   if ( mdl->tableValueAsInteger( "ProjectIoTbl", 0, "ScaleX" ) > 1 || mdl->tableValueAsInteger( "ProjectIoTbl", 0, "ScaleY" ) > 1 )
   {
      LogHandler( LogHandler::WARNING_SEVERITY ) << "The project is subsampled. For the automatic calibration of the depth maps the full resolution is required. Changing the project file to run in full resolution";
#ifdef FULLRES
      if ( error == ErrorHandler::ReturnCode::NoError ) error = mdl->setTableValue( "ProjectIoTbl", 0, "ScaleX", (long)1 );
      if ( error == ErrorHandler::ReturnCode::NoError ) error = mdl->setTableValue( "ProjectIoTbl", 0, "ScaleY", (long)1 );
#endif
   }

   // set full window
   int NumberX = mdl->tableValueAsInteger( "ProjectIoTbl", 0, "NumberX" );
   int NumberY = mdl->tableValueAsInteger( "ProjectIoTbl", 0, "NumberY" );
   if ( mdl->tableValueAsInteger( "ProjectIoTbl", 0, "WindowXMin" ) > 0 || mdl->tableValueAsInteger( "ProjectIoTbl", 0, "WindowYMin" ) > 0 || mdl->tableValueAsInteger( "ProjectIoTbl", 0, "WindowXMax" ) < NumberX - 1 || mdl->tableValueAsInteger( "ProjectIoTbl", 0, "WindowYMax" ) < NumberY - 1 )
   {
      LogHandler( LogHandler::WARNING_SEVERITY ) << "The project is windowed. For the automatic calibration of the depth maps the entire domain is required. Changing the project file to run caldron on the entire domanin";
      if ( error == ErrorHandler::ReturnCode::NoError ) error = mdl->setTableValue( "ProjectIoTbl", 0, "WindowXMin", (long)1 );
      if ( error == ErrorHandler::ReturnCode::NoError ) error = mdl->setTableValue( "ProjectIoTbl", 0, "WindowYMin", (long)1 );
      if ( error == ErrorHandler::ReturnCode::NoError ) error = mdl->setTableValue( "ProjectIoTbl", 0, "WindowXMax", (long)NumberX );
      if ( error == ErrorHandler::ReturnCode::NoError ) error = mdl->setTableValue( "ProjectIoTbl", 0, "WindowYMax", (long)NumberY );
   }

   // clear uncessary tables
   if ( error == ErrorHandler::ReturnCode::NoError ) error = mdl->clearTable( "TimeIoTbl" );
   if ( error == ErrorHandler::ReturnCode::NoError ) error = mdl->clearTable( "3DTimeIoTbl" );
   if ( error == ErrorHandler::ReturnCode::NoError ) error = mdl->clearTable( "1DTimeIoTbl" );
   if ( error == ErrorHandler::ReturnCode::NoError ) error = mdl->clearTable( "ReservoirIoTbl" );
   if ( error == ErrorHandler::ReturnCode::NoError ) error = mdl->clearTable( "MobLayThicknIoTbl" );
   if ( error == ErrorHandler::ReturnCode::NoError ) error = mdl->clearTable( "FilterDepthIoTbl" );
   if ( error == ErrorHandler::ReturnCode::NoError ) error = mdl->clearTable( "FilterTimeIoTbl" );
   if ( error == ErrorHandler::ReturnCode::NoError ) error = mdl->clearTable( "FilterTimeDepthIoTbl" );
   if ( error == ErrorHandler::ReturnCode::NoError ) error = mdl->clearTable( "SnapshotIoTbl" );

   mdl->clearTable( "StratIoTbl" );
   std::vector<std::string> propertiesToSave;
   propertiesToSave.push_back( "Depth" );
   propertiesToSave.push_back( "TwoWayTime" );
   propertiesToSave.push_back( "TwoWayTimeResidual" );

   for ( size_t i = 0; i != propertiesToSave.size(); ++i )
   {
      if ( error == ErrorHandler::ReturnCode::NoError ) error = mdl->addRowToTable( "FilterTimeIoTbl" );
      if ( error == ErrorHandler::ReturnCode::NoError ) error = mdl->setTableValue( "FilterTimeIoTbl", i, "PropertyName", propertiesToSave[i] );
      if ( error == ErrorHandler::ReturnCode::NoError ) error = mdl->setTableValue( "FilterTimeIoTbl", i, "ModellingMode", "3d" );
      if ( error == ErrorHandler::ReturnCode::NoError ) error = mdl->setTableValue( "FilterTimeIoTbl", i, "OutputOption", "SedimentsOnly" );
      if ( error == ErrorHandler::ReturnCode::NoError ) error = mdl->setTableValue( "FilterTimeIoTbl", i, "ResultOption", "Simple" );
   }

   // always copy the layers above the reference and the reference layer
   for ( mbapi::StratigraphyManager::LayerID l = 0; l <= startLayer; ++l )
   {
      if ( error == ErrorHandler::ReturnCode::NoError ) error = mdl->copyRecordFromModel( "StratIoTbl", *refMdl, l );
      if ( error == ErrorHandler::ReturnCode::NoError ) error = mdl->setTableValue( "StratIoTbl", l, "MobileLayer", (long)0 );
   }

   return error;
}

static int runFastCauldron( int argc, char** argv, int & rank, const std::string & layerName, const std::vector<double> & refDepths, const std::vector<double> & tarTwt, std::vector<double> & newDepths )
{
   int returnStatus = 0;
   bool canRunSaltModelling = true;

   // Prepare (check licenses)
   returnStatus = FastcauldronStartup::prepare( canRunSaltModelling );
   // Startup
   if ( returnStatus == 0 ) returnStatus = FastcauldronStartup::startup( argc, argv, canRunSaltModelling );
   // If startup sucessful, run fastcauldron
   if ( returnStatus == 0 ) returnStatus = FastcauldronStartup::run();

   // if fastcauldron run sucefully adjust the depth
   if ( returnStatus == 0 )
   {

      const DataAccess::Interface::Formation * formation = FastcauldronSimulator::getInstance().findFormation( layerName );
      const Interface::Snapshot* presentDaySnapshot = FastcauldronSimulator::getInstance().findOrCreateSnapshot( 0.0 );
      assert( ( "Present day snapshot must be created", presentDaySnapshot != 0 ) );

      const DataAccess::Interface::Property * twtProperty = FastcauldronSimulator::getInstance().findProperty( "TwoWayTime" );
      const DataAccess::Interface::Property * depthProperty = FastcauldronSimulator::getInstance().findProperty( "Depth" );

      const DataAccess::Interface::PropertyValueList * depthPropertyValues = FastcauldronSimulator::getInstance().getPropertyValues( FORMATION, depthProperty, presentDaySnapshot, 0, formation, 0, VOLUME );
      const DataAccess::Interface::PropertyValueList * twtPropertyValues = FastcauldronSimulator::getInstance().getPropertyValues( FORMATION, twtProperty, presentDaySnapshot, 0, formation, 0, VOLUME );

      assert( ( *depthPropertyValues ).size() == 1 );
      assert( ( *twtPropertyValues ).size() == 1 );

      const DataAccess::Interface::GridMap * twtGridMap = ( *twtPropertyValues )[0]->getGridMap();
      const DataAccess::Interface::GridMap * depthGridMap = ( *depthPropertyValues )[0]->getGridMap();

      std::vector<double> twtSim( refDepths.size() );
      std::vector<double> twtSimUpper( refDepths.size() );
      std::vector<double> depthSim( refDepths.size() );
      std::vector<double> depthSimUpper( refDepths.size() );

      // start from surface
      unsigned int maxK = twtGridMap->getDepth() - 1;
      fillArray( twtGridMap, twtSim, maxK, TWTCONVFACT );
      fillArray( depthGridMap, depthSim, maxK, 1.0 );
      bool searching = false;

      for ( size_t i = 0; i != newDepths.size(); ++i )
      {
         newDepths[i] = DataAccess::Interface::DefaultUndefinedScalarValue;
         if ( twtSim[i] > tarTwt[i] )
         {
            if ( abs( twtSim[i] ) > 1e-12 )
               newDepths[i] = depthSim[i] / twtSim[i] * tarTwt[i];
            else
               newDepths[i] = depthSim[i];
         }
         if ( !searching && newDepths[i] == DataAccess::Interface::DefaultUndefinedScalarValue ) searching = true;
      }

      twtSimUpper = twtSim;
      depthSimUpper = depthSim;
      for ( int k = maxK - 1; k >= 0; --k )
      {
         if ( !searching ) break; // all depths are set, no need to retrive other data
         searching = false;

         fillArray( twtGridMap, twtSim, k, TWTCONVFACT );
         fillArray( depthGridMap, depthSim, k, 1.0 );

         for ( size_t i = 0; i != newDepths.size(); ++i )
         {
            if ( twtSim[i] > tarTwt[i] && newDepths[i] == DataAccess::Interface::DefaultUndefinedScalarValue )
            {

               double twtSimDiff = twtSim[i] - twtSimUpper[i];
               double twtMeasDiffUpper = tarTwt[i] - twtSimUpper[i];
               double twtMeasDiffLower = twtSim[i] - tarTwt[i];

               double depthSimDiff = depthSim[i] - depthSimUpper[i];

               if ( abs( twtSimDiff ) > 1e-12 )
               {
                  newDepths[i] = depthSimDiff / twtSimDiff * twtMeasDiffUpper + depthSimUpper[i];
               }
               else
               {
                  if ( abs( twtMeasDiffLower ) < abs( twtMeasDiffUpper ) )
                     newDepths[i] = depthSim[i];
                  else
                     newDepths[i] = depthSimUpper[i];
               }
            }
            if ( !searching && newDepths[i] == DataAccess::Interface::DefaultUndefinedScalarValue ) searching = true;
         }
         twtSimUpper = twtSim;
         depthSimUpper = depthSim;
      }

      for ( size_t i = 0; i != newDepths.size(); ++i )
      {
         if ( newDepths[i] == DataAccess::Interface::DefaultUndefinedScalarValue ) newDepths[i] = depthSim[i]; // set to the last deepmost simulated depth (k == 0)
         if ( newDepths[i] - refDepths[i] < 0.0 ) newDepths[i] = refDepths[i]; // if everything goes wrong, use the same value as the starting map
      }

      delete twtPropertyValues;
      delete depthPropertyValues;
   }

   // delete factory, appctx, exit license
   returnStatus = FastcauldronStartup::finalise( returnStatus );

   return returnStatus;
}

//--Automatic calibration of depth maps--//

int main(int argc, char** argv)
{
   // the master path
   ibs::Path        masterPath( "." );
   ibs::Path        fullMasterPath( masterPath.fullPath( ) );
   ibs::FilePath    masterResults( fullMasterPath );
   std::string      casaResultsFile( "CasaModel_Results.HDF" );
   masterResults << casaResultsFile;

   if ( masterResults.exists() ) 
   {
      LogHandler( LogHandler::WARNING_SEVERITY ) << "Removing existing " << casaResultsFile;
      masterResults.remove( );
   }

   // where the final results are stored
   ibs::FolderPath  finalResultsPath( "." );
   std::string finalResultsFolder( "CalibratedDepthMapsProject" );
   finalResultsPath << finalResultsFolder;
   ibs::FilePath   finalResults( finalResultsPath );
   finalResults << casaResultsFile;

   // If bad_alloc is raised during an allocation of memory then this function will be called.
   std::set_new_handler ( abortOnBadAlloc );
   MemoryChecker mc;

   // Initialise Petsc and get rank & size of MPI
   PetscInitialize( &argc, &argv, (char *)0, PETSC_NULL );
   int rank;
   MPI_Comm_rank( PETSC_COMM_WORLD, &rank );

   // Intitialise fastcauldron logger (it is a singleton, must be initialize only once!)
   try{
      PetscBool log = PETSC_FALSE;
      PetscOptionsHasName( PETSC_NULL, "-verbosity", &log );
      if ( log ){
         char verbosity[11];
         PetscOptionsGetString( PETSC_NULL, "-verbosity", verbosity, 11, 0 );
         if ( !strcmp( verbosity, "quiet" ) ) { LogHandler( "fastcauldron", LogHandler::QUIET_LEVEL, rank ); }
         else if ( !strcmp( verbosity, "minimal" ) ) { LogHandler( "fastcauldron", LogHandler::MINIMAL_LEVEL, rank ); }
         else if ( !strcmp( verbosity, "normal" ) ) { LogHandler( "fastcauldron", LogHandler::NORMAL_LEVEL, rank ); }
         else if ( !strcmp( verbosity, "detailed" ) ) { LogHandler( "fastcauldron", LogHandler::DETAILED_LEVEL, rank ); }
         else if ( !strcmp( verbosity, "diagnostic" ) ) { LogHandler( "fastcauldron", LogHandler::DIAGNOSTIC_LEVEL, rank ); }
         else throw formattingexception::GeneralException( ) << "Unknown <" << verbosity << "> option for -verbosity command line parameter.";
      }
      else{
         LogHandler( "fastcauldron", LogHandler::DETAILED_LEVEL, rank );
      }
   }
   catch ( formattingexception::GeneralException& ex ){
      std::cout << ex.what( );
      return 1;
   }
   catch ( ... ){
      std::cout << "Fatal error when initialising log file(s).";
      return 1;
   }

   // read command line options
   PetscInt   startingLayer;
   PetscInt   endingLayer;
   char projectName[MAXLINESIZE];

   PetscOptionsGetInt( PETSC_NULL, "-startingLayer", &startingLayer, PETSC_NULL);
   PetscOptionsGetInt( PETSC_NULL, "-endingLayer", &endingLayer, PETSC_NULL);
   PetscOptionsGetString( PETSC_NULL, "-project", projectName, MAXLINESIZE, 0 );

   // Models (they get destroyed after petscfinilize)
   std::unique_ptr<mbapi::Model> mdl;
   mdl.reset( new mbapi::Model( ) );
   std::unique_ptr<mbapi::Model> refMdl;
   refMdl.reset( new mbapi::Model( ) );

   try
   {
      //Load the initial model in mdl and reference mdl (refMdl)
      mbapi::StratigraphyManager::LayerID startLayer( startingLayer );
      mbapi::StratigraphyManager::LayerID endLayer( endingLayer );
  
      if ( ErrorHandler::NoError != mdl->loadModelFromProjectFile( projectName ) )
      {
         throw ErrorHandler::Exception( ErrorHandler::UnknownError ) << " wrong input file name ";
      }

      if ( ErrorHandler::NoError != refMdl->loadModelFromProjectFile( projectName ) )
      {
         throw ErrorHandler::Exception( ErrorHandler::UnknownError ) << " wrong input file name ";
      }

      // Get the layers 
      mbapi::StratigraphyManager & stMgr = mdl->stratigraphyManager( );
      const std::vector<mbapi::StratigraphyManager::LayerID> & layersIDs = stMgr.layersIDs( );
      if ( startLayer < 0 || startLayer >= layersIDs.size() )
      {
         throw ErrorHandler::Exception( ErrorHandler::UnknownError ) << " The value of -startingLayer is invalid: " << startLayer;
      }
      if ( endLayer < startLayer || endLayer < 0 || endLayer >= layersIDs.size( ) )
      {
         throw ErrorHandler::Exception( ErrorHandler::UnknownError ) << " The value of -endingLayer is invalid: " << endLayer;
      }

      // Now check the twt maps are present for all layers between the starting and the ending layers
      std::map<mbapi::StratigraphyManager::LayerID, std::string> twtMaps;
      for ( mbapi::StratigraphyManager::LayerID l = startLayer; l <= endLayer; ++l )
      {
         std::string twtGrid = stMgr.twtGridName( l + 1 ); //get the twt of the bottom surface  (+1)
         double twtVal = stMgr.twtValue( l + 1 );
         if ( twtGrid.empty() && twtVal == UndefinedDoubleValue )
         {
            throw ErrorHandler::Exception( ErrorHandler::UnknownError ) << " An invalid twt measurement was found for the top surface of the layer " << l;
         }
         twtMaps[l] = twtGrid ; //stores the bottom of the twtGrid
      }

      // Get the current depth of the endingLayer
      std::vector<double> depthBelowEndLayer;
      if ( ErrorHandler::ReturnCode::NoError != getGridMapDepthValues( mdl.get( ), endLayer + 1, depthBelowEndLayer ))
      {
         throw ErrorHandler::Exception( ErrorHandler::UnknownError ) << " Cannot get the depth map for the layer " << endLayer + 1;
      }
     
      // Calculate the "isopacks" for the layers below the endingLayer
      std::map<mbapi::StratigraphyManager::LayerID, std::vector<double>> isoPacks;
      for ( mbapi::StratigraphyManager::LayerID l = endLayer + 2; l <= layersIDs.size(); ++l )
      {  
         // Get the depths
         std::vector<double> bottomDepth;
         if ( ErrorHandler::ReturnCode::NoError != getGridMapDepthValues( mdl.get( ), l, bottomDepth ) )
         {
            throw ErrorHandler::Exception( ErrorHandler::UnknownError ) << " Cannot get the depth map for the layer " << l;
         }
         // Calculate the isopacks (layer thickness)
         for ( size_t i = 0; i != bottomDepth.size( ); ++i )
         {
            bottomDepth[i] = bottomDepth[i] - depthBelowEndLayer[i];
         }
         isoPacks[l] = bottomDepth;
      }

      // prepare mdl to run the automatic depth calibration 
      if ( ErrorHandler::ReturnCode::NoError != prepareProject( mdl.get(), refMdl.get(), startLayer ) )
      {
         throw ErrorHandler::Exception( ErrorHandler::UnknownError ) << " Project could not be modified before running the automatic depth calibration ";
      }

      // variables that are used in the main depth calibration loop
      std::vector<double> refDepths;
      std::vector<double> newDepths;
      std::vector<double> refTwt;
      std::vector<double> tarTwt;
      std::map<mbapi::StratigraphyManager::LayerID, std::string> correctedMapsNames;
      std::map<mbapi::StratigraphyManager::LayerID, mbapi::MapsManager::MapID > correctedMapsIDs;

      // The main depth calibration loop
      for ( mbapi::StratigraphyManager::LayerID currentLayer = startLayer; currentLayer <= endLayer; ++currentLayer )
      {
         // Add the record of the bottom most layer from the reference model
         mbapi::StratigraphyManager::LayerID nextLayer = currentLayer + 1;
         if ( ErrorHandler::ReturnCode::NoError != mdl->copyRecordFromModel( "StratIoTbl", *( refMdl.get() ), nextLayer ) )
         {
            throw ErrorHandler::Exception( ErrorHandler::UnknownError ) << " Cannot copy record of the layer " << nextLayer <<" from the reference model ";
         }
         if ( ErrorHandler::ReturnCode::NoError != mdl->setTableValue( "StratIoTbl", nextLayer, "MobileLayer", (long)0 ) )
         {
            throw ErrorHandler::Exception( ErrorHandler::UnknownError ) << "Cannot set MobileLayer to 0 for layer " << nextLayer;
         }

         // Change the deposequence everytime a new layer is added
         if ( ErrorHandler::ReturnCode::NoError != mdl->setTableValue( "StratIoTbl", nextLayer, "DepoSequence", (long)-9999 ) )
         {
            throw ErrorHandler::Exception( ErrorHandler::UnknownError ) << "Cannot set the depo sequence of the layer " << nextLayer;
         }
         for ( mbapi::StratigraphyManager::LayerID l = 0; l <= currentLayer; ++l )
         {
            mbapi::StratigraphyManager::LayerID layer = currentLayer + 1 - l;
            if ( ErrorHandler::ReturnCode::NoError != mdl->setTableValue( "StratIoTbl", l, "DepoSequence", (long) layer  ) )
            {
               throw ErrorHandler::Exception( ErrorHandler::UnknownError ) << " Cannot set the depo sequence of the layer " << layer;
            }
         }

         // Get the maximum seismic velocity of the current layer
         std::vector<std::string> lithoList;
         std::vector<double>      lithoPercent;
         std::vector<std::string> lithoPercMap;
         mbapi::LithologyManager & litMgr = mdl->lithologyManager( );
         if ( ErrorHandler::ReturnCode::NoError != stMgr.layerLithologiesList( currentLayer, lithoList, lithoPercent, lithoPercMap ) )
         {
            throw ErrorHandler::Exception( ErrorHandler::UnknownError ) << "Cannot read the lithologies for the current layer";
         }

         double maxSeisVel = 0;
         for ( int lith = 0; lith != lithoList.size(); ++lith )
         {
            mbapi::LithologyManager::LithologyID lithID = litMgr.findID( lithoList[lith] );
            if ( lithID == UndefinedIDValue )
            {
               throw ErrorHandler::Exception( ErrorHandler::UnknownError ) << "Cannot find the id for the lithology " << lith;
            }
            // For these surfaces get the rock seismic velocity
            double seisVel = litMgr.seisVelocity( lithID );
            if ( seisVel == UndefinedIDValue )
            {
               throw ErrorHandler::Exception( ErrorHandler::UnknownError ) << "Cannot find the seismic velocity for the lithology " << lith;
            }
            maxSeisVel < seisVel ? maxSeisVel = seisVel : maxSeisVel = maxSeisVel;
         }

         // Get the depths of the top surface, if currentLayer == starting layer it is the reference surface. Otherwise reuse the adjusted depths from the previous iteration
         if ( currentLayer == startLayer )
         {
            // Get the depths
            if ( ErrorHandler::ReturnCode::NoError != getGridMapDepthValues( mdl.get( ), currentLayer, refDepths ) )
            {
               throw ErrorHandler::Exception( ErrorHandler::UnknownError ) << " Cannot get the depth values of the reference surface ";
            }
            newDepths.resize( refDepths.size() );
            refTwt.resize( refDepths.size() );
         }
         else
         {
            refDepths = newDepths;
            refTwt = tarTwt;
         }

         // Get the measured TWT
         mbapi::MapsManager & mapsMgr = mdl->mapsManager( );
         mbapi::MapsManager::MapID twtMapID = mapsMgr.findID( twtMaps[currentLayer] );
         if ( ErrorHandler::ReturnCode::NoError != mapsMgr.mapGetValues( twtMapID, tarTwt ))
         {
            throw ErrorHandler::Exception( ErrorHandler::UnknownError ) << " Cannot get the measured twt map for layer " << currentLayer;
         }

         // Increase the depths
         std::vector<double> increasedDepths( refDepths.size() );
         if ( currentLayer == startLayer )
         {
            for ( size_t i = 0; i != refDepths.size(); ++i )
            {
               if ( refDepths[i] == DataAccess::Interface::DefaultUndefinedMapValue || tarTwt[i] == DataAccess::Interface::DefaultUndefinedMapValue )
               {
                  increasedDepths[i] = DataAccess::Interface::DefaultUndefinedMapValue;
               }
               else
               {
                  tarTwt[i] = tarTwt[i] * TWTCONVFACT;
                  increasedDepths[i] = refDepths[i] + tarTwt[i] * maxSeisVel * 0.5;
               }
            }
         }
         else
         {
            for ( size_t i = 0; i != refDepths.size(); ++i )
            {
               if ( refDepths[i] == DataAccess::Interface::DefaultUndefinedMapValue || tarTwt[i] == DataAccess::Interface::DefaultUndefinedMapValue || refTwt[i] == DataAccess::Interface::DefaultUndefinedMapValue )
               {
                  increasedDepths[i] = DataAccess::Interface::DefaultUndefinedMapValue;
               }
               else
               {
                  tarTwt[i] = tarTwt[i] * TWTCONVFACT; // convert twt
                  increasedDepths[i] = refDepths[i] + ( tarTwt[i] - refTwt[i] ) * maxSeisVel * 0.5;
               }
            }
         }

         // Create a new map (in map manager) and set it as depth grid ( in the stratigraphy manager) 
         std::string mapName = "Surface_" + std::to_string( nextLayer );
         correctedMapsNames[nextLayer] = mapName;
         
         // Generate a new HDF map
         if ( UndefinedIDValue == mapsMgr.generateMap( "StratIoTbl", mapName, increasedDepths, casaResultsFile ) )
         {
            throw ErrorHandler::Exception( ErrorHandler::UnknownError ) << "Cannot generate the map with increased depths";
         }
         if ( ErrorHandler::ReturnCode::NoError != mdl->setTableValue( "StratIoTbl", nextLayer, "DepthGrid", mapName ) )
         {
            throw ErrorHandler::Exception( ErrorHandler::UnknownError ) << "Cannot set the map " << mapName << " as new depth map";
         }

         // Create a new folder where to run the model and save the input files
         ibs::FolderPath casePath( "." );
         casePath << mapName;
         MPI_Barrier( PETSC_COMM_WORLD );
         if (rank == 0)
         {
            if ( casePath.exists() ) // clean folder if it is already exist
            {
               LogHandler( LogHandler::WARNING_SEVERITY ) << "Folder for " << mapName << " already exist. " << casePath.fullPath().path() << " will be deleted";
               casePath.remove();
            }
            casePath.create();
            ibs::FilePath caseProject( casePath );
            caseProject << mdl->projectFileName();
            mdl->saveModelToProjectFile( caseProject.path().c_str(), true );
            
         }
         MPI_Barrier( PETSC_COMM_WORLD ); 
         
         // Change to mapName directory
         if ( !casePath.setPath() )
         {
            throw ErrorHandler::Exception( ErrorHandler::UnknownError ) << "Cannot change to surface calibration directory " << casePath.fullPath().path();
         }
         
         // Warn that Fastcauldron will run
         std::string layerName = stMgr.layerName( currentLayer );
         if ( rank == 0 )
         {
            LogHandler( LogHandler::INFO_SEVERITY ) << "Running Fastcauldron to adjust the bottom surface of the layer: " << layerName;
         }

         // Run Fastcauldron
         int  returnStatus = runFastCauldron( argc, argv, rank, layerName, refDepths, tarTwt, newDepths );
         if ( returnStatus != 0 )
         {
            throw ErrorHandler::Exception( ErrorHandler::UnknownError ) << "Fastcauldron terminated with status " << returnStatus;
         }

         // Now write the corrected map
         ibs::FilePath localModelResults( "." );
         localModelResults << casaResultsFile;
         mbapi::MapsManager::MapID correctMap = mapsMgr.generateMap( "StratIoTbl", mapName, newDepths, casaResultsFile );
         if ( UndefinedIDValue == correctMap )
         {
            throw ErrorHandler::Exception( ErrorHandler::UnknownError ) << "Could not generate the map with corrected depths";
         }
         correctedMapsIDs[nextLayer] = correctMap;

         // Here casa results file is written, we can copy them to the master directory
         MPI_Barrier( PETSC_COMM_WORLD );
         if ( rank == 0 )
         {
            masterResults.remove();
            if ( !localModelResults.copyFile( masterResults ) )
            {
               throw ErrorHandler::Exception( ErrorHandler::UnknownError ) << "Cannot copy file to master directory " << fullMasterPath.fullPath().path();
            }
         }
         MPI_Barrier( PETSC_COMM_WORLD );

         // Go back to the master directory
         if ( !fullMasterPath.setPath( ) )
         {
            throw ErrorHandler::Exception( ErrorHandler::UnknownError ) << "Cannot change to master directory " << fullMasterPath.fullPath( ).path( );
         }
      }  

      // Modify the REFERENCE model so the modified maps are listed in the GridMapIoTbl and StratIoTbl

      // First update StratIoTbl and GridMapIoTbl
      for ( mbapi::StratigraphyManager::LayerID currentLayer = startLayer; currentLayer <= endLayer; ++currentLayer )
      {
         mbapi::StratigraphyManager::LayerID nextLayer = currentLayer + 1;
         if ( ErrorHandler::ReturnCode::NoError != refMdl->copyRecordFromModel( "GridMapIoTbl", *( mdl.get( ) ), correctedMapsIDs[nextLayer] ) )
         {
            throw ErrorHandler::Exception( ErrorHandler::UnknownError ) << "Cannot copy the modified GridMapIoTbl from the modified model to the reference model ";
         }
         if ( ErrorHandler::ReturnCode::NoError != refMdl->setTableValue( "StratIoTbl", nextLayer, "DepthGrid", correctedMapsNames[nextLayer] ) )
         {
            throw ErrorHandler::Exception( ErrorHandler::UnknownError ) << " Cannot set in the reference model name of the modified depth map";
         }
         if ( ErrorHandler::ReturnCode::NoError != refMdl->setTableValue( "StratIoTbl", currentLayer, "DepoSequence", (long)( layersIDs.size( ) - currentLayer ) ) )
         {
            throw ErrorHandler::Exception( ErrorHandler::UnknownError ) << "Cannot set in the reference model name the original DepoSequence";
         }
      }

      // Create a folder to store the new project and copy the results (rank 0)
      ibs::FilePath finalProject( finalResultsPath );
      finalProject << refMdl->projectFileName( );
      MPI_Barrier( PETSC_COMM_WORLD );
      if ( rank == 0 )
      {
         // Clean the folder
         if ( finalResultsPath.exists( ) )
         {
            LogHandler( LogHandler::WARNING_SEVERITY ) << "Folder to store the adjusted depth maps already exists: " << finalResultsPath.fullPath( ).path( ) << " will be deleted";
            finalResultsPath.remove( );
         }
         finalResultsPath.create( );

         refMdl->saveModelToProjectFile( finalProject.path( ).c_str( ), true );
         
         // The file with the maps needs to be copied separatly because maps manager doesn not know already about it. It will only after reloading the modified model
         if ( !masterResults.copyFile( finalResults ) )
         {
            throw ErrorHandler::Exception( ErrorHandler::UnknownError ) << "Cannot copy file result file to " << fullMasterPath.fullPath( ).path( );
         }
      }
      MPI_Barrier( PETSC_COMM_WORLD );

      // Here we reset the project so we have an up to date new map manager (with the correct sequence the copied maps).
      refMdl.reset( new mbapi::Model( ) );
      if ( ErrorHandler::NoError != refMdl->loadModelFromProjectFile( finalProject.path( ).c_str( ) ) )
      {
         throw ErrorHandler::Exception( ErrorHandler::UnknownError ) << " wrong input file name ";
      }

      // For the layers below the endingLayer append the "isopacks" previously calculated
      mbapi::MapsManager & mapsMgrRefMdl = refMdl->mapsManager( );
      for ( mbapi::StratigraphyManager::LayerID l = endLayer + 2; l <= layersIDs.size(); ++l )
      {
         std::string mapName = "Surface_" + std::to_string( l );
         std::string surfaceName = stMgr.surfaceName( l );
         for ( size_t i = 0; i != newDepths.size(); ++i )
         {
            isoPacks[l][i] = newDepths[i] + isoPacks[l][i];
         }
         correctedMapsNames[l] = mapName;
         correctedMapsIDs[l] = mapsMgrRefMdl.generateMap( "StratIoTbl", mapName, isoPacks[l], casaResultsFile );
         if ( UndefinedIDValue == correctedMapsIDs[l] )
         {
            throw ErrorHandler::Exception( ErrorHandler::UnknownError ) << "Cannot generate the final map for layer " << l;
         }
         // Modify record with the new name for the depth gris map     
         if ( ErrorHandler::ReturnCode::NoError != refMdl->setTableValue( "StratIoTbl", l, "DepthGrid", mapName ) )
         {
            throw ErrorHandler::Exception( ErrorHandler::UnknownError ) << " Cannot set in the reference model the name of the modified depth map " << mapName;
         }
         if ( ErrorHandler::ReturnCode::NoError != refMdl->setTableValue( "StratIoTbl", l, "DepoSequence", (long)( layersIDs.size( ) - l ) ) )
         {
            throw ErrorHandler::Exception( ErrorHandler::UnknownError ) << "Cannot set in the reference model the name the original DepoSequence for layer " << l;
         }
      }

      MPI_Barrier( PETSC_COMM_WORLD );
      if ( rank == 0 )
      {
         refMdl->saveModelToProjectFile( finalProject.path( ).c_str( ), true );
      }
      MPI_Barrier( PETSC_COMM_WORLD );
      
   } 
   catch ( const ErrorHandler::Exception & ex )
   {
      std::cout << ex.what( );
      // Close Petsc
      PetscFinalize( );
      return 1;                           
   }

   // Close Petsc
   PetscFinalize( );
   return 0;
}

//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// to do: add the case when the refence is the sealevel and surface 0 depths are calculated using the seismic velocity of seawater

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

static const double       twtconvFactor(0.001);
static const std::string  resultsFile( "CalibratedInputs.HDF" );
static const std::string  finalResultsFolder( "CalibratedDepthMapsProject" );

// Fill the values of the GridMap in one local array 
static void fillArray( const DataAccess::Interface::GridMap *  grid, std::vector<double>& v, int k, const double convFact )
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
// Fill the values of the GridMap contained in the project in one local array 
static ErrorHandler::ReturnCode getGridMapDepthValues( mbapi::Model * mdl, const mbapi::StratigraphyManager::SurfaceID s, std::vector<double> & v )
{

   mbapi::MapsManager & mapsMgr = mdl->mapsManager( );

   // Get the map name and id
   std::string depthMap = mdl->tableValueAsString( "StratIoTbl", s, "DepthGrid" );
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
// Clean the project tables
static ErrorHandler::ReturnCode  prepareProject( mbapi::Model * mdl )
{

   ErrorHandler::ReturnCode error( ErrorHandler::ReturnCode::NoError );
   // Set full resolution
   if ( mdl->tableValueAsInteger( "ProjectIoTbl", 0, "ScaleX" ) > 1 || mdl->tableValueAsInteger( "ProjectIoTbl", 0, "ScaleY" ) > 1 )
   {
      LogHandler( LogHandler::WARNING_SEVERITY ) << "The project is subsampled. For the depth calibration the full resolution is required. Changing ScaleX and ScaleY to run in full resolution";
      if ( error == ErrorHandler::ReturnCode::NoError ) error = mdl->setTableValue( "ProjectIoTbl", 0, "ScaleX", (long)1 );
      if ( error == ErrorHandler::ReturnCode::NoError ) error = mdl->setTableValue( "ProjectIoTbl", 0, "ScaleY", (long)1 );
   }

   // Set full window
   int NumberX = mdl->tableValueAsInteger( "ProjectIoTbl", 0, "NumberX" );
   int NumberY = mdl->tableValueAsInteger( "ProjectIoTbl", 0, "NumberY" );
   if ( mdl->tableValueAsInteger( "ProjectIoTbl", 0, "WindowXMin" ) > 0 || mdl->tableValueAsInteger( "ProjectIoTbl", 0, "WindowYMin" ) > 0 || mdl->tableValueAsInteger( "ProjectIoTbl", 0, "WindowXMax" ) < NumberX - 1 || mdl->tableValueAsInteger( "ProjectIoTbl", 0, "WindowYMax" ) < NumberY - 1 )
   {
      LogHandler( LogHandler::WARNING_SEVERITY ) << "The project is windowed. For the depth calibration the entire domain is required. Changing WindowX and WindowY in to run the simulation over the entire domain";
      if ( error == ErrorHandler::ReturnCode::NoError ) error = mdl->setTableValue( "ProjectIoTbl", 0, "WindowXMin", (long)1 );
      if ( error == ErrorHandler::ReturnCode::NoError ) error = mdl->setTableValue( "ProjectIoTbl", 0, "WindowYMin", (long)1 );
      if ( error == ErrorHandler::ReturnCode::NoError ) error = mdl->setTableValue( "ProjectIoTbl", 0, "WindowXMax", (long)NumberX );
      if ( error == ErrorHandler::ReturnCode::NoError ) error = mdl->setTableValue( "ProjectIoTbl", 0, "WindowYMax", (long)NumberY );
   }

   // Clear tables
   if ( error == ErrorHandler::ReturnCode::NoError ) error = mdl->clearTable( "TimeIoTbl" );
   if ( error == ErrorHandler::ReturnCode::NoError ) error = mdl->clearTable( "3DTimeIoTbl" );
   if ( error == ErrorHandler::ReturnCode::NoError ) error = mdl->clearTable( "1DTimeIoTbl" );
   if ( error == ErrorHandler::ReturnCode::NoError ) error = mdl->clearTable( "ReservoirIoTbl" );
   if ( error == ErrorHandler::ReturnCode::NoError ) error = mdl->clearTable( "MobLayThicknIoTbl" );
   if ( error == ErrorHandler::ReturnCode::NoError ) error = mdl->clearTable( "FilterDepthIoTbl" );
   if ( error == ErrorHandler::ReturnCode::NoError ) error = mdl->clearTable( "FilterTimeIoTbl" );
   if ( error == ErrorHandler::ReturnCode::NoError ) error = mdl->clearTable( "FilterTimeDepthIoTbl" );
   if ( error == ErrorHandler::ReturnCode::NoError ) error = mdl->clearTable( "SnapshotIoTbl" );

   // Set the properties we want to save in the FilterTimeIoTbl
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

   return error;
}
// Modify the tables
static ErrorHandler::ReturnCode  modifyTables( mbapi::Model *       mdl, 
   const mbapi::StratigraphyManager::SurfaceID                      nextSurface,
   std::map<mbapi::StratigraphyManager::SurfaceID, std::string> &   correctedMapsNames,
   std::map<mbapi::StratigraphyManager::SurfaceID, int > &          correctedMapsSequenceNbr,
   std::map<mbapi::StratigraphyManager::SurfaceID, int > &          correctedMapsIDs )
{
   ErrorHandler::ReturnCode error( ErrorHandler::ReturnCode::NoError );

   mbapi::StratigraphyManager & stMgr = mdl->stratigraphyManager();
   const std::vector<mbapi::StratigraphyManager::SurfaceID> & surfacesIDs = stMgr.surfacesIDs( );

   // Change the deposequence from the top surface to nextSurface - 1
   for ( mbapi::StratigraphyManager::SurfaceID s = 0; s < nextSurface; ++s )
   {
      long deposequence = nextSurface - s;
      if ( error == ErrorHandler::ReturnCode::NoError ) error = mdl->setTableValue( "StratIoTbl", s, "MobileLayer", (long)0 );
      if ( error == ErrorHandler::ReturnCode::NoError ) error = mdl->setTableValue( "StratIoTbl", s, "DepoSequence", deposequence );
   }

   // Set the deposequence of the nextSurface surface to -9999
   if ( error == ErrorHandler::ReturnCode::NoError ) error = mdl->setTableValue( "StratIoTbl", nextSurface, "MobileLayer", (long)0 );
   if ( error == ErrorHandler::ReturnCode::NoError ) error = mdl->setTableValue( "StratIoTbl", nextSurface, "DepoSequence", (long)-9999 );

   // Clean the records below nextSurface (note that erase is used so THE nextSurface + 1 record is deleted recursivly )
   for ( mbapi::StratigraphyManager::SurfaceID s = nextSurface + 1; s < surfacesIDs.size( ); ++s )
   {
      if ( error == ErrorHandler::ReturnCode::NoError ) error = mdl->removeRecordFromTable( "StratIoTbl", nextSurface + 1 );
   }

   // Append the correct maps names if present
   for ( auto it = correctedMapsNames.begin(); it != correctedMapsNames.end(); ++it )
   {
      // StratIoTbl
      if ( error == ErrorHandler::ReturnCode::NoError ) error = mdl->setTableValue( "StratIoTbl", it->first, "DepthGrid", it->second );
      // GridMapIoTbl
      if ( error == ErrorHandler::ReturnCode::NoError ) error = mdl->addRowToTable( "GridMapIoTbl") ;
      if ( error == ErrorHandler::ReturnCode::NoError ) error = mdl->setTableValue( "GridMapIoTbl", correctedMapsIDs[it->first], "ReferredBy", "StratIoTbl" );
      if ( error == ErrorHandler::ReturnCode::NoError ) error = mdl->setTableValue( "GridMapIoTbl", correctedMapsIDs[it->first], "MapName", correctedMapsNames[it->first] );
      if ( error == ErrorHandler::ReturnCode::NoError ) error = mdl->setTableValue( "GridMapIoTbl", correctedMapsIDs[it->first], "MapType", "HDF5" );
      if ( error == ErrorHandler::ReturnCode::NoError ) error = mdl->setTableValue( "GridMapIoTbl", correctedMapsIDs[it->first], "MapFileName", resultsFile );
      if ( error == ErrorHandler::ReturnCode::NoError ) error = mdl->setTableValue( "GridMapIoTbl", correctedMapsIDs[it->first], "FileId", (long)-1 );
      if ( error == ErrorHandler::ReturnCode::NoError ) error = mdl->setTableValue( "GridMapIoTbl", correctedMapsIDs[it->first], "MapSeqNbr", (long)correctedMapsSequenceNbr[it->first] );
   }

   return error;
}
// create a case folder with the input files
static ErrorHandler::ReturnCode createCase( mbapi::Model *    mdl, 
                                            const int         rank,
                                            ibs::FolderPath&  casePath,
                                            ibs::FilePath&    caseProject,
                                            ibs::FilePath&    masterResults, 
                                            ibs::FilePath&    casePathResults )
{
   // Save the project and input data to folder (remove it if it is already present). 
   // The file containing the maps needs to be copied separatly because the map manager does not know it yet (only after the reload).
   // Only rank 0 must perform this operation.

   MPI_Barrier( PETSC_COMM_WORLD );
   if ( rank == 0 )
   {
      if ( casePath.exists() )
      {
         LogHandler( LogHandler::WARNING_SEVERITY ) << "Folder " << casePath.fullPath().path() << " will be deleted";
         casePath.remove();
      }
      casePath.create();

      mdl->saveModelToProjectFile( caseProject.path().c_str(), true );
      if ( !masterResults.copyFile( casePathResults ) )
      {
         return ErrorHandler::UnknownError;
      }
   }
   MPI_Barrier( PETSC_COMM_WORLD );
   return ErrorHandler::NoError;
}
// Run Fastcauldron and truncate the depths
static int runFastCauldron( int argc, 
                            char** argv, int & rank, 
                            const std::string & layerName, 
                            const std::vector<double> & refDepths, 
                            const std::vector<double> & tarTwt, 
                            std::vector<double> & newDepths )
{
   int returnStatus = 0;
   bool canRunSaltModelling = true;

   // Prepare (check licenses)
   returnStatus = FastcauldronStartup::prepare( canRunSaltModelling );
   // Startup
   if ( returnStatus == 0 ) returnStatus = FastcauldronStartup::startup( argc, argv, canRunSaltModelling );
   // If startup sucessful, run fastcauldron
   if ( returnStatus == 0 ) returnStatus = FastcauldronStartup::run();
   // if fastcauldron run successfully adjust the depth
   if ( returnStatus == 0 )
   {
      const DataAccess::Interface::Formation * formation = FastcauldronSimulator::getInstance().findFormation( layerName );
      const Interface::Snapshot* presentDaySnapshot = FastcauldronSimulator::getInstance().findOrCreateSnapshot( 0.0 );
      assert( presentDaySnapshot != 0 );

      const DataAccess::Interface::Property * twtProperty = FastcauldronSimulator::getInstance().findProperty( "TwoWayTime" );
      const DataAccess::Interface::Property * depthProperty = FastcauldronSimulator::getInstance().findProperty( "Depth" );
      const DataAccess::Interface::PropertyValueList * depthPropertyValues = FastcauldronSimulator::getInstance().getPropertyValues( FORMATION, depthProperty, presentDaySnapshot, 0, formation, 0, VOLUME );
      const DataAccess::Interface::PropertyValueList * twtPropertyValues = FastcauldronSimulator::getInstance().getPropertyValues( FORMATION, twtProperty, presentDaySnapshot, 0, formation, 0, VOLUME );
      assert( ( *depthPropertyValues ).size( ) == 1 );
      assert( ( *twtPropertyValues ).size( ) == 1 );

      const DataAccess::Interface::GridMap * twtGridMap = ( *twtPropertyValues )[0]->getGridMap();
      const DataAccess::Interface::GridMap * depthGridMap = ( *depthPropertyValues )[0]->getGridMap();

      std::vector<double> twtSim( refDepths.size() );
      std::vector<double> twtSimUpper( refDepths.size() );
      std::vector<double> depthSim( refDepths.size() );
      std::vector<double> depthSimUpper( refDepths.size() );

      // Start from the top
      unsigned int maxK = twtGridMap->getDepth() - 1;
      fillArray( twtGridMap, twtSim, maxK, twtconvFactor );
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

         fillArray( twtGridMap, twtSim, k, twtconvFactor );
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
         // Set to the last deepmost simulated depth (k == 0)
         if ( newDepths[i] == DataAccess::Interface::DefaultUndefinedScalarValue ) newDepths[i] = depthSim[i]; 
         // If everything goes wrong, use the refDepths value 
         if ( newDepths[i] - refDepths[i] < 0.0 ) newDepths[i] = refDepths[i]; 
      }

      delete twtPropertyValues;
      delete depthPropertyValues;
   }

   // delete factory, appctx, exit license
   returnStatus = FastcauldronStartup::finalise( returnStatus );

   return returnStatus;
}

static void abortOnBadAlloc( ) {
   cerr << " cannot allocate resources, aborting" << endl;
   MPI_Abort( PETSC_COMM_WORLD, 3 );
}

//--Automatic calibration of depth maps--//

int main(int argc, char** argv)
{
   // The master path
   ibs::Path        masterPath( "." );
   ibs::Path        fullMasterPath( masterPath.fullPath( ) );
   ibs::FilePath    masterResults( fullMasterPath );
   masterResults << resultsFile;

   // Removing the result file if exist
   if ( masterResults.exists() ) 
   {
      LogHandler( LogHandler::WARNING_SEVERITY ) << "Removing existing " << resultsFile;
      masterResults.remove( );
   }

   // Where the final results are stored
   ibs::FolderPath  finalResultsPath( "." );
   finalResultsPath << finalResultsFolder;
   ibs::FilePath   finalResults( finalResultsPath );
   finalResults << resultsFile;

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
   int   optionReferenceSurface;
   int   optionEndSurface;
   char projectName[MAXLINESIZE];
   int ierr;
    
   ierr = PetscOptionsGetInt( PETSC_NULL, "-referenceSurface", &optionReferenceSurface, PETSC_NULL ); CHKERRQ( ierr );
   ierr = PetscOptionsGetInt( PETSC_NULL, "-endSurface", &optionEndSurface, PETSC_NULL ); CHKERRQ( ierr );
   ierr = PetscOptionsGetString( PETSC_NULL, "-project", projectName, MAXLINESIZE, 0 ); CHKERRQ( ierr );

   // Try the automatic depth calibration
   try
   {
      // Model: we must have an unique instance of the project database during the runtime, so we load the project several times
      std::unique_ptr<mbapi::Model> mdl( new mbapi::Model( ));
      //Load the initial model in mdl and reference mdl (refMdl)
      if ( ErrorHandler::NoError != mdl->loadModelFromProjectFile( projectName ) )
      {
         throw ErrorHandler::Exception( ErrorHandler::UnknownError ) << " Wrong input file name ";
      }
      MPI_Barrier( PETSC_COMM_WORLD );
      // To be more clear we collect the managers after the load so it is clear to which model they belog to
      mbapi::StratigraphyManager & stMgr = mdl->stratigraphyManager( );
      mbapi::MapsManager &         mapMgr = mdl->mapsManager( );

      // Get the surfaces
      const std::vector<mbapi::StratigraphyManager::SurfaceID> & surfacesIDs = stMgr.surfacesIDs( );
      mbapi::StratigraphyManager::SurfaceID referenceSurface( optionReferenceSurface );
      mbapi::StratigraphyManager::SurfaceID endSurface( optionEndSurface );
      if ( referenceSurface >= surfacesIDs.size( ) )
      {
         throw ErrorHandler::Exception( ErrorHandler::UnknownError ) << " The value of -referenceSurface is invalid: " << referenceSurface;
      }
      if ( endSurface <= referenceSurface || endSurface >= surfacesIDs.size( ) )
      {
         throw ErrorHandler::Exception( ErrorHandler::UnknownError ) << " The value of -endSurface is invalid: " << endSurface;
      }

      // Now check the twt maps are present for all surfaces 
      std::map<mbapi::StratigraphyManager::SurfaceID, std::string> twtMaps;
      for ( mbapi::StratigraphyManager::SurfaceID s = referenceSurface; s <= endSurface; ++s )
      {
         // Get the top twt maps
         std::string twtGrid = stMgr.twtGridName( s ); 
         if ( twtGrid.empty() )
         {
            throw ErrorHandler::Exception( ErrorHandler::UnknownError ) << " No twt maps were found for the surface " << s;
         }
         // Stores the top twt maps of each surface s
         twtMaps[s] = twtGrid; 
      }

      // Get the current depth of the endSurface
      std::vector<double> depthEndSurface;
      if ( ErrorHandler::ReturnCode::NoError != getGridMapDepthValues( mdl.get(), endSurface, depthEndSurface ) )
      {
         throw ErrorHandler::Exception( ErrorHandler::UnknownError ) << " Cannot get the depth map for endSurface";
      }

      // Calculate the "isopacks" for the surfaces below the endSurface
      std::map<mbapi::StratigraphyManager::SurfaceID, std::vector<double>> isoPacks;
      for ( mbapi::StratigraphyManager::SurfaceID s = endSurface + 1; s < surfacesIDs.size( ); ++s )
      {
         // Get the depths
         std::vector<double> bottomDepth;
         if ( ErrorHandler::ReturnCode::NoError != getGridMapDepthValues( mdl.get(), s, bottomDepth ) )
         {
            throw ErrorHandler::Exception( ErrorHandler::UnknownError ) << " Cannot get the depth map for the surface " << s;
         }
         // Calculate the isopacks (layer thickness)
         for ( size_t i = 0; i != bottomDepth.size(); ++i )
         {
            bottomDepth[i] = bottomDepth[i] - depthEndSurface[i];
         }
         isoPacks[s] = bottomDepth;
      }

      // variables that are used in the main depth calibration loop
      std::vector<double>                                            refDepths;
      std::vector<double>                                            newDepths;
      std::vector<double>                                            refTwt;
      std::vector<double>                                            tarTwt;
      std::map<mbapi::StratigraphyManager::SurfaceID, std::string>   correctedMapsNames;
      std::map<mbapi::StratigraphyManager::SurfaceID, int >          correctedMapsSequenceNbr;
      std::map<mbapi::StratigraphyManager::SurfaceID, int >          correctedMapsIDs;
      //create an empty result file
      mapMgr.inizializeMapWriter( resultsFile, false );
      mapMgr.finalizeMapWriter();

      //------------------------------------------------------------------------------------------------------------------------------------------------------------------------//
      // The start of the main depth calibration loop
      //------------------------------------------------------------------------------------------------------------------------------------------------------------------------//
      for ( mbapi::StratigraphyManager::SurfaceID currentSurface = referenceSurface; currentSurface < endSurface; ++currentSurface )
      {
         mbapi::StratigraphyManager::SurfaceID   nextSurface = currentSurface + 1;
         mbapi::StratigraphyManager::LayerID     currentLayer( currentSurface );
         // Destroy, create and reset the model all times, so only one copy of the database is present
         mdl.reset( new mbapi::Model( ) );
         if ( ErrorHandler::NoError != mdl->loadModelFromProjectFile( projectName ) )
         {
            throw ErrorHandler::Exception( ErrorHandler::UnknownError ) << " Wrong input file name ";
         }
         MPI_Barrier( PETSC_COMM_WORLD );
         mbapi::LithologyManager &    litMgrLocal = mdl->lithologyManager( );
         mbapi::StratigraphyManager & stMgrLocal  = mdl->stratigraphyManager( );
         mbapi::MapsManager &         mapsMgrLocal = mdl->mapsManager( );

         // Prepare mdl to run the automatic depth calibration, always load the master project projectName
         if ( ErrorHandler::ReturnCode::NoError != prepareProject( mdl.get() ) )
         {
            throw ErrorHandler::Exception( ErrorHandler::UnknownError ) << " Project could not be prepared for the surface " << currentSurface;
         }

         // Modify StratIoTbl and GridMapIo tbl
         if ( ErrorHandler::NoError != modifyTables( mdl.get( ), nextSurface, correctedMapsNames, correctedMapsSequenceNbr, correctedMapsIDs ) )
         {
            throw ErrorHandler::Exception( ErrorHandler::UnknownError ) << " Tables could not be modified for the surface " << currentSurface;
         }

         // Get the maximum seismic velocity of currentLayer
         std::vector<std::string>   lithoList;
         std::vector<double>        lithoPercent;
         std::vector<std::string>   lithoPercMap;
         if ( ErrorHandler::ReturnCode::NoError != stMgrLocal.layerLithologiesList( currentLayer, lithoList, lithoPercent, lithoPercMap ) )
         {
            throw ErrorHandler::Exception( ErrorHandler::UnknownError ) << "Cannot read the lithologies for the current layer";
         }
         double maxSeisVel = 0;
         for ( int lith = 0; lith != lithoList.size(); ++lith )
         {
            mbapi::LithologyManager::LithologyID lithID = litMgrLocal.findID( lithoList[lith] );
            if ( lithID == UndefinedIDValue )
            {
               throw ErrorHandler::Exception( ErrorHandler::UnknownError ) << "Cannot find the id for the lithology " << lith;
            }
            // For these surfaces get the rock seismic velocity
            double seisVel = litMgrLocal.seisVelocity( lithID );
            if ( seisVel == UndefinedIDValue )
            {
               throw ErrorHandler::Exception( ErrorHandler::UnknownError ) << "Cannot find the seismic velocity for the lithology " << lith;
            }
            maxSeisVel < seisVel ? maxSeisVel = seisVel : maxSeisVel = maxSeisVel;
         }

         // Get the depths of the top surface, if currentSurface == referenceSurface retrive the depths, otherwise reuse the ones of the previous iteration as reference
         if ( currentSurface == referenceSurface )
         {
            if ( ErrorHandler::ReturnCode::NoError != getGridMapDepthValues( mdl.get( ), currentSurface, refDepths ) )
            {
               throw ErrorHandler::Exception( ErrorHandler::UnknownError ) << " Cannot get the depth values of the reference surface ";
            }
            newDepths.resize( refDepths.size() );
         }
         else
         {
            refDepths = newDepths;
         }


         // Get the measured TWT of the top surface
         mbapi::MapsManager::MapID refTwtID = mapsMgrLocal.findID( twtMaps[currentSurface] );
         if ( ErrorHandler::ReturnCode::NoError != mapsMgrLocal.mapGetValues( refTwtID, refTwt ) )
         {
            throw ErrorHandler::Exception( ErrorHandler::UnknownError ) << " Cannot get the measured twt map for the current surface " << currentSurface;
         }
         // Get the measured TWT of the bottom surface
         mbapi::MapsManager::MapID twtMapID = mapsMgrLocal.findID( twtMaps[nextSurface] );
         if ( ErrorHandler::ReturnCode::NoError != mapsMgrLocal.mapGetValues( twtMapID, tarTwt ) )
         {
            throw ErrorHandler::Exception( ErrorHandler::UnknownError ) << " Cannot get the measured twt map for the next surface " << nextSurface;
         }

         // Increase the depths
         std::vector<double> increasedDepths( refDepths.size() );
         // For the topmost layer we take the top depths and we do not need to make twt differences
         if ( currentSurface == 0 )
         {
            for ( size_t i = 0; i != refDepths.size(); ++i )
            {
               if ( refDepths[i] == DataAccess::Interface::DefaultUndefinedMapValue || tarTwt[i] == DataAccess::Interface::DefaultUndefinedMapValue )
               {
                  increasedDepths[i] = DataAccess::Interface::DefaultUndefinedMapValue;
               }
               else
               {
                  tarTwt[i] = tarTwt[i] * twtconvFactor;
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
                  tarTwt[i] = tarTwt[i] * twtconvFactor; // convert twt
                  refTwt[i] = refTwt[i] * twtconvFactor; // convert twt 
                  increasedDepths[i] = refDepths[i] + ( tarTwt[i] - refTwt[i] ) * maxSeisVel * 0.5;
               }
            }
         }

         // Create the name of the new folder where to store the case
         std::string mapName = "Surface_" + std::to_string( nextSurface );
         ibs::FolderPath casePath( "." );
         casePath << mapName;
         // Results path
         ibs::FilePath casePathResults( casePath );
         casePathResults << resultsFile;
         // Project path
         ibs::FilePath caseProject( casePath );
         caseProject << mdl->projectFileName( );

         // Create the case
         if ( ErrorHandler::NoError != createCase( mdl.get( ), rank, casePath, caseProject, masterResults, casePathResults ) )
         {
            throw ErrorHandler::Exception( ErrorHandler::UnknownError ) << " Could not create or copy the inpt files in the folder " << casePath.path().c_str();
         }

         // Reload the model so the map manager gets updated (with the correct map sequence number)
         mdl.reset( new mbapi::Model( ) );
         if ( ErrorHandler::NoError != mdl->loadModelFromProjectFile( caseProject.path( ).c_str( ) ) )
         {
            throw ErrorHandler::Exception( ErrorHandler::UnknownError ) << " wrong input file name ";
         }
         MPI_Barrier( PETSC_COMM_WORLD );
         mbapi::MapsManager & mapsMgrLocalReloaded = mdl->mapsManager( );

         // Create the new map with the increased depths
         correctedMapsNames[nextSurface] = mapName;
         int mapsSequenceNbr = -1;
         correctedMapsIDs[nextSurface] = mapsMgrLocalReloaded.generateMap( "StratIoTbl", mapName, increasedDepths, mapsSequenceNbr, resultsFile );
         if ( UndefinedIDValue == correctedMapsIDs[nextSurface] )
         {
            throw ErrorHandler::Exception( ErrorHandler::UnknownError ) << " Cannot generate the map with increased depths for the surface " << nextSurface;
         }
         if ( ErrorHandler::ReturnCode::NoError != mdl->setTableValue( "StratIoTbl", nextSurface, "DepthGrid", mapName ) )
         {
            throw ErrorHandler::Exception( ErrorHandler::UnknownError ) << "Cannot set the map " << mapName << " as the new depth map of surface " << nextSurface <<" in the StratIoTbl";
         }
         correctedMapsSequenceNbr[nextSurface] = mapsSequenceNbr;

         // Save the project and input data to folder with the new currentSurface map
         MPI_Barrier( PETSC_COMM_WORLD );
         if ( rank == 0 )
         {
            mdl->saveModelToProjectFile( caseProject.path( ).c_str( ), true );
         }
         MPI_Barrier( PETSC_COMM_WORLD );
         
         // Change to the case directory
         if ( !casePath.setPath() )
         {
            throw ErrorHandler::Exception( ErrorHandler::UnknownError ) << "Cannot change to the case directory " << casePath.fullPath().path();
         }

         // Inform that Fastcauldron will run
         std::string layerName = mdl->stratigraphyManager( ).layerName( currentLayer );
         if ( rank == 0 )
         {
            LogHandler( LogHandler::INFO_SEVERITY ) << "Running Fastcauldron to adjust the surface: " << nextSurface;
         }

         // Run Fastcauldron -----------------------------------------------------------------------------------//
         //-----------------------------------------------------------------------------------------------------//

         int  returnStatus = runFastCauldron( argc, argv, rank, layerName, refDepths, tarTwt, newDepths );

         //-----------------------------------------------------------------------------------------------------//
         //-----------------------------------------------------------------------------------------------------//

         if ( returnStatus != 0 )
         {
            throw ErrorHandler::Exception( ErrorHandler::UnknownError ) << " Fastcauldron terminated with status " << returnStatus;
         }

         // Go back to the master path
         if ( !fullMasterPath.setPath( ) )
         {
            throw ErrorHandler::Exception( ErrorHandler::UnknownError ) << " Cannot change to the master directory " << fullMasterPath.fullPath( ).path( );
         }

         // Now write the corrected map (note that the project file path will be appended to the HDF map name, so we must be in the master path to do this operation!)
         mapsSequenceNbr = -1;
         if ( UndefinedIDValue == mapsMgrLocalReloaded.generateMap( "StratIoTbl", mapName, newDepths, mapsSequenceNbr, resultsFile ) )
         {
            throw ErrorHandler::Exception( ErrorHandler::UnknownError ) << " Cannot generate the map with corrected depths for the surface " << nextSurface;
         }

         // Copy the update map file to the master path
         MPI_Barrier( PETSC_COMM_WORLD );
         if ( rank == 0 )
         {
            masterResults.remove( );
            if ( !casePathResults.copyFile( masterResults ) )
            {
               throw ErrorHandler::Exception( ErrorHandler::UnknownError ) << "Cannot copy the result file to " << masterResults.fullPath( ).path( );
            }
         }
         MPI_Barrier( PETSC_COMM_WORLD );
      }  

      //------------------------------------------------------------------------------------------------------------------------------------------------------------------------//
      // The end of the main depth calibration loop
      //------------------------------------------------------------------------------------------------------------------------------------------------------------------------//

      // Reload the original model
      mdl.reset( new mbapi::Model( ) );
      if ( ErrorHandler::NoError != mdl->loadModelFromProjectFile( projectName ) )
      {
         throw ErrorHandler::Exception( ErrorHandler::UnknownError ) << " wrong input file name ";
      }
      MPI_Barrier( PETSC_COMM_WORLD );
      mbapi::StratigraphyManager & strMgrMaster = mdl->stratigraphyManager( );

      // Modify StratIoTbl and GridMapIoTbl with the corrected new maps names
      const std::vector<mbapi::StratigraphyManager::SurfaceID> & surfacesIDFinal = strMgrMaster.surfacesIDs( );
      if ( ErrorHandler::NoError != modifyTables( mdl.get( ), surfacesIDFinal.size( ) - 1, correctedMapsNames, correctedMapsSequenceNbr, correctedMapsIDs ) )
      {
         throw ErrorHandler::Exception( ErrorHandler::UnknownError ) << " Tables could not be modified for the final project";
      }

      // Create the final case
      ibs::FilePath finalProject( finalResultsPath );
      finalProject << mdl->projectFileName( );
      if ( ErrorHandler::NoError != createCase( mdl.get( ), rank, finalResultsPath, finalProject, masterResults, finalResults ) )
      {
         throw ErrorHandler::Exception( ErrorHandler::UnknownError ) << " Could not create or copy the inpt files in the folder " << finalResultsPath.path( ).c_str( );
      }

      // Reload the model so the map manager gets updated (with the correct map sequence numbers)
      mdl.reset( new mbapi::Model( ) );
      if ( ErrorHandler::NoError != mdl->loadModelFromProjectFile( finalProject.path( ).c_str( ) ) )
      {
         throw ErrorHandler::Exception( ErrorHandler::UnknownError ) << " wrong input file name ";
      }
      MPI_Barrier( PETSC_COMM_WORLD );
      mbapi::MapsManager &         mapsMgrFinal  = mdl->mapsManager( );
      mbapi::StratigraphyManager & strMgrFinal   = mdl->stratigraphyManager( );

      // For the surfaces below the endSurface append the "isopacks" previously calculated
      for ( mbapi::StratigraphyManager::SurfaceID s = endSurface + 1; s < surfacesIDFinal.size( ); ++s )
      {
         std::string mapName = "IsoSurface_" + std::to_string( s );
         std::string surfaceName = strMgrFinal.surfaceName( s );
         if ( rank == 0 )
         {
            LogHandler( LogHandler::INFO_SEVERITY ) << " Appending isopack for surface " << s;
         }
         int mapsSequenceNbr = -1;
         for ( size_t i = 0; i != newDepths.size(); ++i )
         {
            isoPacks[s][i] = newDepths[i] + isoPacks[s][i];
         }
         correctedMapsNames[s] = mapName;
         if ( UndefinedIDValue == mapsMgrFinal.generateMap( "StratIoTbl", mapName, isoPacks[s], mapsSequenceNbr, resultsFile ) )
         {
            throw ErrorHandler::Exception( ErrorHandler::UnknownError ) <<  " Cannot generate the map for the iso surface " << s;
         }
         correctedMapsSequenceNbr[s] = mapsSequenceNbr;
         // Modify record with the new name for the depth gris map     
         if ( ErrorHandler::ReturnCode::NoError != mdl->setTableValue( "StratIoTbl", s, "DepthGrid", mapName ) )
         {
            throw ErrorHandler::Exception( ErrorHandler::UnknownError ) << "Cannot set the map " << mapName << " as new depth iso surface in the StratIoTbl";
         }
      }

      // Save the project and input data to folder with the new maps
      MPI_Barrier( PETSC_COMM_WORLD );
      if ( rank == 0 )
      {
         mdl->saveModelToProjectFile( finalProject.path( ).c_str( ), true );
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
//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include <stdafx.h>

#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <cmath>

#if defined(_WIN32) || defined (_WIN64)
#include <direct.h>
#include <time.h>
#endif

#include <algorithm>
#include <cerrno>
#include <cassert>
#include <iostream>
#include <sstream>
#include <vector>
#include <list>
#include <cstring>

#include "database.h"
#include "cauldronschema.h"
#include "cauldronschemafuncs.h"

#include "hdf5.h"
#include "hdf5funcs.h"

#include "EosPack.h"

#include "Interface/ProjectHandle.h"

#include "Interface/AllochthonousLithology.h"
#include "Interface/AllochthonousLithologyDistribution.h"
#include "Interface/AllochthonousLithologyInterpolation.h"
#include "Interface/RunParameters.h"
#include "Interface/ProjectData.h"
#include "Interface/BasementSurface.h"
#include "Interface/BiodegradationParameters.h"
#include "Interface/ConstrainedOverpressureInterval.h"
#include "Interface/DiffusionLeakageParameters.h"
#include "Interface/FluidType.h"
#include "Interface/Formation.h"
#include "Interface/FracturePressureFunctionParameters.h"
#include "Interface/CrustFormation.h"
#include "Interface/InputValue.h"
#include "Interface/IgneousIntrusionEvent.h"
#include "Interface/LithoType.h"
#include "Interface/LithologyHeatCapacitySample.h"
#include "Interface/LithologyThermalConductivitySample.h"
#include "Interface/FluidHeatCapacitySample.h"
#include "Interface/FluidDensitySample.h"
#include "Interface/FluidThermalConductivitySample.h"
#include "Interface/Grid.h"
#include "Interface/GridMap.h"
#include "Interface/MapWriter.h"
#include "Interface/MantleFormation.h"
#include "Interface/MobileLayer.h"
#include "Interface/TouchstoneMap.h"
#include "Interface/ObjectFactory.h"
#include "Interface/OutputProperty.h"
#include "Interface/Parent.h"
#include "Interface/PaleoProperty.h"
#include "Interface/PaleoFormationProperty.h"
#include "Interface/PaleoSurfaceProperty.h"
#include "Interface/PermafrostEvent.h"
#include "Interface/Property.h"
#include "Interface/PropertyValue.h"
#include "Interface/RelatedProject.h"
#include "Interface/Reservoir.h"
#include "Interface/SimulationDetails.h"
#include "Interface/Snapshot.h"
#include "Interface/SourceRock.h"
#include "Interface/Surface.h"
#include "Interface/Trap.h"
#include "Interface/Trapper.h"
#include "Interface/Migration.h"
#include "Interface/FaultCollection.h"
#include "Interface/Faulting.h"
#include "Interface/FaultFileReaderFactory.h"
#include "Interface/FaultFileReader.h"
#include "Interface/IBSFaultFileReader.h"
#include "Interface/LandmarkFaultFileReader.h"
#include "Interface/ZycorFaultFileReader.h"
#include "Interface/PointAdsorptionHistory.h"
#include "Interface/IrreducibleWaterSaturationSample.h"
#include "Interface/LangmuirAdsorptionIsothermSample.h"
#include "Interface/LangmuirAdsorptionTOCEntry.h"
#include "Interface/SGDensitySample.h"
#include "Interface/CrustalThicknessData.h"


#include "errorhandling.h"
#include "wildMatch.h"
#include "array.h"
#include "GenexResultManager.h"
#include "ComponentManager.h"
#include "InterfaceDefs.h"

#include "FilePath.h"

//utilities library
#include "LogHandler.h"


using namespace DataAccess;
using namespace Interface;
using namespace std;

using database::Database;
using database::DataSchema;
using database::Table;
using database::Record;

const double DefaultUndefinedValue = 99999;

typedef formattingexception::GeneralException ProjectHandleException; 

static char * words [] = {"ALCStepBasaltThickness", "ALCStepTopBasaltDepth", "ChemicalCompaction" , "Depth", 
                          "ErosionFactor", "FCTCorrection", "MaxVes",
                          "Pressure", "Temperature", "ThicknessError", "Ves", "Vr" };

DataAccess::Interface::ProjectHandle * DataAccess::Interface::OpenCauldronProject( const string & name, const string & accessMode, ObjectFactory* objectFactory )
{
   Database * tables = CreateDatabaseFromCauldronProject( name );
   if ( tables )
   {
      return objectFactory->produceProjectHandle( tables, name, accessMode );
   }
   else
   {
      return 0;
   }
}

Database * DataAccess::Interface::CreateDatabaseFromCauldronProject( const string & name )
{
   FaultFileReaderFactory::getInstance().registerReader( IBSFaultFileReaderID, allocateIBSFaultFileReader );
   FaultFileReaderFactory::getInstance().registerReader( LandmarkFaultFileReaderID, allocateLandmarkFaultFileReader );
   FaultFileReaderFactory::getInstance().registerReader( ZyCorFaultFileReaderID, allocateZyCorFaultFileReader );

   DataSchema * cauldronSchema = database::createCauldronSchema();
   database::TableDefinition *tableDef = cauldronSchema->getTableDefinition( "DepthIoTbl" );

   if ( tableDef )
   {
      // Adding (volatile, won't be output) definition for DepositionSequence field
      // Required to properly sort the DepthIoTbl, not to be output
      tableDef->addVolatileFieldDefinition( "DepositionSequence", datatype::Int, "", "0" );
   }

   Database * tables = Database::CreateFromFile( name, *cauldronSchema );
   delete cauldronSchema;
   return tables;
}

void DataAccess::Interface::CloseCauldronProject( DataAccess::Interface::ProjectHandle * projectHandle )
{
   if ( projectHandle ) delete projectHandle;
}

int Interface::ProjectHandle::GetNumberOfSpecies( void )
{
   return CBMGenerics::ComponentManager::NumberOfOutputSpecies;
}

std::string Interface::ProjectHandle::GetSpeciesName( int i )
{
   CBMGenerics::ComponentManager & theComponentManager = CBMGenerics::ComponentManager::getInstance();
   return theComponentManager.GetSpeciesName( i );
}

const DataAccess::Interface::MessageHandler& ProjectHandle::getMessageHandler() const {
   return *m_messageHandler;
}

const DataAccess::Interface::ApplicationGlobalOperations& ProjectHandle::getGlobalOperations() const {
   return *m_globalOperations;
}

ProjectHandle::ProjectHandle( Database * tables, const string & name, const string & accessMode, ObjectFactory* objectFactory ) :
   m_database( tables ), m_name( name ), m_accessMode( READWRITE ), m_activityOutputGrid( 0 ), m_mapPropertyValuesWriter( 0 ), m_primaryList( words, words + 12 )
{
   (void) accessMode; // ignore warning about unused parameter

   
   m_messageHandler = 0;
   m_globalOperations = 0;

   m_factory = objectFactory;

   m_rank = ddd::GetRank();
   m_size = ddd::GetSize();
   mapFileCacheConstructor();
   allocateArchitectureRelatedParameters();

   m_currentSnapshot = 0;
   m_currentProperty = 0;

   m_inputGrid = 0;
   m_lowResOutputGrid = 0;
   m_highResOutputGrid = 0;

   m_biodegradationParameters = 0;
   m_fracturePressureFunctionParameters = 0;
   m_diffusionLeakageParameters = 0;

   m_runParameters = 0;
   m_crustFormation = 0;
   m_mantleFormation = 0;
   m_projectData = 0;

   m_sgDensitySample = 0;
   m_irreducibleWaterSample = 0;

   m_primaryDouble = false;

   //1DComponent
   loadModellingMode();

   splitName();

   loadSnapshots();
   loadProperties();
   loadTimeOutputProperties();
   loadDepthOutputProperties();

   loadLithologyHeatCapacitySamples();
   loadLithologyThermalConductivitySamples();

   loadBottomBoundaryConditions();
   loadLithoTypes();
#if 0
   loadSourceRocks ();
#endif
   loadGrids();
   loadSurfaces();
   loadFormations();
   loadReservoirs();
   loadMobileLayers();
   loadTouchstoneMaps();
   loadAllochthonousLithologies();
   loadAllochthonousLithologyDistributions();
   loadAllochthonousLithologyInterpolations();
   loadTraps();
   loadTrappers();
   loadMigrations();
   loadInputValues();
   numberInputValues();
   loadFluidTypes();

#if 0
   loadLithologyHeatCapacitySamples ();
   loadLithologyThermalConductivitySamples ();
#endif

   loadFluidDensitySamples();
   loadFluidThermalConductivitySamples();
   loadFluidHeatCapacitySamples();

   //   loadBottomBoundaryConditions ();
   loadCrustFormation();
   loadMantleFormation();
   loadBasementSurfaces();


   connectSurfaces();
   connectReservoirs();
   connectTraps();
   if ( trappersAreAvailable() )
   {
      connectTrappers();
      connectMigrations();
      connectUpAndDownstreamTrappers();
   }
   
   loadIgneousIntrusions();
   loadFaults();

   /*
   loadCrustThinningHistory ();
   loadMantleThicknessHistory ();
   loadHeatFlowHistory ();
   loadConstrainedOverpressureIntervals ();
   */

   loadRelatedProjects();

   loadMapPropertyValues();
   loadVolumePropertyValues();
   loadBiodegradationParameters();
   loadDiffusionLeakageParameters();

   loadRunParameters();
   loadSimulationDetails ();

   // Depends on the run parameters.
   loadFracturePressureFunctionParameters();
   loadProjectData();
   loadSurfaceDepthHistory();
   loadSurfaceTemperatureHistory();

   loadLangmuirIsotherms();
   loadLangmuirTOCEntries();
   loadPointHistories();
   loadIrreducibleWaterSaturationSample();
   loadSGDensitySample();

   loadCrustalThicknessData();
   loadPermafrostData();
}

int ProjectHandle::getRank() const {
   return m_rank;
}

int ProjectHandle::getSize() const {
   return m_size;
}

//1DComponent
bool ProjectHandle::loadModellingMode( void )
{
   database::Table * projectIoTbl = 0;

   // try to get it from the ProjectIoTbl
   projectIoTbl = getTable( "ProjectIoTbl" );
   if ( !projectIoTbl )
      return false;

   Record *projectIoRecord = projectIoTbl->getRecord( 0 );

   assert( projectIoRecord );

   const string & theModellingMode = database::getModellingMode( projectIoRecord );

   if ( "3d" == theModellingMode )
   {
      m_modellingMode = Interface::MODE3D; //declared in Interface.h 
   }
   else if ( "1d" == theModellingMode )
   {
      m_modellingMode = Interface::MODE1D; //declared in Interface.h 
   }
   else
   {
      cout << "!!Warning!! Modelling mode was not set in table ProjectIoTbl. Setting Modelling mode to 3D..." << endl;
      m_modellingMode = Interface::MODE3D;
   }
   return true;
}

Interface::ModellingMode ProjectHandle::getModellingMode( void ) const
{
   return m_modellingMode;
}


ObjectFactory * ProjectHandle::getFactory( void ) const
{
   return m_factory;
}

ProjectHandle::~ProjectHandle( void )
{
   mapFileCacheDestructor();
   if ( m_inputGrid ) delete m_inputGrid;
   if ( m_lowResOutputGrid ) delete m_lowResOutputGrid;
   if ( m_highResOutputGrid ) delete m_highResOutputGrid;

   if ( m_messageHandler != 0 ) {
      delete m_messageHandler;
   }

   if ( m_globalOperations != 0 ) {
      delete m_globalOperations;
   }

   deleteSnapshots();
   deleteLithoTypes();
   deleteSurfaces();
   deleteFormations();
   deleteIgneousIntrusions();
   deleteSourceRocks();
   deleteReservoirs();
   deleteMobileLayers();
   deleteTouchstoneMaps();
   deleteAllochthonousLithologies();
   deleteAllochthonousLithologyDistributions();
   deleteAllochthonousLithologyInterpolations();
   deleteTraps();
   deleteInputValues();
   deleteProperties();
   deletePropertyValues();
   deleteFluidTypes();
   deleteCrustalThicknessData();

   deleteBiodegradationParameters();
   deleteFracturePressureFunctionParameters();
   deleteDiffusionLeakageParameters();

   deleteHeatFlowHistory();
   deleteCrustThinningHistory();
   deleteRunParameters();
   deleteProjectData();
   deleteSurfaceDepthHistory();
   deleteSurfaceTemperatureHistory();
   deleteRelatedProjects();

   deleteTimeOutputProperties();
   deleteDepthOutputProperties();

   deleteLithologyHeatCapacitySamples();
   deleteLithologyThermalConductivitySamples();
   deleteFluidDensitySamples();
   deleteFluidThermalConductivitySamples();
   deleteFluidHeatCapacitySamples();

   deleteLangmuirIsotherms();
   deleteLangmuirTOCEntries();
   deletePointHistories();
   deleteIrreducibleWaterSaturationSample();
   deleteSGDensitySample();
   deletePermafrost();

   if ( m_database ) delete m_database;
}


/// split the filename into a directory path and a directory entry
/// the directory path we will use to access map files.
void ProjectHandle::splitName( void )
{
   ibs::FilePath ppath( m_name );

   if ( ppath.size() > 0 ) // number path elements more the one
   {
      m_projectPath = ppath.filePath();
      m_fileName = ppath.fileName();
   }
   else
   {
      m_projectPath = ".";
      m_fileName = m_name;
   }

   m_projectName = m_fileName;
   string::size_type dotPos = m_projectName.rfind( ".project" );
   if ( dotPos != string::npos )
   {
      m_projectName.erase( dotPos, string::npos );
   }
}

bool ProjectHandle::saveToFile( const string & fileName )
{
   // #ifdef DISTRIBUTED
   //    if (getRank () == 0)
   // #endif
   if ( getRank() == 0 )
   {
      m_database->saveToFile( fileName );
   }

   return true;
}

const string & ProjectHandle::getName( void ) const
{
   return m_name;
}

const string & ProjectHandle::getProjectPath( void ) const
{
   return m_projectPath;
}

const string & ProjectHandle::getProjectName( void ) const
{
   return m_projectName;
}

const string & ProjectHandle::getFileName( void ) const
{
   return m_fileName;
}

bool ProjectHandle::startActivity( const string & name, const Interface::Grid * grid, bool saveAsInputGrid, bool createResultsFile, bool append )
{
   char * svnRevision = "unknown";

#ifdef SVNREVISION
   svnRevision = SVNREVISION;
#endif

   if ( getRank() == 0 )
   {
      cerr << endl << "Activity: " << name << ", Revision: " << svnRevision << endl << endl;
   }

   checkForValidPartitioning( name, grid->numIGlobal(), grid->numJGlobal() ); // NOOP in case of serial data access

   if ( getActivityName() != "" || name == "" ) return false;
   if ( !setActivityOutputGrid( ( const Interface::Grid * ) grid ) ) return false;

   setActivityName( name );
   m_saveAsInputGrid = saveAsInputGrid;

   bool status = true;

   if ( createResultsFile ) {
      status = initializeMapPropertyValuesWriter( append );
   }

   return status;
}

bool ProjectHandle::abortActivity( void )
{
   return finishActivity( false );
}

bool ProjectHandle::finishActivity( bool isComplete )
{
   if ( isComplete ) {
      continueActivity();
   }
   if ( getActivityName() == "" || getActivityOutputGrid() == 0 )
   {
      resetActivityName();
      resetActivityOutputGrid();

      return false;
   }
   else
   {
      finalizeMapPropertyValuesWriter();

      resetActivityName();
      resetActivityOutputGrid();

      return true;
   }
}

bool ProjectHandle::restartActivity( void )
{

   if ( getActivityName() == "" || getActivityOutputGrid() == 0 )
   {
      return false;
   }
   else if ( getModellingMode() == Interface::MODE1D )
   {
      return true;
   }
   else
   {
      const std::string& directoryName = getOutputDir();
      // create hdf file
      string fileName = getActivityName();

      fileName += "_Results.HDF";
      ibs::FilePath filePathName( getProjectPath() );
      filePathName << directoryName << fileName;

      m_mapPropertyValuesWriter->close();
      m_mapPropertyValuesWriter->open( filePathName.path(), false );
      m_mapPropertyValuesWriter->saveDescription( getActivityOutputGrid() );

      saveCreatedMapPropertyValues();		/// creates new TimeIoRecords

      return true;
   }
}

bool ProjectHandle::continueActivity( void )
{
   if ( getActivityName() == "" || getActivityOutputGrid() == 0 ) return false;

   saveCreatedMapPropertyValues();		/// creates new TimeIoRecords

   saveCreatedVolumePropertyValues();

   return true;
}

void ProjectHandle::resetActivityName( void )
{
   m_activityName = "";
}

bool ProjectHandle::setActivityName( const string & name )
{
   if ( name == "" ) return false;

   m_activityName = name;
   return true;
}

const string & ProjectHandle::getActivityName( void ) const
{
   return m_activityName;
}

bool ProjectHandle::saveAsInputGrid( void ) const
{
   return m_saveAsInputGrid;
}

void ProjectHandle::resetActivityOutputGrid( void )
{
   m_activityOutputGrid = 0;
}

bool ProjectHandle::setActivityOutputGrid( const Grid * grid )
{
   if ( grid != getLowResolutionOutputGrid() && grid != getHighResolutionOutputGrid() )
   {
      m_activityOutputGrid = 0;
      return false;
   }
   else
   {
      m_activityOutputGrid = grid;
      return true;
   }
}

const Grid * ProjectHandle::getActivityOutputGrid( void ) const
{
   return m_activityOutputGrid;
}

Database * ProjectHandle::getDataBase( void ) const
{
   return m_database;
}

database::Table * ProjectHandle::getTable( const string & tableName ) const
{
   return m_database->getTable( tableName );
}

bool ProjectHandle::loadSnapshots( void )
{
   database::Table* snapshotTbl = getTable( "SnapshotIoTbl" );
   database::Table::iterator tblIter;
   if ( snapshotTbl->size() == 0 )
   {
      createSnapshotsAtGeologicalEvents();
   }

   for ( tblIter = snapshotTbl->begin(); tblIter != snapshotTbl->end(); ++tblIter )
   {
      Record * snapshotRecord = *tblIter;
      m_snapshots.push_back( getFactory()->produceSnapshot( this, snapshotRecord ) );
   }

   std::sort( m_snapshots.begin(), m_snapshots.end(), SnapshotLessThan() );

   Table * tbl = getTable( "BasementIoTbl" );
   assert( tbl );
   Record *firstRecord = tbl->getRecord( 0 );
   assert( firstRecord );

   if ( getBottomBoundaryModel( firstRecord ) == "Advanced Lithosphere Calculator" ) {
      createSnapshotsAtRiftEvents();
   }
   return true;
}

bool ProjectHandle::createSnapshotsAtRiftEvents()
{
   const string tableName = getCrustIoTableName();
   unsigned int  i;

   //initialize start age and list of geological events
   double startAge = -1.0;
   std::list<double> geologicalEventAges;

   // Add events from the StratIoTbl
   Table * tbl = getTable( "StratIoTbl" );
   assert( tbl );

   for ( i = 0; i < tbl->size(); i++ )
   {
      Record *record = tbl->getRecord( i );
      assert( record );

      // Add DepoAge
      double depoAge = getDepoAge( record );
      if ( depoAge != 0.0 )
      {
         startAge = max( startAge, depoAge );
      }
   }

   tbl = getTable( tableName );
   assert( tbl );

   for ( i = 0; i < tbl->size(); i++ )
   {
      Record *record = tbl->getRecord( i );
      assert( record );

      double age = getAge( record );
      if ( age < startAge )
      {
         geologicalEventAges.push_back( age );
      }
   }
   //sort and remove duplicates
   geologicalEventAges.sort();
   geologicalEventAges.unique( isEqualTime );

   std::list<double>::const_iterator iter;

   //add sorted entries to SnapshotIoTbl
   database::Table *snapshotIoTbl = getTable( "SnapshotIoTbl" );
   assert( snapshotIoTbl );
   const double tolerance = 1e-6;

   for ( iter = geologicalEventAges.begin(); iter != geologicalEventAges.end(); ++iter )
   {
      const Snapshot * snapshot = findSnapshot( *iter, MAJOR );
      if ( snapshot == 0 ||
         not( snapshot->getTime() >= *iter - tolerance && snapshot->getTime() <= *iter + tolerance ) ) {

         Record* record = snapshotIoTbl->createRecord();

         setTime( record, *iter );
         setIsMinorSnapshot( record, 0 );
         setTypeOfSnapshot( record, "System Generated" );
         setSnapshotFileName( record, "" );

         m_snapshots.push_back (getFactory ()->produceSnapshot (this, record));
      }
   }
   std::sort ( m_snapshots.begin (), m_snapshots.end (), SnapshotLessThan ());

   return true;
}

bool ProjectHandle::createSnapshotsAtGeologicalEvents()
{

   unsigned int  i;

   //initialize start age and list of geological events
   double startAge = -1.0;
   std::list<double> geologicalEventAges;

   // Add events from the StratIoTbl
   database::Table *tbl = getTable( "StratIoTbl" );
   assert( tbl );

   for ( i = 0; i < tbl->size(); i++ )
   {
      Record *record = tbl->getRecord( i );
      assert( record );

      // Add DepoAge
      double depoAge = getDepoAge( record );
      if ( depoAge != 0.0 )
      {
         startAge = max( startAge, depoAge );
         geologicalEventAges.push_back( depoAge );
      }
   }

   //add all names of tables from which ages are to be read in a list
   std::list<string> tableNameList;
   tableNameList.push_back( "SurfaceDepthIoTbl" );
   tableNameList.push_back( "MobLayThicknIoTbl" );
   tableNameList.push_back( "SurfaceTempIoTbl" );

   // add table name "CrustIoTbl" or "MntlHeatFlowIoTbl" to list? Depends on BasementIoTbl! 
   tbl = getTable( "BasementIoTbl" );
   assert( tbl );

   Record *firstRecord = tbl->getRecord( 0 );
   assert( firstRecord );

   bool isFixedTemperature = ( getBottomBoundaryModel( firstRecord ) == "Fixed Temperature" );
   bool isALC = ( getBottomBoundaryModel( firstRecord ) == "Advanced Lithosphere Calculator" );

   const string tableName = ( ( isFixedTemperature || isALC ) ? getCrustIoTableName() : "MntlHeatFlowIoTbl" );

   tableNameList.push_back( tableName );

   //loop over all gathered tables: get ages of entries
   std::list<string>::const_iterator tableNameIter;

   for ( tableNameIter = tableNameList.begin(); tableNameIter != tableNameList.end(); tableNameIter++ )
   {
      tbl = getTable( *tableNameIter );
      assert( tbl );

      for ( i = 0; i < tbl->size(); i++ )
      {
         Record *record = tbl->getRecord( i );
         assert( record );

         double age = getAge( record );
         if ( age < startAge )
         {
            geologicalEventAges.push_back( age );
         }
      }
   }

   // Add events from the GeologicalBoundaryIoTbl

   database::Table* geologicalBoundaryIoTbl = getTable( "GeologicalBoundaryIoTbl" );
   database::Table* propertyBoundaryIoTbl = getTable( "PropertyBoundaryIoTbl" );
   database::Table* boundaryValuesIoTbl = getTable( "BoundaryValuesIoTbl" );

   assert( geologicalBoundaryIoTbl );
   assert( propertyBoundaryIoTbl );
   assert( boundaryValuesIoTbl );

   for ( i = 0; i < geologicalBoundaryIoTbl->size(); i++ )
   {
      Record* geologRecord = geologicalBoundaryIoTbl->getRecord( i );
      assert( geologRecord );

      int firstPropInd = getFirstIndexInPropertyIoTbl( geologRecord ) - 1; //the old C-indexing issue...
      int nProp = getNumberOfProperties( geologRecord );

      for ( int j = 0; j < nProp; ++j )
      {
         Record* propRecord = propertyBoundaryIoTbl->getRecord( j + firstPropInd );
         assert( propRecord );

         int firstBoundInd = getFirstIndexInPropertyBoundaryValuesIoTbl( propRecord ) - 1; //the old C-indexing issue...
         int nIntervals = getNumberOfTimeIntervals( propRecord );

         for ( int k = 0; k < nIntervals; ++k )
         {
            Record* boundRecord = boundaryValuesIoTbl->getRecord( k + firstBoundInd );
            assert( boundRecord );

            double beginTime = getBeginTimeValues( boundRecord );
            double endTime = getEndTimeValues( boundRecord );

            if ( beginTime < startAge )
            {
               geologicalEventAges.push_back( beginTime );
            }
            if ( endTime < startAge )
            {
               geologicalEventAges.push_back( endTime );
            }
         }
      }
   }

   //make sure present day's age is included as well
   geologicalEventAges.push_back( 0.0 );

   //sort and remove duplicates
   geologicalEventAges.sort();
   geologicalEventAges.unique( isEqualTime );

   std::list<double>::const_iterator iter;

   //add sorted entries to SnapshotIoTbl
   database::Table *snapshotIoTbl = getTable( "SnapshotIoTbl" );
   assert( snapshotIoTbl );

   for ( iter = geologicalEventAges.begin(); iter != geologicalEventAges.end(); ++iter )
   {
      Record* record = snapshotIoTbl->createRecord();

      setTime( record, *iter );
      setIsMinorSnapshot( record, 0 );
      setTypeOfSnapshot( record, "System Generated" );
      setSnapshotFileName( record, "" );
   }

   return true;
}

bool ProjectHandle::isEqualTime( double t1, double t2 )
{
   return ( std::fabs( t1 - t2 ) < 1e-5 );
}

bool ProjectHandle::loadGrids( void )
{












   // (void) getHighResolutionOutputGrid();
   // (void) getLowResolutionOutputGrid();
   // (void) getInputGrid();

   return true;
}

bool ProjectHandle::loadSurfaces( void )
{
   database::Table * stratTbl = getTable( "StratIoTbl" );
   database::Table::iterator tblIter;

   for ( tblIter = stratTbl->begin(); tblIter != stratTbl->end(); ++tblIter )
   {
      Record * stratRecord = *tblIter;
      m_surfaces.push_back( getFactory()->produceSurface( this, stratRecord ) );
   }

   // Sort the list of surfaces into age order, youngest first, oldest last on the list.
   std::sort( m_surfaces.begin(), m_surfaces.end(), SurfaceLessThan() );

   return true;
}

/// Actually, the properties are created here, hardwired.
bool ProjectHandle::loadProperties( void )
{
   using Interface::FORMATIONPROPERTY;
   using Interface::RESERVOIRPROPERTY;
   using Interface::TRAPPROPERTY;

   // fastcauldron properties
   m_properties.push_back( getFactory()->produceProperty( this, 0, "ALCOrigLithMantleDepth",         "ALCOrigLithMantleDepth",         "m",     FORMATIONPROPERTY, DataModel::SURFACE_2D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "ALCMaxAsthenoMantleDepth",       "ALCMaxAsthenoMantleDepth",       "m",     FORMATIONPROPERTY, DataModel::SURFACE_2D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "ALCStepContCrustThickness",      "ALCStepContCrustThickness",      "m",     FORMATIONPROPERTY, DataModel::SURFACE_2D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "ALCStepBasaltThickness",         "ALCStepBasaltThickness",         "m",     FORMATIONPROPERTY, DataModel::FORMATION_2D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "ALCStepTopBasaltDepth",          "ALCStepTopBasaltDepth",          "m",     FORMATIONPROPERTY, DataModel::FORMATION_2D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "ALCSmContCrustThickness",        "ALCSmContCrustThickness",        "m",     FORMATIONPROPERTY, DataModel::SURFACE_2D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "ALCSmBasaltThickness",           "ALCSmBasaltThickness",           "m",     FORMATIONPROPERTY, DataModel::SURFACE_2D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "ALCSmTopBasaltDepth",            "ALCSmTopBasaltDepth",            "m",     FORMATIONPROPERTY, DataModel::SURFACE_2D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "ALCSmMohoDepth",                 "ALCSmMohoDepth",                 "m",     FORMATIONPROPERTY, DataModel::SURFACE_2D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "ALCStepMohoDepth",               "ALCStepMohoDepth",               "m",     FORMATIONPROPERTY, DataModel::SURFACE_2D_PROPERTY ));

   m_properties.push_back( getFactory()->produceProperty( this, 0, "AllochthonousLithology",         "AllochthonousLithology",         "",      FORMATIONPROPERTY, DataModel::FORMATION_2D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "CapillaryEntryPressureGas",      "CapillaryEntryPressureGas",      "Pa",    FORMATIONPROPERTY, DataModel::DISCONTINUOUS_3D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "CapillaryEntryPressureOil",      "CapillaryEntryPressureOil",      "Pa",    FORMATIONPROPERTY, DataModel::DISCONTINUOUS_3D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "ChemicalCompaction",             "ChemicalCompaction",             "frac",  FORMATIONPROPERTY, DataModel::DISCONTINUOUS_3D_PROPERTY));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "BulkDensity",                    "BulkDensityVec2",                "kg/m3", FORMATIONPROPERTY, DataModel::DISCONTINUOUS_3D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "Depth",                          "Depth",                          "m",     FORMATIONPROPERTY, DataModel::CONTINUOUS_3D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "DepthHighRes",                   "DepthHighRes",                   "m",     FORMATIONPROPERTY, DataModel::CONTINUOUS_3D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "Diffusivity",                    "DiffusivityVec2",                "m2/s",  FORMATIONPROPERTY, DataModel::DISCONTINUOUS_3D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "ErosionFactor",                  "ErosionFactor",                  "",      FORMATIONPROPERTY, DataModel::FORMATION_2D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "FaultElements",                  "FaultElements",                  "",      FORMATIONPROPERTY, DataModel::FORMATION_2D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "FCTCorrection",                  "FCTCorrection",                  "%",     FORMATIONPROPERTY, DataModel::FORMATION_2D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "FlowDirectionIJK",               "FlowDirectionIJK",               "",      FORMATIONPROPERTY, DataModel::DISCONTINUOUS_3D_PROPERTY));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "FlowDirectionI",                 "FlowDirectionI",                 "",      FORMATIONPROPERTY, DataModel::DISCONTINUOUS_3D_PROPERTY));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "FlowDirectionJ",                 "FlowDirectionJ",                 "",      FORMATIONPROPERTY, DataModel::DISCONTINUOUS_3D_PROPERTY));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "FlowDirectionK",                 "FlowDirectionK",                 "",      FORMATIONPROPERTY, DataModel::DISCONTINUOUS_3D_PROPERTY));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "FlowDirectionX",                 "FlowDirectionX",                 "m",     FORMATIONPROPERTY, DataModel::DISCONTINUOUS_3D_PROPERTY));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "FlowDirectionY",                 "FlowDirectionY",                 "m",     FORMATIONPROPERTY, DataModel::DISCONTINUOUS_3D_PROPERTY));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "FlowDirectionZ",                 "FlowDirectionZ",                 "m",     FORMATIONPROPERTY, DataModel::DISCONTINUOUS_3D_PROPERTY));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "FluidVelocity",                  "FluidVelocity",                  "mm/y",  FORMATIONPROPERTY, DataModel::DISCONTINUOUS_3D_PROPERTY));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "FluidVelocityX",                 "FluidVelocityX",                 "mm/y",  FORMATIONPROPERTY, DataModel::DISCONTINUOUS_3D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "FluidVelocityY",                 "FluidVelocityY",                 "mm/y",  FORMATIONPROPERTY, DataModel::DISCONTINUOUS_3D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "FluidVelocityZ",                 "FluidVelocityZ",                 "mm/y",  FORMATIONPROPERTY, DataModel::DISCONTINUOUS_3D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "HeatFlow",                       "HeatFlow",                       "mW/m2", FORMATIONPROPERTY, DataModel::DISCONTINUOUS_3D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "HeatFlowX",                      "HeatFlowX",                      "mW/m2", FORMATIONPROPERTY, DataModel::DISCONTINUOUS_3D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "HeatFlowY",                      "HeatFlowY",                      "mW/m2", FORMATIONPROPERTY, DataModel::DISCONTINUOUS_3D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "HeatFlowZ",                      "HeatFlowZ",                      "mW/m2", FORMATIONPROPERTY, DataModel::DISCONTINUOUS_3D_PROPERTY ));

   // not sure which attribute this property shoudl have, so give it the most general one
   m_properties.push_back( getFactory()->produceProperty( this, 0, "HopaneIsomerisation",            "HopaneIsomerisation",            "",      FORMATIONPROPERTY, DataModel::DISCONTINUOUS_3D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "HorizontalPermeability",         "HorizontalPermeability",         "mD",    FORMATIONPROPERTY, DataModel::DISCONTINUOUS_3D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "HydroStaticPressure",            "HydroStaticPressure",            "MPa",   FORMATIONPROPERTY, DataModel::CONTINUOUS_3D_PROPERTY ));

   // not sure which attribute this property shoudl have, so give it the most general one
   m_properties.push_back( getFactory()->produceProperty( this, 0, "IlliteFraction",                 "IlliteFraction",                 "",      FORMATIONPROPERTY, DataModel::DISCONTINUOUS_3D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "Lithology",                      "Lithology",                      "",      FORMATIONPROPERTY, DataModel::FORMATION_2D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "LithoStaticPressure",            "LithoStaticPressure",            "MPa",   FORMATIONPROPERTY, DataModel::CONTINUOUS_3D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "MaxVesHighRes",                  "MaxVesHighRes",                  "Pa",    FORMATIONPROPERTY, DataModel::DISCONTINUOUS_3D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "MaxVes",                         "MaxVesVec2",                     "Pa",    FORMATIONPROPERTY, DataModel::DISCONTINUOUS_3D_PROPERTY ));
   m_properties.push_back(getFactory ()->produceProperty( this, 0, "Overburden",                     "Overburden",                     "m",     FORMATIONPROPERTY, DataModel::CONTINUOUS_3D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "OverPressure",                   "OverPressure",                   "MPa",   FORMATIONPROPERTY, DataModel::CONTINUOUS_3D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "PermeabilityH",                  "PermeabilityHVec2",              "mD",    FORMATIONPROPERTY, DataModel::DISCONTINUOUS_3D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "Permeability",                   "PermeabilityVec2",               "mD",    FORMATIONPROPERTY, DataModel::DISCONTINUOUS_3D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "Porosity",                       "PorosityVec2",                   "vol%",  FORMATIONPROPERTY, DataModel::DISCONTINUOUS_3D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "Pressure",                       "Pressure",                       "MPa",   FORMATIONPROPERTY, DataModel::CONTINUOUS_3D_PROPERTY ));
   
   // Reflectivity should be DataModel::SURFACE_2D_PROPERTY.
   // Currently fastcauldron outputs both surface and a volume dataset for this property.
   // The Volume data is largely filled with the null-value (99999) except at the surface.
   m_properties.push_back( getFactory()->produceProperty( this, 0, "Reflectivity",                   "ReflectivityVec2",               "",      FORMATIONPROPERTY, DataModel::SURFACE_2D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "SonicSlowness",                  "SonicVec2",                      "us/m",  FORMATIONPROPERTY, DataModel::DISCONTINUOUS_3D_PROPERTY ));

   // not sure which attribute this property shoudl have, so give it the most general one
   m_properties.push_back( getFactory()->produceProperty( this, 0, "SteraneAromatisation",           "SteraneAromatisation",           "",      FORMATIONPROPERTY, DataModel::DISCONTINUOUS_3D_PROPERTY ));

   // not sure which attribute this property shoudl have, so give it the most general one
   m_properties.push_back( getFactory()->produceProperty( this, 0, "SteraneIsomerisation",           "SteraneIsomerisation",           "",      FORMATIONPROPERTY, DataModel::DISCONTINUOUS_3D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "Temperature",                    "Temperature",                    "C",     FORMATIONPROPERTY, DataModel::CONTINUOUS_3D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "ThCond",                         "ThCondVec2",                     "W/mK",  FORMATIONPROPERTY, DataModel::DISCONTINUOUS_3D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "ThicknessError",                 "ThicknessError",                 "m",     FORMATIONPROPERTY, DataModel::FORMATION_2D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "ThicknessHighRes",               "ThicknessHighRes",               "m",     FORMATIONPROPERTY, DataModel::FORMATION_2D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "Thickness",                      "Thickness",                      "m",     FORMATIONPROPERTY, DataModel::FORMATION_2D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "TwoWayTime",                     "TwoWayTime",                     "ms",    FORMATIONPROPERTY, DataModel::CONTINUOUS_3D_PROPERTY ) );
   m_properties.push_back( getFactory()->produceProperty( this, 0, "TwoWayTimeResidual",             "TwoWayTimeResidual",             "ms",    FORMATIONPROPERTY, DataModel::SURFACE_2D_PROPERTY ) );
   m_properties.push_back( getFactory()->produceProperty( this, 0, "Velocity",                       "VelocityVec2",                   "m/s",   FORMATIONPROPERTY, DataModel::DISCONTINUOUS_3D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "VesHighRes",                     "VesHighRes",                     "Pa",    FORMATIONPROPERTY, DataModel::CONTINUOUS_3D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "Ves",                            "Ves",                            "Pa",    FORMATIONPROPERTY, DataModel::CONTINUOUS_3D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "Vre",                            "Vre",                            "%",     FORMATIONPROPERTY, DataModel::DISCONTINUOUS_3D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "Vr",                             "VrVec2",                         "%",     FORMATIONPROPERTY, DataModel::DISCONTINUOUS_3D_PROPERTY ));

   m_properties.push_back( getFactory()->produceProperty( this, 0, "SourceRockEndMember1",           "SourceRockEndMember1",           "%",     FORMATIONPROPERTY, DataModel::FORMATION_2D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "SourceRockEndMember2",           "SourceRockEndMember2",           "%",     FORMATIONPROPERTY, DataModel::FORMATION_2D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "TOC",                            "TOC",                            "%",     FORMATIONPROPERTY, DataModel::FORMATION_2D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "PorosityLossFromPyroBitumen",    "PorosityLossFromPyroBitumen",    "%",     FORMATIONPROPERTY, DataModel::SURFACE_2D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "H2SRisk",                        "H2SRisk",                        "kg/m2", FORMATIONPROPERTY, DataModel::SURFACE_2D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "HcVapourDensity",                "HcVapourDensity",                "kg/m3", FORMATIONPROPERTY, DataModel::DISCONTINUOUS_3D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "HcLiquidDensity",                "HcLiquidDensity",                "kg/m3", FORMATIONPROPERTY, DataModel::DISCONTINUOUS_3D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "BrineDensity",                   "BrineDensity",                   "kg/m3", FORMATIONPROPERTY, DataModel::DISCONTINUOUS_3D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "HcVapourViscosity",              "HcVapourViscosity",              "Pa.s",  FORMATIONPROPERTY, DataModel::DISCONTINUOUS_3D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "HcLiquidViscosity",              "HcLiquidViscosity",              "Pa.s",  FORMATIONPROPERTY, DataModel::DISCONTINUOUS_3D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "BrineViscosity",                 "BrineViscosity",                 "Pa.s",  FORMATIONPROPERTY, DataModel::DISCONTINUOUS_3D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "BrineSaturation",                "BrineSaturation",                "frac",  FORMATIONPROPERTY, DataModel::DISCONTINUOUS_3D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "HcLiquidSaturation",             "HcLiquidSaturation",             "frac",  FORMATIONPROPERTY, DataModel::DISCONTINUOUS_3D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "HcVapourSaturation",             "HcVapourSaturation",             "frac",  FORMATIONPROPERTY, DataModel::DISCONTINUOUS_3D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "HcLiquidBrineCapillaryPressure", "HcLiquidBrineCapillaryPressure", "Pa",    FORMATIONPROPERTY, DataModel::DISCONTINUOUS_3D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "HcVapourBrineCapillaryPressure", "HcVapourBrineCapillaryPressure", "Pa",    FORMATIONPROPERTY, DataModel::DISCONTINUOUS_3D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "GOR",                            "GOR",                            "m3/m3", FORMATIONPROPERTY, DataModel::DISCONTINUOUS_3D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "CGR",                            "CGR",                            "m3/m3", FORMATIONPROPERTY, DataModel::DISCONTINUOUS_3D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "OilAPI",                         "OilAPI",                         "",      FORMATIONPROPERTY, DataModel::DISCONTINUOUS_3D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "CondensateAPI",                  "CondensateAPI",                  "",      FORMATIONPROPERTY, DataModel::DISCONTINUOUS_3D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "TimeOfInvasion",                 "TimeOfInvasion",                 "frac",  FORMATIONPROPERTY, DataModel::DISCONTINUOUS_3D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "ImmobileSaturation",             "ImmobileSaturation",             "frac",  FORMATIONPROPERTY, DataModel::DISCONTINUOUS_3D_PROPERTY ));
   // m_properties.push_back( getFactory()->produceProperty( this, 0, "AverageBrineSaturation",         "AverageBrineSaturation",         "frac", FORMATIONPROPERTY ));
   // m_properties.push_back( getFactory()->produceProperty( this, 0, "AverageHcLiquidSaturation",      "AverageHcLiquidSaturation",      "frac", FORMATIONPROPERTY ));
   // m_properties.push_back( getFactory()->produceProperty( this, 0, "AverageHcVapourSaturation",      "AverageHcVapourSaturation",      "frac", FORMATIONPROPERTY ));
   // m_properties.push_back( getFactory()->produceProperty( this, 0, "AverageImmobileSaturation",      "AverageImmobileSaturation",      "frac", FORMATIONPROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "HcVapourVolume",                 "HcVapourVolume",                 "m3",    FORMATIONPROPERTY, DataModel::DISCONTINUOUS_3D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "HcLiquidVolume",                 "HcLiquidVolume",                 "m3",    FORMATIONPROPERTY, DataModel::DISCONTINUOUS_3D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "ElementVolume",                  "ElementVolume",                  "m3",    FORMATIONPROPERTY, DataModel::DISCONTINUOUS_3D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "ElementPoreVolume",              "ElementPoreVolume",              "m3",    FORMATIONPROPERTY, DataModel::DISCONTINUOUS_3D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "BrineRelativePermeability",      "BrineRelativePermeability",      "frac",  FORMATIONPROPERTY, DataModel::DISCONTINUOUS_3D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "HcLiquidRelativePermeability",   "HcLiquidRelativePermeability",   "frac",  FORMATIONPROPERTY, DataModel::DISCONTINUOUS_3D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "HcVapourRelativePermeability",   "HcVapourRelativePermeability",   "frac",  FORMATIONPROPERTY, DataModel::DISCONTINUOUS_3D_PROPERTY ));

   m_properties.push_back( getFactory()->produceProperty( this, 0, "HcVapourVelocityX",              "HcVapourVelocityX",              "mm/y",  FORMATIONPROPERTY, DataModel::DISCONTINUOUS_3D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "HcVapourVelocityY",              "HcVapourVelocityY",              "mm/y",  FORMATIONPROPERTY, DataModel::DISCONTINUOUS_3D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "HcVapourVelocityZ",              "HcVapourVelocityZ",              "mm/y",  FORMATIONPROPERTY, DataModel::DISCONTINUOUS_3D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "HcVapourVelocityMagnitude",      "HcVapourVelocityMagnitude",      "mm/y",  FORMATIONPROPERTY, DataModel::DISCONTINUOUS_3D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "HcLiquidVelocityX",              "HcLiquidVelocityX",              "mm/y",  FORMATIONPROPERTY, DataModel::DISCONTINUOUS_3D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "HcLiquidVelocityY",              "HcLiquidVelocityY",              "mm/y",  FORMATIONPROPERTY, DataModel::DISCONTINUOUS_3D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "HcLiquidVelocityZ",              "HcLiquidVelocityZ",              "mm/y",  FORMATIONPROPERTY, DataModel::DISCONTINUOUS_3D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "HcLiquidVelocityMagnitude",      "HcLiquidVelocityMagnitude",      "mm/y",  FORMATIONPROPERTY, DataModel::DISCONTINUOUS_3D_PROPERTY ));


   //Genex5 Properties
   using namespace CBMGenerics;
   ComponentManager & theComponentManager = ComponentManager::getInstance();
   GenexResultManager & theResultManager = GenexResultManager::getInstance();

   int i;
   for ( i = 0; i < ComponentManager::NumberOfOutputSpecies; ++i )
   {
      m_properties.push_back( getFactory()->produceProperty( this, 0,
         theComponentManager.GetSpeciesOutputPropertyName( i, false ),
         theComponentManager.GetSpeciesOutputPropertyName( i, false ),
                                                             theResultManager.GetResultUnit( GenexResultManager::OilGeneratedCum ), FORMATIONPROPERTY,
                                                             DataModel::FORMATION_2D_PROPERTY) );

      m_properties.push_back( getFactory()->produceProperty( this, 0,
         theComponentManager.GetSpeciesOutputPropertyName( i, true ),
         theComponentManager.GetSpeciesOutputPropertyName( i, true ),
                                                             theResultManager.GetResultUnit( GenexResultManager::OilGeneratedCum ), FORMATIONPROPERTY,
                                                             DataModel::FORMATION_2D_PROPERTY ) );

   }


   for ( i = 0; i < GenexResultManager::NumberOfResults; ++i )
   {
      m_properties.push_back( getFactory()->produceProperty( this, 0,
         theResultManager.GetResultName( i ),
         theResultManager.GetResultName( i ),
                                                             theResultManager.GetResultUnit( i ), FORMATIONPROPERTY,
                                                             DataModel::FORMATION_2D_PROPERTY ) );
   }

   for ( i = 0; i < ComponentManager::NumberOfOutputSpecies; ++i )
   {
      m_properties.push_back( getFactory()->produceProperty( this, 0,
         theComponentManager.GetSpeciesName( i ) + "Concentration",
         theComponentManager.GetSpeciesName( i ) + "Concentration",
                                                             "kg/m3", FORMATIONPROPERTY,
                                                             DataModel::DISCONTINUOUS_3D_PROPERTY ) );
   }

   m_properties.push_back( getFactory()->produceProperty( this, 0,
      "ElementMass", "ElementMass",
                                                          "kg/m3", FORMATIONPROPERTY,
                                                          DataModel::DISCONTINUOUS_3D_PROPERTY ) );

   m_properties.push_back( getFactory()->produceProperty( this, 0,
      "TransportedMass", "TransportedMass",
                                                          "kg", FORMATIONPROPERTY,
                                                          DataModel::DISCONTINUOUS_3D_PROPERTY ) );

   for ( i = 0; i < ComponentManager::NumberOfOutputSpecies; ++i )
   {
      m_properties.push_back( getFactory()->produceProperty( this, 0,
         theComponentManager.GetSpeciesName( i ) + "Retained",
         theComponentManager.GetSpeciesName( i ) + "Retained",
                                                             theResultManager.GetResultUnit( GenexResultManager::OilGeneratedCum ), FORMATIONPROPERTY,
                                                             DataModel::FORMATION_2D_PROPERTY ) );

      m_properties.push_back( getFactory()->produceProperty( this, 0,
         theComponentManager.GetSpeciesName( i ) + "Adsorped",
         theComponentManager.GetSpeciesName( i ) + "Adsorped",
                                                             "scf/ton", FORMATIONPROPERTY,
                                                             DataModel::FORMATION_2D_PROPERTY ) );

      m_properties.push_back( getFactory()->produceProperty( this, 0,
         theComponentManager.GetSpeciesName( i ) + "AdsorpedExpelled",
         theComponentManager.GetSpeciesName( i ) + "AdsorpedExpelled",
                                                             "scf/ton", FORMATIONPROPERTY,
                                                             DataModel::FORMATION_2D_PROPERTY ) );

      m_properties.push_back( getFactory()->produceProperty( this, 0,
         theComponentManager.GetSpeciesName( i ) + "AdsorpedFree",
         theComponentManager.GetSpeciesName( i ) + "AdsorpedFree",
                                                             "scf/ton", FORMATIONPROPERTY,
                                                             DataModel::FORMATION_2D_PROPERTY ) );

   }


   m_properties.push_back( getFactory()->produceProperty( this, 0, "AdsorptionCapacity",            "AdsorptionCapacity",            "scf/ton",   FORMATIONPROPERTY, DataModel::FORMATION_2D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "FractionOfAdsorptionCap",       "FractionOfAdsorptionCap",       "%",         FORMATIONPROPERTY, DataModel::FORMATION_2D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "GasExpansionRatio_Bg",          "GasExpansionRatio_Bg",          "m3/m3",     FORMATIONPROPERTY, DataModel::FORMATION_2D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "HcLiquidSat",                   "HcLiquidSat",                   "",          FORMATIONPROPERTY, DataModel::FORMATION_2D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "HcSaturation",                  "HcSaturation",                  "",          FORMATIONPROPERTY, DataModel::FORMATION_2D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "HcVapourSat",                   "HcVapourSat",                   "",          FORMATIONPROPERTY, DataModel::FORMATION_2D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "ImmobileWaterSat",              "ImmobileWaterSat",              "",          FORMATIONPROPERTY, DataModel::FORMATION_2D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "Oil2GasGeneratedCumulative",    "Oil2GasGeneratedCumulative",    "kg/m2",     FORMATIONPROPERTY, DataModel::FORMATION_2D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "OverChargeFactor",              "OverChargeFactor",              "frac",      FORMATIONPROPERTY, DataModel::FORMATION_2D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "RetainedCondensateApiSR",       "RetainedCondensateApiSR",       "",          FORMATIONPROPERTY, DataModel::FORMATION_2D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "RetainedCgrSR",                 "RetainedCgrSR",                 "bbl/mcf",   FORMATIONPROPERTY, DataModel::FORMATION_2D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "RetainedGasVolumeST",           "RetainedGasVolumeST",           "bcf/km2",   FORMATIONPROPERTY, DataModel::FORMATION_2D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "RetainedGorSR",                 "RetainedGorSR",                 "scf/bbl",   FORMATIONPROPERTY, DataModel::FORMATION_2D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "RetainedOilApiSR",              "RetainedOilApiSR",              "",          FORMATIONPROPERTY, DataModel::FORMATION_2D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "RetainedOilVolumeST",           "RetainedOilVolumeST",           "mmbbl/km2", FORMATIONPROPERTY, DataModel::FORMATION_2D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "TotalGasGeneratedCumulative",   "TotalGasGeneratedCumulative",   "kg/m2",     FORMATIONPROPERTY, DataModel::FORMATION_2D_PROPERTY ));

   // reservoir properties
   m_properties.push_back( getFactory()->produceProperty( this, 0, "ResRockTrapArea",               "ResRockTrapArea",               "",        RESERVOIRPROPERTY, DataModel::FORMATION_2D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "ResRockTrapId",                 "ResRockTrapId",                 "",        RESERVOIRPROPERTY, DataModel::FORMATION_2D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "ResRockLeakage",                "ResRockLeakage",                "kg",      RESERVOIRPROPERTY, DataModel::FORMATION_2D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "ResRockLeakageUpward",          "ResRockLeakageUpward",          "kg",      RESERVOIRPROPERTY, DataModel::FORMATION_2D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "ResRockLeakageOutward",         "ResRockLeakageOutward",         "kg",      RESERVOIRPROPERTY, DataModel::FORMATION_2D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "ResRockTop",                    "ResRockTop",                    "m",       RESERVOIRPROPERTY, DataModel::FORMATION_2D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "ResRockOverburden",             "ResRockOverburden",             "m",       RESERVOIRPROPERTY, DataModel::FORMATION_2D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "ResRockBottom",                 "ResRockBottom",                 "m",       RESERVOIRPROPERTY, DataModel::FORMATION_2D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "ResRockThickness",              "ResRockThickness",              "m",       RESERVOIRPROPERTY, DataModel::FORMATION_2D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "ResRockPorosity",               "ResRockPorosity",               "%",       RESERVOIRPROPERTY, DataModel::FORMATION_2D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "ResRockSealPermeability",       "ResRockSealPermeability",       "mD",      RESERVOIRPROPERTY, DataModel::FORMATION_2D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "ResRockFlowDirectionGasIJ",     "ResRockFlowDirectionGasIJ",     "",        RESERVOIRPROPERTY, DataModel::FORMATION_2D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "ResRockFlowDirectionGasI",      "ResRockFlowDirectionGasI",      "",        RESERVOIRPROPERTY, DataModel::FORMATION_2D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "ResRockFlowDirectionGasJ",      "ResRockFlowDirectionGasJ",      "",        RESERVOIRPROPERTY, DataModel::FORMATION_2D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "ResRockFlowDirectionFluidIJ",   "ResRockFlowDirectionFluidIJ",   "",        RESERVOIRPROPERTY, DataModel::FORMATION_2D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "ResRockFlowDirectionFluidI",    "ResRockFlowDirectionFluidI",    "",        RESERVOIRPROPERTY, DataModel::FORMATION_2D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "ResRockFlowDirectionFluidJ",    "ResRockFlowDirectionFluidJ",    "",        RESERVOIRPROPERTY, DataModel::FORMATION_2D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "ResRockFlow",                   "ResRockFlow",                   "log(kg)", RESERVOIRPROPERTY, DataModel::FORMATION_2D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "ResRockFlowDirection",          "ResRockFlowDirection",          "",        RESERVOIRPROPERTY, DataModel::FORMATION_2D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "ResRockFlux",                   "ResRockFlux",                   "kg/m2",   RESERVOIRPROPERTY, DataModel::FORMATION_2D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "ResRockImmobilesVolume",        "ResRockImmobilesVolume",        "m3",      RESERVOIRPROPERTY, DataModel::FORMATION_2D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "ResRockImmobilesDensity",       "ResRockImmobilesDensity",       "kg/m2",   RESERVOIRPROPERTY, DataModel::FORMATION_2D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "ResRockFillGasPhaseQuantity",   "ResRockFillGasPhaseQuantity",   "kg",      RESERVOIRPROPERTY, DataModel::FORMATION_2D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "ResRockFillFluidPhaseQuantity", "ResRockFillFluidPhaseQuantity", "kg",      RESERVOIRPROPERTY, DataModel::FORMATION_2D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "ResRockFillGasPhaseDensity",    "ResRockFillGasPhaseDensity",    "kg/m2",   RESERVOIRPROPERTY, DataModel::FORMATION_2D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "ResRockFillFluidPhaseDensity",  "ResRockFillFluidPhaseDensity",  "kg/m2",   RESERVOIRPROPERTY, DataModel::FORMATION_2D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "ResRockDrainageAreaGasPhase",   "ResRockDrainageAreaGasPhase",   "",        RESERVOIRPROPERTY, DataModel::FORMATION_2D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "ResRockDrainageAreaFluidPhase", "ResRockDrainageAreaFluidPhase", "",        RESERVOIRPROPERTY, DataModel::FORMATION_2D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "ResRockDrainageIdGasPhase",     "ResRockDrainageIdGasPhase",     "",        RESERVOIRPROPERTY, DataModel::FORMATION_2D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "ResRockDrainageIdFluidPhase",   "ResRockDrainageIdFluidPhase",   "",        RESERVOIRPROPERTY, DataModel::FORMATION_2D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "ResRockBarriers",               "ResRockBarriers",               "",        RESERVOIRPROPERTY, DataModel::FORMATION_2D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "ResRockFaultCutEdges",          "ResRockFaultCutEdges",          "",        RESERVOIRPROPERTY, DataModel::FORMATION_2D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "ResRockPressure",               "ResRockPressure",               "MPa",     RESERVOIRPROPERTY, DataModel::FORMATION_2D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "ResRockTemperature",            "ResRockTemperature",            "C",       RESERVOIRPROPERTY, DataModel::FORMATION_2D_PROPERTY ));
   m_properties.push_back( getFactory()->produceProperty( this, 0, "ResRockCapacity",               "ResRockCapacity",               "m3",      RESERVOIRPROPERTY, DataModel::FORMATION_2D_PROPERTY ));

   // Touchstone properties.
   m_properties.push_back( getFactory()->produceProperty( this, 0, "Resq: *", "Resq: *", "", FORMATIONPROPERTY, DataModel::FORMATION_2D_PROPERTY ) );

   // trap properties
   m_properties.push_back( getFactory()->produceProperty( this, 0, "VolumeFGIIP",        "VolumeFGIIP",        "m3",        TRAPPROPERTY, DataModel::TRAP_PROPERTY )); // Volume of free gas initially in place
   m_properties.push_back( getFactory()->produceProperty( this, 0, "VolumeCIIP",         "VolumeCIIP",         "m3",        TRAPPROPERTY, DataModel::TRAP_PROPERTY )); // Volume of condensate initially in place
   m_properties.push_back( getFactory()->produceProperty( this, 0, "VolumeSGIIP",        "VolumeSGIIP",        "m3",        TRAPPROPERTY, DataModel::TRAP_PROPERTY )); // Volume of solution gas initially in place
   m_properties.push_back( getFactory()->produceProperty( this, 0, "VolumeSTOIIP",       "VolumeSTOIIP",       "m3",        TRAPPROPERTY, DataModel::TRAP_PROPERTY )); // Volume of stock tank oil initially in place
   m_properties.push_back( getFactory()->produceProperty( this, 0, "VolumeLiquid",       "VolumeLiquid",       "m3",        TRAPPROPERTY, DataModel::TRAP_PROPERTY )); // Volume of reservoir liquid phase
   m_properties.push_back( getFactory()->produceProperty( this, 0, "VolumeVapour",       "VolumeLiquid",       "m3",        TRAPPROPERTY, DataModel::TRAP_PROPERTY )); // Volume of reservoir vapour phase
   m_properties.push_back( getFactory()->produceProperty( this, 0, "DensityFGIIP",       "DensityFGIIP",       "kg/m3",     TRAPPROPERTY, DataModel::TRAP_PROPERTY )); // Density of free gas initially in place
   m_properties.push_back( getFactory()->produceProperty( this, 0, "DensityCIIP",        "DensityCIIP",        "kg/m3",     TRAPPROPERTY, DataModel::TRAP_PROPERTY )); // Density of condensate initially in place
   m_properties.push_back( getFactory()->produceProperty( this, 0, "DensitySGIIP",       "DensitySGIIP",       "kg/m3",     TRAPPROPERTY, DataModel::TRAP_PROPERTY )); // Density of solution gas initially in place
   m_properties.push_back( getFactory()->produceProperty( this, 0, "DensitySTOIIP",      "DensitySTOIIP",      "kg/m3",     TRAPPROPERTY, DataModel::TRAP_PROPERTY )); // Density of stock tank oil initially in place
   m_properties.push_back( getFactory()->produceProperty( this, 0, "DensityLiquid",      "DensityLiquid",      "kg/m3",     TRAPPROPERTY, DataModel::TRAP_PROPERTY )); // Density of reservoir liquid phase
   m_properties.push_back( getFactory()->produceProperty( this, 0, "DensityVapour",      "DensityLiquid",      "kg/m3",     TRAPPROPERTY, DataModel::TRAP_PROPERTY )); // Density of reservoir vapour phase
   m_properties.push_back( getFactory()->produceProperty( this, 0, "ViscosityFGIIP",     "ViscosityFGIIP",     "Pa*s",      TRAPPROPERTY, DataModel::TRAP_PROPERTY )); // Viscosity of free gas initially in place
   m_properties.push_back( getFactory()->produceProperty( this, 0, "ViscosityCIIP",      "ViscosityCIIP",      "Pa*s",      TRAPPROPERTY, DataModel::TRAP_PROPERTY )); // Viscosity of condensate initially in place
   m_properties.push_back( getFactory()->produceProperty( this, 0, "ViscositySGIIP",     "ViscositySGIIP",     "Pa*s",      TRAPPROPERTY, DataModel::TRAP_PROPERTY )); // Viscosity of solution gas initially in place
   m_properties.push_back( getFactory()->produceProperty( this, 0, "ViscositySTOIIP",    "ViscositySTOIIP",    "Pa*s",      TRAPPROPERTY, DataModel::TRAP_PROPERTY )); // Viscosity of stock tank oil initially in place
   m_properties.push_back( getFactory()->produceProperty( this, 0, "ViscosityLiquid",    "ViscosityLiquid",    "Pa*s",      TRAPPROPERTY, DataModel::TRAP_PROPERTY )); // Viscosity of reservoir liquid phase
   m_properties.push_back( getFactory()->produceProperty( this, 0, "ViscosityVapour",    "ViscosityLiquid",    "Pa*s",      TRAPPROPERTY, DataModel::TRAP_PROPERTY )); // Viscosity of reservoir vapour phase
   m_properties.push_back( getFactory()->produceProperty( this, 0, "MassFGIIP",          "MassFGIIP",          "kg",        TRAPPROPERTY, DataModel::TRAP_PROPERTY )); // Mass of free gas initially in place
   m_properties.push_back( getFactory()->produceProperty( this, 0, "MassCIIP",           "MassCIIP",           "kg",        TRAPPROPERTY, DataModel::TRAP_PROPERTY )); // Mass of condensate initially in place
   m_properties.push_back( getFactory()->produceProperty( this, 0, "MassSGIIP",          "MassSGIIP",          "kg",        TRAPPROPERTY, DataModel::TRAP_PROPERTY )); // Mass of solution gas initially in place
   m_properties.push_back( getFactory()->produceProperty( this, 0, "MassSTOIIP",         "MassSTOIIP",         "kg",        TRAPPROPERTY, DataModel::TRAP_PROPERTY )); // Mass of stock tank oil initially in place
   m_properties.push_back( getFactory()->produceProperty( this, 0, "MassLiquid",         "MassLiquid",         "kg",        TRAPPROPERTY, DataModel::TRAP_PROPERTY )); // Mass of reservoir liquid phase
   m_properties.push_back( getFactory()->produceProperty( this, 0, "MassVapour",         "MassLiquid",         "kg",        TRAPPROPERTY, DataModel::TRAP_PROPERTY )); // Mass of reservoir vapour phase
   m_properties.push_back( getFactory()->produceProperty( this, 0, "CGR",                "CGR",                "m3/m3",     TRAPPROPERTY, DataModel::TRAP_PROPERTY )); // Condensate Gas Ratio: VolumeCIIP / VolumeFGIIP
   m_properties.push_back( getFactory()->produceProperty( this, 0, "GOR",                "GOR",                "m3/m3",     TRAPPROPERTY, DataModel::TRAP_PROPERTY )); // Gas Oil Ratio: VolumeSGIIP / VolumeSTOIIP
   m_properties.push_back( getFactory()->produceProperty( this, 0, "OilAPI",             "OilAPI",             "",          TRAPPROPERTY, DataModel::TRAP_PROPERTY )); // API of STOIIP
   m_properties.push_back( getFactory()->produceProperty( this, 0, "CondensateAPI",      "CondensateAPI",      "",          TRAPPROPERTY, DataModel::TRAP_PROPERTY )); // API of CIIP
   m_properties.push_back( getFactory()->produceProperty( this, 0, "GasWetnessFGIIP",    "GasWetnessFGIIP",    "mole/mole", TRAPPROPERTY, DataModel::TRAP_PROPERTY )); // C1 / Sum (C2 - C5) of FGIIP
   m_properties.push_back( getFactory()->produceProperty( this, 0, "GasWetnessSGIIP",    "GasWetnessSGIIP",    "mole/mole", TRAPPROPERTY, DataModel::TRAP_PROPERTY )); // C1 / Sum (C2 - C5) of SGIIP
   m_properties.push_back( getFactory()->produceProperty( this, 0, "CEPLiquid",          "CEPLiquid",          "MPa",       TRAPPROPERTY, DataModel::TRAP_PROPERTY )); // 
   m_properties.push_back( getFactory()->produceProperty( this, 0, "CEPVapour",          "CEPVapour",          "MPa",       TRAPPROPERTY, DataModel::TRAP_PROPERTY )); //
   m_properties.push_back( getFactory()->produceProperty( this, 0, "FracturePressure",   "FracturePressure",   "MPa",       TRAPPROPERTY, DataModel::TRAP_PROPERTY )); // Fracture pressure of the trap
   m_properties.push_back( getFactory()->produceProperty( this, 0, "ColumnHeightLiquid", "ColumnHeightLiquid", "m",         TRAPPROPERTY, DataModel::TRAP_PROPERTY )); // Height of the liquid column
   m_properties.push_back( getFactory()->produceProperty( this, 0, "ColumnHeightVapour", "ColumnHeightVapour", "m",         TRAPPROPERTY, DataModel::TRAP_PROPERTY )); // Height of the vapour column
   m_properties.push_back( getFactory()->produceProperty( this, 0, "GOC",                "GOC",                "m",         TRAPPROPERTY, DataModel::TRAP_PROPERTY )); // Depth of Vapour-Liquid contact
   m_properties.push_back( getFactory()->produceProperty( this, 0, "OWC",                "OWC",                "m",         TRAPPROPERTY, DataModel::TRAP_PROPERTY )); // Depth of Liquid-Water contact
   m_properties.push_back( getFactory()->produceProperty( this, 0, "SpillDepth",         "SpillDepth",         "m",         TRAPPROPERTY, DataModel::TRAP_PROPERTY )); // Spill depth
   m_properties.push_back( getFactory()->produceProperty( this, 0, "SealPermeability",   "SealPermeability",   "mD",        TRAPPROPERTY, DataModel::TRAP_PROPERTY )); //

   // Crustal Thickness Calculator output property
   for ( i = 0; i < CrustalThicknessInterface::numberOfOutputMaps; ++i )
   {
      m_properties.push_back( getFactory()->produceProperty( this, 0,
         CrustalThicknessInterface::outputMapsNames[ i ],
         CrustalThicknessInterface::outputMapsNames[ i ],
                                                             CrustalThicknessInterface::outputMapsUnits[ i ],
                                                             FORMATIONPROPERTY,
                                                             DataModel::SURFACE_2D_PROPERTY ));
   }

   return true;
}

bool ProjectHandle::loadTimeOutputProperties() {

   database::Table* filterTimeTbl = getTable( "FilterTimeIoTbl" );
   database::Table::iterator tblIter;
   Interface::ModellingMode recordMode;

   m_timeOutputProperties.clear();

   for ( tblIter = filterTimeTbl->begin(); tblIter != filterTimeTbl->end(); ++tblIter )
   {

      if ( database::getModellingMode( *tblIter ) == "1d" ) {
         recordMode = Interface::MODE1D;
      }
      else {
         // It may be that the modelling mode string = "multi-1d", in which case
         // it will be switched to 3d.
         recordMode = Interface::MODE3D;
      }

      if ( recordMode == getModellingMode() )
      {
         m_timeOutputProperties.push_back( getFactory()->produceOutputProperty( this, *tblIter ) );
      }

   }

   return true;
}

bool ProjectHandle::loadDepthOutputProperties() {

   database::Table* filterDepthTbl = getTable( "FilterDepthIoTbl" );
   database::Table::iterator tblIter;
   Interface::ModellingMode recordMode;

   m_depthOutputProperties.clear();

   for ( tblIter = filterDepthTbl->begin(); tblIter != filterDepthTbl->end(); ++tblIter )
   {

      if ( database::getModellingMode( *tblIter ) == "1d" ) {
         recordMode = Interface::MODE1D;
      }
      else {
         // It may be that the modelling mode string = "multi-1d", in which case
         // it will be switched to 3d.
         recordMode = Interface::MODE3D;
      }

      if ( recordMode == getModellingMode() )
      {
         m_depthOutputProperties.push_back( getFactory()->produceOutputProperty( this, *tblIter ) );
      }

   }

   return true;
}

bool ProjectHandle::loadLithologyThermalConductivitySamples() {

   database::Table* thermalConductivityTbl = getTable( "LitThCondIoTbl" );
   database::Table::iterator tblIter;

   m_lithologyThermalConductivitySamples.clear();

   for ( tblIter = thermalConductivityTbl->begin(); tblIter != thermalConductivityTbl->end(); ++tblIter )
   {
      m_lithologyThermalConductivitySamples.push_back( getFactory()->produceLithologyThermalConductivitySample( this, *tblIter ) );
   }

   return true;
}

bool ProjectHandle::loadLithologyHeatCapacitySamples() {

   database::Table* heatCapacityTbl = getTable( "LitHeatCapIoTbl" );
   database::Table::iterator tblIter;

   m_lithologyHeatCapacitySamples.clear();

   for ( tblIter = heatCapacityTbl->begin(); tblIter != heatCapacityTbl->end(); ++tblIter )
   {
      m_lithologyHeatCapacitySamples.push_back( getFactory()->produceLithologyHeatCapacitySample( this, *tblIter ) );

   }

   return true;
}

bool ProjectHandle::loadFluidHeatCapacitySamples() {

   database::Table* heatCapacityTbl = getTable( "FltHeatCapIoTbl" );
   database::Table::iterator tblIter;

   m_fluidHeatCapacitySamples.clear();

   for ( tblIter = heatCapacityTbl->begin(); tblIter != heatCapacityTbl->end(); ++tblIter )
   {
      FluidHeatCapacitySample* sample = getFactory()->produceFluidHeatCapacitySample( this, *tblIter );
      m_fluidHeatCapacitySamples.push_back( sample );

   }

   return true;
}

bool ProjectHandle::loadFluidThermalConductivitySamples() {

   database::Table* thermalConductivityTbl = getTable( "FltThCondIoTbl" );
   database::Table::iterator tblIter;

   m_fluidThermalConductivitySamples.clear();

   for ( tblIter = thermalConductivityTbl->begin(); tblIter != thermalConductivityTbl->end(); ++tblIter )
   {
      m_fluidThermalConductivitySamples.push_back( getFactory()->produceFluidThermalConductivitySample( this, *tblIter ) );
   }

   return true;
}

bool ProjectHandle::loadFluidDensitySamples() {

   database::Table* densityTbl = getTable( "FltDensityIoTbl" );
   database::Table::iterator tblIter;

   m_fluidDensitySamples.clear();

   for ( tblIter = densityTbl->begin(); tblIter != densityTbl->end(); ++tblIter )
   {

      m_fluidDensitySamples.push_back( getFactory()->produceFluidDensitySample( this, *tblIter ) );
   }

   return true;
}


bool ProjectHandle::loadRelatedProjects() {

   database::Table* relatedProjectsTbl = getTable( "RelatedProjectsIoTbl" );
   database::Table::iterator tblIter;

   m_relatedProjects.clear();

   for ( tblIter = relatedProjectsTbl->begin(); tblIter != relatedProjectsTbl->end(); ++tblIter )
   {
      m_relatedProjects.push_back( getFactory()->produceRelatedProject( this, *tblIter ) );
   }

   return true;
}


bool ProjectHandle::loadFormations( void )
{
   database::Table* stratTbl = getTable( "StratIoTbl" );
   database::Table* sourceRockLithoIoTbl = getTable( "SourceRockLithoIoTbl" );
   database::Table::iterator tblIter;
   Record * stratRecord = 0;
   Record * sourceRockRecord = 0;
   Formation* formation;
   SourceRock* sourceRock;

   for ( tblIter = stratTbl->begin(); tblIter != stratTbl->end(); ++tblIter ) {

      stratRecord = *tblIter;

      int depoSequenceNumber = database::getDepoSequence( stratRecord );

      // If the depo-sequence number is the null value then this is the bottom most surface definition.
      // There is no formation contained in this strat-table record.
      if ( depoSequenceNumber != DefaultUndefinedScalarValue ) {
         formation = getFactory()->produceFormation( this, stratRecord );
         m_formations.push_back( formation );

         if ( database::getSourceRock( stratRecord ) ) {
            sourceRockRecord = sourceRockLithoIoTbl->findRecord( "SourceRockType", database::getSourceRockType1( stratRecord ),
               "LayerName", database::getLayerName( stratRecord ) );
            assert( sourceRockRecord != 0 );

            sourceRock = getFactory()->produceSourceRock( this, sourceRockRecord );
            m_sourceRocks.push_back( sourceRock );
            formation->setSourceRock1( sourceRock );

            if ( database::getLayerName( sourceRockRecord ) == "" ) {
               sourceRock->setLayerName( database::getLayerName( stratRecord ) );
            }
            else {
               assert( database::getLayerName( sourceRockRecord ) == database::getLayerName( stratRecord ) );
            }

            if ( database::getEnableSourceRockMixing( stratRecord ) == 1 ) {
               Record * sourceRockRecord2 = sourceRockLithoIoTbl->findRecord( "SourceRockType", database::getSourceRockType2( stratRecord ),
                  "LayerName", database::getLayerName( stratRecord ), sourceRockRecord );
               assert( sourceRockRecord2 != 0 );

               sourceRock = getFactory()->produceSourceRock( this, sourceRockRecord2 );
               m_sourceRocks.push_back( sourceRock );
               sourceRock->setLayerName( database::getLayerName( stratRecord ) );
               formation->setSourceRock2( sourceRock );
            }

         }

      }

   }

   // Sort the list of surfaces into age order, youngest first, oldest last on the list.
   std::sort( m_formations.begin(), m_formations.end(), FormationLessThan() );

   return true;
}

bool ProjectHandle::loadIgneousIntrusions() {

   MutableFormationList::iterator formationIter;

   for ( formationIter = m_formations.begin(); formationIter != m_formations.end(); ++formationIter )
   {
      Formation * formation = *formationIter;

      if ( formation->getIsIgneousIntrusion() ) {
         IgneousIntrusionEvent* igneousIntrusion = getFactory()->produceIgneousIntrusionEvent( this, formation->getRecord() );
         m_igneousIntrusionEvents.push_back( igneousIntrusion );
         formation->setIgneousIntrusionEvent( igneousIntrusion );
      }

   }
   
   m_previousIgneousIntrusionTime = DefaultUndefinedValue;
   return true;
}


bool ProjectHandle::loadFluidTypes() {

   database::Table* fluidTbl = getTable( "FluidtypeIoTbl" );
   database::Table::iterator tblIter;
   Record * fluidRecord = 0;

   for ( tblIter = fluidTbl->begin(); tblIter != fluidTbl->end(); ++tblIter )
   {
      fluidRecord = *tblIter;

      if ( fluidRecord )
      {
         m_fluidTypes.push_back( getFactory()->produceFluidType( this, fluidRecord ) );
      }
   }
   return true;

}

bool ProjectHandle::loadConstrainedOverpressureIntervals() {

   database::Table* geologicalBoundaryIoTbl = getTable( "GeologicalBoundaryIoTbl" );
   database::Table* propertyBoundaryIoTbl = getTable( "PropertyBoundaryIoTbl" );
   database::Table* boundaryValuesIoTbl = getTable( "BoundaryValuesIoTbl" );

   database::Table::iterator geologicalBoundaryTblIter;

   size_t i;
   size_t j;

   for ( geologicalBoundaryTblIter = geologicalBoundaryIoTbl->begin(); geologicalBoundaryTblIter != geologicalBoundaryIoTbl->end(); ++geologicalBoundaryTblIter ) {
      database::Record* geologicalBoundaryRecord = *geologicalBoundaryTblIter;

      const std::string& GeologicalObjectName = database::getGeologicalObjectName( geologicalBoundaryRecord );

      // Since constrained overpressure is the only boundary condition type that has been implemented (this will remain so)
      // using these set of tables then it make sense to get the formation in the outer-most loop. Since the meaning
      // of the GeologicalObjectName is the formation name.
      const Interface::Formation* formation = findFormation( GeologicalObjectName );
      // Minus 1 here because of array indexing.
      const unsigned int FirstIndexInPropertyTable = database::getFirstIndexInPropertyIoTbl( geologicalBoundaryRecord ) - 1;
      const unsigned int NumberOfProperties = database::getNumberOfProperties( geologicalBoundaryRecord );

      for ( i = FirstIndexInPropertyTable; i < FirstIndexInPropertyTable + NumberOfProperties; ++i ) {
         database::Record* propertyRecord = propertyBoundaryIoTbl->getRecord( static_cast<int>(i) );

         const std::string& PropertyName = database::getNameOfProperty( propertyRecord );
         // Minus 1 here because of array indexing.
         const unsigned int FirstIndexInPropertyBoundaryValuesIoTbl = database::getFirstIndexInPropertyBoundaryValuesIoTbl( propertyRecord ) - 1;
         const unsigned int NumberOfTimeIntervals = database::getNumberOfTimeIntervals( propertyRecord );

         // Is this check necessary?
         // Since constrained overpressure is the only boundary condition to be implemented using this set of tables.
         if ( PropertyName == "Pressure" ) {

            for ( j = FirstIndexInPropertyBoundaryValuesIoTbl; j < FirstIndexInPropertyBoundaryValuesIoTbl + NumberOfTimeIntervals; ++j ) {
               database::Record* copRecord = boundaryValuesIoTbl->getRecord( static_cast<int>(j) );

               ConstrainedOverpressureInterval* copInterval = getFactory()->produceConstrainedOverpressureInterval( this, copRecord, formation );

               m_constrainedOverpressureIntervals.push_back( copInterval );

            }

         }

      }

   }

   return true;
}


bool ProjectHandle::loadFaults( void )
{
   loadFaultCollections();
   connectFaultCollections();
   loadFaultEvents();
   return true;
}

bool ProjectHandle::loadFaultCollections( void )
{
   MutableInputValueList::const_iterator inputValueIter;

   for ( inputValueIter = m_inputValues.begin(); inputValueIter != m_inputValues.end(); ++inputValueIter )
   {
      InputValue *inputValue = *inputValueIter;

      if ( inputValue->getType() != Interface::FaultMap )
         continue;

      FaultFileReader *reader = FaultFileReaderFactory::getInstance().createReader( inputValue->getMapType() );

      if ( !reader )
         continue;

      FaultCollection *faultCollection = getFactory()->produceFaultCollection( this, inputValue->getMapName() );

      m_faultCollections.push_back( faultCollection );

      bool fileIsOpen;

      ibs::FilePath fullFileName( getProjectPath() );
      fullFileName << inputValue->getFileName();
      reader->open( fullFileName.path(), fileIsOpen );

      if ( !fileIsOpen )
      {
         cerr << "****************    ERROR FaultManager::readFaultMapFiles (): Fault file "
            << inputValue->getFileName()
            << " could not be opened.   ****************" << endl;
         continue;
      }

      reader->preParseFaults();

      FaultFileReader::FaultDataSetIterator faultIter;
      for ( faultIter = reader->begin(); faultIter != reader->end(); ++faultIter )
      {
         faultCollection->addFault( reader->faultName( faultIter ), reader->faultLine( faultIter ) );
      }

      reader->close();

      delete reader;
   }

   return true;
}

Interface::FaultCollection * ProjectHandle::findFaultCollection( const string & name ) const
{
   MutableFaultCollectionList::const_iterator fcIter;

   for ( fcIter = m_faultCollections.begin(); fcIter != m_faultCollections.end(); ++fcIter )
   {
      FaultCollection * fc = *fcIter;
      if ( fc->getName() == name )
      {
         return fc;
      }
   }
   return 0;
}


bool ProjectHandle::connectFaultCollections( void )
{
   database::Table *palinspasticTbl = getTable( "PalinspasticIoTbl" );
   database::Table::iterator tblIter;

   for ( tblIter = palinspasticTbl->begin(); tblIter != palinspasticTbl->end(); ++tblIter )
   {
      Record * palinspasticRecord = *tblIter;
      if ( !palinspasticRecord ) continue;

      const string & faultcutsMap = database::getFaultcutsMap( palinspasticRecord );
      const string & surfaceName = database::getSurfaceName( palinspasticRecord );
      const string & bottomFormationName = database::getBottomFormationName( palinspasticRecord );

      FaultCollection *faultCollection = (FaultCollection *)findFaultCollection( faultcutsMap );
      if ( !faultCollection ) continue;


      const Surface *topSurface = dynamic_cast<const Surface *>( findSurface( surfaceName ) );

      if ( !topSurface )
      {
         cerr << "ProjectHandle::connectFaultCollections (): could not find Surface " << surfaceName << endl;
         continue;
      }

      const Formation *topFormation = dynamic_cast<const Formation *> ( topSurface->getBottomFormation() );
      if ( !topFormation )
      {
         cerr << "ProjectHandle::connectFaultCollections (): could not find bottom formation of Surface " << surfaceName << endl;
         continue;
      }

      const Formation *bottomFormation;
      if ( bottomFormationName.size() > 0 )
      {
         bottomFormation = dynamic_cast<const Formation *> ( findFormation( bottomFormationName ) );
         if ( !bottomFormation )
         {
            cerr << "ProjectHandle::connectFaultCollections (): could not find Formation " << bottomFormationName << endl;
            continue;
         }
      }
      else
      {
         bottomFormation = topFormation;
      }

      const Surface *bottomSurface = dynamic_cast<const Surface *> ( bottomFormation->getBottomSurface() );

      const Formation *formation = topFormation;
      const Surface *surface;

      do
      {
         faultCollection->addFormation( formation );
#if 0
         cerr << "Linking fault collection " << faultCollection->getName () << " to formation " << formation->getName () << endl;
#endif

         surface = dynamic_cast<const Surface *> ( formation->getBottomSurface() );
         formation = dynamic_cast<const Formation *> ( surface->getBottomFormation() );
      } while ( surface != 0 && formation != 0 && surface != bottomSurface );
   }

   return true;
}

bool ProjectHandle::loadFaultEvents( void )
{
   database::Table *tbl = getTable( "FaultcutIoTbl" );

   if ( tbl )
   {
      database::Table::iterator tblIter;
      for ( tblIter = tbl->begin(); tblIter != tbl->end(); ++tblIter )
      {
         Record *record = *tblIter;

         double age = database::getAge( record );
         const Snapshot * snapshot = (const Snapshot *)findSnapshot( age );
         if ( !snapshot ) continue;

         const string & fcName = database::getSurfaceName( record );
         const string & faultName = database::getFaultName( record );
         const string & status = database::getFaultcutStatus( record );

         FaultCollection * fc = (FaultCollection *)findFaultCollection( fcName );
         if ( !fc ) continue;
         fc->addEvent( faultName, snapshot, status );
      }
   }

   tbl = getTable( "PressureFaultcutIoTbl" );

   if ( tbl )
   {
      database::Table::iterator tblIter;
      for ( tblIter = tbl->begin(); tblIter != tbl->end(); ++tblIter )
      {
         Record *record = *tblIter;

         double age = database::getStartAge( record );
         const Snapshot * snapshot = (const Snapshot *)findSnapshot( age );
         if ( !snapshot ) continue;

         const string & fcName = database::getFaultcutsMap( record );
         const string & faultName = database::getFaultName( record );
         const string & faultLithology = database::getFaultLithology( record );
         bool usedInOverpressure = database::getUsedInOverpressure( record ) ? true : false;

         FaultCollection * fc = (FaultCollection *)findFaultCollection( fcName );
         if ( !fc ) continue;
         fc->addOverpressureEvent( faultName, snapshot, faultLithology, usedInOverpressure );
      }
   }

   return true;
}

bool ProjectHandle::loadLithoTypes( void )
{
   database::Table * lithoTypeTbl = getTable( "LithotypeIoTbl" );
   database::Table::iterator tblIter;
   Record * crustLithoType = NULL;

   for ( tblIter = lithoTypeTbl->begin(); tblIter != lithoTypeTbl->end(); ++tblIter )
   {
      Record * lithoTypeRecord = *tblIter;
      m_lithoTypes.push_back( getFactory()->produceLithoType( this, lithoTypeRecord ) );

      if ( getLithotype( lithoTypeRecord ) == "Crust" ) {
         crustLithoType = lithoTypeRecord;
      }
   }

   if ( m_bottomBoundaryConditions == Interface::ADVANCED_LITHOSPHERE_CALCULATOR ) {
      database::Record * record = NULL;
      if ( crustLithoType != NULL ) {
         record = new Record( *crustLithoType );
      }
      else {
         record = new Record( lithoTypeTbl->getTableDefinition(), lithoTypeTbl );
         setPermMixModel( record, "None" );
      }
      setLithotype( record, DataAccess::Interface::ALCBasalt );
      m_lithoTypes.push_back( getFactory()->produceLithoType( this, record ) );
   }
   return true;
}

bool ProjectHandle::loadCrustalThicknessData( void )
{
   database::Table* CTCTbl = getTable( "CTCIoTbl" );
   database::Table::iterator tblIter;

   for ( tblIter = CTCTbl->begin(); tblIter != CTCTbl->end(); ++tblIter ) {
      Record * CTCRecord = *tblIter;
      m_crustalThicknessData.push_back( getFactory()->produceCrustalThicknessData( this, CTCRecord ) );
   }
   return true;
}

// Source rocks are now created in loadFormations.
#if 0
bool ProjectHandle::loadSourceRocks (void)
{
   database::Table* sourceRockTbl = getTable ("SourceRockLithoIoTbl");
   database::Table::iterator tblIter;
   for (tblIter = sourceRockTbl->begin (); tblIter != sourceRockTbl->end (); ++tblIter)
   {
      Record * sourceRockRecord = * tblIter;
      m_sourceRocks.push_back (getFactory ()->produceSourceRock (this, sourceRockRecord));
   }
   return true;
}
#endif

bool ProjectHandle::loadReservoirs( void )
{
   database::Table* reservoirTbl = getTable( "ReservoirIoTbl" );
   database::Table::iterator tblIter;
   for ( tblIter = reservoirTbl->begin(); tblIter != reservoirTbl->end(); ++tblIter )
   {
      Record * reservoirRecord = *tblIter;
      m_reservoirs.push_back( getFactory()->produceReservoir( this, reservoirRecord ) );
   }
   return true;
}

bool ProjectHandle::loadMobileLayers( void )
{
   database::Table* mobileLayerTbl = getTable( "MobLayThicknIoTbl" );
   database::Table::iterator tblIter;
   for ( tblIter = mobileLayerTbl->begin(); tblIter != mobileLayerTbl->end(); ++tblIter )
   {
      Record * mobileLayerRecord = *tblIter;
      m_mobileLayers.push_back( getFactory()->produceMobileLayer( this, mobileLayerRecord ) );
   }

   std::sort( m_mobileLayers.begin(), m_mobileLayers.end(), PaleoPropertyTimeLessThan() );

   return true;
}


bool ProjectHandle::loadHeatFlowHistory( void )
{

   database::Table* heatFlowTbl = getTable( "MntlHeatFlowIoTbl" );
   database::Table::iterator tblIter;

   for ( tblIter = heatFlowTbl->begin(); tblIter != heatFlowTbl->end(); ++tblIter )
   {
      Record * heatFlowRecord = *tblIter;
      m_heatFlowHistory.push_back( getFactory()->producePaleoSurfaceProperty( this, heatFlowRecord, m_mantleBottomSurface ) );
   }

   // Sort into correct order. Youngest first on the list.
   std::sort( m_heatFlowHistory.begin(), m_heatFlowHistory.end(), PaleoPropertyTimeLessThan() );

   return true;
}

bool ProjectHandle::loadCrustFormation() {

   database::Table* basementIoTbl = getTable( "BasementIoTbl" );
   Record *projectIoRecord = basementIoTbl->getRecord( 0 );

   m_crustFormation = getFactory()->produceCrustFormation( this, projectIoRecord );
   m_formations.push_back( m_crustFormation );

   return m_crustFormation != 0;
}

bool ProjectHandle::loadMantleFormation() {

   database::Table* basementIoTbl = getTable( "BasementIoTbl" );
   Record *projectIoRecord = basementIoTbl->getRecord( 0 );

   m_mantleFormation = getFactory()->produceMantleFormation( this, projectIoRecord );

   m_formations.push_back( m_mantleFormation );

   return m_mantleFormation != 0;
}

bool ProjectHandle::loadBasementSurfaces() {

   m_crustBottomSurface = getFactory()->produceBasementSurface( this, CrustBottomSurfaceName );

   if ( m_crustBottomSurface == 0 ) {
      return false;
   }

   m_surfaces.push_back( m_crustBottomSurface );

   m_mantleBottomSurface = getFactory()->produceBasementSurface( this, MantleBottomSurfaceName );

   if ( m_mantleBottomSurface == 0 ) {
      return false;
   }

   m_surfaces.push_back( m_mantleBottomSurface );

   return true;
}

database::Table* ProjectHandle::getCrustIoTable( void )
{
   database::Table *tbl = getTable( "BasementIoTbl" );
   assert( tbl );

   Record *firstRecord = tbl->getRecord( 0 );
   assert( firstRecord );
   bool isALC = ( getBottomBoundaryModel( firstRecord ) == "Advanced Lithosphere Calculator" );
   database::Table* crustCalculatedThinningTbl = ( isALC ? getTable( "ContCrustalThicknessIoTbl" ) : getTable( "CrustIoTbl" ) );

   return crustCalculatedThinningTbl;
}

const string & ProjectHandle::getCrustIoTableName( void )
{
   return getCrustIoTable()->name();

}

bool ProjectHandle::loadCrustThinningHistory( void )
{
   database::Table* crustThinningTbl = getCrustIoTable();
   database::Table::iterator tblIter;

   for ( tblIter = crustThinningTbl->begin(); tblIter != crustThinningTbl->end(); ++tblIter )
   {
      Record * crustThinningRecord = *tblIter;
      m_crustPaleoThicknesses.push_back( getFactory()->producePaleoFormationProperty( this, crustThinningRecord, m_crustFormation ) );
   }

   // Sort the items in the table into the correct order.
   sort( m_crustPaleoThicknesses.begin(), m_crustPaleoThicknesses.end(), PaleoPropertyTimeLessThan() );

   if ( m_crustPaleoThicknesses.empty() ) {
      return false;
   }

   return true;
}

bool CrustIoTblSorter( database::Record * recordL, database::Record * recordR )
{
   if ( database::getAge( recordL ) < database::getAge( recordR ) ) return true;
   return false;
}

bool ProjectHandle::addCrustThinningHistoryMaps( void ) {

   if ( m_bottomBoundaryConditions == Interface::ADVANCED_LITHOSPHERE_CALCULATOR ) {
      MutablePaleoFormationPropertyList newCrustalThicknesses;

      // calculate and insert additional crust thickness maps at all snapshot events
      MutablePaleoFormationPropertyList::const_iterator thicknessIter2, thicknessIter = m_crustPaleoThicknesses.begin();

      const Interface::Snapshot* oldestSnapshot = getCrustFormation()->getTopSurface()->getSnapshot();
      assert( oldestSnapshot != 0 );

      Interface::SnapshotList* snapshots = getSnapshots( Interface::MAJOR );
      Interface::SnapshotList::const_iterator snapshotIter = snapshots->begin();

      const string tableName = getCrustIoTableName();
      Table * crustIoTbl = getTable( tableName );
      assert( crustIoTbl );

      for ( thicknessIter = m_crustPaleoThicknesses.begin(); thicknessIter != m_crustPaleoThicknesses.end(); ++thicknessIter ) {
         const Interface::GridMap* map1 = ( *thicknessIter )->getMap( Interface::CrustThinningHistoryInstanceThicknessMap );

         thicknessIter2 = thicknessIter + 1;

         const Interface::GridMap* map2 = ( thicknessIter2 != m_crustPaleoThicknesses.end() ?
            ( *thicknessIter2 )->getMap( Interface::CrustThinningHistoryInstanceThicknessMap ) : map1 );
         double age1 = ( *thicknessIter )->getSnapshot()->getTime();
         double age2 = ( thicknessIter2 != m_crustPaleoThicknesses.end() ? ( *thicknessIter2 )->getSnapshot()->getTime() : age1 );

         while ( snapshotIter != snapshots->end() && age2 >= ( *snapshotIter )->getTime() ) {
            double age3 = ( *snapshotIter )->getTime();
            if ( age1 < age3 && age2 != age3 && age3 <= oldestSnapshot->getTime() ) {
               database::Record * record = new Record( *( *thicknessIter )->getRecord() );
               setAge( record, age3 );
               PaleoFormationProperty* crustThicknessMap = getFactory()->producePaleoFormationProperty( this, record, m_crustFormation );
               Interface::InterpolateFunctor functor( age1, age2, age3 );
               crustThicknessMap->computeMap( Interface::CrustThinningHistoryInstanceThicknessMap, map1, map2, functor );

               newCrustalThicknesses.push_back( crustThicknessMap );
               map1 = crustThicknessMap->getMap( Interface::CrustThinningHistoryInstanceThicknessMap );
               age1 = age3;
            }
            ++snapshotIter;
         }
      }
      sort( crustIoTbl->begin(), crustIoTbl->end(), CrustIoTblSorter );
      for ( thicknessIter = newCrustalThicknesses.begin(); thicknessIter != newCrustalThicknesses.end(); ++thicknessIter ) {
         m_crustPaleoThicknesses.push_back( *thicknessIter );
      }
      sort( m_crustPaleoThicknesses.begin(), m_crustPaleoThicknesses.end(), PaleoPropertyTimeLessThan() );

   }

   return true;
}
bool ProjectHandle::loadMantleThicknessHistory( void ) {

   database::Table* crustThinningTbl = getCrustIoTable();
   database::Table::iterator tblIter;

   MutablePaleoFormationPropertyList::const_iterator crustThicknessIter;

   for ( tblIter = crustThinningTbl->begin(); tblIter != crustThinningTbl->end(); ++tblIter ) {

      PaleoFormationProperty* mantleThicknessMap;

      // Here the crust-thinning table-record is used only to get the snapshot-time.
      Record * crustThinningRecord = *tblIter;

      mantleThicknessMap = getFactory()->producePaleoFormationProperty( this, crustThinningRecord, m_mantleFormation );
      m_mantlePaleoThicknesses.push_back( mantleThicknessMap );
   }

   return true;
}

bool ProjectHandle::loadSurfaceTemperatureHistory( void )
{
   database::Table* surfaceTemperatureTbl = getTable( "SurfaceTempIoTbl" );
   database::Table::iterator tblIter;

   for ( tblIter = surfaceTemperatureTbl->begin(); tblIter != surfaceTemperatureTbl->end(); ++tblIter )
   {
      Record * surfaceTemperatureRecord = *tblIter;
      m_surfaceTemperatureHistory.push_back( getFactory()->producePaleoProperty( this, surfaceTemperatureRecord ) );
   }

   // Sort into correct order. Youngest first on the list.
   std::sort( m_surfaceTemperatureHistory.begin(), m_surfaceTemperatureHistory.end(), PaleoPropertyTimeLessThan() );

   return true;
}

bool ProjectHandle::loadSurfaceDepthHistory( void )
{
   database::Table* surfaceDepthTbl = getTable( "SurfaceDepthIoTbl" );
   database::Table::iterator tblIter;

   // Should the present day depth at top surface be added here?
   for ( tblIter = surfaceDepthTbl->begin(); tblIter != surfaceDepthTbl->end(); ++tblIter )
   {
      Record * surfaceDepthRecord = *tblIter;
      m_surfaceDepthHistory.push_back( getFactory()->producePaleoProperty( this, surfaceDepthRecord ) );
   }

   // Sort into correct order. Youngest first on the list.
   std::sort( m_surfaceDepthHistory.begin(), m_surfaceDepthHistory.end(), PaleoPropertyTimeLessThan() );

   return true;
}

bool ProjectHandle::loadRunParameters( void )
{
   database::Table* runParametersIoTbl = getTable( "RunOptionsIoTbl" );

   if ( runParametersIoTbl == 0 ) {
      return false;
   }

   Record* runParametersRecord = runParametersIoTbl->getRecord( 0 );

   if ( runParametersRecord == 0 ) {
      return false;
   }
   else {
      m_runParameters = getFactory()->produceRunParameters( this, runParametersRecord );
      return true;
   }

}

bool ProjectHandle::loadProjectData( void )
{
   database::Table* projectDataIoTbl = getTable( "ProjectIoTbl" );

   if ( projectDataIoTbl == 0 ) {
      return false;
   }

   Record* projectDataRecord = projectDataIoTbl->getRecord( 0 );

   if ( projectDataRecord == 0 ) {
      return false;
   }
   else {
      m_projectData = getFactory()->produceProjectData( this, projectDataRecord );
      return true;
   }

}

bool ProjectHandle::loadSimulationDetails () {

   database::Table* simulationDetailsIoTbl = getTable( "SimulationDetailsIoTbl" );
   database::Table::iterator tblIter;

   if ( simulationDetailsIoTbl == 0 ) {
      return false;
   }

   for ( tblIter = simulationDetailsIoTbl->begin (); tblIter != simulationDetailsIoTbl->end (); ++tblIter ) {
      Record* simulationDetailsRecord = *tblIter;
      m_simulationDetails.push_back ( getFactory ()->produceSimulationDetails ( this, simulationDetailsRecord ));
   }

   std::sort ( m_simulationDetails.begin (), m_simulationDetails.end (), SimulationDetailsComparison ());

   return true;
}


bool ProjectHandle::loadBottomBoundaryConditions( void )
{
   database::Table * projectIoTbl = 0;

   // try to get it from the BasementIoTbl
   projectIoTbl = getTable( "BasementIoTbl" );

   if ( projectIoTbl == 0 ) {
      return false;
   }

   Record *projectIoRecord = projectIoTbl->getRecord( 0 );

   assert( projectIoRecord );
   m_crustPropertyModel = "";
   m_mantlePropertyModel = "";

   const string& theBottomBCsStr = database::getBottomBoundaryModel( projectIoRecord );

   if ( theBottomBCsStr == "Fixed Temperature" )
   {
      m_bottomBoundaryConditions = Interface::FIXED_BASEMENT_TEMPERATURE; //declared in Interface.h 
   }
   else if ( theBottomBCsStr == "Fixed HeatFlow" )
   {
      m_bottomBoundaryConditions = Interface::MANTLE_HEAT_FLOW; //declared in Interface.h 
   }
   else if ( theBottomBCsStr == "Advanced Lithosphere Calculator" )
   {
      m_bottomBoundaryConditions = Interface::ADVANCED_LITHOSPHERE_CALCULATOR;
      if ( ddd::GetRank() == 0 ) {
         cout << "The calculation mode is = " << theBottomBCsStr << endl;
      }
      m_crustPropertyModel = database::getCrustPropertyModel( projectIoRecord );
      m_mantlePropertyModel = database::getMantlePropertyModel( projectIoRecord );
   }
   return true;
}

Interface::BottomBoundaryConditions ProjectHandle::getBottomBoundaryConditions() const {
   return m_bottomBoundaryConditions;
}

const string & ProjectHandle::getCrustPropertyModel() const {
   return m_crustPropertyModel;
}

const string & ProjectHandle::getMantlePropertyModel() const {
   return m_mantlePropertyModel;
}

double ProjectHandle::getBottomMantleTemperature() const {

   database::Table* basementIoTbl;
   database::Record* basementTableRecord;

   basementIoTbl = getTable( "BasementIoTbl" );
   basementTableRecord = basementIoTbl->getRecord( 0 );

   return database::getTopAsthenoTemp( basementTableRecord );
}


bool ProjectHandle::loadTouchstoneMaps( void )
{
   database::Table* touchstoneMapTbl = getTable( "TouchstoneMapIoTbl" );
   database::Table::iterator tblIter;
   for ( tblIter = touchstoneMapTbl->begin(); tblIter != touchstoneMapTbl->end(); ++tblIter )
   {
      Record * touchstoneMapRecord = *tblIter;
      m_touchstoneMaps.push_back( getFactory()->produceTouchstoneMap( this, touchstoneMapRecord ) );
   }

   return true;
}

bool ProjectHandle::loadAllochthonousLithologies( void )
{
   database::Table * allochthonousLithoTbl = getTable( "AllochthonLithoIoTbl" );
   database::Table::iterator tblIter;

   for ( tblIter = allochthonousLithoTbl->begin(); tblIter != allochthonousLithoTbl->end(); ++tblIter )
   {

      Record * allochthonousLithoRecord = *tblIter;

      m_allochthonousLithologies.push_back( getFactory()->produceAllochthonousLithology( this, allochthonousLithoRecord ) );

   }
   return true;
}

bool ProjectHandle::loadAllochthonousLithologyDistributions( void )
{
   database::Table * allochthonousLithoDistTbl = getTable( "AllochthonLithoDistribIoTbl" );
   database::Table::iterator tblIter;

   for ( tblIter = allochthonousLithoDistTbl->begin(); tblIter != allochthonousLithoDistTbl->end(); ++tblIter )
   {
      Record * allochthonousLithoDistRecord = *tblIter;

      m_allochthonousLithologyDistributions.push_back( getFactory()->produceAllochthonousLithologyDistribution( this, allochthonousLithoDistRecord ) );
   }

   sort( m_allochthonousLithologyDistributions.begin(), m_allochthonousLithologyDistributions.end(), AllochthonousLithologyDistributionTimeLessThan() );

   return true;
}

bool ProjectHandle::loadAllochthonousLithologyInterpolations( void )
{
   database::Table * allochthonousLithoInterpTbl = getTable( "AllochthonLithoInterpIoTbl" );
   database::Table::iterator tblIter;

   for ( tblIter = allochthonousLithoInterpTbl->begin(); tblIter != allochthonousLithoInterpTbl->end(); ++tblIter )
   {
      Record * allochthonousLithoInterpRecord = *tblIter;
      m_allochthonousLithologyInterpolations.push_back( getFactory()->produceAllochthonousLithologyInterpolation( this, allochthonousLithoInterpRecord ) );
   }
   return true;
}


bool ProjectHandle::loadTraps( void )
{
   database::Table* trapTbl = getTable( "TrapIoTbl" );
   database::Table::iterator tblIter;
   for ( tblIter = trapTbl->begin(); tblIter != trapTbl->end(); ++tblIter )
   {
      Record * trapRecord = *tblIter;
      m_traps.push_back( getFactory()->produceTrap( this, trapRecord ) );
   }
   return true;
}

struct CmpTrapId
{
  bool operator()(const Trapper* lhs, const Trapper* rhs) const
  {
    return lhs->getId() < rhs->getId();
  }
};

bool ProjectHandle::loadTrappers( void )
{
   database::Table* trapperTbl = getTable( "TrapperIoTbl" );
   database::Table::iterator tblIter;
   for ( tblIter = trapperTbl->begin(); tblIter != trapperTbl->end(); ++tblIter )
   {
      Record * trapperRecord = *tblIter;
      m_trappers.push_back( getFactory()->produceTrapper( this, trapperRecord ) );
   }

   // Sort trappers on id, so we can retrieve them more efficiently

   std::sort(m_trappers.begin(), m_trappers.end(), CmpTrapId());

   return true;
}

bool ProjectHandle::loadMigrations( void )
{
   database::Table* migrationTbl = getTable( "MigrationIoTbl" );
   database::Table::iterator tblIter;
   for ( tblIter = migrationTbl->begin(); tblIter != migrationTbl->end(); ++tblIter )
   {
      Record * migrationRecord = *tblIter;
      m_migrations.push_back( getFactory()->produceMigration( this, migrationRecord ) );
   }
   return true;
}

bool ProjectHandle::loadInputValues( void )
{
   database::Table* gridMapTbl = getTable( "GridMapIoTbl" );
   database::Table::iterator tblIter;
   for ( tblIter = gridMapTbl->begin(); tblIter != gridMapTbl->end(); ++tblIter )
   {
      Record * gridMapRecord = *tblIter;
      m_inputValues.push_back( getFactory()->produceInputValue( this, gridMapRecord ) );
   }
   return true;
}

/// load the 2-D PropertyValues
bool ProjectHandle::loadMapPropertyValues( void )
{
   using Interface::FORMATIONPROPERTY;
   using Interface::RESERVOIRPROPERTY;

   database::Table* timeIoTbl = getTable( "TimeIoTbl" );
   database::Table::iterator tblIter;
   for ( tblIter = timeIoTbl->begin(); tblIter != timeIoTbl->end(); ++tblIter )
   {
      const Property * property;
      Record * timeIoRecord = *tblIter;
      const string & propertyValueName = database::getPropertyName( timeIoRecord );

      if ( ( property = (const Property *)findProperty( propertyValueName ) ) == 0 )
      {
         Interface::PropertyType propertyType;
         if ( strncmp( propertyValueName.c_str(), "ResRock", 7 ) != 0 )
            propertyType = Interface::FORMATIONPROPERTY;
         else
            propertyType = Interface::RESERVOIRPROPERTY;

         cerr << "WARNING: ProjectHandle::loadMapPropertyValues: Could not find property named: " << propertyValueName << ", creating it on the fly" << endl;
         addProperty( getFactory()->produceProperty( this, 0, propertyValueName, propertyValueName, "", propertyType, DataModel::UNKNOWN_PROPERTY_ATTRIBUTE ) );
         property = (const Property *)findProperty( propertyValueName );

      }

      if ( property )
      {
         double time = database::getTime( timeIoRecord );

         const Snapshot * snapshot = (const Snapshot *)findSnapshot( time, MAJOR | MINOR );
         if ( snapshot == 0 )
         {
            continue;
         }

         if ( property->getType() == FORMATIONPROPERTY )
         {
            const string & surfaceName = database::getSurfaceName( timeIoRecord );
            const string & formationName = database::getFormationName( timeIoRecord );

            const Formation * formation = dynamic_cast<const Formation *>( findFormation( formationName ) );
            const Surface * surface = dynamic_cast<const Surface *>( findSurface( surfaceName ) );
            if ( formation == 0 && surface == 0 ) continue;

            addPropertyValue( timeIoRecord, propertyValueName, property, snapshot, 0, formation, surface, TIMEIOTBL );
         }
         else if ( property->getType() == RESERVOIRPROPERTY )
         {
            const string & reservoirName = database::getFormationName( timeIoRecord );

            const Reservoir * reservoir = (const Reservoir *)findReservoir( reservoirName );
            if ( reservoir == 0 ) continue;

            addPropertyValue( timeIoRecord, propertyValueName, property, snapshot, reservoir, 0, 0, TIMEIOTBL );
         }
      }
   }
   return true;
}

bool ProjectHandle::initializeMapPropertyValuesWriter( const bool append )
{
   if ( Interface::MODE3D != getModellingMode() ) return true;
   if ( m_mapPropertyValuesWriter ) return false;

   // create hdf file
   string fileName = getActivityName();

   fileName += "_Results.HDF";
   ibs::FilePath ppath( getFullOutputDir() );
   ppath << fileName;
   string filePathName = ppath.path();

   if ( !makeOutputDir() ) return false;

   m_mapPropertyValuesWriter = getFactory()->produceMapWriter();
   bool status = m_mapPropertyValuesWriter->open( filePathName, append );
   if ( status ) {
      status = m_mapPropertyValuesWriter->saveDescription( saveAsInputGrid() ? getInputGrid() : getActivityOutputGrid() );
   }
   return status;
}

bool ProjectHandle::initializePrimaryPropertyValuesWriter( const bool append )
{
   if ( Interface::MODE3D != getModellingMode() ) return true;

   return true;
}

bool TimeIoTblSorter( database::Record * recordL, database::Record * recordR );

bool ProjectHandle::finalizeMapPropertyValuesWriter( void )
{
   if ( Interface::MODE3D != getModellingMode() )
   {
      database::Table * timeIoTbl = getTable( "TimeIoTbl" );
      if ( !timeIoTbl ) return false;

      sort( timeIoTbl->begin(), timeIoTbl->end(), TimeIoTblSorter );
      return true;
   }
   else
   {
      if ( !m_mapPropertyValuesWriter )
         return false;

      m_mapPropertyValuesWriter->close();
      delete m_mapPropertyValuesWriter;

      return true;
   }
}

bool TimeIoTblSorter( database::Record * recordL, database::Record * recordR )
{
   if ( database::getPropertyName( recordL ) < database::getPropertyName( recordR ) ) return true;
   if ( database::getPropertyName( recordL ) > database::getPropertyName( recordR ) ) return false;
   if ( database::getTime( recordL ) < database::getTime( recordR ) ) return true;
   if ( database::getTime( recordL ) > database::getTime( recordR ) ) return false;
   if ( database::getDepoSequence( recordL ) < database::getDepoSequence( recordR ) ) return false;
   if ( database::getDepoSequence( recordL ) > database::getDepoSequence( recordR ) ) return true;

   return false;
}

/// Write newly created volume properties to depthiotbl file.
bool ProjectHandle::saveCreatedMapPropertyValues( void )
{
   if ( Interface::MODE3D == getModellingMode() )
   {
      return saveCreatedMapPropertyValuesMode3D();
   }
   else
   {
      return saveCreatedMapPropertyValuesMode1D();
   }
}


//1DComponent
bool ProjectHandle::saveCreatedMapPropertyValuesMode1D( void )
{
   database::Table * timeIoTbl = getTable( "TimeIoTbl" );
   if ( !timeIoTbl )
      return false;

   MutablePropertyValueList::iterator propertyValueIter;

   int increment = 1;
   for ( propertyValueIter = m_recordLessMapPropertyValues.begin();
      propertyValueIter != m_recordLessMapPropertyValues.end(); propertyValueIter += increment )
   {
      PropertyValue *propertyValue = *propertyValueIter;

      if ( !propertyValue->toBeSaved() )
      {
         increment = 1;
         continue;
      }
      //1DComponent
      propertyValue->createTimeIoRecord( timeIoTbl, Interface::MODE1D );
      m_propertyValues.push_back( propertyValue );
      propertyValueIter = m_recordLessMapPropertyValues.erase( propertyValueIter );
      increment = 0;
   }

   return true;
}

//1DComponent
bool ProjectHandle::saveCreatedMapPropertyValuesMode3D( void )
{
   database::Table * timeIoTbl = getTable( "TimeIoTbl" );
   if ( !timeIoTbl || !m_mapPropertyValuesWriter )
      return false;

   MutablePropertyValueList::iterator propertyValueIter;

   int increment = 1;
   for ( propertyValueIter = m_recordLessMapPropertyValues.begin();
      propertyValueIter != m_recordLessMapPropertyValues.end(); propertyValueIter += increment )
   {
      PropertyValue *propertyValue = *propertyValueIter;

      if ( !propertyValue->toBeSaved() )
      {
         increment = 1;
         continue;
      }

      propertyValue->createTimeIoRecord( timeIoTbl, Interface::MODE3D );
      m_propertyValues.push_back( propertyValue );
      propertyValueIter = m_recordLessMapPropertyValues.erase( propertyValueIter );
      increment = 0;

      bool saveAsPrimary = false;
      if( m_primaryDouble and propertyValue->isPrimary() ) {
         if(( getActivityName() != "Genex5" and getActivityName() != "HighResMigration" and 
              getActivityName() != "FastTouch" and getActivityName() != "CrustalThicknessCalculator" )) {
            saveAsPrimary = true;
         }
      }   
      propertyValue->saveMapToFile( *m_mapPropertyValuesWriter, saveAsPrimary); // depends on success of createRecord ()
   }

   // sort (timeIoTbl->begin (), timeIoTbl->end (), TimeIoTblSorter);
   return true;
}

bool ProjectHandle::saveCreatedVolumePropertyValues( void )
{
   if ( Interface::MODE3D == getModellingMode() )
   {
      return saveCreatedVolumePropertyValuesMode3D();
   }
   else
   {
      saveCreatedVolumePropertyValuesMode1DOld();
      return saveCreatedVolumePropertyValuesMode1D();
   }
}

/// Write newly created volume properties to snapshot file.
/// This function assumes that all volume properties for a given snapshot are written in one go
/// and that the snapshot file can be re-created.
bool ProjectHandle::saveCreatedVolumePropertyValuesMode3D( void )
{
   database::Table * timeIoTbl = getTable( "3DTimeIoTbl" );
   if ( !timeIoTbl ) return false;

   MutablePropertyValueList::iterator propertyValueIter;
   MapWriter* mapWriter = getFactory()->produceMapWriter();

   bool status = true;

   while ( true )
   {
      Snapshot * snapshotUsed = 0;

      int increment = 1;
      for ( propertyValueIter = m_recordLessVolumePropertyValues.begin();
         propertyValueIter != m_recordLessVolumePropertyValues.end(); propertyValueIter += increment )
      {
         PropertyValue *propertyValue = *propertyValueIter;

         if ( ( snapshotUsed && propertyValue->getSnapshot() != snapshotUsed ) ||
            !propertyValue->toBeSaved() )
         {
            increment = 1;
            continue;
         }

         if ( !snapshotUsed ) 
         {
            // let's use this propertyValue's snapshot during this iteration
            // and open a (new, empty) snapshot file for it.
            // File will be appended if append-flag is true in the snapshot.
            snapshotUsed = (Snapshot *)propertyValue->getSnapshot();
 
            const string & fileName = snapshotUsed->getFileName( true );
            ibs::FilePath filePathName( getFullOutputDir() );
            filePathName << fileName;               


            
#if 0
               cerr << "Saving snapshot ";
               snapshotUsed->printOn (cerr);
               cerr << " to file " << filePathName.path() << "  " << (snapshotUsed->getAppendFile () ? "APPEND" : "CREATE" ) << endl;
#endif
            
               mapWriter->open( filePathName.path(), snapshotUsed->getAppendFile() );
         }
         
         propertyValue->create3DTimeIoRecord( timeIoTbl, Interface::MODE3D );
         m_propertyValues.push_back( propertyValue );
         propertyValueIter = m_recordLessVolumePropertyValues.erase( propertyValueIter );
         increment = 0;

         if( m_primaryDouble and propertyValue->isPrimary() and snapshotUsed->getType() == Interface::MAJOR  ) {
            // save primary propeties in double precision and chunked at major snapshots
            status &= propertyValue->savePrimaryVolumeToFile( *mapWriter, false );
         } else {
            status &= propertyValue->saveVolumeToFile( *mapWriter );
         }
         
      }

      if ( !snapshotUsed ) break; // nothing was written

      mapWriter->close();
   }

   delete mapWriter;
   return status;
}

bool DepthIoTblSorter( database::Record * recordL, database::Record * recordR )
{
   if ( database::getPropertyName( recordL ) < database::getPropertyName( recordR ) ) return true;
   if ( database::getPropertyName( recordL ) > database::getPropertyName( recordR ) ) return false;
   int leftIndex = recordL->getValue<int>( "DepositionSequence" );
   int rightIndex = recordR->getValue<int>( "DepositionSequence" );
   if ( leftIndex > rightIndex ) return true;
   if ( leftIndex < rightIndex ) return false;
   if ( database::getDepth_( recordL ) < database::getDepth_( recordR ) ) return true;
   if ( database::getDepth_( recordL ) > database::getDepth_( recordR ) ) return false;

   return false;
}

bool ProjectHandle::saveCreatedVolumePropertyValuesMode1D( void )
{
   database::Table * timeIoTbl = getTable( "1DTimeIoTbl" );
   if ( !timeIoTbl ) return false;

   MutablePropertyValueList::iterator propertyValueIter;

   bool status = true;

   int index = 0;
   for ( propertyValueIter = m_recordLessVolumePropertyValues.begin();
      propertyValueIter != m_recordLessVolumePropertyValues.end(); ++propertyValueIter, ++index )
   {
      PropertyValue *propertyValue = *propertyValueIter;

      if ( !propertyValue->toBeSaved() ) continue;

      GridMap *gridMap = (GridMap *)propertyValue->getGridMap();

      gridMap->retrieveData();

      unsigned int gridMapDepth = gridMap->getDepth();

      for ( int k = gridMapDepth - 1; k >= 0; --k )
      {

         double value = gridMap->getValue( 0, 0, (unsigned int)k );
         if ( value == gridMap->getUndefinedValue() )
         {
            value = DefaultUndefinedScalarValue; // or, should we just not output?
         }

         Record * timeIoRecord = propertyValue->create1DTimeIoRecord( timeIoTbl, Interface::MODE1D );

         if ( k == 0 )
         {
            database::setSurfaceName( timeIoRecord, propertyValue->getFormation()->getBottomSurfaceName() );
         }
         if ( k == static_cast<int>(gridMapDepth - 1) )
         {
            database::setSurfaceName( timeIoRecord, propertyValue->getFormation()->getTopSurfaceName() );
         }

         database::setNodeIndex( timeIoRecord, k );

         database::setValue( timeIoRecord, value );
      }

      gridMap->restoreData();

      m_propertyValues.push_back( propertyValue );
   }
   m_recordLessVolumePropertyValues.clear();

   return status;
}

/// Write newly created volume properties to depthiotbl file.
/// This function assumes that all volume properties for a given snapshot are written in one go
bool ProjectHandle::saveCreatedVolumePropertyValuesMode1DOld( void )
{
   char * scalarPostfixes[ 2 ] = { "", "" };
   char * vecPostfixes[ 2 ] = { "[0]", "[1]" };

   const Snapshot *zeroSnapshot = (const Snapshot *)findSnapshot( 0 );

   database::Table * depthIoTbl = getTable( "DepthIoTbl" );
   if ( !depthIoTbl )
      return false;

   if ( m_recordLessVolumePropertyValues.size() == 0 ) return true;

   MutablePropertyValueList::iterator depthPropertyValueIter;
   MutablePropertyValueList::iterator propertyValueIter;

   const Property *depthProperty = (const Property *)findProperty( "Depth" );

   assert( depthProperty );

   bool status = true;

   MutablePropertyValueList selectedPropertyValues;
   MutablePropertyValueList selectedDepthPropertyValues;


   // first find the set of all property values with the same snapshot and formation and pinpoint the depth property value in that set.
   for ( propertyValueIter = m_recordLessVolumePropertyValues.begin();
      propertyValueIter != m_recordLessVolumePropertyValues.end(); ++propertyValueIter )
   {
      PropertyValue *propertyValue = *propertyValueIter;

      if ( propertyValue->getSnapshot() != zeroSnapshot || !propertyValue->toBeSaved() )
      {
         continue;
      }

      selectedPropertyValues.push_back( propertyValue );

      if ( propertyValue->getProperty() == depthProperty )
      {
         selectedDepthPropertyValues.push_back( propertyValue );
      }
   }

   const Property *property = 0;

   double previousDepthValue[ 2 ] = { DefaultUndefinedScalarValue, DefaultUndefinedScalarValue };

   for ( depthPropertyValueIter = selectedDepthPropertyValues.begin();
      depthPropertyValueIter != selectedDepthPropertyValues.end(); ++depthPropertyValueIter )
   {
      PropertyValue *depthPropertyValue = *depthPropertyValueIter;

      GridMap *depthGridMap = (GridMap *)depthPropertyValue->getGridMap();

      depthGridMap->retrieveData();

      bool isZeroThickness = ( depthGridMap->getValue( 0, 0, (unsigned int)0 ) == depthGridMap->getValue( 0, 0, (unsigned int)depthGridMap->getDepth() - 1 ) );

      for ( propertyValueIter = selectedPropertyValues.begin();
         propertyValueIter != selectedPropertyValues.end(); ++propertyValueIter )
      {
         PropertyValue *propertyValue = *propertyValueIter;

         if ( propertyValue->getFormation() != depthPropertyValue->getFormation() )
         {
            continue;
         }


         property = (const Property *)propertyValue->getProperty();

         int nrOutputs = 1;

         char ** postFixes = scalarPostfixes;

         if ( property->getCauldronName().rfind( "Vec2" ) != string::npos )
         {
            nrOutputs = 2;
            postFixes = vecPostfixes;
         }

         if ( isZeroThickness ) continue;

         GridMap *gridMap = (GridMap *)propertyValue->getGridMap();

         if ( gridMap != depthGridMap )
         {
            gridMap->retrieveData();
         }

         unsigned int gridMapDepth = gridMap->getDepth();

         assert( gridMapDepth == depthGridMap->getDepth() );

         for ( int k = gridMapDepth - 1; k >= 0; --k )
         {
            assert( depthGridMap->firstI() == 0 );
            assert( depthGridMap->firstJ() == 0 );

            double depthValue = depthGridMap->getValue( 0, 0, (unsigned int)k );
            double value = gridMap->getValue( 0, 0, (unsigned int)k );
            if ( depthValue != depthGridMap->getUndefinedValue() )
            {
               if ( value == gridMap->getUndefinedValue() )
               {
                  value = DefaultUndefinedScalarValue;
               }
               for ( int i = 0; i < nrOutputs; ++i )
               {
                  if ( depthValue != DefaultUndefinedScalarValue && depthValue > previousDepthValue[ i ] - 0.01 && depthValue < previousDepthValue[ i ] + 0.01 ) continue;

                  previousDepthValue[ i ] = depthValue;

                  database::Record * depthIoRecord = depthIoTbl->createRecord();

                  depthIoRecord->setValue( "DepositionSequence", propertyValue->getFormation()->getDepositionSequence() );
                  database::setPropertyName( depthIoRecord, property->getCauldronName() + postFixes[ i ] );
                  database::setTime( depthIoRecord, zeroSnapshot->getTime() );
                  database::setDepth_( depthIoRecord, depthValue );
                  database::setAverage( depthIoRecord, value );
                  database::setStandardDev( depthIoRecord, DefaultUndefinedScalarValue );
                  database::setMinimum( depthIoRecord, value );
                  database::setMaximum( depthIoRecord, value );
                  database::setSum( depthIoRecord, value );
                  database::setSum2( depthIoRecord, DefaultUndefinedScalarValue );
                  database::setNP( depthIoRecord, static_cast<int>( DefaultUndefinedScalarValue ) );
                  database::setP15( depthIoRecord, DefaultUndefinedScalarValue );
                  database::setP50( depthIoRecord, DefaultUndefinedScalarValue );
                  database::setP85( depthIoRecord, DefaultUndefinedScalarValue );
                  database::setSumFirstPower( depthIoRecord, value );
                  database::setSumSecondPower( depthIoRecord, DefaultUndefinedScalarValue );
                  database::setSumThirdPower( depthIoRecord, DefaultUndefinedScalarValue );
                  database::setSumFourthPower( depthIoRecord, DefaultUndefinedScalarValue );
                  database::setSkewness( depthIoRecord, DefaultUndefinedScalarValue );
                  database::setKurtosis( depthIoRecord, DefaultUndefinedScalarValue );


#if 0
                  cerr << "<< " << database::getPropertyName (depthIoRecord) << "(" << database::getDepth_ (depthIoRecord) << ", " << database::getAverage (depthIoRecord) <<
                     ") of layer " << propertyValue->getFormation ()->getName () << endl;
#endif
               }
            }
         }

         if ( gridMap != depthGridMap ) gridMap->restoreData();
      }

      depthGridMap->restoreData();
   }

   // Remove all records that should not be there
   sort( depthIoTbl->begin(), depthIoTbl->end(), DepthIoTblSorter );

   int removalIncrement = 1;
   bool removeNextOne = false;
   for ( database::Table::iterator removalIter = depthIoTbl->begin(); removalIter != depthIoTbl->end(); removalIter += removalIncrement )
   {
      removalIncrement = 1;
      database::Record * currentRecord = *removalIter;

      if ( removeNextOne )
      {
         removalIter = depthIoTbl->removeRecord( removalIter );
         removalIncrement = 0;
         removeNextOne = false;
         continue;
      }

      if ( removalIter + 1 != depthIoTbl->end() )
      {
         database::Record * nextCurrentRecord = *( removalIter + 1 );
         if ( database::getPropertyName( currentRecord ) != database::getPropertyName( nextCurrentRecord ) )
         {
            // removal of any last ...Vec2[0] property
            if ( database::getPropertyName( currentRecord ).find( "Vec2[0]" ) != string::npos )
            {
               removalIter = depthIoTbl->removeRecord( removalIter );
               removalIncrement = 0;
            }
            // removal of any first ...Vec2[1] property
            if ( database::getPropertyName( nextCurrentRecord ).find( "Vec2[1]" ) != string::npos )
            {
               removeNextOne = true;
            }
         }
         else
         {
            // removal of any second Vec2[1] property with the same depth
            if ( database::getPropertyName( currentRecord ).rfind( "Vec2[1]" ) != string::npos && database::getDepth_( currentRecord ) == database::getDepth_( nextCurrentRecord ) )
            {
               removeNextOne = true;
            }
            // removal of any first Vec2[0] property with the same depth
            else if ( database::getPropertyName( currentRecord ).rfind( "Vec2[0]" ) != string::npos && database::getDepth_( currentRecord ) == database::getDepth_( nextCurrentRecord ) )
            {
               removalIter = depthIoTbl->removeRecord( removalIter );
               removalIncrement = 0;
            }
            // removal of any second non-Vec2 properties with equal depth
            else if ( database::getDepth_( currentRecord ) == database::getDepth_( nextCurrentRecord ) &&
               ( database::getPropertyName( currentRecord ).rfind( "Vec2" ) == string::npos ) )
            {
               removeNextOne = true;
            }
         }
      }
   }

   // Remove properties with undefined value
   for ( database::Table::iterator removalIter = depthIoTbl->begin(); removalIter != depthIoTbl->end(); removalIter += removalIncrement )
   {
      removalIncrement = 1;
      database::Record * currentRecord = *removalIter;
      if ( database::getAverage( currentRecord ) == DefaultUndefinedScalarValue )
      {
         removalIter = depthIoTbl->removeRecord( removalIter );
         removalIncrement = 0;
      }
   }

   return status;
}

void ProjectHandle::addProperty( Property * property )
{
   m_properties.push_back( property );
}

void ProjectHandle::addPropertyToFront( Property * property )
{
   MutablePropertyList::iterator propertyIter = m_properties.begin();
 
   m_properties.insert( propertyIter, property );
}

PropertyValue * ProjectHandle::addPropertyValue( database::Record * record, const string & name, const Property * property, const Snapshot * snapshot,
   const Reservoir * reservoir, const Formation * formation, const Surface * surface, PropertyStorage storage )
{
   PropertyValue * propertyValue = getFactory()->producePropertyValue( this, record, name, property, snapshot,
      reservoir, formation, surface, storage );

   if ( record )
   {
      m_propertyValues.push_back( propertyValue );
   }
   else
   {
      if ( storage == THREEDTIMEIOTBL )
         m_recordLessVolumePropertyValues.push_back( propertyValue );
      else
         m_recordLessMapPropertyValues.push_back( propertyValue );
   }

   return propertyValue;
}

PropertyValue * ProjectHandle::createMapPropertyValue( const string &    propertyValueName,
   const Snapshot *  snapshot,
   const Reservoir * reservoir,
   const Formation * formation,
   const Surface *   surface )
{
   if ( getActivityName() == "" || getActivityOutputGrid() == 0 ) {
      return 0;
   }

   const Property * property = (const Property *)findProperty( propertyValueName );

   if ( !property ) {
      return 0;
   }

   if ( reservoir && ( surface || formation ) ) {
      return 0;
   }

   if ( !reservoir && !surface && !formation ) {
      return 0;
   }

   PropertyValue * propertyValue = addPropertyValue( 0, propertyValueName, property, snapshot, reservoir, formation, surface, TIMEIOTBL );
   propertyValue->createGridMap( getActivityOutputGrid(), 1 );

   return propertyValue;
}

PropertyValue * ProjectHandle::createVolumePropertyValue( const string & propertyValueName, const Snapshot * snapshot,
   const Reservoir * reservoir, const Formation * formation, unsigned int depth )
{
   if ( getActivityName() == "" || getActivityOutputGrid() == 0 ) return false;

   const Property * property = (const Property *)findProperty( propertyValueName );
   if ( !property ) return false;

   if ( reservoir && formation ) return false;
   if ( !reservoir && !formation ) return false;

   PropertyValue * propertyValue = addPropertyValue( 0, propertyValueName, property, snapshot, reservoir, formation, 0, THREEDTIMEIOTBL );
   propertyValue->createGridMap( getActivityOutputGrid(), depth );

   return propertyValue;
}

/// Function used to iterate over all the Property volumes in a HDF5 file
/// Does not actually download the values of a Property, this is done on demand only.
static herr_t AddVolumePropertyValue( hid_t groupId, const char * formationName, ProjectHandle * projectHandle )
{
   (void) groupId; // ignore compulsary parameter groupId

   const Formation * formation = dynamic_cast<const Formation *>( projectHandle->findFormation( formationName ) );
   if ( formation != 0 )
   {
#if 0
      cerr << "Found formation " << formationName
         << " while trying to add VolumePropertyValue " << projectHandle->getCurrentPropertyValueName ()
         << " at snapshot " << projectHandle->getCurrentSnapshot ()->getTime () << endl;
#endif

      projectHandle->addPropertyValue( projectHandle->getCurrentSnapshot()->getRecord(),
         projectHandle->getCurrentPropertyValueName(),
         projectHandle->getCurrentProperty(), projectHandle->getCurrentSnapshot(),
         0, formation, 0, SNAPSHOTIOTBL );
   }
   else
   {
#if 0
      cerr << "ERROR: Could not find formation " << formationName
         << " while trying to add VolumePropertyValue " << projectHandle->getCurrentPropertyValueName ()
         << " at snapshot " << projectHandle->getCurrentSnapshot ()->getTime () << endl;
#endif
   }
   return 0;
}

/// Function used to iterate over all the Properties in a HDF5 file
static herr_t ListVolumePropertyValues( hid_t groupId, const char * propertyValueName, ProjectHandle * projectHandle )
{
#if 0
   cerr << "Found property " << propertyValueName << " in group " << groupId << endl;
#endif

   const Property * property = (const Property *)projectHandle->findProperty( propertyValueName );
   if ( property == 0 )
   {
      Interface::PropertyType propertyType;
      if ( strncmp( propertyValueName, "ResRock", 7 ) != 0 )
         propertyType = Interface::FORMATIONPROPERTY;
      else
         propertyType = Interface::RESERVOIRPROPERTY;

      cerr << "WARNING: ListVolumePropertyValues: Could not find property named: " << propertyValueName << ", creating it on the fly" << endl;
      projectHandle->addProperty( projectHandle->getFactory()->produceProperty( projectHandle, 0, propertyValueName, propertyValueName, "", propertyType, DataModel::UNKNOWN_PROPERTY_ATTRIBUTE ) );
      property = (const Property *)projectHandle->findProperty( propertyValueName );
   }

   if ( property != 0 )
   {
      projectHandle->setCurrentProperty( property );
      projectHandle->setCurrentPropertyValueName( propertyValueName );

      H5Giterate( groupId, propertyValueName, NULL, (H5G_iterate_t)AddVolumePropertyValue, (void *)projectHandle );
   }
   return 0;
}

/// load the 3-D PropertyValues
/// Does not load the actual values of thr PropertyValues, this is done on demand only.
bool ProjectHandle::loadVolumePropertyValues( void )
{
   if ( Interface::MODE1D == getModellingMode() ) return false;

   database::Table* timeIoTbl = getTable( "3DTimeIoTbl" );
   if ( timeIoTbl == 0 || timeIoTbl->size() == 0 )
   {
      return loadVolumePropertyValuesViaSnapshotIoTbl();
   }
   else
   {
      return loadVolumePropertyValuesVia3DTimeIoTbl();
   }
}

bool ProjectHandle::loadVolumePropertyValuesVia3DTimeIoTbl( void )
{
   database::Table * timeIoTbl = getTable( "3DTimeIoTbl" );
   database::Table::iterator tblIter;
   for ( tblIter = timeIoTbl->begin(); tblIter != timeIoTbl->end(); ++tblIter )
   {
      Record *timeIoRecord = *tblIter;
      const string & propertyValueName = database::getPropertyName( timeIoRecord );

      const Property *property = findProperty( propertyValueName );
      if ( property == 0 )
      {
         PropertyType propertyType = Interface::FORMATIONPROPERTY;

         cerr << "WARNING: loadVolumePropertyValuesVia3DTimeIoTbl: Could not find property named: " << propertyValueName << ", creating it on the fly" << endl;
         addProperty( getFactory()->produceProperty( this, 0, propertyValueName, propertyValueName, "", propertyType, DataModel::UNKNOWN_PROPERTY_ATTRIBUTE ) );
         property = findProperty( propertyValueName );
      }

      const string & formationName = database::getFormationName( timeIoRecord );
      const Formation * formation = findFormation( formationName );
      assert( formation != 0 );

      double time = database::getTime( timeIoRecord );
      const Snapshot * snapshot = findSnapshot( time, MINOR | MAJOR );
      assert( snapshot != 0 );

      addPropertyValue( timeIoRecord, propertyValueName,
         property, snapshot, 0, formation, 0, THREEDTIMEIOTBL );
   }

   return true;
}


bool ProjectHandle::loadVolumePropertyValuesViaSnapshotIoTbl( void )
{
   database::Table* snapshotIoTbl = getTable( "SnapshotIoTbl" );
   database::Table::iterator tblIter;
   for ( tblIter = snapshotIoTbl->begin(); tblIter != snapshotIoTbl->end(); ++tblIter )
   {
      Record * snapshotIoRecord = *tblIter;

      double time = database::getTime( snapshotIoRecord );
      const Snapshot * snapshot = (const Snapshot *)findSnapshot( time, MINOR | MAJOR );
      if ( !snapshot ) continue;

      setCurrentSnapshot( snapshot );

      const string & fileName = database::getSnapshotFileName( snapshotIoRecord );
      if ( fileName.length() == 0 ) continue;

      ibs::FilePath filePathName( getProjectPath() );
      filePathName << getOutputDir() << fileName;

#if 0
      cerr << "Opening snapshot file " << filePathName.path() << endl;
#endif

      struct stat statbuf;
      if ( stat( filePathName.cpath(), &statbuf ) < 0 )
      {
         if ( getRank() == 0 )
         {
            cerr << "ERROR in ProjectHandle::loadVolumePropertyValues ():: Could not open " << filePathName.path() << ": ";
            perror( "" );
         }
         continue;
      }

      hid_t fileId = H5Fopen( filePathName.cpath(), H5F_ACC_RDONLY, H5P_DEFAULT );
      if ( fileId < 0 )
      {
         if ( getRank() == 0 )
         {
            cerr << "ERROR in ProjectHandle::loadVolumePropertyValues (): Could not open " << filePathName.path() << endl;
         }
         continue;
      }

      H5Giterate( fileId, "/", NULL, (H5G_iterate_t)ListVolumePropertyValues, ( void * ) this );

      H5Fclose( fileId );
   }

   return true;
}

/// load an output GridMap
GridMap * ProjectHandle::loadOutputMap( const Parent * parent, unsigned int childIndex,
   const string & fileName, const string & propertyId )
{
   ibs::FilePath filePathName( getFullOutputDir() );
   filePathName << fileName;

   string dataSetName = propertyId;
   return loadGridMap( parent, childIndex, filePathName.path(), dataSetName );
}

//static float GetUndefinedValue (hid_t fileId);


const char* NULL_VALUE_NAME = "/null value";

#ifdef UNUSED
const char* DELTA_I_DATASET_NAME  = "/delta in I dimension";
const char* DELTA_J_DATASET_NAME  = "/delta in J dimension";
const char* NR_I_DATASET_NAME     = "/number in I dimension";
const char* NR_J_DATASET_NAME     = "/number in J dimension";
const char* ORIGIN_I_DATASET_NAME = "/origin in I dimension";
const char* ORIGIN_J_DATASET_NAME = "/origin in J dimension";
#endif


/// Retrieve the undefined value from a HDF5 file
float ProjectHandle::GetUndefinedValue( hid_t fileId )
{
   float undefinedValue = static_cast<float>( DefaultUndefinedMapValue );

   hid_t dataSetId = -1;
   hid_t dataTypeId = -1;
   hid_t dataSpaceId = -1;
   if ( ( dataSetId = H5Dopen( fileId, NULL_VALUE_NAME, H5P_DEFAULT ) ) >= 0 )
   {
      dataTypeId = H5Tcopy( H5T_NATIVE_FLOAT );
      H5T_class_t storageTypeId = H5Tget_class( dataTypeId );

      assert( storageTypeId == H5T_FLOAT );

      if ( ( dataSpaceId = H5Dget_space( dataSetId ) ) >= 0 )
      {
         int rank = H5Sget_simple_extent_ndims( dataSpaceId );

         assert( rank == 1 );

         hsize_t hdimension;

         rank = H5Sget_simple_extent_dims( dataSpaceId, &hdimension, 0 );
         assert( rank == 1 );
         assert( hdimension == 1 );

         herr_t status = H5Dread( dataSetId, dataTypeId, H5S_ALL, H5S_ALL, H5P_DEFAULT, &undefinedValue );

         H5Sclose( dataSpaceId );
      }
      H5Tclose( dataTypeId );
      H5Dclose( dataSetId );
   }

   return undefinedValue;
}


Interface::SnapshotList * ProjectHandle::getSnapshots( int type ) const
{
   Interface::SnapshotList * snapshotList = new Interface::SnapshotList;

   MutableSnapshotList::const_iterator snapshotIter;

   for ( snapshotIter = m_snapshots.begin(); snapshotIter != m_snapshots.end(); ++snapshotIter )
   {
      Snapshot * snapshot = *snapshotIter;
      if ( snapshot->getType() & type )
      {
         snapshotList->push_back( snapshot );
      }
   }
   return snapshotList;
}

Interface::FormationList * ProjectHandle::getFormations( const Interface::Snapshot * snapshot,
   const bool                  includeBasement ) const
{
   Interface::FormationList * formationList = new Interface::FormationList;

   MutableFormationList::const_iterator formationIter;

   for ( formationIter = m_formations.begin(); formationIter != m_formations.end(); ++formationIter )
   {
      Formation * formation = *formationIter;

      // Add the formation if any of the following are true:
      //
      // 1. The snapshot is null and the formation is a sediment formation;
      //
      // 2. The include-basement flag is true and the formation is a basement 
      //    formation, it is assumed that the basement formations exist always;
      //
      // 3. The snapshot is not null and the snapshot-time is less than the bottom surface
      //    snapshot-time and the formation is a sediment formation;
      //
      if ( ( snapshot == 0 and formation->kind() == Interface::SEDIMENT_FORMATION ) or
         ( includeBasement and formation->kind() == Interface::BASEMENT_FORMATION ) or
         ( snapshot != 0 and formation->kind() == Interface::SEDIMENT_FORMATION and
         snapshot->getTime() < formation->getBottomSurface()->getSnapshot()->getTime() ) )
      {
         formationList->push_back( formation );
      }

   }

   return formationList;
}

Interface::LithoTypeList * ProjectHandle::getLithoTypes( void ) const
{
   Interface::LithoTypeList * lithoTypeList = new Interface::LithoTypeList;

   MutableLithoTypeList::const_iterator lithoTypeIter;

   for ( lithoTypeIter = m_lithoTypes.begin(); lithoTypeIter != m_lithoTypes.end(); ++lithoTypeIter )
   {
      LithoType * lithoType = *lithoTypeIter;
      lithoTypeList->push_back( lithoType );
   }
   return lithoTypeList;
}

Interface::SurfaceList * ProjectHandle::getSurfaces( const Interface::Snapshot * snapshot,
   const bool                  includeBasement ) const
{
   Interface::SurfaceList * surfaceList = new Interface::SurfaceList;

   MutableSurfaceList::const_iterator surfaceIter;

   for ( surfaceIter = m_surfaces.begin(); surfaceIter != m_surfaces.end(); ++surfaceIter )
   {
      Surface * surface = *surfaceIter;

      // Add the surface if any of the following are true:
      //
      // 1. The snapshot is null and the surface is a sediment-surface;
      //
      // 2. The include-basement flag is true and the surface is a basement
      //    surface, it is assumed that the basement surfaces exist always;
      //
      // 3. The snapshot is not null and the snapshot-time is less than the surface snapshot-time
      //    and the surface is a sediment-surface;
      //
      // * A basement-surface is one that lies entirely in the basement. The bottom-surface of
      // the bottom-most formation is not a basement-surface even though the formation below
      // is the Crust-formation (which is a part of the basement).
      //
      if ( ( snapshot == 0 and surface->kind() == Interface::SEDIMENT_SURFACE ) or
         ( includeBasement and surface->kind() == Interface::BASEMENT_SURFACE ) or
         ( snapshot != 0 and surface->kind() == Interface::SEDIMENT_SURFACE and
         snapshot->getTime() <= surface->getSnapshot()->getTime() ) )
      {
         surfaceList->push_back( surface );
      }

   }

   return surfaceList;
}

Interface::ReservoirList * ProjectHandle::getReservoirs( const Interface::Formation * formation ) const
{
   Interface::ReservoirList * reservoirList = new Interface::ReservoirList;

   MutableReservoirList::const_iterator reservoirIter;

   for ( reservoirIter = m_reservoirs.begin(); reservoirIter != m_reservoirs.end(); ++reservoirIter )
   {
      Reservoir * reservoir = *reservoirIter;
      if ( formation && reservoir->getFormation() != formation ) continue;
      reservoirList->push_back( reservoir );
   }
   return reservoirList;
}


Reservoir* ProjectHandle::addDetectedReservoirs (database::Record * record, const Formation * formation) 
{
	DataAccess::Interface::Reservoir * detectedReservoir = getFactory ()->produceReservoir (this, record);
   // connect the detected Reservoir
   detectedReservoir->setFormation(formation);
	// add the detected reservoir to the list of reservoirs
	m_reservoirs.push_back (detectedReservoir);
	// return the reservoir to formation
	return detectedReservoir;
}


Interface::MobileLayerList * ProjectHandle::getMobileLayers( const Interface::Formation * formation ) const
{
   Interface::MobileLayerList * mobileLayerList = new Interface::MobileLayerList;

   MutableMobileLayerList::const_iterator mobileLayerIter;

   for ( mobileLayerIter = m_mobileLayers.begin(); mobileLayerIter != m_mobileLayers.end(); ++mobileLayerIter )
   {
      MobileLayer * mobileLayer = *mobileLayerIter;
      if ( formation && mobileLayer->getFormation() != formation ) continue;
      mobileLayerList->push_back( mobileLayer );
   }
   return mobileLayerList;
}

// Interface::PaleoFormationPropertyList * ProjectHandle::getPaleoThicknesses ( const Interface::Formation * formation ) const {
// {
//    Interface::PaleoFormationPropertyList * paleoThicknessList = new Interface::PaleoFormationPropertyList;

//    MutablePaleoFormationPropertyList::const_iterator paleoThicknessIter;

//    for ( paleoThicknessIter = m_crustPaleoThicknesses.begin (); paleoThicknessIter != m_crustPaleoThicknesses.end (); ++paleoThicknessIter )
//    {
//       PaleoFormationProperty * paleoThickness = * paleoThicknessIter;

//       if ( formation == 0 or formation == paleoProperty->getFormation ()) {
//          paleoThicknessList->push_back ( paleoThickness );
//       }

//    }

//    return paleoThicknessList;
// }


Interface::FluidTypeList * ProjectHandle::getFluids() const {

   Interface::FluidTypeList * fluidList = new Interface::FluidTypeList;

   MutableFluidTypeList::const_iterator fluidIter;

   for ( fluidIter = m_fluidTypes.begin(); fluidIter != m_fluidTypes.end(); ++fluidIter )
   {
      FluidType * fluid = *fluidIter;
      fluidList->push_back( fluid );
   }

   return fluidList;
}

Interface::PaleoFormationPropertyList * ProjectHandle::getCrustPaleoThicknessHistory() const {

   Interface::PaleoFormationPropertyList * crustThinningHistoryList = new Interface::PaleoFormationPropertyList;

   MutablePaleoFormationPropertyList::const_iterator crustThinningHistoryIter;

   for ( crustThinningHistoryIter = m_crustPaleoThicknesses.begin(); crustThinningHistoryIter != m_crustPaleoThicknesses.end(); ++crustThinningHistoryIter )
   {
      PaleoFormationProperty * crustThinningHistoryInstance = *crustThinningHistoryIter;
      crustThinningHistoryList->push_back( crustThinningHistoryInstance );
   }

   return crustThinningHistoryList;
}


void ProjectHandle::computeMantlePaleoThicknessHistory() const {

   bool isALC = ( m_bottomBoundaryConditions == Interface::ADVANCED_LITHOSPHERE_CALCULATOR );
   const GridMap* presentDayCrustThickness = ( isALC ? dynamic_cast<const GridMap*>( m_crustFormation->getInitialThicknessMap() ) :
      dynamic_cast<const GridMap*>( m_crustFormation->getInputThicknessMap() ) );
   const GridMap* presentDayMantleThickness = ( isALC ? dynamic_cast<const GridMap*>( m_mantleFormation->getInitialThicknessMap() ) :
      dynamic_cast<const GridMap*>( m_mantleFormation->getInputThicknessMap() ) );

   AdditionFunctor add;
   SubtractionFunctor subtract;

   GridMap* presentDayBasementThickness = getFactory()->produceGridMap( 0, 0, presentDayCrustThickness, presentDayMantleThickness, add );
   MutablePaleoFormationPropertyList::const_iterator crustThicknessIter;
   MutablePaleoFormationPropertyList::const_iterator mantleThicknessIter;

   if ( isALC ) {
      // the m_mantlePaleoThicknesses size may not be the same as m_crustPaleoThicknesses
      for ( mantleThicknessIter = m_mantlePaleoThicknesses.begin(); mantleThicknessIter != m_mantlePaleoThicknesses.end(); ++mantleThicknessIter ) {

         PaleoFormationProperty* crustThicknessMap = 0;
         PaleoFormationProperty* mantleThicknessMap = *mantleThicknessIter;

         const double age = mantleThicknessMap->getSnapshot()->getTime();
         for ( crustThicknessIter = m_crustPaleoThicknesses.begin(); crustThicknessIter != m_crustPaleoThicknesses.end(); ++crustThicknessIter ) {
            if ( ( *crustThicknessIter )->getSnapshot()->getTime() == age ) {
               crustThicknessMap = *crustThicknessIter;
               break;
            }
         }
         assert( crustThicknessMap != 0 );

         mantleThicknessMap->computeMap( Interface::MantleThicknessHistoryInstanceThicknessMap,
            presentDayBasementThickness,
            dynamic_cast<const GridMap *>( crustThicknessMap->getMap( Interface::CrustThinningHistoryInstanceThicknessMap ) ),
            subtract );
      }
   }
   else {
      assert( m_crustPaleoThicknesses.size() == m_mantlePaleoThicknesses.size() );

      for ( crustThicknessIter = m_crustPaleoThicknesses.begin(), mantleThicknessIter = m_mantlePaleoThicknesses.begin();
         crustThicknessIter != m_crustPaleoThicknesses.end();
         ++crustThicknessIter, ++mantleThicknessIter ) {

         PaleoFormationProperty* crustThicknessMap = *crustThicknessIter;
         PaleoFormationProperty* mantleThicknessMap = *mantleThicknessIter;

         mantleThicknessMap->computeMap( Interface::MantleThicknessHistoryInstanceThicknessMap,
            presentDayBasementThickness,
            dynamic_cast<const GridMap *>( crustThicknessMap->getMap( Interface::CrustThinningHistoryInstanceThicknessMap ) ),
            subtract );
      }

   }

   delete presentDayBasementThickness;
}


Interface::PaleoFormationPropertyList * ProjectHandle::getMantlePaleoThicknessHistory() const {

   if ( m_mantlePaleoThicknesses.size() == 0 ){
      return 0;
   }

   Interface::PaleoFormationPropertyList * mantleThinningHistoryList = new Interface::PaleoFormationPropertyList;

   MutablePaleoFormationPropertyList::const_iterator mantleThinningHistoryIter;

   PaleoFormationProperty* firstItem = *m_mantlePaleoThicknesses.begin();

   if ( firstItem->getChild( Interface::MantleThicknessHistoryInstanceThicknessMap ) == 0 ) {
      computeMantlePaleoThicknessHistory();
   }

   for ( mantleThinningHistoryIter = m_mantlePaleoThicknesses.begin(); mantleThinningHistoryIter != m_mantlePaleoThicknesses.end(); ++mantleThinningHistoryIter )
   {
      PaleoFormationProperty * mantleThinningHistoryInstance = *mantleThinningHistoryIter;
      mantleThinningHistoryList->push_back( mantleThinningHistoryInstance );
   }

   return mantleThinningHistoryList;
}

Interface::PaleoSurfacePropertyList * ProjectHandle::getHeatFlowHistory() const {

   Interface::PaleoSurfacePropertyList * heatFlowHistoryList = new Interface::PaleoSurfacePropertyList;

   MutablePaleoSurfacePropertyList::const_iterator heatFlowHistoryIter;

   for ( heatFlowHistoryIter = m_heatFlowHistory.begin(); heatFlowHistoryIter != m_heatFlowHistory.end(); ++heatFlowHistoryIter )
   {
      PaleoSurfaceProperty * heatFlowHistory = *heatFlowHistoryIter;
      heatFlowHistoryList->push_back( heatFlowHistory );
   }

   return heatFlowHistoryList;
}


Interface::PaleoPropertyList * ProjectHandle::getSurfaceDepthHistory() const
{
   Interface::PaleoPropertyList * surfaceDepthHistoryList = new Interface::PaleoPropertyList;

   MutablePaleoPropertyList::const_iterator surfaceDepthHistoryIter;

   for ( surfaceDepthHistoryIter = m_surfaceDepthHistory.begin(); surfaceDepthHistoryIter != m_surfaceDepthHistory.end(); ++surfaceDepthHistoryIter )
   {
      PaleoProperty * surfaceDepthHistoryInstance = *surfaceDepthHistoryIter;
      surfaceDepthHistoryList->push_back( surfaceDepthHistoryInstance );
   }

   return surfaceDepthHistoryList;
}




Interface::PaleoPropertyList * ProjectHandle::getSurfaceTemperatureHistory() const
{
   Interface::PaleoPropertyList * surfaceTemperatureHistoryList = new Interface::PaleoPropertyList;

   MutablePaleoPropertyList::const_iterator surfaceTemperatureHistoryIter;

   for ( surfaceTemperatureHistoryIter = m_surfaceTemperatureHistory.begin(); surfaceTemperatureHistoryIter != m_surfaceTemperatureHistory.end(); ++surfaceTemperatureHistoryIter )
   {
      PaleoProperty * surfaceTemperatureHistoryInstance = *surfaceTemperatureHistoryIter;
      surfaceTemperatureHistoryList->push_back( surfaceTemperatureHistoryInstance );
   }

   return surfaceTemperatureHistoryList;
}

Interface::TouchstoneMapList * ProjectHandle::getTouchstoneMaps( void ) const
{
   Interface::TouchstoneMapList * touchstoneMapList = new Interface::TouchstoneMapList;

   MutableTouchstoneMapList::const_iterator touchstoneMapIter;

   for ( touchstoneMapIter = m_touchstoneMaps.begin(); touchstoneMapIter != m_touchstoneMaps.end(); ++touchstoneMapIter )
   {
      TouchstoneMap * touchstoneMap = *touchstoneMapIter;
      touchstoneMapList->push_back( touchstoneMap );
   }
   return touchstoneMapList;
}

Interface::AllochthonousLithologyDistributionList * ProjectHandle::getAllochthonousLithologyDistributions( const Interface::AllochthonousLithology * allochthonousLithology ) const
{

   Interface::AllochthonousLithologyDistributionList* allochthonousLithologyDistributionList = new Interface::AllochthonousLithologyDistributionList;
   MutableAllochthonousLithologyDistributionList::const_iterator distributionIter;

   if ( allochthonousLithology == 0 )
   {
      return 0;
   }

   for ( distributionIter = m_allochthonousLithologyDistributions.begin(); distributionIter != m_allochthonousLithologyDistributions.end(); ++distributionIter )
   {

      const Interface::AllochthonousLithologyDistribution * lithologyDistribution = *distributionIter;

      if ( allochthonousLithology->getFormationName() == lithologyDistribution->getFormationName() ) {
         allochthonousLithologyDistributionList->push_back( lithologyDistribution );
      }

   }

   return allochthonousLithologyDistributionList;
}

Interface::AllochthonousLithologyInterpolationList * ProjectHandle::getAllochthonousLithologyInterpolations( const Interface::AllochthonousLithology * allochthonousLithology ) const
{

   Interface::AllochthonousLithologyInterpolationList* allochthonousLithologyInterpolationList = new Interface::AllochthonousLithologyInterpolationList;
   MutableAllochthonousLithologyInterpolationList::const_iterator interpolationIter;

   if ( allochthonousLithology == 0 )
   {
      return 0;
   }

   for ( interpolationIter = m_allochthonousLithologyInterpolations.begin(); interpolationIter != m_allochthonousLithologyInterpolations.end(); ++interpolationIter )
   {

      const Interface::AllochthonousLithologyInterpolation * lithologyInterpolation = *interpolationIter;

      if ( allochthonousLithology->getFormationName() == lithologyInterpolation->getFormationName() ) {
         allochthonousLithologyInterpolationList->push_back( lithologyInterpolation );
      }

   }

   return allochthonousLithologyInterpolationList;
}

Interface::OutputPropertyList * ProjectHandle::getTimeOutputProperties() const {

   Interface::OutputPropertyList * outputPropertyList = new Interface::OutputPropertyList;

   MutableOutputPropertyList::const_iterator outputPropertyIter;

   for ( outputPropertyIter = m_timeOutputProperties.begin(); outputPropertyIter != m_timeOutputProperties.end(); ++outputPropertyIter )
   {
      OutputProperty * outputProperty = *outputPropertyIter;
      outputPropertyList->push_back( outputProperty );
   }

   return outputPropertyList;
}

Interface::OutputPropertyList * ProjectHandle::getDepthOutputProperties() const {

   Interface::OutputPropertyList * outputPropertyList = new Interface::OutputPropertyList;

   MutableOutputPropertyList::const_iterator outputPropertyIter;

   for ( outputPropertyIter = m_depthOutputProperties.begin(); outputPropertyIter != m_depthOutputProperties.end(); ++outputPropertyIter )
   {
      OutputProperty * outputProperty = *outputPropertyIter;
      outputPropertyList->push_back( outputProperty );
   }

   return outputPropertyList;
}

Interface::LithologyHeatCapacitySampleList * ProjectHandle::getLithologyHeatCapacitySampleList( const Interface::LithoType* litho ) const {

   Interface::LithologyHeatCapacitySampleList * heatCapacityList = new Interface::LithologyHeatCapacitySampleList;

   MutableLithologyHeatCapacitySampleList::const_iterator sampleIter;

   for ( sampleIter = m_lithologyHeatCapacitySamples.begin(); sampleIter != m_lithologyHeatCapacitySamples.end(); ++sampleIter ) {

      LithologyHeatCapacitySample * heatCapacitySample = *sampleIter;

      if ( litho == 0 or heatCapacitySample->getLithologyName() == litho->getName() ) {
         heatCapacityList->push_back( heatCapacitySample );
      }

   }

   return heatCapacityList;
}

Interface::LithologyThermalConductivitySampleList * ProjectHandle::getLithologyThermalConductivitySampleList( const Interface::LithoType* litho ) const {

   Interface::LithologyThermalConductivitySampleList * thermalConductivityList = new Interface::LithologyThermalConductivitySampleList;

   MutableLithologyThermalConductivitySampleList::const_iterator sampleIter;

   for ( sampleIter = m_lithologyThermalConductivitySamples.begin(); sampleIter != m_lithologyThermalConductivitySamples.end(); ++sampleIter ) {

      LithologyThermalConductivitySample * thermalConductivitySample = *sampleIter;

      if ( litho == 0 or thermalConductivitySample->getLithologyName() == litho->getName() ) {
         thermalConductivityList->push_back( thermalConductivitySample );
      }

   }

   return thermalConductivityList;
}


Interface::FluidHeatCapacitySampleList * ProjectHandle::getFluidHeatCapacitySampleList( const Interface::FluidType* fluid ) const {

   Interface::FluidHeatCapacitySampleList * heatCapacityList = new Interface::FluidHeatCapacitySampleList;

   MutableFluidHeatCapacitySampleList::const_iterator sampleIter;

   for ( sampleIter = m_fluidHeatCapacitySamples.begin(); sampleIter != m_fluidHeatCapacitySamples.end(); ++sampleIter ) {

      FluidHeatCapacitySample * heatCapacitySample = *sampleIter;

      if ( fluid == 0 or heatCapacitySample->getFluid() == fluid ) {
         heatCapacityList->push_back( heatCapacitySample );
      }

   }

   return heatCapacityList;
}

Interface::FluidThermalConductivitySampleList * ProjectHandle::getFluidThermalConductivitySampleList( const Interface::FluidType* fluid ) const {

   Interface::FluidThermalConductivitySampleList * thermalConductivityList = new Interface::FluidThermalConductivitySampleList;

   MutableFluidThermalConductivitySampleList::const_iterator sampleIter;

   for ( sampleIter = m_fluidThermalConductivitySamples.begin(); sampleIter != m_fluidThermalConductivitySamples.end(); ++sampleIter ) {

      FluidThermalConductivitySample * thermalConductivitySample = *sampleIter;

      if ( fluid == 0 or thermalConductivitySample->getFluid() == fluid ) {
         thermalConductivityList->push_back( thermalConductivitySample );
      }

   }

   return thermalConductivityList;
}


Interface::FluidDensitySampleList * ProjectHandle::getFluidDensitySampleList( const Interface::FluidType* fluid ) const {

   Interface::FluidDensitySampleList * densityList = new Interface::FluidDensitySampleList;

   MutableFluidDensitySampleList::const_iterator sampleIter;

   for ( sampleIter = m_fluidDensitySamples.begin(); sampleIter != m_fluidDensitySamples.end(); ++sampleIter ) {

      FluidDensitySample * densitySample = *sampleIter;

      if ( fluid == 0 or densitySample->getFluid() == fluid ) {
         densityList->push_back( densitySample );
      }

   }

   return densityList;
}

Interface::RelatedProjectList * ProjectHandle::getRelatedProjectList() const {

   Interface::RelatedProjectList * relatedProjects = new Interface::RelatedProjectList;

   MutableRelatedProjectList::const_iterator relatedProjectIter;

   for ( relatedProjectIter = m_relatedProjects.begin(); relatedProjectIter != m_relatedProjects.end(); ++relatedProjectIter ) {
      RelatedProject * project = *relatedProjectIter;
      relatedProjects->push_back( project );
   }

   return relatedProjects;
}

Interface::FaultCollectionList * ProjectHandle::getFaultCollections( const Interface::Formation * formation ) const
{
   Interface::FaultCollectionList * faultCollectionList = new Interface::FaultCollectionList;

   MutableFaultCollectionList::const_iterator faultCollectionIter;

   for ( faultCollectionIter = m_faultCollections.begin(); faultCollectionIter != m_faultCollections.end(); ++faultCollectionIter )
   {
      FaultCollection * faultCollection = *faultCollectionIter;
      if ( formation && !faultCollection->appliesToFormation( formation ) ) continue;
#if 0
      cerr << "found fault collection " << faultCollection->getName () << " for formation " << formation->getName () << endl;
#endif
      faultCollectionList->push_back( faultCollection );
   }
   return faultCollectionList;
}

Interface::ConstrainedOverpressureIntervalList* ProjectHandle::getConstrainedOverpressureIntervalList( const Formation* formation ) const {

   Interface::ConstrainedOverpressureIntervalList * intervalList = new Interface::ConstrainedOverpressureIntervalList;

   MutableConstrainedOverpressureIntervalList::const_iterator intervalIter;

   for ( intervalIter = m_constrainedOverpressureIntervals.begin(); intervalIter != m_constrainedOverpressureIntervals.end(); ++intervalIter )
   {
      ConstrainedOverpressureInterval * interval = *intervalIter;

      if ( formation == 0 or formation == interval->getFormation() ) {
         intervalList->push_back( interval );
      }

   }

   return intervalList;
}


Interface::TrapList * ProjectHandle::getTraps( const Interface::Reservoir * reservoir,
   const Interface::Snapshot * snapshot, unsigned int id ) const
{
   Interface::TrapList * trapList = new Interface::TrapList;

   MutableTrapList::const_iterator trapIter;

   for ( trapIter = m_traps.begin(); trapIter != m_traps.end(); ++trapIter )
   {
      Trap * trap = *trapIter;

      if ( trap->matchesConditions( (Reservoir *)reservoir, (Snapshot *)snapshot, id ) )
         trapList->push_back( trap );
   }
   return trapList;
}

const Interface::Trap * ProjectHandle::findTrap( const Interface::Reservoir * reservoir,
   const Interface::Snapshot * snapshot, unsigned int id ) const
{
   MutableTrapList::const_iterator trapIter;

   for ( trapIter = m_traps.begin(); trapIter != m_traps.end(); ++trapIter )
   {
      Trap * trap = *trapIter;

      if ( trap->matchesConditions( (Reservoir *)reservoir, (Snapshot *)snapshot, id ) )
         return trap;
   }
   return 0;
}

Interface::MigrationList * ProjectHandle::getMigrations( const string & process, const Interface::Formation * sourceFormation,
   const Interface::Snapshot * sourceSnapshot, const Interface::Reservoir * sourceReservoir, const Interface::Trapper * sourceTrapper,
   const Interface::Snapshot * destinationSnapshot, const Interface::Reservoir * destinationReservoir, const Interface::Trapper * destinationTrapper
   ) const
{
   Interface::MigrationList * migrationList = new Interface::MigrationList;

   MutableMigrationList::const_iterator migrationIter;

   for ( migrationIter = m_migrations.begin(); migrationIter != m_migrations.end(); ++migrationIter )
   {
      Migration * migration = *migrationIter;

      if ( migration->matchesConditions( process, dynamic_cast<const Formation *> ( sourceFormation ),
         (const Snapshot *)sourceSnapshot, (const Reservoir *)sourceReservoir, (const Trapper *)sourceTrapper,
         (const Snapshot *)destinationSnapshot, (const Reservoir *)destinationReservoir, (const Trapper *)destinationTrapper ) )
      {
         migrationList->push_back( migration );
      }
   }

   return migrationList;
}

void ProjectHandle::numberInputValues( void )
{
   MutableInputValueList::iterator inputValueIter;

   unsigned int maxIndexUsed = 0;
   unsigned int index;

   for ( inputValueIter = m_inputValues.begin(); inputValueIter != m_inputValues.end(); ++inputValueIter )
   {
      InputValue * inputValue = *inputValueIter;
      if ( ( index = inputValue->computeIndex() ) > maxIndexUsed ) maxIndexUsed = index;
   }

   for ( inputValueIter = m_inputValues.begin(); inputValueIter != m_inputValues.end(); ++inputValueIter )
   {
      InputValue * inputValue = *inputValueIter;
      if ( ( index = inputValue->applyIndex( maxIndexUsed + 1 ) ) > maxIndexUsed ) maxIndexUsed = index;
   }
}


void ProjectHandle::loadPropertyGridMaps( Interface::PropertyValueList * propertyValues ) const
{
   if ( !propertyValues ) return;
   Interface::PropertyValueList::iterator propertyValueIter;

   for ( propertyValueIter = propertyValues->begin(); propertyValueIter != propertyValues->end(); ++propertyValueIter )
   {
      PropertyValue * propertyValue = (PropertyValue *)( *propertyValueIter );
      propertyValue->getGridMap();
   }
}

void ProjectHandle::loadInputGridMaps( void ) const
{
   MutableInputValueList::const_iterator inputValueIter;

   for ( inputValueIter = m_inputValues.begin(); inputValueIter != m_inputValues.end(); ++inputValueIter )
   {
      InputValue * inputValue = *inputValueIter;
      inputValue->getGridMap();
   }
}

Interface::InputValueList * ProjectHandle::getInputValues( void ) const
{
   Interface::InputValueList * inputValueList = new Interface::InputValueList;

   MutableInputValueList::const_iterator inputValueIter;

   for ( inputValueIter = m_inputValues.begin(); inputValueIter != m_inputValues.end(); ++inputValueIter )
   {
      InputValue * inputValue = *inputValueIter;
      inputValueList->push_back( inputValue );
   }
   return inputValueList;
}

Interface::PropertyList * ProjectHandle::getProperties( bool all, int selectionFlags, const Interface::Snapshot * snapshot,
   const Interface::Reservoir * reservoir, const Interface::Formation * formation,
   const Interface::Surface * surface, int propertyTypes ) const
{
   Interface::PropertyList * propertyList = new Interface::PropertyList;

   MutablePropertyList::const_iterator propertyIter;

   for ( propertyIter = m_properties.begin(); propertyIter != m_properties.end(); ++propertyIter )
   {
      Property * property = *propertyIter;
      if ( all || property->hasPropertyValues( selectionFlags, (Snapshot *)snapshot, (Reservoir *)reservoir,
         dynamic_cast<const Formation *>( formation ), dynamic_cast<const Surface *>( surface ), propertyTypes ) )
      {
         propertyList->push_back( property );
      }
   }
   return propertyList;
}

Interface::PropertyListPtr ProjectHandle::getProperties ( const DataModel::PropertyAttribute attr ) const {

   Interface::PropertyListPtr propertyList = Interface::PropertyListPtr ( new Interface::PropertyList );

   for ( size_t i = 0; i < m_properties.size (); ++i ) {

      if ( m_properties [ i ]->getPropertyAttribute () == attr ) {
         propertyList->push_back ( m_properties [ i ]);
      }

   }

   return propertyList;
}


bool ProjectHandle::hasPropertyValues( int selectionFlags, const Property * property, const Snapshot * snapshot,
   const Reservoir * reservoir, const Formation * formation, const Surface * surface, int propertyType ) const
{
   MutablePropertyValueList::const_iterator propertyValueIter;

   for ( propertyValueIter = m_propertyValues.begin();
      propertyValueIter != m_propertyValues.end(); ++propertyValueIter )
   {
      PropertyValue *propertyValue = *propertyValueIter;

      if ( propertyValue->matchesConditions( selectionFlags, property, snapshot,
         reservoir, formation, surface, propertyType ) )
      {
         return true;
      }
   }
   return false;
}

Interface::PropertyValueList * ProjectHandle::getPropertyValues( int selectionFlags,
   const Interface::Property * property, const Interface::Snapshot * snapshot,
   const Interface::Reservoir * reservoir, const Interface::Formation * formation, const Interface::Surface * surface,
   int propertyType ) const
{
   Interface::PropertyValueList * propertyValueList = new Interface::PropertyValueList;

   MutablePropertyValueList::const_iterator propertyValueIter;

   for ( propertyValueIter = m_propertyValues.begin();
      propertyValueIter != m_propertyValues.end();
      ++propertyValueIter )
   {
      PropertyValue * propertyValue = *propertyValueIter;

      if ( propertyValue->matchesConditions( selectionFlags, (Property *)property, (Snapshot *)snapshot, (Reservoir *)reservoir,
         dynamic_cast<const Formation *>( formation ), dynamic_cast<const Surface *>( surface ), propertyType ) )
      {
         //Alfred  propertyValueList->push_back (const_cast<Interface::PropertyValue*> (propertyValue));
         propertyValueList->push_back( propertyValue );
      }
   }

   /// The PropertyValueList needs to be sorted so that user applications know what to expect
   sort( propertyValueList->begin(), propertyValueList->end(), PropertyValue::SortByAgeAndDepoAge );
   return propertyValueList;
}

unsigned int ProjectHandle::deletePropertyValueGridMaps( int selectionFlags,
   const Property * property, const Snapshot * snapshot,
   const Reservoir * reservoir, const Formation * formation, const Surface * surface,
   int propertyType ) const
{
   unsigned int nrDeleted = 0;
   MutablePropertyValueList::const_iterator propertyValueIter;

   for ( propertyValueIter = m_propertyValues.begin();
      propertyValueIter != m_propertyValues.end();
      ++propertyValueIter )
   {
      PropertyValue * propertyValue = *propertyValueIter;

      if ( propertyValue->matchesConditions( selectionFlags, property, snapshot,
         reservoir, formation, surface, propertyType ) )
      {
         // skip over produced propertyvalue gridmaps for the time being
         if ( propertyValue->getStorage() == TIMEIOTBL && propertyValue->getRecord() == 0 ) continue;

         GridMap * localGridMap = propertyValue->hasGridMap();
         if ( localGridMap )
         {
            delete localGridMap;
            ++nrDeleted;
         }
      }
   }
   return nrDeleted;
}

void ProjectHandle::deletePropertyValues( int selectionFlags,
   const Property * property, const Snapshot * snapshot,
   const Reservoir * reservoir, const Formation * formation, const Surface * surface,
   int propertyType )
{
   MutablePropertyValueList::iterator propertyValueIter;

   propertyValueIter = m_propertyValues.begin();
   while ( propertyValueIter != m_propertyValues.end() )
   {
      PropertyValue * propertyValue = *propertyValueIter;

      if ( propertyValue->matchesConditions( selectionFlags, property, snapshot,
         reservoir, formation, surface, propertyType ) )
      {
         propertyValueIter = m_propertyValues.erase( propertyValueIter );

         if ( propertyValue->getRecord() and propertyValue->getStorage() != SNAPSHOTIOTBL )
         {
            propertyValue->getRecord()->getTable()->deleteRecord( propertyValue->getRecord() );
         }
         delete propertyValue;
      }
      else
      {
         ++propertyValueIter;
      }
   }
}

void splitFilePath( const string & filePath, string & directoryName, string & fileName )
{
   size_t slashPos = filePath.rfind( '/' );
   if ( slashPos != string::npos )
   {
      directoryName = filePath.substr( 0, slashPos );
      fileName = filePath.substr( slashPos + 1, string::npos );
   }
   else
   {
      directoryName = ".";
      fileName = filePath;
   }
}

void ProjectHandle::printPropertyValues( Interface::PropertyValueList * propertyValues ) const
{
   Interface::PropertyValueList::iterator propertyValueIter;

   string tmpString;
   for ( propertyValueIter = propertyValues->begin(); propertyValueIter != propertyValues->end(); ++propertyValueIter )
   {
      PropertyValue * propertyValue = (PropertyValue *)( *propertyValueIter );
      propertyValue->asString( tmpString );
      cerr << tmpString;
   }
}
/// Utilities to parse a HDF5 file
void ProjectHandle::setCurrentSnapshot( const Snapshot * snapshot )
{
   m_currentSnapshot = snapshot;
}

/// Utilities to parse a HDF5 file
const Snapshot * ProjectHandle::getCurrentSnapshot( void )
{
   return m_currentSnapshot;
}

/// Utilities to parse a HDF5 file
void ProjectHandle::setCurrentProperty( const Property * property )
{
   m_currentProperty = property;
}

/// Utilities to parse a HDF5 file
const Property * ProjectHandle::getCurrentProperty( void )
{
   return m_currentProperty;
}

/// Utilities to parse a HDF5 file
void ProjectHandle::setCurrentPropertyValueName( const string & name )
{
   m_currentPropertyValueName = name;
}

/// Utilities to parse a HDF5 file
const string & ProjectHandle::getCurrentPropertyValueName( void )
{
   return m_currentPropertyValueName;
}

const Interface::Grid * ProjectHandle::getInputGrid( void ) const
{
   if ( m_inputGrid == 0 )
   {
      database::Table *projectIoTbl = getTable( "ProjectIoTbl" );

      assert( projectIoTbl );

      assert( projectIoTbl->size() != 0 );
      Record *projectIoRecord = projectIoTbl->getRecord( 0 );

      assert( projectIoRecord );

      int numI, numJ;
      double deltaI, deltaJ;
      double minI, minJ;
      double maxI, maxJ;

      numI = database::getNumberX( projectIoRecord );
      numJ = database::getNumberY( projectIoRecord );

      deltaI = database::getDeltaX( projectIoRecord );
      deltaJ = database::getDeltaY( projectIoRecord );

      minI = database::getXCoord( projectIoRecord );
      minJ = database::getYCoord( projectIoRecord );

      maxI = minI + deltaI * ( numI - 1 );
      maxJ = minJ + deltaJ * ( numJ - 1 );

      m_inputGrid = getFactory()->produceGrid( getHighResolutionOutputGrid(), minI, minJ, maxI, maxJ, numI, numJ );
   }

   return m_inputGrid;
}

const Interface::Grid * ProjectHandle::getLowResolutionOutputGrid( void ) const
{
   if ( m_lowResOutputGrid == 0 )
   {
      database::Table *projectIoTbl = getTable( "ProjectIoTbl" );

      assert( projectIoTbl );

      assert( projectIoTbl->size() != 0 );
      Record *projectIoRecord = projectIoTbl->getRecord( 0 );

      assert( projectIoRecord );

      int numI, numJ;
      double deltaI, deltaJ;
      double minI, minJ;
      double maxI, maxJ;
      int offsetI, offsetJ;
      int scaleI, scaleJ;

      numI = database::getWindowXMax( projectIoRecord ) - database::getWindowXMin( projectIoRecord ) + 1;
      numJ = database::getWindowYMax( projectIoRecord ) - database::getWindowYMin( projectIoRecord ) + 1;

      deltaI = database::getDeltaX( projectIoRecord );
      deltaJ = database::getDeltaY( projectIoRecord );

      minI = database::getXCoord( projectIoRecord ) + database::getWindowXMin( projectIoRecord ) * database::getDeltaX( projectIoRecord );
      minJ = database::getYCoord( projectIoRecord ) + database::getWindowYMin( projectIoRecord ) * database::getDeltaY( projectIoRecord );

      maxI = minI + deltaI * ( numI - 1 );
      maxJ = minJ + deltaJ * ( numJ - 1 );

      offsetI = database::getOffsetX( projectIoRecord );
      offsetJ = database::getOffsetY( projectIoRecord );

      scaleI = database::getScaleX( projectIoRecord );
      scaleJ = database::getScaleY( projectIoRecord );

      numI = ( numI - offsetI - 1 ) / scaleI + 1;
      numJ = ( numJ - offsetJ - 1 ) / scaleJ + 1;

      minI = minI + offsetI * deltaI;
      minJ = minJ + offsetJ * deltaJ;

      deltaI = scaleI * deltaI;
      deltaJ = scaleJ * deltaJ;

      maxI = minI + deltaI * ( numI - 1 );
      maxJ = minJ + deltaJ * ( numJ - 1 );

      checkForValidPartitioning( "Unknown", numI, numJ ); // NOOP in case of serial data access

      m_lowResOutputGrid = getFactory()->produceGrid( getHighResolutionOutputGrid(), minI, minJ, maxI, maxJ, numI, numJ );
   }

   return m_lowResOutputGrid;
}

const Interface::Grid * ProjectHandle::getHighResolutionOutputGrid( void ) const
{
   if ( m_highResOutputGrid == 0 )
   {
      database::Table *projectIoTbl = getTable( "ProjectIoTbl" );

      assert( projectIoTbl );

      assert( projectIoTbl->size() != 0 );
      Record *projectIoRecord = projectIoTbl->getRecord( 0 );

      assert( projectIoRecord );

      int numI, numJ;
      int lowResNumI, lowResNumJ;
      double deltaI, deltaJ;
      double minI, minJ;
      double maxI, maxJ;

      numI = database::getWindowXMax( projectIoRecord ) - database::getWindowXMin( projectIoRecord ) + 1;
      numJ = database::getWindowYMax( projectIoRecord ) - database::getWindowYMin( projectIoRecord ) + 1;

      deltaI = database::getDeltaX( projectIoRecord );
      deltaJ = database::getDeltaY( projectIoRecord );

      minI = database::getXCoord( projectIoRecord ) + database::getWindowXMin( projectIoRecord ) * database::getDeltaX( projectIoRecord );
      minJ = database::getYCoord( projectIoRecord ) + database::getWindowYMin( projectIoRecord ) * database::getDeltaY( projectIoRecord );

      maxI = minI + deltaI * ( numI - 1 );
      maxJ = minJ + deltaJ * ( numJ - 1 );

      checkForValidPartitioning( "Unknown", numI, numJ ); // NOOP in case of serial data access

      int offsetI, offsetJ;
      int scaleI, scaleJ;

      offsetI = database::getOffsetX( projectIoRecord );
      offsetJ = database::getOffsetY( projectIoRecord );

      scaleI = database::getScaleX( projectIoRecord );
      scaleJ = database::getScaleY( projectIoRecord );


      lowResNumI = ( numI - offsetI - 1 ) / scaleI + 1;
      lowResNumJ = ( numJ - offsetJ - 1 ) / scaleJ + 1;

      m_highResOutputGrid = getFactory()->produceGrid( minI, minJ, maxI, maxJ, numI, numJ, lowResNumI, lowResNumJ );
   }
   return m_highResOutputGrid;
}

const Interface::Snapshot * ProjectHandle::findSnapshot( double time, int type ) const
{
   // first, try the highway
   const double tolerance = 1e-7;
   MutableSnapshotList::const_iterator snapshotIter;

   for ( snapshotIter = m_snapshots.begin(); snapshotIter != m_snapshots.end(); ++snapshotIter )
   {
      const Snapshot * snapshot = *snapshotIter;
      if ( ( snapshot->getType() & type ) &&
         snapshot->getTime() >= time - tolerance && snapshot->getTime() <= time + tolerance )
      {
         // Note that we return an Interface::Snapshot
         return snapshot;
      }
   }

   // The highway failed, take the scenic road.
   const Snapshot * nearestSnapshot = 0;
   double nearestDifference = 1e12;
   for ( snapshotIter = m_snapshots.begin(); snapshotIter != m_snapshots.end(); ++snapshotIter )
   {
      const Snapshot * snapshot = *snapshotIter;
      if ( ( snapshot->getType() & type ) )
      {
         double difference = std::abs( time - snapshot->getTime() );
         if ( difference < nearestDifference )
         {
            nearestDifference = difference;
            nearestSnapshot = snapshot;
         }
      }
   }


   return nearestSnapshot;
}

void ProjectHandle::sortSnapshots()
{
   std::sort( m_snapshots.begin(), m_snapshots.end(), SnapshotLessThan() );
}
 
void ProjectHandle::printSnapshotTable() const
{
  MutableSnapshotList::const_iterator snapshotIter;

   cout << "Snapshots: " << endl;
   for ( snapshotIter = m_snapshots.begin(); snapshotIter != m_snapshots.end(); ++ snapshotIter )
   {
      const Snapshot * snapshot = *snapshotIter;
      cout << snapshot->getTime() << ", " << ( snapshot->getType() == MINOR ? "minor " : ( snapshot->getType() == MAJOR ? "major " :  "unknown ")) << endl;
   }

}
const Interface::Snapshot * ProjectHandle::findNextSnapshot( double time, int type ) const
{
   // first, try the highway
   const double tolerance = 1e-6;
   MutableSnapshotList::const_reverse_iterator snapshotIter;

   for ( snapshotIter = m_snapshots.rbegin(); snapshotIter != m_snapshots.rend(); ++ snapshotIter )
   {
      const Snapshot * snapshot = *snapshotIter;
      if ( ( snapshot->getType() & type ) && snapshot->getTime() <= time )
      {
         // Note that we return an Interface::Snapshot
         return snapshot;
      }
   }

   return 0;
}
const Interface::Snapshot * ProjectHandle::findPreviousSnapshot( double time, int type ) const
{
   // first, try the highway
   const double tolerance = 1e-6;
   MutableSnapshotList::const_iterator snapshotIter;

   for ( snapshotIter = m_snapshots.begin(); snapshotIter != m_snapshots.end(); ++ snapshotIter )
   {
      const Snapshot * snapshot = *snapshotIter;
      if ( ( snapshot->getType() & type ) && snapshot->getTime() > time )
      {
         // Note that we return an Interface::Snapshot
         return snapshot;
      }
   }

   return 0;
}

const Interface::Reservoir * ProjectHandle::findReservoir( const string & name ) const
{
   MutableReservoirList::const_iterator reservoirIter;

   for ( reservoirIter = m_reservoirs.begin(); reservoirIter != m_reservoirs.end(); ++reservoirIter )
   {
      const Reservoir * reservoir = *reservoirIter;
      if ( reservoir->getName() == name || reservoir->getMangledName() == name )
      {
         // Note that we return an Interface::Reservoir
         return reservoir;
      }
   }
   return 0;
}

const Interface::Formation * ProjectHandle::findFormation( const string & name ) const
{
   MutableFormationList::const_iterator formationIter;

   if ( name == "" ) return 0;

   for ( formationIter = m_formations.begin(); formationIter != m_formations.end(); ++formationIter )
   {
      const Formation * formation = *formationIter;
      if ( formation->getName() == name || formation->getMangledName() == name )
      {
         // Note that we return an Interface::Formation
         return formation;
      }
   }
   return 0;
}


const Interface::FluidType * ProjectHandle::findFluid( const string & name ) const
{
   MutableFluidTypeList::const_iterator fluidIter;

   for ( fluidIter = m_fluidTypes.begin(); fluidIter != m_fluidTypes.end(); ++fluidIter )
   {
      const FluidType * fluid = *fluidIter;

      if ( fluid->getName() == name )
      {
         // Note that we return an Interface::FluidType
         return fluid;
      }
   }

   return 0;
}

const Interface::LithoType * ProjectHandle::findLithoType( const string & name ) const
{
   MutableLithoTypeList::const_iterator lithoTypeIter;

   if ( name == "" ) return 0;

   for ( lithoTypeIter = m_lithoTypes.begin(); lithoTypeIter != m_lithoTypes.end(); ++lithoTypeIter )
   {
      const LithoType * lithoType = *lithoTypeIter;
      if ( lithoType->getName() == name )
      {
         // Note that we return an Interface::LithoType
         return lithoType;
      }
   }
   return 0;
}

const Interface::SourceRock * ProjectHandle::findSourceRock( const string & name ) const
{
   MutableSourceRockList::const_iterator sourceRockIter;

   if ( name == "" ) return 0;

   for ( sourceRockIter = m_sourceRocks.begin(); sourceRockIter != m_sourceRocks.end(); ++sourceRockIter )
   {
      const SourceRock * sourceRock = *sourceRockIter;
      //      if (sourceRock->getLayerName () == name)
      if ( sourceRock->getType() == name )
      {
         return sourceRock;
      }
   }
   return 0;
}

const Interface::Surface * ProjectHandle::findSurface( const string & name ) const
{
   MutableSurfaceList::const_iterator surfaceIter;

   if ( name == "" ) return 0;

   for ( surfaceIter = m_surfaces.begin(); surfaceIter != m_surfaces.end(); ++surfaceIter )
   {
      const Surface * surface = *surfaceIter;
      if ( surface->getName() == name || surface->getMangledName() == name )
      {
         // Note that we return an Interface::Surface
         return surface;
      }
   }
   return 0;
}

const Interface::Property * ProjectHandle::findProperty( const string & name ) const
{
   MutablePropertyList::const_iterator propertyIter;
   for (propertyIter = m_properties.begin();
      propertyIter != m_properties.end();
      ++propertyIter)
   {
      const Property * property = *propertyIter;
      if (WildMatch( property->getCauldronName().c_str(), name.c_str() ) ||
         WildMatch( property->getUserName().c_str(), name.c_str() ))
      {
         // Note that we return an Interface::Property
         return property;
      }
   }
   LogHandler( LogHandler::WARNING_SEVERITY ) << "Property '" << name << "' could not be found by the ProjectHandle.";
   return 0;
}

const Interface::OutputProperty * ProjectHandle::findTimeOutputProperty( const string & propertyName ) const
{
   MutableOutputPropertyList::const_iterator propertyIter;

   for ( propertyIter = m_timeOutputProperties.begin();
      propertyIter != m_timeOutputProperties.end();
      ++propertyIter )
   {
      const OutputProperty * property = *propertyIter;

      if ( property->getName() == propertyName )
         //       if (WildMatch (property->getName ().c_str (), propertyName.c_str ()))
      {
         // Note that we return an Interface::OutputProperty
         return property;
      }
   }
   return 0;
}

/// Find the InputValue  with the given attributes
const Interface::InputValue * ProjectHandle::findInputValue( const string & tableName, const string & mapName ) const
{
   MutableInputValueList::const_iterator inputValueIter;

   for ( inputValueIter = m_inputValues.begin();
      inputValueIter != m_inputValues.end();
      ++inputValueIter )
   {
      const InputValue * inputValue = *inputValueIter;

      if ( inputValue->getReferringTableName() == tableName && inputValue->getMapName() == mapName )
      {
         // Note that we return an Interface::InputValue
         return inputValue;
      }
   }

   return 0;
}

const Interface::AllochthonousLithology * ProjectHandle::findAllochthonousLithology( const string& formationName ) const
{

   MutableAllochthonousLithologyList::const_iterator allochthonousLithologyIter;

   if ( formationName == "" ) return 0;

   for ( allochthonousLithologyIter = m_allochthonousLithologies.begin(); allochthonousLithologyIter != m_allochthonousLithologies.end(); ++allochthonousLithologyIter )
   {
      const Interface::AllochthonousLithology * allochthonousLithology = *allochthonousLithologyIter;

      if ( allochthonousLithology->getFormationName() == formationName )
      {
         return allochthonousLithology;
      }

   }

   return 0;
}


GridMap * ProjectHandle::loadInputMap( const string & referringTable, const string & mapName )
{
   const InputValue * inputValue = (InputValue *)findInputValue( referringTable, mapName );
   if ( !inputValue ) return 0;

   return (GridMap *)inputValue->getGridMap();
}

bool ProjectHandle::loadBiodegradationParameters()
{
   m_biodegradationParameters = 0;
   database::Table* bioDegradIoTbl = getTable( "BioDegradIoTbl" );
   if ( !bioDegradIoTbl )
      return false;

   Record* biodegradationRecord = bioDegradIoTbl->getRecord( 0 );
   if ( biodegradationRecord )
   {
      m_biodegradationParameters = getFactory()->produceBiodegradationParameters(
         this, biodegradationRecord );
      return true;
   }
   else
      return false;
}

const BiodegradationParameters* ProjectHandle::getBiodegradationParameters() const
{
   return m_biodegradationParameters;
}

const CrustFormation* ProjectHandle::getCrustFormation() const
{
   return m_crustFormation;
}

const MantleFormation* ProjectHandle::getMantleFormation() const
{
   return m_mantleFormation;
}

const Interface::RunParameters* ProjectHandle::getRunParameters() const
{
   return m_runParameters;
}

const Interface::ProjectData* ProjectHandle::getProjectData() const
{
   return m_projectData;
}

bool ProjectHandle::loadFracturePressureFunctionParameters()
{
   m_fracturePressureFunctionParameters = 0;

   database::Table* runOptionsIoTbl = getTable( "RunOptionsIoTbl" );
   if ( !runOptionsIoTbl )
      return false;

   Record* runOptionsIoTblRecord = runOptionsIoTbl->getRecord( 0 );

   database::Table* pressureFuncIoTbl = getTable( "PressureFuncIoTbl" );
   if ( !pressureFuncIoTbl )
      return false;

   Record* pressureFuncIoTblRecord = pressureFuncIoTbl->getRecord( 0 );
   for ( size_t r = 0; r < pressureFuncIoTbl->size(); ++r )
   {
      Record* curPressureFuncIoTblRecord = pressureFuncIoTbl->getRecord( static_cast<int>( r ) );
      if ( database::getSelected( curPressureFuncIoTblRecord ) == 1 )
      {
         pressureFuncIoTblRecord = curPressureFuncIoTblRecord;
         break;
      }
   }

   if ( runOptionsIoTblRecord && pressureFuncIoTblRecord )
   {
      m_fracturePressureFunctionParameters = getFactory()->produceFracturePressureFunctionParameters(
         this, runOptionsIoTblRecord, pressureFuncIoTblRecord );
      return true;
   }

   return false;
}

const FracturePressureFunctionParameters* ProjectHandle::getFracturePressureFunctionParameters() const
{
   return m_fracturePressureFunctionParameters;
}

bool ProjectHandle::loadDiffusionLeakageParameters()
{
   m_diffusionLeakageParameters = 0;

   database::Table* diffusionIoTbl = getTable( "DiffusionIoTbl" );
   if ( !diffusionIoTbl )
      return false;

   Record* diffusionLeakageRecord = diffusionIoTbl->getRecord( 0 );
   if ( diffusionLeakageRecord )
   {
      m_diffusionLeakageParameters = getFactory()->produceDiffusionLeakageParameters(
         this, diffusionLeakageRecord );
      return true;
   }
   else
      return false;
}


void ProjectHandle::loadLangmuirIsotherms() {

   database::Table* langmuirIsothermTable = getTable( "LangmuirAdsorptionCapacityIsothermSetIoTbl" );
   database::Table::iterator tblIter;

   for ( tblIter = langmuirIsothermTable->begin(); tblIter != langmuirIsothermTable->end(); ++tblIter ) {

      LangmuirAdsorptionIsothermSample* sample = getFactory()->produceLangmuirAdsorptionIsothermSample( this, *tblIter );

      m_langmuirIsotherms.push_back( sample );
   }

   std::sort( m_langmuirIsotherms.begin(), m_langmuirIsotherms.end(), LangmuirAdsorptionIsothermSampleLessThan() );
}

void ProjectHandle::loadLangmuirTOCEntries() {

   database::Table* langmuirIsothermTable = getTable( "LangmuirAdsorptionCapacityTOCFunctionIoTbl" );
   database::Table::iterator tblIter;

   for ( tblIter = langmuirIsothermTable->begin(); tblIter != langmuirIsothermTable->end(); ++tblIter ) {

      LangmuirAdsorptionTOCEntry* sample = getFactory()->produceLangmuirAdsorptionTOCEntry( this, *tblIter );

      m_langmuirTocAdsorptionEntries.push_back( sample );
   }

}


void ProjectHandle::loadPointHistories() {

   /*
      database::Table* pointHistoryTable = getTable ( "AdsorptionHistoryIoTbl" );
      database::Table::iterator tableIter;

      for ( tableIter = pointHistoryTable->begin (); tableIter != pointHistoryTable->end (); ++tableIter ) {
      PointAdsorptionHistory* newHistory = getFactory ()->producePointAdsorptionHistory ( this, *tableIter );
      m_adsorptionPointHistoryList.push_back ( newHistory );
      }
      */
   database::Table* pointHistoryTable = getTable( "GenexHistoryLocationIoTbl" );
   database::Table::iterator tableIter;

   for ( tableIter = pointHistoryTable->begin(); tableIter != pointHistoryTable->end(); ++tableIter ) {
      PointAdsorptionHistory* newHistory = getFactory()->producePointAdsorptionHistory( this, *tableIter );
      m_adsorptionPointHistoryList.push_back( newHistory );
   }

}

void ProjectHandle::loadIrreducibleWaterSaturationSample() {

   database::Table* irreducibleWaterSaturationTable = getTable( "IrreducibleWaterSaturationIoTbl" );

   if ( irreducibleWaterSaturationTable != 0 and irreducibleWaterSaturationTable->size() >= 1 ) {
      m_irreducibleWaterSample = getFactory()->produceIrreducibleWaterSaturationSample( this, irreducibleWaterSaturationTable->getRecord( 0 ) );
   }
   else {
      m_irreducibleWaterSample = 0;
   }

}


void ProjectHandle::loadSGDensitySample() {

   database::Table* densityTable = getTable( "SGDensityIoTbl" );

   if ( densityTable != 0 and densityTable->size() > 0 ) {
      m_sgDensitySample = getFactory()->produceSGDensitySample( this, densityTable->getRecord( 0 ) );
   }
   else {
      m_sgDensitySample = 0;
   }

}


const DiffusionLeakageParameters* ProjectHandle::getDiffusionLeakageParameters() const
{
   return m_diffusionLeakageParameters;
}

/// Connect Formation objects and Surface objects
bool ProjectHandle::connectSurfaces( void )
{
   MutableSurfaceList::iterator surfaceIter;
   MutableFormationList::iterator formationIter;

   formationIter = m_formations.begin();
   Formation * formation = 0;
   Surface * topSurface = 0;
   Surface * bottomSurface = 0;

   for ( surfaceIter = m_surfaces.begin(); surfaceIter != m_surfaces.end(); ++surfaceIter )
   {
      bottomSurface = *surfaceIter;


      if ( formation )
      {
         formation->setTopSurface( topSurface );
         formation->setBottomSurface( bottomSurface );

         topSurface->setBottomFormation( formation );
         bottomSurface->setTopFormation( formation );
      }

      if ( formationIter != m_formations.end() )
      {
         formation = *formationIter;
         ++formationIter;
      }
      else
      {
         formation = 0;
      }

      topSurface = bottomSurface;
   }

   return true;
}

/// Connect Reservoir objects to their Formation objects
bool ProjectHandle::connectReservoirs( void )
{
   MutableReservoirList::iterator reservoirIter;

   for ( reservoirIter = m_reservoirs.begin(); reservoirIter != m_reservoirs.end(); ++reservoirIter )
   {
      Reservoir * reservoir = *reservoirIter;
      const Formation * formation = dynamic_cast<const Formation *>( findFormation( reservoir->getFormationName() ) );
      assert( formation != 0 );
      reservoir->setFormation( formation );
   }
   return true;
}

/// Connect Trap objects to their Reservoir objects and Snapshot objects
bool ProjectHandle::connectTraps( void )
{
   MutableTrapList::iterator trapIter;

   for ( trapIter = m_traps.begin(); trapIter != m_traps.end(); ++trapIter )
   {
      Trap * trap = *trapIter;
      const string & name = database::getReservoirName( trap->getRecord() );
      const Reservoir * reservoir = (const Reservoir *)findReservoir( name );
      if ( reservoir != 0 )
      {
         trap->setReservoir( reservoir );
      }
      else
      {
         delete * trapIter;
         m_traps.erase( trapIter );
         --trapIter;
         continue;
      }

      double time = database::getAge( trap->getRecord() );
      const Snapshot * snapshot = (const Snapshot *)findSnapshot( time );
      if ( snapshot != 0 )
      {
         trap->setSnapshot( snapshot );
      }
      else
      {
         delete * trapIter;
         m_traps.erase( trapIter );
         --trapIter;
         continue;
      }
   }
   return true;
}

/// check of the m_trappers is not empty
bool ProjectHandle::trappersAreAvailable()
{
   return !m_trappers.empty();
}

/// Connect Trapper objects to their Reservoir objects and Snapshot objects
bool ProjectHandle::connectTrappers( void )
{
   MutableTrapperList::iterator trapperIter;

   for ( trapperIter = m_trappers.begin(); trapperIter != m_trappers.end(); ++trapperIter )
   {
      Trapper * trapper = *trapperIter;
      const string & name = database::getReservoirName( trapper->getRecord() );
      const Reservoir * reservoir = (const Reservoir *)findReservoir( name );
      if ( reservoir != 0 )
      {
         trapper->setReservoir( reservoir );
      }
      else
      {
         delete * trapperIter;
         m_trappers.erase( trapperIter );
         --trapperIter;
         continue;
      }

      double time = database::getAge( trapper->getRecord() );
      const Snapshot * snapshot = (const Snapshot *)findSnapshot( time );
      if ( snapshot != 0 )
      {
         trapper->setSnapshot( snapshot );
      }
      else
      {
         delete * trapperIter;
         m_trappers.erase( trapperIter );
         --trapperIter;
         continue;
      }
   }
   return true;
}

/// Connect Migration objects to their source and destination objects
bool ProjectHandle::connectMigrations( void )
{
   MutableMigrationList::iterator migrationIter;

   int increment = 1;

   for ( migrationIter = m_migrations.begin(); migrationIter != m_migrations.end(); migrationIter += increment )
   {
      increment = 1;
      Migration * migration = *migrationIter;

      try
      {
         double sourceAge = database::getSourceAge( migration->getRecord() );
         const Snapshot * sourceSnapshot = (const Snapshot *)findSnapshot( sourceAge );
         if ( sourceSnapshot == 0 ) throw RecordException( "Undefined sourceSnapshot: %", sourceAge );
         migration->setSourceSnapshot( sourceSnapshot );

         double destinationAge = database::getDestinationAge( migration->getRecord() );
         const Snapshot * destinationSnapshot = (const Snapshot *)findSnapshot( destinationAge );
         if ( destinationSnapshot == 0 ) throw RecordException( "Undefined destinationSnapshot: %", destinationAge );
         migration->setDestinationSnapshot( destinationSnapshot );

         const string & sourceFormationName = database::getSourceRockName( migration->getRecord() );
         if ( sourceFormationName != "" )
         {
            const Formation * sourceFormation = dynamic_cast<const Formation *>( findFormation( sourceFormationName ) );
            if ( sourceFormation == 0 ) throw RecordException( "Undefined sourceFormation: %", sourceFormationName );
            migration->setSourceFormation( sourceFormation );
         }

         const string & sourceReservoirName = database::getSourceReservoirName( migration->getRecord() );
         if ( sourceReservoirName != "" )
         {
            const Reservoir * sourceReservoir = (const Reservoir *)findReservoir( sourceReservoirName );
            if ( sourceReservoir == 0 ) throw RecordException( "Undefined sourceReservoir: %", sourceReservoirName );
            migration->setSourceReservoir( sourceReservoir );
         }

         const string & destinationReservoirName = database::getDestinationReservoirName( migration->getRecord() );
         if ( destinationReservoirName != "" )
         {
            const Reservoir * destinationReservoir = (const Reservoir *)findReservoir( destinationReservoirName );
            if ( destinationReservoir == 0 ) throw RecordException( "Undefined destinationReservoir: %", destinationReservoirName );
            migration->setDestinationReservoir( destinationReservoir );
         }

         int sourceTrapperId = database::getSourceTrapID( migration->getRecord() );
         if ( sourceTrapperId > 0 )
         {
            Trapper * sourceTrapper = findTrapper( migration->getSourceReservoir(), migration->getSourceSnapshot(), sourceTrapperId, 0 );
            if ( sourceTrapper == 0 ) throw RecordException( "Cannot find Trapper: %:%:%", sourceAge, sourceReservoirName, sourceTrapperId );
            migration->setSourceTrapper( sourceTrapper );
         }

         int destinationTrapperId = database::getDestinationTrapID( migration->getRecord() );
         if ( destinationTrapperId > 0 )
         {
            Trapper * destinationTrapper = findTrapper( migration->getDestinationReservoir(), migration->getDestinationSnapshot(), destinationTrapperId, 0 );
            if ( destinationTrapper == 0 ) throw RecordException( "Cannot find Trapper: %:%:%", destinationAge, destinationReservoirName, destinationTrapperId );
            migration->setDestinationTrapper( destinationTrapper );
         }
      }
      catch ( RecordException & recordException )
      {
         cerr << "Error: " << recordException.what() << ", removing offending Migration record" << endl;
         delete * migrationIter;
         migrationIter = m_migrations.erase( migrationIter );
         increment = 0;
         continue;
      }
   }
   return true;
}


bool ProjectHandle::connectUpAndDownstreamTrappers( void ) const
{
   database::Table* migrationTbl = getTable( "MigrationIoTbl" );
   database::Table::iterator tblIter;

   Reservoir * previousReservoir = 0;
   Snapshot * previousSnapshot = 0;

   for ( tblIter = migrationTbl->begin(); tblIter != migrationTbl->end(); ++tblIter )
   {
      Record * migrationRecord = *tblIter;
      if ( database::getMigrationProcess( migrationRecord ) == "Spill" )
      {
         Snapshot * snapshot = (Snapshot *)findSnapshot( database::getSourceAge( migrationRecord ) );
         assert( snapshot );

         Reservoir * reservoir = (Reservoir *)findReservoir( database::getSourceReservoirName( migrationRecord ) );
         assert( reservoir );

         if ( ( reservoir != previousReservoir || snapshot != previousSnapshot ) && previousReservoir && previousSnapshot )
         {
            previousReservoir->setTrappersUpAndDownstreamConnected( previousSnapshot->getTime() );
         }

         if ( reservoir->trappersAreUpAndDownstreamConnected( snapshot->getTime() ) ) continue;

         int upstreamTrapperId = database::getSourceTrapID( migrationRecord );
         int downstreamTrapperId = database::getDestinationTrapID( migrationRecord );
         if ( upstreamTrapperId <= 0 || downstreamTrapperId <= 0 || upstreamTrapperId == downstreamTrapperId ) continue;

         Trapper * upstreamTrapper = findTrapper( ( Interface::Reservoir * ) reservoir, ( Interface::Snapshot * ) snapshot, upstreamTrapperId, 0 );
         Trapper * downstreamTrapper = findTrapper( ( Interface::Reservoir * ) reservoir, ( Interface::Snapshot * ) snapshot, downstreamTrapperId, 0 );
         if ( upstreamTrapper == 0 || downstreamTrapper == 0 ) continue;

         if ( upstreamTrapper->getDownstreamTrapper() != 0 || upstreamTrapper->getDownstreamTrapper() == downstreamTrapper ) continue;

         upstreamTrapper->setDownstreamTrapper( downstreamTrapper );
         downstreamTrapper->addUpstreamTrapper( upstreamTrapper );
      }
   }

   if ( previousReservoir && previousSnapshot )
   {
      previousReservoir->setTrappersUpAndDownstreamConnected( previousSnapshot->getTime() );
   }

   return true;
}

struct CmpTrapIdToValue
{
  bool operator()(const Trapper* trapper, unsigned int trapperId) const
  {
    return trapper->getId() < trapperId;
  }
};

Interface::Trapper * ProjectHandle::findTrapper( const Interface::MutableTrapperList & trappers, const Interface::Reservoir * reservoir,
   const Interface::Snapshot * snapshot, unsigned int id, unsigned int persistentId ) const
{
  // Trappers are sorted on id, so when findTrapper() is called with an id != 0, we can use
  // bisection to quickly find the location of the first trapper with that id. If id == 0,
  // a (slow) linear search is performed.

  if (id != 0)
  {
    Interface::MutableTrapperList::const_iterator lower = std::lower_bound(
      trappers.begin(),
      trappers.end(),
      id,
      CmpTrapIdToValue());

    Interface::MutableTrapperList::const_iterator iter = lower;
    for (; iter != trappers.end() && (*iter)->getId() == id; ++iter)
    {
      if ((*iter)->matchesConditions(reservoir, snapshot, id, persistentId))
        return (*iter);
    }

    return 0;
  }
  else
  {
    Interface::MutableTrapperList::const_iterator trapperIter;

    for (trapperIter = trappers.begin(); trapperIter != trappers.end(); ++trapperIter)
    {
      Trapper * trapper = *trapperIter;

      if (trapper->matchesConditions((Reservoir *)reservoir, (Snapshot *)snapshot, id, persistentId))
        return trapper;
    }
    return 0;
  }
}

Interface::Trapper * ProjectHandle::findTrapper( const Interface::Reservoir * reservoir,
   const Interface::Snapshot * snapshot, unsigned int id, unsigned int persistentId ) const
{
  return findTrapper( m_trappers, reservoir, snapshot, id, persistentId );
}

Interface::TrapperList* ProjectHandle::getTrappers(const Interface::Reservoir* reservoir, 
  const Interface::Snapshot* snapshot, unsigned int id, unsigned int persistentId) const
{
  Interface::TrapperList* trapperList = new Interface::TrapperList;

  Interface::MutableTrapperList::const_iterator trapperIter;

  for (trapperIter = m_trappers.begin(); trapperIter != m_trappers.end(); ++trapperIter)
  {
    Trapper * trapper = *trapperIter;

    if (trapper->matchesConditions(reservoir, snapshot, id, persistentId))
      trapperList->push_back(trapper);
  }

  return trapperList;
}


void ProjectHandle::deleteTimeOutputProperties() {

   MutableOutputPropertyList::const_iterator propIter;

   for ( propIter = m_timeOutputProperties.begin(); propIter != m_timeOutputProperties.end(); ++propIter ) {
      OutputProperty * property = *propIter;
      delete property;
   }

   m_timeOutputProperties.clear();
}

void ProjectHandle::deleteDepthOutputProperties() {

   MutableOutputPropertyList::const_iterator propIter;

   for ( propIter = m_depthOutputProperties.begin(); propIter != m_depthOutputProperties.end(); ++propIter ) {
      OutputProperty * property = *propIter;
      delete property;
   }

   m_depthOutputProperties.clear();
}



void ProjectHandle::deleteLithologyThermalConductivitySamples() {

   MutableLithologyThermalConductivitySampleList::iterator sampleIter;

   for ( sampleIter = m_lithologyThermalConductivitySamples.begin(); sampleIter != m_lithologyThermalConductivitySamples.end(); ++sampleIter ) {
      LithologyThermalConductivitySample * sample = *sampleIter;
      delete sample;
   }

   m_lithologyThermalConductivitySamples.clear();
}

void ProjectHandle::deleteLithologyHeatCapacitySamples() {

   MutableLithologyHeatCapacitySampleList::const_iterator sampleIter;

   for ( sampleIter = m_lithologyHeatCapacitySamples.begin(); sampleIter != m_lithologyHeatCapacitySamples.end(); ++sampleIter ) {
      LithologyHeatCapacitySample * sample = *sampleIter;
      delete sample;
   }

   m_lithologyHeatCapacitySamples.clear();
}


void ProjectHandle::deleteFluidHeatCapacitySamples() {

   MutableFluidHeatCapacitySampleList::const_iterator sampleIter;

   for ( sampleIter = m_fluidHeatCapacitySamples.begin(); sampleIter != m_fluidHeatCapacitySamples.end(); ++sampleIter ) {
      FluidHeatCapacitySample * sample = *sampleIter;
      delete sample;
   }

   m_fluidHeatCapacitySamples.clear();
}

void ProjectHandle::deleteFluidThermalConductivitySamples() {

   MutableFluidThermalConductivitySampleList::const_iterator sampleIter;

   for ( sampleIter = m_fluidThermalConductivitySamples.begin(); sampleIter != m_fluidThermalConductivitySamples.end(); ++sampleIter ) {
      FluidThermalConductivitySample * sample = *sampleIter;
      delete sample;
   }

   m_fluidThermalConductivitySamples.clear();
}

void ProjectHandle::deleteFluidDensitySamples() {

   MutableFluidDensitySampleList::const_iterator sampleIter;

   for ( sampleIter = m_fluidDensitySamples.begin(); sampleIter != m_fluidDensitySamples.end(); ++sampleIter ) {
      FluidDensitySample * sample = *sampleIter;
      delete sample;
   }

   m_fluidDensitySamples.clear();
}


void ProjectHandle::deleteRelatedProjects() {

   MutableRelatedProjectList::const_iterator relatedProjectIter;

   for ( relatedProjectIter = m_relatedProjects.begin(); relatedProjectIter != m_relatedProjects.end(); ++relatedProjectIter ) {
      RelatedProject * project = *relatedProjectIter;
      delete project;
   }

   m_relatedProjects.clear();
}


void ProjectHandle::deleteConstrainedOverpressureIntervals() {

   MutableConstrainedOverpressureIntervalList::iterator constraintIter;

   for ( constraintIter = m_constrainedOverpressureIntervals.begin(); constraintIter != m_constrainedOverpressureIntervals.end(); ++constraintIter ) {
      ConstrainedOverpressureInterval* interval = *constraintIter;
      delete interval;
   }

}


void ProjectHandle::deleteSnapshots( void )
{
   MutableSnapshotList::const_iterator snapshotIter;

   for ( snapshotIter = m_snapshots.begin(); snapshotIter != m_snapshots.end(); ++snapshotIter )
   {
      Snapshot * snapshot = *snapshotIter;
      delete snapshot;
   }
   m_snapshots.clear();
}

void ProjectHandle::deleteLithoTypes( void )
{
   MutableLithoTypeList::const_iterator lithoTypeIter;

   for ( lithoTypeIter = m_lithoTypes.begin(); lithoTypeIter != m_lithoTypes.end(); ++lithoTypeIter )
   {
      LithoType * lithoType = *lithoTypeIter;
      delete lithoType;
   }
   m_lithoTypes.clear();
}

void ProjectHandle::deleteSourceRocks( void )
{
   MutableSourceRockList::const_iterator sourceRockIter;

   for ( sourceRockIter = m_sourceRocks.begin(); sourceRockIter != m_sourceRocks.end(); ++sourceRockIter )
   {
      SourceRock * sourceRock = *sourceRockIter;
      delete sourceRock;
   }
   m_sourceRocks.clear();
}

void ProjectHandle::deleteSurfaces( void )
{
   MutableSurfaceList::const_iterator surfaceIter;

   for ( surfaceIter = m_surfaces.begin(); surfaceIter != m_surfaces.end(); ++surfaceIter )
   {
      Surface * surface = *surfaceIter;
      delete surface;
   }
   m_surfaces.clear();
}

void ProjectHandle::deleteFormations( void )
{
   MutableFormationList::const_iterator formationIter;

   for ( formationIter = m_formations.begin(); formationIter != m_formations.end(); ++formationIter )
   {
      Formation * formation = *formationIter;
      delete formation;
   }
   m_formations.clear();
}

void ProjectHandle::deleteFluidTypes() {

   MutableFluidTypeList::iterator fluidIter;

   for ( fluidIter = m_fluidTypes.begin(); fluidIter != m_fluidTypes.end(); ++fluidIter )
   {
      FluidType* fluid = ( *fluidIter );
      delete fluid;
   }

   m_fluidTypes.clear();
}

void ProjectHandle::deleteCrustalThicknessData() {

   MutableCrustalThicknessDataList::const_iterator ctcIter;

   for ( ctcIter = m_crustalThicknessData.begin(); ctcIter != m_crustalThicknessData.end(); ++ctcIter )
   {
      CrustalThicknessData* ctcData = *ctcIter;
      delete ctcData;
   }

   m_crustalThicknessData.clear();
}

void ProjectHandle::deleteReservoirs( void )
{
   MutableReservoirList::const_iterator reservoirIter;

   for ( reservoirIter = m_reservoirs.begin(); reservoirIter != m_reservoirs.end(); ++reservoirIter )
   {
      Reservoir * reservoir = *reservoirIter;
      delete reservoir;
   }
   m_reservoirs.clear();
}

void ProjectHandle::deleteMobileLayers( void )
{
   MutableMobileLayerList::const_iterator mobileLayerIter;

   for ( mobileLayerIter = m_mobileLayers.begin(); mobileLayerIter != m_mobileLayers.end(); ++mobileLayerIter )
   {
      MobileLayer * mobileLayer = *mobileLayerIter;
      delete mobileLayer;
   }
   m_mobileLayers.clear();
}

void ProjectHandle::deleteTouchstoneMaps( void )
{
   MutableTouchstoneMapList::const_iterator touchstoneMapIter;

   for ( touchstoneMapIter = m_touchstoneMaps.begin(); touchstoneMapIter != m_touchstoneMaps.end(); ++touchstoneMapIter )
   {
      TouchstoneMap * touchstoneMap = *touchstoneMapIter;
      delete touchstoneMap;
   }
   m_touchstoneMaps.clear();
}

void ProjectHandle::deleteAllochthonousLithologies( void ) {

   MutableAllochthonousLithologyList::const_iterator allochthonousLithologyIter;

   for ( allochthonousLithologyIter = m_allochthonousLithologies.begin(); allochthonousLithologyIter != m_allochthonousLithologies.end(); ++allochthonousLithologyIter )
   {
      AllochthonousLithology * allochthonousLithology = *allochthonousLithologyIter;
      delete allochthonousLithology;
   }

   m_allochthonousLithologies.clear();
}



void ProjectHandle::deleteAllochthonousLithologyDistributions( void ) {

   MutableAllochthonousLithologyDistributionList::const_iterator allochthonousLithologyDistributionIter;

   for ( allochthonousLithologyDistributionIter = m_allochthonousLithologyDistributions.begin();
      allochthonousLithologyDistributionIter != m_allochthonousLithologyDistributions.end();
      ++allochthonousLithologyDistributionIter )
   {
      AllochthonousLithologyDistribution * allochthonousLithologyDistribution = *allochthonousLithologyDistributionIter;
      delete allochthonousLithologyDistribution;
   }

   m_allochthonousLithologyDistributions.clear();
}


void ProjectHandle::deleteAllochthonousLithologyInterpolations( void ) {

   MutableAllochthonousLithologyInterpolationList::const_iterator allochthonousLithologyInterpolationIter;

   for ( allochthonousLithologyInterpolationIter = m_allochthonousLithologyInterpolations.begin();
      allochthonousLithologyInterpolationIter != m_allochthonousLithologyInterpolations.end();
      ++allochthonousLithologyInterpolationIter )
   {
      AllochthonousLithologyInterpolation * allochthonousLithologyInterpolation = *allochthonousLithologyInterpolationIter;
      delete allochthonousLithologyInterpolation;
   }

   m_allochthonousLithologyInterpolations.clear();
}


void ProjectHandle::deleteTraps( void )
{
   MutableTrapList::const_iterator trapIter;

   for ( trapIter = m_traps.begin(); trapIter != m_traps.end(); ++trapIter )
   {
      Trap * trap = *trapIter;
      delete trap;
   }
   m_traps.clear();
}

void ProjectHandle::deleteTrappers( void )
{
   MutableTrapperList::const_iterator trapperIter;

   for ( trapperIter = m_trappers.begin(); trapperIter != m_trappers.end(); ++trapperIter )
   {
      Trapper * trapper = *trapperIter;
      delete trapper;
   }
   m_trappers.clear();
}

void ProjectHandle::deleteMigrations( void )
{
   MutableMigrationList::const_iterator migrationIter;

   for ( migrationIter = m_migrations.begin(); migrationIter != m_migrations.end(); ++migrationIter )
   {
      Migration * migration = *migrationIter;
      delete migration;
   }
   m_migrations.clear();
}

void ProjectHandle::deleteIgneousIntrusions() {

   MutableIgneousIntrusionEventList::iterator intrusionIter;

   for ( intrusionIter = m_igneousIntrusionEvents.begin(); intrusionIter != m_igneousIntrusionEvents.end(); ++intrusionIter )
   {
      IgneousIntrusionEvent* intrusionEvent = *intrusionIter;

      delete intrusionEvent;

   }

   m_igneousIntrusionEvents.clear();
}

void ProjectHandle::deleteInputValues( void )
{
   MutableInputValueList::const_iterator inputValueIter;

   for ( inputValueIter = m_inputValues.begin(); inputValueIter != m_inputValues.end(); ++inputValueIter )
   {
      InputValue * inputValue = *inputValueIter;
      delete inputValue;
   }
   m_inputValues.clear();
}

void ProjectHandle::deletePermafrost() {

   MutablePermafrostEventList::iterator permafrostIter;

   for ( permafrostIter = m_permafrostEvents.begin(); permafrostIter != m_permafrostEvents.end(); ++permafrostIter )
   {
      PermafrostEvent* permafrostEvent = *permafrostIter;

      delete permafrostEvent;

   }

   m_permafrostEvents.clear();
}

void ProjectHandle::deleteProperties( void )
{
   MutablePropertyList::const_iterator propertyIter;

   for ( propertyIter = m_properties.begin(); propertyIter != m_properties.end(); ++propertyIter )
   {
      Property * property = *propertyIter;
      delete property;
   }
   m_properties.clear();
}

void ProjectHandle::deletePropertyValues( void )
{
   MutablePropertyValueList::const_iterator propertyValueIter;

   for ( propertyValueIter = m_propertyValues.begin();
      propertyValueIter != m_propertyValues.end();
      ++propertyValueIter )
   {
      PropertyValue * propertyValue = *propertyValueIter;
      delete propertyValue;
   }
   m_propertyValues.clear();
}

void ProjectHandle::deleteBiodegradationParameters( void )
{
   delete m_biodegradationParameters;
}

void ProjectHandle::deleteCrustFormation( void )
{
   delete m_crustFormation;
}

void ProjectHandle::deleteMantleFormation( void )
{
   delete m_mantleFormation;
}

void ProjectHandle::deleteFracturePressureFunctionParameters( void )
{
   delete m_fracturePressureFunctionParameters;
}

void ProjectHandle::deleteDiffusionLeakageParameters( void )
{
   delete m_diffusionLeakageParameters;
}

void ProjectHandle::deleteHeatFlowHistory( void ) {

   MutablePaleoSurfacePropertyList::const_iterator heatFlowHistoryIter;

   for ( heatFlowHistoryIter = m_heatFlowHistory.begin(); heatFlowHistoryIter != m_heatFlowHistory.end(); ++heatFlowHistoryIter )
   {
      PaleoSurfaceProperty * heatFlowInstance = *heatFlowHistoryIter;
      delete heatFlowInstance;
   }

}

void ProjectHandle::deleteCrustThinningHistory( void ) {

   MutablePaleoFormationPropertyList::const_iterator crustThinningHistoryIter;

   for ( crustThinningHistoryIter = m_crustPaleoThicknesses.begin(); crustThinningHistoryIter != m_crustPaleoThicknesses.end(); ++crustThinningHistoryIter )
   {
      PaleoFormationProperty * crustThinningInstance = *crustThinningHistoryIter;
      delete crustThinningInstance;
   }

}

void ProjectHandle::deleteMantleThicknessHistory( void ) {

   MutablePaleoFormationPropertyList::const_iterator mantleThicknessHistoryIter;

   for ( mantleThicknessHistoryIter = m_mantlePaleoThicknesses.begin(); mantleThicknessHistoryIter != m_mantlePaleoThicknesses.end(); ++mantleThicknessHistoryIter )
   {
      PaleoFormationProperty * mantleThicknessInstance = *mantleThicknessHistoryIter;
      delete mantleThicknessInstance;
   }

}

void ProjectHandle::deleteRunParameters( void ) {

   if ( m_runParameters != 0 ) {
      delete m_runParameters;
   }

}

void ProjectHandle::deleteProjectData( void ) {

   if ( m_projectData != 0 ) {
      delete m_projectData;
   }

}

void ProjectHandle::setSimulationDetails ( const std::string& simulatorName,
                                           const std::string& simulatorMode,
                                           const std::string& simulatorCommandLineParams ) {

   database::Table* simulationDetailsIoTbl = getTable( "SimulationDetailsIoTbl" );
   database::Record* sdRecord = simulationDetailsIoTbl->createRecord();

   int lastSequenceNumber = 0;

   if ( m_simulationDetails.size () > 0 ) {
      // Sequence of simulation details should be ordered, so the last entry should have the largest seqnence number.
      lastSequenceNumber = m_simulationDetails [ m_simulationDetails.size () - 1 ]->getSimulationSequenceNumber ();
   }

   database::setSimulatorName ( sdRecord, simulatorName );
   database::setSimulatorMode ( sdRecord, simulatorMode );
   database::setSimulatorCommandLineParameters ( sdRecord, simulatorCommandLineParams );
   database::setSimulationSequenceNumber ( sdRecord, lastSequenceNumber + 1 );
   database::setNumberOfCores ( sdRecord, m_size );
}

SimulationDetailsListPtr ProjectHandle::getSimulationDetails () const {

   SimulationDetailsListPtr result = SimulationDetailsListPtr ( new SimulationDetailsList ( m_simulationDetails.size ()));

   for ( size_t i = 0; i < m_simulationDetails.size (); ++i ) {
      (*result)[ i ] = m_simulationDetails [ i ];
   } 

   return result;
}

const SimulationDetails* ProjectHandle::getDetailsOfLastSimulation ( const std::string& simulatorName ) const {

   MutableSimulationDetailsList::const_reverse_iterator simDetailsIter;

   for ( simDetailsIter = m_simulationDetails.rbegin (); simDetailsIter != m_simulationDetails.rend (); ++simDetailsIter ) {

      if ((*simDetailsIter)->getSimulatorName () == simulatorName ) {
         return *simDetailsIter;
      }
   }
   

   return 0;
}

void ProjectHandle::deleteSimulationDetails () {

   for ( size_t i = 0; i < m_simulationDetails.size (); ++i ) {
      delete m_simulationDetails [ i ];
      m_simulationDetails [ i ] = 0;
   }

   m_simulationDetails.clear ();
}


void ProjectHandle::deleteSurfaceDepthHistory( void ) {

   MutablePaleoPropertyList::const_iterator surfaceDepthHistoryIter;

   for ( surfaceDepthHistoryIter = m_surfaceDepthHistory.begin(); surfaceDepthHistoryIter != m_surfaceDepthHistory.end(); ++surfaceDepthHistoryIter )
   {
      PaleoProperty * surfaceDepthInstance = *surfaceDepthHistoryIter;
      delete surfaceDepthInstance;
   }

}

void ProjectHandle::deleteSurfaceTemperatureHistory( void ) {

   MutablePaleoPropertyList::const_iterator surfaceTemperatureHistoryIter;

   for ( surfaceTemperatureHistoryIter = m_surfaceTemperatureHistory.begin(); surfaceTemperatureHistoryIter != m_surfaceTemperatureHistory.end(); ++surfaceTemperatureHistoryIter )
   {
      PaleoProperty * surfaceTemperatureInstance = *surfaceTemperatureHistoryIter;
      delete surfaceTemperatureInstance;
   }

}

void ProjectHandle::deleteSGDensitySample() {

   if ( m_sgDensitySample != 0 ) {
      delete m_sgDensitySample;
      m_sgDensitySample = 0;
   }

}


void ProjectHandle::deleteIrreducibleWaterSaturationSample() {

   if ( m_irreducibleWaterSample != 0 ) {
      delete m_irreducibleWaterSample;
   }

}

void ProjectHandle::deleteLangmuirIsotherms() {

   MutableLangmuirAdsorptionIsothermSampleList::iterator langIter;

   for ( langIter = m_langmuirIsotherms.begin(); langIter != m_langmuirIsotherms.end(); ++langIter ) {
      delete ( *langIter );
   }

}

void ProjectHandle::deleteLangmuirTOCEntries() {

   MutableLangmuirAdsorptionTOCEntryList::iterator langIter;

   for ( langIter = m_langmuirTocAdsorptionEntries.begin(); langIter != m_langmuirTocAdsorptionEntries.end(); ++langIter ) {
      delete ( *langIter );
   }

}

void ProjectHandle::deletePointHistories() {

   MutablePointAdsorptionHistoryList::iterator histIter;

   for ( histIter = m_adsorptionPointHistoryList.begin(); histIter != m_adsorptionPointHistoryList.end(); ++histIter ) {
      delete ( *histIter );
   }

   m_adsorptionPointHistoryList.clear();
}


/// Get a project's output directory
std::string ProjectHandle::getOutputDir( void ) const
{
   return getProjectName() +  "_CauldronOutputDir";
}

void ProjectHandle::resetSnapshotIoTbl(  ) const
{

   Table * table = getTable( "SnapshotIoTbl" );
   if ( !table ) return;

   for ( size_t i = 0; i < table->size(); ++i )
   {
      Record * record = table->getRecord( static_cast<int>( i ) );
      setSnapshotFileName( record, "" );
   }
}


const Grid * ProjectHandle::findOutputGrid( int numI, int numJ ) const
{
   if ( getLowResolutionOutputGrid()->numIGlobal() == numI && getLowResolutionOutputGrid()->numJGlobal() == numJ )
   {
      return getLowResolutionOutputGrid();
   }
   else if ( getHighResolutionOutputGrid()->numIGlobal() == numI && getHighResolutionOutputGrid()->numJGlobal() == numJ )
   {
      return getHighResolutionOutputGrid();
   }
   else
   {
      return 0;
   }
}

const Grid * ProjectHandle::findGrid( int numI, int numJ ) const
{
   if ( getInputGrid()->numIGlobal() == numI && getInputGrid()->numJGlobal() == numJ )
   {
      return getInputGrid();
   }
   else
   {
      return findOutputGrid( numI, numJ );
   }
}

Interface::PointAdsorptionHistoryList* ProjectHandle::getPointAdsorptionHistoryList( const std::string& sourceRockFormationName ) const {

   Interface::PointAdsorptionHistoryList* historyList = new Interface::PointAdsorptionHistoryList;
   MutablePointAdsorptionHistoryList::const_iterator pointIter;

   for ( pointIter = m_adsorptionPointHistoryList.begin(); pointIter != m_adsorptionPointHistoryList.end(); ++pointIter ) {

      if ( sourceRockFormationName == "" or( *pointIter )->getFormationName() == sourceRockFormationName ) {
         historyList->push_back( *pointIter );
      }

   }

   return historyList;
}

Interface::LangmuirAdsorptionIsothermSampleList* ProjectHandle::getLangmuirAdsorptionIsothermSampleList( const std::string& functionName ) const {

   Interface::LangmuirAdsorptionIsothermSampleList* isothermList = new Interface::LangmuirAdsorptionIsothermSampleList;
   MutableLangmuirAdsorptionIsothermSampleList::const_iterator isothermIter;

   for ( isothermIter = m_langmuirIsotherms.begin(); isothermIter != m_langmuirIsotherms.end(); ++isothermIter ) {

      if ( ( *isothermIter )->getLangmuirName() == functionName ) {
         isothermList->push_back( *isothermIter );
      }

   }

   return isothermList;
}


const Interface::LangmuirAdsorptionTOCEntry* ProjectHandle::getLangmuirAdsorptionTOCEntry( const std::string& langmuirName ) const {

   const Interface::LangmuirAdsorptionTOCEntry* tocEntry = 0;
   MutableLangmuirAdsorptionTOCEntryList::const_iterator tocEntryIter;

   for ( tocEntryIter = m_langmuirTocAdsorptionEntries.begin(); tocEntryIter != m_langmuirTocAdsorptionEntries.end(); ++tocEntryIter ) {

      if ( ( *tocEntryIter )->getLangmuirName() == langmuirName ) {
         tocEntry = *tocEntryIter;
      }

   }

   return tocEntry;
}

const Interface::IrreducibleWaterSaturationSample* ProjectHandle::getIrreducibleWaterSaturationSample() const {
   return m_irreducibleWaterSample;
}

const Interface::SGDensitySample* ProjectHandle::getSGDensitySample() const {
   return m_sgDensitySample;
}

bool ProjectHandle::containsSulphur() const {

   MutableSourceRockList::const_iterator sourceRockIter;
   for ( sourceRockIter = m_sourceRocks.begin(); sourceRockIter != m_sourceRocks.end(); ++sourceRockIter ) {
      if ( !( ( *sourceRockIter )->getLayerName().empty() ) and( *sourceRockIter )->getScVRe05() != DefaultUndefinedValue and( *sourceRockIter )->getScVRe05() > 0.0 ) {
         return true;
      }
   }

   MutableFormationList::const_iterator formationIter;
   for ( formationIter = m_formations.begin(); formationIter != m_formations.end(); ++formationIter ) {
      Formation * formation = *formationIter;
      if ( formation->isSourceRock() ) {
         const SourceRock * sourceRock1 = formation->getSourceRock1();
         if ( sourceRock1 != 0 and sourceRock1->getScVRe05() != DefaultUndefinedValue and sourceRock1->getScVRe05() > 0.0 ) {
            return true;
         }
         if ( formation->getEnableSourceRockMixing() ) {
            const SourceRock * sourceRock2 = formation->getSourceRock2();
            if ( sourceRock2 != 0 and sourceRock2->getScVRe05() != DefaultUndefinedValue and sourceRock2->getScVRe05() > 0.0 ) {
               return true;
            }
         }
      }
   }
   return false;
}

void ProjectHandle::loadPermafrostData() {

   database::Table* permafrostIoTbl = getTable( "PermafrostIoTbl" );

   m_permafrostEvents.clear();
   m_permafrost = false;

   if ( permafrostIoTbl != 0 ) {
      Record *projectIoRecord = permafrostIoTbl->getRecord( 0 );
      if ( projectIoRecord != 0 ) {
         PermafrostEvent * permafrostRecord = getFactory()->producePermafrostEvent( this, projectIoRecord );
         m_permafrostEvents.push_back( permafrostRecord );

         m_permafrost = ( database::getPermafrostInd( projectIoRecord ) == 1 );
      }
   }
}

DataAccess::Interface::PermafrostEvent * ProjectHandle::getPermafrostData() const {

   if ( m_permafrostEvents.size() != 0 ) {
      return *m_permafrostEvents.begin();
   }
   return 0;
}

bool  ProjectHandle::getPermafrost() const {

   return m_permafrost;

}

void ProjectHandle::setPermafrost( const bool aPermafrost )
{
   m_permafrost = aPermafrost;

   DataAccess::Interface::PermafrostEvent * permafrostRecord = getPermafrostData();
   if ( permafrostRecord != 0 ) {
      permafrostRecord->setPermafrost( aPermafrost );
   }
}


double ProjectHandle::getPreviousIgneousIntrusionTime( const double Current_Time ){
   MutableIgneousIntrusionEventList::const_iterator intrusionIter;
   for ( intrusionIter = m_igneousIntrusionEvents.begin(); intrusionIter != m_igneousIntrusionEvents.end(); ++intrusionIter )
   {
      if ( Current_Time == (*intrusionIter)->getEndOfIntrusion()->getTime() )
      {
         m_previousIgneousIntrusionTime = Current_Time;
      }
   }
   return m_previousIgneousIntrusionTime;
}

MapWriter * ProjectHandle::getMapPropertyValuesWriter() {

   return m_mapPropertyValuesWriter;
}

bool ProjectHandle::isPrimaryDouble() const {

   return m_primaryDouble;
}

void ProjectHandle::setPrimaryDouble( const bool PrimaryFlag ) {

   m_primaryDouble = PrimaryFlag;
}

bool ProjectHandle::isPrimaryProperty( const string propertyName ) const {
   return m_primaryList.count( propertyName );
}

//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>
#include <petsc.h>

#include "Interface/SimulationDetails.h"
#include "h5_parallel_file_types.h"
#include "Interface/OutputProperty.h"
#include "Interface/RunParameters.h"
#include "Interface/ProjectData.h"
#include "Interface/MantleFormation.h"
#include "GeoPhysicsFormation.h"
#include "PropertiesCalculator.h"
#include "FilePath.h"
#include "Utilities.h"
#include "ImportFromXML.h"
#include "ExportToHDF.h"

// utilities library
#include "LogHandler.h"

static bool splitString( char * string, char separator, char * & firstPart, char * & secondPart );
static bool parseStrings( StringVector & strings, char * stringsString );
static bool parseAges( DoubleVector & ages, char * agesString );

static bool snapshotSorter( const Interface::Snapshot * snapshot1, const Interface::Snapshot * snapshot2 );
static bool snapshotIsEqual( const Interface::Snapshot * snapshot1, const Interface::Snapshot * snapshot2 );

void displayTime( const double timeToDisplay, const char * msgToDisplay );
void displayProgress( const string & fileName, double startTime, const string & message );


//------------------------------------------------------------//

PropertiesCalculator::PropertiesCalculator( int aRank ) {

   m_rank = aRank;

   m_debug            = false;
   m_copy             = false;
   m_basement         = false;
   m_all2Dproperties  = false;
   m_all3Dproperties  = false;
   m_listProperties   = false;
   m_listSnapshots    = false;
   m_listStratigraphy = false;
   m_convert          = false;
   m_vizFormat        = false;
   m_vizFormatHDF     = false;
   m_vizFormatHDFonly = false;
   m_primaryPod       = false;
   m_extract2D        = false;
   m_no3Dproperties   = false;
   m_projectProperties = false;

   m_snapshotsType = MAJOR;

   m_projectFileName = "";
   m_simulationMode  = "";
   m_activityName    = "";
   m_decompactionMode = false;

   m_projectHandle   = 0;
   m_propertyManager = 0;

}

//------------------------------------------------------------//

PropertiesCalculator::~PropertiesCalculator() {

   if(  m_propertyManager != 0 ) delete m_propertyManager;
   m_propertyManager = 0;
}

//------------------------------------------------------------//

GeoPhysics::ProjectHandle* PropertiesCalculator::getProjectHandle() const {

   return m_projectHandle;
}
//------------------------------------------------------------//

DerivedPropertyManager * PropertiesCalculator::getPropertyManager() const {

   return m_propertyManager;
}
//------------------------------------------------------------//

bool  PropertiesCalculator::finalise ( bool isComplete ) {

   PetscLogDouble Start_Time;
   PetscTime( &Start_Time );

  // set false as there is no need to save any properties - all are saved
   m_projectHandle->finishActivity ( false );

   bool status = true;
   if( isComplete ) {
      if( ! copyFiles ()) {
         PetscPrintf ( PETSC_COMM_WORLD, "  MeSsAgE ERROR Unable to copy output files\n");

         status = false;
      }
   }

   if( isComplete and status and m_rank == 0 ) {
      displayProgress( "Project file", Start_Time, "Start saving " );

      m_projectHandle->setSimulationDetails ( "fastproperties", "Default", "" );

      char outputFileName[ PETSC_MAX_PATH_LEN ];
      outputFileName[0] = '\0';

      PetscBool isDefined = PETSC_FALSE;
      PetscOptionsGetString (PETSC_NULL, "-save", outputFileName, 128, &isDefined);
      if(isDefined) {
         m_projectHandle->saveToFile( outputFileName );
      } else {
         m_projectHandle->saveToFile(m_projectFileName);
      }
      displayProgress( "", Start_Time, "Saving is finished for ProjectFile " );
   }

   delete m_propertyManager;
   m_propertyManager = 0;

   return status;
}

//------------------------------------------------------------//

bool PropertiesCalculator::CreateFrom ( DataAccess::Interface::ObjectFactory* factory ){

    if ( m_projectHandle == 0 ) {
      m_projectHandle = ( GeoPhysics::ProjectHandle* )( OpenCauldronProject( m_projectFileName, "r", factory ) );

      if(  m_projectHandle != 0 ) {
         m_propertyManager = new DerivedPropertyManager ( m_projectHandle, false, m_debug );
      }
   }
   if(  m_projectHandle == 0 ||  m_propertyManager == 0 ) {
      return false;
   }

   if( m_primaryPod ) {
      H5_Parallel_PropertyList::setOneFilePerProcessOption( false );
   }

   return true;

}
//------------------------------------------------------------//

bool PropertiesCalculator::startActivity() {

   if( !setFastcauldronActivityName() ) {
      return false;
   }

   const Interface::Grid * grid = m_projectHandle->getLowResolutionOutputGrid();

   bool started = m_projectHandle->startActivity ( m_activityName, grid, false, true, m_activityName != "Fastproperties" );

   // If this table is not present the assume that the last
   // fastcauldron mode was not pressure mode.
   // This table may not be present because we are running c2e on an old
   // project, before this table was added.
   bool coupledCalculation = false;

   coupledCalculation = m_simulationMode == "Overpressure" or
      m_simulationMode == "LooselyCoupledTemperature" or
      m_simulationMode == "CoupledHighResDecompaction" or
      m_simulationMode == "CoupledPressureAndTemperature" or
      m_simulationMode == "CoupledDarcy";

   if( started ) {
      m_projectHandle->initialise ( coupledCalculation );
   }

   if( started ) {
      started = m_projectHandle->setFormationLithologies ( true, true );
   }
   if( started) {
      started = m_projectHandle->initialiseLayerThicknessHistory ( coupledCalculation );
      if( started ) {
         m_projectHandle->applyFctCorrections();
      }
   }

    return started;
}

//------------------------------------------------------------//
bool PropertiesCalculator::showLists() {

   return m_listProperties || m_listSnapshots || m_listStratigraphy;

}
//------------------------------------------------------------//
void PropertiesCalculator::convertToVisualizationIO( )  {

   if( m_convert ) {
 
      PetscLogDouble Start_Time;
      PetscLogDouble End_Time;
      PetscTime( &Start_Time );

      if( m_rank == 0  ) {
         cout << "Converting to visualization format.." << endl;
      }
      clock_t start = clock();
      float timeInSeconds;
      
      std::shared_ptr<DataAccess::Interface::ObjectFactory> factory(new DataAccess::Interface::ObjectFactory());
      std::shared_ptr<DataAccess::Interface::ProjectHandle> projectHandle(DataAccess::Interface::OpenCauldronProject(m_projectFileName, "r", factory.get()));
      //   projectHandle->setActivityOutputGrid(projectHandle->getLowResolutionOutputGrid ());
    
      timeInSeconds = (float)(clock() - start) / CLOCKS_PER_SEC;
      cout << "Finished opening project handle in " << timeInSeconds << " seconds " << endl;
    
      m_vizProject = ImportProjectHandle::createFromProjectHandle(projectHandle, true );
      timeInSeconds = (float)(clock() - start) / CLOCKS_PER_SEC;
      cout << "Finished import in " << timeInSeconds << " seconds " << endl;
      
#if 0
      ibs::FilePath absPath(projectHandle->getFullOutputDir());
      absPath << m_projectFileName;

      cout << "Writing to new format" << endl;
      start = clock();
      std::shared_ptr<CauldronIO::Project> projectExisting;
      CauldronIO::ExportToXML::exportToXML(project, projectExisting, absPath.path(), 1);
      
      timeInSeconds = (float)(clock() - start) / CLOCKS_PER_SEC;
      cout << "Wrote to new format in " << timeInSeconds << " seconds" << endl;
#endif      
      PetscTime( &End_Time );
      displayTime( End_Time - Start_Time, "Total time: ");
   }
}
//------------------------------------------------------------//
void PropertiesCalculator::createXML() {

   if( m_vizFormat ) {

      boost::filesystem::path pathToxml(m_projectHandle->getProjectPath());

      pathToxml /= m_projectHandle->getProjectName();

      m_fileNameXml = pathToxml.string() + xmlExt;
      const string fileNameExisting = pathToxml.string() + ".xml";
      ibs::FilePath vizFileName( fileNameExisting );
      if( vizFileName.exists() ) {
         m_vizProject = CauldronIO::ImportFromXML::importFromXML(fileNameExisting, false);
      } else {
         // create new xml
         m_vizProject = createStructureFromProjectHandle(false);
      }
   }
}
//------------------------------------------------------------//
void PropertiesCalculator::calculateProperties( FormationSurfaceVector& formationItems, Interface::PropertyList properties, Interface::SnapshotList & snapshots )  {

   if( properties.size () == 0 ) {
      return;
   }
    m_sharedProjectHandle.reset(m_projectHandle);

    if(  m_vizFormat ) {
       createXML();
    }

   Interface::SnapshotList::reverse_iterator snapshotIter;

   Interface::PropertyList::iterator propertyIter;
   FormationSurfaceVector::iterator formationIter;

   SnapshotFormationSurfaceOutputPropertyValueMap allOutputPropertyValues;

   bool zeroSnapshotAdded = false;
   if( snapshots.empty() ) {
      const Snapshot * zeroSnapshot = m_projectHandle->findSnapshot( 0 );

      snapshots.push_back( zeroSnapshot );
      zeroSnapshotAdded = true;
   }

   struct stat fileStatus;
   int fileError;

   for ( snapshotIter = snapshots.rbegin(); snapshotIter != snapshots.rend(); ++snapshotIter )
   {
      const Interface::Snapshot * snapshot = *snapshotIter;

      displayProgress( snapshot->getFileName (), m_startTime, "Start computing " );

      if ( snapshot->getFileName () != "" ) {
         ibs::FilePath fileName( m_projectHandle->getFullOutputDir () );
         fileName << snapshot->getFileName ();
         fileError = stat ( fileName.cpath(), &fileStatus );

         ((Snapshot *)snapshot)->setAppendFile ( not fileError );
      }
      if(  m_vizFormat ) {
         m_formInfoList.reset();
         m_formInfoList = getDepthFormations( m_projectHandle, snapshot );
      }
      for ( formationIter = formationItems.begin(); formationIter != formationItems.end(); ++formationIter )
      {
         const Interface::Formation * formation = ( *formationIter ).first;
         const Interface::Surface   * surface   = ( *formationIter ).second;
         const Interface::Snapshot  * bottomSurfaceSnapshot = ( formation->getBottomSurface() != 0 ? formation->getBottomSurface()->getSnapshot() : 0 );

         if( snapshot->getTime() != 0.0 and surface == 0 and bottomSurfaceSnapshot != 0 ) {
            const double depoAge = bottomSurfaceSnapshot->getTime();
            if ( snapshot->getTime() > depoAge or fabs( snapshot->getTime() - depoAge ) < snapshot->getTime() * 1e-9 ) {
               continue;
            }
         }

         for ( propertyIter = properties.begin(); propertyIter != properties.end(); ++propertyIter )
         {
            const Interface::Property * property = *propertyIter;
            if( m_no3Dproperties and surface == 0 and property->getPropertyAttribute() != DataModel::FORMATION_2D_PROPERTY ) {
               continue;
            }
            if( not m_extract2D and surface != 0 and property->getName() != "Reflectivity" ) {
               continue;
            }

            if ( not m_projectProperties or ( m_projectProperties and allowOutput( property->getCauldronName(), formation, surface ))) {
               OutputPropertyValuePtr outputProperty = DerivedProperties::allocateOutputProperty ( * m_propertyManager, property, snapshot, * formationIter );

               if ( outputProperty != 0 ) {
                  if( m_debug && m_rank == 0 ) {
                     LogHandler( LogHandler::INFO_SEVERITY) << "Snapshot: " << snapshot->getTime() <<
                        " allocate " << property->getName() << " " << ( formation != 0 ? formation->getName() : "" ) << " " <<
                        ( surface != 0 ? surface->getName() : "" );
                  }
                  allOutputPropertyValues [ snapshot ][ * formationIter ][ property ] = outputProperty;
              }
               else{
                  if( m_debug && m_rank == 0 ) {
                     LogHandler( LogHandler::INFO_SEVERITY ) << "Could not calculate derived property " << property->getName()
                                                             << " @ snapshot " << snapshot->getTime() << "Ma for formation " <<
                        ( formation != 0 ? formation->getName() : "" ) << " " <<  ( surface != 0 ? surface->getName() : "" ) << ".";
                  }
               }
            }
         }
         if( not m_vizFormat ) { 
            DerivedProperties::outputSnapshotFormationData( m_projectHandle, snapshot, * formationIter, properties, allOutputPropertyValues );
         } else {
            DerivedProperties::createVizSnapshotFormationData( m_vizProject, m_projectHandle, snapshot, * formationIter, properties, allOutputPropertyValues, m_formInfoList, m_data );
         }
      }

      removeProperties( snapshot, allOutputPropertyValues );
      m_propertyManager->removeProperties( snapshot );

      if( not m_vizFormat ) {
         displayProgress( snapshot->getFileName (), m_startTime, "Start saving " );
      
         m_projectHandle->continueActivity();
      
         displayProgress( snapshot->getFileName (), m_startTime, "Saving is finished for " );
      } else {
         collectVolumeData(  DerivedProperties::getSnapShot( m_vizProject, snapshot->getTime() ), m_data );
         if( m_rank != 0 ) {
            DerivedProperties::getSnapShot( m_vizProject, snapshot->getTime() )->release();
         }
      }
      //     m_projectHandle->deleteRecordLessMapPropertyValues();
      //     m_projectHandle->deleteRecordLessVolumePropertyValues();

      m_projectHandle->deletePropertiesValuesMaps ( snapshot );
   }

   if( m_vizFormat and m_rank == 0) {
      updateVizSnapshotsConstantValue();

      displayProgress( m_fileNameXml, m_startTime, "Writing to visualization format " );
      std::shared_ptr<CauldronIO::Project> projectExisting;
      const string projectFileName = m_projectHandle->getFileName();
      ibs::FilePath absPath(projectFileName);

      CauldronIO::ExportToXML::exportToXML( m_vizProject, projectExisting, m_fileNameXml, 1, false, true );
      displayProgress( "", m_startTime, "Writing to visualization format done " );
  }

   if( m_vizFormatHDF and m_rank == 0 ) {
       writeToHDF( );
   }

   PetscLogDouble End_Time;
   PetscTime( &End_Time );
   
   displayTime( End_Time - m_startTime, "Total derived properties saving: ");
}

//------------------------------------------------------------//
void PropertiesCalculator::writeToHDF() {
    
   boost::filesystem::path pathToxml(m_projectHandle->getProjectPath());
   pathToxml /= m_projectHandle->getProjectName();
   string fileNameXml = pathToxml.string() + xmlExt;
   std::shared_ptr< CauldronIO::Project> vizProject(CauldronIO::ImportFromXML::importFromXML(fileNameXml));
  
   cout << "Writing to HDF from visualization format " << fileNameXml << endl;

   CauldronIO::ExportToHDF::exportToHDF(vizProject, fileNameXml, 1, m_basement, m_projectHandle );
}
//------------------------------------------------------------//

PropertyOutputOption PropertiesCalculator::checkTimeFilter3D ( const string & name ) const {

   if ( name == "AllochthonousLithology" or  name == "Lithology" or name == "BrineDensity" or name == "BrineViscosity" ) {
      return Interface::SEDIMENTS_ONLY_OUTPUT;
   }
   if ( name == "FracturePressure" ) {
      if ( m_projectHandle->getRunParameters ()->getFractureType () == "None" ) {
         return Interface::NO_OUTPUT;
      }
   }
   if ( name == "FaultElements" ) {
      if ( m_projectHandle->getBasinHasActiveFaults ()) {
         return Interface::SEDIMENTS_ONLY_OUTPUT;
      } else {
         return Interface::NO_OUTPUT;
      }
   }
   if ( name == "HorizontalPermeability" ) {

      const Interface::OutputProperty* permeability = m_projectHandle->findTimeOutputProperty ( "PermeabilityVec" );
      const Interface::PropertyOutputOption permeabilityOption = ( permeability == 0 ? Interface::NO_OUTPUT : permeability->getOption ());
      const Interface::OutputProperty* hpermeability = m_projectHandle->findTimeOutputProperty ( "HorizontalPermeability" );
      const Interface::PropertyOutputOption hpermeabilityOption = ( hpermeability == 0 ? Interface::NO_OUTPUT : hpermeability->getOption ());

      if( hpermeabilityOption  == Interface::NO_OUTPUT and permeability != 0 ) {
         return permeabilityOption;
      }
      if( permeability == 0 ) {
         return hpermeabilityOption;
      }
   }

   const Interface::OutputProperty * property = m_projectHandle->findTimeOutputProperty( name );

   if( property != 0 ) {
      if( m_simulationMode == "HydrostaticDecompaction" and name == "LithoStaticPressure" and
          property->getOption () == Interface::SEDIMENTS_AND_BASEMENT_OUTPUT ) {
         return Interface::SEDIMENTS_ONLY_OUTPUT;
      }
     if( name == "HydroStaticPressure" and
         property->getOption () == Interface::SEDIMENTS_AND_BASEMENT_OUTPUT ) {
        return Interface::SEDIMENTS_ONLY_OUTPUT;
     }

     return property->getOption ();
   }

   return Interface::NO_OUTPUT;
}

//------------------------------------------------------------//

bool PropertiesCalculator::allowOutput ( const string & propertyName3D,
                                         const Interface::Formation * formation, const Interface::Surface * surface ) const {


   string propertyName = propertyName3D;

   if ( propertyName.find( "HeatFlow" ) != string::npos ) {
      propertyName = "HeatFlow";
   } else if ( propertyName.find( "FluidVelocity" ) != string::npos ) {
      propertyName = "FluidVelocity";
   } else {
      size_t len = 4;
      size_t pos = propertyName.find( "Vec2" );

      if ( pos != string::npos ) {
         // Replace Vec2 with Vec.
         propertyName.replace( pos, len, "Vec" );
      }

   }

   if(( propertyName == "BrineDensity" or  propertyName == "BrineViscosity" ) and surface != 0 ) {
      return false;
   }
   if( m_decompactionMode and ( propertyName == "BulkDensity" ) and surface == 0 ) {
      return false;
   }

   bool basementFormation = formation->kind () == DataAccess::Interface::BASEMENT_FORMATION;

   // The top of the crust is a part of the sediment
   if( basementFormation and surface != 0 and ( propertyName == "Depth" or propertyName == "Temperature" ) ) {
      if( dynamic_cast<const GeoPhysics::Formation*>( formation )->isCrust() ) {
         if( formation->getTopSurface() and ( formation->getTopSurface() == surface )) {
            return true;
         }
      }
   }
   PropertyOutputOption outputOption = checkTimeFilter3D ( propertyName );

   if( outputOption == Interface::NO_OUTPUT ) {
      return false;
   }
   if( basementFormation ) {
      if( outputOption < Interface::SEDIMENTS_AND_BASEMENT_OUTPUT ) {
         return false;
      }
   }

   return true;

}

//------------------------------------------------------------//

bool PropertiesCalculator::acquireSnapshots( SnapshotList & snapshots )
{
   if ( m_ages.size() == 0 )
   {
      SnapshotList * allSnapshots = m_projectHandle->getSnapshots( m_snapshotsType );
      snapshots = *allSnapshots;
      delete allSnapshots;
      return true;
   }
   else
   {
      int index;
      double firstAge = -1;
      double secondAge = -1;
      for ( index = 0; index < m_ages.size(); ++index )
      {
         if ( m_ages[ index ] >= 0 )
         {
            if ( firstAge < 0 )
               firstAge = m_ages[ index ];
            else
               secondAge = m_ages[ index ];
         }
         else
         {
            if ( secondAge < 0 )
            {
               if ( firstAge >= 0 )
               {
                  const Snapshot * snapshot = m_projectHandle->findSnapshot( firstAge, m_snapshotsType );
                  if ( snapshot ) snapshots.push_back( snapshot );
                  if ( m_debug && m_rank == 0 && snapshot ) LogHandler(LogHandler::INFO_SEVERITY) << "adding single snapshot " << snapshot->getTime();
               }
            }
            else
            {
               if ( firstAge >= 0 )
               {
                  if ( firstAge > secondAge )
                  {
                     Swap( firstAge, secondAge );
                  }

                  SnapshotList * allSnapshots = m_projectHandle->getSnapshots( m_snapshotsType );
                  SnapshotList::iterator snapshotIter;
                  for ( snapshotIter = allSnapshots->begin(); snapshotIter != allSnapshots->end(); ++snapshotIter )
                  {
                     const Snapshot * snapshot = *snapshotIter;
                     if ( snapshot->getTime() >= firstAge && snapshot->getTime() <= secondAge )
                     {
                        if ( snapshot ) snapshots.push_back( snapshot );
                        if ( m_debug && snapshot && m_rank == 0 ) LogHandler( LogHandler::INFO_SEVERITY ) << "adding range snapshot " << snapshot->getTime();
                     }
                  }
                  delete allSnapshots;
               }
            }
            firstAge = secondAge = -1;
         }
      }
   }
   sort( snapshots.begin(), snapshots.end(), snapshotSorter );

   if ( m_debug && m_rank == 0 )
   {
      LogHandler( LogHandler::INFO_SEVERITY ) << "Snapshots ordered";
      SnapshotList::iterator snapshotIter;
      for ( snapshotIter = snapshots.begin(); snapshotIter != snapshots.end(); ++snapshotIter )
      {
         LogHandler( LogHandler::INFO_SEVERITY ) << (*snapshotIter)->getTime();
      }
   }

   SnapshotList::iterator firstObsolete = unique( snapshots.begin(), snapshots.end(), snapshotIsEqual );
   snapshots.erase( firstObsolete, snapshots.end() );

   if ( m_debug && m_rank == 0 )
   {
      LogHandler( LogHandler::INFO_SEVERITY ) << "Snapshots uniquefied";
      SnapshotList::iterator snapshotIter;
      for ( snapshotIter = snapshots.begin(); snapshotIter != snapshots.end(); ++snapshotIter )
      {
         LogHandler( LogHandler::INFO_SEVERITY ) << (*snapshotIter)->getTime();
      }
   }

   return true;
}

//------------------------------------------------------------//
void PropertiesCalculator::acquireProperties( Interface::PropertyList & properties ) {

   if( m_propertyNames.size() != 0 ) {
      // remove duplicated names
      std::sort( m_propertyNames.begin(), m_propertyNames.end() );
      m_propertyNames.erase( std::unique( m_propertyNames.begin(), m_propertyNames.end(), DerivedProperties::isEqualPropertyName ), m_propertyNames.end() );

      DerivedProperties::acquireProperties( m_projectHandle, * m_propertyManager, properties, m_propertyNames );
   }
}
//------------------------------------------------------------//
void PropertiesCalculator::acquireFormationsSurfaces( FormationSurfaceVector& formationSurfaceItems ) {

   bool load3d = false;
   if( m_formationNames.size() != 0 or m_all3Dproperties  ) {
      DerivedProperties::acquireFormations( m_projectHandle, formationSurfaceItems, m_formationNames, m_basement );
      load3d = true;
   }
   if(  m_all2Dproperties  ) {
      if( not load3d ) {
         DerivedProperties::acquireFormations( m_projectHandle, formationSurfaceItems, m_formationNames, m_basement);
      }
      DerivedProperties::acquireFormationSurfaces( m_projectHandle, formationSurfaceItems, m_formationNames, true, m_basement );
      DerivedProperties::acquireFormationSurfaces( m_projectHandle, formationSurfaceItems, m_formationNames, false, m_basement );

      if( not m_all3Dproperties ) {
         m_no3Dproperties = true;
      }
   }

   // if the property is selected but formationSurface list is empty add all formations
   if ( formationSurfaceItems.empty() and m_propertyNames.size() != 0 )
   {
      DerivedProperties::acquireFormations( m_projectHandle, formationSurfaceItems, m_formationNames, m_basement );
      DerivedProperties::acquireFormationSurfaces( m_projectHandle, formationSurfaceItems, m_formationNames, true, m_basement );
      DerivedProperties::acquireFormationSurfaces( m_projectHandle, formationSurfaceItems, m_formationNames, false, m_basement );
   }

}
//------------------------------------------------------------//
void PropertiesCalculator::acquireAll2Dproperties() {

   if ( m_all2Dproperties or ( not m_formationNames.empty() and m_propertyNames.empty () ))
   {
      Interface::PropertyList * allProperties = m_projectHandle->getProperties( true );

      LogHandler( LogHandler::DEBUG_SEVERITY ) << "Acquiring computable 2D properties";
      for ( size_t i = 0; i < allProperties->size (); ++i ) {
         const Interface::Property* property = (*allProperties)[ i ];

         bool addIt = false;

         if ( property->getPropertyAttribute () == DataModel::FORMATION_2D_PROPERTY and
              m_propertyManager->formationMapPropertyIsComputable ( property )) {
            m_propertyNames.push_back( property->getName() );
            LogHandler( LogHandler::DEBUG_SEVERITY ) << "   #" << property->getName() << " (2D formation)";
         }
         if (( property->getPropertyAttribute () == DataModel::CONTINUOUS_3D_PROPERTY or
               property->getPropertyAttribute () == DataModel::SURFACE_2D_PROPERTY ) and
              m_propertyManager->surfacePropertyIsComputable ( property )) {
            m_propertyNames.push_back( property->getName() );
            LogHandler( LogHandler::DEBUG_SEVERITY ) << "   #" << property->getName() << " (2D surface)";
         }
         if ( property->getPropertyAttribute () == DataModel::DISCONTINUOUS_3D_PROPERTY and
              m_propertyManager->formationSurfacePropertyIsComputable ( property )) {
             m_propertyNames.push_back( property->getName() );
             LogHandler( LogHandler::DEBUG_SEVERITY ) << "   #" << property->getName() << " (2D formation-surface)";
         }
      }

      delete allProperties;
   }
}
//------------------------------------------------------------//

void PropertiesCalculator::acquireAll3Dproperties() {

   if ( m_all3Dproperties or ( not m_formationNames.empty() and m_propertyNames.empty () )) {

      Interface::PropertyList * allProperties = m_projectHandle->getProperties( true );
      LogHandler( LogHandler::DEBUG_SEVERITY ) << "Acquiring computable 3D property ";
      for ( size_t i = 0; i < allProperties->size (); ++i ) {
         const Interface::Property* property = (*allProperties)[ i ];

         if (( property->getPropertyAttribute () == DataModel::CONTINUOUS_3D_PROPERTY or
               property->getPropertyAttribute () == DataModel::DISCONTINUOUS_3D_PROPERTY ) and
             m_propertyManager->formationPropertyIsComputable ( property ))
         {
            m_propertyNames.push_back( property->getName() );
            LogHandler( LogHandler::DEBUG_SEVERITY ) << "   #" << property->getName();
         }

      }
      delete allProperties;
   }
}
//------------------------------------------------------------//
void PropertiesCalculator::printListSnapshots ()  {

   if ( m_listSnapshots && m_rank == 0 ) {

      cout << endl;
      SnapshotList * mySnapshots = m_projectHandle->getSnapshots( m_snapshotsType );
      SnapshotList::iterator snapshotIter;

      cout.precision ( 8 );
      cout << "Available snapshots are: ";

      for ( snapshotIter = mySnapshots->begin(); snapshotIter != mySnapshots->end(); ++snapshotIter )
      {
         if ( snapshotIter != mySnapshots->begin() ) cout << ",";
         int oldPrecision = static_cast<int>(cout.precision());
         cout << setprecision(9);
         cout << ( *snapshotIter )->getTime();
         cout <<setprecision(oldPrecision);
      }
      cout << endl;

      delete mySnapshots;
   }
}
//------------------------------------------------------------//
void PropertiesCalculator::printListStratigraphy () {

  if ( m_listStratigraphy && m_rank == 0 )
   {
      cout << endl;
      const Snapshot * zeroSnapshot = m_projectHandle->findSnapshot( 0 );

      Interface::FormationList * myFormations = m_projectHandle->getFormations( zeroSnapshot, true );
      Interface::FormationList::iterator formationIter;
      cout << "Stratigraphy: ";
      cout << endl;
      cout << endl;
      bool firstFormation = true;
      bool arrivedAtBasement = false;
      for ( formationIter = myFormations->begin(); formationIter != myFormations->end(); ++formationIter )
      {
         const Interface::Formation * formation = *formationIter;
         if ( formation->kind() == BASEMENT_FORMATION && arrivedAtBasement == false )
         {
            arrivedAtBasement = true;
            cout << "Next formations and surfaces belong to the basement and only produce output when used with '-basement'" << endl;
         }
         if ( firstFormation )
         {
            cout << "\t" << formation->getTopSurface()->getName() << " (" << formation->getTopSurface()->getSnapshot()->getTime() << " Ma)" << endl;
            firstFormation = false;
         }

         cout << "\t\t" << formation->getName() << endl;
         const Interface::Surface * bottomSurface = formation->getBottomSurface();
         if ( bottomSurface )
         {
            cout << "\t" << bottomSurface->getName();
            if ( bottomSurface->getSnapshot() )
            {
               cout << " (" << bottomSurface->getSnapshot()->getTime() << " Ma)";
            }
            cout << endl;
         }
      }
      cout << endl;

      delete myFormations;
   }


}

//------------------------------------------------------------//

void PropertiesCalculator::printOutputableProperties () {


   if( m_listProperties ) {
      Interface::PropertyList * allProperties = m_projectHandle->getProperties ( true );

      PetscPrintf( PETSC_COMM_WORLD, "Available 3D output properties are: " );

      for ( size_t i = 0; i < allProperties->size (); ++i ) {
         const Interface::Property* property = (*allProperties)[ i ];
         LogHandler( LogHandler::DEBUG_SEVERITY ) << "########################################################";
         LogHandler( LogHandler::DEBUG_SEVERITY ) << "3D-->" << property->getName() << " computable?";

         if (( property->getPropertyAttribute () == DataModel::CONTINUOUS_3D_PROPERTY or
               property->getPropertyAttribute () == DataModel::DISCONTINUOUS_3D_PROPERTY ) and
               m_propertyManager->formationPropertyIsComputable ( property )) {
            PetscPrintf( PETSC_COMM_WORLD, "%s ",  property->getName ().c_str() );
            LogHandler( LogHandler::DEBUG_SEVERITY ) << "YES";
         }
         else{
            LogHandler( LogHandler::DEBUG_SEVERITY ) << "NO";
         }

      }

      PetscPrintf( PETSC_COMM_WORLD, "\n\n" );
      PetscPrintf( PETSC_COMM_WORLD, "Available 2D output properties are: ");

      for ( size_t i = 0; i < allProperties->size (); ++i ) {
         const Interface::Property* property = (*allProperties)[ i ];
         LogHandler( LogHandler::DEBUG_SEVERITY ) << "########################################################";
         LogHandler( LogHandler::DEBUG_SEVERITY ) << "2D-->" << property->getName();

         if ( property->getPropertyAttribute () == DataModel::FORMATION_2D_PROPERTY and
              m_propertyManager->formationMapPropertyIsComputable ( property )) {
            PetscPrintf( PETSC_COMM_WORLD, "%s ", property->getName ().c_str() );
            LogHandler( LogHandler::DEBUG_SEVERITY ) << "YES";

         } else if (( property->getPropertyAttribute () == DataModel::CONTINUOUS_3D_PROPERTY or
               property->getPropertyAttribute () == DataModel::SURFACE_2D_PROPERTY ) and
              m_propertyManager->surfacePropertyIsComputable ( property )) {
            PetscPrintf( PETSC_COMM_WORLD, "%s ", property->getName ().c_str() );
            LogHandler( LogHandler::DEBUG_SEVERITY ) << "YES";

         } else if ( property->getPropertyAttribute () == DataModel::DISCONTINUOUS_3D_PROPERTY and
              m_propertyManager->formationSurfacePropertyIsComputable ( property )) {

            PetscPrintf( PETSC_COMM_WORLD, "%s ", property->getName ().c_str() );
            LogHandler( LogHandler::DEBUG_SEVERITY ) << "YES";
         }
         else{
            LogHandler( LogHandler::DEBUG_SEVERITY ) << "NO";
         }

      }

      PetscPrintf( PETSC_COMM_WORLD, "\n\n" );

      delete allProperties;
   }
}

//------------------------------------------------------------//
bool PropertiesCalculator::setFastcauldronActivityName() {

   if( m_projectHandle->getDetailsOfLastSimulation ( "fastcauldron" ) == 0 ||
       m_projectHandle->getDetailsOfLastSimulation ( "fastcauldron" )->getSimulatorMode () == "NoCalculaction" ) {
      return false;
   }

   m_simulationMode = m_projectHandle->getDetailsOfLastSimulation ( "fastcauldron" )->getSimulatorMode ();

   if( m_simulationMode == "HydrostaticDecompaction" ||
       m_simulationMode == "HydrostaticTemperature" ||
       m_simulationMode == "Overpressure" ) {
      m_activityName = m_simulationMode;
   } else if( m_simulationMode == "CoupledPressureAndTemperature" ) {
      m_activityName = "PressureAndTemperature";
   } else if( m_simulationMode == "LooselyCoupledTemperature" ) {
      m_activityName = "OverpressuredTemperature";
   } else if( m_simulationMode == "CoupledHighResDecompaction" or m_simulationMode == "HydrostaticHighResDecompaction") {
      m_activityName = "HighResDecompaction";
   } else {
      m_activityName = "Fastproperties";
   }
   m_decompactionMode = ( m_activityName == "HydrostaticDecompaction" ) or ( m_activityName == "HighResDecompaction");

   return true;
}


// Methods for visualization format output
//------------------------------------------------------------//

void PropertiesCalculator::updateVizSnapshotsConstantValue() {

   SnapShotList snapShotList = m_vizProject->getSnapShots();
   for (auto& snapShot : snapShotList)
   {
      DerivedProperties::updateConstantValue( snapShot );
   }

}

//------------------------------------------------------------//
std::shared_ptr<CauldronIO::Project> PropertiesCalculator::createStructureFromProjectHandle(bool verbose) {


   // Get modeling mode
    Interface::ModellingMode modeIn = m_sharedProjectHandle->getModellingMode();
    CauldronIO::ModellingMode mode = modeIn == Interface::MODE1D ? CauldronIO::MODE1D : CauldronIO::MODE3D;
    // setActivityGrid
    if( modeIn ==CauldronIO:: MODE1D ) {
       m_sharedProjectHandle->setActivityOutputGrid( m_sharedProjectHandle->getLowResolutionOutputGrid ());
    }
    // Read general project data
    const Interface::ProjectData* projectData = m_sharedProjectHandle->getProjectData();

    // Create the project
    std::shared_ptr<CauldronIO::Project> project(new CauldronIO::Project(
                                                                         projectData->getProjectName(), projectData->getDescription(), projectData->getProjectTeam(),
                                                                         projectData->getProgramVersion(), mode, xml_version_major, xml_version_minor));
    // Import all snapshots
    ImportProjectHandle import(verbose, project, m_sharedProjectHandle);

    if (verbose)
       cout << "Create empty snapshots" << endl;

   
    std::shared_ptr<Interface::SnapshotList> snapShots;
    snapShots.reset(m_projectHandle->getSnapshots(Interface::MAJOR | Interface::MINOR));
    
    for (size_t i = 0; i < snapShots->size(); i++)
    {
       const Interface::Snapshot* snapShot = snapShots->at(i);
       
       // Create a new empty snapshot
       std::shared_ptr<CauldronIO::SnapShot> snapShotIO(new CauldronIO::SnapShot(snapShot->getTime(), DerivedProperties::getSnapShotKind(snapShot), snapShot->getType() == MINOR));
      
       // Add to project
       project->addSnapShot(snapShotIO);
    }   
    return project;
    

}
     
//------------------------------------------------------------//
bool PropertiesCalculator::copyFiles( ) {

   if ( not H5_Parallel_PropertyList::isPrimaryPodEnabled () or
        ( H5_Parallel_PropertyList::isPrimaryPodEnabled () and not m_copy )) {
      return true;
   }

   int rank;
   MPI_Comm_rank(PETSC_COMM_WORLD, &rank);

   if( rank != 0 ) return true;

   PetscBool noFileRemove = PETSC_FALSE;
   PetscOptionsHasName( PETSC_NULL, "-noremove", &noFileRemove );

   PetscLogDouble StartMergingTime;
   PetscTime(&StartMergingTime);
   bool status = true;

   const std::string& directoryName = m_projectHandle->getOutputDir ();

   PetscPrintf ( PETSC_COMM_WORLD, "Copy output files ...\n" );

   SnapshotList * snapshots = m_projectHandle->getSnapshots( MAJOR | MINOR );
   SnapshotList::iterator snapshotIter;

   for ( snapshotIter = snapshots->begin(); snapshotIter != snapshots->end(); ++snapshotIter ) {
      const Interface::Snapshot * snapshot = *snapshotIter;

      if ( snapshot->getFileName () == "" ) {
         continue;
      }
      ibs::FilePath filePathName( m_projectHandle->getProjectPath () );
      filePathName << directoryName << snapshot->getFileName ();

      displayProgress( snapshot->getFileName (), StartMergingTime, "Copy " );

      status = H5_Parallel_PropertyList::copyMergedFile( filePathName.path(), false );

      // delete the file in the shared scratch
      if( status and not noFileRemove ) {
         ibs::FilePath fileName(H5_Parallel_PropertyList::getTempDirName() );
         fileName << filePathName.cpath ();

         int status = std::remove( fileName.cpath() ); //c_str ());
         if (status == -1)
            cerr << fileName.cpath() << " MeSsAgE WARNING  Unable to remove snapshot file, because '"
                 << std::strerror(errno) << "'" << endl;
      }
   }

   string fileName = m_activityName + "_Results.HDF" ;
   ibs::FilePath filePathName( m_projectHandle->getProjectPath () );
   filePathName <<  directoryName << fileName;

   displayProgress( fileName, StartMergingTime, "Copy " );

   status = H5_Parallel_PropertyList::copyMergedFile( filePathName.path(), false );

   // remove the file from the shared scratch
   if( status and  not noFileRemove ) {

    ibs::FilePath fileName(H5_Parallel_PropertyList::getTempDirName() );
      fileName << filePathName.cpath ();
      int status = std::remove( fileName.cpath() );

      if (status == -1) {
         cerr << fileName.cpath () << " MeSsAgE WARNING  Unable to remove file, because '"
              << std::strerror(errno) << "'" << endl;
      }

     // remove the output directory from the shared scratch
      ibs::FilePath dirName(H5_Parallel_PropertyList::getTempDirName() );
      dirName << directoryName;

      displayProgress( dirName.path(), StartMergingTime, "Removing remote output directory " );
      status = std::remove( dirName.cpath() );

      if (status == -1)
         cerr << dirName.cpath () << " MeSsAgE WARNING  Unable to remove the directory, because '"
              << std::strerror(errno) << "'" << endl;
   }

   if( status ) {
      displayTime( StartMergingTime, "Total merging time: " );
   } else {
      PetscPrintf ( PETSC_COMM_WORLD, "  MeSsAgE ERROR Could not merge the file %s.\n", filePathName.cpath() );
   }

   delete snapshots;

   return status;
}
//------------------------------------------------------------//

bool PropertiesCalculator::parseCommandLine( int argc, char ** argv ) {

   int arg;
   for ( arg = 1; arg < argc; arg++ )
   {
      if ( strncmp( argv[ arg ], "-properties", Max( 5, (int)strlen( argv[ arg ] ) ) ) == 0 )
      {
         if ( arg + 1 >= argc || argv[ arg + 1 ][ 0 ] == '-' )
         {
            showUsage( argv[ 0 ], "Argument for '-properties' is missing" );

            return false;
         }
         if ( !parseStrings( m_propertyNames, argv[ ++arg ] ) )
         {
            showUsage( argv[ 0 ], "Illegal argument for '-properties'" );

            return false;
         }
      }
      else if ( strncmp( argv[ arg ], "-ages", Max( 2, (int)strlen( argv[ arg ] ) ) ) == 0 )
      {
         if ( arg + 1 >= argc || argv[ arg + 1 ][ 0 ] == '-' )
         {
            showUsage( argv[ 0 ], "Argument for '-ages' is missing" );

            return false;
         }
         if ( !parseAges( m_ages, argv[ ++arg ] ) )
         {
            showUsage( argv[ 0 ], "Illegal argument for '-ages'" );

            return false;
         }
      }
      else if ( strncmp( argv[ arg ], "-formations", Max( 5, (int)strlen( argv[ arg ] ) ) ) == 0 )
      {
         if ( arg + 1 >= argc || argv[ arg + 1 ][ 0 ] == '-' )
         {
            showUsage( argv[ 0 ], "Argument for '-formations' is missing" );

            return false;
         }
         if ( !parseStrings( m_formationNames, argv[ ++arg ] ) )
         {
            showUsage( argv[ 0 ], "Illegal argument for '-formations'" );

            return false;
         }
      }
      else if ( strncmp( argv[ arg ], "-basement", Max( 3, (int)strlen( argv[ arg ] ) ) ) == 0 )
      {
         m_basement = true;
      }
      else if ( strncmp( argv[ arg ], "-minor", Max( 3, (int)strlen( argv[ arg ] ) ) ) == 0 )
      {
         m_snapshotsType = MAJOR | MINOR;
      }
      else if ( strncmp( argv[ arg ], "-all-2D-properties", Max( 7, (int)strlen( argv[ arg ] ) ) ) == 0 )
      {
         m_all2Dproperties = true;
      }
      else if ( strncmp( argv[ arg ], "-all-3D-properties", Max( 7, (int)strlen( argv[ arg ] ) ) ) == 0 )
      {
         m_all3Dproperties = true;
      }
      else if ( strncmp( argv[ arg ], "-project-properties", Max( 12, (int)strlen( argv[ arg ] ) ) ) == 0 )
      {
         m_projectProperties = true;
      }
      else if ( strncmp( argv[ arg ], "-list-properties", Max( 7, (int)strlen( argv[ arg ] ) ) ) == 0 )
      {
         m_listProperties = true;
      }
      else if ( strncmp( argv[ arg ], "-list-snapshots", Max( 8, (int)strlen( argv[ arg ] ) ) ) == 0 )
      {
         m_listSnapshots = true;
      }
      else if ( strncmp( argv[ arg ], "-list-stratigraphy", Max( 8, (int)strlen( argv[ arg ] ) ) ) == 0 )
      {
         m_listStratigraphy = true;
      }
      else if ( strncmp( argv[ arg ], "-project", Max( 5, (int)strlen( argv[ arg ] ) ) ) == 0 )
      {
         if ( arg + 1 >= argc || argv[ arg + 1 ][ 0 ] == '-' )
         {
            showUsage( argv[ 0 ], "Argument for '-project' is missing" );
            return false;
         }
         m_projectFileName = argv[ ++arg ];
      }
      else if ( strncmp( argv[ arg ], "-copy", Max( 4, (int)strlen( argv[ arg ] ) ) ) == 0 )
      {
         m_copy = true;
      }
      else if ( strncmp( argv[ arg ], "-noremove", Max( 4, (int)strlen( argv[ arg ] ) ) ) == 0 )
      {
      }
      else if ( strncmp( argv[ arg ], "-debug", Max( 2, (int)strlen( argv[ arg ] ) ) ) == 0 )
      {
         m_debug = true;
      }
      else if ( strncmp( argv[ arg ], "-help", Max( 3, (int)strlen( argv[ arg ] ) ) ) == 0 )
      {
         showUsage( argv[ 0 ], " Standard usage." );

         return false;
      }
      else if ( strncmp( argv[ arg ], "-?", Max( 2, (int)strlen( argv[ arg ] ) ) ) == 0 )
      {
         showUsage( argv[ 0 ], " Standard usage." );

         return false;
      }
      else if ( strncmp( argv[ arg ], "-usage", Max( 2, (int)strlen( argv[ arg ] ) ) ) == 0 )
      {
         showUsage( argv[ 0 ], " Standard usage." );

         return false;
      }
      else if ( strncmp( argv[ arg ], "-", 1 ) != 0 )
      {
         m_projectFileName = argv[ arg ];
      }
      else if ( strncmp( argv[ arg ], "-convert", Max( 7, (int)strlen( argv[ arg ] )) ) == 0 )
      {
         m_convert = true;
      }
      else if ( strncmp( argv[ arg ], "-viz", Max( 7, (int)strlen( argv[ arg ] )) ) == 0 )
      {
         m_vizFormat = true;
      }
      else if ( strncmp( argv[ arg ], "-hdf", Max( 7, (int)strlen( argv[ arg ] )) ) == 0 )
      {
         m_vizFormatHDF = true;
      }
      else if ( strncmp( argv[ arg ], "-hdfonly", Max( 7, (int)strlen( argv[ arg ] )) ) == 0 )
      {
         m_vizFormatHDFonly = true;
      }
      else if ( strncmp( argv[ arg ], "-ddd", Max( 3, (int)strlen( argv[ arg ] )) ) == 0 )
      {

      }
      else if ( strncmp( argv[ arg ], "-primaryPod", Max( 8, (int)strlen( argv[ arg ] )) ) == 0 )
      {
         ++ arg;
         m_primaryPod = true;
      }
      else if ( strncmp( argv[ arg ], "-extract2D", Max( 6, (int)strlen( argv[ arg ] )) ) == 0 )
      {
         m_extract2D = true;
      }
      else if ( strncmp( argv[ arg ], "-save", Max( 4, (int)strlen( argv[ arg ] )) ) == 0 )
      {
         ++ arg;

      }
      else if ( strncmp( argv[ arg ], "-verbosity", Max( 4, (int)strlen( argv[ arg ] )) ) == 0 )
      {
         ++ arg;

      }
      else
      {
         LogHandler(LogHandler::ERROR_SEVERITY) << "Unknown or ambiguous option: " << argv[ arg ];
         showUsage( argv[ 0 ] );

         return false;
      }
   }


   if ( m_projectFileName == "" )
   {
      showUsage( argv[ 0 ], "No project file specified" );

      return false;
   }
   if( m_projectProperties ) {
      m_all3Dproperties = true;
      m_all2Dproperties = true;
      // Do not extract and save 2D maps from 3d data, define -extract2D option
      // m_extract2D = false;
      m_basement = true;
   }

   if( m_convert or m_vizFormatHDFonly ) {
      int numberOfRanks;
      
      MPI_Comm_size ( PETSC_COMM_WORLD, &numberOfRanks );
      if( numberOfRanks > 1 ) {
         PetscPrintf( PETSC_COMM_WORLD, "Unable to convert data to Visualization format. Please select 1 core.\n" );
         return false;
      }
   }
   return true;
}
//------------------------------------------------------------//
void PropertiesCalculator::startTimer() {
   PetscTime( &m_startTime );

}
//------------------------------------------------------------//

bool PropertiesCalculator::hdfonly() const {
   return m_vizFormatHDFonly;
}
//------------------------------------------------------------//

bool PropertiesCalculator::convert() const {
   return m_convert;
}
//------------------------------------------------------------//

bool PropertiesCalculator::vizFormat() const {
   return m_vizFormat;
}


//------------------------------------------------------------//

void PropertiesCalculator::showUsage( const char* command, const char* message )
{
   if( m_rank == 0 ) {
      cout << endl;

      if ( message != 0 )
      {
         cout << command << ": " << message << endl;
      }

      cout << "Usage (case sensitive!!): " << command << endl << endl
           << "\t[-properties name1,name2...]               properties to produce output for" << endl
           << "\t[-ages age1[-age2],...]                    select snapshot ages using single values and/or ranges" << endl << endl
           << "\t[-formations formation1,formation2...]     produce output for the given formations" << endl
           << "\t                                           the four options above can include Crust or Mantle" << endl << endl
           << "\t[-basement]                                produce output for the basement as well," << endl
           << "\t                                           only needed if none of the three options above have been specified" << endl << endl
           << "\t[-project] projectname                     name of 3D Cauldron project file to produce output for" << endl
           << "\t[-save filename]                           name of file to save output (*.csv format) table to, otherwise save to stdout" << endl
           << "\t[-verbosity level]                         verbosity level of the log file(s): minimal|normal|detailed|diagnostic. Default value is 'normal'." << endl
           << endl
           << "\t[-all-3D-properties]                       produce output for all 3D properties" << endl
           << "\t[-all-2D-properties]                       produce output for all 2D primary properties" << endl
           << "\t[-project-properties]                      produce output for the properties selected for output in the project file" << endl
           << "\t[-extract2D]                               produce output for all 2D properties (use with -all-2D-properties)" << endl
           << "\t[-list-properties]                         print a list of available properties and exit" << endl
           << "\t[-list-snapshots]                          print a list of available snapshots and exit" << endl
           << "\t[-list-stratigraphy]                       print a list of available surfaces and formations and exit" << endl << endl
           << "\t[-convert]                                 convert the data to visialization format. (run on 1 core)" << endl << endl
           << "\t[-viz]                                     calculate the properties and convert to visialization format." << endl << endl
           << "\t[-help]                                    print this message and exit" << endl << endl
           << "Options for shared cluster storage:" << endl << endl
           << "\t[-primaryPod <dir>]                        use if the fastcauldron data are stored in the shared <dir> on the cluster" << endl << endl
           << "\t[-copy]                                    use in combination with -primaryPod. Copy the results to the local dir and " << endl
           << "\t                                           remove them from the shared dir on the cluster" << endl << endl
           << "\t[-noremove]                                use in combination with -primaryPod and -copy. Don't remove results from " << endl
           << "\t                                           the shared dir on the cluster" << endl << endl;
      cout << "If names in an argument list contain spaces, put the list between double or single quotes, e.g:"
           << "\t-formations \"Dissolved Salt,Al Khalata\"" << endl;
      cout << "Bracketed options are optional and options may be abbreviated" << endl << endl;
   }
}

//------------------------------------------------------------//

/// destructive!!!
bool splitString( char * string, char separator, char * & firstPart, char * & secondPart )
{
   if ( !string || strlen( string ) == 0 ) return false;
   secondPart = strchr( string, separator );
   if ( secondPart != 0 )
   {
      *secondPart = '\0';
      ++secondPart;
      if ( *secondPart == '\0' ) secondPart = 0;
   }

   firstPart = string;
   if ( strlen( firstPart ) == 0 ) return false;
   return true;
}

bool parseStrings( StringVector & strings, char * stringsString )
{
   char * strPtr = stringsString;
   char * section;
   while ( splitString( strPtr, ',', section, strPtr ) )
   {
      strings.push_back( string( section ) );
   }
   return true;
}


bool parseAges( DoubleVector & ages, char * agesString )
{
   char * strPtr = agesString;
   char * commasection;
   char * section;
   while ( splitString( strPtr, ',', commasection, strPtr ) )
   {
      while ( splitString( commasection, '-', section, commasection ) )
      {
         double number = atof( section );
         ages.push_back( number );
      }
      ages.push_back( -1 ); // separator
   }
   ages.push_back( -1 ); // separator
   return true;
}

bool snapshotIsEqual( const Snapshot * snapshot1, const Snapshot * snapshot2 )
{
   return snapshot1->getTime() == snapshot2->getTime();
}

bool snapshotSorter( const Snapshot * snapshot1, const Snapshot * snapshot2 )
{
   return snapshot1->getTime() > snapshot2->getTime();
}

void displayProgress( const string & fileName, double startTime, const string & message ) {

   if (PetscGlobalRank != 0)
      return;
   double EndTime;

   PetscTime(&EndTime);

   double CalculationTime = EndTime - startTime;
   long remainder = (long) CalculationTime;

   long secs = remainder % 60;
   remainder -= secs;
   long mins = (remainder / 60) % 60;
   remainder -= mins * 60;
   long hrs = remainder / (60 * 60);

   char time[124];
   sprintf (time, "%2.2ld:%2.2ld:%2.2ld", hrs, mins, secs);

   ostringstream buf;
   buf.precision(4);
   buf.setf(ios::fixed);

   buf << message << fileName << " : " << setw( 8 ) << " Elapsed: " << time;;

   cout << buf.str() << endl;
   cout << flush;
}

void displayTime ( const double timeToDisplay, const char * msgToDisplay ) {

   int hours   = (int)(  timeToDisplay / 3600.0 );
   int minutes = (int)(( timeToDisplay - (hours * 3600.0) ) / 60.0 );
   int seconds = (int)(  timeToDisplay - hours * 3600.0 - minutes * 60.0 );

   PetscPrintf ( PETSC_COMM_WORLD, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ \n" );
   PetscPrintf ( PETSC_COMM_WORLD, "%s: %d hours %d minutes %d seconds\n", msgToDisplay, hours, minutes, seconds );
   PetscPrintf ( PETSC_COMM_WORLD, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ \n" );

}

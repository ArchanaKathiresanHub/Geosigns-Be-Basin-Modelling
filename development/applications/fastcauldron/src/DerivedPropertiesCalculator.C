#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <iostream>

#include "LogHandler.h"
#include "FastcauldronFactory.h"
#include "FilePath.h"
#include "Interface/SimulationDetails.h"
#include "Interface/MapWriter.h"

#include "DerivedPropertiesCalculator.h"
#include "timefilter.h"


static bool snapshotSorter( const Snapshot * snapshot1, const Snapshot * snapshot2 );

DerivedPropertiesCalculator::DerivedPropertiesCalculator( AppCtx * aAppctx, const PropListVec& propertyVolumeNames, const PropListVec& propertyMapNames ) {

    m_appctx = aAppctx;
    m_simulator = & FastcauldronSimulator::getInstance();
    m_decompactionMode  = false;
    m_opTemperatureMode = false;
    m_hydrostaticTemperatureMode = false;

    m_rank = m_simulator->getRank();
    m_debug = false;

    acquirePropertyNames( propertyVolumeNames );
    acquirePropertyNames( propertyMapNames );
 }

//------------------------------------------------------------//

DerivedPropertiesCalculator::~DerivedPropertiesCalculator() {

   if(  m_propertyManager != 0 ) {

      delete m_propertyManager;
      m_propertyManager = 0;
   }
}
//------------------------------------------------------------//

bool DerivedPropertiesCalculator::compute() {

   if( m_rank == 0 ) {
      cout << "Calculating derived properties..." << endl;
   }

   PetscBool debugOn = PETSC_FALSE;
   PetscOptionsHasName( PETSC_NULL, "-db", &debugOn );
   if( debugOn ) {
      m_debug = true;
   }

   if( m_debug and m_rank == 0 ) {
      printNames();
   }
   // 2D Map file can still be "read-only" opened - reopen it in append mode
   string fileName = m_simulator-> getActivityName();

   fileName += "_Results.HDF";
   ibs::FilePath filePathName ( m_simulator->getFullOutputDir () );
   filePathName << fileName;

   m_simulator->getMapPropertyValuesWriter( )->close();
   m_simulator->getMapPropertyValuesWriter( )->open( filePathName.cpath(), true );

   GeoPhysics::ProjectHandle * projectHandle = dynamic_cast < GeoPhysics::ProjectHandle* >(m_simulator);

   database::Record * tempRecord = m_simulator->addCurrentSimulationDetails();

   // add FracturePressure explicitly as it has the same name as trap property
   Interface::Property * fracturePressure = projectHandle->getFactory()->produceProperty( projectHandle, 0, "FracturePressure",   "FracturePressure",   "MPa",
                                                                                          FORMATIONPROPERTY, DataModel::DISCONTINUOUS_3D_PROPERTY );
   projectHandle->addPropertyToFront( fracturePressure );
   m_simulator->connectOutputProperty( fracturePressure );

   m_propertyManager = new DerivedPropertyManager ( m_simulator, m_debug );

   m_opTemperatureMode = ( m_simulator->getCalculationMode() == OVERPRESSURED_TEMPERATURE_MODE );
   m_hydrostaticTemperatureMode = ( m_simulator->getCalculationMode() ==  HYDROSTATIC_TEMPERATURE_MODE );

   m_simulator->removeRecordlessDerivedPropertyValues();

   projectHandle->sortSnapshots();

   if( m_debug and m_rank == 0 ) {
      projectHandle->printSnapshotTable();
   }
   m_decompactionMode = ( m_simulator->getCalculationMode () == COUPLED_HIGH_RES_DECOMPACTION_MODE or
                          m_simulator->getCalculationMode () == HYDROSTATIC_HIGH_RES_DECOMPACTION_MODE or
                          m_simulator->getCalculationMode () == HYDROSTATIC_DECOMPACTION_MODE );

   if ( m_opTemperatureMode ) {

       m_simulator->setOutputPropertyOption( HYDROSTATICPRESSURE, Interface::SEDIMENTS_AND_BASEMENT_OUTPUT );
       m_simulator->setOutputPropertyOption( LITHOSTATICPRESSURE, Interface::SEDIMENTS_ONLY_OUTPUT );
       m_simulator->setOutputPropertyOption( OVERPRESSURE, Interface::SEDIMENTS_ONLY_OUTPUT );
       m_simulator->setOutputPropertyOption( POROSITYVEC, Interface::SEDIMENTS_ONLY_OUTPUT );
       m_simulator->setOutputPropertyOption( BULKDENSITYVEC, Interface::SEDIMENTS_ONLY_OUTPUT );
       m_simulator->setOutputPropertyOption( PERMEABILITYVEC, Interface::SEDIMENTS_AND_BASEMENT_OUTPUT );
       m_simulator->setOutputPropertyOption( HORIZONTALPERMEABILITY, Interface::SEDIMENTS_AND_BASEMENT_OUTPUT );
       m_simulator->setOutputPropertyOption( THCONDVEC, Interface::SEDIMENTS_AND_BASEMENT_OUTPUT );
       m_simulator->setOutputPropertyOption( DIFFUSIVITYVEC, Interface::SEDIMENTS_AND_BASEMENT_OUTPUT );

       m_simulator->setOutputPropertyOption( DEPTH, Interface::SEDIMENTS_AND_BASEMENT_OUTPUT );
       m_simulator->setOutputPropertyOption( VES, Interface::SEDIMENTS_AND_BASEMENT_OUTPUT );
       m_simulator->setOutputPropertyOption( MAXVES, Interface::SEDIMENTS_AND_BASEMENT_OUTPUT );
       m_simulator->setOutputPropertyOption( PRESSURE, Interface::SEDIMENTS_AND_BASEMENT_OUTPUT );
       m_simulator->setOutputPropertyOption( OVERPRESSURE, Interface::SEDIMENTS_AND_BASEMENT_OUTPUT );
       m_simulator->setOutputPropertyOption( FAULTELEMENTS, Interface::SEDIMENTS_ONLY_OUTPUT );
   }

   SnapshotList snapshots = *( m_simulator->getSnapshots( Interface::MAJOR ));
   sort( snapshots.begin(), snapshots.end(), snapshotSorter );

   Interface::PropertyList propertiesItems;
   acquireProperties( m_simulator, *m_propertyManager, propertiesItems, m_propertyNames );

   FormationSurfaceVector formationsSurfaceItems;
   StringVector formationNames;
   acquireFormations( m_simulator, formationsSurfaceItems, formationNames );
   acquireFormationSurfaces( m_simulator, formationsSurfaceItems, formationNames, true );
   acquireFormationSurfaces( m_simulator, formationsSurfaceItems, formationNames, false );

   SnapshotFormationSurfaceOutputPropertyValueMap allOutputPropertyValues;
   allocateAllProperties( formationsSurfaceItems, propertiesItems, snapshots, allOutputPropertyValues );
   bool status = calculateProperties ( formationsSurfaceItems, snapshots, propertiesItems, allOutputPropertyValues );

   m_simulator->removeCurrentSimulationDetails( tempRecord );

   m_simulator->getMapPropertyValuesWriter( )->close();

   return status;
}
//------------------------------------------------------------//

bool DerivedPropertiesCalculator::calculateProperties ( FormationSurfaceVector& formationItems, SnapshotList & snapshots,
                                                        DataAccess::Interface::PropertyList & properties,
                                                        SnapshotFormationSurfaceOutputPropertyValueMap & allOutputPropertyValues ) {

   // clean mpaCache which can hold read-only opened files
   m_simulator->mapFileCacheDestructor();

   struct stat fileStatus;
   int fileError;

   SnapshotList::iterator snapshotIter;
   FormationSurfaceVector::iterator formationIter;

   PetscLogDouble Accumulated_Saving_Time = 0;
   PetscLogDouble Start_Saving_Time = 0;
   PetscLogDouble Start_Time;
   PetscLogDouble End_Time;

   PetscTime( & Start_Saving_Time );

   for ( snapshotIter = snapshots.begin(); snapshotIter != snapshots.end(); ++snapshotIter ) {

      PetscTime( &Start_Time );
      const Snapshot * snapshot = *snapshotIter;

      if( snapshot->getTime() > m_appctx->Age_Of_Basin () ) {
         continue;
      }
      if ( snapshot->getFileName () != "" ) {

         ibs::FilePath outputFileName ( m_simulator->getFullOutputDir () );
         outputFileName << snapshot->getFileName ();

         ostringstream buff;
         if( H5_Parallel_PropertyList::isOneFilePerProcessEnabled() ) {
            buff << H5_Parallel_PropertyList::getTempDirName() << outputFileName.cpath() << "_" << m_rank;
         } else {
            buff << outputFileName.cpath();
         }
         string fileName = buff.str();

         fileError = stat ( fileName.c_str(), &fileStatus );
         if( fileError < 0 ) {
            printf( "Error in %s: %s\n", fileName.c_str(), strerror( errno ));
         }

         ((Snapshot *)snapshot)->setAppendFile ( not fileError ); //append );//not fileError );
      }
      for ( formationIter = formationItems.begin();  formationIter != formationItems.end(); ++formationIter ) {
         const Interface::Formation * formation = ( *formationIter ).first;
         const Interface::Surface   * surface   = ( *formationIter ).second;

         const Interface::Snapshot * bottomSurfaceSnapshot = ( formation->getBottomSurface() != 0 ? formation->getBottomSurface()->getSnapshot() : 0 );

         if( snapshot->getTime() != 0.0 and surface == 0 and bottomSurfaceSnapshot != 0 ) {
            const double depoAge = bottomSurfaceSnapshot->getTime();
             if ( snapshot->getTime() > depoAge or fabs( snapshot->getTime() - depoAge ) < snapshot->getTime() * 1e-9 ) {
               continue;
            }
         }
         DerivedProperties::outputSnapshotFormationData( m_simulator, snapshot, * formationIter, properties, allOutputPropertyValues );
      }
      PetscTime( &Start_Time );

      Display_Merging_Progress( snapshot->getFileName (), Start_Saving_Time, "Saving " );

      m_simulator->continueActivity();

      PetscTime( &End_Time );
      Accumulated_Saving_Time += ( End_Time - Start_Time );
   }

   if( m_rank == 0 ) {
      displayTime( "Total derived properties saving: ",  Start_Saving_Time );
   }

   return true;
}

//------------------------------------------------------------//

void DerivedPropertiesCalculator::allocateAllProperties( const FormationSurfaceVector & formationSurfacePairs,
                                                         DataAccess::Interface::PropertyList properties,
                                                         const SnapshotList & snapshots,
                                                         SnapshotFormationSurfaceOutputPropertyValueMap & allOutputPropertyValues) {

   SnapshotList::const_iterator snapshotIter;
   Interface::PropertyList::const_iterator propertyIter;
   FormationSurfaceVector::const_iterator formationSurfaceIter;

   for ( snapshotIter = snapshots.begin(); snapshotIter != snapshots.end(); ++snapshotIter ) {
      const Interface::Snapshot * snapshot = *snapshotIter;

      if( snapshot->getTime() > m_appctx->Age_Of_Basin () ) {
         continue;
      }

      for ( formationSurfaceIter = formationSurfacePairs.begin(); formationSurfaceIter != formationSurfacePairs.end(); ++formationSurfaceIter ) {
         const Interface::Formation * formation = ( *formationSurfaceIter ).first;
         const Interface::Surface * surface = ( *formationSurfaceIter ).second;
         const Interface::Snapshot * bottomSurfaceSnapshot = ( formation->getBottomSurface() != 0 ? formation->getBottomSurface()->getSnapshot() : 0 );

         if( snapshot->getTime() != 0.0 and surface == 0 and bottomSurfaceSnapshot != 0 ) {
            const double depoAge = bottomSurfaceSnapshot->getTime();
            if ( snapshot->getTime() > depoAge or fabs( snapshot->getTime() - depoAge ) < snapshot->getTime() * 1e-9 ) {
               continue;
            }
         }

         for ( propertyIter = properties.begin(); propertyIter != properties.end(); ++propertyIter ) {

            const Interface::Property * property = *propertyIter;

           if( allowOutput( property->getName(), formation, surface )) {
              OutputPropertyValuePtr outputProperty =  DerivedProperties::allocateOutputProperty ( * m_propertyManager, property, snapshot, *formationSurfaceIter );

              if ( outputProperty != 0 ) {
                allOutputPropertyValues [ snapshot ][ *formationSurfaceIter ][ property ] = outputProperty;
              }
           }

         }
      }
   }

}
//------------------------------------------------------------//

bool DerivedPropertiesCalculator::allowOutput ( const string & propertyName,
                                                const Interface::Formation * formation, const Interface::Surface * surface ) const {

   if(( propertyName == "BrineDensity" or  propertyName == "BrineViscosity" ) and surface != 0 ) {
      return false;
   }
   if( m_decompactionMode and ( propertyName == "BulkDensity" ) and surface == 0 ) {
      return false;
   }
   bool basementFormation = ( dynamic_cast<const GeoPhysics::Formation*>( formation ) != 0 and
                              dynamic_cast<const GeoPhysics::Formation*>( formation )->kind () == DataAccess::Interface::BASEMENT_FORMATION );

   // The top of the crust is a part of the sediment
   if( basementFormation and surface != 0 and ( propertyName == "Depth" or propertyName == "Temperature" ) ) {
      if( dynamic_cast<const GeoPhysics::Formation*>( formation )->isCrust() ) {
         if( formation->getTopSurface() and ( formation->getTopSurface() == surface )) {
            return true;
         }
      }
   }

   const string outputPropertyName = PropertyManager::getInstance ().findOutputPropertyName( propertyName );
   OutputOption option = m_appctx->timefilter.getPropertyOutputOption( outputPropertyName );
   Interface::PropertyOutputOption fastcauldronOption = m_simulator->getOutputPropertyOption ( outputPropertyName );

   if( fastcauldronOption == Interface::NO_OUTPUT and option == NOOUTPUT ) {
      return false;
   }
   if( basementFormation ) {
      if( fastcauldronOption < Interface::SEDIMENTS_AND_BASEMENT_OUTPUT or option < SEDIMENTSPLUSBASEMENT ) {
         return false;
      }
   }

   return true;

}
//------------------------------------------------------------//

void DerivedPropertiesCalculator::acquirePropertyNames( const PropListVec& propertyNames ) {

   PropListVec::const_iterator propIter;

   string pname;

   for ( propIter = propertyNames.begin(); propIter != propertyNames.end(); ++ propIter ) {
      pname = PropertyManager::getInstance ().findPropertyName( propertyListName ( * propIter ));
      m_propertyNames.push_back( PropertyManager::getInstance ().findPropertyName( propertyListName ( * propIter )) );
   }

   if( not m_decompactionMode ) {
      m_propertyNames.push_back( "HorizontalPermeability" );
      m_propertyNames.push_back( "BrineDensity" );
      m_propertyNames.push_back( "BrineViscosity" );
   }

   // remove duplicated names
   std::sort( m_propertyNames.begin(), m_propertyNames.end() );
   m_propertyNames.erase( std::unique( m_propertyNames.begin(), m_propertyNames.end(), DerivedProperties::isEqualPropertyName ), m_propertyNames.end() );
}

//------------------------------------------------------------//

void DerivedPropertiesCalculator::printNames() {
   StringVector::iterator stringIter;

   cout << "Names: " << endl;
   for ( stringIter = m_propertyNames.begin(); stringIter != m_propertyNames.end(); ++stringIter ) {
      cout << * stringIter << endl;
   }
}
//------------------------------------------------------------//

bool snapshotSorter( const Snapshot * snapshot1, const Snapshot * snapshot2 ) {
   return snapshot1->getTime() < snapshot2->getTime();
}

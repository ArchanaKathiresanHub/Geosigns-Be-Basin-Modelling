//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include <petsc.h>

using namespace std;

// Utility library
#include "FormattingException.h"
#include "LogHandler.h"
#include "StatisticsHandler.h"

#include "AbstractPropertiesCalculator.h"
#include "VisualizationPropertiesCalculator.h"
#include "HdfPropertiesCalculator.h"

using namespace Utilities::CheckMemory;

int main( int argc, char ** argv )
{

   PetscInitialize (&argc, &argv, (char *) 0, PETSC_NULL);
   int rank;

   MPI_Comm_rank ( PETSC_COMM_WORLD, &rank );

#ifndef _MSC_VER
   PetscBool myddd = PETSC_FALSE;
   PetscOptionsHasName (PETSC_NULL, "-ddd", &myddd);
   if (myddd)
   {
      char cmd[150];

      sprintf (cmd, "ddd  %s %d &", argv[0],  getpid ());

      system (cmd);
      sleep (20);
   }
#endif
   //////////////////////////////////////////////////
   ///1. Initialise fastproperties logger
   try{
      PetscBool log = PETSC_FALSE;
      PetscOptionsHasName( PETSC_NULL, "-verbosity", &log );
      if (log){
         char verbosity[11];
         PetscOptionsGetString( PETSC_NULL, "-verbosity", verbosity, 11, 0 );
         if      (!strcmp( verbosity, "quiet"      )) { LogHandler( "fastproperties", LogHandler::QUIET_LEVEL     , rank ); }
         else if (!strcmp( verbosity, "minimal"    )) { LogHandler( "fastproperties", LogHandler::MINIMAL_LEVEL   , rank ); }
         else if (!strcmp( verbosity, "normal"     )) { LogHandler( "fastproperties", LogHandler::NORMAL_LEVEL    , rank ); }
         else if (!strcmp( verbosity, "detailed"   )) { LogHandler( "fastproperties", LogHandler::DETAILED_LEVEL  , rank ); }
         else if (!strcmp( verbosity, "diagnostic" )) { LogHandler( "fastproperties", LogHandler::DIAGNOSTIC_LEVEL, rank ); }
         else throw formattingexception::GeneralException() << "Unknown <" << verbosity << "> option for -verbosity command line parameter.";
      }
      else{
         LogHandler( "fastproperties", LogHandler::MINIMAL_LEVEL, rank );
      }
   }
   catch (formattingexception::GeneralException& ex){
      std::cout << ex.what();
      return 1;
   }
   catch (...){
      std::cout << "Basin_Fatal: Fatal error when initialising log file(s).";
      return 1;
   }

   ////////////////////////////////////////////
   ///2. Parse command line and create calculator
   PetscLogDouble sim_Start_Time;
   PetscTime( &sim_Start_Time );

   StatisticsHandler::initialise();

   GeoPhysics::ObjectFactory* factory = new GeoPhysics::ObjectFactory;

   PetscBool vizOption = PETSC_FALSE;
   PetscOptionsHasName( PETSC_NULL, "-viz", &vizOption );

   AbstractPropertiesCalculator * propCalculator;

   if (vizOption)
   {
      propCalculator = new VisualizationPropertiesCalculator( rank );
   }
   else
   {
      propCalculator = new HdfPropertiesCalculator( rank );
   }
   propCalculator->startTimer();

   if( !propCalculator->parseCommandLine( argc, argv )) {

      PetscFinalize();
      return 1;
   }

   if( ! propCalculator->createFrom(factory) ) {

      propCalculator->showUsage( argv[ 0 ], "Could not open specified project file" );
      delete propCalculator;

      PetscFinalize();

      return 1;
   }

   if( propCalculator->hdfonly() ) {

      propCalculator->writeToHDF();
      propCalculator->finalise ( false );
      delete propCalculator;

      PetscFinalize ();

      return 0;
   };

   if( propCalculator->listXml() ) {

      propCalculator->listXmlProperties();
      propCalculator->finalise ( false );
      delete propCalculator;

      PetscFinalize ();

      return 0;
   };

   ////////////////////////////////////////////
   ///3. Load data
   propCalculator->printOutputableProperties ();
   propCalculator->acquireSimulatorProperties();
   propCalculator->acquireAll2Dproperties();
   propCalculator->acquireAll3Dproperties();

   propCalculator->printListSnapshots();
   propCalculator->printListStratigraphy();

   if ( propCalculator->showLists() ) {
      propCalculator->finalise ( false );
      delete propCalculator;

      PetscFinalize ();

      return 0;
   }

   if ( !propCalculator->startActivity () ) {
      propCalculator->finalise ( false );
      delete propCalculator;

      PetscFinalize ();

      return 1;
   };

   SnapshotList snapshots;
   Interface::PropertyList properties;

   propCalculator->acquireSnapshots( snapshots );
   propCalculator->acquireProperties( properties );

   FormationSurfaceVector formationSurfaceItems;

   propCalculator->acquireFormationsSurfaces( formationSurfaceItems );
   //////////////////////////////////////////////////
   ///4. Compute derived properties
   try{

   propCalculator->calculateProperties( formationSurfaceItems, properties, snapshots );
   }
   catch (formattingexception::GeneralException& ex){
      LogHandler( LogHandler::ERROR_SEVERITY ) << ex.what();
      return 1;
   }
   catch (CauldronIO::CauldronIOException& except)
    {
       cerr << "Error occurred: " << except.what() << endl;
    }
   catch (...){
      LogHandler( LogHandler::FATAL_SEVERITY ) << "Fatal error when computing derived properties.";
      return 1;
   }

   ////////////////////////////////////////////
   ///5. Save results

   std::string statistics = StatisticsHandler::print(rank);

   PetscPrintf(PETSC_COMM_WORLD, "<statistics>\n");
   PetscSynchronizedFlush(PETSC_COMM_WORLD, PETSC_STDOUT);

   PetscSynchronizedPrintf(PETSC_COMM_WORLD, statistics.c_str());
   PetscSynchronizedFlush(PETSC_COMM_WORLD, PETSC_STDOUT);

   PetscPrintf(PETSC_COMM_WORLD, "</statistics>\n");
   PetscSynchronizedFlush(PETSC_COMM_WORLD, PETSC_STDOUT);

   bool status = propCalculator->finalise ( true );

   delete factory;

   delete propCalculator;

   PetscLogDouble sim_End_Time;
   PetscTime( &sim_End_Time );

   AbstractPropertiesCalculator::displayTime( sim_End_Time - sim_Start_Time, "End of calculation" );

   PetscFinalize ();
   return ( status ? 0 : 1 );
}

//
// Copyright (C) 2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include <stdlib.h>

#include "petsc.h"
#include <iostream>
using namespace std;

#include "Migrator.h"
#include "ObjectFactory.h"

#include "migration.h"
#include "rankings.h"
#include "StatisticsHandler.h"

using namespace migration;

#ifdef FLEXLM
#undef FLEXLM
#endif

#ifdef DISABLE_FLEXLM
#undef FLEXLM
#else
#define FLEXLM 1
#endif

#ifdef FLEXLM
// FlexLM license handling
#include <EPTFlexLm.h>
#endif

static char help[] = "Parallel Migration \n\n";

#undef __FUNCT__
#define __FUNCT__ "main"

void printUsage (char * argv0);

string NumProcessorsArg;

/// Main routine
int main (int argc, char ** argv)
{
   PetscInitialize (&argc, &argv, (char *)0, help);

   Utilities::CheckMemory::StatisticsHandler::initialise();

   char * strI = getenv ("DebugPointI");
   char * strJ = getenv ("DebugPointJ");

   if (strI && strJ)
   {
      DebugPoint = true;
      DebugPointI = atoi (strI);
      DebugPointJ = atoi (strJ);
   }

   char * argv0;
   if ((argv0 = strrchr (argv[0], '/')) == 0)
      argv0 = argv[0];
   else
      ++argv0;

   char inputFileName[PETSC_MAX_PATH_LEN];
   PetscBool inputFileSet;

   PetscOptionsGetString (PETSC_IGNORE, PETSC_IGNORE, "-project", inputFileName, PETSC_MAX_PATH_LEN, &inputFileSet);
   if (!inputFileSet)
   {
      printUsage (argv0);
      PetscFinalize ();
      return -1;
   }

   char outputFileName[PETSC_MAX_PATH_LEN];
   PetscBool outputFileSet;

   PetscOptionsGetString (PETSC_IGNORE, PETSC_IGNORE, "-save", outputFileName, PETSC_MAX_PATH_LEN, &outputFileSet);
   if (!outputFileSet)
   {
      strcpy (outputFileName, inputFileName);
   }

   char maxDev[128];
   PetscBool maxDevSet;
   PetscOptionsGetString (PETSC_IGNORE, PETSC_IGNORE, "-maxDev", maxDev, 128, &maxDevSet);

   char numProcessors[PETSC_MAX_PATH_LEN];
   PetscBool numProcessorsSet;

   NumProcessorsArg = "";
   PetscOptionsGetString (PETSC_IGNORE, PETSC_IGNORE, "-procs", numProcessors, PETSC_MAX_PATH_LEN, &numProcessorsSet);
   if (numProcessorsSet)
   {
      NumProcessorsArg += "_";
      NumProcessorsArg += numProcessors;
   }

   bool opLeak = false;
   PetscBool op_leakage = PETSC_FALSE;
   PetscOptionsHasName (PETSC_IGNORE, PETSC_IGNORE, "-op_leak", &op_leakage);
   if (op_leakage)
   {
      opLeak = true;
   }

#ifndef _MSC_VER
   PetscBool ddd = PETSC_FALSE;
   PetscOptionsHasName (PETSC_IGNORE, PETSC_IGNORE, "-ddd", &ddd);
   if (ddd)
   {
      char cmd[150];

      snprintf (cmd, sizeof (cmd), "ddd %s %d &", argv[0],  getpid ());
      system (cmd);
      sleep (10);
   }

   PetscBool idb = PETSC_FALSE;
   PetscOptionsHasName (PETSC_IGNORE, PETSC_IGNORE, "-idb", &idb);
   if (idb)
   {
      char cmd[150];

      snprintf (cmd, sizeof (cmd), "idb -p %d %s &", getpid (), argv[0]);
      system (cmd);
      sleep (15);
   }

   PetscBool cvd = PETSC_FALSE;
   PetscOptionsHasName (PETSC_IGNORE, PETSC_IGNORE, "-cvd", &cvd);
   if (cvd)
   {
      char cmd[150];

      snprintf (cmd, sizeof (cmd), "cvd -pid %d &", getpid ());
      system (cmd);
      sleep (20);
   }

   PetscBool gdb = PETSC_FALSE;
   PetscOptionsHasName (PETSC_IGNORE, PETSC_IGNORE, "-gdb", &gdb);
   if (gdb)
   {
      char cmd[150];

      snprintf (cmd, sizeof (cmd), "gdb %s %d &", argv[0],  getpid ());
      system (cmd);
      sleep (10);
   }
#endif

   int rank = 99999;
   MPI_Comm_rank (PETSC_COMM_WORLD, &rank);


#ifdef FLEXLM
   int rc = EPTFLEXLM_OK;

   char feature[EPTFLEXLM_MAX_FEATURE_LEN];
   char version[EPTFLEXLM_MAX_VER_LEN];
   char errmessage[EPTFLEXLM_MAX_MESS_LEN];

   // FlexLM license handling only for node with rank = 0
   if( rank == 0 )
   {
      snprintf(feature, sizeof (feature), "ibs_cauldron_calc");
#ifdef IBSFLEXLMVERSION
      snprintf(version, sizeof (version), IBSFLEXLMVERSION);
#else
      snprintf(version, sizeof (version), "9999.99");
#endif

      rc = EPTFlexLmInit(errmessage);
      if ( rc != EPTFLEXLM_OK )
      {
         fprintf(stderr, "\n@@@@@@@@@@@@@@@\n Basin_Fatal: FlexLm license init problems: fastcauldron cannot start.\n Please contact your helpdesk\n@@@@@@@@@@@@@@@\n");
      }
      // FlexLM license handling: Checkout
      rc = EPTFlexLmCheckOut( feature, version, errmessage );
      if (rc == EPTFLEXLM_WARN)
      {
         fprintf(stderr,"\n@@@@@@@@@@@@@@@\n Basin_Warning: FlexLm license warning: fastcauldron will still start anyway.\n@@@@@@@@@@@@@@@\n");
      }
      else if ( rc != EPTFLEXLM_OK )
      {
         fprintf(stderr,"\n@@@@@@@@@@@@@@@\n Basin_Error: FlexLm license error: fastcauldron cannot start.\n Please contact your helpdesk\n@@@@@@@@@@@@@@@\n");
      }
   }

   MPI_Bcast ( &rc, 1, MPI_INT, 0, PETSC_COMM_WORLD);

#endif

#ifdef FLEXLM
   if( rc != EPTFLEXLM_OK && rc != EPTFLEXLM_WARN)
   {
      //FlexLM license check in only for node with rank = 0
      if( rank == 0 )
      {
         // FlexLm license check in, close down and enable logging
         EPTFlexLmCheckIn( feature );
         EPTFlexLmTerminate();
      }
      // Close PetSc
      PetscFinalize ();

      return -1;
   }
#endif

   // Intitialise fastmig logger
   try
   {
      PetscBool log = PETSC_FALSE;
      PetscOptionsHasName(PETSC_IGNORE, PETSC_IGNORE, "-verbosity", &log );
      if ( log )
      {
         char verbosity[11];
         PetscOptionsGetString( PETSC_IGNORE, PETSC_IGNORE, "-verbosity", verbosity, 11, 0 );
         if      ( !strcmp( verbosity, "quiet"      ))  { LogHandler( "fastmig", LogHandler::QUIET_LEVEL,      rank ); }
         else if ( !strcmp( verbosity, "minimal"    ) ) { LogHandler( "fastmig", LogHandler::MINIMAL_LEVEL   , rank ); }
         else if ( !strcmp( verbosity, "normal"     ) ) { LogHandler( "fastmig", LogHandler::NORMAL_LEVEL    , rank ); }
         else if ( !strcmp( verbosity, "detailed"   ) ) { LogHandler( "fastmig", LogHandler::DETAILED_LEVEL  , rank ); }
         else if ( !strcmp( verbosity, "diagnostic" ) ) { LogHandler( "fastmig", LogHandler::DIAGNOSTIC_LEVEL, rank ); }
         else throw formattingexception::GeneralException() << "Unknown <" << verbosity << "> option for -verbosity command line parameter.";
      }
      else
      {
         LogHandler( "fastmig", LogHandler::DETAILED_LEVEL, rank );
      }
   }
   catch ( formattingexception::GeneralException & ex )
   {
      std::cerr << ex.what();
      return 1;
   }
   catch (...)
   {
      std::cerr << "Basin_Fatal: Fatal error when initialising log file(s).";
      return 1;
   }

   bool status = true;
   Migrator * migrator = 0;

   StartTime ();

   double maxDevDouble = -1;
   if (maxDevSet)
   {
     try
     {
       maxDevDouble = std::stod(maxDev);
     } catch (...)
     {
     }
   }

   if (status)
   {
      ReportProgress ("Reading Project File: ", inputFileName);
      migrator = new Migrator (inputFileName, maxDevDouble);
      status = (migrator != 0);
   }

   if (status)
   {
      ReportProgress ("Starting Simulation");
      status = migrator->compute (opLeak);
   }
   else
   {
      ReportProgress ("Starting Simulation");
   }

   if (status)
   {
      ReportProgress ("Finished Simulation Time Steps");
      ReportProgress ("Saved Output Maps");
   }
   else
   {
      ReportProgress ("Could not complete Simulation Time Steps");
      ReportProgress ("Did not save Output Maps");
   }

   if (status and GetRank () == 0)
   {
      migrator->sanitizeMigrationRecords ();
      migrator->checkMigrationRecords ();
      migrator->sortMigrationRecords ();
      migrator->checkMigrationRecords ();
      migrator->uniqueMigrationRecords ();
      migrator->checkMigrationRecords ();
      status = migrator->saveTo (outputFileName);
   }

   if (status)
   {
      ReportProgress ("Saved project file: ", outputFileName);
      ReportProgress ("Finished Simulation");
   }
   else
   {
      ReportProgress ("Did not save project file: ", outputFileName);
      ReportProgress ("Finished Simulation prematurely");
   }

   // Save the memory consumption before deleting migrator
   Utilities::CheckMemory::StatisticsHandler::update();
   delete migrator;

#ifdef FLEXLM
   //FlexLM license check in only for node with rank = 0
   if( rank == 0 )
   {
      // FlexLm license check in, close down and enable logging
      EPTFlexLmCheckIn( feature );
      EPTFlexLmTerminate();
   }
#endif

   // Print the memory consumption to standard out
   std::string statistics = Utilities::CheckMemory::StatisticsHandler::print( rank );

   PetscPrintf(PETSC_COMM_WORLD, "<statistics>\n");
   PetscSynchronizedFlush(PETSC_COMM_WORLD, PETSC_STDOUT);

   PetscSynchronizedPrintf(PETSC_COMM_WORLD, statistics.c_str());
   PetscSynchronizedFlush(PETSC_COMM_WORLD, PETSC_STDOUT);

   PetscPrintf(PETSC_COMM_WORLD, "</statistics>\n");
   PetscSynchronizedFlush(PETSC_COMM_WORLD, PETSC_STDOUT);

   PetscFinalize ();

   return status ? 0 : -1;
}

void printUsage (char * argv0)
{
   PetscPrintf (PETSC_COMM_WORLD, "usage: %s -project fileName [-save fileName]\n", argv0);
}

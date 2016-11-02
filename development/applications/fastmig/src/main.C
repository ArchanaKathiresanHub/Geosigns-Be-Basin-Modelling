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

   char inputFileName[128];
   PetscBool inputFileSet;

   PetscOptionsGetString (PETSC_NULL, "-project", inputFileName, 128, &inputFileSet);
   if (!inputFileSet)
   {
      printUsage (argv0);
      PetscFinalize ();
      return -1;
   }

   char outputFileName[128];
   PetscBool outputFileSet;

   PetscOptionsGetString (PETSC_NULL, "-save", outputFileName, 128, &outputFileSet);
   if (!outputFileSet)
   {
      strcpy (outputFileName, inputFileName);
   }

   char numProcessors[128];
   PetscBool numProcessorsSet;

   NumProcessorsArg = "";
   PetscOptionsGetString (PETSC_NULL, "-procs", numProcessors, 128, &numProcessorsSet);
   if (numProcessorsSet)
   {
      NumProcessorsArg += "_";
      NumProcessorsArg += numProcessors;
   }

#ifndef _MSC_VER
   PetscBool ddd = PETSC_FALSE;
   PetscOptionsHasName (PETSC_NULL, "-ddd", &ddd);
   if (ddd)
   {
      char cmd[150];

      sprintf (cmd, "ddd %s %d &", argv[0],  getpid ());
      system (cmd);
      sleep (10);
   }

   PetscBool idb = PETSC_FALSE;
   PetscOptionsHasName (PETSC_NULL, "-idb", &idb);
   if (idb)
   {
      char cmd[150];

      sprintf (cmd, "idb -p %d %s &", getpid (), argv[0]);
      system (cmd);
      sleep (15);
   }

   PetscBool cvd = PETSC_FALSE;
   PetscOptionsHasName (PETSC_NULL, "-cvd", &cvd);
   if (cvd)
   {
      char cmd[150];

      sprintf (cmd, "cvd -pid %d &", getpid ());
      system (cmd);
      sleep (20);
   }

   PetscBool gdb = PETSC_FALSE;
   PetscOptionsHasName (PETSC_NULL, "-gdb", &gdb);
   if (gdb)
   {
      char cmd[150];

      sprintf (cmd, "gdb %s %d &", argv[0],  getpid ());
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
      sprintf(feature, "ibs_cauldron_calc");
#ifdef IBSFLEXLMVERSION
      sprintf(version, IBSFLEXLMVERSION);
#else
      sprintf(version, "9999.99");
#endif

      rc = EPTFlexLmInit(errmessage);
      if ( rc != EPTFLEXLM_OK )
      {
         fprintf(stderr, "\n@@@@@@@@@@@@@@@\n FlexLm license init problems: fastcauldron cannot start.\n Please contact your helpdesk\n@@@@@@@@@@@@@@@\n");
      }
      // FlexLM license handling: Checkout
      rc = EPTFlexLmCheckOut( feature, version, errmessage );
      if (rc == EPTFLEXLM_WARN)
      {
         fprintf(stderr,"\n@@@@@@@@@@@@@@@\n FlexLm license warning: fastcauldron will still start anyway.\n@@@@@@@@@@@@@@@\n");
      }
      else if ( rc != EPTFLEXLM_OK )
      {
         fprintf(stderr,"\n@@@@@@@@@@@@@@@\n FlexLm license error: fastcauldron cannot start.\n Please contact your helpdesk\n@@@@@@@@@@@@@@@\n");
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

   bool status = true;
   Migrator * migrator = 0;

   //ObjectFactory* objectFactory = new ObjectFactory();

   StartTime ();

   if (status)
   {
      ReportProgress ("Reading Project File: ", inputFileName);
      migrator = new Migrator (inputFileName);
      status = (migrator != 0);
   }

   if (status)
   {
      ReportProgress ("Starting Simulation");
      status = migrator->compute ();
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

   if (status)
   {
      if (GetRank () == 0)
      {
         migrator->sanitizeMigrationRecords ();
         migrator->checkMigrationRecords ();
         migrator->sortMigrationRecords ();
         migrator->checkMigrationRecords ();
         migrator->uniqueMigrationRecords ();
         migrator->checkMigrationRecords ();
         status = migrator->saveTo (outputFileName);
      }
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
   //delete objectFactory;

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

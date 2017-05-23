//
// Copyright (C) 2012-2017 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "hdf5.h"
#include <stdlib.h>

#include "petscvec.h"
#include "petsc.h"

#include <iostream>
#include <sstream>
#include <iomanip>
using namespace std;

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "Interface/ObjectFactory.h"

#include "FastTouch.h"

#include "misc.h"

#ifdef _WIN32
   #include <direct.h>
   #include <process.h>
   #include <windows.h>
   #define GetCurrentDir _getcwd
   #define GetProcessID _getpid
   #define DIRDELIMCHAR "\\"
#else
   #ifndef _XOPEN_SOURCE
      #define _XOPEN_SOURCE 500
   #endif
   #include <ftw.h>
   #define GetCurrentDir getcwd
   #define GetProcessID getpid
   #define DIRDELIMCHAR "/"
#endif

using namespace fasttouch;

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

/** Help message to pass it to Petsc. */
static char help[] = "Parallel FastTouch \n\n";

#undef __FUNCT__
#define __FUNCT__ "main"

#define EXIT(v)	{ if (debug) ReportProgress ("Exiting..."); _exit(v); }

/** Print using Petsc print function. */
void PrintUsage (char * argv0);

PetscBool debug = PETSC_FALSE;
//////////////////////////////////////////////////////////////////
// Set of auxillar functions to set/clean up temporary folder per MPI process
// to keep Matlab MCR cache files during CreateRealization call
static void SetUpTempMCRFolder()
{
   std::stringstream oss;
   oss << "MCR_CACHE_ROOT=";

   const char * tmpDir = getenv( "TMPDIR" );
   if ( tmpDir )
   {
      oss << tmpDir;
   }
   else
   {
      char * workingDir = GetCurrentDir( NULL, 0 );
      oss << workingDir;
      free( workingDir );
   }

   oss << DIRDELIMCHAR << "mcrTemp_" << GetProcessID();
   putenv( strdup( oss.str().c_str() ) );
}

static int unlink_cb( const char * fpath, const struct stat * sb, int typeflag, struct FTW *ftwbuf )
{
    int rv = remove(fpath);
    if ( rv ) perror( fpath );
    return rv;
}

static void CleanTempMCRFolder()
{
   const char * tmpDir = getenv( "TMPDIR" );
   if ( !tmpDir ) // clean MCR cache folder only in case if it is in current folder (no TMPDIR variable defined)
   {
      const char * path = getenv( "MCR_CACHE_ROOT" );
#ifndef _WIN32
      nftw( path, unlink_cb, 64, FTW_DEPTH | FTW_PHYS );
#endif
   }
}
//////////////////////////////////////////////////////////////////


/** Fast Touch application: Touch stone 7.0 implementation calculating the
 * porosity of the rock based on the temperature and pressure history. */
int main (int argc, char ** argv)
{
    // before we do anything else ..
    // to make sure matlab is not calling this function with disastrous effects
    H5open();

    PetscInitialize (&argc, &argv, (char *) 0, help);

    char * argv0;
    if ((argv0 = strrchr (argv[0], '/')) == 0)
        argv0 = argv[0];
    else
        ++argv0;

    PetscOptionsHasName (PETSC_NULL, "-debug", &debug);

    char inputFileName[128];
    PetscBool inputFileSet;

    PetscOptionsGetString (PETSC_NULL, "-project", inputFileName, 128, &inputFileSet);
    if (!inputFileSet)
    {
        PrintUsage (argv0);
        PetscFinalize ();
        EXIT (-1);
    }

    char outputFileName[128];
    PetscBool outputFileSet;

    PetscOptionsGetString (PETSC_NULL, "-save", outputFileName, 128, &outputFileSet);
    if (!outputFileSet)
    {
        strcpy (outputFileName, inputFileName);
    }

#ifdef unix

    PetscBool ddd = PETSC_FALSE;
    PetscOptionsHasName (PETSC_NULL, "-ddd", &ddd);
    if (ddd)
    {
        char cmd[150];

        sprintf (cmd, "/usr/bin/ddd --debugger /usr/bin/gdb %s %d &", argv[0],  getpid ());
        system (cmd);
        sleep(10);
    }

    PetscBool myddd = PETSC_FALSE;
    PetscOptionsHasName (PETSC_NULL, "-myddd", &myddd);
    if (myddd)
    {
        char cmd[150];

        sprintf (cmd, "/glb/home/ksaho3/bin/myddd %s %d &", argv[0],  getpid ());
        system (cmd);
        sleep(10);
    }

    PetscBool gdb = PETSC_FALSE;
    PetscOptionsHasName (PETSC_NULL, "-gdb", &gdb);
    if (gdb)
    {
        char cmd[150];

        sprintf (cmd, "/usr/bin/gdb %s %d &", argv[0],  getpid ());
        system (cmd);
        sleep(10);
    }

#endif


#ifdef FLEXLM

    int rc = EPTFLEXLM_OK;

    char feature[EPTFLEXLM_MAX_FEATURE_LEN];
    char version[EPTFLEXLM_MAX_VER_LEN];
    char errmessage[EPTFLEXLM_MAX_MESS_LEN];

    if( GetRank() == 0 )
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
            fprintf(stderr, "\n@@@@@@@@@@@@@@@\n FlexLm license init problems: fasttouch cannot start.\n Please contact your helpdesk\n@@@@@@@@@@@@@@@\n");
        }
        // FlexLM license handling: Checkout
        rc = EPTFlexLmCheckOut( feature, version, errmessage );
        if (rc == EPTFLEXLM_WARN)
        {
            fprintf(stderr,"\n@@@@@@@@@@@@@@@\n FlexLm license warning: fasttouch will still start anyway.\n@@@@@@@@@@@@@@@\n");
        }
        else if ( rc != EPTFLEXLM_OK )
        {
            fprintf(stderr,"\n@@@@@@@@@@@@@@@\n FlexLm license error: fasttouch cannot start.\n Please contact your helpdesk\n@@@@@@@@@@@@@@@\n");
        }
    }
    MPI_Bcast ( &rc, 1, MPI_INT, 0, PETSC_COMM_WORLD);

    if( rc != EPTFLEXLM_OK && rc != EPTFLEXLM_WARN)
    {
        //FlexLM license check in only for node with rank = 0
        if( GetRank() == 0 )
        {
            // FlexLm license check in, close down and enable logging
            EPTFlexLmCheckIn( feature );
            EPTFlexLmTerminate();
        }
        // Close PetSc
        PetscFinalize ();
        EXIT (-1);
   }
#endif
    SetUpTempMCRFolder();

    bool status = true;
    FastTouch * fastTouch = 0;

    StartProgress ();
    if (debug) ReportProgress (string ("XAPPLRESDIR: ") + getenv ("XAPPLRESDIR"));

    if (!status)
    {
       ReportProgress (string ("ERROR: Could not start FastTouch7"));
        EXIT (-1);
    }

    ReportProgress (string ("Reading Project File: ") + inputFileName);

    DataAccess::Interface::ObjectFactory* factory = new DataAccess::Interface::ObjectFactory();
    fastTouch = new FastTouch(inputFileName, factory);

    status = (fastTouch != 0);

    if (!status)
    {
       ReportProgress (string ("ERROR: Could not open Project File: ") + inputFileName);
        EXIT (-1);
    }

    status = fastTouch->removeResqPropertyValues ();
    if (!status)
    {
       ReportProgress ("ERROR: Failed to remove existing Resq property values: ");
        EXIT (-1);
    }

    ReportProgress ("Starting Simulation");


    status = fastTouch->compute ();
    if (!status)
    {
       char * touchstoneWrapperFailure = getenv("touchstoneWrapperFailure");
       int returnStatus = ( touchstoneWrapperFailure == nullptr ? -1 : 100 );
       ReportProgress ("ERROR: Failed to complete simulation");
       EXIT (returnStatus);
    }

    ReportProgress ("Finished Simulation, Saved output maps");
    status = fastTouch->saveTo (outputFileName);
    if (!status)
    {
       ReportProgress (string ("Did not save project file: ") + outputFileName);
       ReportProgress ("Finished Simulation prematurely");
        EXIT (-1);
    }
    ReportProgress (string ("Saved project file: ") + outputFileName);
    ReportProgress ("Finished Simulation");

    if (debug) ReportProgress ("deleting Objects");
    // delete fastTouch;
    // delete objectFactory;

#ifdef FLEXLM
    //FlexLM license check in only for node with rank = 0
    if( GetRank() == 0 )
    {
        // FlexLm license check in, close down and enable logging
        EPTFlexLmCheckIn( feature );
        EPTFlexLmTerminate();
    }
#endif

    CleanTempMCRFolder();

    PetscFinalize ();

    delete factory;

    // To prevent functions registered with atexit being called as they are causing crashes
   EXIT (status ? 0 : -1);
}

/** Print using Petsc print function. */
void PrintUsage (char * argv0)
{
      PetscPrintf (PETSC_COMM_WORLD, "usage: %s -project fileName [-save fileName]\n", argv0);
}

int AndAll (int myValue)
{
   int result;
   MPI_Allreduce (&myValue, &result, 1, MPI_INT, MPI_LAND, PETSC_COMM_WORLD);
   return result;
}

double MaximumAll (int myValue)
{
   int result;
   MPI_Allreduce (&myValue, &result, 1, MPI_INT, MPI_MAX, PETSC_COMM_WORLD);
   return result;
}

double MinimumAll (double myValue)
{
   double result;
   MPI_Allreduce (&myValue, &result, 1, MPI_DOUBLE, MPI_MIN, PETSC_COMM_WORLD);
   // if (rank == 0 && debug) cerr << "Minimum (" << myValue << ") = " << result << endl;
   return result;
}

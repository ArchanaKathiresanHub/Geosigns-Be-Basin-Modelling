#include <stdlib.h>
#include <unistd.h>

#include "petscvec.h"
#include "petscda.h"

#include <iostream>
#include <sstream>
#include <iomanip>
using namespace std;

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "FastTouch.h"
#include "ObjectFactory.h"

#include "WallTime.h"
using namespace WallTime;

using namespace fasttouch;

#ifdef FLEXLM
#undef FLEXLM
#endif

// #define FLEXLM	1

#ifdef FLEXLM
// FlexLM license handling
#include <EPTFlexLm.h>
#endif

/** Help message to pass it to Petsc. */
static char help[] = "Parallel FastTouch \n\n";

#undef __FUNCT__
#define __FUNCT__ "main"

#define EXIT(v)	{ if (debug) ReportProgress ("Exiting..."); _exit(v); }

/** Start clock to measure time taken. */
void StartProgress (void);

/** Stop clock and caluclate time taken. */
void ReportProgress (const string & message);

/** Print using Petsc print function. */
void PrintUsage (char * argv0);

/** Holds the argument for number of processors. */
string NumProcessorsArg;

int rank = -1;
int numProcessors = -1;
PetscTruth debug = PETSC_FALSE;

/** Fast Touch application: Touch stone 7.0 implementation calculating the
 * porosity of the rock based on the temperature and pressure history. */
int main (int argc, char ** argv)
{
    PetscInitialize (&argc, &argv, (char *) 0, help);

    MPI_Comm_rank(PETSC_COMM_WORLD, &rank);
    MPI_Comm_size (PETSC_COMM_WORLD, &numProcessors);
    
    char * argv0;
    if ((argv0 = strrchr (argv[0], '/')) == 0)
        argv0 = argv[0];
    else
        ++argv0;
    
    PetscOptionsHasName (PETSC_NULL, "-debug", &debug);

    char inputFileName[128];
    PetscTruth inputFileSet;
 
    PetscOptionsGetString (PETSC_NULL, "-project", inputFileName, 128, &inputFileSet);
    if (!inputFileSet)
    {
        PrintUsage (argv0);
        PetscFinalize ();
        EXIT (-1);
    }
 
    char outputFileName[128];
    PetscTruth outputFileSet;
 
    PetscOptionsGetString (PETSC_NULL, "-save", outputFileName, 128, &outputFileSet);
    if (!outputFileSet)
    {
        strcpy (outputFileName, inputFileName);
    }
 
    PetscTruth ddd = PETSC_FALSE;
    PetscOptionsHasName (PETSC_NULL, "-ddd", &ddd);
    if (ddd)
    {
        char cmd[150];
        
        sprintf (cmd, "/usr/bin/ddd --debugger /usr/bin/gdb %s %d &", argv[0],  getpid ());
        system (cmd);
        sleep (10);
    }

    PetscTruth myddd = PETSC_FALSE;
    PetscOptionsHasName (PETSC_NULL, "-myddd", &myddd);
    if (myddd)
    {
        char cmd[150];
        
        sprintf (cmd, "/glb/home/ksaho3/bin/myddd %s %d &", argv[0],  getpid ());
        system (cmd);
        sleep (10);
    }
    
    PetscTruth gdb = PETSC_FALSE;
    PetscOptionsHasName (PETSC_NULL, "-gdb", &gdb);
    if (gdb)
    {
        char cmd[150];
        
        sprintf (cmd, "/usr/bin/gdb %s %d &", argv[0],  getpid ());
        system (cmd);
        sleep (10);
    }


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
        if( rank == 0 )
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
 
    bool status = true;
    FastTouch * fastTouch = 0;

    ObjectFactory* objectFactory = new ObjectFactory();
    DataAccess::Interface::ProjectHandle::UseFactory (objectFactory);
    
    StartProgress ();
    if (debug) ReportProgress (string ("XAPPLRESDIR: ") + getenv ("XAPPLRESDIR"));
 
    if (!status)
    {
        ReportProgress (string ("ERROR: Could not start FastTouch7"));
        EXIT (-1);
    }

    ReportProgress (string ("Reading Project File: ") + inputFileName);
    fastTouch = FastTouch::CreateFrom (inputFileName);
    
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
        ReportProgress ("ERROR: Failed to complete simulation");
        EXIT (-1);
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
    if( rank == 0 )
    {
        // FlexLm license check in, close down and enable logging
        EPTFlexLmCheckIn( feature );
        EPTFlexLmTerminate();
    } 
#endif
   
    PetscFinalize ();

    // To prevent functions registered with atexit being called as they are causing crashes
   EXIT (status ? 0 : -1);
}

/** Print using Petsc print function. */
void PrintUsage (char * argv0)
{
      PetscPrintf (PETSC_COMM_WORLD, "usage: %s -project fileName [-save fileName]\n", argv0);
}

WallTime::Time StartTime;
WallTime::Time LapTime;

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

/** Start clock to measure time taken. */
void StartProgress (void)
{
   StartTime = WallTime::clock ();
}

/** Stop clock and calculate time taken. */
void ReportProgress (const string & message)
{
   int hours;
   int minutes;
   double seconds;

   WallTime::Time clockTime = WallTime::clock ();

   WallTime::Duration executionTime = clockTime - StartTime;


   executionTime.separate (hours, minutes, seconds);

   int len = message.size ();

   if (rank == 0)
   {
      cerr << message;
      cerr << " ";
      while (len < 80)
      {
         cerr << " ";
         ++len;
      }
      cerr << "Time = ";
      cerr << setw (2) << setfill ('0') << hours << ":";
      cerr << setw (2) << setfill ('0') << minutes << ":";
      cerr << setw (2) << setfill ('0') << (int) seconds << endl;
      cerr.flush ();
   }
}

bool AllHaveCompleted (bool completed)
{
   if (AndAll ((int) (completed))) return true;   // All have completed
   else return false;
}

void InitializeTimeComplete ()
{
   LapTime = WallTime::clock ();
}

bool ReportTimeToComplete (int stepsCompleted, int totalNumberOfSteps, int & reportAfterTime, double &reportAfterFractionCompleted)
{
   int hours;
   int minutes;
   double seconds;

   const int reportAfterTimeIncrement = 300;
   const double fractionIncrement = 0.1;

   double fractionCompleted = MinimumAll ((double) stepsCompleted / (double) totalNumberOfSteps);

   WallTime::Time clockTime = WallTime::clock ();
   WallTime::Duration executionTime = clockTime - LapTime;

   if (fractionCompleted < reportAfterFractionCompleted && executionTime.floatValue () < reportAfterTime)
   {
      return false;
   }

   if (debug && rank == 0)
   {
      cerr << "executionTime = " << executionTime.floatValue () << endl;
      cerr << "fractionCompleted = " << fractionCompleted << endl;
   }

   while (executionTime.floatValue () >= reportAfterTime)
   {
      if (debug && rank == 0) cerr << reportAfterTime << " -> ";
      reportAfterTime += reportAfterTimeIncrement;
      if (debug && rank == 0) cerr << "reportAfterTime -> " << reportAfterTime << endl;
   }

   while (fractionCompleted >= reportAfterFractionCompleted)
   {
      if (debug && rank == 0) cerr << reportAfterFractionCompleted << " -> ";
      reportAfterFractionCompleted += fractionIncrement;
      if (debug && rank == 0) cerr << "reportAfterFractionCompleted -> " << reportAfterFractionCompleted << endl;
   }


   double multiplicationFactor = (1 - fractionCompleted) / fractionCompleted;
   WallTime::Duration timeToComplete = executionTime * multiplicationFactor;

   ostringstream buf2;
   timeToComplete.separate (hours, minutes, seconds);

   buf2 << "percentage completed: " << fractionCompleted * 100;
   if (debug) buf2 << " (" << stepsCompleted << " of " << totalNumberOfSteps << ")";
   buf2 << ", estimated time to complete: ";
   buf2 << setw (2) << setfill ('0') << hours << ":";
   buf2 << setw (2) << setfill ('0') << minutes << ":";
   buf2 << setw (2) << setfill ('0') << (int) seconds;
   ReportProgress (buf2.str ());

   return false;
}

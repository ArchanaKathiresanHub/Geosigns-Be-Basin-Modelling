#include <stdlib.h>
#include "petsc.h"

#include <iostream>
#include <string>

#ifdef WIN32
#include <io.h>
#else
#include <unistd.h>
#endif // WIN_32

using namespace std;

//utilities library
#include "LogHandler.h"
#include "FormattingException.h"

#include "h5_parallel_file_types.h"

#include "GenexSimulator.h"
#include "GenexSimulatorFactory.h"
using namespace GenexSimulation;

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

static char help[] = "Parallel Generation Expulsion\n\n";

#undef __FUNCT__
#define __FUNCT__ "main"

void printUsage ();


int main (int argc, char ** argv)
{
   PetscInitialize (&argc, &argv, (char *) 0, help);

   char * argv0;
   if ((argv0 = strrchr (argv[0], '/')) == 0)
      argv0 = argv[0];
   else
      ++argv0;

   char inputFileName[128];
   PetscBool inputFileSet;

   PetscOptionsGetString (PETSC_NULL, "-project", inputFileName, 128, &inputFileSet);
   if (!inputFileSet)  {
      printUsage ();
      PetscFinalize ();
      return -1;
   }

   char outputFileName[128];
   PetscBool outputFileSet;

   PetscOptionsGetString (PETSC_NULL, "-save", outputFileName, 128, &outputFileSet);
   if (!outputFileSet) {
      strcpy (outputFileName, inputFileName);
   }

#ifndef _MSC_VER
   PetscBool ddd = PETSC_FALSE;
   PetscOptionsHasName (PETSC_NULL, "-ddd", &ddd);
   if (ddd)  {
      char cmd[150];

      sprintf (cmd, "/usr/bin/ddd --debugger /usr/bin/gdb %s %d &", argv[0],  getpid ());

      system (cmd);
      sleep (20);
   }

   PetscBool myddd = PETSC_FALSE;
   PetscOptionsHasName (PETSC_NULL, "-myddd", &myddd);
   if (myddd)
   {
      char cmd[150];

      sprintf (cmd, "myddd  %s %d &", argv[0],  getpid ());

      system (cmd);
      sleep (20);
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
   if (cvd) {
      char cmd[150];

      sprintf (cmd, "cvd -pid %d &", getpid ());
      system (cmd);
      sleep (20);
   }
#endif

   int rank = 99999;
   MPI_Comm_rank(PETSC_COMM_WORLD, &rank);


#ifdef FLEXLM
   int rc = EPTFLEXLM_OK;

   char feature[EPTFLEXLM_MAX_FEATURE_LEN];
   char version[EPTFLEXLM_MAX_VER_LEN];
   char errmessage[EPTFLEXLM_MAX_MESS_LEN];

   // FlexLM license handling only for node with rank = 0
   if( rank == 0 ) {
      sprintf(feature, "ibs_cauldron_calc");
#ifdef IBSFLEXLMVERSION
      sprintf(version, IBSFLEXLMVERSION);
#else
      sprintf(version, "9999.99");
#endif

      rc = EPTFlexLmInit(errmessage);
      if ( rc != EPTFLEXLM_OK ) {
        fprintf(stderr, "\n@@@@@@@@@@@@@@@\n FlexLm license init problems: fastcauldron cannot start.\n Please contact your helpdesk\n@@@@@@@@@@@@@@@\n");
      }
      // FlexLM license handling: Checkout
      rc = EPTFlexLmCheckOut(feature, version, errmessage);
      if (rc == EPTFLEXLM_WARN) {
        fprintf(stderr,"\n@@@@@@@@@@@@@@@\n FlexLm license warning: fastcauldron will still start anyway.\n@@@@@@@@@@@@@@@\n");
      } else if (rc != EPTFLEXLM_OK) {
        fprintf(stderr,"\n@@@@@@@@@@@@@@@\n FlexLm license error: fastcauldron cannot start.\n Please contact your helpdesk\n@@@@@@@@@@@@@@@\n");
      }
   }

   MPI_Bcast ( &rc, 1, MPI_INT, 0, PETSC_COMM_WORLD);

#endif

#ifdef FLEXLM
   if( rc != EPTFLEXLM_OK && rc != EPTFLEXLM_WARN) {
      //FlexLM license check in only for node with rank = 0
      if( rank == 0 ) {
         // FlexLm license check in, close down and enable logging
         EPTFlexLmCheckIn( feature );
         EPTFlexLmTerminate();
      }
      // Close PetSc
      PetscFinalize ();

      return -1;
   }
#endif

   // Intitialise fastcauldron loger
   try
   {
      const std::string applicatioName = "fastgenex";
      PetscBool log = PETSC_FALSE;
      PetscOptionsHasName( PETSC_NULL, "-verbosity", &log );
      if ( log )
      {
         char verbosity[11];
         PetscOptionsGetString( PETSC_NULL, "-verbosity", verbosity, 11, 0 );
         if      ( !strcmp( verbosity, "quiet"      ))  { LogHandler( applicatioName, LogHandler::QUIET_LEVEL     , rank ); }
         else if ( !strcmp( verbosity, "minimal"    ) ) { LogHandler( applicatioName, LogHandler::MINIMAL_LEVEL   , rank ); }
         else if ( !strcmp( verbosity, "normal"     ) ) { LogHandler( applicatioName, LogHandler::NORMAL_LEVEL    , rank ); }
         else if ( !strcmp( verbosity, "detailed"   ) ) { LogHandler( applicatioName, LogHandler::DETAILED_LEVEL  , rank ); }
         else if ( !strcmp( verbosity, "diagnostic" ) ) { LogHandler( applicatioName, LogHandler::DIAGNOSTIC_LEVEL, rank ); }
         else throw formattingexception::GeneralException() << "Unknown <" << verbosity << "> option for -verbosity command line parameter.";
      }
      else
      {
         LogHandler( applicatioName, LogHandler::DETAILED_LEVEL, rank );
      }
   }
   catch ( formattingexception::GeneralException & ex )
   {
      std::cerr << ex.what();
      return 1;
   }
   catch (...)
   {
      std::cerr << "Fatal error when initialising log file(s).";
      return 1;
   }

   bool status = false;

   try {
     //create the factory
     GenexSimulatorFactory theFactory;

     //create the ProjectHandle
     std::unique_ptr<GenexSimulator> theGenexSimulator( GenexSimulator::CreateFrom (inputFileName, &theFactory) );

     status = (theGenexSimulator != 0);

     H5_Parallel_PropertyList::setOneFilePerProcessOption(false);

     //delete existent properties
     if (status) {
        theGenexSimulator->deleteSourceRockPropertyValues();
        theGenexSimulator->deletePropertyValues (DataAccess::Interface::RESERVOIR , 0, 0, 0, 0, 0,
                                                 DataAccess::Interface::MAP);
     }

     //run the simulation
     if (status) status = theGenexSimulator->run();

     //save results
     if (status) status = theGenexSimulator->saveTo(outputFileName);
   }
   catch ( std::string& s ) {
      LogHandler( LogHandler::ERROR_SEVERITY ) << s;
      exit (1);
   }
   catch ( std::exception& e )
   {
      LogHandler( LogHandler::ERROR_SEVERITY ) << e.what( );
      exit (1);
   }
   catch (...) {
      LogHandler( LogHandler::FATAL_SEVERITY ) << "Unknown exception occured.\n";
      exit (1);
   }

#ifdef FLEXLM
   //FlexLM license check in only for node with rank = 0
   if( rank == 0 ) {
      // FlexLm license check in, close down and enable logging
      EPTFlexLmCheckIn( feature );
      EPTFlexLmTerminate();
   }
#endif

   PetscFinalize ();

   return status ? 0 : -1;
}

void printUsage ()
{
      PetscPrintf (PETSC_COMM_WORLD, "usage: %s -project fileName [-save fileName]\n");
}

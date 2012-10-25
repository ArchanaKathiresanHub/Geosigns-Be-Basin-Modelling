#include <stdlib.h>
#include <unistd.h>

#include "petscvec.h"
#include "petscda.h"

#ifdef sgi
   #ifdef _STANDARD_C_PLUS_PLUS
      #include <iostream>
      using namespace std;
   #else // !_STANDARD_C_PLUS_PLUS
      #include<iostream.h>
   #endif // _STANDARD_C_PLUS_PLUS
#else // !sgi
   #include <iostream>
   using namespace std;
#endif // sgi

#include "GenexSimulator.h"
#include "GenexSimulatorFactory.h"
using namespace GenexSimulation;

#ifdef FLEXLM
#undef FLEXLM
#endif

#define FLEXLM	1

#ifdef FLEXLM
// FlexLM license handling
#include <EPTFlexLm.h>
#endif

static char help[] = "Parallel Generation Expulsion\n\n";

#undef __FUNCT__
#define __FUNCT__ "main"

void printUsage (char * argv0);


int main (int argc, char ** argv)
{
   PetscInitialize (&argc, &argv, (char *) 0, help);

   char * argv0;
   if ((argv0 = strrchr (argv[0], '/')) == 0)
      argv0 = argv[0];
   else
      ++argv0;

   char inputFileName[128];
   PetscTruth inputFileSet;

   PetscOptionsGetString (PETSC_NULL, "-project", inputFileName, 128, &inputFileSet);
   if (!inputFileSet)  {
      printUsage (argv0);
      PetscFinalize ();
      return -1;
   }

   char outputFileName[128];
   PetscTruth outputFileSet;

   PetscOptionsGetString (PETSC_NULL, "-save", outputFileName, 128, &outputFileSet);
   if (!outputFileSet) {
      strcpy (outputFileName, inputFileName);
   }

   PetscTruth ddd = PETSC_FALSE;
   PetscOptionsHasName (PETSC_NULL, "-ddd", &ddd);
   if (ddd)  {
      char cmd[150];

      sprintf (cmd, "/usr/bin/ddd --debugger /usr/bin/gdb %s %d &", argv[0],  getpid ());
      
      system (cmd);
      sleep (20);
   }

   PetscTruth myddd = PETSC_FALSE;
   PetscOptionsHasName (PETSC_NULL, "-myddd", &myddd);
   if (myddd)
   {
      char cmd[150];

      sprintf (cmd, "myddd  %s %d &", argv[0],  getpid ());
      
      system (cmd);
      sleep (20);
   }

   PetscTruth idb = PETSC_FALSE;
   PetscOptionsHasName (PETSC_NULL, "-idb", &idb);
   if (idb)
   {
      char cmd[150];

      sprintf (cmd, "idb -p %d %s &", getpid (), argv[0]);
      system (cmd);
      sleep (15);
   }

   PetscTruth cvd = PETSC_FALSE;
   PetscOptionsHasName (PETSC_NULL, "-cvd", &cvd);
   if (cvd) {
      char cmd[150];

      sprintf (cmd, "cvd -pid %d &", getpid ());
      system (cmd);
      sleep (20);
   }
   
   int rank = 99999;
   MPI_Comm_rank(PETSC_COMM_WORLD, &rank);
   
   int rc = EPTFLEXLM_OK;

#ifdef FLEXLM
  
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
   
   if( rc != EPTFLEXLM_OK && rc != EPTFLEXLM_WARN) {
#ifdef FLEXLM
      //FlexLM license check in only for node with rank = 0
      if( rank == 0 ) {
         // FlexLm license check in, close down and enable logging
         EPTFlexLmCheckIn( feature );
         EPTFlexLmTerminate();
      }
#endif
      // Close PetSc
      PetscFinalize ();
      
      return -1;
   }
   bool status = false;
   
   GenexSimulator        *theGenexSimulator = 0;
   GenexSimulatorFactory *theFactory        = 0;

   //create the factory
   theFactory = new GenexSimulatorFactory;

   if(theFactory) { 
      DataAccess::Interface::ProjectHandle::UseFactory (theFactory);
      status = true;
   }
   //create the ProjectHandle
   if (status) {
      theGenexSimulator = GenexSimulator::CreateFrom (inputFileName);
      status = (theGenexSimulator != 0);
   }
   //delete existent properties
   if (status) {
      theGenexSimulator->deleteSourceRockPropertyValues();
      theGenexSimulator->deletePropertyValues (DataAccess::Interface::RESERVOIR , 0, 0, 0, 0, 0, 
                                               DataAccess::Interface::MAP);
   }

   try {

      //run the simulation
      if (theGenexSimulator) {
         status = theGenexSimulator->run();
      }

   }

   catch ( std::string& s ) {
      std::cerr << "MeSsAgE ERROR " << s << std::endl;
      exit (1);
   }

   catch (...) {
      exit (1);
   }

   //save results
   if (status) status = theGenexSimulator->saveTo(outputFileName);
   
   //clean up
   delete theGenexSimulator;
   delete theFactory;
   
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

void printUsage (char * argv0)
{
      PetscPrintf (PETSC_COMM_WORLD, "usage: %s -project fileName [-save fileName]\n");
}

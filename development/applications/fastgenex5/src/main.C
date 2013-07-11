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

#include "database.h"
#include "cauldronschema.h"
#include "cauldronschemafuncs.h"

#include "AdsorptionProjectHandle.h"

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


   const unsigned int FileNameLength = 128; // The longest string that PETSc can read.

   unsigned int i;

   char inputFileName[FileNameLength+1];
   PetscBool inputFileSet;

   PetscOptionsGetString (PETSC_NULL, "-project", inputFileName, FileNameLength, &inputFileSet);
   if (!inputFileSet)
   {
      printUsage (argv0);
      PetscFinalize ();
      return 1;
   }

   char outputFileName[FileNameLength];
   PetscBool outputFileSet;

   PetscOptionsGetString (PETSC_NULL, "-save", outputFileName, FileNameLength, &outputFileSet);
   if (!outputFileSet)
   {
      strcpy (outputFileName, inputFileName);
   }

   PetscBool ddd = PETSC_FALSE;
   PetscOptionsHasName (PETSC_NULL, "-ddd", &ddd);
   if (ddd)
   {

      char cmd[250];

#if 0
      sprintf (cmd, "/glb/home/nlwse0/sweast/Software/gnu/ddd-3.3.12/bin/ddd %s %d &", argv[0],  getpid ());
#else
      sprintf (cmd, "/usr/bin/ddd --debugger /usr/bin/gdb %s %d &", argv[0],  getpid ());
#endif     
 
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

   PetscBool cvd = PETSC_FALSE;
   PetscOptionsHasName (PETSC_NULL, "-cvd", &cvd);
   if (cvd)
   {
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
   
   if( rc != EPTFLEXLM_OK && rc != EPTFLEXLM_WARN)
   {
      #ifdef FLEXLM
      //FlexLM license check in only for node with rank = 0
      if( rank == 0 )
      {
         // FlexLm license check in, close down and enable logging
         EPTFlexLmCheckIn( feature );
         EPTFlexLmTerminate();
      }
      #endif
      // Close PetSc
      PetscFinalize ();

      return 1;
   }
   
   bool status = false;
   
   GenexSimulator          *theGenexSimulator   = 0;
   GenexSimulatorFactory   *theFactory          = 0;
   AdsorptionProjectHandle* adsorptionProjectHandle = 0;


   //create the factory
   theFactory = new GenexSimulatorFactory;

   if(theFactory)
   { 
      DataAccess::Interface::ProjectHandle::UseFactory (theFactory);
      status = true;
   }
   //create the ProjectHandle
   if (status)
   {
      theGenexSimulator = GenexSimulator::CreateFrom (inputFileName);
      status = (theGenexSimulator != 0);
   }
   //delete existent properties
   if (status)
   {
      theGenexSimulator->deleteSourceRockPropertyValues();
      theGenexSimulator->deletePropertyValues (DataAccess::Interface::RESERVOIR , 0, 0,  0,  0, 0, DataAccess::Interface::MAP );
   }

   if ( theGenexSimulator != 0 ) {
      adsorptionProjectHandle = new AdsorptionProjectHandle ( theGenexSimulator );
      theGenexSimulator->setAdsorptionSimulation ( adsorptionProjectHandle );
   }

   //run the simulation
   if (theGenexSimulator) status = theGenexSimulator->run ();
   //save results
   if (status) status = theGenexSimulator->saveTo (outputFileName);
   
   //clean up
   delete theGenexSimulator;
   delete theFactory;

   if ( adsorptionProjectHandle != 0 ) {
      delete adsorptionProjectHandle;
   }
   
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

   return status ? 0 : 1;
}

void printUsage (char * argv0)
{
      PetscPrintf (PETSC_COMM_WORLD, "usage: %s -project fileName [-save fileName]\n", argv0);
}

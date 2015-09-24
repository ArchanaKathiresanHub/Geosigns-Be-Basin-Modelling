#include <assert.h>
#include <stdlib.h>
#include "history.h"
#include "fctcalc.h"
#include "utils.h"
#include "PressureSolver.h"
#include "fem_grid.h"
#include "readproperties.h"
#include "System.h"
#include "Quadrature.h"
#include "globaldefs.h"

#include "AllochthonousLithologyManager.h"
#include "HydraulicFracturingManager.h"

#include "FastcauldronFactory.h"
#include "FastcauldronSimulator.h"
#include "MultiComponentFlowHandler.h"
#include "StatisticsHandler.h"

#include "Interface/Interface.h"
#include "Interface/ProjectHandle.h"
#include "Interface/Formation.h"
#include "Interface/GridMap.h"
#include "Interface/Grid.h"

#include "MemoryChecker.h"
#include "FastcauldronStartup.h"

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


int ourRank ();

void finaliseFastcauldron ( AppCtx* appctx, 
                            char* feature,
                            const char* errorMessage = "",
                            FastcauldronFactory* factory = 0 );


static void abortOnBadAlloc () {
   cerr << " cannot allocate ersources, aborting"  << endl;
   MPI_Abort ( PETSC_COMM_WORLD, 3 );
}


bool determineSaltModellingCapability ( const AppCtx* appctx );

int main(int argc, char** argv)
{
   MemoryChecker mc;
   bool status;
   int returnStatus = 0;
   bool solverHasConverged;
   bool errorInDarcy;
   bool geometryHasConverged;


   // If bad_alloc is raised during an allocation of memory then this function will be called.
   std::set_new_handler ( abortOnBadAlloc );

   // Initialise Petsc and get rank & size of MPI
   PetscInitialize (&argc, &argv, (char *) 0, PETSC_NULL);

   // If FLEXLM is defined then this is updated within the FLEXLM section.
   // The run should not abort if the ibs_cauldron_halo license cannot be 
   // checked-out instead a warning is issued.
   // Some OU's are no permitted to have access to the salt modelling functionality.
   bool canRunSaltModelling = true;

   StartTiming();

   FastcauldronFactory* factory = new FastcauldronFactory;

   AppCtx *appctx = new AppCtx (argc, argv);
   HydraulicFracturingManager::getInstance ().setAppCtx ( appctx );



#ifdef FLEXLM

   int rc = EPTFLEXLM_OK;

   char feature[EPTFLEXLM_MAX_FEATURE_LEN];
   char version[EPTFLEXLM_MAX_VER_LEN];
   char errmessage[EPTFLEXLM_MAX_MESS_LEN];
   
   // FlexLM license handling only for node with rank = 0
   if ( ourRank () == 0 )
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
   canRunSaltModelling = determineSaltModellingCapability ( appctx );

   
   if( rc != EPTFLEXLM_OK &&  rc != EPTFLEXLM_WARN)
   {
     delete appctx;
     FastcauldronSimulator::finalise ( false );
      // Close PetSc
     PetscFinalize ();

     #ifdef FLEXLM
     //FlexLM license check in only for node with rank = 0
     if ( ourRank () == 0 )
     {
       // FlexLm license check in, close down and enable logging
       EPTFlexLmCheckIn( feature );
       EPTFlexLmTerminate();
     }
     #endif
     
     return 1;
   }

#else
   char feature[33];
   sprintf(feature, "ibs_cauldron_calc");
#endif

   std::string errorMessage;
   returnStatus = FastcauldronStartup::startup ( argc, argv, appctx, factory, canRunSaltModelling, errorMessage );

   if ( returnStatus != 0 ) {

      if ( errorMessage != "" ) {
         PetscPrintf ( PETSC_COMM_WORLD, "\n %s \n", errorMessage.c_str ());
      }

      if ( factory != 0 ) {
         delete factory;
      }

      delete appctx;
      FastcauldronSimulator::finalise ( false );
      PetscFinalize ();

#ifdef FLEXLM
      if ( ourRank () == 0 ) {
         EPTFlexLmCheckIn( feature );
         EPTFlexLmTerminate();
      }
#endif

      return returnStatus;
   }


   // Calculate FCT 
   FCTCalc fctCtx (appctx);

   solverHasConverged = true;
   errorInDarcy = false;
   geometryHasConverged = true;

   if ( appctx->DoHighResDecompaction || appctx->DoDecompaction ) {
     fctCtx.decompact ();
   }

   if ( appctx->DoOverPressure && ! appctx -> Do_Iteratively_Coupled ) {
     Basin_Modelling::FEM_Grid basin ( appctx );

     // Do Pressure Calculation
     basin.solvePressure ( solverHasConverged, errorInDarcy, geometryHasConverged );
   }

   if ( appctx->DoTemperature && ! appctx -> Do_Iteratively_Coupled ) {
     Basin_Modelling::FEM_Grid basin ( appctx );

     // Do Tempearature Calculation
     basin.solveTemperature ( solverHasConverged, errorInDarcy );
   }

   if ( appctx -> Do_Iteratively_Coupled ) {
     Basin_Modelling::FEM_Grid basin ( appctx );
     // Do Coupled Calculation
     basin.solveCoupled ( solverHasConverged, errorInDarcy, geometryHasConverged );
   }

   if ( appctx->integrateGenexEquations ()) {
      FastcauldronSimulator::getInstance ().saveGenexHistory ();
   }

   /// Delete all the arrays used to store the quadrature points and weights.
   /// They will not be used from this point onwards.
   NumericFunctions::Quadrature::finaliseQuadrature ();

   if (( not appctx->saveOnDarcyError () and errorInDarcy ) or not solverHasConverged ) {
     returnStatus = 1;
#if 0
   } else if ( not geometryHasConverged ) {
     returnStatus = GeometricLoopNonConvergenceExitStatus;
#endif
   } else {
     returnStatus = 0;
   }
   
    if( ! FastcauldronSimulator::getInstance ().mergeOutputFiles ()) {
       PetscPrintf ( PETSC_COMM_WORLD, "  MeSsAgE ERROR Unable to merge output files\n");

       returnStatus = 1;
    }
   
   StatisticsHandler::print ();
   FastcauldronSimulator::finalise ( solverHasConverged and ( appctx->saveOnDarcyError () or not errorInDarcy ));
   bool displayEndTime = appctx->debug1 or appctx->verbose;
   delete appctx;

   // Close PetSc
   PetscFinalize ();
   displayTime( displayEndTime, "End of simulation: ");

   //FlexLM license check in only for node with rank = 0
#ifdef FLEXLM
   if ( ourRank () == 0 )
   {
      // FlexLm license check in, close down and enable logging
      EPTFlexLmCheckIn( feature );
      EPTFlexLmTerminate();
   }
#endif

   return returnStatus;
}



bool determineSaltModellingCapability ( const AppCtx* appctx ) {

  int capable = 1;

#ifdef FLEXLM

  int rc = EPTFLEXLM_OK;

  char feature[EPTFLEXLM_MAX_FEATURE_LEN];
  char version[EPTFLEXLM_MAX_VER_LEN];
  char errmessage[EPTFLEXLM_MAX_MESS_LEN];
   
  // FlexLM license handling only for node with rank = 0
  if ( ourRank () == 0 ) {
    sprintf(feature, "ibs_cauldron_halo");
#ifdef IBSFLEXLMVERSION
    sprintf(version, IBSFLEXLMVERSION);
#else
    sprintf(version, "9999.99");
#endif


    rc = EPTFlexLmInit(errmessage);

    if ( rc != EPTFLEXLM_OK ) {
      cout << endl << "@@@@@@@@@@@@@@@\n MeSsAgE ERROR: FlexLm license init problems: fastcauldron cannot start.\n Please contact your helpdesk\n@@@@@@@@@@@@@@@" << endl;
      capable = 0;
    } else {

      // FlexLM license handling: Checkout
      rc = EPTFlexLmCheckOut( feature, version, errmessage );

      if (rc == EPTFLEXLM_WARN) {
        cout << endl << "@@@@@@@@@@@@@@@\n MeSsAgE WARNING: fastcauldron will still start the salt modelling capabilities anyway.\n@@@@@@@@@@@@@@@" << endl;
        capable = 1;
      } else if ( rc != EPTFLEXLM_OK ) {
        cout << endl << "@@@@@@@@@@@@@@@\n MeSsAgE WARNING: FlexLm license error: fastcauldron cannot start the salt modelling capabilities.\n Please contact your helpdesk.\n@@@@@@@@@@@@@@@" << endl;
        capable = 0;
      } else {
        capable = 1;
      }

    }

  }  

  MPI_Bcast ( &capable, 1, MPI_INT, 0, PETSC_COMM_WORLD);

  if ( ourRank () == 0 && rc != EPTFLEXLM_WARN && rc != EPTFLEXLM_OK ) {
    EPTFlexLmCheckIn( feature );
    EPTFlexLmTerminate();
  }

  #endif


  return capable == 1;
}

int ourRank () {

   static int myRank;
   static bool haveRank = false;

   if ( not haveRank ) {
      MPI_Comm_rank ( PETSC_COMM_WORLD, &myRank );
      haveRank = true;
   }

   return myRank;
}


void finaliseFastcauldron ( AppCtx* appctx, 
                            char* feature,
                            const char* errorMessage,
                            FastcauldronFactory* factory ) {

   if ( strlen ( errorMessage ) > 0 ) {
      PetscPrintf ( PETSC_COMM_WORLD,
                    "\n %s \n", errorMessage );
   }

   if ( factory != 0 ) {
      delete factory;
   }

   delete appctx;
   FastcauldronSimulator::finalise ( false );
   PetscFinalize ();
}

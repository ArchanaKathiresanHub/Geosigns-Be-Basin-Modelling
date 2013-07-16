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

#include "AllochthonousLithologyManager.h"
#include "HydraulicFracturingManager.h"

#include <sys/resource.h>

#include "FastcauldronFactory.h"
#include "FastcauldronSimulator.h"
#include "MultiComponentFlowHandler.h"
#include "StatisticsHandler.h"

#include "Interface/Interface.h"
#include "Interface/ProjectHandle.h"
#include "Interface/Formation.h"
#include "Interface/GridMap.h"
#include "Interface/Grid.h"

#include <sched.h>

#ifdef FLEXLM
#undef FLEXLM
#endif

// #define FLEXLM  1

#ifdef FLEXLM
// FlexLM license handling
#include <EPTFlexLm.h>
#endif


int rank ();

void finaliseFastcauldron ( AppCtx* appctx, 
                            char* feature,
                            const char* errorMessage = "",
                            FastcauldronFactory* factory = 0 ) {

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

#ifdef FLEXLM
   if ( rank () == 0 ) {
      EPTFlexLmCheckIn( feature );
      EPTFlexLmTerminate();
   }
#endif

}


bool determineSaltModellingCapability ( const AppCtx* appctx );

int main(int argc, char** argv)
{
   bool status;
   int returnStatus = 0;
   bool solverHasConverged;
   bool errorInDarcy;

   // Initialise Petsc and get rank & size of MPI
   PetscInitialize (&argc, &argv, (char *) 0, PETSC_NULL);

   // If FLEXLM is defined then this is updated within the FLEXLM section.
   // The run should not abort if the ibs_cauldron_halo license cannot be 
   // checked-out instead a warning is issued.
   // Some OU's are no permitted to have access to the salt modelling functionality.
   bool canRunSaltModelling = true;

   StartTiming();

   FastcauldronFactory* factory = new FastcauldronFactory;
   DataAccess::Interface::ProjectHandle::UseFactory (factory);

   AppCtx *appctx = new AppCtx (argc, argv);
   HydraulicFracturingManager::getInstance ().setAppCtx ( appctx );

   string processId = IntegerToString ( GetProcPID ());



#ifdef FLEXLM

   int rc = EPTFLEXLM_OK;

   char feature[EPTFLEXLM_MAX_FEATURE_LEN];
   char version[EPTFLEXLM_MAX_VER_LEN];
   char errmessage[EPTFLEXLM_MAX_MESS_LEN];
   
   // FlexLM license handling only for node with rank = 0
   if ( rank () == 0 )
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
     if ( rank () == 0 )
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


   if ( not appctx->readProjectName ()) {
     finaliseFastcauldron ( appctx, feature, "MeSsAgE ERROR Error when reading the project file", factory );
     return 1;
   }

   StatisticsHandler::initialise ();
   FastcauldronSimulator::CreateFrom ( appctx );

   FastcauldronSimulator::getInstance ().setFormationElementHeightScalingFactors ();

   if ( not FastcauldronSimulator::getInstance ().setCalculationMode ( appctx->getCalculationMode (),
                                                                       appctx->getUnitTestNumber () == 1 )) {
     finaliseFastcauldron ( appctx, feature, "MeSsAgE ERROR Error when setting calculation mode", factory );
     return 1;
   }

   if ( appctx->getUnitTestNumber () == 1 ) {
     finaliseFastcauldron ( appctx, feature, "MeSsAgE INFO Completed unit test 1.", factory );
     return 0;
   }

   FastcauldronSimulator::getInstance ().initialiseFastcauldronLayers ();
   FastcauldronSimulator::getInstance ().getMcfHandler ().determineUsage ();

   if ( not appctx->readProjectFile ()) {
     finaliseFastcauldron ( appctx, feature, "MeSsAgE ERROR Error when reading the project file", factory );
     return 1;
   }

   // There are several command line parameters that can be set only after the project file has been read.
   FastcauldronSimulator::getInstance ().readCommandLineParameters ( argc, argv );

   // Initialise anything that is to be set from the environment.
   appctx->setParametersFromEnvironment ();

#if 0
   appctx->Switch_To_Low_Resolution_Maps ();
#endif

   appctx->Display_Grid_Description();

   Filterwizard *fw = &appctx->filterwizard;

   appctx->setLayerBottSurfaceName ();

   // Process Data Map and Assess Valid Nodes
   appctx->setValidNodeArray ();

#if 0
   FastcauldronSimulator::getInstance ().initialiseElementGrid ( appctx->debug2 );
#endif

   appctx->Examine_Load_Balancing ();
   appctx->Output_Number_Of_Geological_Events();

#if 0
   if ( not appctx->Determine_Maximum_Crust_Thinning_Ratio ()) {
      finaliseFastcauldron ( appctx, feature, "MeSsAgE ERROR Error determining crust thinning history.", factory );
      return 1;
   }
#else
   // cout << " CHeck the crust thinning ration functionality." << endl;
#endif

   if ( ! appctx->createFormationLithologies ( canRunSaltModelling )) {

     PetscPrintf ( PETSC_COMM_WORLD,
                   "\n---------------- Unable to create lithologies ----------------\n");
     delete appctx;
     FastcauldronSimulator::finalise ( false );

     // Close PetSc
     PetscFinalize ();

     #ifdef FLEXLM
     //FlexLM license check in only for node with rank = 0
     if ( rank () == 0 )
     {
       // FlexLm license check in, close down and enable logging
       EPTFlexLmCheckIn( feature );
       EPTFlexLmTerminate();
     }
     #endif

     return 1;
   }

   // Find which derived properties are required
   fw->InitDerivedCalculationsNeeded ();

   appctx->Locate_Related_Project ();
   appctx->setInitialTimeStep ();
   FastcauldronSimulator::getInstance ().getMcfHandler ().initialise ();
   FastcauldronSimulator::getInstance ().updateSourceRocksForDarcy ();
   // Must be done after updating source-rocks for Darcy, since this disables adsorption.
   FastcauldronSimulator::getInstance ().updateSourceRocksForGenex ();

   // Now that every thing has been loaded, we can correct the property lists:
   //     o Property list;
   //     o PropertyValue list;
   //     o OutputPropertyList;
   //
   // And any associations between the objects can be made.
   FastcauldronSimulator::getInstance ().correctAllPropertyLists ();
   FastcauldronSimulator::getInstance ().updateSnapshotFileCreationFlags ();

   const bool overpressureCalculation = FastcauldronSimulator::getInstance ().getCalculationMode () == OVERPRESSURE_MODE or
                                        FastcauldronSimulator::getInstance ().getCalculationMode () == OVERPRESSURED_TEMPERATURE_MODE or
                                        FastcauldronSimulator::getInstance ().getCalculationMode () == COUPLED_HIGH_RES_DECOMPACTION_MODE or
                                        FastcauldronSimulator::getInstance ().getCalculationMode () == PRESSURE_AND_TEMPERATURE_MODE or
                                        FastcauldronSimulator::getInstance ().getCalculationMode () == COUPLED_DARCY_MODE;

   if ( not FastcauldronSimulator::getInstance ().initialiseLayerThicknessHistory ( overpressureCalculation )) {
     finaliseFastcauldron ( appctx, feature, "MeSsAgE ERROR when initialising thickness history.", factory );
     return 1;
   }

   if ( FastcauldronSimulator::getInstance ().getCalculationMode () == OVERPRESSURED_TEMPERATURE_MODE or
        FastcauldronSimulator::getInstance ().getCalculationMode () == COUPLED_HIGH_RES_DECOMPACTION_MODE ) {

      // Scale the initalised solid-thicknesses by the fct-correction factors.
      FastcauldronSimulator::getInstance ().applyFctCorrections ();
   }


   // Calculate FCT 
   FCTCalc fctCtx (appctx);

   solverHasConverged = true;
   errorInDarcy = false;

#if 0
   if ( appctx->getCalculationMode () == HYDROSTATIC_DECOMPACTION_MODE or
        appctx->getCalculationMode () == HYDROSTATIC_HIGH_RES_DECOMPACTION_MODE or
        appctx->getCalculationMode () == COUPLED_HIGH_RES_DECOMPACTION_MODE ) {
     fctCtx.decompact ();
   }

   if ( appctx->getCalculationMode () == HYDROSTATIC_TEMPERATURE_MODE or
        appctx->getCalculationMode () == OVERPRESSURE_MODE or
        appctx->getCalculationMode () == OVERPRESSURED_TEMPERATURE_MODE or
        appctx->getCalculationMode () == COUPLED_HIGH_RES_DECOMPACTION_MODE or
        appctx->getCalculationMode () == PRESSURE_AND_TEMPERATURE_MODE ) {

      Basin_Modelling::FEM_Grid basin ( appctx );

      switch ( appctx->getCalculationMode ()) {

        case OVERPRESSURE_MODE:
           basin.solvePressure ( solverHasConverged, errorInDarcy );
           break;

        case HYDROSTATIC_TEMPERATURE_MODE:
        case OVERPRESSURED_TEMPERATURE_MODE:
           basin.solveTemperature ( solverHasConverged, errorInDarcy );
           break;

        case PRESSURE_AND_TEMPERATURE_MODE:
           basin.solveCoupled ( solverHasConverged, errorInDarcy );
           break;

      }


   }
#endif

   if ( appctx->DoHighResDecompaction || appctx->DoDecompaction ) {
     fctCtx.decompact ();
   }

   if ( appctx->DoOverPressure && ! appctx -> Do_Iteratively_Coupled ) {
     Basin_Modelling::FEM_Grid basin ( appctx );

     // Do Pressure Calculation
     basin.solvePressure ( solverHasConverged, errorInDarcy );
   }

   if ( appctx->DoTemperature && ! appctx -> Do_Iteratively_Coupled ) {
     Basin_Modelling::FEM_Grid basin ( appctx );

     // Do Tempearature Calculation
     basin.solveTemperature ( solverHasConverged, errorInDarcy );
   }

   if ( appctx -> Do_Iteratively_Coupled ) {
     Basin_Modelling::FEM_Grid basin ( appctx );
     // Do Coupled Calculation
     basin.solveCoupled ( solverHasConverged, errorInDarcy );
   }

   if ( appctx->integrateGenexEquations ()) {
      FastcauldronSimulator::getInstance ().saveGenexHistory ();
   }

   /// Delete all the arrays used to store the quadrature points and weights.
   /// They will not be used from this point onwards.
   NumericFunctions::Quadrature::finaliseQuadrature ();

   appctx -> setRunStatus ( "OutputDirOfLastRun", appctx -> getOutputDirectory () );

   if (( not appctx->saveOnDarcyError () and errorInDarcy ) or not solverHasConverged ) {
     returnStatus = 1;
   } else {
     returnStatus = 0;
   }

   StatisticsHandler::print ();
   FastcauldronSimulator::finalise ( solverHasConverged and ( appctx->saveOnDarcyError () or not errorInDarcy ));
   delete appctx;

   // Close PetSc
   PetscFinalize ();
   displayTime(appctx->debug1 or appctx->verbose,"End of simulation: ");

   #ifdef FLEXLM
   //FlexLM license check in only for node with rank = 0
   if ( rank () == 0 )
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
  if ( rank () == 0 ) {
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

  if ( rank () == 0 && rc != EPTFLEXLM_WARN && rc != EPTFLEXLM_OK ) {
    EPTFlexLmCheckIn( feature );
    EPTFlexLmTerminate();
  }

  #endif


  return capable == 1;
}

int rank () {

   static int myRank;
   static bool haveRank = false;

   if ( not haveRank ) {
      MPI_Comm_rank ( PETSC_COMM_WORLD, &myRank );
      haveRank = true;
   }

   return myRank;
}

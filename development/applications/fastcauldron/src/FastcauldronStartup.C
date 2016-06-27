//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "FastcauldronStartup.h"
#include "FastcauldronSimulator.h"
#include "MultiComponentFlowHandler.h"
#include "StatisticsHandler.h"
#include "filterwizard.h"
#include "globaldefs.h"
#include "PressureSolver.h"
#include "readproperties.h"
#include "System.h"

//DataAccess library
#include "Interface/Interface.h"
#include "Interface/ProjectHandle.h"
#include "Interface/Formation.h"
#include "Interface/GridMap.h"
#include "Interface/Grid.h"

#include "fctcalc.h"
#include "DerivedPropertiesCalculator.h"
#include "fem_grid.h"
#include "Quadrature.h"
#include "utils.h"
#include "FastcauldronFactory.h"
#include "MultiComponentFlowHandler.h"

#include "AllochthonousLithologyManager.h"
#include "HydraulicFracturingManager.h"
#include "propinterface.h"

// inizialize the static members
AppCtx * FastcauldronStartup::s_cauldron = 0;
FastcauldronFactory* FastcauldronStartup::s_factory = 0;
std::string FastcauldronStartup::s_errorMessage = "";
bool FastcauldronStartup::s_solverHasConverged = false;
bool FastcauldronStartup::s_errorInDarcy = false;
bool FastcauldronStartup::s_geometryHasConverged = false;
#ifdef FLEXLM
char FastcauldronStartup::s_feature[EPTFLEXLM_MAX_FEATURE_LEN];
#else
char FastcauldronStartup::s_feature[256];
#endif

int FastcauldronStartup::prepare( bool canRunSaltModelling )
{

   // If FLEXLM is defined then this is updated within the FLEXLM section.
   // The run should not abort if the ibs_cauldron_halo license cannot be 
   // checked-out instead a warning is issued.
   // Some OU's are no permitted to have access to the salt modelling functionality.

#ifdef FLEXLM

   int rc = EPTFLEXLM_OK;
   char version[EPTFLEXLM_MAX_VER_LEN];
   char errmessage[EPTFLEXLM_MAX_MESS_LEN];

   // FlexLM license handling only for node with rank = 0
   if ( ourRank( ) == 0 )
   {
      sprintf( s_feature, "ibs_cauldron_calc" );
#ifdef IBSFLEXLMVERSION
      sprintf( version, IBSFLEXLMVERSION );
#else
      sprintf( version, "9999.99" );
#endif
      rc = EPTFlexLmInit( errmessage );
      if ( rc != EPTFLEXLM_OK )
      {
         fprintf( stderr, "\n@@@@@@@@@@@@@@@\n FlexLm license init problems: fastcauldron cannot start.\n Please contact your helpdesk\n@@@@@@@@@@@@@@@\n" );
      }
      // FlexLM license handling: Checkout
      rc = EPTFlexLmCheckOut( s_feature, version, errmessage );
      if ( rc == EPTFLEXLM_WARN )
      {
         fprintf( stderr, "\n@@@@@@@@@@@@@@@\n FlexLm license warning: fastcauldron will still start anyway.\n@@@@@@@@@@@@@@@\n" );
      }
      else if ( rc != EPTFLEXLM_OK )
      {
         fprintf( stderr, "\n@@@@@@@@@@@@@@@\n FlexLm license error: fastcauldron cannot start.\n Please contact your helpdesk\n@@@@@@@@@@@@@@@\n" );
      }
   }
   MPI_Bcast( &rc, 1, MPI_INT, 0, PETSC_COMM_WORLD );
   canRunSaltModelling = determineSaltModellingCapability( );


   if ( rc != EPTFLEXLM_OK &&  rc != EPTFLEXLM_WARN )
   {
      return 1;
   }

#else
   sprintf( s_feature, "ibs_cauldron_calc" );
#endif


   return 0;
}

int FastcauldronStartup::startup( int                  argc,
   char**               argv,
   const bool           canRunSaltModelling,
   const bool           saveAsInputGrid,
   const bool           createResultsFile )
{
   s_factory = new FastcauldronFactory;

   s_cauldron = new AppCtx( argc, argv );

   HydraulicFracturingManager::getInstance( ).setAppCtx( s_cauldron );

   StartTiming( );

   if ( not s_cauldron->readProjectName() ) {
      s_errorMessage = "MeSsAgE ERROR Error when reading the project file";
      return 1;
   }

   StatisticsHandler::initialise();
   FastcauldronSimulator::CreateFrom( s_cauldron, s_factory );
   FastcauldronSimulator::getInstance().readCommandLineParametersEarlyStage( argc, argv );
   FastcauldronSimulator::getInstance().deleteTemporaryDirSnapshots();
   FastcauldronSimulator::getInstance().setFormationElementHeightScalingFactors();

   if ( not FastcauldronSimulator::getInstance ().setCalculationMode( s_cauldron->getCalculationMode( ), saveAsInputGrid, createResultsFile ) ) {
      s_errorMessage = "MeSsAgE ERROR Error when setting calculation mode";
      return 1;
   }

   FastcauldronSimulator::getInstance().getMcfHandler().determineUsage();
   FastcauldronSimulator::getInstance().initialiseFastcauldronLayers();

   if ( not s_cauldron->readProjectFile()) {
      s_errorMessage = "MeSsAgE ERROR Error when reading the project file";
      return 1;
   }

   // There are several command line parameters that can be set only after the project file has been read.
   FastcauldronSimulator::getInstance().readCommandLineParametersLateStage( argc, argv );

   // Initialise anything that is to be set from the environment.
   s_cauldron->setParametersFromEnvironment();
   s_cauldron->Display_Grid_Description();
   s_cauldron->setLayerBottSurfaceName();

   // Process Data Map and Assess Valid Nodes
   s_cauldron->setValidNodeArray();
   s_cauldron->Examine_Load_Balancing();
   s_cauldron->Output_Number_Of_Geological_Events();

   if ( not s_cauldron->createFormationLithologies ( canRunSaltModelling )) {
      s_errorMessage = "MeSsAgE ERROR Unable to create lithologies";
      return 1;
   }

   // Find which derived properties are required
   s_cauldron->filterwizard.InitDerivedCalculationsNeeded( );

   s_cauldron->Locate_Related_Project( );
   s_cauldron->setInitialTimeStep( );
   FastcauldronSimulator::getInstance( ).getMcfHandler( ).initialise( );
   FastcauldronSimulator::getInstance( ).updateSourceRocksForDarcy( );
   // Must be done after updating source-rocks for Darcy, since this disables adsorption.
   FastcauldronSimulator::getInstance( ).updateSourceRocksForGenex( );

   // Now that every thing has been loaded, we can correct the property lists:
   //     o Property list;
   //     o PropertyValue list;
   //     o OutputPropertyList;
   //
   // And any associations between the objects can be made.
   FastcauldronSimulator::getInstance( ).correctAllPropertyLists( );
   FastcauldronSimulator::getInstance( ).updateSnapshotFileCreationFlags( );

   const bool overpressureCalculation = FastcauldronSimulator::getInstance().getCalculationMode() == OVERPRESSURE_MODE or
                                        FastcauldronSimulator::getInstance().getCalculationMode() == OVERPRESSURED_TEMPERATURE_MODE or
                                        FastcauldronSimulator::getInstance().getCalculationMode() == COUPLED_HIGH_RES_DECOMPACTION_MODE or
                                        FastcauldronSimulator::getInstance().getCalculationMode() == PRESSURE_AND_TEMPERATURE_MODE or
                                        FastcauldronSimulator::getInstance().getCalculationMode() == COUPLED_DARCY_MODE;


   if ( not FastcauldronSimulator::getInstance( ).initialiseLayerThicknessHistory( overpressureCalculation ) ) {
      s_errorMessage = "MeSsAgE ERROR when initialising thickness history.";
      return 1;
   }

   if ( FastcauldronSimulator::getInstance( ).getCalculationMode( ) == OVERPRESSURED_TEMPERATURE_MODE or
      FastcauldronSimulator::getInstance( ).getCalculationMode( ) == COUPLED_HIGH_RES_DECOMPACTION_MODE ) {

      // Scale the initalised solid-thicknesses by the fct-correction factors.
      FastcauldronSimulator::getInstance( ).applyFctCorrections( );
   }

   s_errorMessage = "";
   return 0;
}


int FastcauldronStartup::run( )
{
   int returnStatus;
   s_solverHasConverged = true;
   s_errorInDarcy = false;
   s_geometryHasConverged = true;

   // Calculate FCT 
   FCTCalc fctCtx( s_cauldron );

   if ( s_cauldron->DoHighResDecompaction || s_cauldron->DoDecompaction ) {

      fctCtx.decompact( );

      if ( not FastcauldronSimulator::getInstance( ).noDerivedPropertiesCalc( ) ) {
         DerivedPropertiesCalculator propertyCalculator( s_cauldron, fctCtx.getVolumeOutputProperties( ), fctCtx.getMapOutputProperties( ) );

         propertyCalculator.compute( );
      }
   }

   if ( s_cauldron->DoOverPressure && !s_cauldron->Do_Iteratively_Coupled ) {
      Basin_Modelling::FEM_Grid basin( s_cauldron );

      // Do Pressure Calculation
      basin.solvePressure( s_solverHasConverged, s_errorInDarcy, s_geometryHasConverged );
   }

   if ( s_cauldron->DoTemperature && !s_cauldron->Do_Iteratively_Coupled ) {
      Basin_Modelling::FEM_Grid basin( s_cauldron );

      // Do Tempearature Calculation
      basin.solveTemperature( s_solverHasConverged, s_errorInDarcy );

      if ( not FastcauldronSimulator::getInstance( ).noDerivedPropertiesCalc( ) ) {
         DerivedPropertiesCalculator propertyCalculator( s_cauldron, basin.getVolumeOutputProperties( ), basin.getMapOutputProperties( ) );

         propertyCalculator.compute( );
      }
   }

   if ( s_cauldron->Do_Iteratively_Coupled ) {
      Basin_Modelling::FEM_Grid basin( s_cauldron );
      // Do Coupled Calculation
      basin.solveCoupled( s_solverHasConverged, s_errorInDarcy, s_geometryHasConverged );

      if ( not FastcauldronSimulator::getInstance( ).noDerivedPropertiesCalc( ) ) {
         DerivedPropertiesCalculator propertyCalculator( s_cauldron, basin.getVolumeOutputProperties( ), basin.getMapOutputProperties( ) );

         propertyCalculator.compute( );
      }
   }

   if ( s_cauldron->integrateGenexEquations( ) ) {
      FastcauldronSimulator::getInstance( ).saveGenexHistory( );
   }

   /// Delete all the arrays used to store the quadrature points and weights.
   /// They will not be used from this point onwards.
   NumericFunctions::Quadrature::finaliseQuadrature( );

   if ( ( not s_cauldron->saveOnDarcyError( ) and s_errorInDarcy ) or not s_solverHasConverged ) {
      returnStatus = 1;
#if 0
   }
   else if ( not s_geometryHasConverged ) {
      returnStatus = GeometricLoopNonConvergenceExitStatus;
#endif
   }
   else {
      returnStatus = 0;
   }

   if ( !FastcauldronSimulator::getInstance( ).mergeOutputFiles( ) ) {
      PetscPrintf( PETSC_COMM_WORLD, "  MeSsAgE ERROR Unable to merge output files\n" );

      returnStatus = 1;
   }

   StatisticsHandler::print( );

   return returnStatus;
}

int FastcauldronStartup::finalise( bool returnStatus )
{
   if ( returnStatus != 0 )
   {
      if ( s_errorMessage != "" ) {
         PetscPrintf( PETSC_COMM_WORLD, "\n %s \n", s_errorMessage.c_str( ) );
      }

      if ( s_factory != 0 )
      {
         delete s_factory;
      }

      delete s_cauldron;

      FastcauldronSimulator::finalise( false );

      //FlexLM license check in only for node with rank = 0
#ifdef FLEXLM
      if ( ourRank( ) == 0 )
      {
         // FlexLm license check in, close down and enable logging
         EPTFlexLmCheckIn( s_feature );
         EPTFlexLmTerminate( );
      }
#endif
      return 1;
   }

   if ( s_errorMessage != "" )
   {
      PetscPrintf( PETSC_COMM_WORLD, "\n %s \n", s_errorMessage.c_str( ) );
   }

   FastcauldronSimulator::finalise( s_solverHasConverged and( s_cauldron->saveOnDarcyError( ) or not s_errorInDarcy ) );

   if ( s_factory != 0 )
   {
      delete s_factory;
   }

   bool displayEndTime = s_cauldron->debug1 or s_cauldron->verbose;

   delete s_cauldron;

   // display
   displayTime( displayEndTime, "End of simulation: " );

   //FlexLM license check in only for node with rank = 0
#ifdef FLEXLM
   if ( ourRank( ) == 0 )
   {
      // FlexLm license check in, close down and enable logging
      EPTFlexLmCheckIn( s_feature );
      EPTFlexLmTerminate( );
   }
#endif
   return 0;
}

bool FastcauldronStartup::determineSaltModellingCapability( ) {

   int capable = 1;

#ifdef FLEXLM

   int rc = EPTFLEXLM_OK;

   char version[EPTFLEXLM_MAX_VER_LEN];
   char errmessage[EPTFLEXLM_MAX_MESS_LEN];

   // FlexLM license handling only for node with rank = 0
   if ( ourRank( ) == 0 ) {
      sprintf( s_feature, "ibs_cauldron_halo" );
#ifdef IBSFLEXLMVERSION
      sprintf( version, IBSFLEXLMVERSION );
#else
      sprintf( version, "9999.99" );
#endif

      rc = EPTFlexLmInit( errmessage );

      if ( rc != EPTFLEXLM_OK ) {
         cout << endl << "@@@@@@@@@@@@@@@\n MeSsAgE ERROR: FlexLm license init problems: fastcauldron cannot start.\n Please contact your helpdesk\n@@@@@@@@@@@@@@@" << endl;
         capable = 0;
      }
      else {

         // FlexLM license handling: Checkout
         rc = EPTFlexLmCheckOut( s_feature, version, errmessage );

         if ( rc == EPTFLEXLM_WARN ) {
            cout << endl << "@@@@@@@@@@@@@@@\n MeSsAgE WARNING: fastcauldron will still start the salt modelling capabilities anyway.\n@@@@@@@@@@@@@@@" << endl;
            capable = 1;
         }
         else if ( rc != EPTFLEXLM_OK ) {
            cout << endl << "@@@@@@@@@@@@@@@\n MeSsAgE WARNING: FlexLm license error: fastcauldron cannot start the salt modelling capabilities.\n Please contact your helpdesk.\n@@@@@@@@@@@@@@@" << endl;
            capable = 0;
         }
         else {
            capable = 1;
         }

      }

   }

   MPI_Bcast( &capable, 1, MPI_INT, 0, PETSC_COMM_WORLD );

   if ( ourRank( ) == 0 && rc != EPTFLEXLM_WARN && rc != EPTFLEXLM_OK )
   {
      EPTFlexLmCheckIn( s_feature );
      EPTFlexLmTerminate( );
   }

#endif

   return capable == 1;
}

int FastcauldronStartup::ourRank( ) {

   static int myRank;
   static bool haveRank = false;

   if ( not haveRank ) {
      MPI_Comm_rank( PETSC_COMM_WORLD, &myRank );
      haveRank = true;
   }

   return myRank;
}

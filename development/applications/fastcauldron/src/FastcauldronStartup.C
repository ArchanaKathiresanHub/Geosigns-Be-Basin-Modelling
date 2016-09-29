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

unsigned int FastcauldronStartup::s_instances = 0;

FastcauldronStartup::FastcauldronStartup( int argc, char** argv, bool checkLicense, bool saveResults ) :
                                          m_checkLicense( checkLicense ),
                                          m_prepareOk( false ),
                                          m_startUpOk( false ),
                                          m_saveResults( saveResults )
{
   if ( s_instances == 0 )
   {
      // Will be updated in prepare function, if FlexLM has been enabled.
      m_canRunSaltModelling    = true;
      m_prepareOk              = prepare();
      m_factory                = new FastcauldronFactory;
      m_cauldron               = new AppCtx( argc, argv );
      m_solverHasConverged     = true;
      m_errorInDarcy           = false;
      m_geometryHasConverged   = true;
      m_runOk                  = true;
      s_instances              = s_instances + 1;
      m_startUpOk              = startup( argc, argv );
   }
   else
   {
      if ( s_instances != 0 ) m_errorMessage = "MeSsAgE ERROR FastcauldronStartup already instantiated";
   }
};

FastcauldronStartup::~FastcauldronStartup( )
{
   //No action taken by the destructor. We need to make sure that factories and other classes are deleted before PetscFinalize and not after by automatic destruction
}

bool FastcauldronStartup::prepare()
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
   if ( ourRank() == 0 )
   {
      sprintf( m_feauture, "ibs_cauldron_calc" );
#ifdef IBSFLEXLMVERSION
      sprintf( version, IBSFLEXLMVERSION );
#else
      sprintf( version, "9999.99" );
#endif
      if ( m_checkLicense )
      {
         rc = EPTFlexLmInit( errmessage );
         if ( rc != EPTFLEXLM_OK )
         {
            fprintf( stderr, "\n@@@@@@@@@@@@@@@\n FlexLm license init problems: fastcauldron cannot start.\n Please contact your helpdesk\n@@@@@@@@@@@@@@@\n" );
         }
         // FlexLM license handling: Checkout
         rc = EPTFlexLmCheckOut( m_feauture, version, errmessage );
         if ( rc == EPTFLEXLM_WARN )
         {
            fprintf( stderr, "\n@@@@@@@@@@@@@@@\n FlexLm license warning: fastcauldron will still start anyway.\n@@@@@@@@@@@@@@@\n" );
         }
         else if ( rc != EPTFLEXLM_OK )
         {
            fprintf( stderr, "\n@@@@@@@@@@@@@@@\n FlexLm license error: fastcauldron cannot start.\n Please contact your helpdesk\n@@@@@@@@@@@@@@@\n" );
         }
      }
      else { sprintf( m_feauture, "ibs_cauldron_calc" ); }
   }
   MPI_Bcast( &rc, 1, MPI_INT, 0, PETSC_COMM_WORLD );
   m_canRunSaltModelling = m_checkLicense ? determineSaltModellingCapability() : true;

   if ( m_checkLicense && rc != EPTFLEXLM_OK &&  rc != EPTFLEXLM_WARN )
   {
      m_errorMessage = "MeSsAgE ERROR FastcauldronStartup::prepare() failed";
      return false;
   }

#else
   sprintf( m_feauture, "ibs_cauldron_calc" );
#endif

   return true;
}

bool FastcauldronStartup::startup( int        argc,
                                   char**     argv,
                                   const bool saveAsInputGrid,
                                   const bool createResultsFile )
{
   HydraulicFracturingManager::getInstance().setAppCtx( m_cauldron );

   StartTiming();

   if ( not m_cauldron->readProjectName() )
   {
      m_errorMessage = "MeSsAgE ERROR Error when reading the project file";
      return false;
   }

   StatisticsHandler::initialise();
   FastcauldronSimulator::CreateFrom( m_cauldron, m_factory );
   FastcauldronSimulator::getInstance().readCommandLineParametersEarlyStage( argc, argv );
   FastcauldronSimulator::getInstance().deleteTemporaryDirSnapshots();
   FastcauldronSimulator::getInstance().setFormationElementHeightScalingFactors();

   if ( not FastcauldronSimulator::getInstance().setCalculationMode( m_cauldron->getCalculationMode(), saveAsInputGrid, createResultsFile ) )
   {
      m_errorMessage = "MeSsAgE ERROR Error when setting calculation mode";
      return false;
   }

   FastcauldronSimulator::getInstance().getMcfHandler().determineUsage();
   FastcauldronSimulator::getInstance().initialiseFastcauldronLayers();

   if ( not m_cauldron->readProjectFile() )
   {
      m_errorMessage = "MeSsAgE ERROR Error when reading the project file";
      return false;
   }

   // There are several command line parameters that can be set only after the project file has been read.
   FastcauldronSimulator::getInstance().readCommandLineParametersLateStage( argc, argv );

   // Initialise anything that is to be set from the environment.
   m_cauldron->setParametersFromEnvironment();
   m_cauldron->Display_Grid_Description();
   m_cauldron->setLayerBottSurfaceName();

   // Process Data Map and Assess Valid Nodes
   m_cauldron->setValidNodeArray();
   m_cauldron->Examine_Load_Balancing();
   m_cauldron->Output_Number_Of_Geological_Events();

   if ( not m_cauldron->createFormationLithologies( m_canRunSaltModelling ) )
   {
      m_errorMessage = "MeSsAgE ERROR Unable to create lithologies";
      return false;
   }

   // Find which derived properties are required
   m_cauldron->filterwizard.InitDerivedCalculationsNeeded();

   m_cauldron->Locate_Related_Project();
   m_cauldron->setInitialTimeStep();
   FastcauldronSimulator::getInstance().getMcfHandler().initialise();
   FastcauldronSimulator::getInstance().updateSourceRocksForDarcy();
   // Must be done after updating source-rocks for Darcy, since this disables adsorption.
   FastcauldronSimulator::getInstance().updateSourceRocksForGenex();

   // Now that every thing has been loaded, we can correct the property lists:
   //     o Property list;
   //     o PropertyValue list;
   //     o OutputPropertyList;
   //
   // And any associations between the objects can be made.
   FastcauldronSimulator::getInstance().correctAllPropertyLists();
   FastcauldronSimulator::getInstance().updateSnapshotFileCreationFlags();

   const bool overpressureCalculation = FastcauldronSimulator::getInstance().getCalculationMode() == OVERPRESSURE_MODE or
      FastcauldronSimulator::getInstance().getCalculationMode() == OVERPRESSURED_TEMPERATURE_MODE or
      FastcauldronSimulator::getInstance().getCalculationMode() == COUPLED_HIGH_RES_DECOMPACTION_MODE or
      FastcauldronSimulator::getInstance().getCalculationMode() == PRESSURE_AND_TEMPERATURE_MODE or
      FastcauldronSimulator::getInstance().getCalculationMode() == COUPLED_DARCY_MODE;


   if ( not FastcauldronSimulator::getInstance().initialiseLayerThicknessHistory( overpressureCalculation ) )
   {
      m_errorMessage = "MeSsAgE ERROR when initialising thickness history.";
      return false;
   }

   if ( FastcauldronSimulator::getInstance().getCalculationMode() == OVERPRESSURED_TEMPERATURE_MODE or
      FastcauldronSimulator::getInstance().getCalculationMode() == COUPLED_HIGH_RES_DECOMPACTION_MODE )
   {
      // Scale the initalised solid-thicknesses by the fct-correction factors.
      FastcauldronSimulator::getInstance().applyFctCorrections();
   }

   m_errorMessage = "";
   return true;
}

void FastcauldronStartup::run()
{
   if ( m_prepareOk && m_startUpOk )
   {
      bool returnStatus = true;
      // Calculate FCT
      FCTCalc fctCtx( m_cauldron );

      if ( m_cauldron->DoHighResDecompaction || m_cauldron->DoDecompaction )
      {
         fctCtx.decompact();

         if ( not FastcauldronSimulator::getInstance().noDerivedPropertiesCalc() )
         {
            DerivedPropertiesCalculator propertyCalculator( m_cauldron, fctCtx.getVolumeOutputProperties(), fctCtx.getMapOutputProperties() );
            propertyCalculator.compute();
         }
      }

      if ( m_cauldron->DoOverPressure && !m_cauldron->Do_Iteratively_Coupled )
      {
         Basin_Modelling::FEM_Grid basin( m_cauldron );

         // Do Pressure Calculation
         basin.solvePressure( m_solverHasConverged, m_errorInDarcy, m_geometryHasConverged );
      }

      if ( m_cauldron->DoTemperature && !m_cauldron->Do_Iteratively_Coupled )
      {
         Basin_Modelling::FEM_Grid basin( m_cauldron );

         // Do Tempearature Calculation
         basin.solveTemperature( m_solverHasConverged, m_errorInDarcy );

         if ( not FastcauldronSimulator::getInstance().noDerivedPropertiesCalc() )
         {
            DerivedPropertiesCalculator propertyCalculator( m_cauldron, basin.getVolumeOutputProperties(), basin.getMapOutputProperties() );
            propertyCalculator.compute();
         }
      }

      if ( m_cauldron->Do_Iteratively_Coupled )
      {
         Basin_Modelling::FEM_Grid basin( m_cauldron );
         // Do Coupled Calculation
         basin.solveCoupled( m_solverHasConverged, m_errorInDarcy, m_geometryHasConverged );

         if ( not FastcauldronSimulator::getInstance().noDerivedPropertiesCalc() ) {
            DerivedPropertiesCalculator propertyCalculator( m_cauldron, basin.getVolumeOutputProperties(), basin.getMapOutputProperties() );

            propertyCalculator.compute();
         }
      }

      if ( m_cauldron->integrateGenexEquations() )
      {
         FastcauldronSimulator::getInstance().saveGenexHistory();
      }

      /// Delete all the arrays used to store the quadrature points and weights.
      /// They will not be used from this point onwards.
      NumericFunctions::Quadrature::finaliseQuadrature();

      if ( ( not m_cauldron->saveOnDarcyError() and m_errorInDarcy ) or not m_solverHasConverged )
      {
         m_runOk = false;
      }
      else
      {
         m_runOk = true;
      }

      if ( !FastcauldronSimulator::getInstance().mergeOutputFiles() )
      {
         PetscPrintf( PETSC_COMM_WORLD, "  MeSsAgE ERROR Unable to merge output files\n" );
         m_runOk = false;
      }

      StatisticsHandler::print();
   }
   else
   {
      m_runOk = false;
   }
}

void FastcauldronStartup::finalize( )
{
   if ( m_errorMessage != "" ) PetscPrintf( PETSC_COMM_WORLD, "\n %s \n", m_errorMessage.c_str( ) );

   bool displayEndTime = false;

   bool saveResults = m_prepareOk && m_startUpOk && m_runOk && m_saveResults;

   FastcauldronSimulator::finalise( saveResults && m_solverHasConverged && ( m_cauldron->saveOnDarcyError( ) or not m_errorInDarcy ) );

   if ( m_cauldron ) displayEndTime = m_cauldron->debug1 or m_cauldron->verbose;
   if ( m_factory != 0 ) delete m_factory;
   if ( m_cauldron != 0 ) delete m_cauldron;

   // display
   displayTime( displayEndTime, "End of simulation: " );
   //FlexLM license check in only for node with rank = 0
#ifdef FLEXLM
   if ( m_checkLicense && ourRank( ) == 0 )
   {
      // FlexLm license check in, close down and enable logging
      EPTFlexLmCheckIn( m_feauture );
      EPTFlexLmTerminate( );
   }
#endif

   // update the number of instances
   s_instances = s_instances - 1;
}

bool FastcauldronStartup::determineSaltModellingCapability()
{
   int capable = 1;

   if ( !m_checkLicense ) return true;

#ifdef FLEXLM
   int rc = EPTFLEXLM_OK;

   char version[EPTFLEXLM_MAX_VER_LEN];
   char errmessage[EPTFLEXLM_MAX_MESS_LEN];

   // FlexLM license handling only for node with rank = 0
   if ( ourRank() == 0 )
   {
      sprintf( m_feauture, "ibs_cauldron_halo" );
#ifdef IBSFLEXLMVERSION
      sprintf( version, IBSFLEXLMVERSION );
#else
      sprintf( version, "9999.99" );
#endif

      rc = EPTFlexLmInit( errmessage );

      if ( rc != EPTFLEXLM_OK )
      {
         cout << endl << "@@@@@@@@@@@@@@@\n MeSsAgE ERROR: FlexLm license init problems: fastcauldron cannot start.\n Please contact your helpdesk\n@@@@@@@@@@@@@@@" << endl;
         capable = 0;
      }
      else {

         // FlexLM license handling: Checkout
         rc = EPTFlexLmCheckOut( m_feauture, version, errmessage );

         if ( rc == EPTFLEXLM_WARN ) {
            cout << endl << "@@@@@@@@@@@@@@@\n MeSsAgE WARNING: fastcauldron will still start the salt modelling capabilities anyway.\n@@@@@@@@@@@@@@@" << endl;
            capable = 1;
         }
         else if ( rc != EPTFLEXLM_OK )
         {
            cout << endl << "@@@@@@@@@@@@@@@\n MeSsAgE WARNING: FlexLm license error: fastcauldron cannot start the salt modelling capabilities.\n Please contact your helpdesk.\n@@@@@@@@@@@@@@@" << endl;
            capable = 0;
         }
         else
         {
            capable = 1;
         }
      }
   }

   MPI_Bcast( &capable, 1, MPI_INT, 0, PETSC_COMM_WORLD );

   if ( ourRank() == 0 && rc != EPTFLEXLM_WARN && rc != EPTFLEXLM_OK )
   {
      EPTFlexLmCheckIn( m_feauture );
      EPTFlexLmTerminate();
   }

#endif

   return capable == 1;
}

int FastcauldronStartup::ourRank() {

   static int myRank;
   static bool haveRank = false;

   if ( not haveRank ) {
      MPI_Comm_rank( PETSC_COMM_WORLD, &myRank );
      haveRank = true;
   }

   return myRank;
}

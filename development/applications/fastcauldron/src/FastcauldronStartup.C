#include "FastcauldronStartup.h"

#include "FastcauldronSimulator.h"
#include "MultiComponentFlowHandler.h"
#include "StatisticsHandler.h"
#include "filterwizard.h"
#include "globaldefs.h"

int FastcauldronStartup::startup ( int                  argc,
                                   char**               argv,
                                   AppCtx*              cauldron,
                                   FastcauldronFactory* factory,
                                   const bool           canRunSaltModelling,
                                   std::string&         errorMessage ) {

   if ( not cauldron->readProjectName ()) {
      errorMessage = "MeSsAgE ERROR Error when reading the project file";
      return 1;
   }

   StatisticsHandler::initialise ();
   FastcauldronSimulator::CreateFrom ( cauldron, factory );
   FastcauldronSimulator::getInstance().readCommandLineParametersEarlyStage( argc, argv );
   FastcauldronSimulator::getInstance().deleteTemporaryDirSnapshots();
   FastcauldronSimulator::getInstance ().setFormationElementHeightScalingFactors ();

   if ( not FastcauldronSimulator::getInstance ().setCalculationMode ( cauldron->getCalculationMode ())) {
      errorMessage = "MeSsAgE ERROR Error when setting calculation mode";
      return 1;
   }

   FastcauldronSimulator::getInstance ().getMcfHandler ().determineUsage ();
   FastcauldronSimulator::getInstance ().initialiseFastcauldronLayers ();

   if ( not cauldron->readProjectFile ()) {
      errorMessage = "MeSsAgE ERROR Error when reading the project file";
      return 1;
   }

   // There are several command line parameters that can be set only after the project file has been read.
   FastcauldronSimulator::getInstance ().readCommandLineParametersLateStage ( argc, argv );

   // Initialise properties output
   if( not cauldron->no2Doutput() ) {
      cauldron->setNo2Doutput( FastcauldronSimulator::getInstance ().getPrimaryPropertiesFlag() );
   }

   // Initialise anything that is to be set from the environment.
   cauldron->setParametersFromEnvironment ();
   cauldron->Display_Grid_Description();
   cauldron->setLayerBottSurfaceName ();

   // Process Data Map and Assess Valid Nodes
   cauldron->setValidNodeArray ();
   cauldron->Examine_Load_Balancing ();
   cauldron->Output_Number_Of_Geological_Events();

   if ( not cauldron->createFormationLithologies ( canRunSaltModelling )) {
      errorMessage = "MeSsAgE ERROR Unable to create lithologies";
      return 1;
   }

   // Find which derived properties are required
   cauldron->filterwizard.InitDerivedCalculationsNeeded ();

   cauldron->Locate_Related_Project ();
   cauldron->setInitialTimeStep ();
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
      errorMessage = "MeSsAgE ERROR when initialising thickness history.";
      return 1;
   }

   if ( FastcauldronSimulator::getInstance ().getCalculationMode () == OVERPRESSURED_TEMPERATURE_MODE or
        FastcauldronSimulator::getInstance ().getCalculationMode () == COUPLED_HIGH_RES_DECOMPACTION_MODE ) {

      // Scale the initalised solid-thicknesses by the fct-correction factors.
      FastcauldronSimulator::getInstance ().applyFctCorrections ();
   }

   errorMessage = "";
   return 0;
}


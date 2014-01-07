//------------------------------------------------------------//

#include "fem_grid.h"
#include "fem_grid_auxiliary_functions.h"
#include "FiniteElementTypes.h"
#include "AllochthonousLithologyManager.h"
#include "HydraulicFracturingManager.h"
#include "PropertyManager.h"

//------------------------------------------------------------//

#include <sstream>

//------------------------------------------------------------//

#include <petscksp.h>
#include <petsc.h>
#include <petscdmda.h>

//------------------------------------------------------------//

#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>

//------------------------------------------------------------//

#include "globaldefs.h"
#include "utils.h"

//------------------------------------------------------------//

#include "layer.h"
#include "layer_iterators.h"

//------------------------------------------------------------//

#include "Well.h"
#include "element_contributions.h"
#include "property_manager.h"
#include "PropertyManager.h"

//------------------------------------------------------------//

#include "Interface/MantleFormation.h"
#include "Interface/RunParameters.h"
#include "Interface/Grid.h"

//------------------------------------------------------------//

#include "GeoPhysicalFunctions.h"
#include "CompoundProperty.h"
#include "NumericFunctions.h"

#include "GeometricLoopPressureSolver.h"
#include "NonGeometricLoopPressureSolver.h"

#include "PVTCalculator.h"
#include "StatisticsHandler.h"

#include "FormationSubdomainElementGrid.h"
#include "MultiComponentFlowHandler.h"

#include "Interface/MantleFormation.h"
#include "Interface/RunParameters.h"
#include "Interface/Grid.h"

#include "PetscLogStages.h"

using namespace GeoPhysics;

//------------------------------------------------------------//

//
// How to handle the PETSc Matrix allocation. Has the bug been
// fixed in the CBM version of the allocation function.
//
#define PETSc_MATRIX_BUG_FIXED 1

using namespace FiniteElementMethod;

//------------------------------------------------------------//


#undef  __FUNCT__  
#define __FUNCT__ "Basin_Modelling::FEM_Grid::contructor"

Basin_Modelling::FEM_Grid::FEM_Grid ( AppCtx* Application_Context ) 
   : Temperature_Calculator ( Application_Context ), m_surfaceNodeHistory ( Application_Context )
{

  basinModel = Application_Context;

  cauldronCalculator = new CauldronCalculator ( basinModel );

  if ( basinModel->getUnitTestNumber () == 3 ) {
     MatrixUnitTest::initialise ( Application_Context );
  }

  if ( FastcauldronSimulator::getInstance ().getRunParameters ()->getNonGeometricLoop ()) {
     pressureSolver = new NonGeometricLoopPressureSolver ( basinModel );
  } else {
     pressureSolver = new GeometricLoopPressureSolver ( basinModel );
  }

  DMDAGetInfo ( *basinModel -> mapDA, PETSC_NULL,
                &Number_Of_X_Nodes, &Number_Of_Y_Nodes, PETSC_NULL,
                &Number_Of_X_Processors, &Number_Of_Y_Processors,
                PETSC_NULL, PETSC_NULL, PETSC_NULL, PETSC_NULL, PETSC_NULL, PETSC_NULL, PETSC_NULL);

  DMDAGetCorners ( *basinModel -> mapDA, PETSC_NULL, PETSC_NULL, PETSC_NULL,
                   &Local_Number_Of_X_Nodes, &Local_Number_Of_Y_Nodes, PETSC_NULL );

  Include_Ghost_Values = true;
  Update_Ghost_Values  = true;

  const CauldronGridDescription& grid = FastcauldronSimulator::getInstance ().getCauldronGridDescription ();

  Delta_X  = grid.deltaI;
  Delta_Y  = grid.deltaJ;
  Origin_X = grid.originI;
  Origin_Y = grid.originJ;

  Pressure_Newton_Solver_Tolerance = 1.0e-5;
  Temperature_Newton_Solver_Tolerance = 1.0e-5;

  Element_Assembly_Time = 0.0;
  System_Assembly_Time = 0.0;
  System_Solve_Time = 0.0;
  Property_Calculation_Time = 0.0;

  Accumulated_Element_Assembly_Time = 0.0;
  Accumulated_System_Assembly_Time = 0.0;
  Accumulated_System_Solve_Time = 0.0;
  Accumulated_Property_Calculation_Time = 0.0;


  // Set properties that may be required for the calculation
  // Formation properties
  mapOutputProperties.push_back ( THICKNESS );
  mapOutputProperties.push_back ( ALLOCHTHONOUS_LITHOLOGY );

#if 0
  if(basinModel->isModellingMode1D () && basinModel -> filterwizard.IsSmectiteIlliteCalculationNeeded())
  {
     mapOutputProperties.push_back ( ILLITEFRACTION );
  }
  if(basinModel->isModellingMode1D () && basinModel -> filterwizard.IsBiomarkersCalculationNeeded())
  {
     mapOutputProperties.push_back ( STERANEAROMATISATION );
     mapOutputProperties.push_back ( STERANEISOMERISATION );
     mapOutputProperties.push_back ( HOPANEISOMERISATION  );
  }
#endif

  if ( FastcauldronSimulator::getInstance ().getBasinHasActiveFaults ()) {
    mapOutputProperties.push_back ( FAULTELEMENTS );
    basinModel->timefilter.setFilter ( "FaultElements", "SedimentsOnly" );
  }

  // We will always require the erosion-factors to be saved.
  basinModel->timefilter.setFilter ( "ErosionFactor", "SedimentsOnly" );
  mapOutputProperties.push_back ( EROSIONFACTOR );

#if 0
  if (( FastcauldronSimulator::getInstance ().getCalculationMode () == OVERPRESSURE_MODE or
        FastcauldronSimulator::getInstance ().getCalculationMode () == PRESSURE_AND_TEMPERATURE_MODE ) and
      not FastcauldronSimulator::getInstance ().getRunParameters ()->getNonGeometricLoop ()) {
     concludingOutputProperties.push_back ( FCTCORRECTION );
     concludingOutputProperties.push_back ( THICKNESSERROR );
  }
#endif

  if (( FastcauldronSimulator::getInstance ().getCalculationMode () == OVERPRESSURE_MODE or
        FastcauldronSimulator::getInstance ().getCalculationMode () == PRESSURE_AND_TEMPERATURE_MODE or
        FastcauldronSimulator::getInstance ().getCalculationMode () == COUPLED_DARCY_MODE ) and
      not FastcauldronSimulator::getInstance ().getRunParameters ()->getNonGeometricLoop ()) {
     concludingOutputProperties.push_back ( THICKNESSERROR );
  }

  if ( FastcauldronSimulator::getInstance ().getCalculationMode () == OVERPRESSURE_MODE or
       FastcauldronSimulator::getInstance ().getCalculationMode () == PRESSURE_AND_TEMPERATURE_MODE or
       FastcauldronSimulator::getInstance ().getCalculationMode () == COUPLED_DARCY_MODE ) {
     concludingOutputProperties.push_back ( FCTCORRECTION );
  }

  looselyCoupledOutputProperties.push_back ( VES );
  looselyCoupledOutputProperties.push_back ( MAXVES );

  // Surface properties
  basinModel->timefilter.setFilter ( "AllochthonousLithology", "SedimentsOnly" );


  mapOutputProperties.push_back ( DEPTH );
  mapOutputProperties.push_back ( POROSITYVEC );
  mapOutputProperties.push_back ( CHEMICAL_COMPACTION );
  mapOutputProperties.push_back ( PERMEABILITYVEC );
  mapOutputProperties.push_back ( TEMPERATURE );
  mapOutputProperties.push_back ( DIFFUSIVITYVEC );

  //Brine properties: density and viscosity
#if 0
  mapOutputProperties.push_back ( BRINE_PROPERTIES );
#endif 

  m_volumeOutputProperties.push_back ( BRINE_PROPERTIES );     
   
  m_volumeOutputProperties.push_back ( DEPTH );
  m_volumeOutputProperties.push_back ( HYDROSTATICPRESSURE );
  m_volumeOutputProperties.push_back ( LITHOSTATICPRESSURE );
  m_volumeOutputProperties.push_back ( OVERPRESSURE );
  m_volumeOutputProperties.push_back ( PRESSURE );
  m_volumeOutputProperties.push_back ( CHEMICAL_COMPACTION ); 
  m_volumeOutputProperties.push_back ( VES );
  m_volumeOutputProperties.push_back ( MAXVES );
  m_volumeOutputProperties.push_back ( TEMPERATURE );
  m_volumeOutputProperties.push_back ( POROSITYVEC );
  m_volumeOutputProperties.push_back ( PERMEABILITYVEC );
  m_volumeOutputProperties.push_back ( HEAT_FLOW );
  m_volumeOutputProperties.push_back ( DIFFUSIVITYVEC );
  m_volumeOutputProperties.push_back ( BULKDENSITYVEC );
  m_volumeOutputProperties.push_back ( THCONDVEC );
  m_volumeOutputProperties.push_back ( FLUID_VELOCITY );

#if 0
  // Remove from list until the lithology id has been fixed.
  m_volumeOutputProperties.push_back ( LITHOLOGY );
#endif

#if 0
  m_volumeOutputProperties.push_back ( CAPILLARYPRESSUREGAS100 );
  m_volumeOutputProperties.push_back ( CAPILLARYPRESSUREGAS0 );
  m_volumeOutputProperties.push_back ( CAPILLARYPRESSUREOIL100 );
  m_volumeOutputProperties.push_back ( CAPILLARYPRESSUREOIL0 );
#endif

  if (basinModel->isModellingMode1D())
  {
     m_volumeOutputProperties.push_back ( SONICVEC );
  }

  m_volumeOutputProperties.push_back ( VELOCITYVEC );
  m_volumeOutputProperties.push_back ( REFLECTIVITYVEC );
  m_volumeOutputProperties.push_back ( VR );

  if (basinModel->isModellingMode1D())
  {
     m_volumeOutputProperties.push_back ( ILLITEFRACTION );
     m_volumeOutputProperties.push_back ( BIOMARKERS );
  }

  mapOutputProperties.push_back ( VR );

  if (basinModel->isModellingMode1D())
  {
     mapOutputProperties.push_back ( ILLITEFRACTION );
     mapOutputProperties.push_back ( BIOMARKERS );
  }

  mapOutputProperties.push_back ( HEAT_FLOW );

  // If the pressure calculator is changed to solve for the pore-pressure (or Hubberts potential, see Annette) 
  // then remove the if statement. Keep only the assignment of the fluid-velocities.
  if ( basinModel->DoOverPressure or basinModel->Do_Iteratively_Coupled )  {
    mapOutputProperties.push_back ( FLUID_VELOCITY );
  }

  mapOutputProperties.push_back ( BULKDENSITYVEC );
  mapOutputProperties.push_back ( VELOCITYVEC );

  if (basinModel->isModellingMode1D())
  {
     mapOutputProperties.push_back ( SONICVEC );
  }

  mapOutputProperties.push_back ( REFLECTIVITYVEC );
  mapOutputProperties.push_back ( PERMEABILITYHVEC );
  mapOutputProperties.push_back ( THCONDVEC );

  mapOutputProperties.push_back ( VES );
  mapOutputProperties.push_back ( MAXVES );
  mapOutputProperties.push_back ( OVERPRESSURE );
  mapOutputProperties.push_back ( PRESSURE );
  mapOutputProperties.push_back ( HYDROSTATICPRESSURE );
  mapOutputProperties.push_back ( LITHOSTATICPRESSURE );

#if 0
  mapOutputProperties.push_back ( CAPILLARYPRESSUREGAS100 );
  mapOutputProperties.push_back ( CAPILLARYPRESSUREGAS0 );
  mapOutputProperties.push_back ( CAPILLARYPRESSUREOIL100 );
  mapOutputProperties.push_back ( CAPILLARYPRESSUREOIL0 );
#endif

  if ( FastcauldronSimulator::getInstance ().getCalculationMode () == OVERPRESSURED_TEMPERATURE_MODE or 
       FastcauldronSimulator::getInstance ().getCalculationMode () == COUPLED_HIGH_RES_DECOMPACTION_MODE )
  {
     FastcauldronSimulator::getInstance ().setOutputPropertyOption ( VES, Interface::NO_OUTPUT );
     FastcauldronSimulator::getInstance ().setOutputPropertyOption ( MAXVES, Interface::NO_OUTPUT );
     FastcauldronSimulator::getInstance ().setOutputPropertyOption ( OVERPRESSURE, Interface::NO_OUTPUT );
     FastcauldronSimulator::getInstance ().setOutputPropertyOption ( PRESSURE, Interface::NO_OUTPUT );
     FastcauldronSimulator::getInstance ().setOutputPropertyOption ( HYDROSTATICPRESSURE, Interface::NO_OUTPUT );
     FastcauldronSimulator::getInstance ().setOutputPropertyOption ( LITHOSTATICPRESSURE, Interface::NO_OUTPUT );
  }

  // If overpressure is being calculated then Ves and MaxVes MUST be output for
  // possible later loosely coupled calculations.
  if (( basinModel->DoOverPressure || basinModel->Do_Iteratively_Coupled ) && 
      ( basinModel->timefilter.PropertyOutputOption [ VES ] == NOOUTPUT ||
        basinModel->timefilter.PropertyOutputOption [ VES ] == SOURCEROCKONLY )) {
    basinModel->timefilter.setFilter ( "Ves", "SedimentsOnly" );
  }

  // If we are doing overpressure calculation only then set lithostatic to be no more that sediments-only for output.
  if ( basinModel->DoOverPressure && 
       basinModel->timefilter.PropertyOutputOption [ LITHOSTATICPRESSURE ] == SEDIMENTSPLUSBASEMENT ) {
    basinModel->timefilter.setFilter ( "LithoStaticPressure", "SedimentsOnly" );
  }

  if (( basinModel->DoOverPressure || basinModel->Do_Iteratively_Coupled ) && 
      ( basinModel->timefilter.PropertyOutputOption [ MAXVES ] == NOOUTPUT ||
        basinModel->timefilter.PropertyOutputOption [ MAXVES ] == SOURCEROCKONLY )) {
    basinModel->timefilter.setFilter ( "MaxVes", "SedimentsOnly" );
  }

  // If temperature is being calculated then make sure that temperature is being output.
  if (( basinModel->DoTemperature || basinModel->Do_Iteratively_Coupled )) {

    if ( basinModel->timefilter.PropertyOutputOption [ TEMPERATURE ] == NOOUTPUT ) {
      basinModel->timefilter.setFilter ( "Temperature", "SourceRockOnly" );
    }

  }
  if ( basinModel->isALC() ) {
     mapOutputProperties.push_back ( TOPBASALTALC );
     basinModel->timefilter.setFilter ( "ALCStepTopBasaltDepth", "SedimentsPlusBasement" );
     FastcauldronSimulator::getInstance ().setOutputPropertyOption ( TOPBASALTALC, Interface::SEDIMENTS_AND_BASEMENT_OUTPUT );

     mapOutputProperties.push_back ( MOHOALC );
     basinModel->timefilter.setFilter ( "ALCStepMohoDepth", "SedimentsPlusBasement" );
     FastcauldronSimulator::getInstance ().setOutputPropertyOption ( MOHOALC, Interface::SEDIMENTS_AND_BASEMENT_OUTPUT );

     if(  basinModel->timefilter.PropertyOutputOption [ HLMOD ] != NOOUTPUT ) {
        mapOutputProperties.push_back ( HLMOD );
        basinModel->timefilter.setFilter ( "ALCMaxAsthenoMantleDepth", "SedimentsPlusBasement" );
     }
     if(  basinModel->timefilter.PropertyOutputOption [ BASALTTHICKNESS ] != NOOUTPUT ) {
        mapOutputProperties.push_back ( BASALTTHICKNESS );
        basinModel->timefilter.setFilter ( "ALCSmBasaltThickness", "SedimentsPlusBasement" );
     }
     if(  basinModel->timefilter.PropertyOutputOption [ THICKNESSCCRUSTALC ] != NOOUTPUT ) {
        mapOutputProperties.push_back ( THICKNESSCCRUSTALC );
        basinModel->timefilter.setFilter ( "ALCStepContCrustThickness", "SedimentsPlusBasement" );
     }
     if(  basinModel->timefilter.PropertyOutputOption [ THICKNESSBASALTALC ] != NOOUTPUT ) {
        mapOutputProperties.push_back ( THICKNESSBASALTALC );
        basinModel->timefilter.setFilter ( "ALCStepBasaltThickness", "SedimentsPlusBasement" );
     }
     if(  basinModel->timefilter.PropertyOutputOption [ ALCORIGMANTLE ] != NOOUTPUT ) {
        mapOutputProperties.push_back ( ALCORIGMANTLE );
        basinModel->timefilter.setFilter ( "ALCOrigLithMantleDepth", "SedimentsPlusBasement" );
     }
     if(  basinModel->timefilter.PropertyOutputOption [ ALCSMCRUST ] != NOOUTPUT ) {
        mapOutputProperties.push_back ( ALCSMCRUST );
        basinModel->timefilter.setFilter ( "ALCSmContCrustThickness", "SedimentsPlusBasement" );
     }
     if(  basinModel->timefilter.PropertyOutputOption [ ALCSMTOPBASALT ] != NOOUTPUT ) {
        mapOutputProperties.push_back ( ALCSMTOPBASALT );
        basinModel->timefilter.setFilter ( "ALCSmTopBasaltDepth", "SedimentsPlusBasement" );
     }
     if(  basinModel->timefilter.PropertyOutputOption [ ALCSMMOHO ] != NOOUTPUT ) {
        mapOutputProperties.push_back ( ALCSMMOHO );
        basinModel->timefilter.setFilter ( "ALCSmMohoDepth", "SedimentsPlusBasement" );
     }
  }

  if ( FastcauldronSimulator::getInstance ().getCauldron ()->integrateGenexEquations ()) {
     mapOutputProperties.push_back ( GENEX_PROPERTIES );
     mapOutputProperties.push_back ( EXPULSION_API_INST );
     mapOutputProperties.push_back ( EXPULSION_API_CUM );
     mapOutputProperties.push_back ( EXPULSION_CONDENSATE_GAS_RATIO_INST );
     mapOutputProperties.push_back ( EXPULSION_CONDENSATE_GAS_RATIO_CUM );
     mapOutputProperties.push_back ( EXPULSION_GAS_OIL_RATIO_INST );
     mapOutputProperties.push_back ( EXPULSION_GAS_OIL_RATIO_CUM );
     mapOutputProperties.push_back ( EXPULSION_GAS_WETNESS_INST );
     mapOutputProperties.push_back ( EXPULSION_GAS_WETNESS_CUM );
     mapOutputProperties.push_back ( EXPULSION_AROMATICITY_INST );
     mapOutputProperties.push_back ( EXPULSION_AROMATICITY_CUM );
     mapOutputProperties.push_back ( KEROGEN_CONVERSION_RATIO );
     mapOutputProperties.push_back ( OIL_GENERATED_CUM );
     mapOutputProperties.push_back ( OIL_GENERATED_RATE ); 
     mapOutputProperties.push_back ( OIL_EXPELLED_CUM );
     mapOutputProperties.push_back ( OIL_EXPELLEDRATE );
     mapOutputProperties.push_back ( HC_GAS_GENERATED_CUM );
     mapOutputProperties.push_back ( HC_GAS_GENERATED_RATE );
     mapOutputProperties.push_back ( HC_GAS_EXPELLED_CUM );
     mapOutputProperties.push_back ( HC_GAS_EXPELLED_RATE );
     mapOutputProperties.push_back ( DRY_GAS_GENERATED_CUM );
     mapOutputProperties.push_back ( DRY_GAS_GENERATED_RATE );
     mapOutputProperties.push_back ( DRY_GAS_EXPELLED_CUM );
     mapOutputProperties.push_back ( DRY_GAS_EXPELLED_RATE );
     mapOutputProperties.push_back ( WET_GAS_GENERATED_CUM );
     mapOutputProperties.push_back ( WET_GAS_GENERATED_RATE );
     mapOutputProperties.push_back ( WET_GAS_EXPELLED_CUM );
     mapOutputProperties.push_back ( WET_GAS_EXPELLED_RATE );
  }

  if ( FastcauldronSimulator::getInstance ().getMcfHandler ().solveFlowEquations ()) {

     if ( true or FastcauldronSimulator::getInstance ().getMcfHandler ().outputDarcyMaps ()) {
        mapOutputProperties.push_back ( GENEX_PROPERTY_CONCENTRATIONS );
        mapOutputProperties.push_back ( SATURATION );
        // mapOutputProperties.push_back ( AVERAGE_SATURATION );
        // mapOutputProperties.push_back ( CAPILLARY_PRESSURE );
        // mapOutputProperties.push_back ( FLUID_PROPERTIES );
     }


#if 0
     m_volumeOutputProperties.push_back ( AVERAGE_SATURATION );
     m_volumeOutputProperties.push_back ( HC_FLUID_VELOCITY );
#endif

     m_volumeOutputProperties.push_back ( GENEX_PROPERTY_CONCENTRATIONS );
     m_volumeOutputProperties.push_back ( PVT_PROPERTIES );
     m_volumeOutputProperties.push_back ( SATURATION );
     m_volumeOutputProperties.push_back ( RELATIVE_PERMEABILITY );
     m_volumeOutputProperties.push_back ( CAPILLARY_PRESSURE );
     m_volumeOutputProperties.push_back ( FLUID_PROPERTIES );

     if ( FastcauldronSimulator::getInstance ().getMcfHandler ().saveCapillaryEntryPressure ()) {
        m_volumeOutputProperties.push_back ( CAPILLARY_ENTRY_PRESSURE );
     }

     if ( FastcauldronSimulator::getInstance ().getMcfHandler ().saveVolumeOutput ()) {
        m_volumeOutputProperties.push_back ( VOLUME_CALCULATIONS );
     }

     if ( FastcauldronSimulator::getInstance ().getMcfHandler ().saveTransportedVolumeOutput ()) {
        m_volumeOutputProperties.push_back ( TRANSPORTED_VOLUME_CALCULATIONS );
     }

     // Time of invasion is needed only at present day.
     m_concludingVolumeOutputProperties.push_back ( TIME_OF_ELEMENT_INVASION );
  }


#if 0
  // If we are doing a loosely coupled calculation then do not update the pressure related properties.
  if ( basinModel->IsCalculationCoupled && ( basinModel->DoTemperature )) {
    basinModel->timefilter.setFilter ( "Ves", "None" );
    basinModel->timefilter.setFilter ( "MaxVes", "None" );
    basinModel->timefilter.setFilter ( "OverPressure", "None" );
    basinModel->timefilter.setFilter ( "Pressure", "None" );
  }
#endif

  // Properties required by GenEx.
  genexOutputProperties.push_back ( TEMPERATURE );
  genexOutputProperties.push_back ( VES );

  genexOutputProperties.push_back ( VR );
  genexOutputProperties.push_back ( MAXVES );
  genexOutputProperties.push_back ( PRESSURE );
  genexOutputProperties.push_back ( CHEMICAL_COMPACTION ); 
  genexOutputProperties.push_back ( HYDROSTATICPRESSURE );
  genexOutputProperties.push_back ( LITHOSTATICPRESSURE );
  genexOutputProperties.push_back ( POROSITYVEC );
  genexOutputProperties.push_back ( PERMEABILITYVEC );

  genexOutputProperties.push_back ( EROSIONFACTOR );

  // Preevaluated_Basis_Functions::Get_Instance ()->Preevaluate ( pressureSolver->getPlaneQuadratureDegree ( basinModel -> Optimisation_Level ),
  //                                                              pressureSolver->getDepthQuadratureDegree ( basinModel -> Optimisation_Level ));

  // Initialise the surfaceNodeHistory object, by reading in the 
  // specification file containing all nodes that are to be recorded.
  m_surfaceNodeHistory.Read_Spec_File ();
  savedMinorSnapshotTimes.clear ();

}


//------------------------------------------------------------//

#undef  __FUNCT__  
#define __FUNCT__ "Basin_Modelling::FEM_Grid::destructor"

Basin_Modelling::FEM_Grid::~FEM_Grid () {

  if ( basinModel->debug1 or basinModel->verbose) {
    PetscPrintf ( PETSC_COMM_WORLD, " total System_Assembly_Time      %f \n", Accumulated_System_Assembly_Time );
    PetscPrintf ( PETSC_COMM_WORLD, " total Element_Assembly_Time     %f \n", Accumulated_Element_Assembly_Time );
    PetscPrintf ( PETSC_COMM_WORLD, " total System_Solve_Time         %f \n", Accumulated_System_Solve_Time );
    PetscPrintf ( PETSC_COMM_WORLD, " total Property_Calculation_Time %f \n", Accumulated_Property_Calculation_Time );
  }

#if 0
  // PetscSynchronizedPrintf ( PETSC_COMM_WORLD, " Memory usage %f, %f for rank %d \n", m_virtualMemoryUsage, m_residentMemoryUsage, FastcauldronSimulator::getInstance ().getRank ());

  std::stringstream buffer;

  // buffer << " Memory usage " << m_virtualMemoryUsage << "  " << m_residentMemoryUsage << " for rank " << FastcauldronSimulator::getInstance ().getRank () << endl;
  // PetscSynchronizedPrintf ( PETSC_COMM_WORLD, buffer.str ().c_str ());

  buffer << "  <memory_usage>" << endl;
  buffer << "    <rank> " << FastcauldronSimulator::getInstance ().getRank () << " </rank>" << endl;
  buffer << "    <virtual> " << m_virtualMemoryUsage << " </virtual>" << endl;
  buffer << "    <resident> " << m_residentMemoryUsage << " </resident>" << endl;
  buffer << "  </memory_usage>" << endl;

  PetscPrintf ( PETSC_COMM_WORLD, "<statistics>\n");
  PetscSynchronizedFlush ( PETSC_COMM_WORLD );

  PetscSynchronizedPrintf ( PETSC_COMM_WORLD, buffer.str ().c_str ());
  PetscSynchronizedFlush ( PETSC_COMM_WORLD );

  PetscPrintf ( PETSC_COMM_WORLD, "</statistics>\n");
  PetscSynchronizedFlush ( PETSC_COMM_WORLD );
#endif

  if ( basinModel->getUnitTestNumber () == 3 ) {
     MatrixUnitTest::finalise ();
  }

  delete cauldronCalculator;
  delete pressureSolver;

}


//------------------------------------------------------------//

#undef  __FUNCT__  
#define __FUNCT__ "Basin_Modelling::FEM_Grid::solvePressure"

void Basin_Modelling::FEM_Grid::solvePressure ( bool& solverHasConverged,
                                                bool& errorInDarcy,
                                                bool& geometryHasConverged ) {

  int    Maximum_Number_Of_Geometric_Iterations;
  int    Number_Of_Geometric_Iterations = 1;
  bool   Geometry_Has_Converged;
  bool   overpressureHasDiverged;

  geometryHasConverged = true;
  basinModel->initialiseTimeIOTable ( OverpressureRunStatusStr );

  Maximum_Number_Of_Geometric_Iterations = basinModel->MaxNumberOfRunOverpressure;

  if ( basinModel->debug1 or basinModel->verbose) {
    PetscPrintf ( PETSC_COMM_WORLD, "o Maximum number of Geometric iterations: %d \n", Maximum_Number_Of_Geometric_Iterations );
    PetscPrintf ( PETSC_COMM_WORLD, " Optimisation level: %d \n", basinModel -> Optimisation_Level );
  }

  do {
    FastcauldronSimulator::getInstance ().restartActivity ();
    m_surfaceNodeHistory.clearProperties ();
//     basinModel->projectSnapshots.deleteIntermediateMinorSnapshotFiles ( savedMinorSnapshotTimes, basinModel->getOutputDirectory ());

    basinModel->deleteMinorSnapshotsFromTimeIOTable ( savedMinorSnapshotTimes, genexOutputProperties );
    basinModel->deleteMinorSnapshotsFromTimeIOTable ( savedMinorSnapshotTimes, mapOutputProperties );

    FastcauldronSimulator::getInstance ().deleteSnapshotProperties ();
    FastcauldronSimulator::getInstance ().deleteMinorSnapshots ();
    FastcauldronSimulator::getInstance ().deleteMinorSnapshotsFromSnapshotTable ();
    savedMinorSnapshotTimes.clear ();

    Temperature_Calculator.initialiseVReVectors ( basinModel );

    if( basinModel->isModellingMode1D () ) 
    {
       Temperature_Calculator.resetBiomarkerStateVectors ( );
       Temperature_Calculator.resetSmectiteIlliteStateVectors ( );
       Temperature_Calculator.resetFissionTrackCalculator ( );
       basinModel->deleteIsoValues();
    }

    if ( basinModel->debug1 or basinModel->verbose ) {
      PetscPrintf ( PETSC_COMM_WORLD, 
                    "o Starting iteration %d of %d (Maximum number of iterations)",
                    Number_Of_Geometric_Iterations,
                    Maximum_Number_Of_Geometric_Iterations );
    }

    // Compute the overpressure from basin-start-age to present day.
    Evolve_Pressure_Basin ( Number_Of_Geometric_Iterations, 
                            overpressureHasDiverged,
                            errorInDarcy );

    if ( not ( overpressureHasDiverged or errorInDarcy )) {
      // Check that the predicted geometry has converged to with some tolerance of the real (input) geometry
      pressureSolver->adjustSolidThickness ( pressureSolver->getRelativeThicknessTolerance ( basinModel -> Optimisation_Level ),
                                             pressureSolver->getAbsoluteThicknessTolerance ( basinModel -> Optimisation_Level ),
                                             Geometry_Has_Converged );

      Number_Of_Geometric_Iterations = Number_Of_Geometric_Iterations + 1;
      MPI_Barrier(PETSC_COMM_WORLD);
    }

  } while (( Number_Of_Geometric_Iterations <= Maximum_Number_Of_Geometric_Iterations ) && ! Geometry_Has_Converged && ! overpressureHasDiverged );

  const Interface::Snapshot* snapshot = FastcauldronSimulator::getInstance ().findOrCreateSnapshot ( 0.0 );
  assert ( snapshot != 0 );

  // e.g. thickness-error, fct-correction.
  FastcauldronSimulator::getInstance ().saveMapProperties ( concludingOutputProperties, snapshot, Interface::SEDIMENTS_ONLY_OUTPUT );
  

  solverHasConverged = ! overpressureHasDiverged;

  if ( overpressureHasDiverged ) {
    PetscPrintf ( PETSC_COMM_WORLD,
                  "MeSsAgE ERROR Calculation has diverged, see help for possible solutions. \n" );    
  } else {
    displayTime(basinModel->debug1 or basinModel->verbose,"OverPressure Calculation: ");

    m_surfaceNodeHistory.Output_Properties ();
    //FTracks write to database
    //if(basinModel->isModellingMode1D ())
    //{
    //     Temperature_Calculator.computeFissionTracks();
    //     Temperature_Calculator.writeFissionTrackResultsToDatabase();
    //}

    if ( ! basinModel->projectSnapshots.projectPrescribesMinorSnapshots ()) {
      basinModel->projectSnapshots.setActualMinorSnapshots ( savedMinorSnapshotTimes );
    }

    if ( Number_Of_Geometric_Iterations > Maximum_Number_Of_Geometric_Iterations && ! Geometry_Has_Converged ) {
       geometryHasConverged = false;
       PetscPrintf ( PETSC_COMM_WORLD,
                     "MeSsAgE WARNING Maximum number of geometric iterations, %d, exceeded and geometry has not converged \n",
                     Maximum_Number_Of_Geometric_Iterations );
      
       PetscPrintf ( PETSC_COMM_WORLD,
                     "MeSsAgE WARNING Look at the ThicknessError maps in Cauldron to see if the error is acceptable\n" );
    }

  }

}


//------------------------------------------------------------//

#undef  __FUNCT__  
#define __FUNCT__ "Basin_Modelling::FEM_Grid::solveTemperature"

void Basin_Modelling::FEM_Grid::solveTemperature ( bool& solverHasConverged,
                                                   bool& errorInDarcy ) {

  bool temperatureHasDiverged;

  m_surfaceNodeHistory.clearProperties ();
  basinModel->deleteMinorSnapshotsFromTimeIOTable ( savedMinorSnapshotTimes, genexOutputProperties );
  savedMinorSnapshotTimes.clear ();


  if ( FastcauldronSimulator::getInstance ().getCalculationMode () == HYDROSTATIC_TEMPERATURE_MODE )
  {
    basinModel->initialiseTimeIOTable ( HydrostaticTemperatureRunStatusStr );
    basinModel->deleteMinorSnapshotsFromTimeIOTable ( savedMinorSnapshotTimes, genexOutputProperties );
    
    //make sure that surfaceOutputPropterties is a superset of properties output in Output.C:savePropsOnSegmentNodes1D() 
    basinModel->deleteMinorSnapshotsFromTimeIOTable ( savedMinorSnapshotTimes, mapOutputProperties ); 

    FastcauldronSimulator::getInstance ().deleteSnapshotProperties ();

    // Delete the minor snapshots from the snapshot-table.
    FastcauldronSimulator::getInstance ().deleteMinorSnapshots (); 
    FastcauldronSimulator::getInstance ().deleteMinorSnapshotsFromSnapshotTable ();
  }
  else if ( FastcauldronSimulator::getInstance ().getCalculationMode () == OVERPRESSURED_TEMPERATURE_MODE )
  {
    basinModel->initialiseTimeIOTable ( OverpressuredTemperatureRunStatusStr );
  }

  if ( FastcauldronSimulator::getInstance ().getModellingMode () == Interface::MODE1D ) {
     FastcauldronSimulator::getInstance ().clear1DTimeIoTbl ();
     FastcauldronSimulator::getInstance ().clearDepthIoTbl ();
  }

  // Compute the temperature from basin-start-age to present day.
  Evolve_Temperature_Basin ( temperatureHasDiverged, errorInDarcy );

  solverHasConverged = ! temperatureHasDiverged;

  if ( temperatureHasDiverged ) {
    PetscPrintf ( PETSC_COMM_WORLD,
                  "MeSsAgE ERROR Calculation has diverged, see help for possible solutions. \n" );    
  } else if ( errorInDarcy ) {
     //
  } else {
    displayTime(basinModel->debug1 or basinModel->verbose,"Temperature Calculation: ");

    m_surfaceNodeHistory.Output_Properties ();

    const Interface::Snapshot* snapshot = FastcauldronSimulator::getInstance ().findOrCreateSnapshot ( 0.0 );
    assert ( snapshot != 0 );
    
  
    //FTracks write to database
    if(basinModel->isModellingMode1D ())
    {
       Temperature_Calculator.computeFissionTracks();
       Temperature_Calculator.writeFissionTrackResultsToDatabase();
    }

    // If the minor snapshot times were not prescribed (from a previous overpressure/coupled run)
    // then the times that are saved must be assigned to the 
    if ( ! basinModel->projectSnapshots.projectPrescribesMinorSnapshots ()) {
      basinModel->projectSnapshots.setActualMinorSnapshots ( savedMinorSnapshotTimes );
    }

  }

}


//------------------------------------------------------------//

#undef  __FUNCT__  
#define __FUNCT__ "Basin_Modelling::FEM_Grid::solveCoupled"

void Basin_Modelling::FEM_Grid::solveCoupled ( bool& solverHasConverged,
                                               bool& errorInDarcy,
                                               bool& geometryHasConverged ) {

  int    Maximum_Number_Of_Geometric_Iterations;
  int    Number_Of_Geometric_Iterations = 1;
  bool   Geometry_Has_Converged;
  bool   overpressureHasDiverged;

  geometryHasConverged = true;
  basinModel->initialiseTimeIOTable ( CoupledPressureTemperatureRunStatusStr );

  Maximum_Number_Of_Geometric_Iterations = basinModel->MaxNumberOfRunOverpressure;

  if ( basinModel->debug1 or basinModel->verbose ) {
    PetscPrintf ( PETSC_COMM_WORLD, "o Maximum number of Geometric iterations: %d \n", Maximum_Number_Of_Geometric_Iterations );
    PetscPrintf ( PETSC_COMM_WORLD, " Optimisation level: %d \n", basinModel -> Optimisation_Level );
  }

  // Start of geometric loop.
  do {

    FastcauldronSimulator::getInstance ().restartActivity ();
    m_surfaceNodeHistory.clearProperties ();

    // The deleting of the minor snapshot files and deleting the times from the timeio table is required
    // because we are at the start of a possible new overpressure run, with probably slightly different
    // time-steps. So the files will have different names!
//     basinModel->projectSnapshots.deleteIntermediateMinorSnapshotFiles ( savedMinorSnapshotTimes, basinModel->getOutputDirectory ());
    
    if ( basinModel->isModellingMode3D() )
    {
       basinModel->deleteMinorSnapshotsFromTimeIOTable ( savedMinorSnapshotTimes, genexOutputProperties );
    
       //make sure that surfaceOutputPropterties is a superset of properties output in Output.C:savePropsOnSegmentNodes1D() 
       basinModel->deleteMinorSnapshotsFromTimeIOTable ( savedMinorSnapshotTimes, mapOutputProperties ); 

       FastcauldronSimulator::getInstance ().deleteSnapshotProperties ();
       FastcauldronSimulator::getInstance ().deleteMinorSnapshots (); 
       FastcauldronSimulator::getInstance ().deleteMinorSnapshotsFromSnapshotTable ();
   }
    else
    {
       basinModel->timeIoTbl->clear();
       FastcauldronSimulator::getInstance ().deleteMinorSnapshotsFromSnapshotTable ();
       Temperature_Calculator.resetBiomarkerStateVectors ( );
       Temperature_Calculator.resetSmectiteIlliteStateVectors ( );
       Temperature_Calculator.resetFissionTrackCalculator();
       basinModel->deleteIsoValues();
       FastcauldronSimulator::getInstance ().clear1DTimeIoTbl ();
       FastcauldronSimulator::getInstance ().clearDepthIoTbl ();
    }

    savedMinorSnapshotTimes.clear ();
    Temperature_Calculator.initialiseVReVectors ( basinModel );

    if ( basinModel->debug1 or basinModel->verbose ) {
      PetscPrintf ( PETSC_COMM_WORLD, 
                    "o Starting iteration %d of %d (Maximum number of iterations)", 
                    Number_Of_Geometric_Iterations,
                    Maximum_Number_Of_Geometric_Iterations );
    }

    // Compute the coupled pressure-temperature from basin-start-age to present day.
    Evolve_Coupled_Basin ( Number_Of_Geometric_Iterations, overpressureHasDiverged, errorInDarcy );

    if ( not ( overpressureHasDiverged or errorInDarcy )) {

      // Check that the predicted geometry has converged to with some tolerance of the real (input) geometry
      pressureSolver->adjustSolidThickness ( pressureSolver->getRelativeThicknessTolerance ( basinModel -> Optimisation_Level ),
                                             pressureSolver->getAbsoluteThicknessTolerance ( basinModel -> Optimisation_Level ),
                                             Geometry_Has_Converged );

      Number_Of_Geometric_Iterations = Number_Of_Geometric_Iterations + 1;
      MPI_Barrier(PETSC_COMM_WORLD);
    }

  } while (( Number_Of_Geometric_Iterations <= Maximum_Number_Of_Geometric_Iterations ) and ( not Geometry_Has_Converged ) and not overpressureHasDiverged and not errorInDarcy );

  const Interface::Snapshot* snapshot = FastcauldronSimulator::getInstance ().findOrCreateSnapshot ( 0.0 );
  assert ( snapshot != 0 );

  // e.g. thickness-error, fct-correction.
  FastcauldronSimulator::getInstance ().saveMapProperties ( concludingOutputProperties, snapshot, Interface::SEDIMENTS_ONLY_OUTPUT );
  

  solverHasConverged = not overpressureHasDiverged;

  if ( overpressureHasDiverged ) {
    PetscPrintf ( PETSC_COMM_WORLD,
                  "MeSsAgE ERROR Calculation has diverged, see help for possible solutions. \n" );    
  } else if ( errorInDarcy ) {
     // 
  } else {
    displayTime(basinModel->debug1 or basinModel->verbose,"P/T Coupled Calculation: ");

    m_surfaceNodeHistory.Output_Properties ();
    //FTracks write to database
    if(basinModel->isModellingMode1D ())
    {
         Temperature_Calculator.computeFissionTracks();
         Temperature_Calculator.writeFissionTrackResultsToDatabase();
    }


    if ( ! basinModel->projectSnapshots.projectPrescribesMinorSnapshots ()) {
      basinModel->projectSnapshots.setActualMinorSnapshots ( savedMinorSnapshotTimes );
    }

    if ( Number_Of_Geometric_Iterations > Maximum_Number_Of_Geometric_Iterations && ! Geometry_Has_Converged ) {
       geometryHasConverged = false;
       PetscPrintf ( PETSC_COMM_WORLD,
                     "MeSsAgE WARNING Maximum number of geometric iterations, %d, exceeded and geometry has not converged \n",
                     Maximum_Number_Of_Geometric_Iterations );

       PetscPrintf ( PETSC_COMM_WORLD,
                     "MeSsAgE WARNING Look at the ThicknessError maps in Cauldron to see if the error is acceptable\n" );
    }

  }

}


//------------------------------------------------------------//

#undef  __FUNCT__
#define __FUNCT__ "Basin_Modelling::FEM_Grid::Evolve_Pressure_Basin"

void Basin_Modelling::FEM_Grid::Evolve_Pressure_Basin ( const int   Number_Of_Geometric_Iterations,
                                                              bool& overpressureHasDiverged,
                                                              bool& errorInDarcy ) {

  const int MaximumNumberOfNonlinearIterations = pressureSolver->getMaximumNumberOfNonlinearIterations ( basinModel->Optimisation_Level );

  int Number_Of_Timesteps = 0;
  int maximumNumberOfOverpressureIterations;
  int Number_Of_Newton_Iterations = -1;
  double Current_Time, Previous_Time, Time_Step;

  double Po_Norm;

  bool majorSnapshotTimesUpdated;
  bool fracturingOccurredInPreviousTimeStep = false;
  bool fracturingOccurred;
  WallTime::Time startTime;

  System_Assembly_Time      = 0.0;
  Element_Assembly_Time     = 0.0;
  System_Solve_Time         = 0.0;
  Property_Calculation_Time = 0.0;

  majorSnapshots = basinModel->projectSnapshots.majorSnapshotsBegin();
  minorSnapshots = basinModel->projectSnapshots.minorSnapshotsBegin ();

  clearLayerVectors ();

  //Position Time Iterator to Start of Calculation (First Snapshot)
  if ( basinModel->projectSnapshots.majorSnapshotsEnd () != majorSnapshots ) {
     Current_Time = (*majorSnapshots)->time ();
     ++majorSnapshots;
  } else {
     PetscPrintf ( PETSC_COMM_WORLD, "No SnapShotTimes Present" );
     return;
  }

  if ( basinModel->isGeometricLoop ()) {
    maximumNumberOfOverpressureIterations = basinModel -> MaxNumberOfRunOverpressure;
  } else {
    maximumNumberOfOverpressureIterations = 1;
  }

  Pressure_Newton_Solver_Tolerance = pressureSolver->getNewtonSolverTolerance ( basinModel->Optimisation_Level,
                                                                                basinModel->isGeometricLoop (),
                                                                                Number_Of_Geometric_Iterations );

  Time_Step = basinModel->getInitialTimeStep ( Current_Time );
  FastcauldronSimulator::getInstance ().getAllochthonousLithologyManager ().reset ();
  overpressureHasDiverged = false;

  while ( Step_Forward ( Previous_Time, Current_Time, Time_Step, majorSnapshotTimesUpdated, Number_Of_Newton_Iterations ) and not overpressureHasDiverged and not errorInDarcy ) {

    if ( basinModel -> debug1 or basinModel->verbose ) {
      PetscPrintf ( PETSC_COMM_WORLD, "***************************************************************\n" );
    }

    startTime = WallTime::clock ();

    Construct_FEM_Grid ( Previous_Time, Current_Time, majorSnapshots, false );
    FastcauldronSimulator::getInstance ().getMcfHandler ().setSubdomainActivity ( Current_Time );

    Display_Pressure_Solver_Progress ( Number_Of_Geometric_Iterations,
                                       maximumNumberOfOverpressureIterations,
				       Current_Time, Previous_Time - Current_Time, 
                                       true or FastcauldronSimulator::getInstance ().getMcfHandler ().numberOfActiveSubdomains () != 1 );

    Temperature_Calculator.Estimate_Temperature ( basinModel, Current_Time );

    fracturingOccurredInPreviousTimeStep = fracturingOccurred;

    Solve_Pressure_For_Time_Step ( Previous_Time,
                                   Current_Time,
                                   MaximumNumberOfNonlinearIterations,
                                   overpressureHasDiverged,
                                   Number_Of_Newton_Iterations,
                                   Po_Norm,
                                   fracturingOccurred );

    if ( ! overpressureHasDiverged ) {
       Store_Computed_Deposition_Thickness ( Current_Time );
       Integrate_Chemical_Compaction ( Previous_Time, Current_Time );
       integrateGenex ( Previous_Time, Current_Time );

       FastcauldronSimulator::getInstance ().getMcfHandler ().solve ( Previous_Time, Current_Time, errorInDarcy );

       printRelatedProjects ( Current_Time );
       Determine_Next_Pressure_Time_Step ( Current_Time, Time_Step, Number_Of_Newton_Iterations, Number_Of_Geometric_Iterations );

       computeBasementLithostaticPressureForCurrentTimeStep ( basinModel, Current_Time ); 

       Copy_Current_Properties ();
       Destroy_Vectors ();
       Save_Properties ( Current_Time );

       postTimeStepOperations ( Current_Time );
       Number_Of_Timesteps = Number_Of_Timesteps + 1;
    }

    if (( basinModel->debug1 or basinModel->verbose or FastcauldronSimulator::getInstance ().getMcfHandler ().getDebugLevel () > 0 ) ) {
       PetscPrintf(PETSC_COMM_WORLD, " time for time-step: %f\n", (WallTime::clock () - startTime).floatValue());
    }

  }


  if (( basinModel->isGeometricLoop ()) && ( basinModel -> debug1 || basinModel->verbose ) ) {
    PetscPrintf ( PETSC_COMM_WORLD, " total System_Assembly_Time      %f \n", System_Assembly_Time );
    PetscPrintf ( PETSC_COMM_WORLD, " total Element_Assembly_Time     %f \n", Element_Assembly_Time );
    PetscPrintf ( PETSC_COMM_WORLD, " total System_Solve_Time         %f \n", System_Solve_Time );
    PetscPrintf ( PETSC_COMM_WORLD, " total Property_Calculation_Time %f \n", Property_Calculation_Time );
    PetscPrintf ( PETSC_COMM_WORLD, "\n Overpressure Calculation Performed in %d Time Steps\n\n",
                  Number_Of_Timesteps);
  }


  Accumulated_System_Assembly_Time      = Accumulated_System_Assembly_Time      + System_Assembly_Time;
  Accumulated_Element_Assembly_Time     = Accumulated_Element_Assembly_Time     + Element_Assembly_Time;
  Accumulated_System_Solve_Time         = Accumulated_System_Solve_Time         + System_Solve_Time;
  Accumulated_Property_Calculation_Time = Accumulated_Property_Calculation_Time + Property_Calculation_Time;

}


//------------------------------------------------------------//

#undef  __FUNCT__
#define __FUNCT__ "Basin_Modelling::FEM_Grid::Evolve_Temperature_Basin"

void Basin_Modelling::FEM_Grid::Evolve_Temperature_Basin ( bool& temperatureHasDiverged,
                                                           bool& errorInDarcy ) {

  const int MaximumNumberOfNonlinearIterations = Temperature_Calculator.maximumNumberOfNonlinearIterations ( basinModel->Optimisation_Level );

  SnapshotInterval interval;

  int Number_Of_Timesteps = 0;
  int Number_Of_Newton_Iterations = -1;
  double Current_Time;
  double Previous_Time;
  double Time_Step;

  bool   majorSnapshotTimesUpdated;
  double T_Norm;
  WallTime::Time startTime;

  System_Assembly_Time      = 0.0;
  Element_Assembly_Time     = 0.0;
  System_Solve_Time         = 0.0;
  Property_Calculation_Time = 0.0;

  majorSnapshots = basinModel->projectSnapshots.majorSnapshotsBegin ();
  minorSnapshots = basinModel->projectSnapshots.minorSnapshotsBegin ();

  clearLayerVectors ();


  /* Position Snapshots Iterator to Origin of Basin */
  if ( basinModel -> projectSnapshots.majorSnapshotsEnd () != majorSnapshots ) 
  {
    Current_Time = (*majorSnapshots)->time ();
  } 
  else 
  {
    PetscPrintf ( PETSC_COMM_WORLD, " No SnapShotTimes Present \n" );
    return;
  }

  Time_Step = basinModel->getInitialTimeStep ( Current_Time );

  Temperature_Newton_Solver_Tolerance = 1.0e-5;
  temperatureHasDiverged = false;
  errorInDarcy = false;

  Display_Temperature_Solver_Progress ( Current_Time, 0.0, true );
  FastcauldronSimulator::getInstance ().getAllochthonousLithologyManager ().reset ();

  Initialise_Basin_Temperature ( temperatureHasDiverged );

  Save_Properties ( Current_Time );
  Number_Of_Timesteps++;

  cout.precision ( 8 );
  cout.flags ( ios::scientific );


  while ( Step_Forward ( Previous_Time, Current_Time, Time_Step, majorSnapshotTimesUpdated, Number_Of_Newton_Iterations ) and not temperatureHasDiverged and not errorInDarcy ) {

    if ( basinModel -> debug1 or basinModel->verbose ) {
      PetscPrintf ( PETSC_COMM_WORLD, "***************************************************************\n" );
    }

    startTime = WallTime::clock ();

    Construct_FEM_Grid ( Previous_Time, Current_Time, majorSnapshots, majorSnapshotTimesUpdated );
    FastcauldronSimulator::getInstance ().getMcfHandler ().setSubdomainActivity ( Current_Time );

    Display_Temperature_Solver_Progress ( Current_Time, Previous_Time - Current_Time, 
                                          true or FastcauldronSimulator::getInstance ().getMcfHandler ().numberOfActiveSubdomains () != 1 );

    Solve_Temperature_For_Time_Step ( Previous_Time,
                                      Current_Time,
                                      MaximumNumberOfNonlinearIterations,
                                      temperatureHasDiverged,
                                      Number_Of_Newton_Iterations,
                                      T_Norm );

    if ( ! temperatureHasDiverged ) {
       Integrate_Chemical_Compaction ( Previous_Time, Current_Time );
       integrateGenex ( Previous_Time, Current_Time );
       Temperature_Calculator.computeVReIncrement ( basinModel, Previous_Time, Current_Time );

       FastcauldronSimulator::getInstance ().getMcfHandler ().solve ( Previous_Time, Current_Time, errorInDarcy );

       if(  basinModel->isModellingMode1D() )
       {											  
          Temperature_Calculator.computeSmectiteIlliteIncrement ( Previous_Time, Current_Time );
          Temperature_Calculator.computeBiomarkersIncrement ( Previous_Time, Current_Time );
          Temperature_Calculator.collectFissionTrackSampleData( Current_Time );
       }

       printRelatedProjects ( Current_Time );

       Number_Of_Timesteps++;

       Determine_Next_Temperature_Time_Step ( Current_Time, Time_Step );

       computeBasementLithostaticPressureForCurrentTimeStep ( basinModel, Current_Time ); 

       Copy_Current_Properties ();
       Destroy_Vectors ();

       Save_Properties ( Current_Time );
       postTimeStepOperations ( Current_Time );
    }

    if (basinModel->debug1 or basinModel->verbose or FastcauldronSimulator::getInstance ().getMcfHandler ().getDebugLevel () > 0 ) {
       PetscPrintf(PETSC_COMM_WORLD, " time for time-step: %f\n", (WallTime::clock () - startTime).floatValue() );
    }

  }

  if ( basinModel -> debug1 or basinModel->verbose ) {
    PetscPrintf ( PETSC_COMM_WORLD, " total System_Assembly_Time      %f \n", System_Assembly_Time );
    PetscPrintf ( PETSC_COMM_WORLD, " total Element_Assembly_Time     %f \n", Element_Assembly_Time );
    PetscPrintf ( PETSC_COMM_WORLD, " total System_Solve_Time         %f \n", System_Solve_Time );
    PetscPrintf ( PETSC_COMM_WORLD, " total Property_Calculation_Time %f \n", Property_Calculation_Time );
    PetscPrintf ( PETSC_COMM_WORLD, "\n Temperature Calculation Performed in %d Time Steps\n\n",
                  Number_Of_Timesteps);
  }


  Accumulated_System_Assembly_Time      = Accumulated_System_Assembly_Time      + System_Assembly_Time;
  Accumulated_Element_Assembly_Time     = Accumulated_Element_Assembly_Time     + Element_Assembly_Time;
  Accumulated_System_Solve_Time         = Accumulated_System_Solve_Time         + System_Solve_Time;
  Accumulated_Property_Calculation_Time = Accumulated_Property_Calculation_Time + Property_Calculation_Time;

}


//------------------------------------------------------------//

#undef  __FUNCT__
#define __FUNCT__ "Basin_Modelling::FEM_Grid::Evolve_Coupled_Basin"

void Basin_Modelling::FEM_Grid::Evolve_Coupled_Basin ( const int   Number_Of_Geometric_Iterations,
                                                             bool& hasDiverged,
                                                             bool& errorInDarcy ) {

  const int maximumNumberOfNonlinearPressureIterations    = pressureSolver->getMaximumNumberOfNonlinearIterations ( basinModel->Optimisation_Level );
  const int maximumNumberOfNonlinearTemperatureIterations = Temperature_Calculator.maximumNumberOfNonlinearIterations ( basinModel->Optimisation_Level );

  int maximumNumberOfOverpressureIterations;
  int Number_Of_Newton_Iterations = -1;
  double Current_Time, Previous_Time, Time_Step;
  int Number_Of_Timesteps = 0;

  double Po_Norm;
  double T_Norm;

  bool majorSnapshotTimesUpdated;
  bool fracturingOccurred = false;
  bool fracturingOccurredInPreviousTimeStep = false;
  WallTime::Time startTime;

  System_Assembly_Time      = 0.0;
  Element_Assembly_Time     = 0.0;
  System_Solve_Time         = 0.0;
  Property_Calculation_Time = 0.0;

  majorSnapshots = basinModel->projectSnapshots.majorSnapshotsBegin ();
  minorSnapshots = basinModel->projectSnapshots.minorSnapshotsBegin ();

  clearLayerVectors ();

  /* Position Snapshots Iterator to Origin of Basin */
  if ( basinModel -> projectSnapshots.majorSnapshotsEnd () != majorSnapshots ) {
    Current_Time = (*majorSnapshots)->time ();
  } else if ( FastcauldronSimulator::getInstance ().getRank () == 0 ) {
    PetscPrintf ( PETSC_COMM_WORLD, " No SnapShotTimes Present \n" );
    return;
  }

  if ( basinModel->isGeometricLoop ()) {
    maximumNumberOfOverpressureIterations = basinModel -> MaxNumberOfRunOverpressure;
  } else {
    maximumNumberOfOverpressureIterations = 1;
  }

  hasDiverged = false;
  errorInDarcy = false;
  Temperature_Newton_Solver_Tolerance = 1.0e-5;
  Pressure_Newton_Solver_Tolerance = pressureSolver->getNewtonSolverTolerance ( basinModel->Optimisation_Level,
                                                                                basinModel->isGeometricLoop (),
                                                                                Number_Of_Geometric_Iterations );

  Time_Step = basinModel->getInitialTimeStep ( Current_Time );
  Time_Step = NumericFunctions::Minimum ( Time_Step, basinModel->maximumTimeStep ());
  Display_Coupled_Solver_Progress ( Number_Of_Geometric_Iterations,
                                    maximumNumberOfOverpressureIterations,
                                    Current_Time, 0.0, 
                                    true );

  FastcauldronSimulator::getInstance ().getAllochthonousLithologyManager ().reset ();
  Initialise_Basin_Temperature ( hasDiverged );
  Save_Properties ( Current_Time );

  if ( basinModel -> debug1 or basinModel->verbose ) {
    PetscPrintf ( PETSC_COMM_WORLD, "Solving Coupled for Time (Ma): %f \n", Current_Time );
  } 


  // Now only need to do a single newton iteration (keep constant and Newton iterations for future use)
  while ( Step_Forward ( Previous_Time, Current_Time, Time_Step, majorSnapshotTimesUpdated, Number_Of_Newton_Iterations ) and not hasDiverged and not errorInDarcy ) {

    if ( basinModel -> debug1 or basinModel->verbose ) {
      PetscPrintf ( PETSC_COMM_WORLD, "***************************************************************\n" );
    }

    startTime = WallTime::clock ();

    Construct_FEM_Grid ( Previous_Time, Current_Time, majorSnapshots, false );
    FastcauldronSimulator::getInstance ().getMcfHandler ().setSubdomainActivity ( Current_Time );

    Display_Coupled_Solver_Progress ( Number_Of_Geometric_Iterations,
                                      maximumNumberOfOverpressureIterations,
                                      Current_Time, Previous_Time - Current_Time, 
                                      true or FastcauldronSimulator::getInstance ().getMcfHandler ().numberOfActiveSubdomains () != 1 );

    fracturingOccurredInPreviousTimeStep = fracturingOccurred;

    Solve_Coupled_For_Time_Step ( Previous_Time, Current_Time,
                                  maximumNumberOfNonlinearPressureIterations,
                                  maximumNumberOfNonlinearTemperatureIterations,
                                  hasDiverged,
                                  Number_Of_Newton_Iterations,
                                  Po_Norm,
                                  T_Norm,
                                  fracturingOccurred );

    if ( ! hasDiverged ) {
       printRelatedProjects ( Current_Time );
       Store_Computed_Deposition_Thickness ( Current_Time );


       Integrate_Chemical_Compaction ( Previous_Time, Current_Time );
       integrateGenex ( Previous_Time, Current_Time );
       Temperature_Calculator.computeVReIncrement ( basinModel, Previous_Time, Current_Time );

       FastcauldronSimulator::getInstance ().getMcfHandler ().solve ( Previous_Time, Current_Time, errorInDarcy );

       if ( basinModel->isModellingMode1D ()) {											  
          Temperature_Calculator.computeSmectiteIlliteIncrement ( Previous_Time, Current_Time );
          Temperature_Calculator.computeBiomarkersIncrement ( Previous_Time, Current_Time );
          Temperature_Calculator.collectFissionTrackSampleData( Current_Time );
       }

       Determine_Next_Coupled_Time_Step ( Current_Time, Time_Step, Number_Of_Newton_Iterations, Number_Of_Geometric_Iterations );

       computeBasementLithostaticPressureForCurrentTimeStep ( basinModel, Current_Time ); 

       Copy_Current_Properties ();
       Destroy_Vectors ();

       Save_Properties ( Current_Time );
       postTimeStepOperations ( Current_Time );
       Number_Of_Timesteps = Number_Of_Timesteps + 1;
    }

    if (basinModel->debug1 or basinModel->verbose or FastcauldronSimulator::getInstance ().getMcfHandler ().getDebugLevel () > 0 ) {
       PetscPrintf(PETSC_COMM_WORLD, " time for time-step: %f\n", (WallTime::clock () - startTime).floatValue() );
    }

    // printElementNeedle ( 1, 1 );
  }


  if (( basinModel->isGeometricLoop ()) && ( basinModel -> debug1 or basinModel->verbose )) {
    PetscPrintf ( PETSC_COMM_WORLD, " total System_Assembly_Time      %f \n", System_Assembly_Time );
    PetscPrintf ( PETSC_COMM_WORLD, " total Element_Assembly_Time     %f \n", Element_Assembly_Time );
    PetscPrintf ( PETSC_COMM_WORLD, " total System_Solve_Time         %f \n", System_Solve_Time );
    PetscPrintf ( PETSC_COMM_WORLD, " total Property_Calculation_Time %f \n", Property_Calculation_Time );
    PetscPrintf ( PETSC_COMM_WORLD, "\n Coupled Calculation Performed in %d Time Steps\n\n",
                  Number_Of_Timesteps);
  }

  Accumulated_System_Assembly_Time      = Accumulated_System_Assembly_Time      + System_Assembly_Time;
  Accumulated_Element_Assembly_Time     = Accumulated_Element_Assembly_Time     + Element_Assembly_Time;
  Accumulated_System_Solve_Time         = Accumulated_System_Solve_Time         + System_Solve_Time;
  Accumulated_Property_Calculation_Time = Accumulated_Property_Calculation_Time + Property_Calculation_Time;
}

//------------------------------------------------------------//

#undef  __FUNCT__
#define __FUNCT__ "Basin_Modelling::FEM_Grid::Save_Properties"

void Basin_Modelling::FEM_Grid::Save_Properties ( const double Current_Time ) {

   if ( Current_Time == 0.0 ) {
      Well wells( basinModel );
      wells.Save_Present_Day_Data ();
   }
  
   if ( ! basinModel->DoHDFOutput ) {
      return;
   }

   if ( basinModel->m_doOutputAtAge && Current_Time!=basinModel->m_ageToOutput) {
      return;
   }
    
  if (   ( Current_Time == (*majorSnapshots)->time ()  )
      || (    basinModel->isModellingMode1D()  
           && basinModel->projectSnapshots.isMinorSnapshot ( Current_Time, minorSnapshots ) ) ) // 1D model: save minor AND major timesteps
  {

    const Interface::Snapshot* snapshot = FastcauldronSimulator::getInstance ().findOrCreateSnapshot ( Current_Time );
    assert ( snapshot != 0 );

    // Compute the derived properties that are to be output.
    Temperature_Calculator.computeSnapShotVRe ( basinModel, Current_Time );

    if(  basinModel->isModellingMode1D() )
    {
      Temperature_Calculator.computeSnapShotSmectiteIllite ( Current_Time, basinModel->getValidNeedles ());
      Temperature_Calculator.computeSnapShotBiomarkers ( Current_Time, basinModel->getValidNeedles () );
    }

    if( basinModel->isALC() ) {
       basinModel->calcBasementProperties( Current_Time );
	}
  // save both map and volume data.
    if ( Current_Time == 0.0 && m_concludingVolumeOutputProperties.size() > 0)
       {
          m_combinedVolumeOutputProperties.reserve(m_concludingVolumeOutputProperties.size() + m_volumeOutputProperties.size() );
          m_combinedVolumeOutputProperties.insert(m_combinedVolumeOutputProperties.end(), m_volumeOutputProperties.begin(),
                                                  m_volumeOutputProperties.end());
          m_combinedVolumeOutputProperties.insert(m_combinedVolumeOutputProperties.end(), m_concludingVolumeOutputProperties.begin(),
                                                  m_concludingVolumeOutputProperties.end());
          FastcauldronSimulator::getInstance ().saveProperties ( mapOutputProperties,
                                                           m_combinedVolumeOutputProperties,
                                                           snapshot,
                                                           Interface::SEDIMENTS_AND_BASEMENT_OUTPUT );
       }
    else
       {
          FastcauldronSimulator::getInstance ().saveProperties ( mapOutputProperties,
                                                                 m_volumeOutputProperties,
                                                                 snapshot,
                                                              Interface::SEDIMENTS_AND_BASEMENT_OUTPUT );
       }

    if( basinModel->isModellingMode1D() )
    {
      collectAndSaveIsoValues(Current_Time, basinModel);
    }
	
    // Collect surface node properties.
    m_surfaceNodeHistory.Add_Time ( Current_Time );

    // Delete the vectors for derived properties as they are no longer required.
    Temperature_Calculator.deleteVReVectors ( basinModel );

    if(  basinModel->isModellingMode1D() )
    {
      Temperature_Calculator.deleteSmectiteIlliteVector ();
      Temperature_Calculator.deleteBiomarkersVectors ();
    }

    if (      basinModel->isModellingMode1D()  
          &&  Current_Time != (*majorSnapshots)->time () ) // 1D model: save minor snapshot
    {
       savedMinorSnapshotTimes.insert ( Current_Time );
    }

  } else {

    if ( basinModel->projectSnapshots.isMinorSnapshot ( Current_Time, minorSnapshots )) {
       const Interface::Snapshot* snapshot = FastcauldronSimulator::getInstance ().findOrCreateSnapshot ( Current_Time );
       assert ( snapshot != 0 );

      if ( ! basinModel->projectSnapshots.projectPrescribesMinorSnapshots ()) {
         FastcauldronSimulator::getInstance ().saveVolumeProperties ( looselyCoupledOutputProperties,
                                                                      snapshot,
                                                                      Interface::SEDIMENTS_AND_BASEMENT_OUTPUT );
        savedMinorSnapshotTimes.insert ( Current_Time );
      }

      Temperature_Calculator.computeSnapShotVRe ( basinModel, Current_Time );
      computeErosionFactorMaps ( basinModel, Current_Time );

       FastcauldronSimulator::getInstance ().saveMapProperties ( genexOutputProperties, snapshot, Interface::SEDIMENTS_ONLY_OUTPUT );
       Temperature_Calculator.deleteVReVectors ( basinModel );
       deleteErosionFactorMaps ( basinModel );
    }

    if ( m_surfaceNodeHistory.IsDefined ()) {

      Temperature_Calculator.computeSnapShotVRe ( basinModel, Current_Time );
      computePermeabilityVectors ( basinModel );
      computeThermalConductivityVectors ( basinModel );
      computeBulkDensityVectors ( basinModel );

      // Collect surface node properties.
      m_surfaceNodeHistory.Add_Time ( Current_Time );

      deleteThermalConductivityVectors ( basinModel );
      Temperature_Calculator.deleteVReVectors ( basinModel );
      deleteBulkDensityVectors ( basinModel );
    }

  }

}


//------------------------------------------------------------//


bool Basin_Modelling::FEM_Grid::Step_Forward (       double& Previous_Time, 
                                                     double& Current_Time,
                                                     double& Time_Step,
                                                     bool&   majorSnapshotTimesUpdated,
                                               const int     Number_Of_Newton_Iterations ) {

  majorSnapshotTimesUpdated = false;

  if ( Current_Time == Present_Day ) {
    // We are done!
    return false;
  }

  if ( Current_Time == (*majorSnapshots)->time ()) {
    // Step to the next snapshot time.
    ++majorSnapshots;
    majorSnapshotTimesUpdated = true;
  } else if ( basinModel->projectSnapshots.validMinorSnapshots ( minorSnapshots ) &&
              Current_Time <= (*minorSnapshots)->time ()) {
    basinModel->projectSnapshots.advanceMinorSnapshotIterator ( Current_Time, minorSnapshots );
//      ++minorSnapshots;
  }


  Previous_Time = Current_Time;
  Current_Time = Current_Time - Time_Step;

  // Do not want very small time step that may occur as we approach a snapshot time.
  // If the current time is just short of the snapshot, then set it to the snapshot time.
  if ( fabs(Current_Time - (*majorSnapshots)->time ()) < 0.0001 ) {
    Current_Time = (*majorSnapshots)->time ();
  } else if ( basinModel->projectSnapshots.projectPrescribesMinorSnapshots () &&
              basinModel->projectSnapshots.isAlmostSnapshot ( Current_Time, Time_Step, minorSnapshots, 0.1 )) {
    Current_Time = (*minorSnapshots)->time ();
  }

  if ( basinModel->projectSnapshots.projectPrescribesMinorSnapshots () && 
       basinModel->projectSnapshots.validMinorSnapshots ( minorSnapshots ) &&
       Current_Time < (*minorSnapshots)->time () &&
       (*minorSnapshots)->time () > (*majorSnapshots)->time ()) {
    Current_Time = (*minorSnapshots)->time ();
  } else if ( Current_Time < (*majorSnapshots)->time ()) {
    Current_Time = (*majorSnapshots)->time ();
  } 

  basinModel->adjustTimeStepToPermafrost ( Previous_Time, Current_Time );
  
  Time_Step = Previous_Time - Current_Time;

  return true;

}


//------------------------------------------------------------//

#undef  __FUNCT__  
#define __FUNCT__ "Basin_Modelling::FEM_Grid::Determine_Next_Pressure_Time_Step"

void Basin_Modelling::FEM_Grid::Determine_Next_Pressure_Time_Step ( const double  Current_Time,
                                                                          double& Time_Step,
                                                                    const int     Number_Of_Newton_Iterations,
                                                                    const int     numberOfGeometricIterations ) {

  if ( Current_Time == (*majorSnapshots)->time ()) {
    Time_Step = basinModel->getInitialTimeStep ( Current_Time );
  } else {
     const double Increase_Factor = basinModel -> timestepincr;
     const double Decrease_Factor = basinModel -> timestepdecr;

     double Optimal_Pressure_Difference = basinModel->optimalpressdiff;
     double Predicted_Time_Step;

     PetscScalar Maximum_Difference = pressureSolver->maximumPressureDifference ();

     MPI_Barrier(PETSC_COMM_WORLD);

     if ( basinModel -> Optimisation_Level >= 6 && Number_Of_Newton_Iterations >= pressureSolver->getMaximumNumberOfNonlinearIterations ( basinModel->Optimisation_Level )
          && Maximum_Difference > 0.25 * Optimal_Pressure_Difference ) {
        //    if ( basinModel -> Optimisation_Level >= 3 && Number_Of_Newton_Iterations >= Maximum_Number_Of_Nonlinear_Pressure_Iterations ()) {

        // Only allow the time step to decrease by at most a factor of a half.
        Predicted_Time_Step = Time_Step * NumericFunctions::Maximum ( 0.5, Decrease_Factor );
     } else if ( Maximum_Difference <= 0.0 ) {
        Predicted_Time_Step = Time_Step * Increase_Factor;
     } else if ( Maximum_Difference < Optimal_Pressure_Difference ) {
        Predicted_Time_Step = Time_Step * PetscMin ( Optimal_Pressure_Difference / Maximum_Difference, Increase_Factor );
     } else {
        Predicted_Time_Step = Time_Step * PetscMax ( Optimal_Pressure_Difference / Maximum_Difference, Decrease_Factor );    
     }

     if ( basinModel->cflTimeStepping ()) {
        double CFL_Value;

        Determine_CFL_Value ( CFL_Value );
        Time_Step = NumericFunctions::Maximum ( CFL_Value, Predicted_Time_Step );
     }

     Time_Step = NumericFunctions::Maximum ( Predicted_Time_Step, basinModel->minimumTimeStep ());
     Time_Step = NumericFunctions::Minimum ( Time_Step, basinModel->maximumTimeStep ());
  }

  Determine_Permafrost_Time_Step ( Current_Time, Time_Step );
}


//------------------------------------------------------------//

#undef  __FUNCT__  
#define __FUNCT__ "Basin_Modelling::FEM_Grid::Determine_Next_Temperature_Time_Step"

void Basin_Modelling::FEM_Grid::Determine_Next_Temperature_Time_Step ( const double  Current_Time,
                                                                             double& Time_Step ) 
{


   double Optimal_Temperature_Difference                 = basinModel -> optimaltempdiff;
   double Optimal_Temperature_Difference_In_Source_Rocks = basinModel -> optimalsrtempdiff;
   double Increase_Factor                                = basinModel -> timestepincr;
   double Decrease_Factor                                = basinModel -> timestepdecr;
   double Predicted_Time_Step;
   double Predicted_Time_Step_In_Source_Rocks;

   MPI_Barrier(PETSC_COMM_WORLD);

   if ( Current_Time == (*majorSnapshots)->time ()) {
      Time_Step = basinModel->getInitialTimeStep ( Current_Time );
   } else {
      PetscScalar Maximum_Difference;
  
      Maximum_Difference = Temperature_Calculator.Maximum_Temperature_Difference ();

      if ( basinModel -> debug1 or basinModel->verbose ) {
         PetscPrintf ( PETSC_COMM_WORLD, " Maximum temperature difference %f \n", Maximum_Difference );
      }

      if ( Maximum_Difference <= 0.0 )  {
         Predicted_Time_Step = Time_Step * Increase_Factor;
      } else if ( Maximum_Difference < Optimal_Temperature_Difference ) {
         Predicted_Time_Step = Time_Step * 
            PetscMin ( Optimal_Temperature_Difference / Maximum_Difference, Increase_Factor );
      } else {
         Predicted_Time_Step = Time_Step * 
            PetscMax ( Optimal_Temperature_Difference / Maximum_Difference, Decrease_Factor );    
      }

      Maximum_Difference = Temperature_Calculator.Maximum_Temperature_Difference_In_Source_Rocks ();

      if ( basinModel -> debug1 or basinModel->verbose ) {
         PetscPrintf ( PETSC_COMM_WORLD, " Maximum source rock difference %f \n", Maximum_Difference );
      }

      if ( Maximum_Difference <= 0.0 ) {
         Predicted_Time_Step_In_Source_Rocks = Time_Step * Increase_Factor;
      } else if ( Maximum_Difference < Optimal_Temperature_Difference_In_Source_Rocks ) {
         Predicted_Time_Step_In_Source_Rocks = Time_Step * 
            PetscMin ( Optimal_Temperature_Difference_In_Source_Rocks / Maximum_Difference, Increase_Factor );
      } else {
         Predicted_Time_Step_In_Source_Rocks = Time_Step * 
            PetscMax ( Optimal_Temperature_Difference_In_Source_Rocks / Maximum_Difference, Decrease_Factor );    
      }
 
      Time_Step = NumericFunctions::Minimum ( Predicted_Time_Step, Predicted_Time_Step_In_Source_Rocks );
      Time_Step = NumericFunctions::Minimum ( Time_Step, basinModel->maximumTimeStep ());

      if ( basinModel -> isALC() ) {
         Time_Step = NumericFunctions::Maximum ( Time_Step, basinModel->minimumTimeStep ());
      }
   }

   Determine_Permafrost_Time_Step ( Current_Time, Time_Step );
}


//------------------------------------------------------------//

#undef  __FUNCT__  
#define __FUNCT__ "Basin_Modelling::FEM_Grid::Determine_Next_Coupled_Time_Step"

void Basin_Modelling::FEM_Grid::Determine_Next_Coupled_Time_Step ( const double  Current_Time,
                                                                         double& Time_Step,
                                                                   const int     Number_Of_Overpressure_Newton_Iterations,
                                                                   const int     numberOfGeometricIterations ) {

  if ( Current_Time == (*majorSnapshots)->time ()) {
    Time_Step = basinModel->getInitialTimeStep ( Current_Time );
  } else {
     const double Increase_Factor = basinModel->timestepincr;
     const double Decrease_Factor = basinModel->timestepdecr;

     double Optimal_Pressure_Difference;
     double Optimal_Temperature_Difference;
     double Optimal_Temperature_Difference_In_Source_Rocks;

     double Predicted_Overpressure_Time_Step;
     double Predicted_Temperature_Time_Step;
     double Predicted_Source_Rock_Time_Step;

     Optimal_Pressure_Difference = basinModel->optimalpressdiff;
     Optimal_Temperature_Difference                 = basinModel->optimaltempdiff;
     Optimal_Temperature_Difference_In_Source_Rocks = basinModel->optimalsrtempdiff;

     PetscScalar Maximum_Overpressure_Difference = pressureSolver->maximumPressureDifference ();
     PetscScalar Maximum_Temperature_Difference  = Temperature_Calculator.Maximum_Temperature_Difference ();
     PetscScalar Maximum_Source_Rock_Difference  = Temperature_Calculator.Maximum_Temperature_Difference_In_Source_Rocks ();

     MPI_Barrier(PETSC_COMM_WORLD);

     if ( basinModel -> debug1 or basinModel->verbose ) {
        PetscPrintf ( PETSC_COMM_WORLD, " Maximum OVERPRESSURE difference %f \n", Maximum_Overpressure_Difference );
        PetscPrintf ( PETSC_COMM_WORLD, " Maximum TEMPERATURE  difference %f \n", Maximum_Temperature_Difference );
        PetscPrintf ( PETSC_COMM_WORLD, " Maximum SOURCE ROCK  difference %f \n", Maximum_Source_Rock_Difference );
     }

     if ( Maximum_Overpressure_Difference <= 0.0 ) {
        Predicted_Overpressure_Time_Step = Time_Step * Increase_Factor;
     } else if ( Maximum_Overpressure_Difference < Optimal_Pressure_Difference ) {
        Predicted_Overpressure_Time_Step = Time_Step * PetscMin ( Optimal_Pressure_Difference / Maximum_Overpressure_Difference, 
                                                                  Increase_Factor );
     } else {
        Predicted_Overpressure_Time_Step = Time_Step * PetscMax ( Optimal_Pressure_Difference / Maximum_Overpressure_Difference, 
                                                                  Decrease_Factor );    
     }

     // Determine the best TEMPERATURE time step
     if ( Maximum_Temperature_Difference <= 0.0 ) {
        Predicted_Temperature_Time_Step = Time_Step * Increase_Factor;
     } else if ( Maximum_Temperature_Difference < Optimal_Temperature_Difference ) {
        Predicted_Temperature_Time_Step = Time_Step * PetscMin ( Optimal_Temperature_Difference / Maximum_Temperature_Difference,
                                                                 Increase_Factor );
     } else {
        Predicted_Temperature_Time_Step = Time_Step * PetscMax ( Optimal_Temperature_Difference / Maximum_Temperature_Difference, 
                                                                 Decrease_Factor );    
     }

     // Determine the best time step in SOURCE ROCKS
     if ( Maximum_Source_Rock_Difference <= 0.0 ) {
        Predicted_Source_Rock_Time_Step = Time_Step * Increase_Factor;
     } else if ( Maximum_Source_Rock_Difference < Optimal_Temperature_Difference_In_Source_Rocks ) {
        Predicted_Source_Rock_Time_Step = Time_Step * PetscMin ( Optimal_Temperature_Difference_In_Source_Rocks / Maximum_Source_Rock_Difference, 
                                                                 Increase_Factor );
     } else {
        Predicted_Source_Rock_Time_Step = Time_Step * PetscMax ( Optimal_Temperature_Difference_In_Source_Rocks / Maximum_Source_Rock_Difference, 
                                                                 Decrease_Factor );    
     }

     // Find the MINIMUM of these three predicted time steps.
     Time_Step = NumericFunctions::Minimum3 ( Predicted_Temperature_Time_Step, Predicted_Source_Rock_Time_Step, Predicted_Overpressure_Time_Step );

     // This MUST be greater than the minimum allowed time step.
     if ( basinModel->cflTimeStepping ()) {
        double CFL_Value;
        Determine_CFL_Value ( CFL_Value );
        Time_Step = NumericFunctions::Maximum3 ( CFL_Value, Time_Step, basinModel->minimumTimeStep ());
     } else {
        Time_Step = NumericFunctions::Maximum ( Time_Step, basinModel->minimumTimeStep ());
     }

     Time_Step = NumericFunctions::Minimum ( Time_Step, basinModel->maximumTimeStep ());
  }

  Determine_Permafrost_Time_Step ( Current_Time, Time_Step );
}


//------------------------------------------------------------//

#undef  __FUNCT__  
#define __FUNCT__ "Basin_Modelling::FEM_Grid::Determine_CFL_Value"

void Basin_Modelling::FEM_Grid::Determine_CFL_Value ( double& CFL_Value ) {

  PetscLogDouble Start_Time;
  PetscLogDouble End_Time;
  PetscLogDouble Accumulated_Time = 0.0;

  double Global_CFL_Value;
  double Layer_CFL_Value;
  Layer_Iterator Pressure_Layers;
  // Allocate the property vectors in each of the layers
  Pressure_Layers.Initialise_Iterator ( basinModel->layers, Descending, Sediments_Only, Active_Layers_Only );

  CFL_Value = 100.0 * Secs_IN_MA;

  while ( ! Pressure_Layers.Iteration_Is_Done ()) {

    PetscTime(&Start_Time);
    Pressure_Layers.Current_Layer () -> Determine_CFL_Value ( basinModel, Layer_CFL_Value );
    PetscTime(&End_Time);

    Accumulated_Time = Accumulated_Time + End_Time - Start_Time;

    MPI_Allreduce ( &Layer_CFL_Value, &Global_CFL_Value, 1, 
                     MPI_DOUBLE, MPI_MIN, PETSC_COMM_WORLD );

    if ( basinModel -> debug1 or basinModel->verbose ) {
      PetscPrintf(PETSC_COMM_WORLD, " CFL Value: %30s  %14f  %15f\n", 
           Pressure_Layers.Current_Layer () -> layername.c_str(),
           Global_CFL_Value ,
           End_Time - Start_Time
        );
    }

    CFL_Value = NumericFunctions::Minimum ( CFL_Value, Global_CFL_Value );
    Pressure_Layers++;
  }

  if ( basinModel -> debug1 or basinModel->verbose) {

    PetscPrintf ( PETSC_COMM_WORLD, " Basin CFL Value:  %3.4f  %2.4f \n", CFL_Value, Accumulated_Time );
  }

}


//------------------------------------------------------------//

#undef  __FUNCT__  
#define __FUNCT__ "Basin_Modelling::FEM_Grid::Construct_Pressure_FEM_Grid"

void Basin_Modelling::FEM_Grid::Construct_Pressure_FEM_Grid ( const double Previous_Time,
                                                              const double Current_Time ) {

  int  Number_Of_Segments = 0;

  Layer_Iterator Pressure_Layers;

  // Allocate the property vectors in each of the layers
  Pressure_Layers.Initialise_Iterator ( basinModel->layers, Descending, Basement_And_Sediments, Active_Layers_Only );

  while ( ! Pressure_Layers.Iteration_Is_Done ()) {

    Pressure_Layers.Current_Layer ()->allocateNewVecs ( basinModel );

    Pressure_Layers++;
  }

  // Now that the layer DA's have been set we can fill the Topmost Active Segment arrays
  basinModel -> Fill_Topmost_Segment_Arrays ( Previous_Time, Current_Time );

  // Count the number of active segments in the Basin (remembering that the basement is not included)
  // AND find the maximum number of segments in any active layer.
  Pressure_Layers.Initialise_Iterator ( basinModel->layers, Ascending, Sediments_Only, Active_Layers_Only );

  while ( ! Pressure_Layers.Iteration_Is_Done ()) {
    Number_Of_Segments = Number_Of_Segments + Pressure_Layers.Current_Layer ()->getNrOfActiveElements();
    Pressure_Layers++;
  }

  if ( Number_Of_Segments != 0 ) {
    Number_Of_Pressure_Z_Nodes = Number_Of_Segments + 1;
  } else {
    PetscPrintf(PETSC_COMM_WORLD,"No Active Segments");
  }

  // Compute the total number of nodes in the PETSc mesh (this may be different from number of nodes in the true fem mesh)
  Total_Number_Of_Pressure_Nodes = Number_Of_X_Nodes * Number_Of_Y_Nodes * Number_Of_Pressure_Z_Nodes;

  FastcauldronSimulator::DACreate3D ( Number_Of_Pressure_Z_Nodes, Pressure_FEM_Grid );

  m_domainElements.construct ( FastcauldronSimulator::getInstance ().getElementGrid (),
                               Number_Of_Segments,
                               NumberOfPVTComponents );

#if 1
  m_elementRefs.create ( m_domainElements.getDa ());
#endif

  DMCreateGlobalVector ( Pressure_FEM_Grid, &Pressure_Depths );
  DMCreateGlobalVector ( Pressure_FEM_Grid, &Pressure_DOF_Numbers );
  DMCreateGlobalVector ( Pressure_FEM_Grid, &pressureNodeIncluded );

  // This is really to initialise the properties (perhaps create a separate function to do this)
  pressureSolver->initialisePressureProperties ( Previous_Time, Current_Time );
  pressureSolver->computeDependantProperties ( Previous_Time, Current_Time, false );
}

//------------------------------------------------------------//

#undef  __FUNCT__  
#define __FUNCT__ "Basin_Modelling::FEM_Grid::Construct_Temperature_FEM_Grid"

void Basin_Modelling::FEM_Grid::Construct_Temperature_FEM_Grid ( const double                    Previous_Time,
								 const double                    Current_Time,
                                                                 const SnapshotEntrySetIterator& majorSnapshotTimes,
                                                                 const bool                      majorSnapshotTimesUpdated ) {

  int  Number_Of_Segments = 0;

  Layer_Iterator Layers;

  // Allocate the property vectors in each of the layers
  Layers.Initialise_Iterator ( basinModel -> layers, Descending, Basement_And_Sediments, Active_Layers_Only );

  while ( ! Layers.Iteration_Is_Done () ) 
  {
    Layers.Current_Layer ()->allocateNewVecs ( basinModel );

    Layers++;
  } 

  // If the calculation mode is the loosely coupled then we need to set-up the interpolators for the
  // properties that are to be read in and used in the temperature calcualtion.
  if ( majorSnapshotTimesUpdated && basinModel->DoTemperature ) {
    SnapshotInterval interval;

    // Set the snapshots that are to be interpolated (from the current major snapshot back to 
    // the previous major snapshot, including all the minor snapshots in between).
    basinModel->projectSnapshots.getPrecedingSnapshotInterval ( majorSnapshotTimes, true, interval );

    // Now, read in all the snapshot data and compute the interpolants.
    basinModel->setSnapshotInterval ( interval );
  }

  // Count the number of active segments in the Basin ( including the basement )
  // AND find the maximum number of segments in any active layer.
  Layers.Initialise_Iterator ( basinModel->layers, Ascending, Basement_And_Sediments, Active_Layers_Only );

  while ( ! Layers.Iteration_Is_Done () ) 
  {
    Number_Of_Segments = Number_Of_Segments + Layers.Current_Layer ()->getNrOfActiveElements();
    Layers++;
  } 

  if ( Number_Of_Segments != 0 ) 
  {
    Number_Of_Temperature_Z_Nodes = Number_Of_Segments + 1;
  } 
  else 
  {
    PetscPrintf(PETSC_COMM_WORLD,"No Active Segments");
  } 

  // Compute the total number of nodes in the PETSc mesh 
  // This may be different from number of nodes in the true fem mesh
  Total_Number_Of_Temperature_Nodes = Number_Of_X_Nodes * Number_Of_Y_Nodes * Number_Of_Temperature_Z_Nodes;

  FastcauldronSimulator::DACreate3D ( Number_Of_Temperature_Z_Nodes, Temperature_FEM_Grid );

  DMCreateGlobalVector ( Temperature_FEM_Grid, &Temperature_Depths );
  DMCreateGlobalVector ( Temperature_FEM_Grid, &Temperature_DOF_Numbers );

  Temperature_Calculator.setSurfaceTemperature ( basinModel, Current_Time );
  Set_Pressure_Dependent_Properties ( Current_Time );
  Compute_Temperature_Dependant_Properties ( Previous_Time, Current_Time );

}


//------------------------------------------------------------//

#undef  __FUNCT__  
#define __FUNCT__ "Basin_Modelling::FEM_Grid::Construct_FEM_Grid"

void Basin_Modelling::FEM_Grid::Construct_FEM_Grid ( const double                    Previous_Time,
                                                     const double                    Current_Time,
                                                     const SnapshotEntrySetIterator& majorSnapshotTimes,
                                                     const bool                      majorSnapshotTimesUpdated ) {

  // Set the number of active element in the layer thickness
  basinModel->findActiveElements ( Current_Time );

  if ( basinModel -> DoTemperature || basinModel -> Do_Iteratively_Coupled ) {
    Construct_Temperature_FEM_Grid ( Previous_Time, Current_Time, majorSnapshotTimes, majorSnapshotTimesUpdated );
    Temperature_Calculator.Compute_Heat_Flow_Boundary_Conditions ( Current_Time );
  }
  
  if ( basinModel -> DoOverPressure || basinModel -> Do_Iteratively_Coupled ) {
    Construct_Pressure_FEM_Grid ( Previous_Time, Current_Time );
  }

  FastcauldronSimulator::getInstance ().switchLithologies ( Current_Time );

  // Now that the lithologies have been 'switched' the 
  // include-node arrays have to be updated.  
  basinModel->SetIncludedNodeArrays ();

  setDOFs ();

  setLayerElements ( Current_Time );


  if ( basinModel -> DoOverPressure || basinModel -> Do_Iteratively_Coupled ) {
     pressureSolver->setLayerElements ( Pressure_FEM_Grid,
                                        Pressure_DOF_Numbers,
                                        m_elementRefs );
  } 

}

//------------------------------------------------------------//

void Basin_Modelling::FEM_Grid::setLayerElements ( const double age ) {

   Layer_Iterator basinLayers ( basinModel->layers, Descending, Sediments_Only, Active_And_Inactive_Layers );
   // Layer_Iterator basinLayers ( basinModel->layers, Descending, Sediments_Only, Active_Layers_Only );
   LayerProps_Ptr currentLayer;

   while ( not basinLayers.Iteration_Is_Done ()) {
      currentLayer = basinLayers.Current_Layer ();
      currentLayer->setLayerElementActivity ( age );
      basinLayers++;
   }

}

//------------------------------------------------------------//

#undef  __FUNCT__  
#define __FUNCT__ "Basin_Modelling::FEM_Grid::Integrate_Chemical_Compaction"

void Basin_Modelling::FEM_Grid::Integrate_Chemical_Compaction ( const double Previous_Time,
                                                                const double Current_Time ) {

  if ( ! basinModel -> Do_Chemical_Compaction ) {
    return;
  }

  Layer_Iterator Basin_Layers ( basinModel->layers, Descending, Sediments_Only, Active_Layers_Only );
  LayerProps_Ptr Current_Layer;

  while ( ! Basin_Layers.Iteration_Is_Done ()) {
    Current_Layer = Basin_Layers.Current_Layer ();
    Current_Layer -> Integrate_Chemical_Compaction ( Previous_Time, Current_Time, basinModel->getValidNeedles ());
    Basin_Layers++;
  }

}


//------------------------------------------------------------//

#undef  __FUNCT__  
#define __FUNCT__ "Basin_Modelling::FEM_Grid::integrateGenex"

void Basin_Modelling::FEM_Grid::integrateGenex ( const double previousTime,
                                                 const double currentTime ) {

   if ( not basinModel->integrateGenexEquations ()) {
      return;
   }

   Layer_Iterator Basin_Layers ( basinModel->layers, Descending, Sediments_Only, Active_Layers_Only );
   LayerProps_Ptr currentLayer;

   while ( ! Basin_Layers.Iteration_Is_Done ()) {
      currentLayer = Basin_Layers.Current_Layer ();
      currentLayer->integrateGenexEquations ( previousTime, currentTime );
      Basin_Layers++;
   }

}

//------------------------------------------------------------//

#undef  __FUNCT__  
#define __FUNCT__ "Basin_Modelling::FEM_Grid::clearGenexOutput"

void Basin_Modelling::FEM_Grid::clearGenexOutput () {

   if ( not basinModel->integrateGenexEquations ()) {
      return;
   }

   Layer_Iterator Basin_Layers ( basinModel->layers, Descending, Sediments_Only, Active_Layers_Only );
   LayerProps_Ptr Current_Layer;

   while ( ! Basin_Layers.Iteration_Is_Done ()) {
      Current_Layer = Basin_Layers.Current_Layer ();
      Current_Layer->clearGenexOutput ();
      Basin_Layers++;
   }

}

//------------------------------------------------------------//

void Basin_Modelling::FEM_Grid::zeroTransportedMass ( const double currentTime ) {

   if ( not FastcauldronSimulator::getInstance ().getMcfHandler ().solveFlowEquations ()) {
      // If not doing a Darcy simulation then return.
      return;
   }

   if ( currentTime != (*majorSnapshots)->time ()) {
      // If the current time is not a snapshot time then do not zero the transported mass vectors.
      return;
   }

   Layer_Iterator Basin_Layers ( basinModel->layers, Descending, Sediments_Only, Active_Layers_Only );
   LayerProps_Ptr Current_Layer;

   while ( ! Basin_Layers.Iteration_Is_Done ()) {
      Current_Layer = Basin_Layers.Current_Layer ();
      Current_Layer->zeroTransportedMass ();
      Basin_Layers++;
   }

}

//------------------------------------------------------------//

void Basin_Modelling::FEM_Grid::postTimeStepOperations ( const double currentTime ) {
   clearGenexOutput ();
   zeroTransportedMass ( currentTime );
}

//------------------------------------------------------------//

#undef  __FUNCT__  
#define __FUNCT__ "Basin_Modelling::FEM_Grid::RecomputeJacobian"

bool Basin_Modelling::FEM_Grid::RecomputeJacobian ( const int iterationCount,
                                                    const int optimisationLevel ) const {

#if 0
  static const int MinimumInitialJacobianComputations [ 3 ] = { 2, 2, 4 };
  const int RecomputeJacobianEveryNthIteration  = 8;
#endif

  bool recompute;

  if ( basinModel->allowPressureJacobianReuse ) {

     if ( iterationCount <= 3 ) {
        recompute = true;
     } else if ( iterationCount % basinModel->pressureJacobianReuseCount == 0 ) {
        recompute = true;
     } else {
        recompute = false;
     }

  } else {
     recompute = true;
  }

#if 0
  if ( optimisationLevel >= 4 ) {

    // Here we require higher accuracy over speed, therefore ALWAYS compute the Jacobian.
    recompute = true;
  } else {
    recompute = ( iterationCount < MinimumInitialJacobianComputations [ optimisationLevel - 1 ] ) || 
                ( iterationCount % RecomputeJacobianEveryNthIteration ) == 0;
  }
#endif

  return recompute;
}


//------------------------------------------------------------//

#undef  __FUNCT__  
#define __FUNCT__ "Basin_Modelling::FEM_Grid::Solve_Pressure_For_Time_Step"

void Basin_Modelling::FEM_Grid::Solve_Pressure_For_Time_Step ( const double  Previous_Time,
                                                               const double  Current_Time,
                                                               const int     MaximumNumberOfNonlinearIterations,
                                                                     bool&   overpressureHasDiverged,
                                                                     int&    Number_Of_Nonlinear_Iterations,
                                                                     double& Po_Norm,
                                                                     bool&   fracturingOccurred ) {

  PetscLogStages :: push(PetscLogStages :: PRESSURE_LINEAR_SOLVER);

  const int MaximumNumberOfFractureIterations = HydraulicFracturingManager::getInstance ().maximumNumberOfFractureIterations ();


  int Old_Precision;

  ios::fmtflags Old_Flags;

  const int Minimum_Number_Of_Nonlinear_Iterations = 3;

  Old_Precision = cout.precision ( 8 );
  Old_Flags     = cout.flags ( ios::scientific );


  if (basinModel -> debug1 or basinModel->verbose) {
    PetscPrintf ( PETSC_COMM_WORLD, " Current time  %f \n", Current_Time );
  }

  int totalNumberOfNonlinearIterations = 0;
  int numberOfLinearIterations;

  Mat Jacobian;
  Vec Residual;
  Vec Overpressure;
  Vec Residual_Solution;


  PetscLogDouble Start_Time;
  PetscLogDouble End_Time;
  PetscLogDouble timeStepCalculationTime;
  PetscLogDouble Total_Solve_Time = 0.0;
  PetscLogDouble Element_Contributions_Time;

  PetscLogDouble Assembly_End_Time;
  PetscLogDouble Assembly_Start_Time;

  PetscLogDouble Iteration_Start_Time;
  PetscLogDouble Iteration_End_Time;
  PetscLogDouble Iteration_Time;
  PetscLogDouble Total_Iteration_Time = 0.0;

  PetscLogDouble Jacobian_Start_Time;
  PetscLogDouble Jacobian_End_Time;
  PetscLogDouble Jacobian_Time;
  PetscLogDouble Total_Jacobian_Time = 0.0;

  bool Converged = false;
  bool applyNonConservativeModel = false;

  // The default PETSc ILU-fill levels.
  int iluFillLevels = 0;

  int gmresRestartValue = PressureSolver::DefaultGMResRestartValue;
  int linearSolverIterationCount = PressureSolver::DefaultMaximumPressureLinearSolverIterations;
  int linearSolverTotalIterationCount;
  int linearSolveAttempts;

  bool isDefaultSolver = true;

  PetscScalar Residual_Solution_Length;

  // Initialised to 1, to stop the compiler from complaining. This initialisation is legitimate
  // because this variable IS assigned to before use, it is used in the 3rd and subsequent iterations.
  PetscScalar Residual_Length = 1.0;
  PetscScalar Previous_Residual_Length = 1.0;
  PetscScalar Solution_Length;
  PetscReal   linearSolverResidualNorm;

  KSP Pressure_Linear_Solver;
  // SLES Pressure_Linear_Solver;
  // KSP  pressureLinearSolverKsp;
  KSPConvergedReason convergedReason;

  bool JacobianComputed;
  bool changedSolver = false;

  basinModel -> setPressureLinearSolver ( Pressure_Linear_Solver,
                                           pressureSolver->linearSolverTolerance ( basinModel->Optimisation_Level ));

#if PETSc_MATRIX_BUG_FIXED
  pressureSolver->createMatrixStructure ( *basinModel -> mapDA,
                                          Pressure_FEM_Grid,
                                          basinModel->getValidNeedles (),
                                          Pressure_DOF_Numbers, 
                                          &Jacobian,
                                          pressureStencilWidth );

#else
//    MatCreate ( PETSC_COMM_WORLD, PETSC_DECIDE, PETSC_DECIDE, Total_Number_Of_Pressure_Nodes, Total_Number_Of_Pressure_Nodes, &Jacobian );
  DMCreateMatrix ( Pressure_FEM_Grid, MATAIJ, &Jacobian );
#endif

  DMCreateGlobalVector ( Pressure_FEM_Grid, &Overpressure );

  VecDuplicate( Overpressure, &Residual );
  VecDuplicate( Overpressure, &Residual_Solution );


  PetscViewerSetFormat(PETSC_VIEWER_STDOUT_WORLD, PETSC_VIEWER_ASCII_MATLAB );

  pressureSolver->restorePressureSolution ( Pressure_FEM_Grid, Pressure_DOF_Numbers, Overpressure );

  PetscScalar Previous_Po_Norm;
  PetscScalar Previous_TS_Po_Norm;

  PetscScalar First_Po_Norm;

  PetscScalar Theta;
  PetscScalar Theta_Increment;


  VecNorm ( Overpressure, NORM_2, &Po_Norm );
  Previous_TS_Po_Norm = Po_Norm;

  if (basinModel -> debug1 or basinModel->verbose ) {
    PetscPrintf ( PETSC_COMM_WORLD, " Number of DOFs  =  %d \n", Total_Number_Of_Pressure_Nodes );
    PetscPrintf ( PETSC_COMM_WORLD, " Initial Po norm =  %f \n", Po_Norm );
  }

  if (basinModel -> debug1 or basinModel->verbose ) {
    PetscPrintf(PETSC_COMM_WORLD, "%193s\n", "    Jacobian      Linear solver     Iteration   ");
    PetscPrintf(PETSC_COMM_WORLD, "%193s\n", "  --------------  --------------  --------------");
  }

  bool hasFractured;

  int fractureIterations = 1;

  HydraulicFracturingManager::getInstance ().resetFracturing ();
  overpressureHasDiverged  = false;
  fracturingOccurred = false;

  do {

    Previous_Po_Norm = 0.0;
    Previous_TS_Po_Norm = 0.0;
    First_Po_Norm = 0.0;
    Theta_Increment = 0.05;

    Number_Of_Nonlinear_Iterations = 0;
    Converged = false;

    if ( basinModel -> debug1 or basinModel->verbose) {
      PetscPrintf ( PETSC_COMM_WORLD,
                    "Starting fracture iteration  %i  of  %i \n",
                    fractureIterations,
                    MaximumNumberOfFractureIterations );
    }

    while ( ! Converged && ! overpressureHasDiverged ) {
      PetscTime(&Iteration_Start_Time);

      VecSet ( Residual, Zero );

      PetscTime(&Jacobian_Start_Time);


      // Compute the Jacobian and residual for the nonlinear solver (Newton).
      MatZeroEntries ( Jacobian ); 
      PetscTime(&Assembly_Start_Time);
      pressureSolver->assembleSystem ( Previous_Time, Current_Time, 
                                       Pressure_FEM_Grid, 
                                       Pressure_DOF_Numbers, 
                                       pressureNodeIncluded,
                                       Jacobian, Residual,
                                       Element_Contributions_Time );
      PetscTime(&Assembly_End_Time);

      Element_Assembly_Time = Element_Assembly_Time + Element_Contributions_Time;
      System_Assembly_Time = System_Assembly_Time + Assembly_End_Time - Assembly_Start_Time;

      PetscTime(&Start_Time);

      KSPSetOperators ( Pressure_Linear_Solver, Jacobian, Jacobian, SAME_NONZERO_PATTERN );

#if 0
      if ( Number_Of_Nonlinear_Iterations <= basinModel->pressureJacobianReuseCount ) {
         KSPSetOperators ( Pressure_Linear_Solver, Jacobian, Jacobian, SAME_NONZERO_PATTERN );
      } else {
         KSPSetOperators ( Pressure_Linear_Solver, Jacobian, Jacobian, SAME_PRECONDITIONER );
      }
#endif

      JacobianComputed = true;

      PetscTime(&Jacobian_End_Time);
      Jacobian_Time = Jacobian_End_Time - Jacobian_Start_Time; 
      Total_Jacobian_Time = Total_Jacobian_Time + Jacobian_Time; 

      VecSet ( Residual_Solution, Zero );

      // Solve the matrix equation (Jacobian^{-1} \times residual) to some acceptable tolerance.
      KSPSolve ( Pressure_Linear_Solver, Residual, Residual_Solution );

      KSPGetIterationNumber ( Pressure_Linear_Solver, &numberOfLinearIterations );
      KSPGetConvergedReason ( Pressure_Linear_Solver, &convergedReason );
      // First attempt to solve linear system of equations.
      linearSolveAttempts = 1;

      if ( convergedReason < 0 ) {
         KSPGetIterationNumber ( Pressure_Linear_Solver, &numberOfLinearIterations );
         linearSolverTotalIterationCount = numberOfLinearIterations;

         PetscPrintf ( PETSC_COMM_WORLD,
                       " MeSsAgE WARNING The pressure solver exit condition was: %s. Retrying with another linear solver. \n",
                       getKspConvergedReasonImage ( convergedReason ).c_str ());

         // Now iterate several times until the linear system has been solved.
         // If, however, the number of iterations exceeds the maximum then this will result in a simulation failure.
         // On the first iteration the linear solver is switched to gmres.
         // On subsequent iterations the restart level and the maximum number of iterations are both increased,
         for ( int linearSolveLoop = 1; linearSolveLoop <= PressureSolver::MaximumLinearSolveAttempts and convergedReason < 0; ++linearSolveLoop, ++linearSolveAttempts ) {

            if ( not changedSolver ) {
               KSPType currentLinearSolverType;
               KSPGetType ( Pressure_Linear_Solver, &currentLinearSolverType );

               if ( strcmp ( currentLinearSolverType, KSPGMRES ) != 0 ) {
                  // It is possible to select gmres from the command line then it is not necessary to change it.
                  setLinearSolverType ( Pressure_Linear_Solver, KSPGMRES );
               }

               // If gmres has been selected frmo the command line or config file then
               // use at least the restart value that has been selected.
               int currentGMResRestart;
               KSPGMRESGetRestart ( Pressure_Linear_Solver, &currentGMResRestart );
               gmresRestartValue = std::max ( currentGMResRestart, PressureSolver::DefaultGMResRestartValue );

               // Set at least the current number of iterations defined for the linear solver.
               int currentMaximumNumberOfIterations = getSolverMaxIterations ( Pressure_Linear_Solver );
               linearSolverIterationCount = std::max ( currentMaximumNumberOfIterations, PressureSolver::DefaultMaximumPressureLinearSolverIterations );

               // Indicate that the linear solver has been changed.
               changedSolver = true;
            } else {
               gmresRestartValue += PressureSolver::GMResRestartIncrementValue;
               linearSolverIterationCount = ( 3 * linearSolverIterationCount ) / 2;
            }

            KSPGMRESSetRestart ( Pressure_Linear_Solver, gmresRestartValue );
            setSolverMaxIterations ( Pressure_Linear_Solver, linearSolverIterationCount );

            KSPSetOperators ( Pressure_Linear_Solver, Jacobian, Jacobian, SAME_NONZERO_PATTERN );
            KSPSolve ( Pressure_Linear_Solver, Residual, Residual_Solution );
            KSPGetIterationNumber ( Pressure_Linear_Solver, &numberOfLinearIterations );
            KSPGetConvergedReason ( Pressure_Linear_Solver, &convergedReason );

            linearSolverTotalIterationCount += numberOfLinearIterations;

            if ( basinModel->debug1 ) {
               PetscPrintf ( PETSC_COMM_WORLD, " The re-tried (%i) pressure solver exit condition was: %s \n", linearSolveLoop, getKspConvergedReasonImage ( convergedReason ).c_str ());
            }

         }

      }

#if 0
      if ( basinModel -> debug2 && numberOfLinearIterations == 0 ) {
        PetscViewer    viewer;
        PetscViewerCreate( PETSC_COMM_WORLD, &viewer);
        PetscViewerSetType(viewer, PETSC_VIEWER_ASCII );
        PetscViewerSetFormat(viewer, PETSC_VIEWER_ASCII_MATLAB );
        MatView ( Jacobian, viewer );
        VecView ( Residual, viewer );
      }
#endif

#if 0
      if ( FastcauldronSimulator::getInstance ().getModellingMode () == Interface::MODE3D ) {
         // Only update solver/preconditioner in 3d mode.

         if ( basinModel->allowSolverChange and 
              ( numberOfLinearIterations <= 2 or numberOfLinearIterations == PressureSolver::DefaultMaximumPressureLinearSolverIterations )
              and isDefaultSolver ) {

            // Change the linear solver to gmres (from cg). This sometimes helps (but is generally slower).
            setLinearSolverType ( Pressure_Linear_Solver,
                                  KSPGMRES );

            KSPSetOperators ( Pressure_Linear_Solver, Jacobian, Jacobian, SAME_NONZERO_PATTERN );
            KSPSolve ( Pressure_Linear_Solver, Residual, Residual_Solution );
            isDefaultSolver = false;
            changedSolver = true;
         } else if ( basinModel->allowIluFillIncrease and
                     numberOfLinearIterations >= PressureSolver::getIterationsForIluFillLevelIncrease ( basinModel->Optimisation_Level, iluFillLevels ) and 
                     iluFillLevels < 4 ) {

            iluFillLevels += 2;

            // Some problem with this function
            // KSPSetUp ( Pressure_Linear_Solver, Residual, Residual_Solution );
            setPreconditionerFillLevels ( Pressure_Linear_Solver,
                                          iluFillLevels );

            changedSolver = true;
         }

      }
#endif

      PetscTime(&End_Time);
      timeStepCalculationTime = End_Time - Start_Time; 
      Total_Solve_Time = Total_Solve_Time + timeStepCalculationTime;

      System_Solve_Time = System_Solve_Time + timeStepCalculationTime;

      if ( totalNumberOfNonlinearIterations == 0 ) {

        // Set theta to 0.5, so that the initial update is not too big a jump.
        // This helps to improve the solving on, especially, layers with high deposition rates.
        Theta = -0.5;
      } else if ( totalNumberOfNonlinearIterations <= 2 ) {

        // Set to 1, since the initial value is 0.5 only to help with the initial pressure solution.
        Theta = -1.0;
      } else if ( totalNumberOfNonlinearIterations > 2 ) {

        if ( Previous_Residual_Length < Residual_Length ) {
          Theta = 0.5 * Theta;
          Theta = PetscMin ( Theta, -0.1 );
        } else {
          Theta = PetscMax ( -1.0, Theta - Theta_Increment );
        }

      }

      Previous_Residual_Length = Residual_Length;

      // Update the overpressure solution. p_{n+1} = p_{n} - \theta \times R(p_{n}) / Jac(p_{n})
      // VecAXPY(&Theta, Residual_Solution, Overpressure );
      // Check this!!
      VecAXPY( Overpressure, Theta, Residual_Solution );
      VecNorm ( Overpressure, NORM_2, &Po_Norm );

      pressureSolver->storePressureSolution ( Pressure_FEM_Grid, Pressure_DOF_Numbers, Overpressure );

      VecNorm ( Overpressure, NORM_2, &Solution_Length );
      VecNorm ( Residual_Solution, NORM_2, &Residual_Solution_Length );
      VecNorm ( Residual, NORM_2, &Residual_Length );

      if ( convergedReason < 0 ) {
         PetscPrintf ( PETSC_COMM_WORLD, " MeSsAgE ERROR pressure solver exit condition was: %s \n",
                       getKspConvergedReasonImage ( convergedReason ).c_str ());
         PetscPrintf ( PETSC_COMM_WORLD, " MeSsAgE ERROR the linear solver could not converge to a solution after %i attempts and %i iterations. \n",
                       linearSolveAttempts, 
                       linearSolverTotalIterationCount );
         overpressureHasDiverged = true;
      } else {
         // If solver has converged now check that none of the vectors contain a nan.
         overpressureHasDiverged = isnan ( Po_Norm ) || isnan ( Solution_Length ) || isnan ( Residual_Solution_Length ) || isnan ( Residual_Length );

         if ( overpressureHasDiverged ) {
            PetscPrintf ( PETSC_COMM_WORLD, " MeSsAgE ERROR pressure solution contains a NaN. \n");
         }

      }

      if ( Number_Of_Nonlinear_Iterations == 0 ) {
        First_Po_Norm = Solution_Length;
      }

      if ( overpressureHasDiverged ) {
        PetscPrintf ( PETSC_COMM_WORLD, " Overpressure calculation has diverged.\n" );
      } else {
        pressureSolver->computeDependantProperties ( Previous_Time, Current_Time, false );
      }

      PetscTime(&Iteration_End_Time);
      Iteration_Time = Iteration_End_Time - Iteration_Start_Time; 
      Total_Iteration_Time = Total_Iteration_Time + Iteration_Time; 
      KSPGetResidualNorm ( Pressure_Linear_Solver, &linearSolverResidualNorm );

      if (( basinModel -> debug1 || basinModel->verbose ) && ( FastcauldronSimulator::getInstance ().getRank () == 0 )) 
      {
        cout << " Newton iteration (p): " 
             << setw (  5 ) << totalNumberOfNonlinearIterations + 1
             << setw ( 16 ) << Residual_Length / Element_Scaling
             << setw ( 16 ) << Solution_Length
             << setw ( 16 ) << Residual_Solution_Length
             << setw ( 16 ) << ( Residual_Solution_Length / Solution_Length )
             << setw ( 16 ) << Po_Norm - Previous_Po_Norm
             << setw ( 16 ) << linearSolverResidualNorm
             << setw (  5 ) << numberOfLinearIterations
             << setw ( 16 ) << fabs ( Theta )
             << setw ( 16 ) << Jacobian_Time
             << setw ( 16 ) << timeStepCalculationTime
             << setw ( 16 ) << Iteration_Time
             << endl;
      }

      Previous_Po_Norm = Po_Norm;
      Number_Of_Nonlinear_Iterations = Number_Of_Nonlinear_Iterations + 1;
      ++totalNumberOfNonlinearIterations;

      if ( Solution_Length > 1.0 ) {
        Converged = Residual_Solution_Length / Solution_Length < Pressure_Newton_Solver_Tolerance;
      } else {
        Converged = Residual_Solution_Length < Pressure_Newton_Solver_Tolerance;
      }

      // Should do at least 'Minimum_Number_Of_Nonlinear_Iterations' iterations.
      Converged = Converged && ( Number_Of_Nonlinear_Iterations >= Minimum_Number_Of_Nonlinear_Iterations );

      // Should do no more than 'MaximumNumberOfNonlinearIterations' iterations.
      // There is a conflict here, if the maximum number is smaller than the minimum number of iterations.
      // In this case the Maximum number of iterations it the number that is taken.
      Converged = Converged || ( Number_Of_Nonlinear_Iterations >= MaximumNumberOfNonlinearIterations );
    }

    if ( HydraulicFracturingManager::getInstance ().isNonConservativeFractureModel () and fractureIterations == MaximumNumberOfFractureIterations - 1 ) {
      applyNonConservativeModel = true;

      if ( basinModel->debug1 or basinModel->verbose ) {
        PetscPrintf ( PETSC_COMM_WORLD, " Set applyNonConservativeModel = true\n" );
      }

    }

    hasFractured = false;

    if ( not overpressureHasDiverged ) {
      HydraulicFracturingManager::getInstance ().checkForFracturing ( Current_Time, applyNonConservativeModel, hasFractured );
    }

    fracturingOccurred = fracturingOccurred or hasFractured;
    ++fractureIterations;
  } while ( hasFractured and not overpressureHasDiverged and 
            fractureIterations <= MaximumNumberOfFractureIterations );
  

  StatisticsHandler::update ();

  if ( HydraulicFracturingManager::getInstance ().isNonConservativeFractureModel ()) {
    HydraulicFracturingManager::getInstance ().restrictPressure ( Current_Time );
  }

  PetscScalar Maximum_Overpressure;
  VecMax( Overpressure, PETSC_NULL, &Maximum_Overpressure );

  PetscScalar Max_Difference = pressureSolver->maximumPressureDifference ();

  if (( basinModel -> debug1 or basinModel->verbose ) && ( FastcauldronSimulator::getInstance ().getRank () == 0 )) {
    cout << setw ( 193 ) << "  --------------  --------------  --------------" << endl;
    cout << setw ( 161 )
         << Total_Jacobian_Time 
         << setw ( 16 ) << Total_Solve_Time 
         << setw ( 16 ) << Total_Iteration_Time 
         << endl;

    cout << " Maximum_Pressure               " << Maximum_Overpressure << endl;
    cout << " Maximum_Pressure_Difference    " << Max_Difference << endl;
    cout << " Number Of Nonlinear Iterations " << Number_Of_Nonlinear_Iterations << endl;
    cout << " Number Of Nonlinear Iterations " << totalNumberOfNonlinearIterations << endl;
    cout << " Pressure_Norms                 " 
         << Previous_TS_Po_Norm << "  " 
         << First_Po_Norm << "   " 
         << Po_Norm << "  " 
         << Previous_TS_Po_Norm / First_Po_Norm << "  " 
         << Po_Norm / First_Po_Norm << "  " 
         << Previous_TS_Po_Norm / Po_Norm << "  " 
         << endl;
  }

  // if DEBUG3 set then call the script fastcauldron_performance 
  if ( basinModel -> debug3 ) { 
    system( "fastcauldron_PostNLsolve" );
  }

  KSPDestroy ( &Pressure_Linear_Solver );

  VecDestroy  ( &Residual );
  VecDestroy  ( &Overpressure );
  VecDestroy  ( &Residual_Solution );
  MatDestroy  ( &Jacobian );

  cout.precision ( Old_Precision );
  cout.flags     ( Old_Flags );

  PetscLogStages::pop();
}

//------------------------------------------------------------//

#undef  __FUNCT__  
#define __FUNCT__ "Basin_Modelling::FEM_Grid::Solve_Temperature_For_Time_Step"

void Basin_Modelling::FEM_Grid::Solve_Temperature_For_Time_Step
   ( const double  Previous_Time,
     const double  Current_Time,
     const int     MaximumNumberOfNonlinearIterations,
           bool&   temperatureHasDiverged,
           int&    Number_Of_Nonlinear_Iterations,
           double& T_Norm ) {

  if ( basinModel -> Nonlinear_Temperature ) {
    Solve_Nonlinear_Temperature_For_Time_Step ( Previous_Time,
                                                Current_Time,
                                                MaximumNumberOfNonlinearIterations,
                                                false,
                                                temperatureHasDiverged,
                                                Number_Of_Nonlinear_Iterations,
                                                T_Norm );
  } else {
    Solve_Linear_Temperature_For_Time_Step ( Previous_Time,
                                             Current_Time,
                                             temperatureHasDiverged,
                                             T_Norm );
    Number_Of_Nonlinear_Iterations = 1;
  }

}


//------------------------------------------------------------//

#undef  __FUNCT__  
#define __FUNCT__ "Basin_Modelling::FEM_Grid::Solve_Nonlinear_Temperature_For_Time_Step"

void Basin_Modelling::FEM_Grid::Solve_Nonlinear_Temperature_For_Time_Step
   ( const double  Previous_Time,
     const double  Current_Time,
     const int     MaximumNumberOfNonlinearIterations,
     const bool    isSteadyStateCalculation,
           bool&   temperatureHasDiverged,
           int&    Number_Of_Nonlinear_Iterations,
           double& T_Norm ) {


  int numberOfLinearIterations;
  int maximumNumberOfLinearSolverIterations;


  Mat Jacobian;
  Vec Residual;
  Vec Temperature;
  Vec Residual_Solution;

  bool Converged = false;

  PetscScalar Residual_Solution_Length;
  PetscScalar Residual_Length;
  PetscScalar Solution_Length;

  PetscLogDouble Start_Time;
  PetscLogDouble End_Time;
  PetscLogDouble timeStepCalculationTime;
  PetscLogDouble Total_Solve_Time = 0.0;
  PetscLogDouble Element_Contributions_Time;

  PetscLogDouble Assembly_End_Time;
  PetscLogDouble Assembly_Start_Time;

  PetscLogDouble Iteration_Start_Time;
  PetscLogDouble Iteration_End_Time;
  PetscLogDouble Iteration_Time;
  PetscLogDouble Total_Iteration_Time = 0.0;

  PetscLogDouble Jacobian_Start_Time;
  PetscLogDouble Jacobian_End_Time;
  PetscLogDouble Jacobian_Time;
  PetscLogDouble Total_Jacobian_Time = 0.0;

  KSP Temperature_Linear_Solver;  
  KSPConvergedReason convergedReason;

  PetscScalar Previous_T_Norm = 0.0;

  PetscLogStages :: push( PetscLogStages :: TEMPERATURE_INITIALISATION_LINEAR_SOLVER );

  basinModel -> setTemperatureLinearSolver ( Temperature_Linear_Solver,
                                              Temperature_Calculator.linearSolverTolerance ( basinModel->Optimisation_Level ),
                                              isSteadyStateCalculation );

  maximumNumberOfLinearSolverIterations = getSolverMaxIterations ( Temperature_Linear_Solver );

#if PETSc_MATRIX_BUG_FIXED
  cauldronCalculator->createMatrixStructure ( *basinModel -> mapDA,
                                              Temperature_FEM_Grid,
                                              basinModel->getValidNeedles (),
                                              Temperature_DOF_Numbers, 
                                              &Jacobian,
                                              temperatureStencilWidth );
  // Temperature_Calculator.Create_Matrix_Structure ( *basinModel -> mapDA,
  //       					   Temperature_FEM_Grid,
  //                                                  basinModel->getValidNeedles (),
  //       					   Temperature_DOF_Numbers, 
  //       					   &Jacobian,
  //       					   temperatureStencilWidth );
#else
//    MatCreate ( PETSC_COMM_WORLD, PETSC_DECIDE, PETSC_DECIDE, Total_Number_Of_Temperature_Nodes, Total_Number_Of_Temperature_Nodes, &Jacobian );
  DMCreateMatrix ( Temperature_FEM_Grid, MATAIJ, &Jacobian );
#endif

  DMCreateGlobalVector ( Temperature_FEM_Grid, &Temperature );
  VecDuplicate( Temperature, &Residual );
  VecDuplicate( Temperature, &Residual_Solution );

  Temperature_Calculator.Restore_Temperature_Solution ( Temperature_FEM_Grid, Temperature_DOF_Numbers, 
							Temperature );

  PetscScalar Theta = -1.0;

  VecNorm ( Temperature, NORM_2, &T_Norm );

  if (( basinModel -> debug1 || basinModel->verbose) && ( FastcauldronSimulator::getInstance ().getRank () == 0 )) {
    cout << " Number of DOFs  = " << Total_Number_Of_Temperature_Nodes << endl;
    cout << " Initial T norm = " << T_Norm << endl;
  }

  if (( basinModel -> debug1 || basinModel->verbose ) && ( FastcauldronSimulator::getInstance ().getRank () == 0 )) {
    cout << setw ( 160 ) << "    Jacobian      Linear solver     Iteration   " << endl;
    cout << setw ( 160 ) << "  --------------  --------------  --------------" << endl;
  }


  Number_Of_Nonlinear_Iterations = 0;


  cout.precision ( 8 );
  cout.flags ( ios::scientific );

  temperatureHasDiverged = false;

  while ( ! Converged && ! temperatureHasDiverged ) {
    PetscTime(&Iteration_Start_Time);

    VecSet ( Residual, Zero );

    PetscTime(&Jacobian_Start_Time);

    if ( Number_Of_Nonlinear_Iterations == 0 ) {
      MatZeroEntries ( Jacobian ); 
      PetscTime(&Assembly_Start_Time);
      Temperature_Calculator.Assemble_System ( Previous_Time, Current_Time, 
                                               Temperature_FEM_Grid, 
                                               Temperature_DOF_Numbers, 
                                               Jacobian, Residual,
                                               Element_Contributions_Time );
      PetscTime(&Assembly_End_Time);

      Element_Assembly_Time = Element_Assembly_Time + Element_Contributions_Time;
      System_Assembly_Time = System_Assembly_Time + Assembly_End_Time - Assembly_Start_Time;

      PetscTime(&Start_Time);
      KSPSetOperators ( Temperature_Linear_Solver, Jacobian, Jacobian, SAME_NONZERO_PATTERN);
    } else {

      // It is not always necessary to compute the Jacobian, the one from the previous 
      // iteration is good enough. This saves some time not only because the Jacobian
      // is not computed but also the preconditioner remains the same as for the previous
      // iteration.
      PetscTime(&Assembly_Start_Time);
      Temperature_Calculator.Assemble_Residual ( Previous_Time, Current_Time, 
                                                 Temperature_FEM_Grid, 
                                                 Temperature_DOF_Numbers, 
                                                 Residual,
                                                 Element_Contributions_Time );
      PetscTime(&Assembly_End_Time);


      System_Assembly_Time = System_Assembly_Time + Assembly_End_Time - Assembly_Start_Time;
      Element_Assembly_Time = Element_Assembly_Time + Element_Contributions_Time;

      PetscTime(&Start_Time);
    }

    PetscTime(&Jacobian_End_Time);
    Jacobian_Time = Jacobian_End_Time - Jacobian_Start_Time; 
    Total_Jacobian_Time = Total_Jacobian_Time + Jacobian_Time; 
    
    VecSet ( Residual_Solution, Zero );
    
    KSPSolve( Temperature_Linear_Solver, Residual, Residual_Solution );
    KSPGetIterationNumber ( Temperature_Linear_Solver, &numberOfLinearIterations );

    KSPGetConvergedReason ( Temperature_Linear_Solver, &convergedReason );
    temperatureHasDiverged = ( numberOfLinearIterations == maximumNumberOfLinearSolverIterations ) || convergedReason == KSP_DIVERGED_NANORINF;

    PetscTime(&End_Time);

    timeStepCalculationTime   = End_Time - Start_Time; 
    Total_Solve_Time  = Total_Solve_Time + timeStepCalculationTime;
    System_Solve_Time = System_Solve_Time + timeStepCalculationTime;

    // Check this!!
    // VecAXPY(&Theta, Residual_Solution, Temperature );
    VecAXPY( Temperature, Theta, Residual_Solution );

    VecNorm ( Temperature, NORM_2, &T_Norm );

    Temperature_Calculator.Store_Temperature_Solution ( Temperature_FEM_Grid, Temperature_DOF_Numbers, 
							Temperature, Current_Time );

    Compute_Temperature_Dependant_Properties ( Previous_Time, Current_Time );

    VecNorm ( Temperature, NORM_2, &Solution_Length );
    VecNorm ( Residual_Solution, NORM_2, &Residual_Solution_Length );
    VecNorm ( Residual, NORM_2, &Residual_Length );

    PetscTime(&Iteration_End_Time);
    Iteration_Time = Iteration_End_Time - Iteration_Start_Time; 
    Total_Iteration_Time = Total_Iteration_Time + Iteration_Time; 

    if ( FastcauldronSimulator::getInstance ().getRank () == 0 && ( basinModel -> debug1 || basinModel->verbose) ){
      cout << " Newton iteration (t): " 
           << setw (  4 ) << Number_Of_Nonlinear_Iterations + 1
           << setw ( 16 ) << Residual_Length / Element_Scaling
           << setw ( 16 ) << Solution_Length
           << setw ( 16 ) << Residual_Solution_Length
           << setw ( 16 ) << ( Residual_Solution_Length / Solution_Length )
           << setw ( 16 ) << T_Norm - Previous_T_Norm
           << setw (  5 ) << numberOfLinearIterations
           << setw ( 16 ) << Jacobian_Time
           << setw ( 16 ) << timeStepCalculationTime
           << setw ( 16 ) << Iteration_Time
           << endl;
    }

    Previous_T_Norm = T_Norm;
    Number_Of_Nonlinear_Iterations = Number_Of_Nonlinear_Iterations + 1;

    if ( Solution_Length > 1.0 ) {
      Converged = Residual_Solution_Length / Solution_Length < Temperature_Newton_Solver_Tolerance;
    } else {
      Converged = Residual_Solution_Length < Temperature_Newton_Solver_Tolerance;
    }

    Converged = Converged || ( Number_Of_Nonlinear_Iterations >= MaximumNumberOfNonlinearIterations );
  }

  if (( basinModel -> debug1 || basinModel->verbose ) && ( FastcauldronSimulator::getInstance ().getRank () == 0 )) {
    cout << setw ( 160 ) << "  --------------  --------------  --------------" << endl;
    cout << setw ( 128 )
         << Total_Jacobian_Time 
         << setw ( 16 ) << Total_Solve_Time 
         << setw ( 16 ) << Total_Iteration_Time 
         << endl;

    cout << " Number Of Nonlinear Iterations " << Number_Of_Nonlinear_Iterations << endl;
  }

  // if DEBUG3 set then call the script fastcauldron_performance 
  if ( basinModel -> debug3 ) { 
    system( "fastcauldron_PostNLsolve" );
  }

  StatisticsHandler::update ();

  KSPDestroy ( &Temperature_Linear_Solver );

  VecDestroy  ( &Residual );
  VecDestroy  ( &Temperature );
  VecDestroy  ( &Residual_Solution );
  MatDestroy  ( &Jacobian );

  PetscLogStages :: pop();
}


//------------------------------------------------------------//

#undef  __FUNCT__  
#define __FUNCT__ "Basin_Modelling::FEM_Grid::Solve_Linear_Temperature_For_Time_Step"

void Basin_Modelling::FEM_Grid::Solve_Linear_Temperature_For_Time_Step
   ( const double  Previous_Time,
     const double  Current_Time,
           bool&   temperatureHasDiverged,
           double& T_Norm ) {


  int numberOfLinearIterations;
  int maximumNumberOfLinearSolverIterations;

  Mat Stiffness_Matrix;
  Vec Load_Vector;
  Vec Temperature;

  PetscLogDouble Start_Time;
  PetscLogDouble End_Time;
  PetscLogDouble Property_Time;
  PetscLogDouble Solve_Time;
  PetscLogDouble Element_Contributions_Time;

  PetscLogDouble Iteration_Start_Time;
  PetscLogDouble Iteration_End_Time;
  PetscLogDouble Iteration_Time;

  PetscLogDouble matrixStartTime;
  PetscLogDouble matrixEndTime;
  PetscLogDouble matrixCreationTime;

  PetscLogDouble storeStartTime;
  PetscLogDouble storeEndTime;
  PetscLogDouble storeCreationTime;

  PetscLogDouble restoreStartTime;
  PetscLogDouble restoreEndTime;
  PetscLogDouble restoreCreationTime;

  PetscLogDouble dupStartTime;
  PetscLogDouble dupEndTime;
  PetscLogDouble dupCreationTime;


  PetscLogDouble System_Assembly_Start_Time;
  PetscLogDouble System_Assembly_End_Time;
  PetscLogDouble Total_System_Assembly_Time;

  KSP Temperature_Linear_Solver;  
  KSPConvergedReason convergedReason;

  PetscLogStages::push( PetscLogStages :: TEMPERATURE_LINEAR_SOLVER );
  PetscTime(&Iteration_Start_Time);

  basinModel -> setTemperatureLinearSolver ( Temperature_Linear_Solver,
                                              Temperature_Calculator.linearSolverTolerance ( basinModel->Optimisation_Level ),
                                              false );

  maximumNumberOfLinearSolverIterations = getSolverMaxIterations ( Temperature_Linear_Solver );



  PetscTime(&matrixStartTime);

#if PETSc_MATRIX_BUG_FIXED
  cauldronCalculator->createMatrixStructure ( *basinModel -> mapDA,
                                              Temperature_FEM_Grid,
                                              basinModel->getValidNeedles (),
                                              Temperature_DOF_Numbers, 
                                              &Stiffness_Matrix,
                                              temperatureStencilWidth );

  // Temperature_Calculator.Create_Matrix_Structure ( *basinModel -> mapDA,
  //       					   Temperature_FEM_Grid,
  //                                                  basinModel->getValidNeedles (),
  //       					   Temperature_DOF_Numbers, 
  //       					   &Stiffness_Matrix,
  //       					   temperatureStencilWidth );
#else
//    MatCreate ( PETSC_COMM_WORLD, PETSC_DECIDE, PETSC_DECIDE, Total_Number_Of_Temperature_Nodes, Total_Number_Of_Temperature_Nodes, &Stiffness_Matrix );
  DMCreateMatrix ( Temperature_FEM_Grid, MATAIJ, &Stiffness_Matrix );
#endif

  PetscTime(&matrixEndTime);
  matrixCreationTime = matrixEndTime - matrixStartTime;

  PetscTime(&dupStartTime);
  DMCreateGlobalVector ( Temperature_FEM_Grid, &Temperature );
  VecDuplicate( Temperature, &Load_Vector );
  PetscTime(&dupEndTime);
  dupCreationTime = dupEndTime - dupStartTime;

  PetscTime(&System_Assembly_Start_Time);
  VecSet ( Load_Vector, Zero );

  MatZeroEntries ( Stiffness_Matrix ); 
  Temperature_Calculator.Assemble_Stiffness_Matrix ( Previous_Time, Current_Time, 
                                                     Temperature_FEM_Grid, 
                                                     Temperature_DOF_Numbers, 
                                                     Stiffness_Matrix, Load_Vector,
                                                     Element_Contributions_Time );

  KSPSetOperators ( Temperature_Linear_Solver, Stiffness_Matrix, Stiffness_Matrix, SAME_NONZERO_PATTERN );
  PetscTime(&System_Assembly_End_Time);
  Total_System_Assembly_Time = System_Assembly_End_Time - System_Assembly_Start_Time;

  // Solve the linear system Temperature = Stiffness_Matrix^-1 * Load_Vector
  PetscTime(&Start_Time);

  PetscTime(&restoreStartTime);

  Temperature_Calculator.Restore_Temperature_Solution ( Temperature_FEM_Grid, Temperature_DOF_Numbers, 
							Temperature );

  PetscTime(&restoreEndTime);
  restoreCreationTime = restoreEndTime - restoreStartTime;

//    VecSet ( &Zero, Temperature );
  KSPSolve ( Temperature_Linear_Solver, Load_Vector, Temperature );
  KSPGetIterationNumber ( Temperature_Linear_Solver, &numberOfLinearIterations );
  PetscTime(&End_Time);

  Solve_Time = End_Time - Start_Time;

  VecNorm ( Temperature, NORM_2, &T_Norm );
  KSPGetConvergedReason ( Temperature_Linear_Solver, &convergedReason );

  temperatureHasDiverged = isnan ( T_Norm ) || ( numberOfLinearIterations == maximumNumberOfLinearSolverIterations ) || convergedReason == KSP_DIVERGED_NANORINF;

  PetscTime(&storeStartTime);

  Temperature_Calculator.Store_Temperature_Solution ( Temperature_FEM_Grid,
                                                      Temperature_DOF_Numbers, 
                                                      Temperature, Current_Time );
  PetscTime(&storeEndTime);
  storeCreationTime = storeEndTime - storeStartTime;

  PetscTime(&Start_Time);

  Compute_Temperature_Dependant_Properties ( Previous_Time, Current_Time );
  PetscTime(&End_Time);
  Property_Time = End_Time - Start_Time;

  // if DEBUG3 set then call the script fastcauldron_performance 
  if ( basinModel -> debug3 ) { 
    system( "fastcauldron_PostNLsolve" );
  }

  StatisticsHandler::update ();

  KSPDestroy ( &Temperature_Linear_Solver );


  PetscTime(&Iteration_End_Time);
  Iteration_Time = Iteration_End_Time - Iteration_Start_Time; 

  if (( basinModel -> debug1 || basinModel->verbose ) && ( FastcauldronSimulator::getInstance ().getRank () == 0 )) {
    cout << " Temperature solving: "  
         << Total_Number_Of_Temperature_Nodes << "  "
         << numberOfLinearIterations << "  " 
         << Total_System_Assembly_Time << "  " 
         << Solve_Time << "  " 
         << Property_Time << "  " 
         << Iteration_Time << "  "
         << matrixCreationTime << "  "
         << storeCreationTime << "  "
         << restoreCreationTime << "  "
         << dupCreationTime << "  "
         << endl;
  }

  VecDestroy  ( &Load_Vector );
  VecDestroy  ( &Temperature );
  MatDestroy  ( &Stiffness_Matrix );


  System_Assembly_Time = System_Assembly_Time + Total_System_Assembly_Time;
  System_Solve_Time = System_Solve_Time + Solve_Time;
  Property_Calculation_Time = Property_Calculation_Time + Property_Time;
  Element_Assembly_Time = Element_Assembly_Time + Element_Contributions_Time;

  PetscLogStages::pop();
}


//------------------------------------------------------------//

#undef  __FUNCT__  
#define __FUNCT__ "Basin_Modelling::FEM_Grid::Solve_Coupled_For_Time_Step"

void Basin_Modelling::FEM_Grid::Solve_Coupled_For_Time_Step ( const double  Previous_Time,
                                                              const double  Current_Time,
                                                              const int     Maximum_Number_Of_Nonlinear_Pressure_Iterations,
                                                              const int     Maximum_Number_Of_Nonlinear_Temperature_Iterations,
                                                                    bool&   hasDiverged,
                                                                    int&    Number_Of_Nonlinear_Iterations,
                                                                    double& Po_Norm,
                                                                    double& T_Norm,
                                                                    bool&   fracturingOccurred ) {

  int Number_Of_Pressure_Nonlinear_Iterations;
  int Number_Of_Temperature_Nonlinear_Iterations;

  int I;
  int Number_Of_Coupled_Iterations = 1;
  double Previous_Po_Norm = 0.0;
  double Previous_T_Norm  = 0.0;

  bool overpressureHasDiverged;
  bool temperatureHasDiverged;

  T_Norm = 1.0;

  if ( ( basinModel -> debug1 || basinModel->verbose) && FastcauldronSimulator::getInstance ().getRank () == 0 ) {
    cout << "****************************************************************" << endl;
  }

  for ( I = 1; I <= Number_Of_Coupled_Iterations; I++ ) {

    // Solve pressure equations before the temperature, the main reason for this
    // is that the pressure is thought to have more of an influence on the temperature
    // than vice-versa. Also, the pressure will affect the depth, if using the geometric 
    // loop, therefore therefore we need to solve the temperature uising the correct depth.
    //
    // If the number of iteration in the outer coupling loop is greater than 1,
    // then there is no need to check to see if the temperature solver has diverged
    // since if it has the loop will be exited anyway.
    Solve_Pressure_For_Time_Step ( Previous_Time,
                                   Current_Time,
                                   Maximum_Number_Of_Nonlinear_Pressure_Iterations,
                                   overpressureHasDiverged,
                                   Number_Of_Pressure_Nonlinear_Iterations,
                                   Po_Norm,
                                   fracturingOccurred );

    // compute lithostatic pressure in basement.

    if ( not overpressureHasDiverged ) {
      Solve_Temperature_For_Time_Step ( Previous_Time, 
                                        Current_Time, 
                                        Maximum_Number_Of_Nonlinear_Temperature_Iterations, 
                                        temperatureHasDiverged,
                                        Number_Of_Temperature_Nonlinear_Iterations,
                                        T_Norm );
    } else {
      T_Norm = 99999.0;
    }

    hasDiverged = overpressureHasDiverged || temperatureHasDiverged;

    if ( (basinModel -> debug1 || basinModel->verbose) && FastcauldronSimulator::getInstance ().getRank () == 0 ) {
      cout << " Norms" << setw ( 3 ) << I << " "
           << Po_Norm << "  " 
           << T_Norm << "  " 
           << fabs ( Po_Norm - Previous_Po_Norm ) / Po_Norm << "  " 
           << fabs ( T_Norm - Previous_T_Norm ) / T_Norm << "  "
           << Number_Of_Pressure_Nonlinear_Iterations << "  " 
           << Number_Of_Temperature_Nonlinear_Iterations << "  " 
           << endl;
    }

    if ( hasDiverged ) {
      // One of the solvers has diverged and so we should exit the soupled solver as soon as possible.
      break;
    }

#if 0
    // Try to get the matrix-unit test working with the subdomain.
    // the problem is related to the element-activity and on-domain-boundary of neighbour elements.
    if ( basinModel->getUnitTestNumber () == 3 ) {
       MatrixUnitTest::getInstance ().solveForTimeStep ( Previous_Time, Current_Time );
    }
#endif

    if ( basinModel->getUnitTestNumber () == 3 ) {
       MatrixUnitTest::getInstance ().solveForTimeStep ( Previous_Time, Current_Time, m_domainElements, m_elementRefs );
    }

    if ( fabs (( Po_Norm - Previous_Po_Norm ) / Po_Norm ) < Pressure_Newton_Solver_Tolerance || 
         fabs (( T_Norm  - Previous_T_Norm  ) / T_Norm  ) < Temperature_Newton_Solver_Tolerance ) {

      // We 'OR' these 2 convergence checks because if one has converged then
      // the other is not going to be effected.
      break;
    }

    Previous_Po_Norm  = Po_Norm;
    Previous_T_Norm  = T_Norm;

  }

  Number_Of_Nonlinear_Iterations = Number_Of_Pressure_Nonlinear_Iterations;
}


//------------------------------------------------------------//

#undef  __FUNCT__  
#define __FUNCT__ "Basin_Modelling::FEM_Grid::Destroy_Vectors"

void Basin_Modelling::FEM_Grid::Destroy_Vectors () {

  if ( basinModel -> DoTemperature || basinModel -> Do_Iteratively_Coupled ) {
     Destroy_Petsc_Vector ( Temperature_Depths );
     Destroy_Petsc_Vector ( Temperature_DOF_Numbers );
     DMDestroy ( &Temperature_FEM_Grid );
  }

  if ( basinModel->DoOverPressure || basinModel -> Do_Iteratively_Coupled ) {
    Destroy_Petsc_Vector ( Pressure_Depths );
    Destroy_Petsc_Vector ( Pressure_DOF_Numbers );
    Destroy_Petsc_Vector ( pressureNodeIncluded );
    DMDestroy ( &Pressure_FEM_Grid );
  }

}


//------------------------------------------------------------//

#undef  __FUNCT__  
#define __FUNCT__ "Basin_Modelling::FEM_Grid::Compute_Temperature_Dependant_Properties"

void Basin_Modelling::FEM_Grid::Compute_Temperature_Dependant_Properties ( const double Previous_Time,
                                                                           const double Current_Time ) {

  PetscLogDouble Start_Time;
  PetscLogDouble End_Time;

  PetscTime(&Start_Time);

  if ( ! basinModel -> DoOverPressure && ! basinModel -> Do_Iteratively_Coupled ) {
    basinModel -> Calculate_Pressure ( Current_Time );
  }

  double Surface_Temperature;
  double Seabottom_Depth;
  double Estimated_Temperature;
  double Heat_Production;

  int I;
  int J;
  int K;
  int X_Start;
  int Y_Start;
  int Z_Start;
  int X_Count;
  int Y_Count;
  int Z_Count;

  LayerProps_Ptr Current_Layer;
  Layer_Iterator Layers;
  const CompoundLithology*  Current_Lithology;

  DMDAGetCorners ( *basinModel->mapDA, &X_Start, &Y_Start, PETSC_NULL, &X_Count, &Y_Count, PETSC_NULL );
  const Boolean2DArray& Valid_Needle = basinModel->getValidNeedles ();


  Double_Array_2D Temperature_Above ( X_Count, Y_Count );
  
  Layers.Initialise_Iterator ( basinModel -> layers, Descending, Sediments_Only, Active_Layers_Only );

  if ( Layers.Iteration_Is_Done ()) {
    return;
  }

  // Current top layer
  Current_Layer = Layers.Current_Layer ();

  Current_Layer -> Current_Properties.Activate_Property ( Basin_Modelling::Temperature );
  Current_Layer -> Current_Properties.Activate_Property ( Basin_Modelling::Depth );

  DMDAGetCorners ( Current_Layer->layerDA, &X_Start, &Y_Start, &Z_Start, &X_Count, &Y_Count, &Z_Count );

  for ( I = X_Start; I < X_Start + X_Count; I++ ) {

    for ( J = Y_Start; J < Y_Start + Y_Count; J++ ) {

      if ( Valid_Needle ( I, J )) {
        Surface_Temperature = FastcauldronSimulator::getInstance ().getSeaBottomTemperature ( I, J, Current_Time );
        Seabottom_Depth     = FastcauldronSimulator::getInstance ().getSeaBottomDepth ( I, J, Current_Time );

        for ( K = Z_Count - 2; K >= 0; K-- ) {

          if ( Current_Layer -> Current_Properties ( Basin_Modelling::Temperature, K, J, I ) == CAULDRONIBSNULLVALUE ) {

            Estimated_Temperature = basinModel->Estimate_Temperature_At_Depth ( Current_Layer->Current_Properties ( Basin_Modelling::Depth, K, J, I ),
                                                                                Surface_Temperature,
                                                                                Seabottom_Depth );

            Current_Layer -> Current_Properties ( Basin_Modelling::Temperature, K, J, I ) = Estimated_Temperature;
          }

        }

      }

    }

  }

  Current_Layer -> Current_Properties.Restore_Property ( Basin_Modelling::Temperature );
  Current_Layer -> Current_Properties.Restore_Property ( Basin_Modelling::Depth );

  while ( ! Layers.Iteration_Is_Done () ) {
    
    Current_Layer = Layers.Current_Layer ();

    PETSC_3D_Array Layer_Porosity        ( Current_Layer -> layerDA, Current_Layer -> Porosity );
    PETSC_3D_Array Layer_Heat_Production ( Current_Layer -> layerDA, Current_Layer -> BulkHeatProd );
    DMDAGetCorners ( Current_Layer->layerDA, &X_Start, &Y_Start, &Z_Start, &X_Count, &Y_Count, &Z_Count );

    for ( I = X_Start; I < X_Start + X_Count; I++ ) {

      for ( J = Y_Start; J < Y_Start + Y_Count; J++ ) {

        if ( Valid_Needle ( I, J )) {
          Current_Lithology = Current_Layer->getLithology ( I, J );

          for ( K = 0; K < Z_Count; K++ ) {
            Current_Lithology -> calcBulkHeatProd ( Layer_Porosity ( K, J, I ), Heat_Production );
            Layer_Heat_Production ( K, J, I ) = Heat_Production;
          }

        }

      }

    }

    Layers++;
  }

  PetscTime(&End_Time);
  Property_Calculation_Time = Property_Calculation_Time + ( End_Time - Start_Time );

}

//------------------------------------------------------------//

void Basin_Modelling::FEM_Grid::clearLayerVectors () {

   using namespace Basin_Modelling;

   CalculationMode calculationMode = FastcauldronSimulator::getInstance ().getCalculationMode ();

   Layer_Range range;

   if ( calculationMode == OVERPRESSURE_MODE ) {
      // Probably it would not matter if it were always Basement_And_Sediments.
      range = Sediments_Only;
   } else {
      range = Basement_And_Sediments;
   }

   Layer_Iterator Layers ( basinModel -> layers, Ascending, range,
                           Active_And_Inactive_Layers );

   while ( not Layers.Iteration_Is_Done ()) {
      Layers.Current_Layer ()->reInitialise ();
      Layers++;
   }

}

//------------------------------------------------------------//

#undef  __FUNCT__  
#define __FUNCT__ "Basin_Modelling::FEM_Grid::setDOFs"

void Basin_Modelling::FEM_Grid::setDOFs () {

  if ( basinModel->DoTemperature || basinModel -> Do_Iteratively_Coupled ) {
     cauldronCalculator->setDepths ( Temperature_FEM_Grid, true, Temperature_Depths );
     cauldronCalculator->setDOFs ( Temperature_FEM_Grid, 
                                   Temperature_Depths, 
                                   Temperature_DOF_Numbers, 
                                   temperatureStencilWidth );
  }

  if ( basinModel->DoOverPressure || basinModel -> Do_Iteratively_Coupled ) {
     pressureSolver->setDepths ( Pressure_FEM_Grid, false, Pressure_Depths );
     pressureSolver->setDOFs ( Pressure_FEM_Grid, 
                               Pressure_Depths, 
                               Pressure_DOF_Numbers, 
                               pressureStencilWidth );
     pressureSolver->setRealNodes ( Pressure_FEM_Grid,
                                    Pressure_DOF_Numbers,
                                    pressureNodeIncluded );
  }

}

//------------------------------------------------------------//

#undef  __FUNCT__  
#define __FUNCT__ "Basin_Modelling::FEM_Grid::Copy_Current_Properties"

void Basin_Modelling::FEM_Grid::Copy_Current_Properties () {

  Layer_Iterator Layers ( basinModel -> layers, Ascending, Basement_And_Sediments, Active_Layers_Only );
//    Layer_Iterator Layers ( basinModel -> layers, Ascending, Sediments_Only, Active_Layers_Only );
  LayerProps_Ptr Current_Layer;

//    PetscFunctionBegin;

  const Boolean2DArray& Valid_Needle = basinModel->getValidNeedles ();

  for ( Layers.Initialise_Iterator (); ! Layers.Iteration_Is_Done (); Layers++ ) {
     Layers.Current_Layer ()->copyProperties ();
    // Current_Layer = Layers.Current_Layer ();
    // Current_Layer -> Previous_Properties.Copy ( Current_Layer -> Current_Properties, Valid_Needle );
  } 

//    PetscFunctionReturnVoid();

}


//------------------------------------------------------------//

#undef  __FUNCT__  
#define __FUNCT__ "Basin_Modelling::FEM_Grid::Store_Computed_Deposition_Thickness"

void Basin_Modelling::FEM_Grid::Store_Computed_Deposition_Thickness ( const double Current_Time ) {

  int I, J;
  int K;
  int X_Start;
  int Y_Start;
  int Z_Start;
  int X_End;
  int Y_End;
  int Z_End;
  int X_Count;
  int Y_Count;
  int Z_Count;

  Layer_Iterator Layers ( basinModel -> layers, Descending, Sediments_Only, Active_Layers_Only );
  LayerProps_Ptr Current_Layer = Layers.Current_Layer ();
  // Is this the best number to have here? I think so, but cannot prove it.
  // The snapshot times, time step, ... are written out (perhaps read in too) 
  // as a float, but stored and used as doubles.
  const double Float_Epsilon = pow ( 2.0, -23 ); 

  if ( fabs ( Current_Time - Current_Layer -> depoage ) < NumericFunctions::Maximum ( Current_Time, 1.0 ) * Float_Epsilon ) {

     const Boolean2DArray& Valid_Needle = basinModel->getValidNeedles ();

    DMCreateGlobalVector ( *basinModel->mapDA, &(Current_Layer -> Computed_Deposition_Thickness) );

    PETSC_2D_Array Computed_Deposition_Thickness ( *basinModel->mapDA, 
						   Current_Layer -> Computed_Deposition_Thickness );

    DMDAGetCorners ( Current_Layer->layerDA, &X_Start, &Y_Start, &Z_Start, &X_Count, &Y_Count, &Z_Count );
    X_End = X_Start + X_Count;
    Y_End = Y_Start + Y_Count;

    if ( basinModel->isGeometricLoop ()) {

      PETSC_3D_Array Depth ( Current_Layer->layerDA, Current_Layer->Current_Properties ( Basin_Modelling::Depth ));

      Z_End = Z_Start + Z_Count;

      for ( I = X_Start; I < X_End; I++ ) {

        for ( J = Y_Start; J < Y_End; J++ ) {

          if ( Valid_Needle ( I, J )) {
            Computed_Deposition_Thickness ( J, I ) = Depth ( Z_Start, J, I ) - Depth ( Z_End - 1, J, I );
          }

        }

      }

    } else {

      double         Computed_Solid_Thickness;
      PETSC_3D_Array Solid_Thickness ( Current_Layer->layerDA, Current_Layer->Current_Properties ( Basin_Modelling::Solid_Thickness ));

        // Minus 1 here because we are looping over segment arrays NOT node arrays.
        Z_End = Z_Start + Z_Count - 1;

      for ( I = X_Start; I < X_End; I++ ) {

        for ( J = Y_Start; J < Y_End; J++ ) {

          if ( Valid_Needle ( I, J )) {
            Computed_Solid_Thickness = 0.0;

            for ( K = Z_Start; K < Z_End; K++ ) {
              Computed_Solid_Thickness = Computed_Solid_Thickness + Solid_Thickness ( K, J, I );
            }

            Computed_Deposition_Thickness ( J, I ) = Computed_Solid_Thickness;
          }

        }

      }

    }

  }

}

//------------------------------------------------------------//

#undef  __FUNCT__  
#define __FUNCT__ "Basin_Modelling::FEM_Grid::Set_Pressure_Dependent_Properties"

void Basin_Modelling::FEM_Grid::Set_Pressure_Dependent_Properties ( const double Current_Time ) {

   CalculationMode calculationMode = FastcauldronSimulator::getInstance ().getCalculationMode ();

   if ( calculationMode == OVERPRESSURE_MODE or 
        calculationMode == NO_CALCULATION_MODE ) {
      return;
   }

   if ( Current_Time == FastcauldronSimulator::getInstance ().getAgeOfBasin ()) {
      // There is no need to distinguish between geometric-loop and non-geometric-loop 
      // since the basement lithologies have zero porosity.
      basinModel->calcNodeDepths ( Current_Time );
   } else if ( calculationMode != PRESSURE_AND_TEMPERATURE_MODE and calculationMode != COUPLED_DARCY_MODE ) {
      // } else if ( ! basinModel -> Do_Iteratively_Coupled ) {

      basinModel->calcNodeVes    ( Current_Time );
      basinModel->calcNodeMaxVes ( Current_Time );
      basinModel->calcPorosities ( Current_Time );

      if ( calculationMode == OVERPRESSURED_TEMPERATURE_MODE and not basinModel->isGeometricLoop ()) {
         basinModel->setNodeDepths ( Current_Time );
      } else {
         basinModel->calcNodeDepths ( Current_Time );
      }

   }

   computeBasementLithostaticPressure ( basinModel, Current_Time );
}


//------------------------------------------------------------//

#undef  __FUNCT__  
#define __FUNCT__ "Basin_Modelling::FEM_Grid::Initialise_Basin_Temperature"

void Basin_Modelling::FEM_Grid::Initialise_Basin_Temperature ( bool& temperatureHasDiverged ) {

   int Number_Of_Newton_Iterations = -1;
   double Current_Time = FastcauldronSimulator::getInstance ().getAgeOfBasin ();
   // double Current_Time = basinModel -> Age_Of_Basin ();

  double T_Norm;

  basinModel -> findActiveElements ( Current_Time );
  Construct_Temperature_FEM_Grid ( Current_Time, FastcauldronSimulator::getInstance ().getAgeOfBasin (), majorSnapshots, false );
  // Construct_Temperature_FEM_Grid ( Current_Time, basinModel -> Age_Of_Basin (), majorSnapshots, false );
  cauldronCalculator->setDepths ( Temperature_FEM_Grid, true, Temperature_Depths );
  cauldronCalculator->setDOFs ( Temperature_FEM_Grid, 
                                Temperature_Depths, 
                                Temperature_DOF_Numbers, 
                                temperatureStencilWidth );

  Temperature_Calculator.Compute_Crust_Heat_Production ();
  Temperature_Calculator.Compute_Heat_Flow_Boundary_Conditions ( Current_Time );
  Temperature_Calculator.Estimate_Basement_Temperature ();

  Solve_Nonlinear_Temperature_For_Time_Step ( Current_Time,
                                              FastcauldronSimulator::getInstance ().getAgeOfBasin (),
                                              // basinModel->Age_Of_Basin (),
                                              10,
                                              true, // steady state calculation.
                                              temperatureHasDiverged,
		          		      Number_Of_Newton_Iterations,
                                              T_Norm );

  printRelatedProjects ( Current_Time );
  Copy_Current_Properties ();

}


//------------------------------------------------------------//

#undef  __FUNCT__  
#define __FUNCT__ "Basin_Modelling::FEM_Grid::printRelatedProjects"

void Basin_Modelling::FEM_Grid::printRelatedProjects ( const double currentAge ) const {

  int X_Position;
  int Y_Position;
  unsigned int I;

  int Old_Precision;
  ios::fmtflags Old_Flags;

  if ( basinModel -> debug1 ) {
    Old_Precision = cout.precision ( 8 );
    Old_Flags     = cout.flags ( ios::scientific );

    for ( I = 0; I < basinModel -> Related_Projects.size (); I++ ) {
      cout << flush;
      fflush ( stdout );
      MPI_Barrier(PETSC_COMM_WORLD);
      cout << flush;
      fflush ( stdout );

      X_Position = int (( basinModel -> Related_Projects [ I ] -> X_Coord - Origin_X ) / Delta_X );
      Y_Position = int (( basinModel -> Related_Projects [ I ] -> Y_Coord - Origin_Y ) / Delta_Y );

      if ( basinModel -> debug1 && FastcauldronSimulator::getInstance ().getRank () == 0 ) {
        cout << " outputting pseudo well for project " <<  basinModel -> Related_Projects [ I ] -> Name 
             << " at location ( " 
             << basinModel -> Related_Projects [ I ] -> X_Coord
             << ", "
             << basinModel -> Related_Projects [ I ] -> Y_Coord
             << " ), ( "
             << X_Position 
             << ", "
             << Y_Position 
             << " )"
             << endl << flush;
        fflush ( stdout );
      }
 
      Print_Needle ( currentAge, X_Position, Y_Position );
//        Print_Needle ( currentAge, X_Position, Y_Position, temperatureInterpolants [ I ], vesInterpolants [ I ] );
    }

    if ( basinModel -> Related_Projects.size () == 0 ) {
      cout << flush;
      fflush ( stdout );
      MPI_Barrier(PETSC_COMM_WORLD);

      // Better to make this some valid needle, in case 0,0 is not valid.
      Print_Needle ( currentAge, 0, 0 );
    }

    cout.precision ( Old_Precision );
    cout.flags     ( Old_Flags );
  }

}


//------------------------------------------------------------//


void Basin_Modelling::FEM_Grid::Print_Needle ( const double currentAge, const int I, const int J ) const {

   PVTComponents m_defaultMolarMasses = PVTCalc::getInstance ().getMolarMass ();
   m_defaultMolarMasses *= 1.0e-3;


   bool includedInDarcySimulation = FastcauldronSimulator::getInstance ().getMcfHandler ().solveFlowEquations ();

  // The Pressure_FEM_Grid MUST have been allocated at this point.
  int K;
  int X_Start;
  int Y_Start;
  int Z_Start;
  int X_Count;
  int Y_Count;
  int Z_Count;
  int includedDOF;

  const Boolean2DArray& Valid_Needle = basinModel->getValidNeedles ();

  Layer_Iterator Pressure_Layers;
  LayerProps_Ptr Current_Layer;
  const CompoundLithology*  Current_Lithology;

  const FluidType* currentFluid;
//   FluidProps*    currentFluid;

  bool   Include_Chemical_Compaction;
  double Permeability_Normal_Compound;
  double Permeability_Plane_Compound;
  double fracturePressure;

  DM  const* gridUsed;
  Vec const* dofNumbers;

  CompoundProperty Porosity;

  string Layer_Name;
  std::stringstream buffer;
 
  int Old_Precision = buffer.precision ( 6 );
  buffer.setf ( ios::scientific );

  Layer_Range Basin_Bottom;

  if ( basinModel->DoOverPressure ) {
    Basin_Bottom = Sediments_Only;
    gridUsed = &Pressure_FEM_Grid;
    dofNumbers = &Pressure_DOF_Numbers;
  } else {
    Basin_Bottom = Basement_And_Sediments;
    gridUsed = &Temperature_FEM_Grid;
    dofNumbers = &Temperature_DOF_Numbers;
  }

  Pressure_Layers.Initialise_Iterator ( basinModel->layers, Descending, Basin_Bottom, Active_Layers_Only );
  DMDAGetCorners ( *gridUsed, &X_Start, &Y_Start, PETSC_NULL, &X_Count, &Y_Count, &Z_Count );

  int  DOF_Position = Z_Count - 1;
  int  Number_Of_Segments;
  int* GlobalK;
  bool onThisNode = false;

  double c1Sum = 0.0;
  double c2Sum = 0.0;
  double c3Sum = 0.0;
  double c4Sum = 0.0;
  double c5Sum = 0.0;
  double c6AroSum = 0.0;
  double c6SatSum = 0.0;
  double c15AroSum = 0.0;
  double c15SatSum = 0.0;

  double c1Sum2 = 0.0;
  double c2Sum2 = 0.0;
  double c3Sum2 = 0.0;
  double c4Sum2 = 0.0;
  double c5Sum2 = 0.0;
  double c6AroSum2 = 0.0;
  double c6SatSum2 = 0.0;
  double c15AroSum2 = 0.0;
  double c15SatSum2 = 0.0;

  int elementJ = NumericFunctions::clipValueToRange ( J, FastcauldronSimulator::getInstance ().getMapElementArray ().firstJ (),
                                                      FastcauldronSimulator::getInstance ().getMapElementArray ().lastJ ());

  int elementI = NumericFunctions::clipValueToRange ( I, FastcauldronSimulator::getInstance ().getMapElementArray ().firstI (),
                                                      FastcauldronSimulator::getInstance ().getMapElementArray ().lastI ());

  PETSC_3D_Array DOFs ( *gridUsed, *dofNumbers );

  if (( X_Start <= I ) && ( I < X_Start + X_Count ) && ( Y_Start <= J ) && ( J < Y_Start + Y_Count ) && Valid_Needle ( I, J )) {
    buffer << endl << endl;
    buffer << " Printing details of needle " << I << "  " << J << endl;
    buffer << "                                       Age            Depth          Po            Pp            Ph            Pl            Pf           VES         Max_VES       Porosity        PermN          PermH              Temperature" << endl;
    onThisNode = true;
  }

  for ( Pressure_Layers.Initialise_Iterator (); ! Pressure_Layers.Iteration_Is_Done (); Pressure_Layers++ ) {
    Current_Layer = Pressure_Layers.Current_Layer ();
    Layer_Name = Current_Layer -> layername;
    blankSpaceUnderscore ( Layer_Name );

    Include_Chemical_Compaction = (( basinModel -> Do_Chemical_Compaction ) && ( Current_Layer -> Get_Chemical_Compaction_Mode ()));

    Number_Of_Segments = Current_Layer->getNrOfActiveElements();
    GlobalK = new int [ Number_Of_Segments + 1 ];

    for ( K = Number_Of_Segments; K >= 0; K-- ) {
      GlobalK [ K ] = DOF_Position;
      DOF_Position = DOF_Position - 1;
    }

    DOF_Position = DOF_Position + 1;

    // Get the size of the layer DA.
    DMDAGetCorners ( Current_Layer->layerDA, &X_Start, &Y_Start, &Z_Start, &X_Count, &Y_Count, &Z_Count );
    Current_Layer -> Current_Properties.Activate_Properties ();

    PetscBlockVector<Saturation> saturations;
    PetscBlockVector<PVTComponents> concentrations;

    if ( includedInDarcySimulation and Current_Layer->kind () == Interface::SEDIMENT_FORMATION ) {
       saturations.setVector ( Current_Layer->getVolumeGrid ( Saturation::NumberOfPhases ), Current_Layer->getPhaseSaturationVec (), INSERT_VALUES );
       concentrations.setVector ( Current_Layer->getVolumeGrid ( NumberOfPVTComponents ), Current_Layer->getPreviousComponentVec (), INSERT_VALUES );
    }

    // Copy ALL the layer depths into the FEM Grid depths
    if ( X_Start <= I && I < X_Start + X_Count ) {

      if ( Y_Start <= J && J < Y_Start + Y_Count ) {

        if ( Valid_Needle ( I, J )) {
          Current_Lithology = Current_Layer->getLithology ( I, J );
          currentFluid = Current_Layer->fluid;

          for ( K = Z_Start + Z_Count - 1; K >= Z_Start; K-- ) {

            Current_Lithology->getPorosity ( Current_Layer -> Current_Properties ( Basin_Modelling::VES_FP, K, J, I ),
                                             Current_Layer -> Current_Properties ( Basin_Modelling::Max_VES, K, J, I ),
                                             Include_Chemical_Compaction,
                                             Current_Layer -> Current_Properties ( Basin_Modelling::Chemical_Compaction, K, J, I ),
                                             Porosity );

            Current_Lithology->calcBulkPermeabilityNP ( Current_Layer -> Current_Properties ( Basin_Modelling::VES_FP,  K, J, I ),
                                                        Current_Layer -> Current_Properties ( Basin_Modelling::Max_VES, K, J, I ),
                                                        Porosity,
                                                        Permeability_Normal_Compound,
                                                        Permeability_Plane_Compound );

            if ( Current_Layer->isBasement ()) {
              includedDOF = 999;
            } else {
              includedDOF = int ( Current_Layer -> includedNodes ( I, J, K  ));
            }

            fracturePressure = HydraulicFracturingManager::getInstance ().fracturePressure ( Current_Lithology,
                                                                                             currentFluid,
                                                                                             FastcauldronSimulator::getInstance ().getSeaBottomTemperature ( I, J, currentAge ),
                                                                                             FastcauldronSimulator::getInstance ().getSeaBottomDepth ( I, J, currentAge ),
                                                                                             Current_Layer -> Current_Properties ( Basin_Modelling::Depth, K, J, I ),
                                                                                             Current_Layer -> Current_Properties ( Basin_Modelling::Hydrostatic_Pressure, K, J, I ),
                                                                                             Current_Layer -> Current_Properties ( Basin_Modelling::Lithostatic_Pressure, K, J, I ));

            int value;

            if ( Current_Layer -> Current_Properties ( Basin_Modelling::Lithostatic_Pressure, K, J, I ) == 0.0 ) {
              value = 999;
            } else {
              value = int ( 100.0 * ( Current_Layer -> Current_Properties ( Basin_Modelling::Pore_Pressure, K, J, I ) /
                                      Current_Layer -> Current_Properties ( Basin_Modelling::Lithostatic_Pressure, K, J, I )));
            }

//             buffer << setw ( 20 ) << Layer_Name << "  " 
//                  << setw (  4 ) << I
//                  << setw (  4 ) << J
//                  << setw (  4 ) << K
//                  << setw ( 14 ) << currentAge
//                  << setw ( 14 ) << Current_Layer -> Current_Properties ( Basin_Modelling::Depth, K, J, I )
//                  << setw ( 14 ) << Current_Layer -> Current_Properties ( Basin_Modelling::Overpressure, K, J, I )
//                  << setw ( 14 ) << Current_Layer -> Current_Properties ( Basin_Modelling::Pore_Pressure, K, J, I )
//                  << setw ( 14 ) << Current_Layer -> Current_Properties ( Basin_Modelling::Hydrostatic_Pressure, K, J, I )
//                  << setw ( 14 ) << Current_Layer -> Current_Properties ( Basin_Modelling::Lithostatic_Pressure, K, J, I )
//                  << setw ( 14 ) << fracturePressure
//                  << setw ( 14 ) << Current_Layer -> Current_Properties ( Basin_Modelling::VES_FP, K, J, I ) * Pa_To_MPa
//                  << setw ( 14 ) << Current_Layer -> Current_Properties ( Basin_Modelling::Max_VES, K, J, I ) * Pa_To_MPa
//                  << setw ( 14 ) << Porosity.mixedProperty ()
//                  << setw ( 15 ) << log10 ( Permeability_Normal_Compound / MILLIDARCYTOM2 )
//                  << setw (  4 ) << int ( Current_Layer -> fracturedPermeabilityScaling ( I, J, K ))
//                  << setw (  4 ) << value
//                  << setw ( 14 ) << Current_Layer -> Current_Properties ( Basin_Modelling::Temperature, K, J, I )
//                  << setw ( 14 ) << Current_Layer -> Current_Properties ( Basin_Modelling::Chemical_Compaction, K, J, I )
//                  << setw (  4 ) << int ( DOFs ( GlobalK [ K ], J, I ))
//                  << setw (  4 ) << includedDOF
//                  << endl;

            buffer << setw ( 20 ) << Layer_Name << "  " ;
            buffer << setw (  4 ) << I;
            buffer << setw (  4 ) << J;
            buffer << setw (  4 ) << K;
            buffer << setw ( 14 ) << currentAge;
            buffer << setw ( 14 ) << Current_Layer -> Current_Properties ( Basin_Modelling::Depth, K, J, I );
            buffer << setw ( 14 ) << Current_Layer -> Current_Properties ( Basin_Modelling::Overpressure, K, J, I );
            buffer << setw ( 14 ) << Current_Layer -> Current_Properties ( Basin_Modelling::Pore_Pressure, K, J, I );
            buffer << setw ( 14 ) << Current_Layer -> Current_Properties ( Basin_Modelling::Hydrostatic_Pressure, K, J, I );
            buffer << setw ( 14 ) << Current_Layer -> Current_Properties ( Basin_Modelling::Lithostatic_Pressure, K, J, I );
            buffer << setw ( 14 ) << fracturePressure;
            buffer << setw ( 14 ) << Current_Layer -> Current_Properties ( Basin_Modelling::VES_FP, K, J, I ) * Pa_To_MPa;
            buffer << setw ( 14 ) << Current_Layer -> Current_Properties ( Basin_Modelling::Max_VES, K, J, I ) * Pa_To_MPa;
            buffer << setw ( 14 ) << Porosity.mixedProperty ();
            buffer << setw ( 14 ) << log10 ( Permeability_Normal_Compound / MILLIDARCYTOM2 );
            buffer << setw ( 15 ) << log10 ( Permeability_Plane_Compound / MILLIDARCYTOM2 );
            buffer << setw (  4 ) << ( Current_Layer->kind () == Interface::SEDIMENT_FORMATION ? int ( Current_Layer -> fracturedPermeabilityScaling ( I, J, K )) : -1 );
            buffer << setw (  4 ) << value;
            buffer << setw ( 14 ) << Current_Layer -> Current_Properties ( Basin_Modelling::Temperature, K, J, I );
            buffer << setw ( 14 ) << Current_Layer -> Current_Properties ( Basin_Modelling::Chemical_Compaction, K, J, I );
            buffer << setw (  4 ) << int ( DOFs ( GlobalK [ K ], J, I ));
            buffer << setw (  4 ) << includedDOF;

            if ( includedInDarcySimulation and Current_Layer->kind () == Interface::SEDIMENT_FORMATION ) {

               if ( K == Z_Start + Z_Count - 1 ) {
                  buffer << setw ( 14 ) << saturations ( K - 1, elementJ, elementI )( Saturation::WATER );
                  buffer << setw ( 14 ) << saturations ( K - 1, elementJ, elementI )( Saturation::LIQUID );
                  buffer << setw ( 14 ) << saturations ( K - 1, elementJ, elementI )( Saturation::VAPOUR );
                  buffer << setw ( 14 ) << saturations ( K - 1, elementJ, elementI )( Saturation::IMMOBILE );
                  buffer << setw ( 14 ) << concentrations ( K - 1, elementJ, elementI )( pvtFlash::C1 );
                  buffer << setw ( 14 ) << concentrations ( K - 1, elementJ, elementI )( pvtFlash::C2 );
                  buffer << setw ( 14 ) << concentrations ( K - 1, elementJ, elementI )( pvtFlash::C3 );
                  buffer << setw ( 14 ) << concentrations ( K - 1, elementJ, elementI )( pvtFlash::C4 );
                  buffer << setw ( 14 ) << concentrations ( K - 1, elementJ, elementI )( pvtFlash::C5 );
                  buffer << setw ( 14 ) << concentrations ( K - 1, elementJ, elementI )( pvtFlash::C6_14SAT );
                  buffer << setw ( 14 ) << concentrations ( K - 1, elementJ, elementI )( pvtFlash::C6_14ARO );
                  buffer << setw ( 14 ) << concentrations ( K - 1, elementJ, elementI )( pvtFlash::C15_SAT );
                  buffer << setw ( 14 ) << concentrations ( K - 1, elementJ, elementI )( pvtFlash::C15_ARO );
               } else {

                  PVTComponents composition = concentrations ( K, elementJ, elementI );
                  PVTPhaseComponents phaseComposition;
                  PVTPhaseValues     phaseDensities;
                  PVTPhaseValues     phaseViscosities;

                  // composition *= PVTCalc::getInstance ().getMolarMass ();
                  composition *= 0.001;


                  pvtFlash::EosPack::getInstance ().computeWithLumping ( Current_Layer -> Current_Properties ( Basin_Modelling::Temperature, K, J, I ) + 273.15,
                                                              Current_Layer -> Current_Properties ( Basin_Modelling::Pore_Pressure, K, J, I ) * 1.0e6,
                                                              composition.m_components,
                                                              phaseComposition.m_masses,
                                                              phaseDensities.m_values,
                                                              phaseViscosities.m_values );


                  buffer << setw ( 14 ) << saturations ( K, elementJ, elementI )( Saturation::WATER );
                  buffer << setw ( 14 ) << saturations ( K, elementJ, elementI )( Saturation::LIQUID );
                  buffer << setw ( 14 ) << saturations ( K, elementJ, elementI )( Saturation::VAPOUR );
                  buffer << setw ( 14 ) << saturations ( K, elementJ, elementI )( Saturation::IMMOBILE );

                  buffer << setw ( 14 ) << concentrations ( K, elementJ, elementI )( pvtFlash::C1 );
                  buffer << setw ( 14 ) << concentrations ( K, elementJ, elementI )( pvtFlash::C2 );
                  buffer << setw ( 14 ) << concentrations ( K, elementJ, elementI )( pvtFlash::C3 );
                  buffer << setw ( 14 ) << concentrations ( K, elementJ, elementI )( pvtFlash::C4 );
                  buffer << setw ( 14 ) << concentrations ( K, elementJ, elementI )( pvtFlash::C5 );
                  buffer << setw ( 14 ) << concentrations ( K, elementJ, elementI )( pvtFlash::C6_14SAT );
                  buffer << setw ( 14 ) << concentrations ( K, elementJ, elementI )( pvtFlash::C6_14ARO );
                  buffer << setw ( 14 ) << concentrations ( K, elementJ, elementI )( pvtFlash::C15_SAT );
                  buffer << setw ( 14 ) << concentrations ( K, elementJ, elementI )( pvtFlash::C15_ARO );
                  buffer << setw ( 14 ) << concentrations ( K, elementJ, elementI )( pvtFlash::RESINS );
                  buffer << setw ( 14 ) << concentrations ( K, elementJ, elementI )( pvtFlash::ASPHALTENES );

                  buffer << setw ( 14 ) << phaseDensities ( pvtFlash::VAPOUR_PHASE );
                  buffer << setw ( 14 ) << phaseDensities ( pvtFlash::LIQUID_PHASE );
                  buffer << setw ( 14 ) << phaseViscosities ( pvtFlash::VAPOUR_PHASE );
                  buffer << setw ( 14 ) << phaseViscosities ( pvtFlash::LIQUID_PHASE );

                  c1Sum += concentrations ( K, elementJ, elementI )( pvtFlash::C1 );
                  c2Sum += concentrations ( K, elementJ, elementI )( pvtFlash::C2 );
                  c3Sum += concentrations ( K, elementJ, elementI )( pvtFlash::C3 );
                  c4Sum += concentrations ( K, elementJ, elementI )( pvtFlash::C4 );
                  c5Sum += concentrations ( K, elementJ, elementI )( pvtFlash::C5 );
                  c6AroSum += concentrations ( K, elementJ, elementI )( pvtFlash::C6_14ARO );
                  c6SatSum += concentrations ( K, elementJ, elementI )( pvtFlash::C6_14SAT );
                  c15AroSum += concentrations ( K, elementJ, elementI )( pvtFlash::C15_ARO );
                  c15SatSum += concentrations ( K, elementJ, elementI )( pvtFlash::C15_SAT );

                  double phiAbove = Current_Lithology->porosity ( Current_Layer->Current_Properties ( Basin_Modelling::VES_FP, K + 1, J, I ),
                                                                  Current_Layer->Current_Properties ( Basin_Modelling::Max_VES, K + 1, J, I ), false, 0.0 );

                  double phiBelow = Current_Lithology->porosity ( Current_Layer->Current_Properties ( Basin_Modelling::VES_FP, K, J, I ),
                                                                  Current_Layer->Current_Properties ( Basin_Modelling::Max_VES, K, J, I ), false, 0.0 );

                  double averagePorosity = 0.5 * ( phiAbove + phiBelow );

                  double depthAbove = Current_Layer->Current_Properties ( Basin_Modelling::Depth, K + 1, J, I );
                  double depthBelow = Current_Layer->Current_Properties ( Basin_Modelling::Depth, K, J, I );

                  double thickness = 0.5 * ( depthAbove + depthBelow );

                  c1Sum2 += concentrations ( K, elementJ, elementI )( pvtFlash::C1 ) * averagePorosity * thickness;
                  c2Sum2 += concentrations ( K, elementJ, elementI )( pvtFlash::C2 ) * averagePorosity * thickness;
                  c3Sum2 += concentrations ( K, elementJ, elementI )( pvtFlash::C3 ) * averagePorosity * thickness;
                  c4Sum2 += concentrations ( K, elementJ, elementI )( pvtFlash::C4 ) * averagePorosity * thickness;
                  c5Sum2 += concentrations ( K, elementJ, elementI )( pvtFlash::C5 ) * averagePorosity * thickness;
                  c6AroSum2 += concentrations ( K, elementJ, elementI )( pvtFlash::C6_14ARO ) * averagePorosity * thickness;
                  c6SatSum2 += concentrations ( K, elementJ, elementI )( pvtFlash::C6_14SAT ) * averagePorosity * thickness;
                  c15AroSum2 += concentrations ( K, elementJ, elementI )( pvtFlash::C15_ARO ) * averagePorosity * thickness;
                  c15SatSum2 += concentrations ( K, elementJ, elementI )( pvtFlash::C15_SAT ) * averagePorosity * thickness;

               }

            } else {
               buffer << setw ( 14 ) << 1.0 << setw ( 14 ) << 0.0 << setw ( 14 ) << 0.0 << setw ( 14 ) << 0.0
                      << setw ( 14 ) << 0.0 << setw ( 14 ) << 0.0 << setw ( 14 ) << 0.0
                      << setw ( 14 ) << 0.0 << setw ( 14 ) << 0.0 << setw ( 14 ) << 0.0
                      << setw ( 14 ) << 0.0 << setw ( 14 ) << 0.0 << setw ( 14 ) << 0.0;
            }
            buffer << endl;
          }

        }

      }

    }

    Current_Layer -> Current_Properties.Restore_Properties ();
  }

  buffer << setw ( 303 ) << " ";
  buffer << setw ( 14 ) << c1Sum;
  buffer << setw ( 14 ) << c2Sum;
  buffer << setw ( 14 ) << c3Sum;
  buffer << setw ( 14 ) << c4Sum;
  buffer << setw ( 14 ) << c5Sum;
  buffer << setw ( 14 ) << c6AroSum;
  buffer << setw ( 14 ) << c6SatSum;
  buffer << setw ( 14 ) << c15AroSum;
  buffer << setw ( 14 ) << c15SatSum;
  buffer << endl;

  buffer << setw ( 303 ) << " ";
  buffer << setw ( 14 ) << c1Sum2;
  buffer << setw ( 14 ) << c2Sum2;
  buffer << setw ( 14 ) << c3Sum2;
  buffer << setw ( 14 ) << c4Sum2;
  buffer << setw ( 14 ) << c5Sum2;
  buffer << setw ( 14 ) << c6AroSum2;
  buffer << setw ( 14 ) << c6SatSum2;
  buffer << setw ( 14 ) << c15AroSum2;
  buffer << setw ( 14 ) << c15SatSum2;
  buffer << endl;


  if (( X_Start <= I ) && ( I < X_Start + X_Count ) && ( Y_Start <= J ) && ( J < Y_Start + Y_Count ) && Valid_Needle ( I, J )) {
    buffer << endl << endl;
  }

  if ( onThisNode ) {
     cout << buffer.str () << endl << flush;
  }

#if 0
  PetscSynchronizedPrintf ( PETSC_COMM_WORLD, buffer.str ().c_str ());
#endif

  PetscSynchronizedFlush ( PETSC_COMM_WORLD );
}

//------------------------------------------------------------//


void Basin_Modelling::FEM_Grid::printElementNeedle ( const int i, const int j ) const {

   Layer_Iterator layers;
   layers.Initialise_Iterator ( basinModel->layers, Descending, Sediments_Only, Active_Layers_Only );

   int k;

   const FastcauldronSimulator& fastcauldron = FastcauldronSimulator::getInstance ();
   const ElementGrid&           grid         = fastcauldron.getElementGrid ();
   const MapElementArray&       mapElements  = fastcauldron.getMapElementArray ();

   if ( not NumericFunctions::inRange ( i, grid.firstI (), grid.lastI ()) or
        not NumericFunctions::inRange ( j, grid.firstJ (), grid.lastJ ())) {
      return;
   }


   const LayerElement* neighbour;
  
   for ( layers.Initialise_Iterator (); ! layers.Iteration_Is_Done (); layers++ ) {

      LayerProps* layer = layers.Current_Layer ();
      LayerProps* above = layers.Layer_Above ();
      LayerProps* below = layers.Layer_Below ();

      cout << "------------------------------------------------------------"  << endl;
      cout << " Formation name: " << layer->layername << endl;

      if ( above != 0 ) {
         cout << " Element above " << endl;
         cout << layer->getLayerElement ( i, j, layer->getMaximumNumberOfElements () - 1 ).getNeighbour ( VolumeData::ShallowFace )->image () << endl; 
      }

      neighbour = layer->getLayerElement ( i, j, layer->getMaximumNumberOfElements () - 1 ).getActiveNeighbour ( VolumeData::ShallowFace );

      if ( neighbour != 0 ) {
         cout << " active element above " << endl;
         cout << neighbour->image () << endl; 
      } else {
         cout << " neighbour is null " << endl; 
      } 

      for ( k = layer->getMaximumNumberOfElements () - 1; k >= 0; --k ) {
         cout << " Element " << k << endl;
         cout << layer->getLayerElement ( i, j, k ).image () << endl;
         cout << endl;
      }

      if ( below != 0 ) {
         cout << " Element below " << endl;
         cout << layer->getLayerElement ( i, j, 0 ).getNeighbour ( VolumeData::DeepFace )->image () << endl;        
      }

      neighbour = layer->getLayerElement ( i, j, 0 ).getActiveNeighbour ( VolumeData::DeepFace );

      if ( neighbour != 0 ) {
         cout << " active element below " << endl;
         cout << neighbour->image () << endl; 
      } else {
         cout << " neighbour is null " << endl; 
      } 

      cout << endl;
      cout << endl;
   }


}

//------------------------------------------------------------//
void Basin_Modelling::FEM_Grid::Determine_Permafrost_Time_Step ( const double  Current_Time, double & Time_Step ) {

   if( basinModel->permafrost () ) {
      if( basinModel->fixedTimeStep() > 0.0 ) {
         // the fixed time step overwrites all other timesteps
         Time_Step = basinModel->fixedTimeStep();
      } else if ( basinModel->switchPermafrostTimeStep( Current_Time )) {
         Time_Step = NumericFunctions::Minimum ( Time_Step, basinModel->permafrostTimeStep() );
      }
   } 
}


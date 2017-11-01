//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

//------------------------------------------------------------//

#include "fem_grid.h"
#include "fem_grid_auxiliary_functions.h"
#include "FiniteElementTypes.h"
#include "AllochthonousLithologyManager.h"
#include "HydraulicFracturingManager.h"
#include "PropertyManager.h"

//------------------------------------------------------------//

#include <sstream>
#include <algorithm>

//------------------------------------------------------------//

#include <petscksp.h>
#include <petsc.h>
#include <petscdmda.h>

//------------------------------------------------------------//

#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>

//utilities library
#include "LogHandler.h"

//------------------------------------------------------------//

#include "ConstantsFastcauldron.h"
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
//utilities library
#include "LogHandler.h"
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
#include "PetscSolver.h"

#include "VitriniteReflectance.h"
#include "TemperatureForVreInputGrid.h"
#include "VreOutputGrid.h"

#include "ComputationalDomain.h"
#include "CompositeElementActivityPredicate.h"
#include "ElementNonZeroPorosityActivityPredicate.h"
#include "ElementThicknessActivityPredicate.h"
#include "SedimentElementActivityPredicate.h"
#include "MatrixNonZeroCalculator.h"
#include "VtkMeshWriter.h"
#include "SolutionVectorMapping.h"
#include "PetscObjectAllocator.h"


#include <boost/shared_ptr.hpp>

#include "PetscObjectsIO.h"

//------------------------------------------------------------//

// utilities library
#include "ConstantsMathematics.h"

#include "ConstantsNumerical.h"
#include "ConstantsGeology.h"

//------------------------------------------------------------//

// CBMGenerics library
#include "ComponentManager.h"

//------------------------------------------------------------//

//
// How to handle the PETSc Matrix allocation. Has the bug been
// fixed in the CBM version of the allocation function.
//
#define PETSc_MATRIX_BUG_FIXED 1

//------------------------------------------------------------//

using Utilities::Maths::Zero;
using Utilities::Maths::PaToMegaPa;
using Utilities::Maths::MilliDarcyToM2;
using Utilities::Numerical::CauldronNoDataValue;
using Utilities::Geology::PresentDay;

using namespace FiniteElementMethod;
using namespace GeoPhysics;
using namespace Utilities::CheckMemory;

typedef CBMGenerics::ComponentManager::SpeciesNamesId ComponentId;

//------------------------------------------------------------//


#undef  __FUNCT__
#define __FUNCT__ "Basin_Modelling::FEM_Grid::contructor"

Basin_Modelling::FEM_Grid::FEM_Grid ( AppCtx* Application_Context )
   : m_vreOutputGrid(Application_Context->mapDA, Application_Context->layers),
     m_vreAlgorithm(GeoPhysics::VitriniteReflectance::create( FastcauldronSimulator::getInstance ().getRunParameters ()->getVreAlgorithm () ) ),
     m_chemicalCompactionGrid ( ChemicalCompactionGrid::create( FastcauldronSimulator::getInstance ().getRunParameters ()->getChemicalCompactionAlgorithm (),
                                                                FastcauldronSimulator::getInstance().getRunParameters()->getLegacy(),
                                                                Application_Context -> mapDA,
                                                                Application_Context -> layers )),
     m_chemicalCompactionCalculator ( m_chemicalCompactionGrid -> createChemicalCompaction() ),
     Temperature_Calculator ( Application_Context ),
     m_surfaceNodeHistory ( Application_Context ),

     // Construct the computational domain for the temperature simulation.
     // This requires all layers in the domain.
     m_temperatureComputationalDomain ( *Application_Context->layers [ 0 ],
                                        *Application_Context->layers [ Application_Context->layers.size () - 1 ],
                                        CompositeElementActivityPredicate ().compose ( ElementActivityPredicatePtr ( new ElementThicknessActivityPredicate ))),

     // Construct the computational domain for the pressure simulation.
     // This requires only the sediment layers in the domain.
     m_pressureComputationalDomain ( *Application_Context->layers [ 0 ],
                                     *Application_Context->layers [ Application_Context->layers.size () - 3 ],
                                     CompositeElementActivityPredicate ().compose ( ElementActivityPredicatePtr ( new ElementThicknessActivityPredicate ))
                                                                         .compose ( ElementActivityPredicatePtr ( new SedimentElementActivityPredicate ))
                                                                         .compose ( ElementActivityPredicatePtr ( new ElementNonZeroPorosityActivityPredicate )))

{

  basinModel = Application_Context;

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
  Property_Saving_Time = 0.0;

  Accumulated_Element_Assembly_Time = 0.0;
  Accumulated_System_Assembly_Time = 0.0;
  Accumulated_System_Solve_Time = 0.0;
  Accumulated_Property_Calculation_Time = 0.0;
  Accumulated_Property_Saving_Time = 0.0;

  m_temperatureDomainResetTime = 0.0;
  m_temperatureMatrixAllocationTime = 0.0;
  m_temperatureSolutionMappingTime = 0.0;

  // RESPECT ALPHABETIC ORDER WHEN ADDING/EDITING PROPERTY
  // RESPECT ORGANISATION WHEN ADDING/EDITING PROPERTY

  //------------------------------------------------------------------------------------------------
  //1. Map properties----------------------------------------------------------------------------------
  //---------------------------------------------------------------------------------------------------


  //1.0 Forced outputs map properties-----
  //--------------------------------------
  // We update the project file to force output, they are required for the calculation

  basinModel->timefilter.setFilter( "AllochthonousLithology", "SedimentsOnly" );
  mapOutputProperties.push_back ( ALLOCHTHONOUS_LITHOLOGY );
  basinModel->timefilter.setFilter( "ErosionFactor", "SedimentsOnly" );
  mapOutputProperties.push_back ( EROSIONFACTOR );

  // If we have active faults
  if (FastcauldronSimulator::getInstance().getBasinHasActiveFaults()) {
     mapOutputProperties.push_back ( FAULTELEMENTS );
     basinModel->timefilter.setFilter( "FaultElements", "SedimentsOnly" );
  }

  // If we are using the advanced lithospheric calculator
  if (basinModel->isALC()) {
     PetscBool debugAlc = PETSC_FALSE;
     PetscOptionsHasName( PETSC_NULL, "-debugalc", &debugAlc );

     //default alc outputs
     mapOutputProperties.push_back( TOP_BASALT_ALC );
     basinModel->timefilter.setFilter( "ALCStepTopBasaltDepth", "SedimentsPlusBasement" );
     FastcauldronSimulator::getInstance().setOutputPropertyOption( TOP_BASALT_ALC, Interface::SEDIMENTS_AND_BASEMENT_OUTPUT );
     mapOutputProperties.push_back( MOHO_ALC );
     basinModel->timefilter.setFilter( "ALCStepMohoDepth", "SedimentsPlusBasement" );
     FastcauldronSimulator::getInstance().setOutputPropertyOption( MOHO_ALC, Interface::SEDIMENTS_AND_BASEMENT_OUTPUT );
     if (basinModel->timefilter.PropertyOutputOption[THICKNESS_CONTINENTAL_CRUST_ALC] != NOOUTPUT) {
        mapOutputProperties.push_back( THICKNESS_CONTINENTAL_CRUST_ALC );
        basinModel->timefilter.setFilter( "ALCStepContCrustThickness", "SedimentsPlusBasement" );
     }
     if (basinModel->timefilter.PropertyOutputOption[THICKNESS_OCEANIC_CRUST_ALC] != NOOUTPUT) {
        mapOutputProperties.push_back( THICKNESS_OCEANIC_CRUST_ALC );
        basinModel->timefilter.setFilter( "ALCStepBasaltThickness", "SedimentsPlusBasement" );
     }

     //debug alc outputs
     if (debugAlc) {
        LogHandler( LogHandler::DEBUG_SEVERITY ) << "Outputing alc debug outputs because '-debugalc' is specified.";
        if (basinModel->timefilter.PropertyOutputOption[ALC_ORIGINAL_MANTLE] != NOOUTPUT) {
           mapOutputProperties.push_back( ALC_ORIGINAL_MANTLE );
           basinModel->timefilter.setFilter( "ALCOrigLithMantleDepth", "SedimentsPlusBasement" );
        }
        if (basinModel->timefilter.PropertyOutputOption[ALC_SM_THICKNESS_CONTINENTAL_CRUST] != NOOUTPUT) {
           mapOutputProperties.push_back( ALC_SM_THICKNESS_CONTINENTAL_CRUST );
           basinModel->timefilter.setFilter( "ALCSmContCrustThickness", "SedimentsPlusBasement" );
        }
        if (basinModel->timefilter.PropertyOutputOption[ALC_SM_TOP_BASALT] != NOOUTPUT) {
           mapOutputProperties.push_back( ALC_SM_TOP_BASALT );
           basinModel->timefilter.setFilter( "ALCSmTopBasaltDepth", "SedimentsPlusBasement" );
        }
        if (basinModel->timefilter.PropertyOutputOption[ALC_SM_MOHO] != NOOUTPUT) {
           mapOutputProperties.push_back( ALC_SM_MOHO );
           basinModel->timefilter.setFilter( "ALCSmMohoDepth", "SedimentsPlusBasement" );
        }
        if (basinModel->timefilter.PropertyOutputOption[ALC_MAX_MANTLE_DEPTH] != NOOUTPUT) {
           mapOutputProperties.push_back( ALC_MAX_MANTLE_DEPTH );
           basinModel->timefilter.setFilter( "ALCMaxAsthenoMantleDepth", "SedimentsPlusBasement" );
        }
        if (basinModel->timefilter.PropertyOutputOption[ALC_SM_THICKNESS_OCEANIC_CRUST] != NOOUTPUT) {
           mapOutputProperties.push_back( ALC_SM_THICKNESS_OCEANIC_CRUST );
           basinModel->timefilter.setFilter( "ALCSmBasaltThickness", "SedimentsPlusBasement" );
        }
     }
  }


  //1.1 Regular map properties------------
  //--------------------------------------
  // Set map properties that may be required for the calculation

  mapOutputProperties.push_back ( BULKDENSITYVEC );
  mapOutputProperties.push_back ( CHEMICAL_COMPACTION );
  mapOutputProperties.push_back ( DEPTH );
  mapOutputProperties.push_back ( DIFFUSIVITYVEC );
  mapOutputProperties.push_back ( FRACTURE_PRESSURE );
  mapOutputProperties.push_back ( HEAT_FLOW );
  mapOutputProperties.push_back ( HYDROSTATICPRESSURE );
  mapOutputProperties.push_back ( LITHOSTATICPRESSURE );
  mapOutputProperties.push_back ( MAXVES );
  mapOutputProperties.push_back ( OVERPRESSURE );
  mapOutputProperties.push_back ( PERMEABILITYHVEC );
  mapOutputProperties.push_back ( PERMEABILITYVEC );
  mapOutputProperties.push_back ( PRESSURE );
  mapOutputProperties.push_back ( POROSITYVEC );
  mapOutputProperties.push_back ( REFLECTIVITYVEC );
  mapOutputProperties.push_back ( TEMPERATURE );
  mapOutputProperties.push_back ( THCONDVEC );
  mapOutputProperties.push_back ( THICKNESS );
  mapOutputProperties.push_back ( TWOWAYTIME );
  mapOutputProperties.push_back ( TWOWAYTIME_RESIDUAL );
  mapOutputProperties.push_back ( SONICVEC );
  mapOutputProperties.push_back ( VELOCITYVEC );
  mapOutputProperties.push_back ( VES );
  mapOutputProperties.push_back ( VR );

  if( not FastcauldronSimulator::getInstance ().noDerivedPropertiesCalc() ) {
     m_mapDerivedOutputProperties.push_back ( ALLOCHTHONOUS_LITHOLOGY );
     m_mapDerivedOutputProperties.push_back ( BULKDENSITYVEC );
     m_mapDerivedOutputProperties.push_back ( DIFFUSIVITYVEC );
     m_mapDerivedOutputProperties.push_back ( FAULTELEMENTS );
     m_mapDerivedOutputProperties.push_back ( FLUID_VELOCITY );
     m_mapDerivedOutputProperties.push_back ( FRACTURE_PRESSURE );
     m_mapDerivedOutputProperties.push_back ( HEAT_FLOW );
     m_mapDerivedOutputProperties.push_back ( HYDROSTATICPRESSURE );
     m_mapDerivedOutputProperties.push_back ( LITHOSTATICPRESSURE );
     m_mapDerivedOutputProperties.push_back ( OVERPRESSURE );
     m_mapDerivedOutputProperties.push_back ( PERMEABILITYVEC );
     m_mapDerivedOutputProperties.push_back ( POROSITYVEC );
     m_mapDerivedOutputProperties.push_back ( REFLECTIVITYVEC );
     m_mapDerivedOutputProperties.push_back ( THCONDVEC );
     m_mapDerivedOutputProperties.push_back ( THICKNESS );
     m_mapDerivedOutputProperties.push_back ( TWOWAYTIME );
     m_mapDerivedOutputProperties.push_back ( TWOWAYTIME_RESIDUAL );
     m_mapDerivedOutputProperties.push_back ( SONICVEC );
     m_mapDerivedOutputProperties.push_back ( VELOCITYVEC );

     m_mapDerivedOutputProperties.push_back ( CHEMICAL_COMPACTION );
     m_mapDerivedOutputProperties.push_back ( VES );
     m_mapDerivedOutputProperties.push_back ( DEPTH );
     m_mapDerivedOutputProperties.push_back ( MAXVES );
     m_mapDerivedOutputProperties.push_back ( TEMPERATURE );
     m_mapDerivedOutputProperties.push_back ( PRESSURE );
     m_mapDerivedOutputProperties.push_back ( VR );
   }
  // Set map properties for 1D simulation mode only
  if (basinModel->isModellingMode1D())
  {
     mapOutputProperties.push_back( ILLITEFRACTION );
     mapOutputProperties.push_back( BIOMARKERS );
  }

  // If the pressure calculator is changed to solve for the pore-pressure (or Hubberts potential, see Annette)
  // then remove the if statement. Keep only the assignment of the fluid-velocities.
  if (basinModel->DoOverPressure or basinModel->Do_Iteratively_Coupled)  {
     mapOutputProperties.push_back ( FLUID_VELOCITY );
  }

  // Set map properties computed by Darcy flow simulation
  if (FastcauldronSimulator::getInstance().getMcfHandler().solveFlowEquations()) {
     mapOutputProperties.push_back( GENEX_PROPERTY_CONCENTRATIONS );
     mapOutputProperties.push_back( SATURATION );
  }

  // Set map properties computed by GenEx
  if (FastcauldronSimulator::getInstance().getCauldron()->integrateGenexEquations()) {
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

  // Set map properties required by GenEx , they are saved at minor snapshots for Genex
  // In case GenEx is run after fastcauldron
  genexOutputProperties.push_back ( EROSIONFACTOR );
  genexOutputProperties.push_back ( MAXVES );
  genexOutputProperties.push_back ( PRESSURE );
  genexOutputProperties.push_back ( TEMPERATURE );
  genexOutputProperties.push_back ( VES );
  genexOutputProperties.push_back ( VR );

  // Set map properties required by SGS (Shale Gas Simulator) , they are saved at minor snapshots for Genex
  // In case SGS is run after fastcauldron
  shaleGasOutputProperties.push_back ( CHEMICAL_COMPACTION );
  shaleGasOutputProperties.push_back ( EROSIONFACTOR );
  shaleGasOutputProperties.push_back ( HYDROSTATICPRESSURE );
  shaleGasOutputProperties.push_back ( LITHOSTATICPRESSURE );
  shaleGasOutputProperties.push_back ( MAXVES );
  shaleGasOutputProperties.push_back ( PERMEABILITYVEC );
  shaleGasOutputProperties.push_back ( POROSITYVEC );
  shaleGasOutputProperties.push_back ( PRESSURE );
  shaleGasOutputProperties.push_back ( TEMPERATURE );
  shaleGasOutputProperties.push_back ( VES );
  shaleGasOutputProperties.push_back ( VR );


  //1.2 Concluding map properties---------
  //--------------------------------------
  // Set the map properties that may only required at present time t=0Ma

  if (FastcauldronSimulator::getInstance().getCalculationMode() == OVERPRESSURE_MODE or
     FastcauldronSimulator::getInstance().getCalculationMode() == PRESSURE_AND_TEMPERATURE_MODE or
     FastcauldronSimulator::getInstance().getCalculationMode() == COUPLED_DARCY_MODE) {
     m_concludingMapOutputProperties.push_back ( FCTCORRECTION );
  }
  if ((FastcauldronSimulator::getInstance().getCalculationMode() == OVERPRESSURE_MODE or
     FastcauldronSimulator::getInstance().getCalculationMode() == PRESSURE_AND_TEMPERATURE_MODE or
     FastcauldronSimulator::getInstance().getCalculationMode() == COUPLED_DARCY_MODE) and
     not FastcauldronSimulator::getInstance().getRunParameters()->getNonGeometricLoop()) {
     m_concludingMapOutputProperties.push_back ( THICKNESSERROR );
  }


  //1.4 Debug properties------------------
  //--------------------------------------

#if BRINE_PROPERTIES //Brine properties: density and viscosity
  mapOutputProperties.push_back ( BRINE_PROPERTIES );
#endif

#if DEBUG_1D_PROPERTIES
  if (basinModel->isModellingMode1D() && basinModel->filterwizard.IsSmectiteIlliteCalculationNeeded())
  {
     mapOutputProperties.push_back ( ILLITEFRACTION );
  }
  if (basinModel->isModellingMode1D() && basinModel->filterwizard.IsBiomarkersCalculationNeeded())
  {
     mapOutputProperties.push_back ( STERANEAROMATISATION );
     mapOutputProperties.push_back ( STERANEISOMERISATION );
     mapOutputProperties.push_back ( HOPANEISOMERISATION  );
  }
#endif

#if DEBUG_DARCY
  if (FastcauldronSimulator::getInstance().getMcfHandler().solveFlowEquations()) {
     if (true or FastcauldronSimulator::getInstance().getMcfHandler().outputDarcyMaps()) {
        mapOutputProperties.push_back ( AVERAGE_SATURATION );
        mapOutputProperties.push_back ( CAPILLARY_PRESSURE );
        mapOutputProperties.push_back ( FLUID_PROPERTIES );
     }
  }
#endif


  //------------------------------------------------------------------------------------------------
  //2. Volume properties----------------------------------------------------------------------------
  //------------------------------------------------------------------------------------------------


  //2.0 Forced/Blocked volume (and therefore map) properties
  //--------------------------------------
  // We update the project file to force or block output (both for map and volume), they are required for the calculation

  // Block properties output for overpressure-temperature and coupled-high-resolution-decompaction
  if (FastcauldronSimulator::getInstance().getCalculationMode() == OVERPRESSURED_TEMPERATURE_MODE or
     FastcauldronSimulator::getInstance().getCalculationMode() == COUPLED_HIGH_RES_DECOMPACTION_MODE)
  {
     FastcauldronSimulator::getInstance().setOutputPropertyOption( HYDROSTATICPRESSURE, Interface::NO_OUTPUT );
     FastcauldronSimulator::getInstance().setOutputPropertyOption( LITHOSTATICPRESSURE, Interface::NO_OUTPUT );
     FastcauldronSimulator::getInstance().setOutputPropertyOption( MAXVES, Interface::NO_OUTPUT );
     FastcauldronSimulator::getInstance().setOutputPropertyOption( OVERPRESSURE, Interface::NO_OUTPUT );
     FastcauldronSimulator::getInstance().setOutputPropertyOption( PRESSURE, Interface::NO_OUTPUT );
     FastcauldronSimulator::getInstance().setOutputPropertyOption( VES, Interface::NO_OUTPUT );
  }

  // If we are doing overpressure calculation only then set lithostatic to be no more that sediments-only for output.
  if (basinModel->DoOverPressure &&
     basinModel->timefilter.PropertyOutputOption[LITHOSTATICPRESSURE] == SEDIMENTSPLUSBASEMENT) {
     basinModel->timefilter.setFilter( "LithoStaticPressure", "SedimentsOnly" );
  }

  // If temperature is being calculated then make sure that temperature is being output.
  if ((basinModel->DoTemperature || basinModel->Do_Iteratively_Coupled) and
      (basinModel->timefilter.PropertyOutputOption[TEMPERATURE] == NOOUTPUT)) {
     basinModel->timefilter.setFilter( "Temperature", "SourceRockOnly" );
  }

  // If overpressure is being calculated then Ves and MaxVes MUST be output for
  // possible later loosely coupled calculations.
  if (basinModel->DoOverPressure || basinModel->Do_Iteratively_Coupled) {
     if (basinModel->timefilter.PropertyOutputOption[VES] == NOOUTPUT ||
        basinModel->timefilter.PropertyOutputOption[VES] == SOURCEROCKONLY) {
        basinModel->timefilter.setFilter( "Ves", "SedimentsOnly" );
     }
     if (basinModel->timefilter.PropertyOutputOption[MAXVES] == NOOUTPUT ||
        basinModel->timefilter.PropertyOutputOption[MAXVES] == SOURCEROCKONLY) {
        basinModel->timefilter.setFilter( "MaxVes", "SedimentsOnly" );
     }
  }


  //2.1 Primary volume properties---------
  //--------------------------------------
  //Set the primary volume properties that may be required for the simulation

  m_volumeOutputProperties.push_back( CHEMICAL_COMPACTION );
  m_volumeOutputProperties.push_back( DEPTH );
  m_volumeOutputProperties.push_back( MAXVES );
  m_volumeOutputProperties.push_back( PRESSURE );
  m_volumeOutputProperties.push_back( TEMPERATURE );
  m_volumeOutputProperties.push_back( VES );
  m_volumeOutputProperties.push_back( VR );

  // Set primary volume properties for 1D simulation mode only
  // Will they be derived properties?
  if (basinModel->isModellingMode1D())
  {
     m_volumeOutputProperties.push_back( ILLITEFRACTION );
     m_volumeOutputProperties.push_back( BIOMARKERS );
  }

  // Set primary volume properties computed by Darcy flow equation
  if (FastcauldronSimulator::getInstance().getMcfHandler().solveFlowEquations()) {
     m_volumeOutputProperties.push_back( CAPILLARY_PRESSURE );
     m_volumeOutputProperties.push_back( FLUID_PROPERTIES );
     m_volumeOutputProperties.push_back( GENEX_PROPERTY_CONCENTRATIONS );
     m_volumeOutputProperties.push_back( PVT_PROPERTIES );
     m_volumeOutputProperties.push_back( RELATIVE_PERMEABILITY );
     m_volumeOutputProperties.push_back( SATURATION );
     if (FastcauldronSimulator::getInstance().getMcfHandler().saveVolumeOutput()) {
        m_volumeOutputProperties.push_back( VOLUME_CALCULATIONS );
     }
     if (FastcauldronSimulator::getInstance().getMcfHandler().saveTransportedVolumeOutput()) {
        m_volumeOutputProperties.push_back( TRANSPORTED_VOLUME_CALCULATIONS );
     }
  }

  //2.2 Derieved volume properties--------
  //--------------------------------------
  //Set the derived volume properties that may be required for the simulation

  if( not Application_Context->primaryOutput() ) {
     m_volumeOutputProperties.push_back ( BRINE_PROPERTIES );
     m_volumeOutputProperties.push_back ( BULKDENSITYVEC );
     m_volumeOutputProperties.push_back ( DIFFUSIVITYVEC );
     m_volumeOutputProperties.push_back ( FLUID_VELOCITY );
     m_volumeOutputProperties.push_back ( FRACTURE_PRESSURE );
     m_volumeOutputProperties.push_back ( HEAT_FLOW );
     m_volumeOutputProperties.push_back ( HYDROSTATICPRESSURE );
     m_volumeOutputProperties.push_back ( LITHOSTATICPRESSURE );
     m_volumeOutputProperties.push_back ( OVERPRESSURE );
     m_volumeOutputProperties.push_back ( PERMEABILITYVEC );
     m_volumeOutputProperties.push_back ( POROSITYVEC );
     m_volumeOutputProperties.push_back ( REFLECTIVITYVEC );
     m_volumeOutputProperties.push_back ( SONICVEC );
     m_volumeOutputProperties.push_back ( THCONDVEC );
     m_volumeOutputProperties.push_back ( TWOWAYTIME );
     m_volumeOutputProperties.push_back ( VELOCITYVEC );
  } else {
     if( not FastcauldronSimulator::getInstance ().noDerivedPropertiesCalc() ) {
        m_volumeDerivedOutputProperties.push_back ( BULKDENSITYVEC );
        m_volumeDerivedOutputProperties.push_back ( DIFFUSIVITYVEC );
        m_volumeDerivedOutputProperties.push_back ( FLUID_VELOCITY );
        m_volumeDerivedOutputProperties.push_back ( FRACTURE_PRESSURE );
        m_volumeDerivedOutputProperties.push_back ( HEAT_FLOW );
        m_volumeDerivedOutputProperties.push_back ( HYDROSTATICPRESSURE );
        m_volumeDerivedOutputProperties.push_back ( LITHOSTATICPRESSURE );
        m_volumeDerivedOutputProperties.push_back ( OVERPRESSURE );
        m_volumeDerivedOutputProperties.push_back ( PERMEABILITYVEC );
        m_volumeDerivedOutputProperties.push_back ( POROSITYVEC );
        m_volumeDerivedOutputProperties.push_back ( REFLECTIVITYVEC );
        m_volumeDerivedOutputProperties.push_back ( SONICVEC );
        m_volumeDerivedOutputProperties.push_back ( THCONDVEC );
        m_volumeDerivedOutputProperties.push_back ( THICKNESS);
        m_volumeDerivedOutputProperties.push_back ( TWOWAYTIME );
        m_volumeDerivedOutputProperties.push_back ( VELOCITYVEC );

        if( FastcauldronSimulator::getInstance().getCalculationMode() == OVERPRESSURED_TEMPERATURE_MODE ) {
           m_volumeDerivedOutputProperties.push_back ( FAULTELEMENTS );
        }
     }
  }


  //2.3 Concluding volume properties------
  //--------------------------------------
  //Set the volume properties that may only required at present time t = 0Ma

  // Set concluding volume properties computed by flow equation
  if (FastcauldronSimulator::getInstance().getMcfHandler().solveFlowEquations()) {
     m_concludingVolumeOutputProperties.push_back( TIME_OF_ELEMENT_INVASION );
  }

  //2. Loosely coupled properties--------
  //--------------------------------------
  // Not clear to us why we need this list, this will need to be checked when revising properties

  PetscBool addMinorProperties;
  PetscOptionsHasName( PETSC_NULL, "-minor", &addMinorProperties );

  if ( addMinorProperties || Application_Context->primaryOutput() ) {
     looselyCoupledOutputProperties.push_back( CHEMICAL_COMPACTION );
     looselyCoupledOutputProperties.push_back( PRESSURE );
     looselyCoupledOutputProperties.push_back( TEMPERATURE );
     looselyCoupledOutputProperties.push_back( VR );
     looselyCoupledOutputProperties.push_back( DEPTH );
  }
  
  if (Application_Context->primaryOutput() and FastcauldronSimulator::getInstance().getCalculationMode() != OVERPRESSURE_MODE) {
     basinModel->timefilter.setFilter("Depth", "SedimentsPlusBasement");
     FastcauldronSimulator::getInstance().setOutputPropertyOption( DEPTH, Interface::SEDIMENTS_AND_BASEMENT_OUTPUT );
     
     basinModel->timefilter.setFilter("Temperature", "SedimentsPlusBasement");
     FastcauldronSimulator::getInstance().setOutputPropertyOption( TEMPERATURE, Interface::SEDIMENTS_AND_BASEMENT_OUTPUT );
  }

  looselyCoupledOutputProperties.push_back( MAXVES );
  looselyCoupledOutputProperties.push_back( VES );


  //2.2 Debug properties------------------
  //--------------------------------------

#ifdef DEBUG_CAPILLARYPRESSURE
  m_volumeOutputProperties.push_back( CAPILLARYPRESSUREGAS100 );
  m_volumeOutputProperties.push_back( CAPILLARYPRESSUREGAS0 );
  m_volumeOutputProperties.push_back( CAPILLARYPRESSUREOIL100 );
  m_volumeOutputProperties.push_back( CAPILLARYPRESSUREOIL0 );
#endif

#if DEBUG_DARCY
  m_volumeOutputProperties.push_back( AVERAGE_SATURATION );
  m_volumeOutputProperties.push_back( HC_FLUID_VELOCITY );
#endif

#if DEBUG_LOOSELY_COUPLED
  // If we are doing a loosely coupled calculation then do not update the pressure related properties.
  if (basinModel->IsCalculationCoupled && (basinModel->DoTemperature)) {
     basinModel->timefilter.setFilter ( "Ves", "None" );
     basinModel->timefilter.setFilter ( "MaxVes", "None" );
     basinModel->timefilter.setFilter ( "OverPressure", "None" );
     basinModel->timefilter.setFilter ( "Pressure", "None" );
  }
#endif



  //------------------------------------------------------------------------------------------------
  //3. Initialise-----------------------------------------------------------------------------------
  //------------------------------------------------------------------------------------------------
  // Initialise the surfaceNodeHistory object, by reading in the
  // specification file containing all nodes that are to be recorded.
  // Is it used by someone? This will need to be checked when revising properties

  m_surfaceNodeHistory.Read_Spec_File ();
  savedMinorSnapshotTimes.clear ();


  //------------------------------------------------------------------------------------------------
  //4. Matrix and RHS I/O --------------------------------------------------------------------------
  //------------------------------------------------------------------------------------------------
  // The user can request to save the matrix and RHS at a specific time step (or the next closest one)
  // If the saving time step has not been defined nothing will be saved.
  m_saveMatrixToFile   = PETSC_FALSE;
  m_saveInMatlabFormat = PETSC_FALSE;
  m_saveTimeStep       = 0.0;
  PetscOptionsHasName( NULL, "-saveMatrix", &m_saveMatrixToFile );
  if( m_saveMatrixToFile )
  {
     PetscBool status = PETSC_FALSE;
     PetscOptionsGetReal( NULL, "-saveMatrix", &m_saveTimeStep, &status );
     if( !status )
     {
        m_saveMatrixToFile = PETSC_FALSE;
     }
     else
     {
        PetscOptionsHasName( NULL, "-matlab", &m_saveInMatlabFormat );
        m_saveTimeStep = std::min( m_saveTimeStep, FastcauldronSimulator::getInstance().getAgeOfBasin() );
        m_saveTimeStep = std::max( m_saveTimeStep, 0.0 );
     }
  }

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

    PetscPrintf ( PETSC_COMM_WORLD, " total domain reset time         %f \n", m_temperatureDomainResetTime );
    PetscPrintf ( PETSC_COMM_WORLD, " total matrix allocation time    %f \n", m_temperatureMatrixAllocationTime );
    PetscPrintf ( PETSC_COMM_WORLD, " total solution mapping          %f \n", m_temperatureSolutionMappingTime );

  }
  PetscPrintf ( PETSC_COMM_WORLD, "Total Property_Saving_Time %f \n", Accumulated_Property_Saving_Time );

#if 0
  std::stringstream buffer;

  std::string statistics = StatisticsHandler::print(FastcauldronSimulator::getInstance().getRank());
  PetscPrintf(PETSC_COMM_WORLD, "<statistics>\n");
  PetscSynchronizedFlush(PETSC_COMM_WORLD, PETSC_STDOUT);

  PetscSynchronizedPrintf(PETSC_COMM_WORLD, statistics.c_str());
  PetscSynchronizedFlush(PETSC_COMM_WORLD, PETSC_STDOUT);

  PetscPrintf(PETSC_COMM_WORLD, "</statistics>\n");
  PetscSynchronizedFlush(PETSC_COMM_WORLD, PETSC_STDOUT);

#endif

  delete pressureSolver;
}


//------------------------------------------------------------//

#undef  __FUNCT__
#define __FUNCT__ "Basin_Modelling::FEM_Grid::solvePressure"

void Basin_Modelling::FEM_Grid::solvePressure ( bool& solverHasConverged,
                                                bool& errorInDarcy,
                                                bool& geometryHasConverged ) {

  int    maximumNumberOfGeometricIterations;
  int    numberOfGeometricIterations = 1;
  bool   overpressureHasDiverged;

  geometryHasConverged = true;
  basinModel->initialiseTimeIOTable ( OverpressureRunStatusStr );

  maximumNumberOfGeometricIterations = basinModel->MaxNumberOfRunOverpressure;

  if ( basinModel->debug1 or basinModel->verbose) {
    PetscPrintf ( PETSC_COMM_WORLD, "o Maximum number of Geometric iterations: %d \n", maximumNumberOfGeometricIterations );
    PetscPrintf ( PETSC_COMM_WORLD, " Optimisation level: %d \n", basinModel -> Optimisation_Level );
  }

  do {
    FastcauldronSimulator::getInstance ().restartActivity ();
    m_surfaceNodeHistory.clearProperties ();

     if ( basinModel->isModellingMode3D() ) {
        database::Table* table = FastcauldronSimulator::getInstance ().getTable ("3DTimeIoTbl");

        if ( table != nullptr ) {
           table->clear ();
        }

     }

    basinModel->deleteMinorSnapshotsFromTimeIOTable ( savedMinorSnapshotTimes, genexOutputProperties );
    basinModel->deleteMinorSnapshotsFromTimeIOTable ( savedMinorSnapshotTimes, shaleGasOutputProperties );
    basinModel->deleteMinorSnapshotsFromTimeIOTable ( savedMinorSnapshotTimes, mapOutputProperties );

    FastcauldronSimulator::getInstance ().deleteSnapshotProperties ();
    FastcauldronSimulator::getInstance ().deleteMinorSnapshots ();
    FastcauldronSimulator::getInstance ().deleteMinorSnapshotsFromSnapshotTable ();

    savedMinorSnapshotTimes.clear ();

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
                    numberOfGeometricIterations,
                    maximumNumberOfGeometricIterations );
    }

    // Compute the overpressure from basin-start-age to present day.
    Evolve_Pressure_Basin ( numberOfGeometricIterations,
                            overpressureHasDiverged,
                            errorInDarcy );

    if ( not ( overpressureHasDiverged or errorInDarcy )) {
      // Check that the predicted geometry has converged to with some tolerance of the real (input) geometry
      pressureSolver->adjustSolidThickness ( pressureSolver->getRelativeThicknessTolerance ( basinModel -> Optimisation_Level ),
                                             pressureSolver->getAbsoluteThicknessTolerance ( basinModel -> Optimisation_Level ),
                                             geometryHasConverged );

      numberOfGeometricIterations = numberOfGeometricIterations + 1;
      MPI_Barrier(PETSC_COMM_WORLD);
    }

  } while (( numberOfGeometricIterations <= maximumNumberOfGeometricIterations ) && ! geometryHasConverged && ! overpressureHasDiverged );

  const Interface::Snapshot* snapshot = FastcauldronSimulator::getInstance ().findOrCreateSnapshot ( 0.0 );
  assert ( snapshot != 0 );

  // e.g. thickness-error, fct-correction.
  FastcauldronSimulator::getInstance ().saveMapProperties ( m_concludingMapOutputProperties, snapshot, Interface::SEDIMENTS_ONLY_OUTPUT );

  solverHasConverged = ! overpressureHasDiverged;

  if ( overpressureHasDiverged ) {
    PetscPrintf ( PETSC_COMM_WORLD,
                  "MeSsAgE ERROR Calculation has diverged, see help for possible solutions. \n" );
  } else {
    displayTime(basinModel->debug1 or basinModel->verbose,"OverPressure Calculation: ");

    m_surfaceNodeHistory.Output_Properties ();

    if ( ! basinModel->projectSnapshots.projectPrescribesMinorSnapshots ()) {
      basinModel->projectSnapshots.setActualMinorSnapshots ( savedMinorSnapshotTimes );
    }

    if ( numberOfGeometricIterations > maximumNumberOfGeometricIterations && ! geometryHasConverged ) {
       PetscPrintf ( PETSC_COMM_WORLD,
                     "MeSsAgE WARNING Maximum number of geometric iterations, %d, exceeded and geometry has not converged \n",
                     maximumNumberOfGeometricIterations );

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
  basinModel->deleteMinorSnapshotsFromTimeIOTable ( savedMinorSnapshotTimes, shaleGasOutputProperties );
  savedMinorSnapshotTimes.clear ();


  if ( FastcauldronSimulator::getInstance ().getCalculationMode () == HYDROSTATIC_TEMPERATURE_MODE )
  {
    basinModel->initialiseTimeIOTable ( HydrostaticTemperatureRunStatusStr );
    basinModel->deleteMinorSnapshotsFromTimeIOTable ( savedMinorSnapshotTimes, genexOutputProperties );
    basinModel->deleteMinorSnapshotsFromTimeIOTable ( savedMinorSnapshotTimes, shaleGasOutputProperties );

    //make sure that surfaceOutputPropterties is a superset of properties output in Output.C:savePropsOnSegmentNodes1D()
    basinModel->deleteMinorSnapshotsFromTimeIOTable ( savedMinorSnapshotTimes, mapOutputProperties );

    FastcauldronSimulator::getInstance ().deleteSnapshotProperties ();

    // Delete the minor snapshots from the snapshot-table.
    FastcauldronSimulator::getInstance ().deleteMinorSnapshots ();
    FastcauldronSimulator::getInstance ().deleteMinorSnapshotsFromSnapshotTable ();

    // delete the propertyValues from the previous iteration
    FastcauldronSimulator::getInstance ().deletePropertyValues();
  }
  else if ( FastcauldronSimulator::getInstance ().getCalculationMode () == OVERPRESSURED_TEMPERATURE_MODE )
  {
    basinModel->initialiseTimeIOTable ( OverpressuredTemperatureRunStatusStr );
  }

  if ( FastcauldronSimulator::getInstance ().getModellingMode () == Interface::MODE1D ) {
     FastcauldronSimulator::getInstance ().clear1DTimeIoTbl ();
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

  int    maximumNumberOfGeometricIterations;
  int    numberOfGeometricIterations = 1;
  bool   overpressureHasDiverged;

  geometryHasConverged = true;
  basinModel->initialiseTimeIOTable ( CoupledPressureTemperatureRunStatusStr );

  maximumNumberOfGeometricIterations = basinModel->MaxNumberOfRunOverpressure;

  if ( basinModel->debug1 or basinModel->verbose ) {
    PetscPrintf ( PETSC_COMM_WORLD, "o Maximum number of Geometric iterations: %d \n", maximumNumberOfGeometricIterations );
    PetscPrintf ( PETSC_COMM_WORLD, " Optimisation level: %d \n", basinModel -> Optimisation_Level );
  }

  // Start of geometric loop.
  do {

    FastcauldronSimulator::getInstance ().restartActivity ();
    m_surfaceNodeHistory.clearProperties ();

    // The deleting of the minor snapshot files and deleting the times from the timeio table is required
    // because we are at the start of a possible new overpressure run, with probably slightly different
    // time-steps. So the files will have different names!

    if ( basinModel->isModellingMode3D() )
    {
       database::Table* table = FastcauldronSimulator::getInstance ().getTable ("3DTimeIoTbl");

       if ( table != nullptr ) {
          table->clear ();
       }

       basinModel->deleteMinorSnapshotsFromTimeIOTable ( savedMinorSnapshotTimes, genexOutputProperties );
       basinModel->deleteMinorSnapshotsFromTimeIOTable ( savedMinorSnapshotTimes, shaleGasOutputProperties );

       //make sure that surfaceOutputPropterties is a superset of properties output in Output.C:savePropsOnSegmentNodes1D()
       basinModel->deleteMinorSnapshotsFromTimeIOTable ( savedMinorSnapshotTimes, mapOutputProperties );

       FastcauldronSimulator::getInstance ().deleteSnapshotProperties ();
       FastcauldronSimulator::getInstance ().deleteMinorSnapshots ();
       FastcauldronSimulator::getInstance ().deleteMinorSnapshotsFromSnapshotTable ();

       // delete the propertyValues from the previous iteration
       FastcauldronSimulator::getInstance ().deletePropertyValues();
   }
    else
    {
       database::Table* table = FastcauldronSimulator::getInstance ().getTable ("TimeIoTbl");

       if ( table != nullptr ) {
          table->clear ();
       }

       FastcauldronSimulator::getInstance ().deleteMinorSnapshotsFromSnapshotTable ();

       Temperature_Calculator.resetBiomarkerStateVectors ( );
       Temperature_Calculator.resetSmectiteIlliteStateVectors ( );
       Temperature_Calculator.resetFissionTrackCalculator();
       basinModel->deleteIsoValues();
       FastcauldronSimulator::getInstance ().clear1DTimeIoTbl ();
    }

    savedMinorSnapshotTimes.clear ();

    if ( basinModel->debug1 or basinModel->verbose ) {
      PetscPrintf ( PETSC_COMM_WORLD,
                    "o Starting iteration %d of %d (Maximum number of iterations)",
                    numberOfGeometricIterations,
                    maximumNumberOfGeometricIterations );
    }

    // Compute the coupled pressure-temperature from basin-start-age to present day.
    Evolve_Coupled_Basin ( numberOfGeometricIterations, overpressureHasDiverged, errorInDarcy );

    if ( not ( overpressureHasDiverged or errorInDarcy )) {

      // Check that the predicted geometry has converged to with some tolerance of the real (input) geometry
      pressureSolver->adjustSolidThickness ( pressureSolver->getRelativeThicknessTolerance ( basinModel -> Optimisation_Level ),
                                             pressureSolver->getAbsoluteThicknessTolerance ( basinModel -> Optimisation_Level ),
                                             geometryHasConverged );

      numberOfGeometricIterations = numberOfGeometricIterations + 1;
      MPI_Barrier(PETSC_COMM_WORLD);
    }

  } while (( numberOfGeometricIterations <= maximumNumberOfGeometricIterations ) and ( not geometryHasConverged ) and not overpressureHasDiverged and not errorInDarcy );

  const Interface::Snapshot* snapshot = FastcauldronSimulator::getInstance ().findOrCreateSnapshot ( 0.0 );
  assert ( snapshot != 0 );

  // e.g. thickness-error, fct-correction.
  FastcauldronSimulator::getInstance ().saveMapProperties ( m_concludingMapOutputProperties, snapshot, Interface::SEDIMENTS_ONLY_OUTPUT );

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

    if ( numberOfGeometricIterations > maximumNumberOfGeometricIterations && ! geometryHasConverged ) {
       PetscPrintf ( PETSC_COMM_WORLD,
                     "MeSsAgE WARNING Maximum number of geometric iterations, %d, exceeded and geometry has not converged \n",
                     maximumNumberOfGeometricIterations );

       PetscPrintf ( PETSC_COMM_WORLD,
                     "MeSsAgE WARNING Look at the ThicknessError maps in Cauldron to see if the error is acceptable\n" );
    }

  }

}


//------------------------------------------------------------//

#undef  __FUNCT__
#define __FUNCT__ "Basin_Modelling::FEM_Grid::Evolve_Pressure_Basin"

void Basin_Modelling::FEM_Grid::Evolve_Pressure_Basin ( const int   numberOfGeometricIterations,
                                                              bool& overpressureHasDiverged,
                                                              bool& errorInDarcy ) {

  const int MaximumNumberOfNonlinearIterations = pressureSolver->getMaximumNumberOfNonlinearIterations ( basinModel->Optimisation_Level );

  int numberOfTimesteps = 0;
  int maximumNumberOfOverpressureIterations;
  int numberOfNewtonIterations = -1;
  double currentTime, previousTime, timeStep;

  double Po_Norm;

  bool majorSnapshotTimesUpdated;
  bool fracturingOccurred;
  WallTime::Time startTime;

  System_Assembly_Time      = 0.0;
  Element_Assembly_Time     = 0.0;
  System_Solve_Time         = 0.0;
  Property_Calculation_Time = 0.0;
  Property_Saving_Time      = 0.0;

  majorSnapshots = basinModel->projectSnapshots.majorSnapshotsBegin();
  minorSnapshots = basinModel->projectSnapshots.minorSnapshotsBegin ();

  clearLayerVectors ();

  //Position Time Iterator to Start of Calculation (First Snapshot)
  if ( basinModel->projectSnapshots.majorSnapshotsEnd () != majorSnapshots ) {
     currentTime = (*majorSnapshots)->time ();
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
                                                                                numberOfGeometricIterations );

  timeStep = basinModel->getInitialTimeStep ( currentTime );
  FastcauldronSimulator::getInstance ().getAllochthonousLithologyManager ().reset ();
  overpressureHasDiverged = false;

  m_chemicalCompactionGrid->emptyGrid();
  while ( Step_Forward ( previousTime, currentTime, timeStep, majorSnapshotTimesUpdated ) and not overpressureHasDiverged and not errorInDarcy ) {

    if ( basinModel -> debug1 or basinModel->verbose ) {
      PetscPrintf ( PETSC_COMM_WORLD, "***************************************************************\n" );
    }

    startTime = WallTime::clock ();

    Construct_FEM_Grid ( previousTime, currentTime, majorSnapshots, false );
    FastcauldronSimulator::getInstance ().getMcfHandler ().setSubdomainActivity ( currentTime );

    Display_Pressure_Solver_Progress ( numberOfGeometricIterations,
                                       maximumNumberOfOverpressureIterations,
                                       currentTime,
                                       previousTime - currentTime,
                                       true or FastcauldronSimulator::getInstance ().getMcfHandler ().numberOfActiveSubdomains () != 1 );

    Temperature_Calculator.Estimate_Temperature ( basinModel, currentTime );

    m_pressureComputationalDomain.resetAge ( currentTime );

    Solve_Pressure_For_Time_Step ( previousTime,
                                   currentTime,
                                   MaximumNumberOfNonlinearIterations,
                                   overpressureHasDiverged,
                                   numberOfNewtonIterations,
                                   Po_Norm,
                                   fracturingOccurred );

    if ( ! overpressureHasDiverged ) {
       Store_Computed_Deposition_Thickness ( currentTime );

       //Do chemical compaction computation on time step
       integrateChemicalCompaction ( previousTime, currentTime );

       integrateGenex ( previousTime, currentTime );

       FastcauldronSimulator::getInstance ().getMcfHandler ().solve ( previousTime, currentTime, errorInDarcy );

       printRelatedProjects ( currentTime );
       Determine_Next_Pressure_Time_Step ( currentTime, timeStep, numberOfNewtonIterations );

       computeBasementLithostaticPressureForCurrentTimeStep ( basinModel, currentTime );

       Copy_Current_Properties ();
       Save_Properties ( currentTime );

       postTimeStepOperations ( currentTime );
       numberOfTimesteps = numberOfTimesteps + 1;
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
                  numberOfTimesteps);
  }


  Accumulated_System_Assembly_Time      = Accumulated_System_Assembly_Time      + System_Assembly_Time;
  Accumulated_Element_Assembly_Time     = Accumulated_Element_Assembly_Time     + Element_Assembly_Time;
  Accumulated_System_Solve_Time         = Accumulated_System_Solve_Time         + System_Solve_Time;
  Accumulated_Property_Calculation_Time = Accumulated_Property_Calculation_Time + Property_Calculation_Time;
  Accumulated_Property_Saving_Time      = Accumulated_Property_Saving_Time      + Property_Saving_Time;
}


//------------------------------------------------------------//

#undef  __FUNCT__
#define __FUNCT__ "Basin_Modelling::FEM_Grid::Evolve_Temperature_Basin"

void Basin_Modelling::FEM_Grid::Evolve_Temperature_Basin ( bool& temperatureHasDiverged,
                                                           bool& errorInDarcy ) {

  const int MaximumNumberOfNonlinearIterations = Temperature_Calculator.maximumNumberOfNonlinearIterations ( basinModel->Optimisation_Level );

  SnapshotInterval interval;

  int numberOfTimesteps = 0;
  int numberOfNewtonIterations = -1;
  double currentTime;
  double previousTime;
  double timeStep;

  bool   majorSnapshotTimesUpdated;
  double T_Norm;
  WallTime::Time startTime;

  System_Assembly_Time      = 0.0;
  Element_Assembly_Time     = 0.0;
  System_Solve_Time         = 0.0;
  Property_Calculation_Time = 0.0;
  Property_Saving_Time      = 0.0;

  majorSnapshots = basinModel->projectSnapshots.majorSnapshotsBegin ();
  minorSnapshots = basinModel->projectSnapshots.minorSnapshotsBegin ();

  clearLayerVectors ();


  /* Position Snapshots Iterator to Origin of Basin */
  if ( basinModel -> projectSnapshots.majorSnapshotsEnd () != majorSnapshots )
  {
    currentTime = (*majorSnapshots)->time ();
  }
  else
  {
    PetscPrintf ( PETSC_COMM_WORLD, " No SnapShotTimes Present \n" );
    return;
  }

  timeStep = basinModel->getInitialTimeStep ( currentTime );

  Temperature_Newton_Solver_Tolerance = 1.0e-5;
  temperatureHasDiverged = false;
  errorInDarcy = false;

  Display_Temperature_Solver_Progress ( currentTime, 0.0, true );
  FastcauldronSimulator::getInstance ().getAllochthonousLithologyManager ().reset ();
  Initialise_Basin_Temperature ( temperatureHasDiverged );

  m_chemicalCompactionGrid->emptyGrid();
  m_vreAlgorithm->reset();

  Save_Properties ( currentTime );
  numberOfTimesteps++;

  cout.precision ( 8 );
  cout.flags ( ios::scientific );

  while ( Step_Forward ( previousTime, currentTime, timeStep, majorSnapshotTimesUpdated ) and not temperatureHasDiverged and not errorInDarcy ) {

    if ( basinModel -> debug1 or basinModel->verbose ) {
      PetscPrintf ( PETSC_COMM_WORLD, "***************************************************************\n" );
    }

    startTime = WallTime::clock ();

    Construct_FEM_Grid ( previousTime, currentTime, majorSnapshots, majorSnapshotTimesUpdated );
    FastcauldronSimulator::getInstance ().getMcfHandler ().setSubdomainActivity ( currentTime );

    Display_Temperature_Solver_Progress ( currentTime, previousTime - currentTime,
                                          true or FastcauldronSimulator::getInstance ().getMcfHandler ().numberOfActiveSubdomains () != 1 );


    WallTime::Time resetStartTime = WallTime::clock ();
    m_temperatureComputationalDomain.resetAge ( currentTime );
    m_temperatureDomainResetTime += ( WallTime::clock () - resetStartTime ).floatValue ();

    Solve_Temperature_For_Time_Step ( previousTime,
                                      currentTime,
                                      MaximumNumberOfNonlinearIterations,
                                      temperatureHasDiverged,
                                      numberOfNewtonIterations,
                                      T_Norm );

    if ( ! temperatureHasDiverged ) {

       //Do chemical computation on time step
       integrateChemicalCompaction ( previousTime, currentTime );
       integrateGenex ( previousTime, currentTime );

       // Do a time step in the Vre algorithm
       m_vreAlgorithm->doTimestep (
          TemperatureForVreInputGrid(
             basinModel->mapDA,
             basinModel->layers,
             basinModel->getValidNeedles(),
             previousTime,
             true // use previous temperature
             ),
          TemperatureForVreInputGrid(
             basinModel->mapDA,
             basinModel->layers,
             basinModel->getValidNeedles(),
             currentTime,
             false // use current temperature
             )
          );
       //

       FastcauldronSimulator::getInstance ().getMcfHandler ().solve ( previousTime, currentTime, errorInDarcy );

       if(  basinModel->isModellingMode1D() )
       {
          Temperature_Calculator.computeSmectiteIlliteIncrement ( previousTime, currentTime );
          Temperature_Calculator.computeBiomarkersIncrement ( previousTime, currentTime );
          Temperature_Calculator.collectFissionTrackSampleData( currentTime );
       }

       printRelatedProjects ( currentTime );

       numberOfTimesteps++;

       Determine_Next_Temperature_Time_Step ( currentTime, timeStep );

       computeBasementLithostaticPressureForCurrentTimeStep ( basinModel, currentTime );

       Copy_Current_Properties ();

       Save_Properties ( currentTime );
       postTimeStepOperations ( currentTime );
    }

    if (basinModel->debug1 or basinModel->verbose or FastcauldronSimulator::getInstance ().getMcfHandler ().getDebugLevel () > 0 ) {
       PetscPrintf(PETSC_COMM_WORLD, " time for time-step: %f\n", (WallTime::clock () - startTime).floatValue() );
    }

    if ( basinModel->exitAtAgeDefined () and basinModel->getExitAtAge () >= currentTime ) {
       break;
    }

  }

  if ( basinModel -> debug1 or basinModel->verbose ) {
    PetscPrintf ( PETSC_COMM_WORLD, " total System_Assembly_Time      %f \n", System_Assembly_Time );
    PetscPrintf ( PETSC_COMM_WORLD, " total Element_Assembly_Time     %f \n", Element_Assembly_Time );
    PetscPrintf ( PETSC_COMM_WORLD, " total System_Solve_Time         %f \n", System_Solve_Time );
    PetscPrintf ( PETSC_COMM_WORLD, " total Property_Calculation_Time %f \n", Property_Calculation_Time );
    PetscPrintf ( PETSC_COMM_WORLD, "\n Temperature Calculation Performed in %d Time Steps\n\n",
                  numberOfTimesteps);
  }


  Accumulated_System_Assembly_Time      = Accumulated_System_Assembly_Time      + System_Assembly_Time;
  Accumulated_Element_Assembly_Time     = Accumulated_Element_Assembly_Time     + Element_Assembly_Time;
  Accumulated_System_Solve_Time         = Accumulated_System_Solve_Time         + System_Solve_Time;
  Accumulated_Property_Calculation_Time = Accumulated_Property_Calculation_Time + Property_Calculation_Time;
  Accumulated_Property_Saving_Time      = Accumulated_Property_Saving_Time      + Property_Saving_Time;

}


//------------------------------------------------------------//

#undef  __FUNCT__
#define __FUNCT__ "Basin_Modelling::FEM_Grid::Evolve_Coupled_Basin"

void Basin_Modelling::FEM_Grid::Evolve_Coupled_Basin ( const int   numberOfGeometricIterations,
                                                             bool& hasDiverged,
                                                             bool& errorInDarcy ) {

  const int maximumNumberOfNonlinearPressureIterations    = pressureSolver->getMaximumNumberOfNonlinearIterations ( basinModel->Optimisation_Level );
  const int maximumNumberOfNonlinearTemperatureIterations = Temperature_Calculator.maximumNumberOfNonlinearIterations ( basinModel->Optimisation_Level );

  int maximumNumberOfOverpressureIterations;
  int numberOfNewtonIterations = -1;
  double currentTime, previousTime, timeStep;
  int numberOfTimesteps = 0;

  double Po_Norm;
  double T_Norm;

  bool majorSnapshotTimesUpdated;
  bool fracturingOccurred = false;
  WallTime::Time startTime;

  System_Assembly_Time      = 0.0;
  Element_Assembly_Time     = 0.0;
  System_Solve_Time         = 0.0;
  Property_Calculation_Time = 0.0;
  Property_Saving_Time      = 0.0;

  majorSnapshots = basinModel->projectSnapshots.majorSnapshotsBegin ();
  minorSnapshots = basinModel->projectSnapshots.minorSnapshotsBegin ();

  clearLayerVectors ();

  /* Position Snapshots Iterator to Origin of Basin */
  if ( basinModel -> projectSnapshots.majorSnapshotsEnd () != majorSnapshots ) {
    currentTime = (*majorSnapshots)->time ();
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
                                                                                numberOfGeometricIterations );

  timeStep = basinModel->getInitialTimeStep ( currentTime );
  timeStep = NumericFunctions::Minimum ( timeStep, basinModel->maximumTimeStep ());
  Display_Coupled_Solver_Progress ( numberOfGeometricIterations,
                                    maximumNumberOfOverpressureIterations,
                                    currentTime, 0.0,
                                    true );

  FastcauldronSimulator::getInstance ().getAllochthonousLithologyManager ().reset ();

  Initialise_Basin_Temperature ( hasDiverged );

  m_chemicalCompactionGrid->emptyGrid();
  m_vreAlgorithm->reset();

  Save_Properties ( currentTime );

  if ( basinModel -> debug1 or basinModel->verbose ) {
    PetscPrintf ( PETSC_COMM_WORLD, "Solving Coupled for Time (Ma): %f \n", currentTime );
  }


  // Now only need to do a single newton iteration (keep constant and Newton iterations for future use)
  while ( Step_Forward ( previousTime, currentTime, timeStep, majorSnapshotTimesUpdated ) and not hasDiverged and not errorInDarcy ) {

    if ( basinModel -> debug1 or basinModel->verbose ) {
      PetscPrintf ( PETSC_COMM_WORLD, "***************************************************************\n" );
    }

    startTime = WallTime::clock ();

    Construct_FEM_Grid ( previousTime, currentTime, majorSnapshots, false );
    FastcauldronSimulator::getInstance ().getMcfHandler ().setSubdomainActivity ( currentTime );

    Display_Coupled_Solver_Progress ( numberOfGeometricIterations,
                                      maximumNumberOfOverpressureIterations,
                                      currentTime, previousTime - currentTime,
                                      true or FastcauldronSimulator::getInstance ().getMcfHandler ().numberOfActiveSubdomains () != 1 );

    m_pressureComputationalDomain.resetAge ( currentTime );
    m_temperatureComputationalDomain.resetAge ( currentTime );

    Solve_Coupled_For_Time_Step ( previousTime, currentTime,
                                  maximumNumberOfNonlinearPressureIterations,
                                  maximumNumberOfNonlinearTemperatureIterations,
                                  hasDiverged,
                                  numberOfNewtonIterations,
                                  Po_Norm,
                                  T_Norm,
                                  fracturingOccurred );

    if ( ! hasDiverged ) {
       printRelatedProjects ( currentTime );
       Store_Computed_Deposition_Thickness ( currentTime );

       // Do chemical compaction computation on time step
       integrateChemicalCompaction ( previousTime, currentTime );
       integrateGenex ( previousTime, currentTime );

      // Do a time step in the Vre algorithm
       m_vreAlgorithm->doTimestep (
          TemperatureForVreInputGrid(
             basinModel->mapDA,
             basinModel->layers,
             basinModel->getValidNeedles(),
             previousTime,
             true // use previous temperature
             ),
          TemperatureForVreInputGrid(
             basinModel->mapDA,
             basinModel->layers,
             basinModel->getValidNeedles(),
             currentTime,
             false // use current temperature
             )
          );
       //

       FastcauldronSimulator::getInstance ().getMcfHandler ().solve ( previousTime, currentTime, errorInDarcy );

       if ( basinModel->isModellingMode1D ()) {
          Temperature_Calculator.computeSmectiteIlliteIncrement ( previousTime, currentTime );
          Temperature_Calculator.computeBiomarkersIncrement ( previousTime, currentTime );
          Temperature_Calculator.collectFissionTrackSampleData( currentTime );
       }

       Determine_Next_Coupled_Time_Step ( currentTime, timeStep );

       computeBasementLithostaticPressureForCurrentTimeStep ( basinModel, currentTime );

       Copy_Current_Properties ();

       Save_Properties ( currentTime );
       postTimeStepOperations ( currentTime );
       numberOfTimesteps = numberOfTimesteps + 1;
    }

    if (basinModel->debug1 or basinModel->verbose or FastcauldronSimulator::getInstance ().getMcfHandler ().getDebugLevel () > 0 ) {
       PetscPrintf(PETSC_COMM_WORLD, " time for time-step: %f\n", (WallTime::clock () - startTime).floatValue() );
    }

  }


  if (( basinModel->isGeometricLoop ()) && ( basinModel -> debug1 or basinModel->verbose )) {
    PetscPrintf ( PETSC_COMM_WORLD, " total System_Assembly_Time      %f \n", System_Assembly_Time );
    PetscPrintf ( PETSC_COMM_WORLD, " total Element_Assembly_Time     %f \n", Element_Assembly_Time );
    PetscPrintf ( PETSC_COMM_WORLD, " total System_Solve_Time         %f \n", System_Solve_Time );
    PetscPrintf ( PETSC_COMM_WORLD, " total Property_Calculation_Time %f \n", Property_Calculation_Time );
    PetscPrintf ( PETSC_COMM_WORLD, "\n Coupled Calculation Performed in %d Time Steps\n\n",
                  numberOfTimesteps);
  }

  Accumulated_System_Assembly_Time      = Accumulated_System_Assembly_Time      + System_Assembly_Time;
  Accumulated_Element_Assembly_Time     = Accumulated_Element_Assembly_Time     + Element_Assembly_Time;
  Accumulated_System_Solve_Time         = Accumulated_System_Solve_Time         + System_Solve_Time;
  Accumulated_Property_Calculation_Time = Accumulated_Property_Calculation_Time + Property_Calculation_Time;
  Accumulated_Property_Saving_Time      = Accumulated_Property_Saving_Time      + Property_Saving_Time;
}

//------------------------------------------------------------//

#undef  __FUNCT__
#define __FUNCT__ "Basin_Modelling::FEM_Grid::Save_Properties"

void Basin_Modelling::FEM_Grid::Save_Properties ( const double currentTime ) {

   if ( currentTime == 0.0 ) {
      Well wells( basinModel );
      wells.Save_Present_Day_Data ();
   }

   if ( ! basinModel->DoHDFOutput ) {
      return;
   }

   if ( basinModel->m_doOutputAtAge && currentTime!=basinModel->m_ageToOutput) {
      return;
   }

  PetscLogDouble Start_Time;
  PetscLogDouble End_Time;

  PetscTime(&Start_Time);
  if (   ( currentTime == (*majorSnapshots)->time ()  )
      || (    basinModel->isModellingMode1D()
           && basinModel->projectSnapshots.isMinorSnapshot ( currentTime, minorSnapshots ) ) ) // 1D model: save minor AND major timesteps
  {

     const Interface::Snapshot* snapshot = FastcauldronSimulator::getInstance ().findOrCreateSnapshot ( currentTime );
     assert ( snapshot != 0 );

     // Compute the derived properties that are to be output.
     m_vreAlgorithm->getResults( m_vreOutputGrid );
     m_vreOutputGrid.exportToModel( basinModel->layers, basinModel->getValidNeedles() );

     if(  basinModel->isModellingMode1D() )
     {
        Temperature_Calculator.computeSnapShotSmectiteIllite ( currentTime, basinModel->getValidNeedles ());
        Temperature_Calculator.computeSnapShotBiomarkers ( currentTime, basinModel->getValidNeedles () );
     }

     if ( basinModel->isALC() ) {
        basinModel->calcBasementProperties( currentTime );
     }

     // save both map and volume data.
     if ( currentTime == 0.0 && m_concludingVolumeOutputProperties.size() > 0)
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
        collectAndSaveIsoValues(currentTime, basinModel);
     }

     // Collect surface node properties.
     m_surfaceNodeHistory.Add_Time ( currentTime );

     // Delete the vectors for derived properties as they are no longer required.
     if(  basinModel->isModellingMode1D() )
     {
        Temperature_Calculator.deleteSmectiteIlliteVector ();
        Temperature_Calculator.deleteBiomarkersVectors ();
     }

     if (      basinModel->isModellingMode1D()
          &&  currentTime != (*majorSnapshots)->time () ) // 1D model: save minor snapshot
     {
        savedMinorSnapshotTimes.insert ( currentTime );
     }

  } else {

     if ( basinModel->projectSnapshots.isMinorSnapshot ( currentTime, minorSnapshots )) {
        const Interface::Snapshot* snapshot = FastcauldronSimulator::getInstance ().findOrCreateSnapshot ( currentTime );
        assert ( snapshot != 0 );

        m_vreAlgorithm->getResults( m_vreOutputGrid );
        m_vreOutputGrid.exportToModel( basinModel->layers, basinModel->getValidNeedles() );

        if ( ! basinModel->projectSnapshots.projectPrescribesMinorSnapshots ()) {
           FastcauldronSimulator::getInstance ().saveVolumeProperties ( looselyCoupledOutputProperties,
                                                                        snapshot,
                                                                        Interface::SEDIMENTS_AND_BASEMENT_OUTPUT );
           savedMinorSnapshotTimes.insert ( currentTime );
        }

        computeErosionFactorMaps ( basinModel, currentTime );

        FastcauldronSimulator::getInstance ().saveSourceRockProperties ( snapshot, genexOutputProperties, shaleGasOutputProperties );

        deleteErosionFactorMaps ( basinModel );
     }

     if ( m_surfaceNodeHistory.IsDefined ()) {
        m_vreAlgorithm->getResults( m_vreOutputGrid );
        m_vreOutputGrid.exportToModel( basinModel->layers, basinModel->getValidNeedles() );
        computePermeabilityVectors ( basinModel );
        computeThermalConductivityVectors ( basinModel );
        computeBulkDensityVectors ( basinModel );

        // Collect surface node properties.
        m_surfaceNodeHistory.Add_Time ( currentTime );

        deleteThermalConductivityVectors ( basinModel );
        deleteBulkDensityVectors ( basinModel );
     }

  }

  PetscTime(&End_Time);
  Property_Saving_Time = Property_Saving_Time + ( End_Time - Start_Time );

}


//------------------------------------------------------------//


bool Basin_Modelling::FEM_Grid::Step_Forward ( double& previousTime,
                                               double& currentTime,
                                               double& timeStep,
                                               bool&   majorSnapshotTimesUpdated ) {

   majorSnapshotTimesUpdated = false;

  if ( currentTime == PresentDay ) {
    // We are done!
    return false;
  }

  if ( currentTime == (*majorSnapshots)->time ()) {
    // Step to the next snapshot time.
    ++majorSnapshots;
    majorSnapshotTimesUpdated = true;
  } else if ( basinModel->projectSnapshots.validMinorSnapshots ( minorSnapshots ) &&
              currentTime <= (*minorSnapshots)->time ()) {
    basinModel->projectSnapshots.advanceMinorSnapshotIterator ( currentTime, minorSnapshots );
  }


  previousTime = currentTime;
  currentTime = currentTime - timeStep;

  // Do not want very small time step that may occur as we approach a snapshot time.
  // If the current time is just short of the snapshot, then set it to the snapshot time.
  if ( fabs(currentTime - (*majorSnapshots)->time ()) < 0.0001 ) {
    currentTime = (*majorSnapshots)->time ();
  } else if ( basinModel->projectSnapshots.projectPrescribesMinorSnapshots () &&
              basinModel->projectSnapshots.isAlmostSnapshot ( currentTime, timeStep, minorSnapshots, 0.1 )) {
    currentTime = (*minorSnapshots)->time ();
  }

  if ( basinModel->projectSnapshots.projectPrescribesMinorSnapshots () &&
       basinModel->projectSnapshots.validMinorSnapshots ( minorSnapshots ) &&
       currentTime < (*minorSnapshots)->time () &&
       (*minorSnapshots)->time () > (*majorSnapshots)->time ()) {
    currentTime = (*minorSnapshots)->time ();
  } else if ( currentTime < (*majorSnapshots)->time ()) {
    currentTime = (*majorSnapshots)->time ();
  }

  basinModel->adjustTimeStepToPermafrost ( previousTime, currentTime );

  timeStep = previousTime - currentTime;

  return true;

}


//------------------------------------------------------------//

#undef  __FUNCT__
#define __FUNCT__ "Basin_Modelling::FEM_Grid::Determine_Next_Pressure_Time_Step"

void Basin_Modelling::FEM_Grid::Determine_Next_Pressure_Time_Step ( const double  currentTime,
                                                                          double& timeStep,
                                                                    const int     numberOfNewtonIterations ) {

   //Store timeStep for igneous intrusion computation
   const double previousTimeStep = timeStep;

   if ( currentTime == (*majorSnapshots)->time ()) {
    timeStep = basinModel->getInitialTimeStep ( currentTime );
  } else {
     const double Increase_Factor = basinModel -> timestepincr;
     const double Decrease_Factor = basinModel -> timestepdecr;

     double Optimal_Pressure_Difference = basinModel->optimalpressdiff;
     double Predicted_Time_Step;

     PetscScalar Maximum_Difference = pressureSolver->maximumPressureDifference ();

     MPI_Barrier(PETSC_COMM_WORLD);

     if ( basinModel -> Optimisation_Level >= 6 && numberOfNewtonIterations >= pressureSolver->getMaximumNumberOfNonlinearIterations ( basinModel->Optimisation_Level )
          && Maximum_Difference > 0.25 * Optimal_Pressure_Difference ) {
        //    if ( basinModel -> Optimisation_Level >= 3 && numberOfNewtonIterations >= Maximum_Number_Of_Nonlinear_Pressure_Iterations ()) {

        // Only allow the time step to decrease by at most a factor of a half.
        Predicted_Time_Step = timeStep * NumericFunctions::Maximum ( 0.5, Decrease_Factor );
     } else if ( Maximum_Difference <= 0.0 ) {
        Predicted_Time_Step = timeStep * Increase_Factor;
     } else if ( Maximum_Difference < Optimal_Pressure_Difference ) {
        Predicted_Time_Step = timeStep * PetscMin ( Optimal_Pressure_Difference / Maximum_Difference, Increase_Factor );
     } else {
        Predicted_Time_Step = timeStep * PetscMax ( Optimal_Pressure_Difference / Maximum_Difference, Decrease_Factor );
     }

     timeStep = NumericFunctions::Maximum ( Predicted_Time_Step, basinModel->minimumTimeStep ());
     timeStep = NumericFunctions::Minimum ( timeStep, basinModel->maximumTimeStep ());
  }

  Determine_Permafrost_Time_Step ( currentTime, timeStep );
  //If during an igneous intrusion, change the time step
  determineIgneousIntrusionTimeStep( currentTime, previousTimeStep, timeStep);

}


//------------------------------------------------------------//

#undef  __FUNCT__
#define __FUNCT__ "Basin_Modelling::FEM_Grid::Determine_Next_Temperature_Time_Step"

void Basin_Modelling::FEM_Grid::Determine_Next_Temperature_Time_Step ( const double  currentTime,
                                                                             double& timeStep )
{


   double Optimal_Temperature_Difference                 = basinModel -> optimaltempdiff;
   double Optimal_Temperature_Difference_In_Source_Rocks = basinModel -> optimalsrtempdiff;
   double Increase_Factor                                = basinModel -> timestepincr;
   double Decrease_Factor                                = basinModel -> timestepdecr;
   double Predicted_Time_Step;
   double Predicted_Time_Step_In_Source_Rocks;
   //Store Time_Step for igneous intrusion computation
   const double previousTimeStep = timeStep;

   MPI_Barrier(PETSC_COMM_WORLD);

   if ( currentTime == (*majorSnapshots)->time ()) {
      timeStep = basinModel->getInitialTimeStep ( currentTime );
   } else {
      PetscScalar Maximum_Difference;

      Maximum_Difference = Temperature_Calculator.Maximum_Temperature_Difference ();

      if ( basinModel -> debug1 or basinModel->verbose ) {
         PetscPrintf ( PETSC_COMM_WORLD, " Maximum temperature difference %f \n", Maximum_Difference );
      }

      if ( Maximum_Difference <= 0.0 )  {
         Predicted_Time_Step = timeStep * Increase_Factor;
      } else if ( Maximum_Difference < Optimal_Temperature_Difference ) {
         Predicted_Time_Step = timeStep *
            PetscMin ( Optimal_Temperature_Difference / Maximum_Difference, Increase_Factor );
      } else {
         Predicted_Time_Step = timeStep *
            PetscMax ( Optimal_Temperature_Difference / Maximum_Difference, Decrease_Factor );
      }

      Maximum_Difference = Temperature_Calculator.Maximum_Temperature_Difference_In_Source_Rocks ();

      if ( basinModel -> debug1 or basinModel->verbose ) {
         PetscPrintf ( PETSC_COMM_WORLD, " Maximum source rock difference %f \n", Maximum_Difference );
      }

      if ( Maximum_Difference <= 0.0 ) {
         Predicted_Time_Step_In_Source_Rocks = timeStep * Increase_Factor;
      } else if ( Maximum_Difference < Optimal_Temperature_Difference_In_Source_Rocks ) {
         Predicted_Time_Step_In_Source_Rocks = timeStep *
            PetscMin ( Optimal_Temperature_Difference_In_Source_Rocks / Maximum_Difference, Increase_Factor );
      } else {
         Predicted_Time_Step_In_Source_Rocks = timeStep *
            PetscMax ( Optimal_Temperature_Difference_In_Source_Rocks / Maximum_Difference, Decrease_Factor );
      }

      timeStep = NumericFunctions::Minimum ( Predicted_Time_Step, Predicted_Time_Step_In_Source_Rocks );
      timeStep = NumericFunctions::Minimum ( timeStep, basinModel->maximumTimeStep ());

      if ( basinModel -> isALC() ) {
         timeStep = NumericFunctions::Maximum ( timeStep, basinModel->minimumTimeStep ());
      }
   }

   Determine_Permafrost_Time_Step ( currentTime, timeStep );
   //If during an igneous intrusion, change the time step
   determineIgneousIntrusionTimeStep( currentTime, previousTimeStep, timeStep);
}


//------------------------------------------------------------//

#undef  __FUNCT__
#define __FUNCT__ "Basin_Modelling::FEM_Grid::Determine_Next_Coupled_Time_Step"

void Basin_Modelling::FEM_Grid::Determine_Next_Coupled_Time_Step ( const double  currentTime,
                                                                         double& timeStep ) {


   //Store timeStep for igneous intrusion computation
   const double previousTimeStep = timeStep;

   if ( currentTime == (*majorSnapshots)->time ()) {
    timeStep = basinModel->getInitialTimeStep ( currentTime );
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
        Predicted_Overpressure_Time_Step = timeStep * Increase_Factor;
     } else if ( Maximum_Overpressure_Difference < Optimal_Pressure_Difference ) {
        Predicted_Overpressure_Time_Step = timeStep * PetscMin ( Optimal_Pressure_Difference / Maximum_Overpressure_Difference,
                                                                  Increase_Factor );
     } else {
        Predicted_Overpressure_Time_Step = timeStep * PetscMax ( Optimal_Pressure_Difference / Maximum_Overpressure_Difference,
                                                                  Decrease_Factor );
     }

     // Determine the best TEMPERATURE time step
     if ( Maximum_Temperature_Difference <= 0.0 ) {
        Predicted_Temperature_Time_Step = timeStep * Increase_Factor;
     } else if ( Maximum_Temperature_Difference < Optimal_Temperature_Difference ) {
        Predicted_Temperature_Time_Step = timeStep * PetscMin ( Optimal_Temperature_Difference / Maximum_Temperature_Difference,
                                                                 Increase_Factor );
     } else {
        Predicted_Temperature_Time_Step = timeStep * PetscMax ( Optimal_Temperature_Difference / Maximum_Temperature_Difference,
                                                                 Decrease_Factor );
     }

     // Determine the best time step in SOURCE ROCKS
     if ( Maximum_Source_Rock_Difference <= 0.0 ) {
        Predicted_Source_Rock_Time_Step = timeStep * Increase_Factor;
     } else if ( Maximum_Source_Rock_Difference < Optimal_Temperature_Difference_In_Source_Rocks ) {
        Predicted_Source_Rock_Time_Step = timeStep * PetscMin ( Optimal_Temperature_Difference_In_Source_Rocks / Maximum_Source_Rock_Difference,
                                                                 Increase_Factor );
     } else {
        Predicted_Source_Rock_Time_Step = timeStep * PetscMax ( Optimal_Temperature_Difference_In_Source_Rocks / Maximum_Source_Rock_Difference,
                                                                 Decrease_Factor );
     }

     // Find the MINIMUM of these three predicted time steps.
     timeStep = NumericFunctions::Minimum3 ( Predicted_Temperature_Time_Step, Predicted_Source_Rock_Time_Step, Predicted_Overpressure_Time_Step );
     timeStep = NumericFunctions::Maximum ( timeStep, basinModel->minimumTimeStep ());
     timeStep = NumericFunctions::Minimum ( timeStep, basinModel->maximumTimeStep ());
  }

  Determine_Permafrost_Time_Step ( currentTime, timeStep );
  //If during an igneous intrusion, change the time step
  determineIgneousIntrusionTimeStep( currentTime, previousTimeStep, timeStep);
}


//------------------------------------------------------------//

#undef  __FUNCT__
#define __FUNCT__ "Basin_Modelling::FEM_Grid::Construct_Pressure_FEM_Grid"

void Basin_Modelling::FEM_Grid::Construct_Pressure_FEM_Grid ( const double previousTime,
                                                              const double currentTime ) {

   Layer_Iterator Pressure_Layers;

   // Allocate the property vectors in each of the layers
   Pressure_Layers.Initialise_Iterator ( basinModel->layers, Descending, Basement_And_Sediments, Active_Layers_Only );

   while ( ! Pressure_Layers.Iteration_Is_Done ()) {
      Pressure_Layers.Current_Layer ()->allocateNewVecs ( basinModel, currentTime );
      Pressure_Layers++;
   }

   // Now that the layer DA's have been set we can fill the Topmost Active Segment arrays
   basinModel -> Fill_Topmost_Segment_Arrays ( previousTime, currentTime );

   // This is really to initialise the properties (perhaps create a separate function to do this)
   pressureSolver->initialisePressureProperties ( previousTime, currentTime );
   pressureSolver->computeDependantProperties ( previousTime, currentTime, false );
}

//------------------------------------------------------------//

#undef  __FUNCT__
#define __FUNCT__ "Basin_Modelling::FEM_Grid::Construct_Temperature_FEM_Grid"

void Basin_Modelling::FEM_Grid::Construct_Temperature_FEM_Grid ( const double                    previousTime,
                         const double                    currentTime,
                                                                 const SnapshotEntrySetIterator& majorSnapshotTimes,
                                                                 const bool                      majorSnapshotTimesUpdated ) {

   Layer_Iterator Layers;

   // Allocate the property vectors in each of the layers
   Layers.Initialise_Iterator ( basinModel -> layers, Descending, Basement_And_Sediments, Active_Layers_Only );

   while ( ! Layers.Iteration_Is_Done () )
   {
      Layers.Current_Layer ()->allocateNewVecs ( basinModel, currentTime );
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

   Temperature_Calculator.setSurfaceTemperature ( basinModel, currentTime );
   Set_Pressure_Dependent_Properties ( currentTime );
   Compute_Temperature_Dependant_Properties ( currentTime );
   Temperature_Calculator.computeHeatProduction ( previousTime, currentTime );

}


//------------------------------------------------------------//

#undef  __FUNCT__
#define __FUNCT__ "Basin_Modelling::FEM_Grid::Construct_FEM_Grid"

void Basin_Modelling::FEM_Grid::Construct_FEM_Grid ( const double                    previousTime,
                                                     const double                    currentTime,
                                                     const SnapshotEntrySetIterator& majorSnapshotTimes,
                                                     const bool                      majorSnapshotTimesUpdated ) {

  // Set the number of active element in the layer thickness
  basinModel->findActiveElements ( currentTime );

  if ( basinModel -> DoTemperature || basinModel -> Do_Iteratively_Coupled ) {
    Construct_Temperature_FEM_Grid ( previousTime, currentTime, majorSnapshotTimes, majorSnapshotTimesUpdated );
  }

  if ( basinModel -> DoOverPressure || basinModel -> Do_Iteratively_Coupled ) {
    Construct_Pressure_FEM_Grid ( previousTime, currentTime );
  }

  FastcauldronSimulator::getInstance ().switchLithologies ( currentTime );
  setLayerElements ( currentTime );
}

//------------------------------------------------------------//

void Basin_Modelling::FEM_Grid::setLayerElements ( const double age ) {

   Layer_Iterator basinLayers ( basinModel->layers, Descending, Basement_And_Sediments, Active_And_Inactive_Layers );
   LayerProps_Ptr currentLayer;

   while ( not basinLayers.Iteration_Is_Done ()) {
      currentLayer = basinLayers.Current_Layer ();
      currentLayer->setLayerElementActivity ( age );
      basinLayers++;
   }

}

//------------------------------------------------------------//

void Basin_Modelling::FEM_Grid::integrateChemicalCompaction( const double PreviousTime, const double CurrentTime )
{
   //The chemical compaction is computed only if the global boolean is true
   if ( basinModel -> Do_Chemical_Compaction ) {
      //Get the data from the fem_grid objects
      bool runCC = m_chemicalCompactionGrid->addLayers ( basinModel->mapDA,
                                                         basinModel->layers,
                                                         basinModel->getValidNeedles(),
                                                         PreviousTime,
                                                         CurrentTime );
      if (runCC){
         m_chemicalCompactionCalculator->computeOnTimeStep(*m_chemicalCompactionGrid);
         m_chemicalCompactionGrid->exportToModel(basinModel->layers,
                                                 basinModel->getValidNeedles());
      }
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
#define __FUNCT__ "Basin_Modelling::FEM_Grid::Solve_Pressure_For_Time_Step"

void Basin_Modelling::FEM_Grid::Solve_Pressure_For_Time_Step ( const double  previousTime,
                                                               const double  currentTime,
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
    PetscPrintf ( PETSC_COMM_WORLD, " Current time  %f \n", currentTime );
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

  // Initialised in order to prevent a compiler warning.
  int linearSolverTotalIterationCount = 0;
  int linearSolveAttempts;

  PetscScalar Residual_Solution_Length;

  // Initialised to 1, to stop the compiler from complaining. This initialisation is legitimate
  // because this variable IS assigned to before use, it is used in the 3rd and subsequent iterations.
  PetscScalar Residual_Length = 0.0;
  PetscScalar Previous_Residual_Length = 0.0;
  PetscScalar Solution_Length = 0.0;
  PetscReal   linearSolverResidualNorm;

  boost::shared_ptr<PetscSolver> pressureLinearSolver ( new PetscCG ( pressureSolver->linearSolverTolerance ( basinModel->Optimisation_Level ),
                                                                      PressureSolver::DefaultMaximumPressureLinearSolverIterations ));
  pressureLinearSolver->loadCmdLineOptions();


  Jacobian = PetscObjectAllocator::allocateMatrix ( m_pressureComputationalDomain );
  Residual = PetscObjectAllocator::allocateVector ( m_pressureComputationalDomain );
  Overpressure = PetscObjectAllocator::allocateVector ( m_pressureComputationalDomain );
  Residual_Solution = PetscObjectAllocator::allocateVector ( m_pressureComputationalDomain );

  SolutionVectorMapping mapping ( m_pressureComputationalDomain, Basin_Modelling::Overpressure );

  PetscViewerSetFormat(PETSC_VIEWER_STDOUT_WORLD, PETSC_VIEWER_ASCII_MATLAB );

  mapping.getSolution ( Overpressure );

  PetscScalar Previous_Po_Norm;
  PetscScalar Previous_TS_Po_Norm;

  PetscScalar First_Po_Norm;

  // Initilaised in order to prevent a compiler warning.
  PetscScalar Theta = 0.0;
  PetscScalar Theta_Increment;


  VecNorm ( Overpressure, NORM_2, &Po_Norm );
  Previous_TS_Po_Norm = Po_Norm;

  if (basinModel -> debug1 or basinModel->verbose ) {
    PetscPrintf ( PETSC_COMM_WORLD, " Number of DOFs  =  %d \n", m_pressureComputationalDomain.getGlobalNumberOfActiveNodes ());
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

  // If the input provided m_saveTimeStep is equal to the age of the basin (case m_saveTimeStep == previousTime) or
  // the current time step is the first one greater or equal to m_saveTimeStep, then matrix and RHS will be saved
  const bool saveMatrix = m_saveMatrixToFile and
                          ( (( m_saveTimeStep - currentTime >= 0.0 ) and ( m_saveTimeStep - previousTime < 0.0 ))
                            or (m_saveTimeStep == previousTime) );
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
      pressureSolver->assembleSystem ( m_pressureComputationalDomain,
                                       previousTime, currentTime,
                                       Jacobian, Residual,
                                       Element_Contributions_Time );

      PetscTime(&Assembly_End_Time);

      Element_Assembly_Time = Element_Assembly_Time + Element_Contributions_Time;
      System_Assembly_Time = System_Assembly_Time + Assembly_End_Time - Assembly_Start_Time;

      PetscTime(&Start_Time);

      PetscTime(&Jacobian_End_Time);
      Jacobian_Time = Jacobian_End_Time - Jacobian_Start_Time;
      Total_Jacobian_Time = Total_Jacobian_Time + Jacobian_Time;

      VecSet ( Residual_Solution, Zero );

      // Print matrix adn rhs to file
      if( saveMatrix and totalNumberOfNonlinearIterations == 0 )
      {
         const std::string matrixFileName = std::string("presMatrix_t" + static_cast<ostringstream*>( &(ostringstream() << currentTime) )->str() );
         const std::string rhsFileName    = std::string(   "presRhs_t" + static_cast<ostringstream*>( &(ostringstream() << currentTime) )->str() );

         int rc = 0;
         rc = PetscObjectsIO::writeMatrixToFile( Jacobian, basinModel->getOutputDirectory(), matrixFileName, !m_saveInMatlabFormat );
         assert( rc == 0 );
         rc = PetscObjectsIO::writeVectorToFile( Residual, basinModel->getOutputDirectory(), rhsFileName, !m_saveInMatlabFormat );
         assert( rc == 0 );
      }

      // Solve the matrix equation (Jacobian^{-1} \times residual) to some acceptable tolerance.
      KSPConvergedReason convergedReason;
      pressureLinearSolver->solve ( Jacobian,
                                    Residual,
                                    Residual_Solution,
                                    &numberOfLinearIterations,
                                    &convergedReason,
                                    &linearSolverResidualNorm );

      // Print solution to file
      if( saveMatrix and totalNumberOfNonlinearIterations == 0 )
      {
         const std::string solFileName = std::string( "presSol_t" + static_cast<ostringstream*>( &(ostringstream() << currentTime) )->str() );
         int rc = PetscObjectsIO::writeVectorToFile( Residual_Solution, basinModel->getOutputDirectory(), solFileName, !m_saveInMatlabFormat );
         assert( rc == 0 );

         pressureLinearSolver->viewSettings();
      }

      // First attempt to solve linear system of equations.
      linearSolveAttempts = 1;

      if ( convergedReason < 0 ) {
         linearSolverTotalIterationCount = numberOfLinearIterations;

         // Check if HYPRE has been disabled from command line
         PetscBool disableHypre = PETSC_FALSE;
         PetscOptionsHasName( NULL, "-disableHypre", &disableHypre );

         if( (PETSC_FALSE == disableHypre) && (convergedReason == KSP_DIVERGED_ITS) && (pressureLinearSolver->getPCtype() != PCHYPRE) )
         {
            // If the linear solver has diverged due to the maximum number of iterations it then tries HYPRE
            PetscPrintf ( PETSC_COMM_WORLD,
                          " MeSsAgE WARNING The pressure solver exit condition was: %s. Retrying with HYPRE preconditioner. \n",
                          getKspConvergedReasonImage ( convergedReason ).c_str ());

            pressureLinearSolver->setPCtype( PCHYPRE );

            pressureLinearSolver->solve ( Jacobian, Residual, Residual_Solution,
                                          &numberOfLinearIterations, &convergedReason, &linearSolverResidualNorm );

            linearSolverTotalIterationCount += numberOfLinearIterations;
         }
         else
         {
            // If the linear solver has diverged for other reasons it then tries GMRES with several settings
            PetscPrintf ( PETSC_COMM_WORLD,
                          " MeSsAgE WARNING The pressure solver exit condition was: %s. Retrying with another linear solver. \n",
                          getKspConvergedReasonImage ( convergedReason ).c_str ());

            // Now iterate several times until the linear system has been solved.
            // If, however, the number of iterations exceeds the maximum then this will result in a simulation failure.
            // On the first iteration the linear solver is switched to gmres.
            // On subsequent iterations the restart level and the maximum number of iterations are both increased,
            for ( int linearSolveLoop = 1; linearSolveLoop <= PressureSolver::MaximumLinearSolveAttempts and convergedReason < 0; ++linearSolveLoop, ++linearSolveAttempts ) {

               boost::shared_ptr<PetscGMRES> gmres = boost::dynamic_pointer_cast<PetscGMRES>( pressureLinearSolver);
               if ( ! gmres  ) {
                  pressureLinearSolver.reset ( new PetscGMRES ( pressureLinearSolver->getTolerance(),
                                                                PressureSolver::DefaultGMResRestartValue,
                                                                pressureLinearSolver->getMaxIterations ()));

                  gmres = boost::dynamic_pointer_cast<PetscGMRES>( pressureLinearSolver);
                  gmres->loadCmdLineOptions();
                  gmres->setRestart ( std::max( gmres->getRestart(), PressureSolver::DefaultGMResRestartValue ));
                  gmres->setMaxIterations( std::max( pressureLinearSolver->getMaxIterations(), PressureSolver::DefaultMaximumPressureLinearSolverIterations) );

               } else {
                  gmres->setRestart( gmres->getRestart() + PressureSolver::GMResRestartIncrementValue );
                  gmres->setMaxIterations(( 3 * gmres->getMaxIterations ()) / 2 );
               }

               pressureLinearSolver->solve ( Jacobian, Residual, Residual_Solution,
                                             &numberOfLinearIterations, &convergedReason, &linearSolverResidualNorm );

               linearSolverTotalIterationCount += numberOfLinearIterations;

               if ( basinModel->debug1 ) {
                  PetscPrintf ( PETSC_COMM_WORLD, " The re-tried (%i) pressure solver exit condition was: %s \n", linearSolveLoop, getKspConvergedReasonImage ( convergedReason ).c_str ());
               }

            }

            // If all the loops with GMRES have falied we give one last shot to HYPRE
            if( (PETSC_FALSE == disableHypre) && (convergedReason < 0) && (pressureLinearSolver->getPCtype() != PCHYPRE) )
            {
               PetscPrintf ( PETSC_COMM_WORLD,
                             " MeSsAgE WARNING The pressure solver exit condition was: %s. Retrying with HYPRE preconditioner. \n",
                             getKspConvergedReasonImage ( convergedReason ).c_str ());

               pressureLinearSolver->setPCtype( PCHYPRE );

               pressureLinearSolver->solve ( Jacobian, Residual, Residual_Solution,
                                             &numberOfLinearIterations, &convergedReason, &linearSolverResidualNorm );

               linearSolverTotalIterationCount += numberOfLinearIterations;
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

      PetscTime(&End_Time);
      timeStepCalculationTime = End_Time - Start_Time;
      Total_Solve_Time = Total_Solve_Time + timeStepCalculationTime;

      System_Solve_Time = System_Solve_Time + timeStepCalculationTime;

      if ( totalNumberOfNonlinearIterations == 0 ) {

        // Set theta to 0.5, so that the initial update is not too big a jump.
        // This helps to improve the solving on, especially, layers with high deposition rates.
        Theta = 0.5;
      } else if ( totalNumberOfNonlinearIterations <= 2 ) {

        // Set to 1, since the initial value is 0.5 only to help with the initial pressure solution.
        Theta = 1.0;
      } else if ( totalNumberOfNonlinearIterations > 2 ) {

        if ( Previous_Residual_Length < Residual_Length ) {
          Theta = 0.5 * Theta;
          Theta = PetscMax ( Theta, 0.1 );
        } else {
          Theta = PetscMin ( 1.0, Theta + Theta_Increment );
        }

      }

      Previous_Residual_Length = Residual_Length;

      // Update the overpressure solution. p_{n+1} = p_{n} + \theta \times R(p_{n}) / Jac(p_{n})
      VecAXPY( Overpressure, Theta, Residual_Solution );
      VecNorm ( Overpressure, NORM_2, &Po_Norm );

      mapping.putSolution ( Overpressure );
      pressureSolver->checkPressureSolution ();

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
         overpressureHasDiverged = std::isnan( Po_Norm ) || std::isnan( Solution_Length ) || std::isnan( Residual_Solution_Length ) || std::isnan( Residual_Length );

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
        pressureSolver->computeDependantProperties ( previousTime, currentTime, false );
      }

      PetscTime(&Iteration_End_Time);
      Iteration_Time = Iteration_End_Time - Iteration_Start_Time;
      Total_Iteration_Time = Total_Iteration_Time + Iteration_Time;

      if (( basinModel -> debug1 || basinModel->verbose ) && ( FastcauldronSimulator::getInstance ().getRank () == 0 ))
      {
        cout << " Newton iteration (p): "
             << setw (  5 ) << totalNumberOfNonlinearIterations + 1
             << setw ( 16 ) << Residual_Length
             << setw ( 16 ) << Solution_Length
             << setw ( 16 ) << Residual_Solution_Length
             << setw ( 16 ) << ( Residual_Solution_Length / Solution_Length )
             << setw ( 16 ) << Po_Norm - Previous_Po_Norm
             << setw ( 16 ) << linearSolverResidualNorm
             << setw (  5 ) << numberOfLinearIterations
             << setw ( 16 ) << Theta
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
      HydraulicFracturingManager::getInstance ().checkForFracturing ( currentTime, applyNonConservativeModel, hasFractured );
    }

    fracturingOccurred = fracturingOccurred or hasFractured;
    ++fractureIterations;
  } while ( hasFractured and not overpressureHasDiverged and
            fractureIterations <= MaximumNumberOfFractureIterations );


  StatisticsHandler::update ();

  if ( HydraulicFracturingManager::getInstance ().isNonConservativeFractureModel ()) {
    HydraulicFracturingManager::getInstance ().restrictPressure ( currentTime );
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
   ( const double  previousTime,
     const double  currentTime,
     const int     MaximumNumberOfNonlinearIterations,
           bool&   temperatureHasDiverged,
           int&    Number_Of_Nonlinear_Iterations,
           double& T_Norm ) {

  if ( basinModel -> Nonlinear_Temperature ) {
    Solve_Nonlinear_Temperature_For_Time_Step ( previousTime,
                                                currentTime,
                                                MaximumNumberOfNonlinearIterations,
                                                false,
                                                temperatureHasDiverged,
                                                Number_Of_Nonlinear_Iterations,
                                                T_Norm );
  } else {
    Solve_Linear_Temperature_For_Time_Step ( previousTime,
                                             currentTime,
                                             temperatureHasDiverged,
                                             T_Norm );
    Number_Of_Nonlinear_Iterations = 1;
  }

}


//------------------------------------------------------------//

#undef  __FUNCT__
#define __FUNCT__ "Basin_Modelling::FEM_Grid::Solve_Nonlinear_Temperature_For_Time_Step"

void Basin_Modelling::FEM_Grid::Solve_Nonlinear_Temperature_For_Time_Step ( const double  previousTime,
                                                                            const double  currentTime,
                                                                            const int     MaximumNumberOfNonlinearIterations,
                                                                            const bool    isSteadyStateCalculation,
                                                                            bool&         temperatureHasDiverged,
                                                                            int&          Number_Of_Nonlinear_Iterations,
                                                                            double&       T_Norm ) {

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

  boost::shared_ptr< PetscSolver > temperatureLinearSolver;

  if (isSteadyStateCalculation || strcmp(basinModel->Temperature_Linear_Solver_Type, KSPCG) == 0)
  {
     temperatureLinearSolver.reset ( new PetscCG ( Temperature_Calculator.linearSolverTolerance ( basinModel->Optimisation_Level )));
  }
  else
  {
     temperatureLinearSolver.reset ( new PetscGMRES ( Temperature_Calculator.linearSolverTolerance ( basinModel->Optimisation_Level ),
                                                      basinModel->Temperature_GMRes_Restart ));
  }

  temperatureLinearSolver->loadCmdLineOptions();

  PetscScalar Previous_T_Norm = 0.0;

  PetscLogStages :: push( PetscLogStages :: TEMPERATURE_INITIALISATION_LINEAR_SOLVER );

  maximumNumberOfLinearSolverIterations = temperatureLinearSolver->getMaxIterations();

  WallTime::Time matrixAllocationStartTime = WallTime::clock ();

  Jacobian = PetscObjectAllocator::allocateMatrix ( m_temperatureComputationalDomain );
  m_temperatureMatrixAllocationTime += ( WallTime::clock () - matrixAllocationStartTime ).floatValue ();

  Residual = PetscObjectAllocator::allocateVector ( m_temperatureComputationalDomain );
  Residual_Solution = PetscObjectAllocator::allocateVector ( m_temperatureComputationalDomain );
  Temperature = PetscObjectAllocator::allocateVector ( m_temperatureComputationalDomain );

  SolutionVectorMapping mapping ( m_temperatureComputationalDomain, Basin_Modelling::Temperature );
  mapping.getSolution ( Temperature );

  PetscScalar Theta = 1.0;

  VecNorm ( Temperature, NORM_2, &T_Norm );

  if (( basinModel -> debug1 || basinModel->verbose) && ( FastcauldronSimulator::getInstance ().getRank () == 0 )) {
    cout << " Number of DOFs  = " << m_temperatureComputationalDomain.getGlobalNumberOfActiveNodes () << endl;
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

  // If the input provided m_saveTimeStep is equal to the age of the basin (case m_saveTimeStep == previousTime) or
  // the current time step is the first one greater or equal to m_saveTimeStep, then matrix and RHS will be saved
  const bool saveMatrix = m_saveMatrixToFile and
                          ( (( m_saveTimeStep - currentTime >= 0.0 ) and ( m_saveTimeStep - previousTime < 0.0 ))
                            or (m_saveTimeStep == previousTime) );

  while ( ! Converged && ! temperatureHasDiverged ) {
    PetscTime(&Iteration_Start_Time);

    VecSet ( Residual, Zero );

    PetscTime(&Jacobian_Start_Time);

    if ( Number_Of_Nonlinear_Iterations == 0 ) {
      MatZeroEntries ( Jacobian );
      PetscTime(&Assembly_Start_Time);
      Temperature_Calculator.assembleSystem ( m_temperatureComputationalDomain,
                                              previousTime, currentTime,
                                              Jacobian, Residual,
                                              Element_Contributions_Time );
      PetscTime(&Assembly_End_Time);

      Element_Assembly_Time = Element_Assembly_Time + Element_Contributions_Time;
      System_Assembly_Time = System_Assembly_Time + Assembly_End_Time - Assembly_Start_Time;

      // Print matrix and rhs to file
      if( saveMatrix )
      {
         const std::string matrixFileName = std::string("nlTempMatrix_t" + static_cast<ostringstream*>( &(ostringstream() << currentTime) )->str() );
         const std::string rhsFileName    = std::string(   "nlTempRhs_t" + static_cast<ostringstream*>( &(ostringstream() << currentTime) )->str() );

         int rc = 0;
         rc = PetscObjectsIO::writeMatrixToFile( Jacobian, basinModel->getOutputDirectory(), matrixFileName, !m_saveInMatlabFormat );
         assert( rc == 0 );
         rc = PetscObjectsIO::writeVectorToFile( Residual, basinModel->getOutputDirectory(), rhsFileName, !m_saveInMatlabFormat );
         assert( rc == 0 );
      }

    } else {

      // It is not always necessary to compute the Jacobian, the one from the previous
      // iteration is good enough. This saves some time not only because the Jacobian
      // is not computed but also the preconditioner remains the same as for the previous
      // iteration.
      PetscTime(&Assembly_Start_Time);
      Temperature_Calculator.assembleResidual ( m_temperatureComputationalDomain,
                                                previousTime, currentTime,
                                                Residual,
                                                Element_Contributions_Time );
      PetscTime(&Assembly_End_Time);


      System_Assembly_Time = System_Assembly_Time + Assembly_End_Time - Assembly_Start_Time;
      Element_Assembly_Time = Element_Assembly_Time + Element_Contributions_Time;

    }

    PetscTime(&Start_Time);

    PetscTime(&Jacobian_End_Time);
    Jacobian_Time = Jacobian_End_Time - Jacobian_Start_Time;
    Total_Jacobian_Time = Total_Jacobian_Time + Jacobian_Time;

    VecSet ( Residual_Solution, Zero );

    KSPConvergedReason convergedReason;
    temperatureLinearSolver->solve( Jacobian, Residual, Residual_Solution, &numberOfLinearIterations, &convergedReason);
    temperatureHasDiverged = ( numberOfLinearIterations == maximumNumberOfLinearSolverIterations ) || convergedReason == KSP_DIVERGED_NANORINF;

    PetscTime(&End_Time);

    // Print solution to file
    if( saveMatrix )
    {
       const std::string solFileName = std::string( "nlTemSol_t" + static_cast<ostringstream*>( &(ostringstream() << currentTime) )->str() );
       int rc = PetscObjectsIO::writeVectorToFile( Residual_Solution, basinModel->getOutputDirectory(), solFileName, !m_saveInMatlabFormat );
       assert( rc == 0 );

       temperatureLinearSolver->viewSettings();
    }

    timeStepCalculationTime   = End_Time - Start_Time;
    Total_Solve_Time  = Total_Solve_Time + timeStepCalculationTime;
    System_Solve_Time = System_Solve_Time + timeStepCalculationTime;

    // Check this!!
    VecAXPY( Temperature, Theta, Residual_Solution );
    VecNorm ( Temperature, NORM_2, &T_Norm );

    mapping.putSolution ( Temperature );
    Temperature_Calculator.correctTemperatureSolution ( currentTime );

    Compute_Temperature_Dependant_Properties ( currentTime );

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

  VecDestroy  ( &Residual );
  VecDestroy  ( &Temperature );
  VecDestroy  ( &Residual_Solution );
  MatDestroy  ( &Jacobian );

  PetscLogStages :: pop();
}

//------------------------------------------------------------//

#undef  __FUNCT__
#define __FUNCT__ "Basin_Modelling::FEM_Grid::Solve_Linear_Temperature_For_Time_Step"

void Basin_Modelling::FEM_Grid::Solve_Linear_Temperature_For_Time_Step ( const double previousTime,
                                                                         const double currentTime,
                                                                         bool&        temperatureHasDiverged,
                                                                         double&      T_Norm ) {

  int numberOfLinearIterations;

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

  boost::shared_ptr< PetscSolver > temperatureLinearSolver;
  temperatureHasDiverged = false;

  if ( strcmp(basinModel->Temperature_Linear_Solver_Type, KSPCG) == 0)
  {
     temperatureLinearSolver.reset ( new PetscCG ( Temperature_Calculator.linearSolverTolerance ( basinModel->Optimisation_Level )));
  }
  else
  {
     temperatureLinearSolver.reset ( new PetscGMRES ( Temperature_Calculator.linearSolverTolerance ( basinModel->Optimisation_Level ),
                                                      basinModel->Temperature_GMRes_Restart ));
  }

  temperatureLinearSolver->setInitialGuessNonZero ( true );
  temperatureLinearSolver->loadCmdLineOptions();

  PetscLogStages::push( PetscLogStages :: TEMPERATURE_LINEAR_SOLVER );
  PetscTime(&Iteration_Start_Time);

  PetscTime(&matrixStartTime);


  WallTime::Time matrixAllocationStartTime = WallTime::clock ();
  Stiffness_Matrix = PetscObjectAllocator::allocateMatrix ( m_temperatureComputationalDomain );
  m_temperatureMatrixAllocationTime += ( WallTime::clock () - matrixAllocationStartTime ).floatValue ();

  Load_Vector = PetscObjectAllocator::allocateVector ( m_temperatureComputationalDomain );
  Temperature = PetscObjectAllocator::allocateVector ( m_temperatureComputationalDomain );


  PetscTime(&matrixEndTime);
  matrixCreationTime = matrixEndTime - matrixStartTime;

  PetscTime(&dupStartTime);

  PetscTime(&dupEndTime);
  dupCreationTime = dupEndTime - dupStartTime;

  PetscTime(&System_Assembly_Start_Time);
  VecSet ( Load_Vector, Zero );

  MatZeroEntries ( Stiffness_Matrix );

  Temperature_Calculator.assembleStiffnessMatrix ( m_temperatureComputationalDomain,
                                                   previousTime,
                                                   currentTime,
                                                   Stiffness_Matrix,
                                                   Load_Vector,
                                                   Element_Contributions_Time );

  PetscTime(&System_Assembly_End_Time);
  Total_System_Assembly_Time = System_Assembly_End_Time - System_Assembly_Start_Time;

  // Print matrix and rhs to file
  // If the input provided m_saveTimeStep is equal to the age of the basin (case m_saveTimeStep == previousTime) or
  // the current time step is the first one greater or equal to m_saveTimeStep, then matrix and RHS will be saved
  const bool writeToFile = m_saveMatrixToFile and
      ( (( m_saveTimeStep - currentTime >= 0.0 ) and ( m_saveTimeStep - previousTime < 0.0 ))
        or (m_saveTimeStep == previousTime) );
  if( writeToFile )
  {
    const std::string matrixFileName = std::string("tempMatrix_t" + static_cast<ostringstream*>( &(ostringstream() << currentTime) )->str() );
    const std::string rhsFileName    = std::string(   "tempRhs_t" + static_cast<ostringstream*>( &(ostringstream() << currentTime) )->str() );

    int rc = 0;
    rc = PetscObjectsIO::writeMatrixToFile( Stiffness_Matrix, basinModel->getOutputDirectory(), matrixFileName, !m_saveInMatlabFormat );
    assert( rc == 0 );
    rc = PetscObjectsIO::writeVectorToFile( Load_Vector, basinModel->getOutputDirectory(), rhsFileName, !m_saveInMatlabFormat );
    assert( rc == 0 );
  }

  // Solve the linear system Temperature = Stiffness_Matrix^-1 * Load_Vector
  PetscTime(&Start_Time);
  PetscTime(&restoreStartTime);

  WallTime::Time solutionMappingStartTime = WallTime::clock ();
  SolutionVectorMapping mapping ( m_temperatureComputationalDomain, Basin_Modelling::Temperature );

  mapping.getSolution ( Temperature );
  m_temperatureSolutionMappingTime += ( WallTime::clock () - solutionMappingStartTime ).floatValue ();

  PetscTime(&restoreEndTime);
  restoreCreationTime = restoreEndTime - restoreStartTime;

  KSPConvergedReason convergedReason;
  temperatureLinearSolver->solve(Stiffness_Matrix, Load_Vector, Temperature, &numberOfLinearIterations, &convergedReason);

  if ( convergedReason < 0 ) {
     // convergedReason < 0 indicates that the linear solver has failed for some reason.

     LogHandler( LogHandler::DEBUG_SEVERITY ) << "Switching linear solver for temperature due to failure: "
                                              << getKspConvergedReasonImage ( convergedReason ).c_str ();

     // Do not use the solution obtained from the first attempt, as the last attempt may have diverged
     // and filled the vector with garbage. So get the initial solution again.
     mapping.getSolution ( Temperature );

     // Switch linear solver to GMRes with a sufficiently large restart value.
     temperatureLinearSolver.reset ( new PetscGMRES ( Temperature_Calculator.linearSolverTolerance ( basinModel->Optimisation_Level ),
                                                      basinModel->Temperature_GMRes_Restart ));

     // Tell the linear solver that the initial guess is not the zero vector.
     temperatureLinearSolver->setInitialGuessNonZero ( true );
     temperatureLinearSolver->loadCmdLineOptions();
     temperatureLinearSolver->solve(Stiffness_Matrix, Load_Vector, Temperature, &numberOfLinearIterations, &convergedReason);

     if ( convergedReason < 0 ) {
        // Despite changing linear solvers from the default to GMRes, for
        // some reason the linear solver still failed to reach a solution.
        temperatureHasDiverged = true;
     }

  }

  VecNorm ( Temperature, NORM_2, &T_Norm );

  PetscTime(&End_Time);

  // Print solution to file
  if( writeToFile )
  {
     const std::string solFileName    = std::string( "tempSol_t" + static_cast<ostringstream*>( &(ostringstream() << currentTime) )->str() );
     int rc = PetscObjectsIO::writeVectorToFile( Temperature, basinModel->getOutputDirectory(), solFileName, !m_saveInMatlabFormat );
     assert( rc == 0 );

     temperatureLinearSolver->viewSettings();
  }

  Solve_Time = End_Time - Start_Time;


  PetscTime(&storeStartTime);

  mapping.putSolution ( Temperature );
  Temperature_Calculator.correctTemperatureSolution ( currentTime );
  PetscTime(&storeEndTime);
  storeCreationTime = storeEndTime - storeStartTime;

  PetscTime(&Start_Time);

  Compute_Temperature_Dependant_Properties ( currentTime );
  PetscTime(&End_Time);
  Property_Time = End_Time - Start_Time;

  // if DEBUG3 set then call the script fastcauldron_performance
  if ( basinModel -> debug3 ) {
    system( "fastcauldron_PostNLsolve" );
  }

  StatisticsHandler::update ();

  PetscTime(&Iteration_End_Time);
  Iteration_Time = Iteration_End_Time - Iteration_Start_Time;

  if (( basinModel -> debug1 || basinModel->verbose ) && ( FastcauldronSimulator::getInstance ().getRank () == 0 )) {
    cout << " Temperature solving: "
         << m_temperatureComputationalDomain.getGlobalNumberOfActiveNodes () << "  "
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

void Basin_Modelling::FEM_Grid::Solve_Coupled_For_Time_Step ( const double  previousTime,
                                                              const double  currentTime,
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
    Solve_Pressure_For_Time_Step ( previousTime,
                                   currentTime,
                                   Maximum_Number_Of_Nonlinear_Pressure_Iterations,
                                   overpressureHasDiverged,
                                   Number_Of_Pressure_Nonlinear_Iterations,
                                   Po_Norm,
                                   fracturingOccurred );

    // compute lithostatic pressure in basement.

    if ( not overpressureHasDiverged ) {
      Solve_Temperature_For_Time_Step ( previousTime,
                                        currentTime,
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
#define __FUNCT__ "Basin_Modelling::FEM_Grid::Compute_Temperature_Dependant_Properties"

void Basin_Modelling::FEM_Grid::Compute_Temperature_Dependant_Properties ( const double currentTime ) {

  PetscLogDouble Start_Time;
  PetscLogDouble End_Time;

  PetscTime(&Start_Time);

  if ( ! basinModel -> DoOverPressure && ! basinModel -> Do_Iteratively_Coupled ) {
    basinModel -> Calculate_Pressure ( currentTime );
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
        Surface_Temperature = FastcauldronSimulator::getInstance ().getSeaBottomTemperature ( I, J, currentTime );
        Seabottom_Depth     = FastcauldronSimulator::getInstance ().getSeaBottomDepth ( I, J, currentTime );

        for ( K = Z_Count - 2; K >= 0; K-- ) {

          if ( Current_Layer -> Current_Properties ( Basin_Modelling::Temperature, K, J, I ) == CauldronNoDataValue ) {

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
#define __FUNCT__ "Basin_Modelling::FEM_Grid::Copy_Current_Properties"

void Basin_Modelling::FEM_Grid::Copy_Current_Properties () {

  Layer_Iterator Layers ( basinModel -> layers, Ascending, Basement_And_Sediments, Active_Layers_Only );

  for ( Layers.Initialise_Iterator (); ! Layers.Iteration_Is_Done (); Layers++ ) {
     Layers.Current_Layer ()->copyProperties ();
  }

}


//------------------------------------------------------------//

#undef  __FUNCT__
#define __FUNCT__ "Basin_Modelling::FEM_Grid::Store_Computed_Deposition_Thickness"

void Basin_Modelling::FEM_Grid::Store_Computed_Deposition_Thickness ( const double currentTime ) {

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

   // Most of the time only the first layer has to be treated. However, for igneous intrusion and mobile layers,
   // the time step of deposition can be zero, which means that two layers are deposited at the same time
   // hence the for loop and the "if (!condition) break";
   for (Layers.Initialise_Iterator (); ! Layers.Iteration_Is_Done (); Layers.Next() )
   {

      LayerProps_Ptr Current_Layer = Layers.Current_Layer ();
      // Is this the best number to have here? I think so, but cannot prove it.
      // The snapshot times, time step, ... are written out (perhaps read in too)
      // as a float, but stored and used as doubles.
      const double Float_Epsilon = pow ( 2.0, -23 );

      if ( fabs ( currentTime - Current_Layer -> depoage ) >= NumericFunctions::Maximum ( currentTime, 1.0 ) * Float_Epsilon ) break;

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

void Basin_Modelling::FEM_Grid::Set_Pressure_Dependent_Properties ( const double currentTime ) {

   CalculationMode calculationMode = FastcauldronSimulator::getInstance ().getCalculationMode ();

   if ( calculationMode == OVERPRESSURE_MODE or
        calculationMode == NO_CALCULATION_MODE ) {
      return;
   }

   if ( currentTime == FastcauldronSimulator::getInstance ().getAgeOfBasin ()) {
      // There is no need to distinguish between geometric-loop and non-geometric-loop
      // since the basement lithologies have zero porosity.
      basinModel->calcNodeDepths ( currentTime );
   } else if ( calculationMode != PRESSURE_AND_TEMPERATURE_MODE and calculationMode != COUPLED_DARCY_MODE ) {
      basinModel->calcNodeVes    ( currentTime );
      basinModel->calcNodeMaxVes ( currentTime );
      basinModel->calcPorosities ( currentTime );

      if ( calculationMode == OVERPRESSURED_TEMPERATURE_MODE and not basinModel->isGeometricLoop ()) {
         basinModel->setNodeDepths ( currentTime );
      } else {
         basinModel->calcNodeDepths ( currentTime );
      }

   }

   computeBasementLithostaticPressure ( basinModel, currentTime );
}


//------------------------------------------------------------//

#undef  __FUNCT__
#define __FUNCT__ "Basin_Modelling::FEM_Grid::Initialise_Basin_Temperature"

void Basin_Modelling::FEM_Grid::Initialise_Basin_Temperature ( bool& temperatureHasDiverged ) {

   int numberOfNewtonIterations = -1;
   double currentTime = FastcauldronSimulator::getInstance ().getAgeOfBasin ();
   // double currentTime = basinModel -> Age_Of_Basin ();

  double T_Norm;

  basinModel -> findActiveElements ( currentTime );
  Construct_Temperature_FEM_Grid ( currentTime, FastcauldronSimulator::getInstance ().getAgeOfBasin (), majorSnapshots, false );

  FastcauldronSimulator::getInstance ().switchLithologies ( basinModel->Crust ()->depoage );
  setLayerElements ( basinModel->Crust ()->depoage );

  Temperature_Calculator.Compute_Crust_Heat_Production ();
  Temperature_Calculator.Estimate_Basement_Temperature ();

  m_temperatureComputationalDomain.resetAge ( basinModel->Crust ()->depoage );
  Solve_Nonlinear_Temperature_For_Time_Step ( currentTime,
                                              FastcauldronSimulator::getInstance ().getAgeOfBasin (),
                                              10,
                                              true, // steady state calculation.
                                              temperatureHasDiverged,
                                              numberOfNewtonIterations,
                                              T_Norm );

  computeBasementLithostaticPressureForCurrentTimeStep ( basinModel, currentTime );
  printRelatedProjects ( currentTime );
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

  int K;
  int X_Start;
  int Y_Start;
  int Z_Start;
  int X_Count;
  int Y_Count;
  int Z_Count;

  const Boolean2DArray& Valid_Needle = basinModel->getValidNeedles ();

  Layer_Iterator Pressure_Layers;
  LayerProps_Ptr Current_Layer;
  const CompoundLithology*  Current_Lithology;

  const FluidType* currentFluid;

  bool   Include_Chemical_Compaction;
  double Permeability_Normal_Compound;
  double Permeability_Plane_Compound;
  double fracturePressure;

  CompoundProperty Porosity;

  string Layer_Name;
  std::stringstream buffer;

  buffer.setf ( ios::scientific );

  Layer_Range Basin_Bottom;

  if ( basinModel->DoOverPressure ) {
    Basin_Bottom = Sediments_Only;
  } else {
    Basin_Bottom = Basement_And_Sediments;
  }

  size_t maximumNameLength = 0;

  for ( size_t i = 0; i < basinModel->layers.size (); ++i ) {
     std::string name = basinModel->layers [ i ]->layername;

     if ( basinModel->layers [ i ]->layername.length () > maximumNameLength ) {
        maximumNameLength = basinModel->layers [ i ]->layername.length ();
     }

  }

  Pressure_Layers.Initialise_Iterator ( basinModel->layers, Descending, Basin_Bottom, Active_Layers_Only );

  DMDAGetCorners ( *basinModel->mapDA, &X_Start, &Y_Start, PETSC_NULL, &X_Count, &Y_Count, PETSC_NULL );

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

  if (( X_Start <= I ) && ( I < X_Start + X_Count ) && ( Y_Start <= J ) && ( J < Y_Start + Y_Count ) && Valid_Needle ( I, J )) {
    buffer << endl << endl;
    buffer << " Printing details of needle " << I << "  " << J << endl;
    buffer << setw ( maximumNameLength ) << "  ";
    buffer << "                 Age            Depth          Po            Pp            Ph            Pl            Pf           VES         Max_VES       Porosity        PermN          PermH              Temperature" << endl;
    onThisNode = true;
  }

  for ( Pressure_Layers.Initialise_Iterator (); ! Pressure_Layers.Iteration_Is_Done (); Pressure_Layers++ ) {
    Current_Layer = Pressure_Layers.Current_Layer ();
    Layer_Name = Current_Layer -> layername;
    blankSpaceUnderscore ( Layer_Name );

    Include_Chemical_Compaction = (( basinModel -> Do_Chemical_Compaction ) && ( Current_Layer -> Get_Chemical_Compaction_Mode ()));

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

            buffer << setw ( maximumNameLength ) << Layer_Name << "  " ;
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
            buffer << setw ( 14 ) << Current_Layer -> Current_Properties ( Basin_Modelling::VES_FP, K, J, I ) * PaToMegaPa;
            buffer << setw ( 14 ) << Current_Layer -> Current_Properties ( Basin_Modelling::Max_VES, K, J, I ) * PaToMegaPa;
            buffer << setw ( 14 ) << Porosity.mixedProperty ();
            buffer << setw ( 14 ) << log10 ( Permeability_Normal_Compound / MilliDarcyToM2 );
            buffer << setw ( 15 ) << log10 ( Permeability_Plane_Compound / MilliDarcyToM2 );
            buffer << setw (  4 ) << ( Current_Layer->kind () == Interface::SEDIMENT_FORMATION ? int ( Current_Layer -> fracturedPermeabilityScaling ( I, J, K )) : -1 );
            buffer << setw (  4 ) << value;
            buffer << setw ( 14 ) << Current_Layer -> Current_Properties ( Basin_Modelling::Temperature, K, J, I );
            buffer << setw ( 14 ) << Current_Layer -> Current_Properties ( Basin_Modelling::Chemical_Compaction, K, J, I );

            if ( includedInDarcySimulation and Current_Layer->kind () == Interface::SEDIMENT_FORMATION ) {

               if ( K == Z_Start + Z_Count - 1 ) {
                  buffer << setw ( 14 ) << saturations ( K - 1, elementJ, elementI )( Saturation::WATER              );
                  buffer << setw ( 14 ) << saturations ( K - 1, elementJ, elementI )( Saturation::LIQUID             );
                  buffer << setw ( 14 ) << saturations ( K - 1, elementJ, elementI )( Saturation::VAPOUR             );
                  buffer << setw ( 14 ) << saturations ( K - 1, elementJ, elementI )( Saturation::IMMOBILE           );
                  buffer << setw ( 14 ) << concentrations ( K - 1, elementJ, elementI )( ComponentId::C1             );
                  buffer << setw ( 14 ) << concentrations ( K - 1, elementJ, elementI )( ComponentId::C2             );
                  buffer << setw ( 14 ) << concentrations ( K - 1, elementJ, elementI )( ComponentId::C3             );
                  buffer << setw ( 14 ) << concentrations ( K - 1, elementJ, elementI )( ComponentId::C4             );
                  buffer << setw ( 14 ) << concentrations ( K - 1, elementJ, elementI )( ComponentId::C5             );
                  buffer << setw ( 14 ) << concentrations ( K - 1, elementJ, elementI )( ComponentId::C6_MINUS_14SAT );
                  buffer << setw ( 14 ) << concentrations ( K - 1, elementJ, elementI )( ComponentId::C6_MINUS_14ARO );
                  buffer << setw ( 14 ) << concentrations ( K - 1, elementJ, elementI )( ComponentId::C15_PLUS_SAT   );
                  buffer << setw ( 14 ) << concentrations ( K - 1, elementJ, elementI )( ComponentId::C15_PLUS_ARO   );
               } else {

                  PVTComponents composition = concentrations ( K, elementJ, elementI );
                  PVTPhaseComponents phaseComposition;
                  PVTPhaseValues     phaseDensities;
                  PVTPhaseValues     phaseViscosities;

                  composition *= 0.001;


                  pvtFlash::EosPack::getInstance ().computeWithLumping ( Current_Layer -> Current_Properties ( Basin_Modelling::Temperature, K, J, I ) + 273.15,
                                                              Current_Layer -> Current_Properties ( Basin_Modelling::Pore_Pressure, K, J, I ) * 1.0e6,
                                                              composition.m_components,
                                                              phaseComposition.m_masses,
                                                              phaseDensities.m_values,
                                                              phaseViscosities.m_values );


                  buffer << setw ( 14 ) << saturations ( K, elementJ, elementI )( Saturation::WATER    );
                  buffer << setw ( 14 ) << saturations ( K, elementJ, elementI )( Saturation::LIQUID   );
                  buffer << setw ( 14 ) << saturations ( K, elementJ, elementI )( Saturation::VAPOUR   );
                  buffer << setw ( 14 ) << saturations ( K, elementJ, elementI )( Saturation::IMMOBILE );

                  buffer << setw ( 14 ) << concentrations ( K, elementJ, elementI )( ComponentId::C1             );
                  buffer << setw ( 14 ) << concentrations ( K, elementJ, elementI )( ComponentId::C2             );
                  buffer << setw ( 14 ) << concentrations ( K, elementJ, elementI )( ComponentId::C3             );
                  buffer << setw ( 14 ) << concentrations ( K, elementJ, elementI )( ComponentId::C4             );
                  buffer << setw ( 14 ) << concentrations ( K, elementJ, elementI )( ComponentId::C5             );
                  buffer << setw ( 14 ) << concentrations ( K, elementJ, elementI )( ComponentId::C6_MINUS_14SAT );
                  buffer << setw ( 14 ) << concentrations ( K, elementJ, elementI )( ComponentId::C6_MINUS_14ARO );
                  buffer << setw ( 14 ) << concentrations ( K, elementJ, elementI )( ComponentId::C15_PLUS_SAT   );
                  buffer << setw ( 14 ) << concentrations ( K, elementJ, elementI )( ComponentId::C15_PLUS_ARO   );
                  buffer << setw ( 14 ) << concentrations ( K, elementJ, elementI )( ComponentId::RESIN          );
                  buffer << setw ( 14 ) << concentrations ( K, elementJ, elementI )( ComponentId::ASPHALTENE     );

                  buffer << setw ( 14 ) << phaseDensities   ( PhaseId::VAPOUR );
                  buffer << setw ( 14 ) << phaseDensities   ( PhaseId::LIQUID );
                  buffer << setw ( 14 ) << phaseViscosities ( PhaseId::VAPOUR );
                  buffer << setw ( 14 ) << phaseViscosities ( PhaseId::LIQUID );

                  c1Sum     += concentrations ( K, elementJ, elementI )( ComponentId::C1             );
                  c2Sum     += concentrations ( K, elementJ, elementI )( ComponentId::C2             );
                  c3Sum     += concentrations ( K, elementJ, elementI )( ComponentId::C3             );
                  c4Sum     += concentrations ( K, elementJ, elementI )( ComponentId::C4             );
                  c5Sum     += concentrations ( K, elementJ, elementI )( ComponentId::C5             );
                  c6AroSum  += concentrations ( K, elementJ, elementI )( ComponentId::C6_MINUS_14ARO );
                  c6SatSum  += concentrations ( K, elementJ, elementI )( ComponentId::C6_MINUS_14SAT );
                  c15AroSum += concentrations ( K, elementJ, elementI )( ComponentId::C15_PLUS_ARO   );
                  c15SatSum += concentrations ( K, elementJ, elementI )( ComponentId::C15_PLUS_SAT   );

                  double phiAbove = Current_Lithology->porosity ( Current_Layer->Current_Properties ( Basin_Modelling::VES_FP, K + 1, J, I ),
                                                                  Current_Layer->Current_Properties ( Basin_Modelling::Max_VES, K + 1, J, I ), false, 0.0 );

                  double phiBelow = Current_Lithology->porosity ( Current_Layer->Current_Properties ( Basin_Modelling::VES_FP, K, J, I ),
                                                                  Current_Layer->Current_Properties ( Basin_Modelling::Max_VES, K, J, I ), false, 0.0 );

                  double averagePorosity = 0.5 * ( phiAbove + phiBelow );

                  double depthAbove = Current_Layer->Current_Properties ( Basin_Modelling::Depth, K + 1, J, I );
                  double depthBelow = Current_Layer->Current_Properties ( Basin_Modelling::Depth, K, J, I );

                  double thickness = 0.5 * ( depthAbove + depthBelow );

                  c1Sum2     += concentrations ( K, elementJ, elementI )( ComponentId::C1             ) * averagePorosity * thickness;
                  c2Sum2     += concentrations ( K, elementJ, elementI )( ComponentId::C2             ) * averagePorosity * thickness;
                  c3Sum2     += concentrations ( K, elementJ, elementI )( ComponentId::C3             ) * averagePorosity * thickness;
                  c4Sum2     += concentrations ( K, elementJ, elementI )( ComponentId::C4             ) * averagePorosity * thickness;
                  c5Sum2     += concentrations ( K, elementJ, elementI )( ComponentId::C5             ) * averagePorosity * thickness;
                  c6AroSum2  += concentrations ( K, elementJ, elementI )( ComponentId::C6_MINUS_14ARO ) * averagePorosity * thickness;
                  c6SatSum2  += concentrations ( K, elementJ, elementI )( ComponentId::C6_MINUS_14SAT ) * averagePorosity * thickness;
                  c15AroSum2 += concentrations ( K, elementJ, elementI )( ComponentId::C15_PLUS_ARO   ) * averagePorosity * thickness;
                  c15SatSum2 += concentrations ( K, elementJ, elementI )( ComponentId::C15_PLUS_SAT   ) * averagePorosity * thickness;

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

  PetscSynchronizedFlush ( PETSC_COMM_WORLD, PETSC_STDOUT );
}

//------------------------------------------------------------//


void Basin_Modelling::FEM_Grid::printElementNeedle ( const int i, const int j ) const {

   Layer_Iterator layers;
   layers.Initialise_Iterator ( basinModel->layers, Descending, Sediments_Only, Active_Layers_Only );

   int k;

   const FastcauldronSimulator& fastcauldron = FastcauldronSimulator::getInstance ();
   const ElementGrid&           grid         = fastcauldron.getElementGrid ();

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
const PropListVec & Basin_Modelling::FEM_Grid::getMapOutputProperties() const {

   return m_mapDerivedOutputProperties;
}
//------------------------------------------------------------//
const PropListVec & Basin_Modelling::FEM_Grid::getVolumeOutputProperties() const {

   return m_volumeDerivedOutputProperties;
}
//------------------------------------------------------------//
void Basin_Modelling::FEM_Grid::Determine_Permafrost_Time_Step ( const double  currentTime, double & timeStep ) {

   if( basinModel->permafrost () ) {
      if( basinModel->fixedTimeStep() > 0.0 ) {
         // the fixed time step overwrites all other timesteps
         timeStep = basinModel->fixedTimeStep();
      } else if ( basinModel->switchPermafrostTimeStep( currentTime )) {
         timeStep = NumericFunctions::Minimum ( timeStep, basinModel->permafrostTimeStep() );
      }
   }
}

//------------------------------------------------------------//
void Basin_Modelling::FEM_Grid::determineIgneousIntrusionTimeStep ( const double currentTime, const double previousTimeStep, double & timeStep ) {
   const double previousIgneousIntrusionTime = basinModel->getPreviousIgneousIntrusionTime( currentTime );
   //Most of the time, this function is not needed
   if( previousIgneousIntrusionTime == 99999 )
   {
      return;
   }

   //If the current time step is the beginning of an igneous intrusion, the next time step should last 25 years
   //This value allows the temperature solver not to oscillate
   if( currentTime == previousIgneousIntrusionTime )
   {
      timeStep = 0.000025;
      return;
   }
   //If the current time step is after an igneous intrusion but not too far from this intrusion
   double maxDurationEffect = std::max(0.100000, basinModel->getInitialTimeStep( currentTime ));
   if( (previousIgneousIntrusionTime - maxDurationEffect) < currentTime )
   {
      timeStep = 2 * previousTimeStep;
   }

   return;

}
//------------------------------------------------------------//

AppCtx* Basin_Modelling::FEM_Grid::getAppCtx( ) const {
   return basinModel;
}

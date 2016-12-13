//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "GeometricLoopPressureSolver.h"
#include "propinterface.h"
#include "CompoundLithology.h"
#include "PetscVectors.h"

#include "Interface/GridMap.h"
#include "NumericFunctions.h"

#include "FastcauldronSimulator.h"
#include "layer_iterators.h"
#include "fem_grid_auxiliary_functions.h"

// utilities library
#include "ConstantsNumerical.h"
using Utilities::Numerical::CauldronNoDataValue;
using Utilities::Numerical::IbsNoDataValue;
#include "ConstantsMathematics.h"
using Utilities::Maths::PaToMegaPa;
using Utilities::Maths::MegaPaToPa;
using Utilities::Maths::MilliDarcyToM2;
#include "ConstantsPhysics.h"
using Utilities::Physics::AccelerationDueToGravity;

GeometricLoopPressureSolver::GeometricLoopPressureSolver ( AppCtx* appl ) : PressureSolver ( appl ) {}

GeometricLoopPressureSolver::~GeometricLoopPressureSolver() {}


void GeometricLoopPressureSolver::adjustSolidThickness ( const double relativeThicknessTolerance,
                                                         const double absoluteThicknessTolerance,
                                                               bool&  geometryHasConverged ) {

  using namespace Basin_Modelling;

  const double scalingWeight = FastcauldronSimulator::getInstance ().getFctCorrectionScalingWeight ();

  Layer_Iterator Pressure_Layers ( cauldron->layers, Descending, Sediments_Only, Active_Layers_Only );
  LayerProps_Ptr currentLayer;

  double FCT_Scaling;
  double Input_Thickness;
  double Computed_Thickness;
  double Thickness_Relative_Error;
  double Layer_Max_Error = -1.0;
  double Local_Max_Error = -1.0;
  double Global_Max_Error = -1.0;
  double Global_Layer_Max_Error;
  int xStart, yStart, xCount, yCount;
  int localGeometryHasConverged = 1;
  int globalGeometryHasConverged;

  double sumErrorSquared;
  int    segmentCount;

  double sumAbsErrorSquared;
  double sumAbsThicknessSquared;

  double Top_Depth;
  double Bottom_Depth;

  // Get the size of the layer DA.
  DMDAGetGhostCorners ( *cauldron->mapDA, &xStart, &yStart, PETSC_NULL, &xCount, &yCount, PETSC_NULL );

  while ( ! Pressure_Layers.Iteration_Is_Done ()) {
    currentLayer = Pressure_Layers.Current_Layer ();

    sumErrorSquared = 0.0;
    segmentCount = 0;
    Layer_Max_Error = -1.0;

    sumAbsErrorSquared = 0.0;
    sumAbsThicknessSquared = 0.0;

    PETSC_2D_Array FCTCorrection;
    FCTCorrection.Set_Global_Array( *cauldron -> mapDA, currentLayer->FCTCorrection, INSERT_VALUES, true );

    PETSC_2D_Array Thickness_Error;
    Thickness_Error.Set_Global_Array( *cauldron -> mapDA, currentLayer->Thickness_Error, INSERT_VALUES, true );

    //
    //  Computed_Depths:                 Present day depths, as computed by the pressure calculator
    //  Present_Day_Eroded_Thicknesses:  Input thicknesses, but with any erosion taken into account
    //  Deposited_Thicknesses:           Computed thicknesses at deposition
    //  Present_Day_Thicknesses:         Input thicknesses as they appear in the strat table (ie with no erosion taken into account)
    //
    currentLayer->Current_Properties.Activate_Property ( Basin_Modelling::Depth, INSERT_VALUES, true );

    PETSC_2D_Array Deposited_Thicknesses ( *cauldron->mapDA, currentLayer->Computed_Deposition_Thickness, INSERT_VALUES, true );
    const Interface::GridMap * const Present_Day_Thicknesses = currentLayer->presentDayThickness;
    Present_Day_Thicknesses->retrieveGhostedData ();

    const unsigned int lastX = static_cast<unsigned int>(xStart + xCount);
    const unsigned int lastY = static_cast<unsigned int>(yStart + yCount);
    const unsigned int lastZ = currentLayer->getMaximumNumberOfElements();
    for ( unsigned int I = static_cast<unsigned int>(xStart); I < lastX; ++I ) {

      for ( unsigned int J = static_cast<unsigned int>(yStart); J < lastY; ++J ) {

        if ( cauldron->nodeIsDefined ( I, J )) {

          Bottom_Depth = currentLayer->Current_Properties ( Basin_Modelling::Depth, 0, J, I );
          Top_Depth    = currentLayer->Current_Properties ( Basin_Modelling::Depth, lastZ, J, I );

          if ( currentLayer->getPresentDayErodedThickness ( I, J ) > 0.1 ) {
            Input_Thickness    = currentLayer->getPresentDayErodedThickness ( I, J );
            Computed_Thickness = Bottom_Depth - Top_Depth;
          } else if ( Present_Day_Thicknesses->getValue ( I, J ) > 0.1 ) {
            Input_Thickness    = Present_Day_Thicknesses->getValue ( I, J );
            Computed_Thickness = Deposited_Thicknesses ( J, I );
          } else {
            Input_Thickness    = Deposited_Thicknesses ( J, I );
            Computed_Thickness = Deposited_Thicknesses ( J, I );
          }

          Thickness_Relative_Error = fabs( ( Input_Thickness - Computed_Thickness ) / Input_Thickness );

          if ( cauldron -> debug2 ) {
            PetscPrintf ( PETSC_COMM_WORLD, " Needle: %3.4f  %3.4f  %3.4f  %3.4f  %3.4f  %3.4f  %3.4f  %3.4f  %3.4f \n",
                          Input_Thickness,
                          Computed_Thickness,
                          Present_Day_Thicknesses->getValue ( I, J ),
                          currentLayer->getPresentDayErodedThickness ( I, J ),
                          Bottom_Depth,
                          Deposited_Thicknesses ( J, I ),
                          Top_Depth,
                          Computed_Thickness / Input_Thickness,
                          Thickness_Relative_Error );
          }

          // Is 1.0e-10 the best value here? Perhaps a larger value would be better.
          if ( fabs ( Input_Thickness ) > 1.0e-10 && fabs ( Computed_Thickness ) > 1.0e-10 ) {
            FCT_Scaling = ( 1.0 - scalingWeight + scalingWeight * Input_Thickness / Computed_Thickness );
          } else {
            FCT_Scaling = 1.0;
          }

          FCTCorrection ( J, I ) *= FCT_Scaling;

          if ( Input_Thickness == 0.0 ) {
            Thickness_Error ( J, I ) = 0.0;
          } else {
            Thickness_Error ( J, I ) = Thickness_Relative_Error * 100.0;
          }

          for ( unsigned int K = 0; K < lastZ; ++K ) {
            currentLayer->getSolidThickness ( I, J, K ).ScaleBy ( FCT_Scaling );
          }

          Layer_Max_Error = PetscMax ( Layer_Max_Error, Thickness_Relative_Error );

          double tmpVal = fabs ( Input_Thickness - Computed_Thickness );
          sumAbsErrorSquared += tmpVal * tmpVal;
          sumAbsThicknessSquared += Input_Thickness * Input_Thickness;

          sumErrorSquared += Thickness_Relative_Error * Thickness_Relative_Error;
          ++segmentCount;

          // Now check to see of the layer-thickness has converged.
          // The layer has not converged if at least one of the sub-needles of the layer
          // has not converged.
          // There is a special check for layer-thickness that are less than 100 mts.
          if ( Input_Thickness < 100.0 ) {

            if ( Input_Thickness > 10.0 and fabs ( Input_Thickness - Computed_Thickness ) > absoluteThicknessTolerance ) {
              localGeometryHasConverged = 0;
            }

          } else {

            if ( Thickness_Relative_Error > relativeThicknessTolerance ) {
              localGeometryHasConverged = 0;
            }

          }

        } else {
           FCTCorrection   ( J, I ) = CauldronNoDataValue;
           Thickness_Error ( J, I ) = CauldronNoDataValue;
        }

      }

    }

    Present_Day_Thicknesses->restoreData ( false, false );
    currentLayer->Current_Properties.Restore_Property ( Basin_Modelling::Depth );

    FCTCorrection.Restore_Global_Array ( Update_Excluding_Ghosts );
    Thickness_Error.Restore_Global_Array ( Update_Excluding_Ghosts );

    if ( cauldron -> debug1 or cauldron->verbose) {
      MPI_Allreduce( &Layer_Max_Error, &Global_Layer_Max_Error, 1,
                      MPI_DOUBLE, MPI_MAX, PETSC_COMM_WORLD );

      double globalSumAbsErrorSquared;
      double globalSumAbsThicknessSquared;

      MPI_Allreduce( &sumAbsErrorSquared, &globalSumAbsErrorSquared, 1,
                      MPI_DOUBLE, MPI_SUM, PETSC_COMM_WORLD );

      MPI_Allreduce( &sumAbsThicknessSquared, &globalSumAbsThicknessSquared, 1,
                      MPI_DOUBLE, MPI_SUM, PETSC_COMM_WORLD );

      PetscPrintf ( PETSC_COMM_WORLD, " Current layer %s has a maximum error of %3.4f%%  %e  %e  %e  %e %i  \n",
                    currentLayer->layername.c_str (),
                    100.0 * Global_Layer_Max_Error,
                    sumErrorSquared,
                    sumErrorSquared / double ( segmentCount ),
                    globalSumAbsErrorSquared,
                    globalSumAbsErrorSquared / globalSumAbsThicknessSquared,
                    segmentCount );

    }

    Local_Max_Error = PetscMax ( Local_Max_Error, Layer_Max_Error );

    Pressure_Layers++;
  }

  MPI_Allreduce( &Local_Max_Error, &Global_Max_Error, 1,
                 MPI_DOUBLE, MPI_MAX, PETSC_COMM_WORLD );

  // Find the minimum of all 'localGeometryHasConverged' variables.
  // If any are zero then this implies that the geometry has not converged.
  MPI_Allreduce( &localGeometryHasConverged, &globalGeometryHasConverged, 1,
                 MPI_INT, MPI_MIN, PETSC_COMM_WORLD );

  geometryHasConverged = globalGeometryHasConverged == 1;

  if ( cauldron->debug1 or cauldron->Output_Level > 0 or cauldron->verbose ) {
    PetscPrintf ( PETSC_COMM_WORLD, " Max Error: %3.4f %% \n", 100.0 * Global_Max_Error );
  }

}

//------------------------------------------------------------//

//
//
// On entry the parameters Hydrostatic_Pressure, Pore_Pressure, ..., Bulk_Density have the values
// at the top of the segment. On exit they contain values at the bottom of the segment.
//
// Real_Thickness
//
void GeometricLoopPressureSolver::computeRealThickness ( const LayerProps_Ptr currentLayer,
                                                         const CompoundLithology*  Current_Lithology,
                                                         const bool           Include_Chemical_Compaction,

                                                         const double         Solid_Thickness,

                                                         const double         Temperature_Top,
                                                         const double         Temperature_Bottom,

                                                         const double         Overpressure_Top,
                                                         const double         Overpressure_Bottom,

                                                         const double         Intermediate_Max_VES_Top,
                                                         const double         Intermediate_Max_VES_Bottom,
                                                               double&        VES,
                                                               double&        Max_VES,

                                                         const double         Porosity_Top,
                                                         const double         Chemical_Compaction,
                                                               double&        Porosity_Bottom,

                                                               double&        Hydrostatic_Pressure,
                                                               double&        Pore_Pressure_Top,
                                                               double&        Pore_Pressure_Bottom,
                                                               double&        Lithostatic_Pressure,

                                                               double&        Fluid_Density,
                                                               double&        Bulk_Density,
                                                               double&        Real_Thickness ) const {

  const int Number_Of_Segments = numberOfStepsForCompactionEquation ( cauldron -> Optimisation_Level );
  const double H = Solid_Thickness / double ( Number_Of_Segments );

  double Overpressure;
  double Temperature;

  double Segment_Real_Thickness;
  double Solid_Density = Current_Lithology->density();

  Pore_Pressure_Bottom = Pore_Pressure_Top;
  Porosity_Bottom = Current_Lithology -> porosity ( VES, Max_VES, Include_Chemical_Compaction, Chemical_Compaction );

  Real_Thickness = 0.0;

  for ( int I = 1; I <= Number_Of_Segments; ++I ) {
    Overpressure = ( Overpressure_Top * ( Number_Of_Segments - I ) + Overpressure_Bottom * I ) / Number_Of_Segments;
    Temperature  = ( Temperature_Top  * ( Number_Of_Segments - I ) + Temperature_Bottom  * I ) / Number_Of_Segments;

    Fluid_Density = currentLayer->fluid->density ( Temperature, Pore_Pressure_Bottom );

    Segment_Real_Thickness = H / ( 1.0 - Porosity_Bottom );

    Bulk_Density = Porosity_Bottom * Fluid_Density + ( 1.0 - Porosity_Bottom ) * Solid_Density;

    // We assume the solid is ice in this case
    // for the else case     H * Fluid_Density * GRAVITY * PaToMegaPa     should be 0 to be consistant.
    if ( not ( (Fluid_Density > Solid_Density) && currentLayer->fluid->SwitchPermafrost() ) )
    {
      Hydrostatic_Pressure = Hydrostatic_Pressure + Segment_Real_Thickness * Fluid_Density * AccelerationDueToGravity * PaToMegaPa;
    }

    Lithostatic_Pressure = Lithostatic_Pressure + Segment_Real_Thickness * Bulk_Density  * AccelerationDueToGravity * PaToMegaPa;

    Max_VES      = ( Intermediate_Max_VES_Top * ( Number_Of_Segments - I ) + Intermediate_Max_VES_Bottom * I ) / Number_Of_Segments;

    if ( (Fluid_Density > Solid_Density) && ( currentLayer->fluid->SwitchPermafrost() ) )  // NLSAY3: We assume the solid is ice in this case
    {
      Pore_Pressure_Bottom = Lithostatic_Pressure;
    }
    else
    {
      Pore_Pressure_Bottom = NumericFunctions::Minimum ( Hydrostatic_Pressure + Overpressure, Lithostatic_Pressure );
    }

    VES = ( Lithostatic_Pressure - Pore_Pressure_Bottom ) * MegaPaToPa;

    Max_VES = NumericFunctions::Maximum ( Max_VES, VES );

    Porosity_Bottom = Current_Lithology -> porosity ( VES, Max_VES, Include_Chemical_Compaction, Chemical_Compaction );
    Real_Thickness = Real_Thickness + Segment_Real_Thickness;
  }

}

//------------------------------------------------------------//

void GeometricLoopPressureSolver::computeDependantPropertiesForLayer
   ( const LayerProps_Ptr  currentLayer,
     const double          previousTime,
     const double          currentTime,
           PETSC_3D_Array& layerDepth,
           PETSC_3D_Array& hydrostaticPressure,
     const PETSC_3D_Array& Overpressure,
           PETSC_3D_Array& porePressure,
           PETSC_3D_Array& lithostaticPressure,
           PETSC_3D_Array& VES,
           PETSC_3D_Array& intermediateMaxVES,
     const PETSC_3D_Array& maxVES,
           PETSC_3D_Array& layerPorosity,
           PETSC_3D_Array& layerPermeabilityNormal,
           PETSC_3D_Array& layerPermeabilityPlane,
           PETSC_3D_Array& layerTemperature,
     const PETSC_3D_Array& layerChemicalCompaction ) {

  const bool includeChemicalCompaction = (( cauldron -> Do_Chemical_Compaction ) && ( currentLayer->Get_Chemical_Compaction_Mode ()));

  // Need to do some name changing here, especially with the maxVES and
  // intermediateMaxVES variables (NOT the Vectors). It can be a little confusing!
  int needleTopActiveNode;
  int currentTopmostActiveSegment;

  double depthTop;
  double solidDensity;
  double temperatureTop;
  double hydrostaticPressureTop;
  double lithostaticPressureTop;
  double porePressureTop;
  double porosityTop;
  double fluidDensityTop;
  double VESTop;
  double maxVESTop;
  double maxVESBottom;
  double bulkDensityTop;
  double temperatureBottom;
  double porePressureBottom;
  double overpressureBottom;
  double porosityBottom;
  double intermediateMaxVESBottom;
  double permeabilityNormalValue;
  double permeabilityPlaneValue;
  double realThickness;
  double solidThickness;
  double surfaceTemperature;
  double surfaceDepth;
  double layerThickness;
  double previousSolidThickness;
  double currentSolidThickness;
  double estimatedMaxVES;

  double chemicalCompactionTop;
  double chemicalCompactionBottom;

  const bool layerIsMobile = currentLayer->isMobile ();

  // There must be a better name for this variable.
  CompoundProperty porosityMixture;

  const CompoundLithology*  currentLithology;

  int X_Start, Y_Start, Z_Start, X_Count, Y_Count, Z_Count;
  DMDAGetCorners ( currentLayer->layerDA, &X_Start, &Y_Start, &Z_Start, &X_Count, &Y_Count, &Z_Count );
  const int zTopIndex = Z_Start + Z_Count - 1;

  const int lastX = X_Start + X_Count;
  const int lastY = Y_Start + Y_Count;
  for ( int I = X_Start; I < lastX; ++I ) {

    for ( int J = Y_Start; J < lastY; ++J ) {

      if ( cauldron->nodeIsDefined ( I, J )) {
        depthTop = layerDepth ( zTopIndex, J, I );

        currentLithology = currentLayer->getLithology ( I, J );

        surfaceTemperature = FastcauldronSimulator::getInstance ().getSeaBottomTemperature ( I, J, currentTime );
        surfaceDepth       = FastcauldronSimulator::getInstance ().getSeaBottomDepth ( I, J, currentTime );
        solidDensity       = currentLithology->density();
        temperatureTop     = layerTemperature ( zTopIndex, J, I );

        if ( temperatureTop == CauldronNoDataValue ) {
          temperatureTop = cauldron->Estimate_Temperature_At_Depth ( depthTop, surfaceTemperature, surfaceDepth );
        }

        hydrostaticPressureTop = hydrostaticPressure ( zTopIndex, J, I );
        lithostaticPressureTop = lithostaticPressure ( zTopIndex, J, I );

        porePressureTop        = porePressure ( zTopIndex, J, I );
        porePressureBottom     = porePressure ( zTopIndex - 1, J, I );

        // Pressure constrained here
        porePressureTop        = NumericFunctions::Minimum ( porePressureTop, lithostaticPressureTop );
        VESTop                 = VES ( zTopIndex, J, I );

        currentTopmostActiveSegment = currentLayer->Current_Topmost_Segments ( I, J );

        if ( currentTopmostActiveSegment == -1 ) {

          // Should not really be in here! In the exceptional case where we do enter
          // this then copy maxVESTop from the current top of the layer.
          maxVESTop = maxVES ( zTopIndex, J, I );
          maxVESTop = NumericFunctions::Maximum ( maxVES ( zTopIndex, J, I ), VESTop );

          needleTopActiveNode = Z_Start + Z_Count - 1;
        } else {

          previousSolidThickness = currentLayer->Previous_Properties ( Basin_Modelling::Solid_Thickness, currentTopmostActiveSegment, J, I );
          currentSolidThickness  = currentLayer->Current_Properties  ( Basin_Modelling::Solid_Thickness, currentTopmostActiveSegment, J, I );

          if ( previousSolidThickness == IbsNoDataValue ) {
            previousSolidThickness = 0.0;
          }

          if ( currentSolidThickness == IbsNoDataValue ) {
            currentSolidThickness = 0.0;
          }

          if ( not layerIsMobile and previousSolidThickness > currentSolidThickness ) {
            //
            // Layer is eroding.
            //
            // As the layer is eroding then:
            //
            //      i) Must interpolate the Max VES at the top of the layer
            //     ii) Set the properties, which depend on the Max VES, at the top of the layer to be those
            //         of interpolated Max VES. So set the Top index to be the top index of the layer.
            //

            maxVESTop    = NumericFunctions::Maximum ( maxVES ( currentTopmostActiveSegment + 1, J, I ), VESTop );
            maxVESBottom = NumericFunctions::Maximum ( maxVES ( currentTopmostActiveSegment,     J, I ), VES ( currentTopmostActiveSegment, J, I ));

            estimatedMaxVES = ( maxVESTop - maxVESBottom ) * ( currentSolidThickness / previousSolidThickness ) + maxVESBottom;

            maxVESTop = estimatedMaxVES;
            needleTopActiveNode = Z_Start + Z_Count - 1;
          } else {

            // Layer is NOT eroding.
            maxVESTop = NumericFunctions::Maximum ( maxVES ( zTopIndex, J, I ), VESTop );
            needleTopActiveNode = Z_Start + Z_Count - 1;
          }

        }

        chemicalCompactionTop = layerChemicalCompaction ( Z_Start + Z_Count - 1, J, I );
        porosityTop = currentLithology->porosity ( VESTop, maxVESTop, includeChemicalCompaction, chemicalCompactionTop );
        layerPorosity ( Z_Start + Z_Count - 1, J, I ) = porosityTop;

        fluidDensityTop = currentLayer->fluid->density ( temperatureTop, porePressureTop );
        bulkDensityTop  = porosityTop * fluidDensityTop + ( 1.0 - porosityTop ) * solidDensity;
        layerThickness  = 0.0;

        // Compute the permeability and set the vectors accordingly.
        currentLithology->getPorosity ( VESTop, maxVESTop, includeChemicalCompaction, chemicalCompactionTop, porosityMixture );
        currentLithology->calcBulkPermeabilityNP ( VESTop, maxVESTop, porosityMixture,
                                                    permeabilityNormalValue,
                                                    permeabilityPlaneValue );

        layerPermeabilityNormal ( Z_Start + Z_Count - 1, J, I ) = permeabilityNormalValue / MilliDarcyToM2;
        layerPermeabilityPlane  ( Z_Start + Z_Count - 1, J, I ) = permeabilityPlaneValue  / MilliDarcyToM2;

        // Initialise properties on all inactive nodes and the top most
        // active node to be the the value at the top of the layer.
        for ( int K = needleTopActiveNode; K >= currentTopmostActiveSegment + 1; --K ) {
          layerDepth          ( K, J, I ) = depthTop;
          hydrostaticPressure ( K, J, I ) = hydrostaticPressureTop;
          lithostaticPressure ( K, J, I ) = lithostaticPressureTop;
          porePressure        ( K, J, I ) = porePressureTop;
          layerPorosity       ( K, J, I ) = porosityTop;
          VES                 ( K, J, I ) = VESTop;
          intermediateMaxVES  ( K, J, I ) = maxVESTop;
          layerTemperature    ( K, J, I ) = temperatureTop;
        }

        // Now compute the properties at the bottom of all active segments
        for ( int K = currentTopmostActiveSegment; K >= 0; --K ) {

          intermediateMaxVESBottom = maxVES ( K, J, I );
          solidThickness  = currentLayer->Current_Properties ( Basin_Modelling::Solid_Thickness, K, J, I );

          if ( solidThickness == IbsNoDataValue ) {
            solidThickness = 0.0;
          }

          // Make first estimate of real thickness, based on the currently
          // stored value of the porosity at the bottom of the segment.
          double Overpressure_Top = Overpressure ( K + 1, J, I );
          double intermediateMaxVESTop = intermediateMaxVES ( K + 1, J, I );

          temperatureTop     = layerTemperature ( K + 1, J, I );
          overpressureBottom = Overpressure ( K, J, I );
          temperatureBottom  = layerTemperature ( K, J, I );
          porosityBottom     = layerPorosity ( K, J, I );
          chemicalCompactionBottom = layerChemicalCompaction ( K, J, I );

          if ( porosityBottom == 0.0 || porosityBottom == CauldronNoDataValue ) {
            porosityBottom = porosityTop;
          }

          if ( temperatureBottom == CauldronNoDataValue ) {
             double estimatedRealThickness = 0.5 * solidThickness * ( 1.0 / ( 1.0 - porosityBottom ) + 1.0 / ( 1.0 - porosityTop ));
             temperatureBottom = cauldron->Estimate_Temperature_At_Depth ( depthTop + estimatedRealThickness, surfaceTemperature, surfaceDepth );
          }

          if ( temperatureTop == CauldronNoDataValue ) {
            temperatureTop = cauldron->Estimate_Temperature_At_Depth ( depthTop, surfaceTemperature, surfaceDepth );
          }

          computeRealThickness ( currentLayer,
                                 currentLithology,
                                 includeChemicalCompaction,
                                 solidThickness,
                                 temperatureTop,
                                 temperatureBottom,
                                 Overpressure_Top,
                                 overpressureBottom,
                                 intermediateMaxVESTop,
                                 intermediateMaxVESBottom,
                                 VESTop,
                                 maxVESTop,
                                 porosityTop,
                                 chemicalCompactionBottom,
                                 porosityBottom,
                                 hydrostaticPressureTop,
                                 porePressureTop,
                                 porePressureBottom,
                                 lithostaticPressureTop,
                                 fluidDensityTop,
                                 bulkDensityTop,
                                 realThickness );

          layerThickness = layerThickness + realThickness;

          // Copy bottom values, they are for the top of the next segment!
          temperatureTop  = temperatureBottom;
          porePressureTop = porePressureBottom;
          porosityTop     = porosityBottom;
          depthTop        = depthTop + realThickness;

          // Copy computed values back to the PETSc arrays
          layerDepth          ( K, J, I ) = depthTop; // This is the bottom depth of the segment (depth_top + thickness)
          hydrostaticPressure ( K, J, I ) = hydrostaticPressureTop;
          lithostaticPressure ( K, J, I ) = lithostaticPressureTop;
          porePressure        ( K, J, I ) = porePressureTop;
          layerPorosity       ( K, J, I ) = porosityTop;
          VES                 ( K, J, I ) = VESTop;
          intermediateMaxVES  ( K, J, I ) = maxVESTop;

          // The _Top suffix here MUST be ignored, it is really the bottom of the segment,
          // the variables have the _Top suffix because on entry to Compute_realThickness
          // the variables store the properties at the top of the segment, on exit they
          // hold values at the bottom.
          currentLithology->getPorosity ( VESTop, maxVESTop, includeChemicalCompaction, chemicalCompactionBottom, porosityMixture );
          currentLithology->calcBulkPermeabilityNP ( VESTop, maxVESTop, porosityMixture,
                                                     permeabilityNormalValue,
                                                     permeabilityPlaneValue );

          layerPermeabilityNormal  ( K, J, I ) = permeabilityNormalValue / MilliDarcyToM2;
          layerPermeabilityPlane   ( K, J, I ) = permeabilityPlaneValue  / MilliDarcyToM2;
        }

      } else {

        for ( int K = zTopIndex; K >= 0; --K ) {
          layerDepth                ( K, J, I ) = CauldronNoDataValue;
          hydrostaticPressure       ( K, J, I ) = CauldronNoDataValue;
          lithostaticPressure       ( K, J, I ) = CauldronNoDataValue;
          porePressure              ( K, J, I ) = CauldronNoDataValue;
          layerPorosity             ( K, J, I ) = CauldronNoDataValue;
          VES                       ( K, J, I ) = CauldronNoDataValue;
          intermediateMaxVES        ( K, J, I ) = CauldronNoDataValue;
          layerPermeabilityNormal   ( K, J, I ) = CauldronNoDataValue;
          layerPermeabilityPlane    ( K, J, I ) = CauldronNoDataValue;
        }

      }

    }

  }

}

//------------------------------------------------------------//

void GeometricLoopPressureSolver::computeDependantProperties ( const double previousTime,
                                                               const double currentTime,
                                                               const bool   outputProperties ) {

   using namespace Basin_Modelling;

   Layer_Iterator FEM_Layers ( cauldron->layers, Descending, Sediments_Only, Active_Layers_Only );
   LayerProps_Ptr currentLayer;

   int Z_Top;

   const Boolean2DArray& Valid_Needle = cauldron->getValidNeedles ();

   int X_Start, Y_Start, Z_Start, X_Count, Y_Count, Z_Count;
   DMDAGetCorners ( *cauldron->mapDA, &X_Start, &Y_Start, PETSC_NULL, &X_Count, &Y_Count, PETSC_NULL );

   Double_Array_2D Hydrostatic_Pressure_Above ( X_Count, Y_Count );
   Double_Array_2D Pore_Pressure_Above        ( X_Count, Y_Count );
   Double_Array_2D Lithostatic_Pressure_Above ( X_Count, Y_Count );
   Double_Array_2D VES_Above                  ( X_Count, Y_Count );
   Double_Array_2D Max_VES_Above              ( X_Count, Y_Count );
   Double_Array_2D Depth_Above                ( X_Count, Y_Count );

   currentLayer = FEM_Layers.Current_Layer ();

   Initialise_Top_Depth ( cauldron, currentTime,
                          X_Start, X_Count,
                          Y_Start, Y_Count,
                          Depth_Above );

   Initialise_Top_Properties ( cauldron, currentLayer->fluid, currentTime,
                               X_Start, X_Count,
                               Y_Start, Y_Count,
                               Hydrostatic_Pressure_Above,
                               Pore_Pressure_Above,
                               Lithostatic_Pressure_Above,
                               VES_Above,
                               Max_VES_Above );

  while ( ! FEM_Layers.Iteration_Is_Done ()) {
    currentLayer = FEM_Layers.Current_Layer ();

    // Get the size of the layer DA.
    DMDAGetCorners ( currentLayer->layerDA, &X_Start, &Y_Start, &Z_Start, &X_Count, &Y_Count, &Z_Count );
    Z_Top = Z_Start + Z_Count - 1;

    // Fundamental Properties
    currentLayer->Current_Properties.Activate_Property  ( Basin_Modelling::Solid_Thickness );
    currentLayer->Previous_Properties.Activate_Property ( Basin_Modelling::Solid_Thickness );

    PETSC_3D_Array Layer_Depth          ( currentLayer->layerDA, currentLayer->Current_Properties ( Basin_Modelling::Depth ));
    PETSC_3D_Array Hydrostatic_Pressure ( currentLayer->layerDA, currentLayer->Current_Properties ( Basin_Modelling::Hydrostatic_Pressure ));
    PETSC_3D_Array Lithostatic_Pressure ( currentLayer->layerDA, currentLayer->Current_Properties ( Basin_Modelling::Lithostatic_Pressure ));
    PETSC_3D_Array Pore_Pressure        ( currentLayer->layerDA, currentLayer->Current_Properties ( Basin_Modelling::Pore_Pressure ));
    PETSC_3D_Array Overpressure         ( currentLayer->layerDA, currentLayer->Current_Properties ( Basin_Modelling::Overpressure ));
    PETSC_3D_Array VES                  ( currentLayer->layerDA, currentLayer->Current_Properties ( Basin_Modelling::VES_FP ));
    PETSC_3D_Array Intermediate_Max_VES ( currentLayer->layerDA, currentLayer->Current_Properties ( Basin_Modelling::Max_VES ));
    PETSC_3D_Array Temperature          ( currentLayer->layerDA, currentLayer->Current_Properties ( Basin_Modelling::Temperature ));
    PETSC_3D_Array Chemical_Compaction  ( currentLayer->layerDA, currentLayer->Current_Properties ( Basin_Modelling::Chemical_Compaction ));

    PETSC_3D_Array Max_VES              ( currentLayer->layerDA, currentLayer->Previous_Properties ( Basin_Modelling::Max_VES ));

    // Dependant Properties
    PETSC_3D_Array Layer_Porosity            ( currentLayer->layerDA, currentLayer->Porosity );
    PETSC_3D_Array Layer_Permeability_Normal ( currentLayer->layerDA, currentLayer->PermeabilityV );
    PETSC_3D_Array Layer_Permeability_Plane  ( currentLayer->layerDA, currentLayer->PermeabilityH );

    // Copy the bottom of the PETSc_Arrays to the *_Above arrays for
    // the top of the next layer.
    initialiseTopNodes ( X_Start, X_Count, Y_Start, Y_Count, Z_Top, Valid_Needle, Hydrostatic_Pressure_Above, Hydrostatic_Pressure );
    initialiseTopNodes ( X_Start, X_Count, Y_Start, Y_Count, Z_Top, Valid_Needle, Pore_Pressure_Above, Pore_Pressure );
    initialiseTopNodes ( X_Start, X_Count, Y_Start, Y_Count, Z_Top, Valid_Needle, Lithostatic_Pressure_Above, Lithostatic_Pressure );
    initialiseTopNodes ( X_Start, X_Count, Y_Start, Y_Count, Z_Top, Valid_Needle, VES_Above, VES );
    initialiseTopNodes ( X_Start, X_Count, Y_Start, Y_Count, Z_Top, Valid_Needle, Depth_Above, Layer_Depth );

    // Compute the values for the layer!
    computeDependantPropertiesForLayer ( currentLayer,
                                         previousTime,
                                         currentTime,
                                         Layer_Depth,
                                         Hydrostatic_Pressure,
                                         Overpressure,
                                         Pore_Pressure,
                                         Lithostatic_Pressure,
                                         VES,
                                         Intermediate_Max_VES,
                                         Max_VES,
                                         Layer_Porosity,
                                         Layer_Permeability_Normal,
                                         Layer_Permeability_Plane,
                                         Temperature,
                                         Chemical_Compaction );

    // Now, save the bottom of the arrays for the top of the next layer.
    copyBottomNodes ( X_Start, X_Count, Y_Start, Y_Count, Valid_Needle, Hydrostatic_Pressure_Above, Hydrostatic_Pressure );
    copyBottomNodes ( X_Start, X_Count, Y_Start, Y_Count, Valid_Needle, Pore_Pressure_Above, Pore_Pressure );
    copyBottomNodes ( X_Start, X_Count, Y_Start, Y_Count, Valid_Needle, Lithostatic_Pressure_Above, Lithostatic_Pressure );
    copyBottomNodes ( X_Start, X_Count, Y_Start, Y_Count, Valid_Needle, VES_Above, VES );
    copyBottomNodes ( X_Start, X_Count, Y_Start, Y_Count, Valid_Needle, Depth_Above, Layer_Depth );

    currentLayer->Current_Properties.Restore_Property  ( Basin_Modelling::Solid_Thickness );
    currentLayer->Previous_Properties.Restore_Property ( Basin_Modelling::Solid_Thickness );

    FEM_Layers++;
  }

  if ( cauldron -> Do_Iteratively_Coupled ) {
     setBasementDepths ( currentTime, Depth_Above, Valid_Needle );
  }

}

//------------------------------------------------------------//

void GeometricLoopPressureSolver::initialisePressureProperties ( const double previousTime,
                                                                 const double currentTime ) {

  using namespace Basin_Modelling;

  int X_Start, Y_Start, Z_Start, X_Count, Y_Count, Z_Count;

  double Solid_Thickness_Value;

  Basin_Modelling::Layer_Iterator Layers ( cauldron->layers, Ascending, Sediments_Only, Active_Layers_Only );
  LayerProps_Ptr currentLayer;
  const Boolean2DArray& Valid_Needle = cauldron->getValidNeedles ();

  for ( Layers.Initialise_Iterator (); ! Layers.Iteration_Is_Done (); Layers++ ) {
    currentLayer = Layers.Current_Layer ();

    DMDAGetCorners( currentLayer->layerDA, &X_Start, &Y_Start, &Z_Start, &X_Count, &Y_Count, &Z_Count);

    PETSC_3D_Array Solid_Thickness ( currentLayer->layerDA, currentLayer->Current_Properties ( Basin_Modelling::Solid_Thickness ));

    const int lastX = X_Start + X_Count;
    const int lastY = Y_Start + Y_Count;
    const int lastZ = Z_Start + Z_Count;
    for ( int I = X_Start; I < lastX; ++I ) {

      for ( int J = Y_Start; J < lastY; ++J ) {

        if ( Valid_Needle ( I, J )) {

          for ( int K = Z_Start; K < lastZ; ++K ) {

            if ( K != lastZ - 1 ) {
              Solid_Thickness_Value = currentLayer->getSolidThickness ( I, J, K, currentTime );

              if ( Solid_Thickness_Value != IbsNoDataValue ) {
                Solid_Thickness ( K, J, I ) = Solid_Thickness_Value;
              } else {
                Solid_Thickness ( K, J, I ) = 0.0;
              }

            }

          }

        }

      }

    }

  }

}

//------------------------------------------------------------//

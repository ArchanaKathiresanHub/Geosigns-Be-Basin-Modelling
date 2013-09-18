#include "NonGeometricLoopPressureSolver.h"

#include "FiniteElementTypes.h"
#include "NumericFunctions.h"

#include "FastcauldronSimulator.h"
#include "HydraulicFracturingManager.h"

#include "element_contributions.h"
#include "layer_iterators.h"
#include "fem_grid_auxiliary_functions.h"


//------------------------------------------------------------//

NonGeometricLoopPressureSolver::NonGeometricLoopPressureSolver ( AppCtx* appl ) : PressureSolver ( appl ) {
}

//------------------------------------------------------------//

void NonGeometricLoopPressureSolver::adjustSolidThickness ( const double relativeThicknessTolerance,
                                                            const double absoluteThicknessTolerance,
                                                                  bool&  geometryHasConverged ) {

   using namespace Basin_Modelling;
  
  Layer_Iterator Pressure_Layers ( cauldron->layers, Descending, Sediments_Only, Active_Layers_Only );
  LayerProps_Ptr Current_Layer;

  double segmentFCT;
  double FCT_Scaling; 
  double Initial_FCT;
  double Computed_Solid_Thickness;
  int I;
  int J;
  int K;
  int xStart;
  int yStart;
  int zStart;
  int xCount;
  int yCount;
  int zCount;

  int Layer_Count = 0;

  DMDAGetGhostCorners ( *cauldron->mapDA, &xStart, &yStart, PETSC_NULL, &xCount, &yCount, PETSC_NULL );

  PETSc_Local_2D_Array<double> FCT_Scaling_Factors;

  FCT_Scaling_Factors.create ( *cauldron->mapDA );

  while ( ! Pressure_Layers.Iteration_Is_Done ()) {
    Current_Layer = Pressure_Layers.Current_Layer ();
    FCT_Scaling_Factors.fill ( 0.0 );
    //
    //
    // Get the size of the layer DA.
    //
    DMDAGetGhostCorners ( Current_Layer->layerDA, &xStart, &yStart, &zStart, &xCount, &yCount, &zCount );
    DMDALocalInfo Local_Info;

    DMDAGetLocalInfo ( *cauldron->mapDA, &Local_Info );

    PETSC_2D_Array FCTCorrection;
    FCTCorrection.Set_Global_Array( *cauldron -> mapDA, Current_Layer -> FCTCorrection, 
				    INSERT_VALUES, true );

    PETSC_2D_Array Thickness_Error;
    Thickness_Error.Set_Global_Array( *cauldron -> mapDA, Current_Layer -> Thickness_Error, 
				      INSERT_VALUES, true );
    //
    //
    //  Computed_Depths:                 Present day depths, as computed by the pressure calculator
    //  Present_Day_Eroded_Thicknesses:  Input thicknesses, but with any erosion taken into account
    //  Deposited_Thicknesses:           Computed thicknesses at deposition
    //  Present_Day_Thicknesses:         Input thicknesses as they appear in the strat table (ie with no erosion taken into account)
    //
    Current_Layer -> Current_Properties.Activate_Property ( Basin_Modelling::Solid_Thickness, INSERT_VALUES, true );

    for ( I = xStart; I < xStart + xCount; I++ ) {

      for ( J = yStart; J < yStart + yCount; J++ ) {

        if ( cauldron->nodeIsDefined ( I, J )) {

          Computed_Solid_Thickness = 0.0;
          Initial_FCT = 0.0;

          for ( K = 0; K < zCount - 2; K++ ) {
            Computed_Solid_Thickness = Computed_Solid_Thickness + Current_Layer -> Current_Properties ( Basin_Modelling::Solid_Thickness, K, J, I );
            segmentFCT = Current_Layer->getSolidThickness ( I, J, K, 0.0 );

            if ( segmentFCT != CAULDRONIBSNULLVALUE && segmentFCT != IBSNULLVALUE ) {
              Initial_FCT = Initial_FCT + segmentFCT;
            }

          }

          // Is 1.0e-10 the best value here? Perhaps a larger value would be better.
          if ( fabs ( Initial_FCT ) > 1.0e-10 && fabs ( Computed_Solid_Thickness ) > 1.0e-10 ) {
            FCT_Scaling = Computed_Solid_Thickness / Initial_FCT;
          } else {
            FCT_Scaling = 1.0;
          }

          FCT_Scaling_Factors ( I, J ) = FCTCorrection ( J, I ) * FCT_Scaling;
	  FCTCorrection ( J, I ) = FCTCorrection ( J, I ) * FCT_Scaling;

          if ( Computed_Solid_Thickness == 0.0 ) {
            //
            //
            // Not sure what number should be assigned here.
            //
            //      i) 0.0 is probably the most intelligent value, because if there 
            //         is no computed_solid_thickness then there can be no error
            //     ii) CAULDRONIBSNULLVALUE (=99999)
            //    iii) nan/inf
            //     iv) some other value
            //
            Thickness_Error ( J, I ) = 0.0;
          } else {

            //
            // Scale by 100 to make a percentage.
            //
            Thickness_Error ( J, I ) = fabs (( Initial_FCT - Computed_Solid_Thickness ) / Computed_Solid_Thickness ) * 100.0;
          }

          for ( K = zStart; K < zStart + zCount - 1; K++ ) {
            Current_Layer->getSolidThickness ( I, J, K ).ScaleBy ( FCT_Scaling );
          }

        } else {
	  FCTCorrection   ( J, I ) = CAULDRONIBSNULLVALUE;
	  Thickness_Error ( J, I ) = CAULDRONIBSNULLVALUE;
        }

      }

    }

    Current_Layer -> Current_Properties.Restore_Property ( Basin_Modelling::Solid_Thickness );

    FCTCorrection.Restore_Global_Array ( Update_Excluding_Ghosts );
    Thickness_Error.Restore_Global_Array ( Update_Excluding_Ghosts );

    Pressure_Layers++;
    Layer_Count++;
  }

  //
  // This has to be the case, as we are driving the computation by the 
  // input real thicknesses and not the FCT. So the final thicknesses
  // MUST be the input thicknesses!!
  //
  geometryHasConverged = true;
}

//------------------------------------------------------------//

//
//
// On entry the parameters Hydrostatic_Pressure, Pore_Pressure, ..., Bulk_Density have the values
// at the top of the segment. On exit they contain values at the bottom of the segment.
//
void NonGeometricLoopPressureSolver::computeSolidThickness ( const LayerProps_Ptr Current_Layer,
                                                             const CompoundLithology*  Current_Lithology,
                                                             const bool           Include_Chemical_Compaction,
                                                             const double         Real_Thickness,
                                                             const double         lateralStressFactor,
                                                             const double         Temperature_Bottom,
                                                             const double         Overpressure_Bottom,

                                                             const double         Intermediate_Max_VES_Bottom,
                                                                   double&        VES,
                                                                   double&        Max_VES,

                                                             const double         Porosity_Top,
                                                             const double         Chemical_Compaction_Bottom,
                                                                   double&        Porosity_Bottom,

                                                                   double&        Hydrostatic_Pressure,
                                                                   double&        Pore_Pressure,
                                                                   double&        Lithostatic_Pressure,

                                                                   double&        Fluid_Density,
                                                                   double&        Bulk_Density,
                                                                   double&        Solid_Thickness ) const {

  //
  // As the values are at the top on entry into the function
  // a copy is made, so that when they are overwritten we still 
  // have them.
  //
  const double Hydrostatic_Pressure_Top = Hydrostatic_Pressure;
  const double Pore_Pressure_Top        = Pore_Pressure;
  const double Lithostatic_Pressure_Top = Lithostatic_Pressure;

  const double Fluid_Density_Top        = Fluid_Density;
  const double Bulk_Density_Top         = Bulk_Density;
  const double Solid_Density            = Current_Lithology->density();

  double Solid_Thickness_New;
  double Hydrostatic_Pressure_Predicted;
  double Pore_Pressure_Predicted;

  int  Iteration_Count = 0;
  bool Converged = false;
  //
  //
  // Initial estimate of real thickness (based on initial estimate of bottom porosity)
  //
  Solid_Thickness = 0.5 * Real_Thickness * ( 2.0 - Porosity_Top - Porosity_Bottom );

  //
  // New Hydrostatic pressure computation needed perhaps, RK4 type scheme?
  //
  // make a prediction of the fluid density (and hence hydrostatic and pore pressure), based on the
  // pore pressure at the top of the segment.
  //
  Fluid_Density = Current_Layer->fluid->density ( Temperature_Bottom, Pore_Pressure_Top );
  Hydrostatic_Pressure_Predicted = Hydrostatic_Pressure_Top
                                   + 0.5 * GRAVITY * Real_Thickness * ( Fluid_Density_Top + Fluid_Density ) * Pa_To_MPa;

  Pore_Pressure_Predicted = Hydrostatic_Pressure_Predicted + Overpressure_Bottom;

  //
  // Correct the predicted fluid density and hence the hydrostatic and pore pressure
  //
  Fluid_Density = Current_Layer->fluid->density ( Temperature_Bottom, Pore_Pressure_Predicted );

  Hydrostatic_Pressure = Hydrostatic_Pressure_Top
                         + 0.5 * GRAVITY * Real_Thickness * ( Fluid_Density_Top + Fluid_Density ) * Pa_To_MPa;

  Pore_Pressure = Hydrostatic_Pressure + Overpressure_Bottom;

  while ( ! Converged ) {

 //   Fluid_Density = Current_Layer->fluid->density ( Temperature_Bottom, Pore_Pressure_Predicted );
 
    Bulk_Density = Porosity_Bottom * Fluid_Density + ( 1.0 - Porosity_Bottom ) * Solid_Density;




    //
    // Integrate both hydrostatic and lithostatic pressures using a trapezoidal method.
    // This should be okay, as both are almost linear throughout the height of the segment.
    //
    Lithostatic_Pressure = Lithostatic_Pressure_Top
                           + 0.5 * GRAVITY * Real_Thickness * ( Bulk_Density_Top + Bulk_Density ) * Pa_To_MPa;

    //
    // What to do here? Should we keep VES >= 0?
    //
    // Should we rely on the fracture pressure to keep VES to sensible values (ie nonnegative)?
    //
    VES = ( Lithostatic_Pressure - Pore_Pressure ) * MPa_To_Pa;
    VES     = NumericFunctions::Maximum ( 0.0, ( Lithostatic_Pressure - Pore_Pressure ) * MPa_To_Pa );
    Max_VES = NumericFunctions::Maximum (( 1.0 + lateralStressFactor ) * VES, Intermediate_Max_VES_Bottom );

    Porosity_Bottom = Current_Lithology->porosity ( VES, Max_VES, Include_Chemical_Compaction, Chemical_Compaction_Bottom );

    //
    // The solid thickness is determined by the ODE:
    //
    //
    //  ds
    //  -- = 1 - phi
    //  dz
    //
    //
    // Here we integrate this ODE to get the solid thickness from the computed overpressure.
    //
    Solid_Thickness_New = 0.5 * Real_Thickness * ( 2.0 - Porosity_Top - Porosity_Bottom );

    // Need to change tolerance here.
    if ( fabs ( Solid_Thickness_New ) > PressureSolver::GeometricLoopThicknessTolerance ) {
      Converged = fabs (( Solid_Thickness_New - Solid_Thickness ) / Solid_Thickness_New ) < GeometricLoopThicknessTolerance;
    } else {
      Converged = fabs (( Solid_Thickness_New - Solid_Thickness )) < GeometricLoopThicknessTolerance;
    }

    Solid_Thickness = Solid_Thickness_New;
    Iteration_Count = Iteration_Count + 1;
  }

//Pore_Pressure = Pore_Pressure_Predicted;



/*    if ( (Fluid_Density > Solid_Density) && ( Current_Layer->fluid->SwitchPermafrost() ) )  // NLSAY3: We assume the solid is ice in this case
    {
    Hydrostatic_Pressure = Hydrostatic_Pressure_Top
                         + 0.5 * GRAVITY * Real_Thickness * ( Bulk_Density_Top + Bulk_Density ) * Pa_To_MPa;
    Pore_Pressure = Hydrostatic_Pressure + Overpressure_Bottom;
    }*/


}

//------------------------------------------------------------//

void NonGeometricLoopPressureSolver::computeDependantPropertiesForLayer
   ( const LayerProps_Ptr  currentLayer, 
     const double          previousTime, 
     const double          currentTime, 
           PETSC_3D_Array& previousLayerDepth,
           PETSC_3D_Array& currentlayerDepth,
           PETSC_3D_Array& layerFCT,
           PETSC_3D_Array& layerHydrostaticPressure,
           PETSC_3D_Array& layerOverpressure,
           PETSC_3D_Array& layerPorePressure,
           PETSC_3D_Array& layerLithostaticPressure,
           PETSC_3D_Array& layerVES,
           PETSC_3D_Array& layerIntermediateMaxVES,
           PETSC_3D_Array& layerMaxVES,
           PETSC_3D_Array& layerPorosity,
           PETSC_3D_Array& layerPermeabilityNormal,
           PETSC_3D_Array& layerPermeabilityPlane,
           PETSC_3D_Array& layerTemperature,
           PETSC_3D_Array& layerChemicalCompaction ) {


  const bool includeChemicalCompaction = (( cauldron->Do_Chemical_Compaction ) && ( currentLayer -> Get_Chemical_Compaction_Mode ()));

  int I, J, K;
  int IX, JY;
  int X_Start;
  int Y_Start;
  int Z_Start;
  int X_Count;
  int Y_Count;
  int Z_Count;
  int zTopIndex;

  const CompoundLithology*  currentLithology;

  double solidDensity;
  double Fluid_Density;
  double Bulk_Density;

  double overpressureBottom;
  double hydrostaticPressure;
  double lithostaticPressure;
  double porePressure;
  double temperatureTop;
  double temperatureBottom;
  double surfaceDepth;

  // Initialised to stop compiler from complaining, the initialisation is okay,
  // since the surfaceTemperature WILL be assigned to before reading.
  double surfaceTemperature = 0.0;
  double VESValue;
  double maxVESValue;
  double maxVESTop;
  double maxVESBottom;
  double intermediateMaxVESBottom;
  double estimatedMaxVES;
  double depthTop;
  double Depth_Bottom;
  double realThickness;
  double solidThickness;
  double previousRealThickness;
  double currentRealThickness;
  double erodedRealThickness;
  double porosityTop;
  double porosityBottom;

  double chemicalCompactionTop;
  double chemicalCompactionBottom;

  CompoundProperty porosityMixture;

  double permeabilityNormalValue;
  double permeabilityPlaneValue;

  int currentTopmostActiveSegment;
  int needleTopActiveNode;
  bool  layerIsMobile = currentLayer->isMobile ();

  DMDAGetCorners ( currentLayer->layerDA, &X_Start, &Y_Start, &Z_Start, &X_Count, &Y_Count, &Z_Count );
  zTopIndex = Z_Start + Z_Count - 1;

  double lateralStressFactor = FastcauldronSimulator::getInstance ().lateralStressFactor ( currentTime );

  for ( I = X_Start, IX = 0; I < X_Start + X_Count; I++, IX++ ) {

    for ( J = Y_Start, JY = 0; J < Y_Start + Y_Count; J++, JY++ ) {

      if ( cauldron->nodeIsDefined ( I, J )) {

        currentLithology = currentLayer->getLithology ( I, J );

        temperatureTop = layerTemperature ( zTopIndex, J, I );
        surfaceDepth   = FastcauldronSimulator::getInstance ().getSeaBottomDepth ( I, J, currentTime );
        depthTop       = currentlayerDepth ( zTopIndex, J, I );

        if ( temperatureTop == CAULDRONIBSNULLVALUE ) {
          surfaceTemperature = FastcauldronSimulator::getInstance ().getSeaBottomTemperature ( I, J, currentTime );
          temperatureTop     = cauldron->Estimate_Temperature_At_Depth ( depthTop, surfaceTemperature, surfaceDepth );
        }

        hydrostaticPressure = layerHydrostaticPressure ( zTopIndex, J, I );
        lithostaticPressure = layerLithostaticPressure ( zTopIndex, J, I );
        porePressure        = layerPorePressure ( zTopIndex, J, I );

        // Pressure constrained here
        porePressure        = NumericFunctions::Minimum ( porePressure, lithostaticPressure );
        VESValue            = layerVES ( zTopIndex, J, I );

        solidDensity = currentLithology -> density ();

        currentTopmostActiveSegment  = currentLayer -> Current_Topmost_Segments ( I, J );

        if ( currentTopmostActiveSegment == -1 ) {
          // Should not really be in here! In the exceptional case where we do enter 
          // this then copy maxVESTop from the current top of the layer.
          //
          // Explain why we should not be in here, and why we may be in here!!!
          maxVESTop = layerMaxVES ( zTopIndex, J, I );
          maxVESTop = NumericFunctions::Maximum ( layerMaxVES ( zTopIndex, J, I ), ( 1.0 + lateralStressFactor ) * VESValue );

          needleTopActiveNode = Z_Start + Z_Count - 1;
        } else {

          previousRealThickness = previousLayerDepth ( currentTopmostActiveSegment,     J, I ) - 
                                  previousLayerDepth ( currentTopmostActiveSegment + 1, J, I );

          currentRealThickness  = currentlayerDepth ( currentTopmostActiveSegment,     J, I ) - 
                                  currentlayerDepth ( currentTopmostActiveSegment + 1, J, I );

          // previousRealThickness = previousLayerDepth ( currentTopmostActiveSegment + 1, J, I ) - 
          //                         previousLayerDepth ( currentTopmostActiveSegment,     J, I );

          // currentRealThickness  = currentlayerDepth ( currentTopmostActiveSegment + 1, J, I ) - 
          //                         currentlayerDepth ( currentTopmostActiveSegment,     J, I );

          if ( previousRealThickness < 0.0 || previousRealThickness == IBSNULLVALUE ) {
            previousRealThickness = 0.0;
          }

          if ( currentRealThickness < 0.0 || currentRealThickness == IBSNULLVALUE ) {
            currentRealThickness = 0.0;
          }

          if ( not layerIsMobile and previousRealThickness > currentRealThickness ) {

            //
            // Layer is eroding.
            //
            // As the layer is eroding then:
            //
            //      i) Must interpolate the Max VES at the top of the layer
            //     ii) Set the properties, which depend on the Max VES, at the top of the layer to be those 
            //         of interpolated Max VES. So set the Top index to be the top index of the layer.
            //
            erodedRealThickness = previousRealThickness - currentRealThickness;

            maxVESTop    = NumericFunctions::Maximum ( layerMaxVES ( currentTopmostActiveSegment + 1, J, I ), ( 1.0 + lateralStressFactor ) * VESValue );
            maxVESBottom = NumericFunctions::Maximum ( layerMaxVES ( currentTopmostActiveSegment,     J, I ), ( 1.0 + lateralStressFactor ) * layerVES ( currentTopmostActiveSegment, J, I ));

            estimatedMaxVES = ( maxVESBottom - maxVESTop ) * ( erodedRealThickness / previousRealThickness ) + maxVESTop;

            maxVESTop = estimatedMaxVES;
            needleTopActiveNode = Z_Start + Z_Count - 1;
          } else {

             // Layer is NOT eroding.
             maxVESTop = NumericFunctions::Maximum ( layerMaxVES ( zTopIndex, J, I ), ( 1.0 + lateralStressFactor ) * VESValue );
             needleTopActiveNode = Z_Start + Z_Count - 1;
          }

        }

        chemicalCompactionTop = layerChemicalCompaction ( Z_Start + Z_Count - 1, J, I );
        porosityTop = currentLithology->porosity ( VESValue, maxVESTop, includeChemicalCompaction, chemicalCompactionTop );
        layerPorosity ( Z_Start + Z_Count - 1, J, I ) = porosityTop;

        Fluid_Density = currentLayer->fluid->density ( temperatureTop, porePressure );
        Bulk_Density  = porosityTop * Fluid_Density + ( 1.0 - porosityTop ) * solidDensity;

        // Compute the permeability and set the vectors accordingly.
        currentLithology->getPorosity ( VESValue, maxVESTop, includeChemicalCompaction, chemicalCompactionTop, porosityMixture );
        currentLithology->calcBulkPermeabilityNP ( VESValue, maxVESTop, porosityMixture,
                                                    permeabilityNormalValue,
                                                    permeabilityPlaneValue );

        layerPermeabilityNormal  ( Z_Start + Z_Count - 1, J, I ) = permeabilityNormalValue / MILLIDARCYTOM2;
        layerPermeabilityPlane   ( Z_Start + Z_Count - 1, J, I ) = permeabilityPlaneValue  / MILLIDARCYTOM2;

        // Initialise properties on all inactive nodes and the top most 
        // active node to be the the value at the top of the layer.
        for ( K = needleTopActiveNode; K >= currentTopmostActiveSegment + 1; K-- ) {
          layerFCT                 ( K, J, I ) = 0.0;
          layerHydrostaticPressure ( K, J, I ) = hydrostaticPressure;
          layerLithostaticPressure ( K, J, I ) = lithostaticPressure;
          layerPorePressure        ( K, J, I ) = porePressure;
          layerPorosity            ( K, J, I ) = porosityTop;
          layerVES                 ( K, J, I ) = VESValue;
          layerIntermediateMaxVES  ( K, J, I ) = maxVESTop;
        }

        for ( K = currentTopmostActiveSegment; K >= 0; K-- ) {

          intermediateMaxVESBottom = layerMaxVES       ( K, J, I );
          Depth_Bottom             = currentlayerDepth ( K, J, I );
          temperatureBottom        = layerTemperature   ( K, J, I );
          overpressureBottom       = layerOverpressure  ( K, J, I );
          chemicalCompactionBottom = layerChemicalCompaction ( K, J, I );

          realThickness = Depth_Bottom - depthTop;

          // Fix this, put correct value for VES
          // Only used to initialise anyway, so not so important
          porosityBottom = currentLithology->porosity ( intermediateMaxVESBottom,
                                                        intermediateMaxVESBottom,
                                                        includeChemicalCompaction,
                                                        chemicalCompactionBottom );

          if ( temperatureBottom == CAULDRONIBSNULLVALUE ) {
            temperatureBottom = cauldron->Estimate_Temperature_At_Depth ( Depth_Bottom, surfaceTemperature, surfaceDepth );
          }

          computeSolidThickness ( currentLayer, currentLithology,
                                  includeChemicalCompaction,
                                  realThickness, 
                                  lateralStressFactor,
                                  temperatureBottom,
                                  overpressureBottom,
                                  intermediateMaxVESBottom,
                                  VESValue, 
                                  maxVESValue,
                                  porosityTop,
                                  chemicalCompactionBottom,
                                  porosityBottom,
                                  hydrostaticPressure,
                                  porePressure,
                                  lithostaticPressure,
                                  Fluid_Density,
                                  Bulk_Density,
                                  solidThickness );

          // Copy bottom values, they are for the top of the next segment!
          depthTop       = Depth_Bottom;
          temperatureTop = temperatureBottom;
          porosityTop    = porosityBottom;

          // Copy computed values back to the PETSc arrays
          layerFCT                 ( K, J, I ) = solidThickness;
          layerHydrostaticPressure ( K, J, I ) = hydrostaticPressure;
          layerLithostaticPressure ( K, J, I ) = lithostaticPressure;
          layerPorePressure        ( K, J, I ) = porePressure;

          layerPorosity            ( K, J, I ) = porosityBottom;

          layerVES                 ( K, J, I ) = VESValue;
          layerIntermediateMaxVES  ( K, J, I ) = maxVESValue;

          currentLithology->getPorosity ( VESValue, maxVESValue, includeChemicalCompaction, chemicalCompactionBottom, porosityMixture );
          currentLithology->calcBulkPermeabilityNP ( VESValue, maxVESValue, porosityMixture,
                                                     permeabilityNormalValue,
                                                     permeabilityPlaneValue );

          layerPermeabilityNormal  ( K, J, I ) = permeabilityNormalValue / MILLIDARCYTOM2;
          layerPermeabilityPlane   ( K, J, I ) = permeabilityPlaneValue  / MILLIDARCYTOM2;

        }

      } else {

        for ( K = zTopIndex; K >= 0; K-- ) {
          currentlayerDepth        ( K, J, I ) = CAULDRONIBSNULLVALUE;
          layerFCT                 ( K, J, I ) = CAULDRONIBSNULLVALUE;
          layerHydrostaticPressure ( K, J, I ) = CAULDRONIBSNULLVALUE;
          layerLithostaticPressure ( K, J, I ) = CAULDRONIBSNULLVALUE;
          layerPorePressure        ( K, J, I ) = CAULDRONIBSNULLVALUE;
          layerPorosity            ( K, J, I ) = CAULDRONIBSNULLVALUE;
          layerVES                 ( K, J, I ) = CAULDRONIBSNULLVALUE;
          layerIntermediateMaxVES  ( K, J, I ) = CAULDRONIBSNULLVALUE;
          layerPermeabilityNormal  ( K, J, I ) = CAULDRONIBSNULLVALUE;
          layerPermeabilityPlane   ( K, J, I ) = CAULDRONIBSNULLVALUE;
        }

      }

    }

  }

}


//------------------------------------------------------------//

void NonGeometricLoopPressureSolver::computeDependantProperties ( const double Previous_Time, 
                                                                  const double Current_Time, 
                                                                  const bool   Output_Properties ) {


   using namespace Basin_Modelling;

  MPI_Barrier(PETSC_COMM_WORLD);

  #if defined (sgi)
     unsigned long Old_Flags = cout.flags ( ios::scientific );
  #else
     ios::fmtflags Old_Flags = cout.flags ( ios::scientific );
  #endif

  int Old_Precision = cout.precision ( 8 );

  Layer_Iterator FEM_Layers ( cauldron->layers, Descending, Sediments_Only, Active_Layers_Only );
  LayerProps_Ptr Current_Layer;

  int X_Start;
  int Y_Start;
  int Z_Start;
  int X_Count;
  int Y_Count;
  int Z_Count;
  int Z_Top;

  DMDAGetCorners ( *cauldron->mapDA, &X_Start, &Y_Start, PETSC_NULL, &X_Count, &Y_Count, PETSC_NULL );

  const Boolean2DArray& Valid_Needle = cauldron->getValidNeedles ();

  MPI_Barrier(PETSC_COMM_WORLD);

  Double_Array_2D Hydrostatic_Pressure_Above ( X_Count, Y_Count );
  Double_Array_2D Pore_Pressure_Above        ( X_Count, Y_Count );
  Double_Array_2D Lithostatic_Pressure_Above ( X_Count, Y_Count );
  Double_Array_2D VES_Above                  ( X_Count, Y_Count );
  Double_Array_2D Max_VES_Above              ( X_Count, Y_Count );
  Double_Array_2D Depth_Above                ( X_Count, Y_Count );

  Current_Layer = FEM_Layers.Current_Layer ();

  Initialise_Top_Properties ( cauldron, Current_Layer -> fluid, Current_Time,
                              X_Start, X_Count, 
                              Y_Start, Y_Count, 
                              Hydrostatic_Pressure_Above,
                              Pore_Pressure_Above,
                              Lithostatic_Pressure_Above,
                              VES_Above,
                              Max_VES_Above );

  while ( ! FEM_Layers.Iteration_Is_Done ()) {
    Current_Layer = FEM_Layers.Current_Layer ();

    // Get the size of the layer DA.
    DMDAGetCorners ( Current_Layer->layerDA, &X_Start, &Y_Start, &Z_Start, &X_Count, &Y_Count, &Z_Count );
    Z_Top = Z_Start + Z_Count - 1;

    // Fundamental Properties
    PETSC_3D_Array Layer_FCT            ( Current_Layer->layerDA, Current_Layer->Current_Properties ( Basin_Modelling::Solid_Thickness ));
    PETSC_3D_Array Layer_Depth          ( Current_Layer->layerDA, Current_Layer->Current_Properties ( Basin_Modelling::Depth ));
    PETSC_3D_Array Hydrostatic_Pressure ( Current_Layer->layerDA, Current_Layer->Current_Properties ( Basin_Modelling::Hydrostatic_Pressure ));
    PETSC_3D_Array Lithostatic_Pressure ( Current_Layer->layerDA, Current_Layer->Current_Properties ( Basin_Modelling::Lithostatic_Pressure ));
    PETSC_3D_Array Pore_Pressure        ( Current_Layer->layerDA, Current_Layer->Current_Properties ( Basin_Modelling::Pore_Pressure ));
    PETSC_3D_Array Overpressure         ( Current_Layer->layerDA, Current_Layer->Current_Properties ( Basin_Modelling::Overpressure ));
    PETSC_3D_Array VES                  ( Current_Layer->layerDA, Current_Layer->Current_Properties ( Basin_Modelling::VES_FP ));
    PETSC_3D_Array Intermediate_Max_VES ( Current_Layer->layerDA, Current_Layer->Current_Properties ( Basin_Modelling::Max_VES ));
    PETSC_3D_Array Temperature          ( Current_Layer->layerDA, Current_Layer->Current_Properties ( Basin_Modelling::Temperature ));
    PETSC_3D_Array Chemical_Compaction  ( Current_Layer->layerDA, Current_Layer->Current_Properties ( Basin_Modelling::Chemical_Compaction ));

    // Properties from previous time step
    PETSC_3D_Array Previous_Layer_Depth ( Current_Layer->layerDA, Current_Layer->Previous_Properties ( Basin_Modelling::Depth ));

    // The Max_VES array actually contains the Max_VES of the whole basin.
    // Intermediate_Max_VES (from current time step) contains the Max_VES computed at the current Newton iteration.
    PETSC_3D_Array Max_VES              ( Current_Layer->layerDA, Current_Layer->Previous_Properties ( Basin_Modelling::Max_VES ));

    // Dependant Properties
    PETSC_3D_Array Layer_Porosity            ( Current_Layer->layerDA, Current_Layer->Porosity );
    PETSC_3D_Array Layer_Permeability_Normal ( Current_Layer->layerDA, Current_Layer->PermeabilityV );
    PETSC_3D_Array Layer_Permeability_Plane  ( Current_Layer->layerDA, Current_Layer->PermeabilityH );

    // Copy values stored in the 2D arrays to top of current layer properties.
    initialiseTopNodes ( X_Start, X_Count, Y_Start, Y_Count, Z_Top, Valid_Needle, Hydrostatic_Pressure_Above, Hydrostatic_Pressure );
    initialiseTopNodes ( X_Start, X_Count, Y_Start, Y_Count, Z_Top, Valid_Needle, Pore_Pressure_Above, Pore_Pressure );
    initialiseTopNodes ( X_Start, X_Count, Y_Start, Y_Count, Z_Top, Valid_Needle, Lithostatic_Pressure_Above, Lithostatic_Pressure );
    initialiseTopNodes ( X_Start, X_Count, Y_Start, Y_Count, Z_Top, Valid_Needle, VES_Above, VES );

    // Compute the values for the layer!
    computeDependantPropertiesForLayer ( Current_Layer, 
                                         Previous_Time, 
                                         Current_Time, 
                                         Previous_Layer_Depth,
                                         Layer_Depth,
                                         Layer_FCT,
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

    // Copy from bottom of current layer properties.
    copyBottomNodes ( X_Start, X_Count, Y_Start, Y_Count, Valid_Needle, Hydrostatic_Pressure_Above, Hydrostatic_Pressure );
    copyBottomNodes ( X_Start, X_Count, Y_Start, Y_Count, Valid_Needle, Pore_Pressure_Above, Pore_Pressure );
    copyBottomNodes ( X_Start, X_Count, Y_Start, Y_Count, Valid_Needle, Lithostatic_Pressure_Above, Lithostatic_Pressure );
    copyBottomNodes ( X_Start, X_Count, Y_Start, Y_Count, Valid_Needle, VES_Above, VES );
    copyBottomNodes ( X_Start, X_Count, Y_Start, Y_Count, Valid_Needle, Depth_Above, Layer_Depth );

    FEM_Layers++;
  }

  if ( cauldron->Do_Iteratively_Coupled ) {
    setBasementDepths ( Current_Time, Depth_Above, Valid_Needle );
  }

  cout.flags ( Old_Flags );
  cout.precision ( Old_Precision );
}


//------------------------------------------------------------//

void NonGeometricLoopPressureSolver::initialisePressureProperties ( const double Previous_Time, 
                                                                    const double Current_Time ) {

   using namespace Basin_Modelling;

  int X_Start;
  int Y_Start;
  int Z_Start;

  int X_Count;
  int Y_Count;
  int Z_Count;

  int I;
  int J;
  int K;
  double Real_Thickness;


  Layer_Iterator Layers ( cauldron->layers, Descending, Sediments_Only, Active_Layers_Only );
  LayerProps_Ptr Current_Layer;
  const Boolean2DArray& Valid_Needle = cauldron->getValidNeedles ();

  DMDAGetCorners ( *cauldron->mapDA, &X_Start, &Y_Start, PETSC_NULL, &X_Count, &Y_Count, PETSC_NULL );

  Double_Array_2D Top_Depth ( X_Count, Y_Count );

  // Initialise Sea Bottom Depth
  for ( I = X_Start; I < X_Start + X_Count; I++ ) {

    for ( J = Y_Start; J < Y_Start + Y_Count; J++ ) {

      if ( Valid_Needle ( I, J )) {
        Top_Depth ( I - X_Start, J - Y_Start ) = FastcauldronSimulator::getInstance ().getSeaBottomDepth ( I, J, Current_Time );
      }

    }

  }

  for ( Layers.Initialise_Iterator (); ! Layers.Iteration_Is_Done (); Layers++ ) {
    Current_Layer = Layers.Current_Layer ();


    DMDAGetCorners( Current_Layer->layerDA, &X_Start, &Y_Start, &Z_Start, &X_Count, &Y_Count, &Z_Count);

    PETSC_3D_Array Depth ( Current_Layer->layerDA, Current_Layer->Current_Properties ( Basin_Modelling::Depth ));

    // Copy Top_Depth array for top of current layer (from either surface or bottom of previous layer)
    for ( I = X_Start; I < X_Start + X_Count; I++ ) {

      for ( J = Y_Start; J < Y_Start + Y_Count; J++ ) {

        if ( Valid_Needle ( I, J )) {
          Depth ( Z_Start + Z_Count - 1, J, I ) = Top_Depth ( I - X_Start, J - Y_Start );
        }

      }

    }

    // 
    for ( I = X_Start; I < X_Start + X_Count; I++ ) {

      for ( J = Y_Start; J < Y_Start + Y_Count; J++ ) {

        if ( Valid_Needle ( I, J )) {

          for ( K = Z_Start + Z_Count - 2; K >= 0; K-- ) {
            Real_Thickness = Current_Layer->getRealThickness ( I, J, K, Current_Time );

            if ( Real_Thickness != IBSNULLVALUE ) {
              Depth ( K, J, I ) = Depth ( K + 1, J, I ) + Real_Thickness;
            } else {
              Depth ( K, J, I ) = Depth ( K + 1, J, I );
            }

          }

        }

      }

    }

    // Copy bottom depth for top of next layer
    for ( I = X_Start; I < X_Start + X_Count; I++ ) {

      for ( J = Y_Start; J < Y_Start + Y_Count; J++ ) {

        if ( Valid_Needle ( I, J )) {
          Top_Depth ( I - X_Start, J - Y_Start ) = Depth ( 0, J, I );
        }

      }

    }

  }


}



//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include "PressureSolver.h"

#include "Interface/RunParameters.h"


#include "FiniteElementTypes.h"

#include "FastcauldronSimulator.h"
#include "HydraulicFracturingManager.h"

#include "element_contributions.h"
#include "layer_iterators.h"

#include "fem_grid_auxiliary_functions.h"

#include "Nodal3DIndexArray.h"
#include "MultiComponentFlowHandler.h"

#include "PetscLogStages.h"

#include "BoundaryConditions.h"
#include "ElementThicknessActivityPredicate.h"
#include "CompositeElementActivityPredicate.h"
#include "LayerElement.h"
#include "ElementContributions.h"
#include "Lithology.h"
#include "BoundaryId.h"

#include "PressureElementMatrixAssembly.h"

using namespace FiniteElementMethod;

// utilities library
#include "ConstantsMathematics.h"
using Utilities::Maths::Zero;
using Utilities::Maths::One;
using Utilities::Maths::NegOne;
#include "ConstantsNumerical.h"
using Utilities::Numerical::CauldronNoDataValue;


//------------------------------------------------------------//

const double PressureSolver::GeometricLoopThicknessTolerance = 0.001;


const int PressureSolver::DefaultMaximumPressureLinearSolverIterations = 1000;
const int PressureSolver::MaximumGMResRestartValue = 200;
const int PressureSolver::GMResRestartIncrementValue = 20;
const int PressureSolver::DefaultGMResRestartValue = 80;



int PressureSolver::PlaneQuadratureDegrees [ NumberOfOptimisationLevels ] = { 2, 2, 2, 2, 3 };

int PressureSolver::DepthQuadratureDegrees [ NumberOfOptimisationLevels ] = { 2, 2, 2, 3, 4 };

int PressureSolver::s_iterationsForiluFillLevelIncrease [ NumberOfOptimisationLevels ] = { 80, 80, 80, 80, 80 };

int PressureSolver::s_numberOfIterations [ NumberOfOptimisationLevels ] = { 8, 12, 15, 20, 30 };

double PressureSolver::NewtonSolverTolerances [ NumberOfOptimisationLevels ][ 3 ] = {{ 1.0e-2, 1.0e-2, 1.0e-2 },
                                                                                     { 1.0e-2, 1.0e-3, 1.0e-3 },
                                                                                     { 1.0e-2, 1.0e-3, 1.0e-3 },
                                                                                     { 5.0e-3, 1.0e-3, 1.0e-4 },
                                                                                     { 1.0e-4, 1.0e-4, 1.0e-5 }};


//------------------------------------------------------------//

PressureSolver::PressureSolver(AppCtx *appl) : cauldron ( appl ) {
  initialiseFctCorrection();
  basisFunctions = nullptr;
}


PressureSolver::~PressureSolver() {

   if ( basisFunctions != nullptr ) {
      delete basisFunctions;
      basisFunctions = nullptr;
   }

}

//------------------------------------------------------------//

void PressureSolver::initialiseFctCorrection () {

  using namespace Basin_Modelling;

  cauldron->timefilter.setFilter ( "FCTCorrection", "SedimentsOnly" );
  cauldron->timefilter.setFilter ( "ThicknessError", "SedimentsOnly" );

  LayerProps_Ptr Current_Layer;

  Layer_Iterator Layers ( cauldron->layers, Ascending, Sediments_Only,
           Active_And_Inactive_Layers );


  for ( Layers.Initialise_Iterator (); ! Layers.Iteration_Is_Done (); Layers++ )
  {
    Current_Layer = Layers.Current_Layer ();

    if ( ! cauldron->readFCTCorrectionFactor ) {
      DMCreateGlobalVector( *cauldron->mapDA, &Current_Layer -> FCTCorrection );
      VecSet( Current_Layer -> FCTCorrection, One );
    }

    DMCreateGlobalVector( *cauldron->mapDA, &Current_Layer -> Thickness_Error );
    VecSet( Current_Layer -> Thickness_Error, Zero );
  }

}

//------------------------------------------------------------//

void PressureSolver::checkPressureSolution () {

  using namespace Basin_Modelling;

  // Minimum amount of Overpressure, 1Pa (1.0e-6 Mpa)
  const double Minimum_Absolute_Overpressure_MPa = 1.0e-30;

  int xStart;
  int yStart;
  int zStart;
  int xCount;
  int yCount;
  int zCount;

  PetscScalar Overpressure_Value;
  PetscScalar Fracture_Pressure;

  Layer_Iterator Layers ( cauldron->layers, Ascending, Sediments_Only, Active_Layers_Only );
  LayerProps_Ptr Current_Layer;

  DMDAGetCorners ( *cauldron->mapDA, &xStart, &yStart, PETSC_NULL, &xCount, &yCount, PETSC_NULL );

  for ( Layers.Initialise_Iterator (); ! Layers.Iteration_Is_Done (); Layers++ ) {
    Current_Layer = Layers.Current_Layer ();


    DMDAGetCorners ( Current_Layer->layerDA, &xStart, &yStart, &zStart, &xCount, &yCount, &zCount );

    Current_Layer -> Current_Properties.Activate_Property ( Basin_Modelling::Overpressure );
    Current_Layer -> Current_Properties.Activate_Property ( Basin_Modelling::Pore_Pressure );
    Current_Layer -> Current_Properties.Activate_Property ( Basin_Modelling::Hydrostatic_Pressure );
    Current_Layer -> Current_Properties.Activate_Property ( Basin_Modelling::Lithostatic_Pressure );

    for ( int I = xStart; I < xStart + xCount; I++ ) {

      for ( int J = yStart; J < yStart + yCount; J++ ) {

        if ( cauldron->nodeIsDefined ( I, J ) ) {

          for ( int K = zStart; K < zStart + zCount; K++ ) {
            Overpressure_Value = Current_Layer -> Current_Properties ( Basin_Modelling::Overpressure,  K, J, I );

            if ( fabs ( Overpressure_Value ) < Minimum_Absolute_Overpressure_MPa ) {
              Overpressure_Value = 0.0;
            }

            Current_Layer -> Current_Properties ( Basin_Modelling::Overpressure,  K, J, I ) = Overpressure_Value;
            Current_Layer -> Current_Properties ( Basin_Modelling::Pore_Pressure, K, J, I ) = Overpressure_Value +
                                                                                              Current_Layer -> Current_Properties ( Basin_Modelling::Hydrostatic_Pressure, K, J, I );

            if ( Current_Layer -> Current_Properties ( Basin_Modelling::Pore_Pressure, K, J, I ) >
                   Current_Layer -> Current_Properties ( Basin_Modelling::Lithostatic_Pressure, K, J, I )) {

              Fracture_Pressure = Current_Layer -> Current_Properties ( Basin_Modelling::Lithostatic_Pressure, K, J, I ) -
                                  Current_Layer -> Current_Properties ( Basin_Modelling::Hydrostatic_Pressure, K, J, I );

              Current_Layer -> Current_Properties ( Basin_Modelling::Overpressure,  K, J, I ) = Fracture_Pressure;
              Current_Layer -> Current_Properties ( Basin_Modelling::Pore_Pressure, K, J, I ) = Current_Layer -> Current_Properties ( Basin_Modelling::Lithostatic_Pressure,  K, J, I );
            }

          }

   }

      }

    }

    Current_Layer -> Current_Properties.Restore_Property ( Basin_Modelling::Overpressure );
    Current_Layer -> Current_Properties.Restore_Property ( Basin_Modelling::Pore_Pressure );
    Current_Layer -> Current_Properties.Restore_Property ( Basin_Modelling::Hydrostatic_Pressure );
    Current_Layer -> Current_Properties.Restore_Property ( Basin_Modelling::Lithostatic_Pressure );

  }


}

//------------------------------------------------------------//


PetscScalar PressureSolver::maximumPressureDifference () {

  using namespace Basin_Modelling;

  int xStart;
  int yStart;
  int zStart;

  int xCount;
  int yCount;
  int zCount;

  Vec Current_Overpressure;
  Vec Previous_Overpressure;
  Vec Pressure_Difference;

  Layer_Iterator Layers;
  LayerProps_Ptr Current_Layer;

  PetscScalar Maximum_Difference = -1.0e10;
  PetscScalar Maximum_Layer_Difference;

  Layers.Initialise_Iterator ( cauldron->layers, Descending, Sediments_Only, Active_Layers_Only );

  while ( ! Layers.Iteration_Is_Done ()) {
    Current_Layer = Layers.Current_Layer ();
    //
    //
    // Get the size of the layer DA.
    //
    DMDAGetCorners ( Current_Layer->layerDA, &xStart, &yStart, &zStart, &xCount, &yCount, &zCount );

    Current_Overpressure  = Current_Layer->Current_Properties  ( Basin_Modelling::Overpressure );
    Previous_Overpressure = Current_Layer->Previous_Properties ( Basin_Modelling::Overpressure );

    DMCreateGlobalVector ( Current_Layer->layerDA, & Pressure_Difference );

    VecWAXPY( Pressure_Difference, NegOne, Previous_Overpressure, Current_Overpressure );
    VecAbs( Pressure_Difference );
    VecMax( Pressure_Difference,PETSC_NULL,&Maximum_Layer_Difference );
    Destroy_Petsc_Vector( Pressure_Difference );

    Maximum_Difference = PetscMax ( Maximum_Difference, Maximum_Layer_Difference );

    Layers++;
  }

  return Maximum_Difference;
}

//------------------------------------------------------------//


PetscScalar PressureSolver::maximumPressureDifference2 () {

  using namespace Basin_Modelling;

  int xStart;
  int yStart;
  int zStart;

  int xCount;
  int yCount;
  int zCount;

  int I;
  int J;
  int K;

  int maxI;
  int maxJ;
  int maxK;

  int iMin;
  int iMax;
  int jMin;
  int jMax;

  Layer_Iterator Layers;
  LayerProps_Ptr currentLayer;
  LayerProps_Ptr layerAbove;
  LayerProps_Ptr layerBelow;

  PetscScalar pressureDifference;
  PetscScalar maximumLayerPressureDifference;
  PetscScalar localMaximumPressureDifference = -1.0;
  PetscScalar globalMaximumPressureDifference;

  Layers.Initialise_Iterator ( cauldron->layers, Descending, Sediments_Only, Active_Layers_Only );

  while ( ! Layers.Iteration_Is_Done ()) {
    currentLayer = Layers.Current_Layer ();
    maximumLayerPressureDifference = -1.0;

    maxI = -1;
    maxJ = -1;
    maxK = -1;


    if ( Layers.Layer_Above () == nullptr ) {
      layerAbove = currentLayer;
    } else {
      layerAbove = Layers.Layer_Above ();
    }

    if ( Layers.Layer_Below () == nullptr ) {
      layerBelow = currentLayer;
    } else {
      layerBelow = Layers.Layer_Below ();
    }

    //
    // Get the size of the layer DA.
    //
    DMDAGetCorners ( currentLayer->layerDA, &xStart, &yStart, &zStart, &xCount, &yCount, &zCount );

    PETSC_3D_Array currentPressure  ( currentLayer->layerDA, currentLayer->Current_Properties  ( Basin_Modelling::Overpressure ));
    PETSC_3D_Array previousPressure ( currentLayer->layerDA, currentLayer->Previous_Properties ( Basin_Modelling::Overpressure ));

    for ( I = xStart; I < xStart + xCount; I++ ) {

      if ( I == xStart ) {
        iMin = xStart;
      } else {
        iMin = I - 1;
      }

      if ( I == xStart + xCount - 1 ) {
        iMax = xStart + xCount - 1;
      } else {
        iMax = I + 1;
      }

      for ( J = yStart; J < yStart + yCount; J++ ) {

        if ( J == yStart ) {
          jMin = yStart;
        } else {
          jMin = J - 1;
        }

        if ( J == yStart + yCount - 1 ) {
          jMax = yStart + yCount - 1;
        } else {
          jMax = J + 1;
        }

        if ( cauldron->nodeIsDefined ( I, J ) && ! currentLayer->getLithologyHasSwitched ( I, J )) {

          for ( K = zStart; K < zStart + zCount; K++ ) {

            pressureDifference = fabs ( currentPressure ( K, J, I ) - previousPressure ( K, J, I ));

            if ( pressureDifference > maximumLayerPressureDifference ) {
              maxI = I;
              maxJ = J;
              maxK = K;
              maximumLayerPressureDifference = pressureDifference;
            }

          }

        }

      }

    }

    localMaximumPressureDifference = NumericFunctions::Maximum ( localMaximumPressureDifference, maximumLayerPressureDifference );

    if ( cauldron->debug2 ) {
      PetscSynchronizedPrintf ( PETSC_COMM_WORLD, " Maximum difference for layer: %d  %s   %d %d %d   %3.4f %3.4f %3.4f \n",
                                FastcauldronSimulator::getInstance ().getRank (),
                                currentLayer -> layername.c_str (),
                                maxI,  maxJ,  maxK,
                                maximumLayerPressureDifference,
                                currentPressure ( maxK, maxJ, maxI ),
                                previousPressure ( maxK, maxJ, maxI ));

      PetscSynchronizedFlush ( PETSC_COMM_WORLD, PETSC_STDOUT );
    }

    Layers++;
  }

  // Now find the maximum pressure difference of all processes
  MPI_Allreduce( &localMaximumPressureDifference, &globalMaximumPressureDifference, 1,
                 MPI_DOUBLE, MPI_MAX, PETSC_COMM_WORLD );


  return globalMaximumPressureDifference;
}

//------------------------------------------------------------//

void PressureSolver::setBasementDepths ( const double           Current_Time,
                                               Double_Array_2D& Depth_Above,
                                         const Boolean2DArray&  Valid_Needle ) {

  // As the depth values in the basement are computed before the overpressure is calculated they
  // will probably be incorrect. Therefore, they must be set to the correct value.
  //
  // Should only be called for iteratively coupled problems.

  const double presentDayMantleThickness = FastcauldronSimulator::getInstance ().getMantleFormation ()->getPresentDayThickness ();

  int X_Start;
  int Y_Start;
  int Z_Start;
  int X_Count;
  int Y_Count;
  int Z_Count;
  int Z_Top;

  int I;
  int J;
  int K;

  double BasementThickness;
  double CrustThinningRatio;
  double MantleThickness;
  double Mantle_Segment_Height;
  double Max_Depth;
  double currentCrustThickness;

  double mantleMaximumElementThickness = FastcauldronSimulator::getInstance ().getRunParameters ()->getBrickHeightMantle ();

  DMDAGetCorners ( *cauldron->mapDA, &X_Start, &Y_Start, PETSC_NULL, &X_Count, &Y_Count, PETSC_NULL );

  LayerProps_Ptr Crust_Layer  = cauldron->Crust ();
  LayerProps_Ptr Mantle_Layer = cauldron->Mantle ();

  PETSC_3D_Array Crust_Depth     ( Crust_Layer->layerDA, Crust_Layer->Current_Properties ( Basin_Modelling::Depth ));
  PETSC_3D_Array Mantle_Depth    ( Mantle_Layer->layerDA, Mantle_Layer->Current_Properties ( Basin_Modelling::Depth ));

  // Get the size of the layer DA.
  DMDAGetCorners ( Crust_Layer->layerDA, &X_Start, &Y_Start, &Z_Start, &X_Count, &Y_Count, &Z_Count );
  Z_Top = Z_Start + Z_Count - 1;

  Basin_Modelling::initialiseTopNodes ( X_Start, X_Count, Y_Start, Y_Count, Z_Top, Valid_Needle, Depth_Above, Crust_Depth );

  // Compute CRUST depths
  for ( I = X_Start; I < X_Start + X_Count; I++ ) {

    for ( J = Y_Start; J < Y_Start + Y_Count; J++ ) {

      if ( Valid_Needle ( I, J )) {

        for ( K = Z_Start + Z_Count - 2; K >= 0; K-- ) {
          Crust_Depth ( K, J, I ) = Crust_Depth ( K + 1, J, I ) + Crust_Layer->getSolidThickness ( I, J, K, Current_Time );
        }

      } else {

        for ( K = Z_Start + Z_Count - 2; K >= 0; K-- ) {
          Crust_Depth ( K, J, I ) = CauldronNoDataValue;
        }

      }

    }

  }

  Basin_Modelling::copyBottomNodes ( X_Start, X_Count, Y_Start, Y_Count, Valid_Needle, Depth_Above, Crust_Depth );

  DMDAGetCorners ( Mantle_Layer->layerDA, &X_Start, &Y_Start, &Z_Start, &X_Count, &Y_Count, &Z_Count );
  Z_Top = Z_Start + Z_Count - 1;

  Basin_Modelling::initialiseTopNodes ( X_Start, X_Count, Y_Start, Y_Count, Z_Top, Valid_Needle, Depth_Above, Mantle_Depth );

  // Compute MANTLE depths
  for ( I = X_Start; I < X_Start + X_Count; I++ ) {

    for ( J = Y_Start; J < Y_Start + Y_Count; J++ ) {

      if ( Valid_Needle ( I, J )) {
            currentCrustThickness = FastcauldronSimulator::getInstance ().getCrustThickness ( I, J, Current_Time );
            CrustThinningRatio = cauldron->Crust ()->getCrustMaximumThickness ( I, J ) / currentCrustThickness;

         if( cauldron->isALC() ) {
            BasementThickness = cauldron->Mantle()->getInitialLithosphericMantleThickness () + cauldron->Crust()->getInitialCrustalThickness();
            Mantle_Segment_Height = cauldron->Mantle()->m_mantleElementHeight0 / CrustThinningRatio;
         } else {
            BasementThickness = presentDayMantleThickness + FastcauldronSimulator::getInstance ().getCrustThickness ( I, J, 0.0 );
            Mantle_Segment_Height = mantleMaximumElementThickness / CrustThinningRatio;
         }
         MantleThickness = BasementThickness - currentCrustThickness;
         Max_Depth = Mantle_Depth ( Z_Top, J, I ) + MantleThickness;

         for ( K = Z_Top - 1; K >= 0; K-- ) {
            Mantle_Depth ( K, J, I ) = PetscMin ( Max_Depth, Mantle_Depth ( K + 1, J, I ) + Mantle_Segment_Height );
         }

      } else {

        for ( K = Z_Top - 1; K >= 0; K-- ) {
          Mantle_Depth ( K, J, I ) = CauldronNoDataValue;
        }

      }

    }

  }

}


//------------------------------------------------------------//

int PressureSolver::getPlaneQuadratureDegree ( const int optimisationLevel ) const {

  return PlaneQuadratureDegrees [ optimisationLevel - 1 ];
}

//------------------------------------------------------------//

int PressureSolver::getDepthQuadratureDegree ( const int optimisationLevel ) const {

  return DepthQuadratureDegrees [ optimisationLevel - 1 ];
}

//------------------------------------------------------------//

void PressureSolver::setPlaneQuadratureDegree ( const int optimisationLevel,
                                                const int newDegree ) {

  PlaneQuadratureDegrees [ optimisationLevel - 1 ] = newDegree;

}

//------------------------------------------------------------//

void PressureSolver::setDepthQuadratureDegree ( const int optimisationLevel,
                                                const int newDegree ) {

  DepthQuadratureDegrees [ optimisationLevel - 1 ] = newDegree;

}

//------------------------------------------------------------//

void PressureSolver::setMaximumNumberOfNonlinearIterations ( const int optimisationLevel,
                                                             const int maximumIterationCount ) {
  s_numberOfIterations [ optimisationLevel - 1 ] = maximumIterationCount;
}

//------------------------------------------------------------//

int PressureSolver::getMaximumNumberOfNonlinearIterations ( const int optimisationLevel ) {
  return s_numberOfIterations [ optimisationLevel - 1 ];
}

//------------------------------------------------------------//

double PressureSolver::getNewtonSolverTolerance ( const int  optimisationLevel,
                                                  const bool isGeometricLoop,
                                                  const int  geometricLoopNumber ) {

  int overpressureRun;

  if ( isGeometricLoop and not FastcauldronSimulator::getInstance ().getCauldron ()->readFCTCorrectionFactor ) {
    overpressureRun = int ( PetscMin ( geometricLoopNumber, 3 ));
  } else {
    overpressureRun = 3;
  }

  return NewtonSolverTolerances [ optimisationLevel - 1 ][ overpressureRun - 1 ];

}

//------------------------------------------------------------//

void PressureSolver::setNewtonSolverTolerance ( const int    optimisationLevel,
                                                const double newTolerance ) {

  NewtonSolverTolerances [ optimisationLevel - 1 ][ 0 ] = newTolerance;
  NewtonSolverTolerances [ optimisationLevel - 1 ][ 1 ] = newTolerance;
  NewtonSolverTolerances [ optimisationLevel - 1 ][ 2 ] = newTolerance;

}

//------------------------------------------------------------//

double PressureSolver::linearSolverTolerance ( const int optimisationLevel ) const {

  static const double LinearSolverTolerances [ NumberOfOptimisationLevels ] = { 1.0e-5, 1.0e-5, 1.0e-5, 1.0e-6, 1.0e-7 };

  return LinearSolverTolerances [ optimisationLevel - 1 ];

}

//------------------------------------------------------------//

double PressureSolver::getRelativeThicknessTolerance ( const int optimisationLevel ) const {

  static const double ThicknessTolerances [ NumberOfOptimisationLevels ] = { 0.05, 0.03, 0.01, 0.01, 0.01 };

  return ThicknessTolerances [ optimisationLevel - 1 ];

}

//------------------------------------------------------------//

double PressureSolver::getAbsoluteThicknessTolerance ( const int optimisationLevel ) const {

  static const double ThicknessTolerances [ NumberOfOptimisationLevels ] = { 5.0, 3.0, 1.0, 1.0, 1.0 };

  return ThicknessTolerances [ optimisationLevel - 1 ];

}

//------------------------------------------------------------//

int PressureSolver::numberOfStepsForCompactionEquation ( const int optimisationLevel ) const {

  static const int NumberOfSteps [ NumberOfOptimisationLevels ] = { 3, 3, 4, 4, 5 };

  return NumberOfSteps [ optimisationLevel - 1 ];

}

//------------------------------------------------------------//

int PressureSolver::getIterationsForIluFillLevelIncrease ( const int optimisationLevel,
                                                           const int currentIluFillLevel ) {

   return s_iterationsForiluFillLevelIncrease [ optimisationLevel ];

}

//------------------------------------------------------------//

void PressureSolver::setIterationsForIluFillLevelIncrease ( const int newIluFillLevelIterations ) {

   int i;

   // Since the optimisation-level is not known at this time,
   // set all of the values to the new value.
   for ( i = 0; i < NumberOfOptimisationLevels; ++i ) {
      s_iterationsForiluFillLevelIncrease [ i ]= newIluFillLevelIterations;
   }

}

//------------------------------------------------------------//

void PressureSolver::getBoundaryConditions ( const GeneralElement& element,
                                             const double          currentTime,
                                             const int             topIndex,
                                             const bool            constrainedOverPressure,
                                             const double          constrainedOverpressureValue,
                                             const bool            isIceSheetLayer,
                                             ElementVector&        fracturePressureExceeded,
                                             BoundaryConditions&   bcs ) const {

   const HydraulicFracturingManager& hfm = HydraulicFracturingManager::getInstance ();
   const FastcauldronSimulator& fc = FastcauldronSimulator::getInstance ();

   static const double surfaceOverPressureValue = 0.0;

   const LayerElement& layerElement = element.getLayerElement ();
   const LayerProps* currentLayer = layerElement.getFormation ();
   const Lithology* elementLithology = layerElement.getLithology ();

   int numberOfDepthElements = currentLayer->getMaximumNumberOfElements ();
   bool constrainedOverPressureEnabled;
   double constrainedOverPressureValue;

   currentLayer->getConstrainedOverpressure ( currentTime, constrainedOverPressureValue, constrainedOverPressureEnabled );
   bcs.reset ();

   const GeneralElement* elementAbove = element.getActiveNeighbour ( VolumeData::ShallowFace );

   for ( int n = 0; n < 8; ++n ) {
      int nodeK = element.getNodeK ( n );
      int layerNodeK = layerElement.getNodeLocalKPosition ( n );
      int nodeJ = element.getNodeJ ( n );
      int nodeI = element.getNodeI ( n );

      fracturePressureExceeded ( n + 1 ) = currentLayer->fracturedPermeabilityScaling ( nodeI, nodeJ, layerNodeK );

      if ( nodeK == topIndex )
      {
         bcs.setBoundaryConditions ( n, Surface_Boundary, surfaceOverPressureValue );
      }
      else if ( constrainedOverPressureEnabled )
      {
         bcs.setBoundaryConditions ( n, Interior_Constrained_Overpressure, constrainedOverPressureValue );
      }
      else if ( ( layerNodeK == numberOfDepthElements - 1 and fracturePressureExceeded ( n + 1 ) > 0.0 and elementAbove != nullptr and  elementAbove->getLayerElement ().getLithology ()->surfacePorosity () == 0.0 ) or
              ( hfm.isNonConservativeFractureModel()    and currentLayer->nodeIsTemporarilyDirichlet( nodeI, nodeJ, layerNodeK ) ) )
      {
         // Need elemnt boundary conditions in order to eliminate this check for surface porosity == 0.
         double hydrostaticPressure = currentLayer->Current_Properties( Basin_Modelling::Hydrostatic_Pressure,
                                                                        layerNodeK, nodeJ, nodeI );

         double fracturePressure = hfm.fracturePressure ( elementLithology,
                                                          currentLayer->fluid,
                                                          fc.getSeaBottomTemperature ( nodeI, nodeJ, currentTime ),
                                                          fc.getSeaBottomDepth ( nodeI, nodeJ, currentTime ),
                                                          currentLayer->Current_Properties ( Basin_Modelling::Depth, layerNodeK, nodeJ, nodeI ),
                                                          hydrostaticPressure,
                                                          currentLayer->Current_Properties ( Basin_Modelling::Lithostatic_Pressure, layerNodeK, nodeJ, nodeI ));
         bcs.setBoundaryConditions ( n, Interior_Constrained_Overpressure,
                                     NumericFunctions::Maximum ( fracturePressure - hydrostaticPressure, 0.0 ));

      }
      else if ( isIceSheetLayer )
      {
         double hydrostaticPressure = currentLayer->Current_Properties ( Basin_Modelling::Hydrostatic_Pressure,
                                                                         layerNodeK, nodeJ, nodeI );
         double lithostaticPressure = currentLayer->Current_Properties ( Basin_Modelling::Lithostatic_Pressure,
                                                                         layerNodeK, nodeJ, nodeI );


         // For the ice sheet with Permafrost taking in account, we do noy want to "compute" the overpressure in the ice lithology - we want to impose it.
         bcs.setBoundaryConditions ( n, Interior_Constrained_Overpressure,
                                     NumericFunctions::Maximum ( lithostaticPressure - hydrostaticPressure, 0.0 ));
      }
      else if ( constrainedOverPressure )
      {
         bcs.setBoundaryConditions ( n, Interior_Constrained_Overpressure, constrainedOverpressureValue );

      }

   }

}

//------------------------------------------------------------//

void PressureSolver::assembleSystem ( const ComputationalDomain& computationalDomain,
                                      const double               previousTime,
                                      const double               currentTime,
                                      Mat&                       jacobian,
                                      Vec&                       residual,
                                      double&                    elementContributionsTime ) {

  using namespace Basin_Modelling;

  PetscLogStages::push( PetscLogStages::PRESSURE_SYSTEM_ASSEMBLY );

  const int Plane_Quadrature_Degree = getPlaneQuadratureDegree ( cauldron->Optimisation_Level );
  const int Depth_Quadrature_Degree = getDepthQuadratureDegree ( cauldron->Optimisation_Level );

  double timeStep = previousTime - currentTime;

  PetscLogDouble Element_Start_Time;
  PetscLogDouble Element_End_Time;
  PetscLogDouble Start_Time;
  PetscLogDouble End_Time;

  PetscTime(&Start_Time);

  bool includeGhostValues = true;

  const CauldronGridDescription& grid = FastcauldronSimulator::getInstance ().getCauldronGridDescription ();

  LayerProps_Ptr currentLayer;
  const CompoundLithology* elementLithology;

  Layer_Iterator FEM_Layers ( cauldron->layers, Ascending, Sediments_Only, Active_Layers_Only );
  Layer_Iterator Layers_Above; // CHANGE THIS NAME!!!!!

  ElementMatrix elementJacobian;
  ElementVector elementResidual;

  ElementVector exceededFracturePressure;
  ElementVector preFractureScaling;
  double fracturePressure;

  double constrainedPoValue;
  bool   overpressureIsConstrained;
  BoundaryConditions bcs;
  bool includeChemicalCompaction;

  Saturation currentSaturation;
  Saturation previousSaturation;

  // If Darcy simulation has not been enabled then set the saturation to ( Water => 1, Vapour => 0, Liquid => 0 )
  currentSaturation.initialise ();
  previousSaturation.initialise ();

  bool includeWaterSaturation = FastcauldronSimulator::getInstance ().getMcfHandler ().includeWaterSaturationInOp ();
  bool includedInDarcySimulation = FastcauldronSimulator::getInstance ().getMcfHandler ().solveFlowEquations ();

  const CompositeElementActivityPredicate& activityPredicate = computationalDomain.getActivityPredicate ();
  int topIndex = computationalDomain.getStratigraphicColumn ().getNumberOfLogicalNodesInDepth ( currentTime ) - 1;

  VecSet ( residual, Zero );

  elementContributionsTime = 0.0;

  if ( basisFunctions == nullptr ) {
     basisFunctions = new FiniteElementMethod::BasisFunctionCache ( Plane_Quadrature_Degree, Plane_Quadrature_Degree, Depth_Quadrature_Degree );
  }

  PressureElementMatrixAssembly pressureAssembly ( *basisFunctions );

  for ( FEM_Layers.Initialise_Iterator (); ! FEM_Layers.Iteration_Is_Done (); FEM_Layers++ ) {
    currentLayer  = FEM_Layers.Current_Layer ();

    const ComputationalDomain::FormationGeneralElementGrid* formationGrid = computationalDomain.getFormationGrid ( currentLayer );

    double fluidDensityForP0_1andT0 = currentLayer->fluid->density ( 0,  0.1 );


    PetscBlockVector<Saturation> layerSaturations;
    PetscBlockVector<Saturation> previousLayerSaturations;

    if ( includedInDarcySimulation ) {
       layerSaturations.setVector ( currentLayer->getVolumeGrid ( Saturation::NumberOfPhases ), currentLayer->getPhaseSaturationVec (), INSERT_VALUES, false );
       previousLayerSaturations.setVector ( currentLayer->getVolumeGrid ( Saturation::NumberOfPhases ), currentLayer->getPreviousPhaseSaturationVec (), INSERT_VALUES, false );
    }

    includeChemicalCompaction = (( cauldron->Do_Chemical_Compaction ) && ( currentLayer -> Get_Chemical_Compaction_Mode ()));

    Layers_Above.Initialise_Iterator ( cauldron->layers, Ascending, currentLayer, Ascending, Sediments_Only, Active_Layers_Only );

    currentLayer->getConstrainedOverpressure ( currentTime,
                                               constrainedPoValue,
                                               overpressureIsConstrained );

    currentLayer->Current_Properties.Activate_Properties  ( INSERT_VALUES, includeGhostValues );
    currentLayer->Previous_Properties.Activate_Properties ( INSERT_VALUES, includeGhostValues );

    for ( size_t i = formationGrid->firstI (); i <= formationGrid->lastI (); ++i ) {

       for ( size_t j = formationGrid->firstJ (); j <= formationGrid->lastJ (); ++j ) {

          for ( size_t k = formationGrid->firstK (); k <= formationGrid->lastK (); ++k ) {
             const GeneralElement& gridElement = formationGrid->getElement ( i, j, k );
             const LayerElement& layerElement = gridElement.getLayerElement ();

             if ( activityPredicate.isActive ( layerElement )) {

                exceededFracturePressure.fill ( 0.0 );
                elementResidual.zero ();
                elementJacobian.zero ();

                if ( includedInDarcySimulation and includeWaterSaturation ) {
                   currentSaturation = layerSaturations ( layerElement.getLocalKPosition (), layerElement.getJPosition (), layerElement.getIPosition ());
                   previousSaturation = previousLayerSaturations ( layerElement.getLocalKPosition (), layerElement.getJPosition (), layerElement.getIPosition ());
                }

                // Copy segment lithology
                elementLithology = layerElement.getLithology ();

                // if element hase fluid density more than matrix density, we assuming the solid is ice in this case.
                bool isIceSheetLayer = layerElement.getFluid()->isPermafrostEnabled() and fluidDensityForP0_1andT0 > elementLithology->density();

                PetscTime(&Element_Start_Time);
                getBoundaryConditions ( gridElement,
                                        currentTime,
                                        topIndex,
                                        overpressureIsConstrained,
                                        constrainedPoValue,
                                        isIceSheetLayer,
                                        exceededFracturePressure,
                                        bcs );

                pressureAssembly.compute ( gridElement.getLayerElement (),
                                           currentTime, timeStep,
                                           bcs,
                                           isIceSheetLayer,
                                           includeChemicalCompaction,
                                           HydraulicFracturingManager::getInstance ().getModel (),
                                           exceededFracturePressure,
                                           includeWaterSaturation,
                                           currentSaturation,
                                           previousSaturation,
                                           elementJacobian,
                                           elementResidual );

                PetscTime(&Element_End_Time);
                elementContributionsTime = elementContributionsTime + Element_End_Time - Element_Start_Time;

                MatSetValues ( jacobian,
                               8, gridElement.getDofs ().data (),
                               8, gridElement.getDofs ().data (),
                               elementJacobian.C_Array (),
                               ADD_VALUES );

                VecSetValues ( residual,
                               8, gridElement.getDofs ().data (),
                               elementResidual.data (),
                               ADD_VALUES );

          }

        }

      }

    }

    currentLayer->Current_Properties.Restore_Properties ();
    currentLayer->Previous_Properties.Restore_Properties ();
  }

  VecAssemblyBegin ( residual );
  VecAssemblyEnd   ( residual );

  MatAssemblyBegin ( jacobian, MAT_FINAL_ASSEMBLY );
  MatAssemblyEnd   ( jacobian, MAT_FINAL_ASSEMBLY );

  PetscTime(&End_Time);

  PetscLogStages::pop();
}

//------------------------------------------------------------//

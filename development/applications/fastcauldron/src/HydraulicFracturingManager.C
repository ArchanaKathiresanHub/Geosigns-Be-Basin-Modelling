 //------------------------------------------------------------//

#include "HydraulicFracturingManager.h"

#include "propinterface.h"
#include "CompoundLithology.h"
#include "layer.h"
#include "PressureSolver.h"
#include "layer_iterators.h"
#include "utils.h"

#include "FastcauldronSimulator.h"

//------------------------------------------------------------//

#include "GeoPhysicalFunctions.h"
#include "GeoPhysicsFluidType.h"

//------------------------------------------------------------//

HydraulicFracturingManager* HydraulicFracturingManager::s_hydraulicFracturingManagerInstance = 0;

// Not rigorous.
int HydraulicFracturingManager::NumberOfIterations [ NumberOfFractureModels ] = { 2, 5, 8, 8, 16 };
double HydraulicFracturingManager::ScalingFactors [ NumberOfFractureModels ] = { 4.0, 8.0, 4.0, 8.0, 4.0 };


//------------------------------------------------------------//

int HydraulicFracturingManager::maximumNumberOfFractureIterations () const {

  return NumberOfIterations [ int ( m_fractureModel ) - 1 ];
}

//------------------------------------------------------------//

double HydraulicFracturingManager::permeabilityScalingIncreaseFactor () const {

  return ScalingFactors [ int ( m_fractureModel ) - 1 ];
}

//------------------------------------------------------------//

void HydraulicFracturingManager::setPermeabilityScalingIncreaseFactor ( const double scalingFactor ) {
  ScalingFactors [ int ( m_fractureModel ) - 1 ] = scalingFactor;
}

//------------------------------------------------------------//

HydraulicFracturingManager& HydraulicFracturingManager::getInstance () {

  if ( s_hydraulicFracturingManagerInstance == 0 ) {
    s_hydraulicFracturingManagerInstance = new HydraulicFracturingManager;
  }

  return *s_hydraulicFracturingManagerInstance;
}

//------------------------------------------------------------//

const std::string& hydraulicFractureFunctionTypeStr ( const HydraulicFractureFunctionType functionType ) {

  static const std::string UnknownFractureType = "Unknown fracture type";

  static const std::string FunctionStrings [ 5 ] = { HydraulicFracturingNone,
                                                     HydraulicFracturingFunctionOfDepthBelowSeaLevel,
                                                     HydraulicFracturingFunctionOfDepthBelowSedimentSurface,
                                                     HydraulicFracturingFunctionOfLithostaticPressure,
                                                     UnknownFractureType };

  int functionTypeInt = int ( functionType ) - int ( Interface::None );

  if ( inRange ( functionTypeInt, 0, 3 )) {
    return FunctionStrings [ functionTypeInt ];
  } else {
    return FunctionStrings [ 4 ];
  }

}

//------------------------------------------------------------//

const std::string& fracturePressureModelStr ( const Interface::FracturePressureModel model ) {

  static const std::string ModelStrings [ NumberOfFractureModels + 1 ] = { "NON_CONSERVATIVE_TOTAL",
                                                                           "NON_CONSERVATIVE_PARTIAL_1",
                                                                           "NON_CONSERVATIVE_PARTIAL_2",
                                                                           "CONSERVATIVE_1",
                                                                           "CONSERVATIVE_2",
                                                                           "Unknown fracture pressure model" };

  int modelInt = int ( model ) - 1;

  if ( inRange ( modelInt, 0, NumberOfFractureModels - 1 )) {
    return ModelStrings [ modelInt ];
  } else {
    return ModelStrings [ NumberOfFractureModels ];
  }

}

//------------------------------------------------------------//

HydraulicFracturingManager::HydraulicFracturingManager () {
  m_basinModel = 0;
}

//------------------------------------------------------------//

HydraulicFracturingManager::~HydraulicFracturingManager () {
  // Nothing to do here.
}

//------------------------------------------------------------//

void HydraulicFracturingManager::setAppCtx ( const AppCtx* newAppCtx ) {
  m_basinModel = newAppCtx;
}

//------------------------------------------------------------//

bool HydraulicFracturingManager::setModel ( const int newModel ) {

  bool result = true;

  if ( newModel == IBSNULLVALUE or newModel == CAULDRONIBSNULLVALUE ) {
    m_fractureModel = DefaultFractureModel;
  } else if ( inRange ( newModel, 1, NumberOfFractureModels )) {
    m_fractureModel = Interface::FracturePressureModel ( newModel );
  } else {
    m_fractureModel = DefaultFractureModel;
    result = false;
  }

  return result;
}

//------------------------------------------------------------//

bool HydraulicFracturingManager::setSelectedFunction () {

  m_fracturePressureCalculator = &FastcauldronSimulator::getInstance ().getFracturePressureCalculator ();

  // Initialised as true, only set to false if fractureType 
  // string contains an undefined value.
  bool functionDefined = true;

  m_fractureModel = m_fracturePressureCalculator->getFracturePressureFunctionParameters ().getFractureModel ();

  if ( m_basinModel->debug1 ) {
    PetscPrintf ( PETSC_COMM_WORLD, " set selected function: %i  %s \n",
                  int ( m_fracturePressureCalculator->getFracturePressureFunctionParameters ().type ()),
                  hydraulicFractureFunctionTypeStr ( m_fracturePressureCalculator->getFracturePressureFunctionParameters ().type ()).c_str ());
  }

  return functionDefined;
}

//------------------------------------------------------------//

double HydraulicFracturingManager::fracturePressure ( const CompoundLithology* lithology,
                                                      const GeoPhysics::FluidType*  currentFluid,
                                                      const double      seaTemperature,
                                                      const double      surfaceDepth,
                                                      const double      depth,
                                                      const double      hydrostaticPressure,
                                                      const double      lithostaticPressure ) const {

   return m_fracturePressureCalculator->fracturePressure ( lithology, currentFluid, seaTemperature, surfaceDepth, depth, hydrostaticPressure, lithostaticPressure );
}

//------------------------------------------------------------//

bool HydraulicFracturingManager::hasFractured ( const CompoundLithology* lithology,
                                                const GeoPhysics::FluidType*  currentFluid,
                                                const double      seaTemperature,
                                                const double      surfaceDepth,
                                                const double      depth,
                                                const double      hydrostaticPressure,
                                                const double      lithostaticPressure,
                                                const double      porePressure ) const {

   return m_fracturePressureCalculator->hasFractured ( lithology, currentFluid, seaTemperature, surfaceDepth, depth, hydrostaticPressure, lithostaticPressure, porePressure );
}

//------------------------------------------------------------//

void HydraulicFracturingManager::checkForFracturing ( const double currentTime,
                                                      const bool   applyNonConservativeModel,
                                                            bool&  modelHasFractured ) {

  using namespace Basin_Modelling;

  if ( m_fracturePressureCalculator->getFracturePressureFunctionParameters ().type () == Interface::None ) {
//   if ( m_functionSelection == NO_FRACTURE_PRESSURE ) {
    return;
  }

  bool layerHasFractured;

  modelHasFractured = false;

  Layer_Iterator Layers ( m_basinModel->layers, Ascending, Sediments_Only, Active_Layers_Only );

  while ( ! Layers.Iteration_Is_Done ()) {
    checkForFracturing ( Layers.Current_Layer (), currentTime, m_basinModel->getValidNeedles (), applyNonConservativeModel, layerHasFractured );

    // Since there has been a synchronisation at the layer level, there is no need to perform a further sync here.
    // The layerHasFractured variable already contains the globally (all processes) value.
    modelHasFractured = modelHasFractured || layerHasFractured;
    Layers++;
  }

  if ( FastcauldronSimulator::getInstance ().getRank () == 0 and m_basinModel->debug1 ) {

    if ( modelHasFractured ) {
      PetscPrintf ( PETSC_COMM_WORLD, " Model has fractured \n" );
    } else {
      PetscPrintf ( PETSC_COMM_WORLD, " Model has not fractured \n" );
    }

  }

}

//------------------------------------------------------------//

void HydraulicFracturingManager::checkForFracturing ( LayerProps*           theLayer,
                                                      const double          currentTime,
                                                      const Boolean2DArray& Valid_Needle,
                                                      const bool            applyNonConservativeModel,
                                                            bool&           layerHasFractured ) {

  int X_Start;
  int X_Count;
  int Y_Start;
  int Y_Count;
  int Z_Start;
  int Z_Count;

  int xLocalStart;
  int xLocalCount;
  int yLocalStart;
  int yLocalCount;


  int i;
  int j;
  int k;
  int localI;
  int localJ;

  const CompoundLithology*  currentLithology;

  bool nodeHasFractured;
  bool nodeBelowHasFractured;
  int  fracturedNodeCount = 0;
  int  scaledNodeCount = 0;

  double hydrostaticPressure;
  double lithostaticPressure;
  double porePressure;
  double overpressure;
  double computedFracturePressure;
  double nodeDepth;
  double surfaceDepth;
  double seaTemperature;

  double increaseFactor;
  double decreaseFactor;

#if 0
  double startFracturingPressure;
#endif

  // For output and debugging purposes only.
  double maxFractureValue = 0.0;

  DAGetCorners ( theLayer->layerDA, &xLocalStart, &yLocalStart, PETSC_NULL, &xLocalCount, &yLocalCount, PETSC_NULL );
  DAGetCorners ( theLayer->layerDA, &X_Start, &Y_Start, &Z_Start, &X_Count, &Y_Count, &Z_Count );

  theLayer->Current_Properties.Activate_Property ( Basin_Modelling::Depth );
  theLayer->Current_Properties.Activate_Property ( Basin_Modelling::Overpressure );
  theLayer->Current_Properties.Activate_Property ( Basin_Modelling::Pore_Pressure );
  theLayer->Current_Properties.Activate_Property ( Basin_Modelling::Hydrostatic_Pressure );
  theLayer->Current_Properties.Activate_Property ( Basin_Modelling::Lithostatic_Pressure );

  layerHasFractured = false;

  for ( i = X_Start; i < X_Start + X_Count; ++i ) {
    localI = Integer_Min ( Integer_Max ( i, xLocalStart ), xLocalStart + xLocalCount - 1 ) - xLocalStart;

    for ( j = Y_Start; j < Y_Start + Y_Count; ++j ) {
      localJ = Integer_Min ( Integer_Max ( j, yLocalStart ), yLocalStart + yLocalCount - 1 ) - yLocalStart;

      if ( Valid_Needle ( i, j )) {
        currentLithology = theLayer->getLithology ( i, j );
        surfaceDepth = FastcauldronSimulator::getInstance ().getSeaBottomDepth ( i, j, currentTime );
        seaTemperature = FastcauldronSimulator::getInstance ().getSeaBottomTemperature ( i, j, currentTime );
        nodeBelowHasFractured = false;

        for ( k = Z_Start; k < Z_Start + Z_Count; ++k ) {
          overpressure        = theLayer->Current_Properties ( Basin_Modelling::Overpressure, k, j, i );
          hydrostaticPressure = theLayer->Current_Properties ( Basin_Modelling::Hydrostatic_Pressure, k, j, i );
          lithostaticPressure = theLayer->Current_Properties ( Basin_Modelling::Lithostatic_Pressure, k, j, i );
          nodeDepth           = theLayer->Current_Properties ( Basin_Modelling::Depth, k, j, i );

          porePressure = hydrostaticPressure + overpressure;

          computedFracturePressure = fracturePressure ( currentLithology,
                                                        theLayer->fluid,
                                                        seaTemperature,
                                                        surfaceDepth,
                                                        nodeDepth,
                                                        hydrostaticPressure,
                                                        lithostaticPressure );

          if ( computedFracturePressure == CAULDRONIBSNULLVALUE || computedFracturePressure == NoFracturePressureValue ) {
            theLayer->fracturedPermeabilityScaling ( i, j, k ) = 0.0;
            nodeHasFractured = false;
          } else {

            increaseFactor = permeabilityScalingIncreaseFactor ();
            decreaseFactor = NumericFunctions::Maximum ( 1.0, 0.25 * increaseFactor );

#if 0
            startFracturingPressure = hydrostaticPressure + StartFracturingScalingFactor [ m_basinModel->fracturingModel ] * ( computedFracturePressure - hydrostaticPressure );
            startFracturingPressure = NumericFunctions::Maximum ( startFracturingPressure, hydrostaticPressure );

            computedFracturePressure = FracurePressureScalingFactor [ m_basinModel->fracturingModel ] * computedFracturePressure;
            computedFracturePressure = NumericFunctions::Minimum ( computedFracturePressure, lithostaticPressure );
            nodeHasFractured = false;
#endif

            nodeHasFractured = porePressure > hydrostaticPressure and porePressure >= computedFracturePressure;

#if 0
            if ( porePressure > startFracturingPressure ) {

              if ( porePressure >= computedFracturePressure ) {
                nodeHasFractured = true;
                increaseFactor = 4;
              } else {
                double scaling = MaximumPreFracturingScaling * ( porePressure - startFracturingPressure ) / ( computedFracturePressure - startFracturingPressure );
                preFractureScaling ( i, j, k ) = scaling;
              }

            } else {
              preFractureScaling ( i, j, k ) = MaximumPreFracturingScaling;
              preFractureScaling ( i, j, k ) = 0.0;
            }
#endif

            if ( nodeHasFractured ) {
              theLayer->fracturedPermeabilityScaling ( i, j, k ) = NumericFunctions::Minimum ( theLayer->fracturedPermeabilityScaling ( i, j, k ) + increaseFactor, 40.0 );
              ++fracturedNodeCount;
            } else if ( nodeBelowHasFractured and k > 0 ) {
              // Scale the nodes above the node that has fractured, but decrease the factor.
              theLayer->fracturedPermeabilityScaling ( i, j, k ) = NumericFunctions::Maximum ( theLayer->fracturedPermeabilityScaling ( i, j, k - 1 ) - 4.0, 0.0 );
            } else {
              theLayer->fracturedPermeabilityScaling ( i, j, k ) = NumericFunctions::Maximum ( theLayer->fracturedPermeabilityScaling ( i, j, k ) - decreaseFactor, 0.0 );
            }

            if ( theLayer->fracturedPermeabilityScaling ( i, j, k ) > 0.0 ) {
              scaledNodeCount += 1;
            }

            // Check to see if the node needs to be set as a Dirichlet type node.
            theLayer->nodeIsTemporarilyDirichlet ( i, j, k ) = applyNonConservativeModel and nodeHasFractured;

            layerHasFractured = layerHasFractured or nodeHasFractured;
          }

          maxFractureValue = NumericFunctions::Maximum ( maxFractureValue, theLayer->fracturedPermeabilityScaling ( i, j, k ));
          nodeBelowHasFractured = nodeBelowHasFractured or nodeHasFractured;
        }

      }

    }

  }

  // If process section of the layer has fractured, then tell all processes.
  layerHasFractured = broadcastAnyTrueBooleanValue ( layerHasFractured );

  if ( m_basinModel->debug1 and maxFractureValue > 0 ) {
    cout << theLayer->layername << " maxFractureValue " << maxFractureValue << endl;
  }

  if ( m_basinModel->debug1 ) {
    int globalFracturedNodeCount = 0;
    int globalScaledNodeCount = 0;

    MPI_Allreduce( &fracturedNodeCount, &globalFracturedNodeCount, 1, 
                   MPI_INT, MPI_SUM, PETSC_COMM_WORLD );

    MPI_Allreduce( &scaledNodeCount, &globalScaledNodeCount, 1, 
                   MPI_INT, MPI_SUM, PETSC_COMM_WORLD );

    PetscPrintf ( PETSC_COMM_WORLD, 
                  " Number of fractured and scaled nodes for layer '%s' is  %i  %i \n", 
                  theLayer->layername.c_str (),
                  globalFracturedNodeCount,
                  globalScaledNodeCount );

  }

  theLayer->Current_Properties.Restore_Property ( Basin_Modelling::Depth );
  theLayer->Current_Properties.Restore_Property ( Basin_Modelling::Overpressure );
  theLayer->Current_Properties.Restore_Property ( Basin_Modelling::Pore_Pressure );
  theLayer->Current_Properties.Restore_Property ( Basin_Modelling::Hydrostatic_Pressure );
  theLayer->Current_Properties.Restore_Property ( Basin_Modelling::Lithostatic_Pressure );
}

//------------------------------------------------------------//

void HydraulicFracturingManager::resetFracturing () {

  using namespace Basin_Modelling;

  if ( m_fracturePressureCalculator->getFracturePressureFunctionParameters ().type () == Interface::None ) {
//   if ( m_functionSelection == NO_FRACTURE_PRESSURE ) {
    return;
  }

  Layer_Iterator Layers ( m_basinModel->layers, Ascending, Sediments_Only, Active_Layers_Only );

  while ( ! Layers.Iteration_Is_Done ()) {
    resetFracturing ( Layers.Current_Layer (), m_basinModel->getValidNeedles ());
    Layers++;
  }

}

//------------------------------------------------------------//

void HydraulicFracturingManager::resetFracturing ( LayerProps*           theLayer,
                                                   const Boolean2DArray& validNeedle ) {


  int xStart;
  int xCount;
  int yStart;
  int yCount;
  int zStart;
  int zCount;

  int i;
  int j;
  int k;

  DAGetCorners ( theLayer->layerDA, &xStart, &yStart, &zStart, &xCount, &yCount, &zCount );

  for ( i = xStart; i < xStart + xCount; ++i ) {

    for ( j = yStart; j < yStart + yCount; ++j ) {

      if ( validNeedle ( i, j )) {

        for ( k = zStart; k < zStart + zCount; ++k ) {

#if 0
          // Decrease the scaling factor by some amount, this maintains some permeability 
          // increase even if there has been no fracturing for this time-step.
          theLayer->fracturedPermeabilityScaling ( i, j, k ) = NumericFunctions::Maximum ( theLayer->fracturedPermeabilityScaling ( i, j, k ) - 16.0, 0.0 );
#endif

          theLayer->fracturedPermeabilityScaling ( i, j, k ) = 0.0;
          theLayer->pressureExceedsFracturePressure ( i, j, k ) = false;
          theLayer->nodeIsTemporarilyDirichlet ( i, j, k ) = false;
        }

      }

    }

  }

}

//------------------------------------------------------------//

void HydraulicFracturingManager::restrictPressure ( const double currentTime ) {

  using namespace Basin_Modelling;

  if ( m_fracturePressureCalculator->getFracturePressureFunctionParameters ().type () == Interface::None or not isNonConservativeFractureModel ()) {
//   if ( m_functionSelection == NO_FRACTURE_PRESSURE or not isNonConservativeFractureModel ()) {
    return;
  }

  if ( m_basinModel->debug1 ) {
    PetscPrintf ( PETSC_COMM_WORLD, " Restricting pressure \n" );
  }

  Layer_Iterator Layers ( m_basinModel->layers, Ascending, Sediments_Only, Active_Layers_Only );

  while ( ! Layers.Iteration_Is_Done ()) {
    restrictPressure ( Layers.Current_Layer (), m_basinModel->getValidNeedles (), currentTime );
    Layers++;
  }

}

//------------------------------------------------------------//

void HydraulicFracturingManager::restrictPressure ( LayerProps*           theLayer,
                                                    const Boolean2DArray& validNeedle,
                                                    const double          currentTime ) {


  int xStart;
  int xCount;
  int yStart;
  int yCount;
  int zStart;
  int zCount;

  int localI;
  int localJ;

  int i;
  int j;
  int k;

  const CompoundLithology*  currentLithology;

  double hydrostaticPressure;
  double lithostaticPressure;
  double porePressure;
  double overpressure;
  double ves;
  double computedFracturePressure;
  double nodeDepth;
  double surfaceDepth;
  double seaTemperature;

  if ( m_basinModel->debug1 ) {
    PetscPrintf ( PETSC_COMM_WORLD, " Restricting pressure for layer: %s \n", theLayer->layername.c_str ());
  }

  DAGetCorners ( theLayer->layerDA, &xStart, &yStart, &zStart, &xCount, &yCount, &zCount );

  theLayer->Current_Properties.Activate_Property ( Basin_Modelling::Depth );
  theLayer->Current_Properties.Activate_Property ( Basin_Modelling::Overpressure );
  theLayer->Current_Properties.Activate_Property ( Basin_Modelling::Pore_Pressure );
  theLayer->Current_Properties.Activate_Property ( Basin_Modelling::Hydrostatic_Pressure );
  theLayer->Current_Properties.Activate_Property ( Basin_Modelling::Lithostatic_Pressure );
  theLayer->Current_Properties.Activate_Property ( Basin_Modelling::VES_FP );

  for ( i = xStart; i < xStart + xCount; ++i ) {
    localI = i - xStart;

    for ( j = yStart; j < yStart + yCount; ++j ) {
      localJ = j - yStart;

      if ( validNeedle ( i, j )) {
        currentLithology = theLayer->getLithology ( i, j );
        surfaceDepth = FastcauldronSimulator::getInstance ().getSeaBottomDepth ( i, j, currentTime );
        seaTemperature = FastcauldronSimulator::getInstance ().getSeaBottomTemperature ( i, j, currentTime );

        for ( k = zStart; k < zStart + zCount; ++k ) {
          porePressure        = theLayer->Current_Properties ( Basin_Modelling::Pore_Pressure, k, j, i );
          overpressure        = theLayer->Current_Properties ( Basin_Modelling::Overpressure, k, j, i );
          hydrostaticPressure = theLayer->Current_Properties ( Basin_Modelling::Hydrostatic_Pressure, k, j, i );
          lithostaticPressure = theLayer->Current_Properties ( Basin_Modelling::Lithostatic_Pressure, k, j, i );
          nodeDepth           = theLayer->Current_Properties ( Basin_Modelling::Depth, k, j, i );
          ves                 = theLayer->Current_Properties ( Basin_Modelling::VES_FP, k, j, i ) / 1.0e6; // Convert to MPa

          computedFracturePressure = fracturePressure ( currentLithology,
                                                        theLayer->fluid,
                                                        seaTemperature,
                                                        surfaceDepth,
                                                        nodeDepth,
                                                        hydrostaticPressure,
                                                        lithostaticPressure );

          if ( porePressure > computedFracturePressure ) {
             porePressure = lithostaticPressure - ves;
            theLayer->Current_Properties ( Basin_Modelling::Pore_Pressure, k, j, i ) = porePressure;
            theLayer->Current_Properties ( Basin_Modelling::Overpressure, k, j, i ) = porePressure - hydrostaticPressure;

#if 0
            theLayer->Current_Properties ( Basin_Modelling::Pore_Pressure, k, j, i ) = computedFracturePressure;
            theLayer->Current_Properties ( Basin_Modelling::Overpressure, k, j, i ) = computedFracturePressure - hydrostaticPressure;
#endif

          }

        }

      }

    }

  }

  theLayer->Current_Properties.Restore_Property ( Basin_Modelling::VES_FP );
  theLayer->Current_Properties.Restore_Property ( Basin_Modelling::Depth );
  theLayer->Current_Properties.Restore_Property ( Basin_Modelling::Overpressure );
  theLayer->Current_Properties.Restore_Property ( Basin_Modelling::Pore_Pressure );
  theLayer->Current_Properties.Restore_Property ( Basin_Modelling::Hydrostatic_Pressure );
  theLayer->Current_Properties.Restore_Property ( Basin_Modelling::Lithostatic_Pressure );
}

//------------------------------------------------------------//

//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 
#ifndef _HydraulicFracturingManager_H_
#define _HydraulicFracturingManager_H_

#include <string>

#include "ConstantsFastcauldron.h"

#include "GeoPhysicsFluidType.h"

#include "FracturePressureFunctionParameters.h"
#include "Interface.h"

#include "FracturePressureCalculator.h"

#include "CompoundLithology.h"

using namespace DataAccess;
using namespace GeoPhysics;


// Forward declarations.
class AppCtx;
class LayerProps;
class PETSC_2D_Array;



/// \name Fracture pressure 
/// @{

/// \var HydraulicFracturingNone
/// Fracture type string for hydraulic fracturing.
///
/// None (No hydraulic fracturing)
static const std::string HydraulicFracturingNone = "None";

/// \var HydraulicFracturingFunctionOfDepthBelowSeaLevel
/// Fracture function is function of depth, starting point is sea level
static const std::string HydraulicFracturingFunctionOfDepthBelowSeaLevel = "FunctionOfDepthBelowSeaLevel";

/// \var HydraulicFracturingFunctionOfDepthBelowSedimentSurface
/// Fracture function is function of depth, starting point is sediments surface
static const std::string HydraulicFracturingFunctionOfDepthBelowSedimentSurface = "FunctionOfDepthBelowSedimentSurface";

/// \var HydraulicFracturingFunctionOfLithostaticPressure
/// Fracture function is function of lithostatic and hydrostatic pressures.
static const std::string HydraulicFracturingFunctionOfLithostaticPressure = "FunctionOfLithostaticPressure";

typedef Interface::FracturePressureFunctionType HydraulicFractureFunctionType;

// /// \brief Enumeration of the fracture function type.
// enum HydraulicFractureFunctionType { 
//   ///< Do not apply any hydraulic fracturing.
//   NO_FRACTURE_PRESSURE,

//   ///< Apply hydraulic fracturing, the function is defined w.r.t. depth from 0m.
//   DEPTH_FUNCTION_ZERO_DEPTH,

//   ///< Apply hydraulic fracturing, the function is defined w.r.t. depth from the top of the sediments.
//   DEPTH_FUNCTION_SEDIMENTS_SURFACE,

//   ///< Apply hydraulic fracturing, the function is defined as a fraction of the lithostatic-pressure
//   ///< Pf = Ph + C * ( Pl - Ph )
//   FUNCTION_OF_LITHOSTATIC_PRESSURE };


/// \brief Return string representation of the fracture function.
// const std::string& hydraulicFractureFunctionTypeStr ( const Interface::FracturePressureFunctionType functionType ); 
// // const std::string& hydraulicFractureFunctionTypeStr ( const HydraulicFractureFunctionType functionType ); 


// /// Enumerations of the hydraulic fracture models.
// enum FracturePressureModel {
//   ///< Totally non-conservative model.
//   NON_CONSERVATIVE_TOTAL = 1,

//   ///< Non-conservative model, but first attempt to restrict the pressure using the conservative method.
//   NON_CONSERVATIVE_PARTIAL_1,

//   ///< Non-conservative model, but first attempt (this time a bit harder)
//   ///< to restrict the pressure using the conservative method.
//   NON_CONSERVATIVE_PARTIAL_2,

//   ///< Conservative model, try to restrict the pressure using the conservative method.
//   CONSERVATIVE_1,

//   ///< Conservative model, try (a bit harder) to restrict the pressure using the conservative method.
//   CONSERVATIVE_2 };


/// \brief Return string representation of the fracture model type.
const std::string& fracturePressureModelStr ( const Interface::FracturePressureModel model );


/// \var DefaultFractureModel
/// If the fracture-model is not specified in the project file, then use this value.
static const Interface::FracturePressureModel DefaultFractureModel = Interface::NON_CONSERVATIVE_TOTAL;

/// \var NumberOfFractureModels
/// The number of different fracture-models.
static const int NumberOfFractureModels = int ( Interface::CONSERVATIVE_2 );

/// @}



/// \brief Handles hydraulic fracturing.
///
/// Initialised from whole or parts of tables in the project file.
class HydraulicFracturingManager {

public :


  static HydraulicFracturingManager& getInstance ();

  static void deleteInstance();

  void setAppCtx ( const AppCtx* newAppCtx );

  /// \brief Set the fracture constraining model.
  bool setModel ( const int newModel );

  /// \brief Return the fracture constraining model that has been set.
  Interface::FracturePressureModel getModel () const;

  /// \brief Return whether the model is a non-conservative model or not.
  bool isNonConservativeFractureModel () const;

  /// \brief Set the function that is to be used for the hydraulic fracturing.
  ///
  /// If this function returns a false then an error has occurred.
  bool setSelectedFunction ();

  /// \brief Return which fracture function has been set.
  Interface::FracturePressureFunctionType fracturePressureSelection () const;


  /// \brief Computes the fracture-pressure.
  double fracturePressure
     ( const CompoundLithology* lithology,
       const GeoPhysics::FluidType*  currentFluid,
       const double      seaTemperature,
       const double      surfaceDepth,
       const double      depth,
       const double      hydrostaticPressure,
       const double      lithostaticPressure ) const;

  /// \brief Determines if the pore-pressure has exceeded the fracture-pressure.
  bool hasFractured
     ( const CompoundLithology* lithology,
       const GeoPhysics::FluidType*  currentFluid,
       const double      seaTemperature,
       const double      surfaceDepth,
       const double      depth,
       const double      hydrostaticPressure,
       const double      lithostaticPressure,
       const double      porePressure ) const;


  /// \brief Check for hydraulic fractring in the domain.
  void checkForFracturing
     ( const double          currentTime,
       const bool            applyNonConservativeModel,
             bool&           layerHasFractured );

  /// \brief Reset all hydraulic fracturing related variables in domain.
  void resetFracturing ();

  /// \brief In the non-conservative case, restrict all pore-pressures to be equal
  /// the fracture-pressure in cases where they may have exceeded it.
  void restrictPressure ( const double currentTime );

  /// \brief Get the maximum number of fracture iterations that can be performed when solving the overpressure.
  int maximumNumberOfFractureIterations () const;

  /// \brief Set the maximum number of fracture iterations that can be performed when solving the overpressure.
  void setMaximumNumberOfFractureIterations ( const int maxIterations );

  /// \brief Get the permeability scaling increase factor.
  ///
  /// Each time a node is fractured during the fracture-iteration the permeability
  /// scaling factor will be increased by this amount.
  double permeabilityScalingIncreaseFactor () const;

  /// \brief Over-ride the default permeability scaling factor.
  void setPermeabilityScalingIncreaseFactor ( const double scalingFactor );


private :

  HydraulicFracturingManager ();

  ~HydraulicFracturingManager ();

  /// \brief Check for hydraulic fractring in the layer.
  void checkForFracturing
     ( LayerProps*           theLayer,
       const double          currentTime,
       const Boolean2DArray& validNeedle,
       const bool            applyNonConservativeModel,
             bool&           layerHasFractured );

  /// \brief Reset all hydraulic fracturing related variables in layer.
  void resetFracturing
     ( LayerProps*           theLayer,
       const Boolean2DArray& validNeedle );

  /// \brief In the non-conservative case, restrict all pore-pressures to be less
  /// than the fracture-pressure in cases where they may have exceeded it in the layer.
  void restrictPressure
     ( LayerProps*           theLayer,
       const Boolean2DArray& validNeedle,
       const double          currentTime );

  /// Array of the number of maximum number of fracture iterations to be performed.
  /// Depends on the fracture model to be used.
  static int NumberOfIterations [ NumberOfFractureModels ];

  /// Array of the permeability scaling factors. 
  /// Depends in the fracture model to be used.
  static double ScalingFactors [ NumberOfFractureModels ];

  // This must be set after the project file has been read.
  const GeoPhysics::FracturePressureCalculator* m_fracturePressureCalculator;

  /// The fracture pressure model that has been selected.
  // Remove when fracture-pressure-model cannot be changed from the command line.
  Interface::FracturePressureModel         m_fractureModel;

  /// A pointer to the application context.
  AppCtx const* m_basinModel;

  static HydraulicFracturingManager* s_hydraulicFracturingManagerInstance;

};

//------------------------------------------------------------//
// Define all functions to be inlined.
//------------------------------------------------------------//

inline bool HydraulicFracturingManager::isNonConservativeFractureModel () const {
   return m_fractureModel <= Interface::NON_CONSERVATIVE_PARTIAL_2;
}

//------------------------------------------------------------//

inline Interface::FracturePressureModel HydraulicFracturingManager::getModel () const {
  return m_fractureModel;
}

//------------------------------------------------------------//

inline Interface::FracturePressureFunctionType HydraulicFracturingManager::fracturePressureSelection () const {
	if (m_fracturePressureCalculator->getFracturePressureFunctionParameters())
		return m_fracturePressureCalculator->getFracturePressureFunctionParameters()->type();
	else
		return DataAccess::Interface::FracturePressureFunctionType::None;
}

//------------------------------------------------------------//


#endif // _HydraulicFracturingManager_H_

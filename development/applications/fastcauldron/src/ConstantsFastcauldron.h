//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

#ifndef CONSTANTS_FASTCAULDRON_H
#define CONSTANTS_FASTCAULDRON_H

// std library
#include <vector>
#include <set>
#include <string>

// Geophysics library
#include "Local3DArray.h"

// utilities library
#include "array.h"

// fastcauldron application
#include "ghost_array.h"

using std::string;

#ifndef WIN32
constexpr bool NO_ERROR = true;
#endif

//------------------------------------------------------------//
// Geological constants

/// \var RockViscosityReferenceTemperature
/// The temperature used in the reference rock viscosity, units Celcius.
constexpr double RockViscosityReferenceTemperature = 15.0;

/// \var DefaultVesScalingForFctInitialisation
/// \brief The default value used to scale the ves when initialising the fct's.
constexpr double DefaultVesScalingForFctInitialisation = 0.5;

/// \var DefaultCrustThinningModel
/// Default value for the crust-thinning model.
constexpr int DefaultCrustThinningModel = 1;

/// \var StandardDepth
/// \brief The depth at which it is determined if a formation is a low permeability.
constexpr double StandardDepth = 2000.0;

/// \var StandardTemperatureGradient
/// \brief The standard temperature gradient for determining low perm formations, 30C/km.
constexpr double StandardTemperatureGradient = 30.0;

/// \var StandardStressGradient
/// \brief The standard stress gradient for determining low perm formations, 6MPa/km
constexpr double StandardStressGradient = 6.0;

/// \var DefaultMcfMaximumPermeability
/// \brief The default value for the maximum permeability of layer to be used in mcf calculator.
constexpr double DefaultMcfMaximumPermeability = 2.0e-4;

/// \var DefaultMaximumHCFractionForFlux
/// \brief The fraction of total-HC that can be "fluxed" through the element.
constexpr double DefaultMaximumHCFractionForFlux = 1.0;

/// \var DefaultResidualHcSaturationScaling
/// \brief The scaling to be used for the Sor when estimating whether or not transport has occurred in the Darcy simulator.
constexpr double DefaultResidualHcSaturationScaling = 0.75;

/// \brief The lower limit of hc-cnocentration for which any calculation is performed in the Darcy simulator.
constexpr double HcConcentrationLowerLimit = 1.0e-20;

/// \var DefaultMinimumHcSaturation
/// \brief The default value for the minimum hc-saturation.
constexpr double DefaultMinimumHcSaturation = 0.02;

/// \var DefaultMinimumWaterSaturation
/// \brief The default value for the minimum water-saturation.
constexpr double DefaultMinimumWaterSaturation = 0.1;

/// \var DefaultWaterCurveExponent
/// \brief The default value for the water-curve exponent.
constexpr double DefaultWaterCurveExponent = 2.0;

/// \var DefaultHcCurveExponent
/// \brief The default value for the hc-curve exponent.
constexpr double DefaultHcCurveExponent = 2.0;

//------------------------------------------------------------//
// Numerical constants

/// \var DefaultMcfAdaptiveTimeStepFraction
/// \brief The fraction of the time-step size gien by the CFL value.
constexpr double DefaultMcfAdaptiveTimeStepFraction = 0.05;

/// \var DefaultMaximumTimeStep
constexpr double DefaultMaximumTimeStep = 1.0e9;

/// \var DefaultAgeTolerance
/// The relative tolerance used when comparing ages.
constexpr double DefaultAgeTolerance = 1.0e-6;

/// \var Minimum_Pressure_Time_Step
/// Minimum time step of 1000 years, 0.001 Ma = 1000 years.
constexpr double Minimum_Pressure_Time_Step = 0.001;

/// \var DefaultQuadratureDegree
/// \brief The default value for any numerical quadrature.
constexpr int DefaultQuadratureDegree = 2;

/// \brief Quadrature degree for face-flux integrals.
constexpr int DefaultFaceQuadratureDegree = 2;

/// \brief Quadrature degree for 'previous term' integrals.
///
/// This term also include the derivative of the porosity.
constexpr int DefaultPreviousContributionTermQuadratureDegree = 3;

/// \brief Quadrature degree for source-term integrals.
constexpr int DefaultSourceTermQuadratureDegree = 2;

/// \brief Quadrature degree for mass-matrix integrals.
constexpr int DefaultMassMatrixQuadratureDegree = 3;

/// \brief The quadrature degree that is used when computing the interpolators.
constexpr int DarcyInterpolatorQuadratureDegree = 3;

/// \brief The number of nodes that make up a hexahedral element.
constexpr int NumberOfElementNodes = 8;

/// \var DepositingThicknessTolerance
/// \brief The minimum thickness.
constexpr double DepositingThicknessTolerance = 0.1;

/// \var DefaultMaximumGradPressure
/// \brief The maximum grad pressure in Pa.
///
/// Used only if values are to be clipped to the maximum value.
/// grad-pressure should then lie in the interval [ -val, val ].
constexpr double DefaultMaximumGradPressure = 30000.0;

/// \var DefaultFluxPermeabilityMaximum
/// \brief The maximum permeability to be used when calculating the fluxes.
///
/// Used only if the permeability is to be limited.
/// Units in milli-darcy.
constexpr double DefaultFluxPermeabilityMaximum = 1.0e-3;

/// \var DefaultDarcyTimeStepSmoothingFactor
/// \brief The maximum factor by which the darcy time-step size can increase.
constexpr double DefaultDarcyTimeStepSmoothingFactor = 1.5;

/// \var DefaultNumberOfMinorSnapshots
/// The default maximum number of minor snapshots that can lie in a snapshot interval.
constexpr int DefaultNumberOfMinorSnapshots = 5;

/// \var MaximumInitialElementDeposition
/// The maximum portion of an element that can be deposited at the start of 
/// layer deposition. Only used in the overpressure and coupled calculation.
constexpr double MaximumInitialElementDeposition = 0.5;

/// \var DefaultElementBurialFraction
/// The default amount of element that will be buried.
constexpr double DefaultElementBurialFraction = 1.0;

/// \var DefaultElementErosionFraction
/// The default amount of standard element that will be eroded.
// Is there a way to write the Ada equivalent 3#0.1# = Third in base 3.
constexpr double DefaultElementErosionFraction = 0.3333333333333;

/// \var DefaultElementHiatusFraction
/// The default scaling factor to number of time-steps over the hiatus interval.
constexpr double DefaultElementHiatusFraction = 0.3333333333333;

/// \var Element_Scaling
/// The scaling factor here is used to scale the element jacobian and residual.
/// It is necessary when we have very small permeabilities to stop PETSc from 
/// complaining or to force a linear solve in the PETSc.
constexpr double Element_Scaling = 1.0e10;

/// \var Dirichlet_Scaling_Value
/// Scaling factor used to indicate a node subject to Dirichlet boundary conditions.
constexpr double Dirichlet_Scaling_Value = 1.0e25;

/// \var NumberOfOptimisationLevels
/// The number of optimisation levels that have been defined.
constexpr int NumberOfOptimisationLevels = 5;

//------------------------------------------------------------//
// Naming constants

/// \name Run status strings
/// @{

/// \var DecompactionRunStatusStr
/// Values of MCStatusOfLastRun (RunStatusIoTbl) indicating 
/// last calculation mode.
///
/// Hydrostatic decompaction
const string DecompactionRunStatusStr               = "HydrostaticDecompaction";

/// \var HighResDecompactionRunStatusStr
/// High resolution decompaction, possibly using the overpressure results
const string HighResDecompactionRunStatusStr        = "HighResDecompaction";

/// \var OverpressureRunStatusStr
/// Overpressure calculation
const string OverpressureRunStatusStr               = "Overpressure";

/// \var OverpressuredTemperatureRunStatusStr
/// Loosely coupled temperature calculation.
const string OverpressuredTemperatureRunStatusStr   = "OverpressuredTemperature";

/// \var HydrostaticTemperatureRunStatusStr
/// Hydrostatic temperature calculation.
const string HydrostaticTemperatureRunStatusStr     = "HydrostaticTemperature";

/// \var CoupledPressureTemperatureRunStatusStr
/// Iteratively coupled presure and temperature calculation.
const string CoupledPressureTemperatureRunStatusStr = "PressureAndTemperature";

/// \var HydrostaticDarcyRunStatusStr
/// Hydrostatic temperature with Darcy flow calculation.
const string HydrostaticDarcyRunStatusStr           = "HydrostaticDarcy";

/// \var CoupledDarcyRunStatusStr
/// Iteratively coupled presure and temperature and Darcy flow calculation.
const string CoupledDarcyRunStatusStr               = "CoupledDarcy";

/// @}

//------------------------------------------------------------//

/// \name HDF5 file attributes
/// @{

/// \var LAYER_DATASET_PREFIX
/// \brief Prefix of all layer map properties.
const string LAYER_DATASET_PREFIX  = "/Layer=";

/// \var DELTA_I_DATASET_NAME
/// \brief Delta-x of input map.
const string DELTA_I_DATASET_NAME  = "/delta in I dimension";

/// \var DELTA_J_DATASET_NAME
/// \brief Delta-y of input map.
const string DELTA_J_DATASET_NAME  = "/delta in J dimension";

/// \var NR_I_DATASET_NAME
/// \brief Number of nodes in x-direction.
const string NR_I_DATASET_NAME     = "/number in I dimension";

/// \var NR_J_DATASET_NAME
/// \brief Number of nodes in y-direction.
const string NR_J_DATASET_NAME     = "/number in J dimension";

/// \var ORIGIN_I_DATASET_NAME
/// \brief Origin in x-direction.
const string ORIGIN_I_DATASET_NAME = "/origin in I dimension";

/// \var ORIGIN_J_DATASET_NAME
/// \brief Origin in y-direction.
const string ORIGIN_J_DATASET_NAME = "/origin in J dimension";

/// @}

//------------------------------------------------------------//

/// \name Snapshot related strings
/// @{

/// \var SystemGeneratedSnapshotStr
/// Text used in TypeOfSnapshot field in SnapshotIoTbl
const string SystemGeneratedSnapshotStr = "System Generated";

/// \var MinorSnapshotFileName
/// Part of file name used to store snapshot data
/// for both minor and major snapshot times
const string MinorSnapshotFileName     = "MinorTime";

/// \var MajorSnapshotFileName
const string MajorSnapshotFileName     = "MajorTime";

/// @}

//------------------------------------------------------------//
// Enumerations


/// \brief Fastcauldron Pressure/Temperature calculation modes
enum CalculationMode
{

  /// Decompaction assuming a hydrostatic pore-pressure
  HYDROSTATIC_DECOMPACTION_MODE,

  /// High-resolution decompaction assuming a hydrostatic pore-pressure
  HYDROSTATIC_HIGH_RES_DECOMPACTION_MODE,

  /// Temperature assuming a hydrostatic pore-pressure
  HYDROSTATIC_TEMPERATURE_MODE, 

  /// Pressure calculation assuming a linear temperature gradient
  OVERPRESSURE_MODE,

  /// Temperature calculation using the data from a previous overpressure calculation, i.e. loosely-coupled
  OVERPRESSURED_TEMPERATURE_MODE,

  /// High-resolution decompaction using the results from a previous 
  ///    loosely-coupled or fully-coupled  calculation
  COUPLED_HIGH_RES_DECOMPACTION_MODE,

  /// Fully-coupled PT calculation.
  PRESSURE_AND_TEMPERATURE_MODE,

  /// Hydrostatic temperature with Darcy flow calculation
  HYDROSTATIC_DARCY_MODE,

  /// Fully-coupled PT with Darcy flow calculation
  COUPLED_DARCY_MODE,

  /// No calculation selected
  NO_CALCULATION_MODE
};

/// \brief The number of calculation modes
constexpr int NumberOfCalculationModes = int ( NO_CALCULATION_MODE + 1 );

/// \brief Get the string representation of the calculation mode enumeration.
const std::string& getSimulationModeString ( const CalculationMode mode );

/// \brief Get the calculation mode enumeration from its string representation.
CalculationMode getSimulationMode ( const std::string& modeStr );

//------------------------------------------------------------//

/// \brief An indicator of the status after a Darcy time-step calculation
enum DarcyErrorIndicator {
   NO_DARCY_ERROR,                  ///< No error has occurred
   ERROR_CALCULATING_SATURATION,    ///< Error when calculating the saturations, probably as a result of negative concentrations
   ERROR_CALCULATING_CONCENTRATION, ///< Error when calculating the concentrations, probably as a result of negative concentrations.
   UNKNOWN_DARCY_ERROR              ///< This one must be the last in the list.
};

//------------------------------------------------------------//

enum Interpolation_Method {
   PIECEWISE_LINEAR,
   CUBIC_SPLINE
};

//------------------------------------------------------------//

enum SurfacePosition {
   TOPSURFACE,
   BOTTOMSURFACE
};

//------------------------------------------------------------//

enum Boundary_Conditions {
   Interior_Boundary,
   Interior_Constrained_Overpressure,
   Surface_Boundary,
   Bottom_Boundary,
   Bottom_Boundary_Flux,
   Interior_Neumann_Boundary,
   Side_Neumann_Boundary,
   Bottom_Neumann_Boundary,
   Interior_Constrained_Temperature
};

/// \retyrn The string representation of the boundary conditions enumeration
const std::string& getBoundaryConditionsImage ( const Boundary_Conditions bcs );

//------------------------------------------------------------//
// TypeDefs

typedef ibs::Array2D <double> Double_Array_2D;

typedef PETSc_Local_2D_Array <bool> Boolean2DArray;

typedef PETSc_Local_3D_Array <bool> Boolean3DArray;

typedef GeoPhysics::Local3DArray<bool> LocalBooleanArray3D;

typedef GeoPhysics::Local3DArray<int> LocalIntegerArray3D;

typedef std::vector<int> IntegerArray;

#endif // CONSTANTS_FASTCAULDRON_H
 

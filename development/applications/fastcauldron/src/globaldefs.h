#ifndef _GLOBALDEFS_H_
#define _GLOBALDEFS_H_

#include <set>

#include <string>
using std::string;

#include "ghost_array.h"

#include <assert.h>
#include "array.h"

const bool   NO_ERROR              = true;

const string BLANKQUOTE            = "";

const int    MAXLINESIZE            = 512;
const int    Null                   = 0;

const double IBSNULLVALUE           = -9999;
const double CAULDRONIBSNULLVALUE   = 99999;

/// \var GRAVITY
/// Acceleration due to gravity in m/s^2, more accurate value: 9.80665 m /s^2.
const double GRAVITY = 9.81;

// /// \var MicroWattsToWatts
// /// Number of Watts in a microWatt.
// const double MicroWattsToWatts = 0.000001;

/// \var MilliWattsToWatts
/// Number of Watts in a milliWatt.
const double MilliWattsToWatts = 0.001;

/// \var RockViscosityReferenceTemperature
/// The temperature used in the reference rock viscosity, units Celcius.
const double RockViscosityReferenceTemperature = 15.0; 

/// \var Gas_Constant
/// Units are J . K^-1 . mol^-1.
const double Gas_Constant = 8.314472;

/// \var Minimum_Pressure_Time_Step
/// Minimum time step of 1000 years, 0.001 Ma = 1000 years.
const double Minimum_Pressure_Time_Step = 0.001; 

/// \var Minimum_Porosity
/// The minimum porosity allowed for Soil Mechanics type lithologies,
/// this includes the porosity loss in chemical compaction.
const double Minimum_Porosity           = 0.03;

const double PRESSURE_AT_SEA_TOP    = 0.1;    /* value in MPa */
const double DENSITY_FOR_STANDARD_WATER = 1000.0;  // Kg m^-3

/// \var MaximumReasonableCrustThinningRatio
/// \brief A reasonable maximum ratio of the crust thickness differences.
///
/// There is no basis for choosing the value of this number, other than the number of elements
/// that will be used in the mantle. Numbers smaller than this would also be possible.
/// For this reason if the computed thinning-ratio is greater than the value here, it
/// should result in a warning and not an error condtion.
const double MaximumReasonableCrustThinningRatio = 10.0;

/// \var DefaultVesScalingForFctInitialisation
/// \brief The default value used to scale the ves when initialising the fct's.
const double DefaultVesScalingForFctInitialisation = 0.5;

/// \var DefaultCrustThinningModel
/// Default value for the crust-thinning model.
const int DefaultCrustThinningModel = 1;

/// \var MaximumCrustThinningRatio
/// The maximum allowable crust thinning ration when using thinning model 2.
const double MaximumCrustThinningRatio = 22026.46579; // exp ( 10 );



/// \var DefaultQuadratureDegree
/// \brief The default value for any numerical quadrature.
const int DefaultQuadratureDegree = 2;

/// \brief Quadrature degree for face-flux integrals.
const int DefaultFaceQuadratureDegree = 2;

/// \brief Quadrature degree for 'previous term' integrals.
///
/// This term also include the derivative of the porosity.
const int DefaultPreviousContributionTermQuadratureDegree = 3;

/// \brief Quadrature degree for source-term integrals.
const int DefaultSourceTermQuadratureDegree = 2;

/// \brief Quadrature degree for mass-matrix integrals.
const int DefaultMassMatrixQuadratureDegree = 3;

/// \brief The quadrature degree that is used when computing the interpolators.
const int DarcyInterpolatorQuadratureDegree = 3;



/// \var DepositingThicknessTolerance
/// \brief The minimum thickness.
const double DepositingThicknessTolerance = 0.001;


/// \var DefaultMaximumGradPressure
/// \brief The maximum grad pressure in Pa.
///
/// Used only if values are to be clipped to the maximum value.
/// grad-pressure should then lie in the interval [ -val, val ].
const double DefaultMaximumGradPressure = 30000.0;

/// \var DefaultFluxPermeabilityMaximum
/// \brief The maximum permeability to be used when calculating the fluxes.
///
/// Used only if the permeability is to be limited.
/// Units in milli-darcy.
const double DefaultFluxPermeabilityMaximum = 1.0e-3;

/// \var DefaultDarcyTimeStepSmoothingFactor
/// \brief The maximum factor by which the darcy time-step size can increase.
const double DefaultDarcyTimeStepSmoothingFactor = 1.5;

//------------------------------------------------------------//

/// \var StandardDepth
/// \brief The depth at which it is determined if a formation is a low permeability.
const double StandardDepth = 2000.0;

/// \var StandardTemperatureGradient
/// \brief The standard temperature gradient for determining low perm formations, 30C/km.
const double StandardTemperatureGradient = 30.0;

/// \var StandardStressGradient
/// \brief The standard stress gradient for determining low perm formations, 6MPa/km
const double StandardStressGradient = 6.0;

/// \var DefaultMcfMaximumPermeability
/// \brief The default value for the maximum permeability of layer to be used in mcf calculator.
const double DefaultMcfMaximumPermeability = 2.0e-4;

/// \var DefaultMaximumHCFractionForFlux
/// \brief The fraction of total-HC that can be "fluxed" through the element.
const double DefaultMaximumHCFractionForFlux = 1.0;

/// \var DefaultResidualHcSaturationScaling
/// \brief The scaling to be used for the Sor when estimating whether or not transport has occurred in the Darcy simulator.
const double DefaultResidualHcSaturationScaling = 0.75;

/// \brief The lower limit of hc-cnocentration for which any calculation is performed in the Darcy simulator.
const double HcConcentrationLowerLimit = 1.0e-20;

/// \var DefaultMcfAdaptiveTimeStepFraction
/// \brief The fraction of the time-step size gien by the CFL value.
const double DefaultMcfAdaptiveTimeStepFraction = 0.05;

/// \var DefaultMaximumTimeStep
const double DefaultMaximumTimeStep = 1.0e9;

/// \var DefaultAgeTolerance
/// The relative tolerance used when comparing ages.
const double DefaultAgeTolerance = 1.0e-6;

/// \var MinimumWaterRelativePermeability
/// \brief The minimum value for the water-saturation.
const double MinimumWaterRelativePermeability = 1.0e-2;

/// \var MinimumHcRelativePermeability
/// \brief The minimum value for the hydrocarbon-saturation.
const double MinimumHcRelativePermeability = 1.0e-2;

/// \var DefaultMinimumHcSaturation
/// \brief The default value for the minimum hc-saturation.
const double DefaultMinimumHcSaturation = 0.02;

/// \var DefaultMinimumWaterSaturation
/// \brief The default value for the minimum water-saturation.
const double DefaultMinimumWaterSaturation = 0.1;

/// \var DefaultWaterCurveExponent
/// \brief The default value for the water-curve exponent.
const double DefaultWaterCurveExponent = 2.0;

/// \var DefaultHcCurveExponent
/// \brief The default value for the hc-curve exponent.
const double DefaultHcCurveExponent = 2.0;

//------------------------------------------------------------//

/// \name VRe names and constants.
/// @{

const double LOPATINREFTEMP         = 105.0;
const double LOPATINTEMPINC         = 10.0;
const double LOPATINFACTOR          = 2.0;
const double INITIALVRE             = 0.2;
const double MODIFIEDLOPATINFACTOR  = 5.1;
const double MODIFIEDLOPATINP       = 0.0821;
const double MODIFIEDLOPATINQ       = 0.6250;
const double LOPATINGOFFP           = 0.20799;
const double LOPATINGOFFQ           = 1.08680;
const double LOPATINHOODP           = 0.17340;
const double LOPATINHOODQ           = 0.98875;
const double LOPATINISSLERP         = 0.16171;
const double LOPATINISSLERQ         = 0.98882;
const double LOPATINROYDENP         = 0.17300;
const double LOPATINROYDENQ         = 0.80360;
const double LOPATINWAPPLESP        = 0.24300;
const double LOPATINWAPPLESQ        = 1.01772;
const double LOPATINDYKSTRAP        = 0.23697;
const double LOPATINDYKSTRAQ        = 1.04431;

const string VRALGBURNHAMSWEENEY    = "Burnham & Sweeney";
const string VRALGLOPATINMODIFIED   = "Modified Lopatin";
const string VRALGLARTER            = "Larter";
const string VRALGLOPATINDYKSTRA    = "Lopatin & Dykstra";
const string VRALGLOPATINWAPPLES    = "Lopatin & Wapples";
const string VRALGLOPATINROYDEN     = "Lopatin & Royden";
const string VRALGLOPATINISSLER     = "Lopatin & Issler";
const string VRALGLOPATINHOOD       = "Lopatin & Hood";
const string VRALGLOPATINGOFF       = "Lopatin & Goff";

/// @}

//------------------------------------------------------------//

/// \var GardnerVelocityConstant
/// Constant used in computation of the Gardner velocity-density velocity algorithm.
const double GardnerVelocityConstant = 309.4;

/// \var GardnersVelocityDensityAlgorithm
/// Gardners velocity-density velocity algorithm.
const string GardnersVelocityDensityAlgorithm = "Gardner\'s Velocity-Density";

/// \var WylliesTimeAverageAlgorithm
/// Wyllie's Time-Average velocity algorithm.
const string WylliesTimeAverageAlgorithm = "Wyllie\'s Time-Average";

/// \var DefaultVelocityAlgorithm
/// The default velocity-algorithm to be used if none is specified in the project file.
const string DefaultVelocityAlgorithm = GardnersVelocityDensityAlgorithm;

// /// Indicates which velocity algorithm should be used.
// enum VelocityAlgorithm { GARDNERS_VELOCITY_ALGORITHM, WYLLIES_VELOCITY_ALGORITHM };

// const std::string& velocityAlgorithmImage ( const VelocityAlgorithm vel );

// VelocityAlgorithm velocityAlgorithmValue ( const std::string& algorithmName );

//------------------------------------------------------------//


const double Zero                   = 0.0;
const double NegOne                 = -1.0;
const double One                    = 1.0;
const double Hundred                = 100.0;
const double EPS1                   = 0.1;
const double TempNLSolverTolerance  = 1.0E-06;
const double TempNLSolverMaxIter    = 20;
const double PressNLSolverTolerance = 1.0E-06;
const double Log_10                 = 2.3025850929940459;
const double Present_Day            = 0.0;

// /// \var MaximumNumberOfLithologies
// /// The maximum number of lithologies that can be named in a formation.
// const int MaximumNumberOfLithologies = 3;

/// \var DefaultNumberOfMinorSnapshots
/// The default maximum number of minor snapshots that can lie in a snapshot interval.
const int DefaultNumberOfMinorSnapshots = 5;

/// \var MaximumInitialElementDeposition
/// The maximum portion of an element that can be deposited at the start of 
/// layer deposition. Only used in the overpressure and coupled calculation.
const double MaximumInitialElementDeposition = 0.5;

/// \var DefaultElementBurialFraction
/// The default amount of element that will be buried.
const double DefaultElementBurialFraction = 1.0;

/// \var DefaultElementErosionFraction
/// The default amount of standard element that will be eroded.
// Is there a way to write the Ada equivalent 3#0.1# = Third in base 3.
const double DefaultElementErosionFraction = 0.3333333333333; 

/// \var DefaultElementHiatusFraction
/// The default scaling factor to number of time-steps over the hiatus interval.
const double DefaultElementHiatusFraction = 0.3333333333333; 

/// \var Element_Scaling
/// The scaling factor here is used to scale the element jacobian and residual.
/// It is necessary when we have very small permeabilities to stop PETSc from 
/// complaining or to force a linear solve in the PETSc.
const double Element_Scaling = 1.0e10;

/// \var Dirichlet_Scaling_Value
/// Scaling factor used to indicate a node subject to Dirichlet boundary conditions.
const double Dirichlet_Scaling_Value = 1.0e25;

/// \var DefaultHydrostaticPressureGradient
/// Assumed pressure gradient used when computing the simple fluid density.
/// Units are MPa/km.
const double DefaultHydrostaticPressureGradient = 10.0; 

/// \var StandardSurfaceTemperature
/// Assumed surface temperature when computing the simple fluid density.
/// Units are Celcius.
const double StandardSurfaceTemperature = 10.0;

/// \var DefaultStandardDepth
/// Assumed 'standard' depth at which the parameters for the fluid density 
/// are evaluated at.
/// Units are Metres.
const double DefaultStandardDepth = 2000.0;

/// \var GMRes_Restart_Value
// The default restart is 30, this is not enough for some (most?) pressure problems.
// It will also help the steady state computation for the temperature.
const int GMRes_Restart_Value = 120;

//------------------------------------------------------------//

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
const string HydrostaticDarcyRunStatusStr = "HydrostaticDarcy";

/// \var CoupledDarcyRunStatusStr
/// Iteratively coupled presure and temperature and Darcy flow calculation.
const string CoupledDarcyRunStatusStr = "CoupledDarcy";

/// @}


//------------------------------------------------------------//

/// \name Exit status values
/// @{

/// \var GeometricLoopNonConvergence
/// \brief If the input layer thicknesses do not converge in the geometric loop pressure/coupled simulation then return this error value.
const int GeometricLoopNonConvergenceExitStatus = 2;

/// @}

//------------------------------------------------------------//

/// \name Unit test data
/// @{

/// \var UnitTest1FileName
/// Part of the name of the file that will contain the result of unit test 1.
///
/// The whole name is intended to be 'project-file-name'_'#-procs'_UnitTest1FileName.
const std::string UnitTest1FileName = "unit_test_1.txt";

/// \var UnitTest3FileName
/// Part of the name of the file that will contain the result of unit test 3.
const std::string UnitTest3FileName = "unit_test_3_residuals.txt";

/// \var UnitTest3MatrixFileName
/// Part of the name of the file that will contain the result of unit test 3.
const std::string UnitTest3MatrixFileName = "unit_test_3_matrix.txt";


/// @}

//------------------------------------------------------------//

/// \var FixedHeatFlowStr
/// Value  stored in project file indicating heat flow boundary conditions.
const string FixedHeatFlowStr = "Fixed HeatFlow";

/// \var FixedTemperatureStr
/// Value  stored in project file indicating dirichlet (temperature) bottom boundary conditions and crustal thinning.
const string FixedTemperatureStr = "Fixed Temperature";

//------------------------------------------------------------//

/// \var OneDHiatusLithologyName
/// Name of the lithology that is to be used in a formation that has not lithologies defined.
///
/// To be used in one-dimension only. The actual value used here is irrelevant, as
/// long as the simple-lithology exists, since the lithology itself is not used.
const std::string OneDHiatusLithologyName = "Std. Sandstone";

/// \var FaultDefaultSealingLithologyName
/// Value stored in project file indicating the fault lithology type.a
const std::string FaultDefaultSealingLithologyName = "Default: Sealing litho.";

/// \var FaultDefaultPassingLithologyName
/// Value stored in project file indicating the fault lithology type.a
const std::string FaultDefaultPassingLithologyName = "Default: Passing litho.";

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

/// \var NumberOfOptimisationLevels
/// The number of optimisation levels that have been defined.
const int NumberOfOptimisationLevels = 5;

/// \name Snapshot related strings
/// @{

/// \var SystemGeneratedSnapshotStr
/// Text used in TypeOfSnapshot field in SnapshotIoTbl
const string SystemGeneratedSnapshotStr = "System Generated";

/// \var MinorSnapshotFileName
/// Part of file name used to store snapshot data
/// for both minor and major snapshot times
const string MinorSnapshotFileName = "MinorTime";

/// \var MajorSnapshotFileName
const string MajorSnapshotFileName = "MajorTime";

/// @}

//------------------------------------------------------------//

enum CalculationMode { 

  /// Decompaction assuming a hydrostatic pore-pressure.
  HYDROSTATIC_DECOMPACTION_MODE,

  /// High-resolution decompaction assuming a hydrostatic pore-pressure.
  HYDROSTATIC_HIGH_RES_DECOMPACTION_MODE,

  /// Temperature assuming a hydrostatic pore-pressure.
  HYDROSTATIC_TEMPERATURE_MODE, 

  /// Pressure calculation assuming a linear temperature gradient.
  OVERPRESSURE_MODE,

  /// Temperature calculation using the data from a previous overpressure calculation, i.e. loosely-coupled.
  OVERPRESSURED_TEMPERATURE_MODE,

  /// High-resolution decompaction using the results from a previous 
  /// loosely-coupled or fully-coupled  calculation.
  COUPLED_HIGH_RES_DECOMPACTION_MODE,

  /// Fully-coupled PT calculation.
  PRESSURE_AND_TEMPERATURE_MODE,

  /// Hydrostatic temperature with Darcy flow calculation.
  HYDROSTATIC_DARCY_MODE,

  /// Fully-coupled PT with Darcy flow calculation.
  COUPLED_DARCY_MODE,

  /// No calculaction selected.
  NO_CALCULATION_MODE };

const int NumberOfCalculationModes = int ( NO_CALCULATION_MODE + 1 );

const std::string CalculationModeImage [ NumberOfCalculationModes ] = { "Hydrostatic_Decompaction",
                                                                        "Hydrostatic_High_Res_Decompaction",
                                                                        "Hydrostatic_Temperature",
                                                                        "Overpressure",
                                                                        "Overpressured_Temperature",
                                                                        "Coupled_High_Res_Decompaction",
                                                                        "Pressure_And_Temperature",
                                                                        "HydrostaticDarcy",
                                                                        "CoupledDarcy",
                                                                        "No_Calculaction" };

#define IBSASSERT(t) assert(t);

/// \brief An indicator of the status after a Darcy time-step calculation.
enum DarcyErrorIndicator { // No error has occurred.
                           NO_DARCY_ERROR,

                           // Error when calculating the saturations, probably as a result of negative concentrations.
                           ERROR_CALCULATING_SATURATION,

                           // Error when calculating the concentrations, probably as a result of negative concentrations.
                           ERROR_CALCULATING_CONCENTRATION,

                           // This one must be the last in the list.
                           UNKNOWN_DARCY_ERROR };


typedef enum {PIECEWISE_LINEAR, CUBIC_SPLINE} Interpolation_Method;

typedef enum {EXPONENTIAL, SOIL_MECHANICS} PorosityModelType;

const std::string PorosityModelTypeImage [ 2 ] = { "Exponential",
                                                   "Soil Mechanics"};

typedef enum {CONST = 1, CALC, TABLE, UNSPECIFIED} ModelType;

// typedef enum {HEATFLOW , FIXED_TEMPERATURE} BottomBoundaryType;

typedef enum {SECONDS, MINUTES, HOURS} Time;

enum SurfacePosition {TOPSURFACE, BOTTOMSURFACE};

enum OutputFormat{FCMAP,FCVOLUME,FCMAPandVOLUME};

enum Resolution{HIGHRES,LOWRES};

enum ModelArea{ON,OFF};

enum Boundary_Conditions { Interior_Boundary,
			   Interior_Constrained_Overpressure,
			   Surface_Boundary,
			   Bottom_Boundary,
			   Bottom_Boundary_Flux,
			   Interior_Neumann_Boundary,
			   Side_Neumann_Boundary,
			   Bottom_Neumann_Boundary,
               Interior_Constrained_Temperature };

//  enum Boundary_Conditions { None,
//                             Dirichlet_Boundary,
//                             Neumann_Boundary };


enum RelativePermeabilityType { NO_RELATIVE_PERMEABILITY_FUNCTION, /// Use a rel-perm of 1.
                                TEMIS_PACK_RELATIVE_PERMEABILITY_FUNCTION, /// Use the Temis pack rel-prem functions
                                ANNETTE_RELATIVE_PERMEABILITY_FUNCTION,
                                UNKNOWN_RELATIVE_PERMEABILITY_FUNCTION }; /// Use the rel-perm proposed by Annette.

/// \var DefaultRelativePermeabilityFunction
/// \brief The defualt value for the relative-premeability function.
const RelativePermeabilityType DefaultRelativePermeabilityFunction = ANNETTE_RELATIVE_PERMEABILITY_FUNCTION;

/// \brief Return the string representation of the relative-premeability function type.
const std::string& RelativePermeabilityTypeImage ( const RelativePermeabilityType type );

/// \brief Return the relative-premeability function type frmo the string.
RelativePermeabilityType RelativePermeabilityTypeValue ( const std::string& type );


typedef std::set< int, std::less<int> > Node_Manager;

typedef ibs::Array2D <double> Double_Array_2D;

typedef ibs::Array3D <double> Double_Array_3D;


typedef ibs::Array2D <int> Integer_Array_2D;

typedef PETSc_Local_2D_Array <bool> Boolean2DArray;

typedef PETSc_Local_3D_Array <bool> Boolean3DArray;


//------------------------------------------------------------//

// inline const std::string& velocityAlgorithmImage ( const VelocityAlgorithm vel ) {

//    static const std::string UnknownAlgorithmName = "UnknownAlgorithm";

//    if ( vel == GARDNERS_VELOCITY_ALGORITHM ) {
//       return GardnersVelocityDensityAlgorithm;
//    } else if ( vel == WYLLIES_VELOCITY_ALGORITHM ) {
//       return WylliesTimeAverageAlgorithm;
//    } else {
//       return UnknownAlgorithmName;
//    }

// }

// inline VelocityAlgorithm velocityAlgorithmValue ( const std::string& algorithmName ) {

//    if ( algorithmName == GardnersVelocityDensityAlgorithm ) {
//       return GARDNERS_VELOCITY_ALGORITHM;
//    } else if ( algorithmName == WylliesTimeAverageAlgorithm ) {
//       return WYLLIES_VELOCITY_ALGORITHM;
//    } else {
//       return GARDNERS_VELOCITY_ALGORITHM;
//    }

// }


#endif // GLOBALDEFS
 

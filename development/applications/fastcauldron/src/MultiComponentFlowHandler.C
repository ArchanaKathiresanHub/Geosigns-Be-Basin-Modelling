//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include "MultiComponentFlowHandler.h"

#include "Quadrature.h"
#include "NumericFunctions.h"

#include "RunParameters.h"
#include "FastcauldronSimulator.h"
#include "ConstantsFastcauldron.h"
#include "layer.h"
#include "Interface/RunParameters.h"

#ifdef _MSC_VER
#define strcasecmp _stricmp
#endif
//------------------------------------------------------------//

const std::string& MultiComponentFlowHandler::getErrorString ( const DarcyErrorIndicator id ) {

   const int NumberOfErrorEnumerations = static_cast<int>(UNKNOWN_DARCY_ERROR) + 1;

   static std::string names [ NumberOfErrorEnumerations ];

   static bool initialised = false;

   if ( not initialised ) {

      std::stringstream buffer;

      // Set NO_ERROR string.
      buffer.str ( "" );
      buffer << " No error. ";
      names [ NO_DARCY_ERROR ] = buffer.str ();

      //--------------------------------//

      // Set ERROR_CALCULATING_SATURATION string.
      buffer.str ( "" );
      buffer << " Basin_Error:  Error when calculating the saturations. " << std::endl;
      buffer << " Basin_Error:  To fix this try reducing the Darcy time-step size." << std::endl;

      names [ ERROR_CALCULATING_SATURATION ] = buffer.str ();

      //--------------------------------//

      // Set ERROR_CALCULATING_CONCENTRATION string.
      buffer.str ( "" );
      buffer << " Basin_Error:  Error when calculating the concentrations. " << std::endl;
      buffer << " Basin_Error:  To fix this try reducing the Darcy time-step size." << std::endl;

      names [ ERROR_CALCULATING_CONCENTRATION ] = buffer.str ();

      //--------------------------------//

      // Set UNKNOWN_DARCY_ERROR string.
      buffer.str ( "" );
      buffer << " Basin_Error: An unknown error occurred " << std::endl;
      names [ UNKNOWN_DARCY_ERROR ] = buffer.str ();

      initialised = true;
   }

   if ( NO_DARCY_ERROR <= id and id < UNKNOWN_DARCY_ERROR ) {
      return names [ id ];
   } else {
      return names [ UNKNOWN_DARCY_ERROR ];
   }

}

//------------------------------------------------------------//

std::string MultiComponentFlowHandler::getCommandLineOptions () {

   std::stringstream commandLineOptions;

   commandLineOptions << endl;
   commandLineOptions << endl;
   commandLineOptions << "      In order to run the mcf solver several options must be selected: "  << endl;
   commandLineOptions << endl;
   commandLineOptions << "      Required parameters:" << endl;
   commandLineOptions << "           -genex -mcf                 Initialise the solver (genex is needed so that there is hc to flow)." << endl;
   commandLineOptions << "         either" << endl;
   commandLineOptions << "           -mcfforms <f1,[f2,f3,...]>  A comma separated list of formations, must be in descending order of depth. "  << endl;
   commandLineOptions << "         or" << endl;
   commandLineOptions << "           -mcfmaxformperm <val>       The maximum permeability used to determine if a layer is a low perm layer or not" << endl
                      << "                                       (the temperature and ves are computed at a 2km depth assuming simple gradients). "  << endl;
   commandLineOptions << endl;
   commandLineOptions << "      Optional parameters:" << endl;
   commandLineOptions << "           -mcfotgc                    Include oil-to-gas cracking in the multi-component flow solver." << endl;
   commandLineOptions << "           -mcfnootgc                  Do not include oil-to-gas cracking in the multi-component flow solver." << endl;
   commandLineOptions << "           -mcfimmobsat                Include the immobile-species in the porosity calculation (only valid when otgc is on)." << endl;

   commandLineOptions << "           -mcfmaxts <ts>              Set the maximum time-step to be used in the multi-component solver, this does not" << endl
                      << "                                       affect time-stepping in the pressure/temperature solvers, ts > 0." << endl;
   commandLineOptions << "           -mcfcflts                   Use the time-step size given by the adaptive time-stepping function." << endl;
   commandLineOptions << "           -mcfcflfrac <frac>          Scale the adaptive time-stepping value by this fraction, frac > 0." << endl;
   commandLineOptions << "           -mcfmaxfluxfrac <frac>      What is the max fraction of the total hc in the element that can be \"fluxed\" through the" << endl
                      << "                                       element (temporary testing parameter)." << endl;
   commandLineOptions << "           -mcfquad <deg>              Set the quadrature degree for all integrals (face-fluxes, 'previous term', source-term and" << endl
                      << "                                       the mass-matrix)." << endl;
   commandLineOptions << "           -mcffacequad <deg>          Set the quadrature degree for the face flux integrals." << endl;
   commandLineOptions << "           -mcfprevquad <deg>          Set the quadrature degree for the 'previous term' integrals." << endl;
   commandLineOptions << "           -mcfsourcequad <deg>        Set the quadrature degree for the source term integral." << endl;
   commandLineOptions << "           -mcfmassmatquad <deg>       Set the quadrature degree for the mass matrix integrals." << endl;
   commandLineOptions << "           -mcfdebug <n>               Set the debug level for the multi-component solver." << endl;
   commandLineOptions << "           -mcfmaxgp <val>             Limit the grad-pressure when computing the fluxes, default: " << DefaultMaximumGradPressure << " Pa/m." << endl;
   commandLineOptions << "           -mcfmaxperm <val>           Limit the permeability when computing the fluxes, default: " << DefaultFluxPermeabilityMaximum << " milli-Darcy." << endl;
   commandLineOptions << "           -mcfsavevol                 Save volume calculations (debugging only)." << endl;
   commandLineOptions << "           -mcfnosatop                 Do not include the water-saturation in the over-pressure calculation." << endl;
   commandLineOptions << "           -mcfcp                      Use the capillary pressure in the Darcy flux calculation." << endl;
   commandLineOptions << "           -mcfpvtaverage              Average the hc-densities and -viscosities." << endl;
   commandLineOptions << "           -mcfstopsource <age>        Age at which to stop genex contributions from being added to system (testing parameter)." << endl;
   commandLineOptions << "           -mcfstoptrans <age>         Age at which to remove hc-transport from being included in the Darcy sim (testing parameter)." << endl;
   commandLineOptions << "           -mcfinode <i>               " << endl;
   commandLineOptions << "           -mcfjnode <j>               " << endl;
   commandLineOptions << "           -mcfknode <k>               " << endl;

   commandLineOptions << "           -mcftssubsamp <n>           Time step sub-sampling for sub-processes, n > 1." << endl;
   commandLineOptions << "           -mcftssubsampproc proc1,proc2,proc3 " << endl
                      << "                                       The comma separated list of sub-processes that should use sub-sampling of the Darcy time-steps, currently: otgc, pvt, flux." << endl;

   commandLineOptions << "           -mcfmaps                    Include maps in the Darcy output." << endl;
   commandLineOptions << endl;

   return commandLineOptions.str ();
}

//------------------------------------------------------------//

MultiComponentFlowHandler::MultiComponentFlowHandler () {
   m_solveFlowEquations = PETSC_FALSE;
   m_outputDarcyMaps = false;
   m_outputElementMasses = false;

   // m_userDefinedSubdomains = false;
   m_userDefinedMaximumPermeability = DefaultMcfMaximumPermeability;
   m_uniformTimeStepping = PETSC_FALSE;
   m_adaptiveTimeStepFraction = DefaultMcfAdaptiveTimeStepFraction;
   m_maximumTimeStepSize = DefaultMaximumTimeStep; //static_cast<double>(FastcauldronSimulator::getInstance().getRunParameters ()->getDarcyMaxTimeStep());//DefaultMaximumTimeStep;
   m_maximumHCFractionForFlux = DefaultMaximumHCFractionForFlux;
   m_residualHcSaturationScaling = DefaultResidualHcSaturationScaling;
   m_debugLevel = 0;
   m_applyOtgc = PETSC_FALSE;
   m_includeWaterSaturationInOp = PETSC_FALSE;
   m_includeCapillaryPressureInDarcy = false;
   m_explicitFlowSolver = 0;

   m_faceQuadratureDegree = DefaultFaceQuadratureDegree;
   m_previousContributionTermQuadratureDegree = DefaultPreviousContributionTermQuadratureDegree;
   m_sourceTermQuadratureDegree = DefaultSourceTermQuadratureDegree;
   m_massMatrixQuadratureDegree = DefaultMassMatrixQuadratureDegree;

   m_stopHcContributions = false;
   m_stopHcContributionsAge = 0.0;

#if 1
   m_stopHcTransport = false;
   m_stopHcTransportAge = 0.0;
#else
   // Temporarily stop transport for the BPA testing.
   m_stopHcTransport = true;
   m_stopHcTransportAge = 5000.0;
#endif

   m_saveVolumeOutput = false;
   m_saveTransportedVolumeOutput = false;

   m_limitGradPressure = false;
   m_gradPressureMaximum = DefaultMaximumGradPressure;

   m_limitPermeabilityInFlux = false;
   m_maximumFluxPermeability = DefaultFluxPermeabilityMaximum;

   m_useImmobileSaturation = false;

   m_timeStepSubSample = 1;
   m_timeStepSubSampleOtgc = false;
   m_timeStepSubSamplePvt = false;
   m_timeStepSubSampleFlux = false;
   m_applyPvtAveraging = false;

   m_interpolatePermeability = true;
   m_interpolatePoreVolume = true;
   m_interpolateFaceArea = true;
   m_interpolateSourceTerm = true;

   // Use of the estimated saturation is not enabled by default yet.
   // More testing of this is needed.
   m_useEstimatedSaturation = false;

   m_timeStepSmoothingFactor = DefaultDarcyTimeStepSmoothingFactor;
   m_applyTimeStepSmoothing = true;
}

//------------------------------------------------------------//

MultiComponentFlowHandler::~MultiComponentFlowHandler () {

   size_t i;

   for ( i = 0; i < m_subdomains.size (); ++i ) {
      delete m_subdomains [ i ];
   }

   m_subdomains.clear ();

   if ( m_explicitFlowSolver != 0 ) {
      delete m_explicitFlowSolver;
   }

}

//------------------------------------------------------------//

void MultiComponentFlowHandler::determineUsage () {
   PetscOptionsHasName ( PETSC_NULL, "-mcf", &m_solveFlowEquations );
}

//------------------------------------------------------------//

void MultiComponentFlowHandler::initialise () {

   if ( not m_solveFlowEquations ) {
      // If the flow equations are not to be solved then exit immediately.
      return;
   }

   const int MaximumNumberOfFormations = 1000;

   const int MaximumNumberOfSubProcesses = 100;

   PetscBool iNodeWanted;
   PetscBool jNodeWanted;
   PetscBool kNodeWanted;
   PetscBool mcfDebugLevelSet = PETSC_FALSE;

   PetscBool formationRangeInput    = PETSC_FALSE;

   PetscInt formationRangeArray [ MaximumNumberOfFormations ];
   PetscInt formationCount = MaximumNumberOfFormations;

   PetscBool subSamplingForSubProcessesRequired = PETSC_FALSE;
   char* subProcessNames [ MaximumNumberOfSubProcesses ];
   int numberOfSubProcesses = MaximumNumberOfSubProcesses;

   PetscBool maximumMcfTimeStepChanged = PETSC_FALSE;
   double newMaximumMcfTimeStep;

   double newMaximumMcfHCFractionForFlux;
   PetscBool maximumMcfHCFractionForFluxChanged = PETSC_FALSE;

   double newResidualHcSaturationScaling;
   PetscBool residualHcSaturationScalingChanged = PETSC_FALSE;

   double newMcfAdaptiveTimeStepFraction;
   PetscBool mcfAdaptiveTimeStepFractionChanged = PETSC_FALSE;

   double lowPermeability;
   PetscBool lowPermeabilityDefined = PETSC_FALSE;
   PetscBool doNotIncludeWaterSaturationInOp = PETSC_FALSE;
   PetscBool includeCapillaryPressureInDarcy = PETSC_FALSE;
   PetscBool includePvtAveraging = PETSC_FALSE;

   PetscBool notIncludePermeabilityInterpolation = PETSC_FALSE;
   PetscBool notIncludePoreVolumeInterpolation = PETSC_FALSE;
   PetscBool notIncludeFaceAreaInterpolation = PETSC_FALSE;
   PetscBool notIncludeSourceTermInterpolation = PETSC_FALSE;
   PetscBool removeAllInterpolation = PETSC_FALSE;
   PetscBool useSaturationEstimate = PETSC_FALSE;

   double     ageToStopHCSource;
   PetscBool ageToStopHCSourceChanged;

   double     ageToStopHCTransport;
   PetscBool ageToStopHCTransportChanged;

   double     newGradPressureMaximum;
   PetscBool gradPressureMaximumChanged;

   double     newFluxPermeabilityMaximum;
   PetscBool fluxPermeabilityMaximumChanged;

   PetscInt   faceQuadDegree;
   PetscBool faceQuadDegreeSet;
   PetscInt   prevQuadDegree;
   PetscBool prevQuadDegreeSet;
   PetscInt   sourceQuadDegree;
   PetscBool sourceQuadDegreeSet;
   PetscInt   massMatQuadDegree;
   PetscBool massMatQuadDegreeSet;
   PetscInt   allQuadDegree;
   PetscBool allQuadDegreeSet;

   PetscBool saveVolumeOutput;
   PetscBool saveTransportedVolumeOutput;
   PetscBool useImmobileSaturation;

   PetscBool changeTimeStepSubSampleStep;
   int        newTimeStepSubSampleStep;


   double timeStepSmoothingValue;
   PetscBool timeStepSmoothgingSet;

   PetscBool nonuniformTimeStepping;
   PetscBool uniformTimeStepping;

   PetscBool timeStepSmoothingUnset;

   PetscBool doNotApplyOtgc;
   PetscBool applyOtgc;
   PetscBool doNotStopHCTransportChanged;
   PetscBool includeDarcyMaps = PETSC_FALSE;
   PetscBool includeElementMasses = PETSC_FALSE;

   m_applyOtgc = static_cast<PetscBool>(FastcauldronSimulator::getInstance ().getRunParameters ()->getApplyOtgcToDarcy ());
   m_maximumTimeStepSize = static_cast<double>(FastcauldronSimulator::getInstance().getRunParameters ()->getDarcyMaxTimeStep());

   PetscOptionsHasName ( PETSC_NULL, "-mcfnootgc",  &doNotApplyOtgc );
   PetscOptionsHasName ( PETSC_NULL, "-mcfotgc",  &applyOtgc );
   PetscOptionsGetInt  ( PETSC_NULL, "-mcfinode", &m_debugINode, &iNodeWanted );
   PetscOptionsGetInt  ( PETSC_NULL, "-mcfjnode", &m_debugJNode, &jNodeWanted );
   PetscOptionsGetInt  ( PETSC_NULL, "-mcfknode", &m_debugKNode, &kNodeWanted );

   PetscOptionsGetReal ( PETSC_NULL, "-mcfmaxgp", &newGradPressureMaximum, &gradPressureMaximumChanged );
   PetscOptionsGetReal ( PETSC_NULL, "-mcfmaxperm", &newFluxPermeabilityMaximum, &fluxPermeabilityMaximumChanged );

   PetscOptionsGetInt  ( PETSC_NULL, "-mcfdebug", &m_debugLevel, &mcfDebugLevelSet );
   PetscOptionsGetReal ( PETSC_NULL, "-mcfmaxts", &newMaximumMcfTimeStep, &maximumMcfTimeStepChanged );
   PetscOptionsGetReal ( PETSC_NULL, "-mcfmaxfluxfrac", &newMaximumMcfHCFractionForFlux, &maximumMcfHCFractionForFluxChanged );

   PetscOptionsHasName ( PETSC_NULL, "-mcfcflts", &nonuniformTimeStepping);
   PetscOptionsHasName ( PETSC_NULL, "-mcfuniformts", &uniformTimeStepping);

   PetscOptionsGetReal ( PETSC_NULL, "-mcfmaxformperm", &lowPermeability, &lowPermeabilityDefined );

   PetscOptionsHasName ( PETSC_NULL, "-mcfsavevol",  &saveVolumeOutput );
   PetscOptionsHasName ( PETSC_NULL, "-mcfsavevoltrans",  &saveTransportedVolumeOutput );

   PetscOptionsHasName ( PETSC_NULL, "-mcfnosatop",  &doNotIncludeWaterSaturationInOp );
   PetscOptionsHasName ( PETSC_NULL, "-mcfcp",  &includeCapillaryPressureInDarcy );

   PetscOptionsHasName ( PETSC_NULL, "-mcfpvtaverage",  &includePvtAveraging );

   PetscOptionsHasName ( PETSC_NULL, "-mcfnoperminterp", &notIncludePermeabilityInterpolation );
   PetscOptionsHasName ( PETSC_NULL, "-mcfnopvinterp", &notIncludePoreVolumeInterpolation );
   PetscOptionsHasName ( PETSC_NULL, "-mcfnofainterp", &notIncludeFaceAreaInterpolation );
   PetscOptionsHasName ( PETSC_NULL, "-mcfnosrterminterp",&notIncludeSourceTermInterpolation );
   PetscOptionsHasName ( PETSC_NULL, "-mcfnointerp",  &removeAllInterpolation );

   PetscOptionsHasName ( PETSC_NULL, "-mcfusesatest", &useSaturationEstimate );
   PetscOptionsGetReal ( PETSC_NULL, "-mcfsorscal", &newResidualHcSaturationScaling, &residualHcSaturationScalingChanged );

   PetscOptionsGetReal ( PETSC_NULL, "-mcfstopsource", &ageToStopHCSource, &ageToStopHCSourceChanged );
   PetscOptionsGetReal ( PETSC_NULL, "-mcfstoptrans", &ageToStopHCTransport, &ageToStopHCTransportChanged );
   PetscOptionsHasName ( PETSC_NULL, "-mcfnostoptrans", &doNotStopHCTransportChanged );
   PetscOptionsHasName ( PETSC_NULL, "-mcfmaps", &includeDarcyMaps );
   PetscOptionsHasName ( PETSC_NULL, "-mcfelemmasses", &includeElementMasses );

   PetscOptionsGetReal ( PETSC_NULL, "-mcfcflfrac", &newMcfAdaptiveTimeStepFraction, &mcfAdaptiveTimeStepFractionChanged );
   PetscOptionsGetIntArray ( PETSC_NULL, "-mcfforms", formationRangeArray, &formationCount, &formationRangeInput );

   PetscOptionsGetInt  ( PETSC_NULL, "-mcffacequad", &faceQuadDegree, &faceQuadDegreeSet );
   PetscOptionsGetInt  ( PETSC_NULL, "-mcfprevquad", &prevQuadDegree, &prevQuadDegreeSet );
   PetscOptionsGetInt  ( PETSC_NULL, "-mcfsourcequad", &sourceQuadDegree, &sourceQuadDegreeSet );
   PetscOptionsGetInt  ( PETSC_NULL, "-mcfmassmatquad", &massMatQuadDegree, &massMatQuadDegreeSet );
   PetscOptionsGetInt  ( PETSC_NULL, "-mcfquad", &allQuadDegree, &allQuadDegreeSet );

   PetscOptionsGetInt  ( PETSC_NULL, "-mcftssubsamp", &newTimeStepSubSampleStep, &changeTimeStepSubSampleStep );
   PetscOptionsGetStringArray ( PETSC_NULL, "-mcftssubsampproc", subProcessNames, &numberOfSubProcesses, &subSamplingForSubProcessesRequired );

   PetscOptionsHasName ( PETSC_NULL, "-mcfimmobsat", &useImmobileSaturation );
   PetscOptionsGetReal ( PETSC_NULL, "-mcftssmooth", &timeStepSmoothingValue,  &timeStepSmoothgingSet );
   PetscOptionsHasName ( PETSC_NULL, "-mcfnotssmooth", &timeStepSmoothingUnset );


   if ( applyOtgc and not doNotApplyOtgc ) {
      m_applyOtgc = PETSC_TRUE;
   }

   if ( doNotApplyOtgc and not applyOtgc ) {
      m_applyOtgc = PETSC_FALSE;
   }

   if ( includeDarcyMaps ) {
      m_outputDarcyMaps = true;
   } else {
      m_outputDarcyMaps = false;
   }

   if ( includeElementMasses ) {
      m_outputElementMasses = true;
   } else {
      m_outputElementMasses = false;
   }

   if ( doNotApplyOtgc and applyOtgc ) {
      PetscPrintf ( PETSC_COMM_WORLD, " Basin_Warning: both -mcfotgc and -mcfnootgc have been set. Both will be ignored and the value defined in the project file will be used." );
   }

   //
   m_useImmobileSaturation = ( useImmobileSaturation == PETSC_TRUE );

   if ( changeTimeStepSubSampleStep and subSamplingForSubProcessesRequired ) {
      int i;

      for ( i = 0; i < numberOfSubProcesses; ++i ) {
         const char* name = subProcessNames [ i ];

         if ( strcasecmp ( name, "otgc" ) == 0 ) {
            m_timeStepSubSampleOtgc = true;
         } else if ( strcasecmp ( name, "pvt" ) == 0 or strcasecmp ( name, "flash" ) == 0 ) {
            m_timeStepSubSamplePvt = true;
         } else if ( strcasecmp ( name, "flux" ) == 0 ) {
            m_timeStepSubSampleFlux = true;
         } else {
            PetscPrintf ( PETSC_COMM_WORLD, " Basin_Warning: Option '%s' is not known.\n", name );
         }

      }

   }

   if ( changeTimeStepSubSampleStep ) {
      m_timeStepSubSample = newTimeStepSubSampleStep;
   }

   if ( saveVolumeOutput ) {
      m_saveVolumeOutput = bool ( saveVolumeOutput );
   }

   if ( saveTransportedVolumeOutput ) {
      m_saveTransportedVolumeOutput = bool ( saveTransportedVolumeOutput );
   }

   //----------------------------//

   if ( uniformTimeStepping and nonuniformTimeStepping ) {
      PetscPrintf ( PETSC_COMM_WORLD, " Basin_Warning: both -mcfcflts and -mcfuniformts have been set. Setting to non-uniform time-stepping." );
      nonuniformTimeStepping = PETSC_TRUE;
      uniformTimeStepping = PETSC_FALSE;
   }

   if ( uniformTimeStepping ) {
      m_uniformTimeStepping = PETSC_TRUE;
   } else  {
      m_uniformTimeStepping = PETSC_FALSE;
   }

   if ( maximumMcfHCFractionForFluxChanged ) {
      m_maximumHCFractionForFlux = newMaximumMcfHCFractionForFlux;
   }

   if ( residualHcSaturationScalingChanged ) {
      m_residualHcSaturationScaling = newResidualHcSaturationScaling;
   }

   if ( mcfAdaptiveTimeStepFractionChanged ) {
      m_adaptiveTimeStepFraction = newMcfAdaptiveTimeStepFraction;
   }

   if ( timeStepSmoothgingSet ) {

      if ( timeStepSmoothingValue < 1.0 ) {
         PetscPrintf ( PETSC_COMM_WORLD, " Time step smoothing factor is less than 1, value = %e. Smoothing will not be applied.\n", timeStepSmoothingValue );
      } else {
         m_timeStepSmoothingFactor = timeStepSmoothingValue;
         m_applyTimeStepSmoothing = true;
      }

   }

   if ( timeStepSmoothingUnset ) {
      m_applyTimeStepSmoothing = false;
   }

   //----------------------------//

   if ( lowPermeabilityDefined ) {
      m_userDefinedMaximumPermeability = lowPermeability;
   }

   if ( gradPressureMaximumChanged ) {
      m_limitGradPressure = true;
      m_gradPressureMaximum = newGradPressureMaximum;
   }

   if ( fluxPermeabilityMaximumChanged ) {
      m_limitPermeabilityInFlux = true;
      m_maximumFluxPermeability = newFluxPermeabilityMaximum;
   }

   if ( mcfDebugLevelSet ) {
      m_debugLevel = NumericFunctions::Maximum ( 0, m_debugLevel );
      PetscPrintf ( PETSC_COMM_WORLD, " mcf debug level: %i.\n", m_debugLevel );
   }

   if ( ageToStopHCSourceChanged ) {
      m_stopHcContributions = true;
      m_stopHcContributionsAge = ageToStopHCSource;
   }

   if ( doNotStopHCTransportChanged ) {
      m_stopHcTransport = false;
      m_stopHcTransportAge = 0.0;
   }

   if ( ageToStopHCTransportChanged ) {
      m_stopHcTransport = true;
      m_stopHcTransportAge = ageToStopHCTransport;
   }

   if ( allQuadDegreeSet ) {
      allQuadDegree = NumericFunctions::clipValueToRange <int>( allQuadDegree, 1, NumericFunctions::Quadrature::MaximumQuadratureDegree );
      m_faceQuadratureDegree = allQuadDegree;
      m_previousContributionTermQuadratureDegree = allQuadDegree;
      m_sourceTermQuadratureDegree = allQuadDegree;
      m_massMatrixQuadratureDegree = allQuadDegree;
   }

   if ( faceQuadDegreeSet ) {
      m_faceQuadratureDegree = NumericFunctions::clipValueToRange <int>( faceQuadDegree, 1, NumericFunctions::Quadrature::MaximumQuadratureDegree );
   }

   if ( prevQuadDegreeSet ) {
      m_previousContributionTermQuadratureDegree = NumericFunctions::clipValueToRange <int>( prevQuadDegree, 1, NumericFunctions::Quadrature::MaximumQuadratureDegree );
   }

   if ( sourceQuadDegreeSet ) {
      m_sourceTermQuadratureDegree = NumericFunctions::clipValueToRange <int>( sourceQuadDegree, 1, NumericFunctions::Quadrature::MaximumQuadratureDegree );
   }

   if ( massMatQuadDegreeSet ) {
      m_massMatrixQuadratureDegree = NumericFunctions::clipValueToRange <int>( massMatQuadDegree, 1, NumericFunctions::Quadrature::MaximumQuadratureDegree );
   }

   m_includeWaterSaturationInOp = not static_cast<bool>(doNotIncludeWaterSaturationInOp);
   m_includeCapillaryPressureInDarcy = static_cast<bool>(includeCapillaryPressureInDarcy);
   m_applyPvtAveraging = static_cast<bool>(includePvtAveraging);

   if ( removeAllInterpolation ) {
      m_interpolatePermeability = false;
      m_interpolatePoreVolume = false;
      m_interpolateFaceArea = false;
      m_interpolateSourceTerm = false;
   } else {
      m_interpolatePermeability = not static_cast<bool>(notIncludePermeabilityInterpolation);
      m_interpolatePoreVolume = not static_cast<bool>(notIncludePoreVolumeInterpolation);
      m_interpolateFaceArea = not static_cast<bool>(notIncludeFaceAreaInterpolation);
      m_interpolateSourceTerm = not static_cast<bool>(notIncludeSourceTermInterpolation);
   }

   m_useEstimatedSaturation = static_cast<bool>(useSaturationEstimate);

   if ( not iNodeWanted ) {
      m_debugINode = -1;
   }

   if ( not jNodeWanted ) {
      m_debugJNode = -1;
   }

   if ( not kNodeWanted ) {
      m_debugKNode = -1;
   }

   if ( m_debugINode == -1  or m_debugJNode == -1 ) {
      m_debugINode = -1;
      m_debugJNode = -1;
      m_debugKNode = -1;
   }

   if ( iNodeWanted and jNodeWanted and not kNodeWanted ) {
      PetscPrintf ( PETSC_COMM_WORLD, " debugging output for needle: %i  %i.\n", m_debugINode, m_debugJNode );
   }

   if ( maximumMcfTimeStepChanged ) {
      m_maximumTimeStepSize = newMaximumMcfTimeStep;

      if (  m_debugLevel > 0 ) {
         PetscPrintf ( PETSC_COMM_WORLD, " maximum mcf time step size: %i.\n", m_maximumTimeStepSize );
      }

   }

   // Determine the subdomains for the solver.
   if ( formationRangeInput ) {
      addSubdomains ( formationRangeArray, formationCount );
   } else {
      determineSubdomains ();
   }

   numberSubdomains ();
   m_explicitFlowSolver = new ExplicitMultiComponentFlowSolver;
}

//------------------------------------------------------------//

void MultiComponentFlowHandler::numberSubdomains () {

   size_t i;

   for ( i = 0; i < m_subdomains.size (); ++i ) {
      m_subdomains [ i ]->setId ( int ( i + 1 ));
   }

}

//------------------------------------------------------------//

bool MultiComponentFlowHandler::addSubdomain ( Subdomain* subdomain ) {

   if ( subdomain == 0 ) {
      return false;
   }

   DarcySubdomainArray::const_iterator iter;

   for ( iter = m_subdomains.begin (); iter != m_subdomains.end (); ++iter ) {

      if ((*iter)->overlaps ( *subdomain )) {
         return false;
      }

   }

   m_subdomains.push_back ( subdomain );
   return true;
}

//------------------------------------------------------------//

void MultiComponentFlowHandler::addSubdomains ( const int* formationRangeArray,
                                                const int  formationRangeCount ) {

   LayerList layers;
   getLayers ( layers );

   // The number of formations excluding the crust and mantle (so subtract 2) and the array indexing starts at 0 (so subtract 1).
   const int numberOfFormations = layers.size () - 1;

   int i;
   int start;
   int end = -1;
   int contiguousCount;
   int rangeCount;
   Subdomain* subdomain;
   bool subdomainWasAdded;

   start = formationRangeArray [ 0 ];
   contiguousCount = 1;
   rangeCount = 0;

   for ( i = 1; i < formationRangeCount; ++i ) {

      if ( start + contiguousCount != formationRangeArray [ i ] ) {
         end = formationRangeArray [ i - 1 ];
         contiguousCount = 1;
         ++rangeCount;

         if ( not NumericFunctions::inRange<int>( start, 0, numberOfFormations ) or not NumericFunctions::inRange<int>( end, 0, numberOfFormations )) {
            PetscPrintf ( PETSC_COMM_WORLD, " Basin_Error: invalid formation range: [%i,%i]\n", start, end );
            PetscPrintf ( PETSC_COMM_WORLD, " Basin_Error: This formation range will not be a part of the multi-component flow solver.\n" );
         } else {
            subdomain = new Subdomain ( *layers [ start ], *layers [ end ]);
            subdomainWasAdded = addSubdomain ( subdomain );

            if ( not subdomainWasAdded ) {
               PetscPrintf ( PETSC_COMM_WORLD, "Subdomain %i with range [%i,%i] was not added to the calculator.\n", rangeCount, start, end );
               PetscPrintf ( PETSC_COMM_WORLD, "Subdomain %i with range [%s,%s] was not added to the calculator.\n",
                             rangeCount,
                             layers [ start ]->getName ().c_str (),
                             layers [ end ]->getName ().c_str ());
               delete subdomain;
            } else {
               PetscPrintf ( PETSC_COMM_WORLD, "Subdomain %i:\n", rangeCount );
               PetscPrintf ( PETSC_COMM_WORLD, subdomain->image ().c_str ());
               PetscPrintf ( PETSC_COMM_WORLD, "\n");
            }

         }

         start = formationRangeArray [ i ];
      } else if ( i == formationRangeCount - 1 ) {
         end = formationRangeArray [ i ];
         contiguousCount = 1;
         ++rangeCount;

         if ( not NumericFunctions::inRange<int>( start, 0, numberOfFormations ) or not NumericFunctions::inRange<int>( end, 0, numberOfFormations )) {
            PetscPrintf ( PETSC_COMM_WORLD, " Basin_Error: invalid formation range: [%i,%i]\n", start, end );
            PetscPrintf ( PETSC_COMM_WORLD, " Basin_Error: This formation range will not be a part of the multi-component flow solver.\n" );
         } else {
            subdomain = new Subdomain ( *layers [ start ], *layers [ end ]);
            subdomainWasAdded = addSubdomain ( subdomain );

            if ( not subdomainWasAdded ) {
               PetscPrintf ( PETSC_COMM_WORLD, "Subdomain %i with range [%i,%i] was not added to the calculator.\n", rangeCount, start, end );
               PetscPrintf ( PETSC_COMM_WORLD, "Subdomain %i with range [%s,%s] was not added to the calculator.\n",
                             rangeCount,
                             layers [ start ]->getName ().c_str (),
                             layers [ end ]->getName ().c_str ());
               delete subdomain;
            } else {
               PetscPrintf ( PETSC_COMM_WORLD, "Subdomain %i:\n", rangeCount );
               PetscPrintf ( PETSC_COMM_WORLD, subdomain->image ().c_str ());
               PetscPrintf ( PETSC_COMM_WORLD, "\n");
            }

         }

      } else {
         contiguousCount += 1;
      }

   }

   if ( end < start ) {
      end = formationRangeArray [ formationRangeCount - 1 ];
      ++rangeCount;

      if ( not NumericFunctions::inRange<int>( start, 0, numberOfFormations ) or not NumericFunctions::inRange<int>( end, 0, numberOfFormations )) {
         PetscPrintf ( PETSC_COMM_WORLD, " Basin_Error: invalid formation range: [%i,%i]\n", start, end );
         PetscPrintf ( PETSC_COMM_WORLD, " Basin_Error: This formation range will not be a part of the multi-component flow solver.\n" );
      } else {
         subdomain = new Subdomain ( *layers [ start ], *layers [ end ]);
         subdomainWasAdded = addSubdomain ( subdomain );

         if ( not subdomainWasAdded ) {
            PetscPrintf ( PETSC_COMM_WORLD, "Subdomain %i with range [%i,%i] was not added to the calculator.\n", rangeCount, start, end );
            PetscPrintf ( PETSC_COMM_WORLD, "Subdomain %i with range [%s,%s] was not added to the calculator.\n",
                          rangeCount,
                          layers [ start ]->getName ().c_str (),
                          layers [ end ]->getName ().c_str ());
            delete subdomain;
         } else {
            PetscPrintf ( PETSC_COMM_WORLD, "Subdomain %i:\n", rangeCount );
            PetscPrintf ( PETSC_COMM_WORLD, subdomain->image ().c_str ());
            PetscPrintf ( PETSC_COMM_WORLD, "\n");
         }

      }

   }

}

//------------------------------------------------------------//

void MultiComponentFlowHandler::determineSubdomains () {

   unsigned int i;
   LayerList layers;
   Subdomain* subdomain;
   bool subdomainWasAdded;
   bool foundStart;
   bool foundEnd;
   bool continueSearching = true;
   bool containsSourceRock;
   int first = 0;
   int last;

   getLayers ( layers );

   while ( continueSearching ) {

      // Find the index of the first layer that is part of a Darcy domain (from the current start position)
      foundStart = false;

      for ( i = first; i < layers.size (); ++i ) {

         if ( layers [ i ]->getIncludeInDarcy ()) {
            first = i;
            foundStart = true;
            break;
         }

      }

      // If we have found the start of the Darcy domain search for the
      // end of the range of layers that are a part of the domain.
      if ( foundStart ) {

         // Find the index of the layer that is the bottom of the subdomain.
         last = first + 1;
         foundEnd = false;

         for ( i = first + 1; i < layers.size (); ++i ) {

            if ( not layers [ i ]->getIncludeInDarcy ()) {
               last = i - 1;
               foundEnd = true;
               break;
            }

         }

         // If the end is not found then the end must be the last layer.
         if ( not foundEnd ) {
            last = layers.size () - 1;
         }

         // Now check to see of the subdomain contains a source-rock.
         containsSourceRock = false;

         for ( i = first; i <= last; ++i ) {
            containsSourceRock = containsSourceRock or layers [ i ]->isSourceRock ();
         }

         // If it does contain then we can allocate a subdomain.
         if ( containsSourceRock ) {
            subdomain = new Subdomain ( *layers [ first ], *layers [ last ]);
            PetscPrintf ( PETSC_COMM_WORLD, subdomain->image ().c_str ());
            PetscPrintf ( PETSC_COMM_WORLD, "\n");

            // If formation was not added for any reason, e.g. the formation was duplicate it will be deleted.
            subdomainWasAdded = addSubdomain ( subdomain );

            if ( not subdomainWasAdded ) {
               delete subdomain;
            }

         }

         // Set the first index to be 1 past the last ready to find the next subdomain.
         first = last + 1;

      } else {
         continueSearching = false;
      }

   }

}

//------------------------------------------------------------//

void MultiComponentFlowHandler::getLayers ( LayerList& layers ) {

   Interface::FormationList* formations = FastcauldronSimulator::getInstance ().getFormations ();
   Interface::FormationList::const_iterator formationIter;

   for ( formationIter = formations->begin (); formationIter != formations->end (); ++formationIter ) {
      LayerProps* formation = const_cast<LayerProps*>(dynamic_cast<const LayerProps*>( *formationIter ));

      if ( formation != 0 and formation->isSediment() ) {
         layers.push_back ( formation );
      }

   }

   delete formations;
}

//------------------------------------------------------------//

void MultiComponentFlowHandler::setSubdomainActivity ( const double currentTime ) {

   if ( solveFlowEquations ()) {
      size_t i;

      for ( i = 0; i < m_subdomains.size (); ++i ) {
         m_subdomains [ i ]->setActivity ( currentTime );
      }

   }

}

//------------------------------------------------------------//

bool MultiComponentFlowHandler::anySubdomainIsActive () const {

   if ( solveFlowEquations ()) {
      size_t i;

      for ( i = 0; i < m_subdomains.size (); ++i ) {

         if ( m_subdomains [ i ]->isActive ()) {
            return true;
         }

      }

   }

   return false;
}

//------------------------------------------------------------//

int MultiComponentFlowHandler::numberOfActiveSubdomains () const {

   int count = 0;

   if ( solveFlowEquations ()) {
      size_t i;

      for ( i = 0; i < m_subdomains.size (); ++i ) {

         if ( m_subdomains [ i ]->isActive ()) {
            ++count;
         }

      }

   }

   return count;
}

//------------------------------------------------------------//

void MultiComponentFlowHandler::solve ( const double previousTime,
                                        const double currentTime,
                                              bool&  errorOccurred ) {

   errorOccurred = false;

   if ( solveFlowEquations ()) {
      size_t i;
      DarcyErrorIndicator errorInSubdomain;

      for ( i = 0; i < m_subdomains.size (); ++i ) {

         if ( m_subdomains [ i ]->isActive ()) {
            m_explicitFlowSolver->solve ( *m_subdomains [ i ], previousTime, currentTime, errorInSubdomain );

            if ( errorInSubdomain != NO_DARCY_ERROR ) {

               PetscPrintf ( PETSC_COMM_WORLD, "\n%s",
                             getErrorString ( errorInSubdomain ).c_str ());

               errorOccurred = true;
               break;
            }

         }

      }

   }

}

//------------------------------------------------------------//

bool MultiComponentFlowHandler::modelContainsSulphur () const {

   if ( solveFlowEquations ()) {
      size_t i;

      for ( i = 0; i < m_subdomains.size (); ++i ) {

         if ( m_subdomains [ i ]->containsSulphurSourceRock ()) {
            return true;
         }

      }

   }

   return false;
}

//------------------------------------------------------------//

bool MultiComponentFlowHandler::containsFormation ( const LayerProps* formation ) const {

   size_t i;

   for ( i = 0; i < m_subdomains.size (); ++i ) {

      if ( m_subdomains [ i ]->hasLayer ( formation )) {
         return true;
      }

   }

   return false;
}

//------------------------------------------------------------//

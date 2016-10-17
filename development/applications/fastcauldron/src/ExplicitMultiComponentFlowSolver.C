//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 
#include "ExplicitMultiComponentFlowSolver.h"

#include <time.h>
#include <sstream>

#include "MpiFunctions.h"

#include "layer.h"

#include "capillarySealStrength.h"
#include "ComponentManager.h"
#include "SpeciesState.h"

#include "Species.h"
#include "OTGC_kernel6/src/SimulatorState.h"

#include "Interface/GridMap.h"
using namespace DataAccess;

#include "NumericFunctions.h"

#include "CauldronGridDescription.h"

#include "FastcauldronSimulator.h"
#include "MultiComponentFlowHandler.h"
#include "ElementContributions.h"
#include "Quadrature.h"
#include "Quadrature3D.h"

#include "GeoPhysicalConstants.h"
#include "GeoPhysicsFluidType.h"

#include "Lithology.h"

#include "PoreVolumeInterpolatorCalculator.h"

#include "BrooksCorey.h"

// utilities library
#include "ConstantsMathematics.h"
using Utilities::Maths::MillyDarcyToM2;
using Utilities::Maths::MillionYearToSecond;
// utilities library
#include "ConstantsPhysics.h"
using Utilities::Physics::AccelerationDueToGravity;


//------------------------------------------------------------//

ExplicitMultiComponentFlowSolver::ExplicitMultiComponentFlowSolver ()
{
   m_defaultMolarMasses = PVTCalc::getInstance ().getMolarMass ();
   // Kg/mol
   m_defaultMolarMasses *= 1.0e-3;

   m_otgcTime = 0.0;
   m_flashTime = 0.0;
   m_fluxTime = 0.0;
   m_pressureTime = 0.0;
   m_permeabilityTime = 0.0;
   m_previousTime = 0.0;
   m_sourceTime = 0.0;
   m_transportTime = 0.0;
   m_massTime = 0.0;
   m_concTime = 0.0;
   m_satTime = 0.0;
   m_sat2Time = 0.0;
   m_estimatedSaturationTime = 0.0;
   m_totalTime = 0.0;

   m_flashCount = 0;
   m_transportInCount = 0;
   m_transportOutCount = 0;
   m_transportCount = 0;

   m_maximumHCFractionForFlux = FastcauldronSimulator::getInstance ().getMcfHandler ().getMaximumHCFractionForFlux ();

   m_faceQuadratureDegree                  = FastcauldronSimulator::getInstance ().getMcfHandler ().getFaceQuadratureDegree ();
   m_previousContributionsQuadratureDegree = FastcauldronSimulator::getInstance ().getMcfHandler ().getPreviousTermQuadratureDegree ();
   m_massMatrixQuadratureDegree            = FastcauldronSimulator::getInstance ().getMcfHandler ().getMassMatrixQuadratureDegree ();
   m_limitGradPressure                     = FastcauldronSimulator::getInstance ().getMcfHandler ().limitGradPressure ();
   m_gradPressureMaximum                   = FastcauldronSimulator::getInstance ().getMcfHandler ().gradPressureMaximum ();
   m_limitFluxPermeability                 = FastcauldronSimulator::getInstance ().getMcfHandler ().limitFluxPermeability ();
   m_fluxPermeabilityMaximum               = FastcauldronSimulator::getInstance ().getMcfHandler ().maximumFluxPermeability ();
   m_includeCapillaryPressure              = FastcauldronSimulator::getInstance ().getMcfHandler ().includeCapillaryPressure ();
   m_useImmobileSaturation                 = FastcauldronSimulator::getInstance ().getMcfHandler ().useImmobileSaturation ();

   m_timeStepSubSample                     = FastcauldronSimulator::getInstance ().getMcfHandler ().getTimeStepSubSample ();
   m_timeStepSubSampleOtgc                 = FastcauldronSimulator::getInstance ().getMcfHandler ().timeStepSubSampleOtgc ();
   m_timeStepSubSamplePvt                  = FastcauldronSimulator::getInstance ().getMcfHandler ().timeStepSubSamplePvt ();
   m_timeStepSubSampleFlux                 = FastcauldronSimulator::getInstance ().getMcfHandler ().timeStepSubSampleFlux ();

   m_interpolateFacePermeability           = FastcauldronSimulator::getInstance ().getMcfHandler ().getInterpolatePermeability ();
   m_interpolatePoreVolume                 = FastcauldronSimulator::getInstance ().getMcfHandler ().getInterpolatePoreVolume ();
   m_interpolateFaceArea                   = FastcauldronSimulator::getInstance ().getMcfHandler ().getInterpolateFaceArea ();
   m_useSaturationEstimate                 = FastcauldronSimulator::getInstance ().getMcfHandler ().getUseEstimatedSaturation ();
   m_residualHcSaturationScaling           = FastcauldronSimulator::getInstance ().getMcfHandler ().getResidualHcSaturationScaling ();

   if ( FastcauldronSimulator::getInstance ().getMcfHandler ().getDebugLevel () > 0 ) {
      bool doGenex = FastcauldronSimulator::getInstance ().getCauldron ()->integrateGenexEquations ();
      bool doOtgc  = FastcauldronSimulator::getInstance ().getMcfHandler ().applyOtgc ();

      PetscPrintf ( PETSC_COMM_WORLD, " Multi-component flow command-line parameters (or equivalent default value): \n" );
      PetscPrintf ( PETSC_COMM_WORLD, "   -mcf            yes\n" );
      PetscPrintf ( PETSC_COMM_WORLD, "   -genex          %s\n", ( doGenex ? "yes" : "no" ));
      PetscPrintf ( PETSC_COMM_WORLD, "   -mcfotgc        %s\n", ( doOtgc  ? "yes" : "no" ));
      // This one looks odd, because the command line parameter switches off the use of water-saturation in the overpressure calculation.
      // but the function determines whether it should be used.
      PetscPrintf ( PETSC_COMM_WORLD, "   -mcfstopsource  %s  %f\n", ( FastcauldronSimulator::getInstance ().getMcfHandler ().removeSourceTerm () ? "yes" : "no" ),
                    FastcauldronSimulator::getInstance ().getMcfHandler ().removeSourceTermAge ());

      PetscPrintf ( PETSC_COMM_WORLD, "   -mcfstoptrans   %s  %f\n", ( FastcauldronSimulator::getInstance ().getMcfHandler ().removeHcTransport () ? "yes" : "no" ),
                    FastcauldronSimulator::getInstance ().getMcfHandler ().removeHcTransportAge ());

      PetscPrintf ( PETSC_COMM_WORLD, "   -mcfnosatop     %s\n", ( FastcauldronSimulator::getInstance ().getMcfHandler ().includeWaterSaturationInOp () ? "no" : "yes" ));
      PetscPrintf ( PETSC_COMM_WORLD, "   -mcfclfts       %s\n", ( FastcauldronSimulator::getInstance ().getMcfHandler ().useAdaptiveTimeStepping () ? "yes" : "no" ));
      PetscPrintf ( PETSC_COMM_WORLD, "   -mcfcflfrac     %f \n", FastcauldronSimulator::getInstance ().getMcfHandler ().adaptiveTimeStepFraction ());
      PetscPrintf ( PETSC_COMM_WORLD, "   -mcfmaxfluxfrac %f \n", m_maximumHCFractionForFlux );
      PetscPrintf ( PETSC_COMM_WORLD, "   -mcfinode       %i \n", FastcauldronSimulator::getInstance ().getMcfHandler ().getDebugINode ());
      PetscPrintf ( PETSC_COMM_WORLD, "   -mcfjnode       %i \n", FastcauldronSimulator::getInstance ().getMcfHandler ().getDebugJNode ());
      PetscPrintf ( PETSC_COMM_WORLD, "   -mcfknode       %i \n", FastcauldronSimulator::getInstance ().getMcfHandler ().getDebugKNode ());
      PetscPrintf ( PETSC_COMM_WORLD, "   -mcfdebug       %i \n", FastcauldronSimulator::getInstance ().getMcfHandler ().getDebugLevel ());
      PetscPrintf ( PETSC_COMM_WORLD, "   -mcfmaxts       %f \n", FastcauldronSimulator::getInstance ().getMcfHandler ().getMaximumTimeStepSize ());

      PetscPrintf ( PETSC_COMM_WORLD, "   -mcffacequad    %i \n", FastcauldronSimulator::getInstance ().getMcfHandler ().getFaceQuadratureDegree ());
      PetscPrintf ( PETSC_COMM_WORLD, "   -mcfprevquad    %i \n", FastcauldronSimulator::getInstance ().getMcfHandler ().getPreviousTermQuadratureDegree ());
      PetscPrintf ( PETSC_COMM_WORLD, "   -mcfsourcequad  %i \n", FastcauldronSimulator::getInstance ().getMcfHandler ().getSourceTermQuadratureDegree ());
      PetscPrintf ( PETSC_COMM_WORLD, "   -mcfmassmatquad %i \n", FastcauldronSimulator::getInstance ().getMcfHandler ().getMassMatrixQuadratureDegree ());
      PetscPrintf ( PETSC_COMM_WORLD, "   -mcfmaxgp       %s %f \n", ( m_limitGradPressure ? "set" : "not set" ), m_gradPressureMaximum );
      PetscPrintf ( PETSC_COMM_WORLD, "   -mcfmaxperm     %s %f \n", ( m_limitFluxPermeability ? "set" : "not set" ), m_fluxPermeabilityMaximum );
      PetscPrintf ( PETSC_COMM_WORLD, "   -mcfpvtaverage  %s \n", ( FastcauldronSimulator::getInstance ().getMcfHandler ().getApplyPvtAveraging () ? "yes" : "no" ));
      PetscPrintf ( PETSC_COMM_WORLD, "   -mcfperminterp  %s \n", ( FastcauldronSimulator::getInstance ().getMcfHandler ().getInterpolatePermeability () ? "yes" : "no" ));
      PetscPrintf ( PETSC_COMM_WORLD, "   -mcfpvinterp    %s \n", ( FastcauldronSimulator::getInstance ().getMcfHandler ().getInterpolatePoreVolume () ? "yes" : "no" ));
      PetscPrintf ( PETSC_COMM_WORLD, "   -mcffainterp    %s \n", ( FastcauldronSimulator::getInstance ().getMcfHandler ().getInterpolateFaceArea () ? "yes" : "no" ));

      PetscPrintf ( PETSC_COMM_WORLD, "   -mcfusesatest   %s \n", ( FastcauldronSimulator::getInstance ().getMcfHandler ().getUseEstimatedSaturation () ? "yes" : "no" ));
      PetscPrintf ( PETSC_COMM_WORLD, "    Sor scaling    %f \n", ( FastcauldronSimulator::getInstance ().getMcfHandler (). getResidualHcSaturationScaling ()));


      PetscPrintf ( PETSC_COMM_WORLD, "   -mcftssmooth    %s %e \n", 
                    ( FastcauldronSimulator::getInstance ().getMcfHandler ().applyTimeStepSmoothing () ? "yes" : "no" ),
                    FastcauldronSimulator::getInstance ().getMcfHandler ().getTimeStepSmoothingFactor ());

      PetscPrintf ( PETSC_COMM_WORLD, "   -mcfforms " );

      size_t i;

      const DarcySubdomainArray& subdomains = FastcauldronSimulator::getInstance ().getMcfHandler ().getSubdomains ();

      for ( i = 0; i < subdomains.size (); ++i ) {
         PetscPrintf ( PETSC_COMM_WORLD, "\nSubdomain %i:\n%s\n\n", i + 1, subdomains [ i ]->image ().c_str ());
      }

   }

   // now scale the permeability maximum to SI units.
   m_fluxPermeabilityMaximum *= MillyDarcyToM2;

   m_maximumTimeStepSize = FastcauldronSimulator::getInstance ().getMcfHandler ().getMaximumTimeStepSize ();

}

//------------------------------------------------------------//

ExplicitMultiComponentFlowSolver::~ExplicitMultiComponentFlowSolver ()
{

   std::stringstream buffer;

   buffer << " Times for subdomain: " << FastcauldronSimulator::getInstance ().getRank ()  << std::endl;
   buffer << " Operation             cumulative time   %-age of total " << std::endl;
   buffer << " OTGC time :             " << std::setw ( 20 ) << m_otgcTime         << " " << std::setw ( 20 ) << m_otgcTime / m_totalTime.floatValue () * 100.0 << std::endl;
   buffer << " Flash time :            " << std::setw ( 20 ) << m_flashTime        << " " << std::setw ( 20 ) << m_flashTime / m_totalTime.floatValue () * 100.0 << std::endl;
   buffer << " Flux times :            " << std::setw ( 20 ) << m_fluxTime         << " " << std::setw ( 20 ) << m_fluxTime / m_totalTime.floatValue () * 100.0 << std::endl;
   buffer << " Pressure times :        " << std::setw ( 20 ) << m_pressureTime     << " " << std::setw ( 20 ) << m_pressureTime / m_totalTime.floatValue () * 100.0 << std::endl;
   buffer << " Permeability times :    " << std::setw ( 20 ) << m_permeabilityTime << " " << std::setw ( 20 ) << m_permeabilityTime / m_totalTime.floatValue () * 100.0 << std::endl;
   buffer << " Previous time :         " << std::setw ( 20 ) << m_previousTime     << " " << std::setw ( 20 ) << m_previousTime / m_totalTime.floatValue () * 100.0 << std::endl;
   buffer << " Source time :           " << std::setw ( 20 ) << m_sourceTime       << " " << std::setw ( 20 ) << m_sourceTime / m_totalTime.floatValue () * 100.0 << std::endl;
   buffer << " Transport time :        " << std::setw ( 20 ) << m_transportTime    << " " << std::setw ( 20 ) << m_transportTime / m_totalTime.floatValue () * 100.0 << std::endl;
   buffer << " Mass time :             " << std::setw ( 20 ) << m_massTime         << " " << std::setw ( 20 ) << m_massTime / m_totalTime.floatValue () * 100.0 << std::endl;
   buffer << " Concentration time :    " << std::setw ( 20 ) << m_concTime         << " " << std::setw ( 20 ) << m_concTime / m_totalTime.floatValue () * 100.0 << std::endl;
   buffer << " Saturation time :       " << std::setw ( 20 ) << m_satTime          << " " << std::setw ( 20 ) << m_satTime / m_totalTime.floatValue () * 100.0 << std::endl;
   buffer << " Inner saturation time : " << std::setw ( 20 ) << m_sat2Time         << " " << std::setw ( 20 ) << m_sat2Time / m_totalTime.floatValue () * 100.0 << std::endl;
   buffer << " Estd. saturation time : " << std::setw ( 20 ) << m_estimatedSaturationTime << " " << std::setw ( 20 ) << m_estimatedSaturationTime / m_totalTime.floatValue () * 100.0 << std::endl;
   buffer << " Total time :         " << std::setw ( 20 ) << m_totalTime << std::endl;

   buffer << " Flashed and transported "
          << std::setw ( 10 ) << m_flashCount << "  "
          << std::setw ( 10 ) << m_transportInCount << "  " 
          << std::setw ( 10 ) << m_transportOutCount << "  "
          << std::setw ( 10 ) << m_transportCount << "  "
          << endl;

   PetscSynchronizedPrintf ( PETSC_COMM_WORLD, "%s", buffer.str ().c_str ());

}

//------------------------------------------------------------//

void ExplicitMultiComponentFlowSolver::solve ( Subdomain&   subdomain,
                                               const double startTime,
                                               const double endTime,
                                               DarcyErrorIndicator& errorOccurred ) {

   const MultiComponentFlowHandler& mcfHandler = FastcauldronSimulator::getInstance ().getMcfHandler ();

   PoreVolumeInterpolatorCalculator poreVolumeCalculator;
   PoreVolumeTemporalInterpolator poreVolumeInterpolator ( subdomain, poreVolumeCalculator );

   FaceAreaInterpolatorCalculator faceAreaCalculator;
   FaceAreaTemporalInterpolator faceAreaInterpolator ( subdomain, faceAreaCalculator );

   // First set the error indicator to false.
   errorOccurred = NO_DARCY_ERROR;

   WallTime::Time otgcStart;
   WallTime::Time flashStart;
   WallTime::Time fluxStart;
   WallTime::Time pressureStart;
   WallTime::Time permeabilityStart;
   WallTime::Time previousStart;
   WallTime::Time sourceStart;
   WallTime::Time transportStart;
   WallTime::Time massStart;
   WallTime::Time concStart;
   WallTime::Time satStart;
   WallTime::Time totalStart = WallTime::clock ();
   WallTime::Time iterationStart;

   WallTime::Duration otgcIntervalTime = 0.0;
   WallTime::Duration flashIntervalTime = 0.0;
   WallTime::Duration fluxIntervalTime = 0.0;
   WallTime::Duration pressureIntervalTime = 0.0;
   WallTime::Duration permeabilityIntervalTime = 0.0;

   WallTime::Duration previousIntervalTime = 0.0;
   WallTime::Duration sourceIntervalTime = 0.0;
   WallTime::Duration transportIntervalTime = 0.0;
   WallTime::Duration massIntervalTime = 0.0;
   WallTime::Duration concIntervalTime = 0.0;
   WallTime::Duration satIntervalTime = 0.0;
   WallTime::Duration sat2IntervalTime = 0.0;
   WallTime::Duration estimatedSaturationIntervalTime = 0.0;
   WallTime::Duration totalIntervalTime = 0.0;


   bool removeSourceTerm = FastcauldronSimulator::getInstance ().getMcfHandler ().removeSourceTerm ();
   double sourceTermRemovalAge = FastcauldronSimulator::getInstance ().getMcfHandler ().removeSourceTermAge ();

   bool stopHcTransport = FastcauldronSimulator::getInstance ().getMcfHandler ().removeHcTransport ();
   double stopHcTransportAge = FastcauldronSimulator::getInstance ().getMcfHandler ().removeHcTransportAge ();


   Subdomain::ActiveLayerIterator layerIter;

   // Time-step size in Ma.
   double deltaTMa;

   // Time-step size in seconds.
   double deltaTSec;

   double uniformDeltaTMa;
   double uniformDeltaTSec;
   double uniformFractionScaling;

   double timeStepStartMa;
   double timeStepEndMa = startTime;
   double timeStepStartSec;
   double timeStepEndSec;

   double lambdaStart;
   double lambdaEnd;

   double totalMassAddedForInterval = 0.0;
   double sourceMassAdded;
   double calculatedTimeStepSize;
   double previousTimeStepSize = m_maximumTimeStepSize;

   double fractionScaling;

   int iterationCount = 0;
   int uniformTimeStepCount;


   ConstrainedBooleanArrayMap currentAlreadyActivatedProperties;
   ConstrainedBooleanArrayMap previousAlreadyActivatedProperties;
   bool averagedSaturationsActivated;

   PhaseCompositionArray phaseComposition;
   SaturationArray       saturations;
   PhaseValueArray       phaseDensities;
   PhaseValueArray       phaseViscosities;
   CompositionArray      computedConcentrations;
   ScalarArray           transportedMasses;
   Boolean3DArray        elementContainsHc;
   Boolean3DArray        elementTransportsHc;
   CompositionArray      kValues;

   // Holds the pressure at the centre of every element in the sub-domain.
   Vec                   subdomainLiquidPressureVec;
   Vec                   subdomainVapourPressureVec;


   DarcyCalculations darcyCalculations;



   Vec                    elementGasFluxTermsVec;
   ElementFaceValueVector elementGasFluxTerms;

   Vec                    elementOilFluxTermsVec;
   ElementFaceValueVector elementOilFluxTerms;

   Vec                    subdomainPermeabilityNVec;
   ElementFaceValueVector subdomainPermeabilityN;

   Vec                    subdomainPermeabilityHVec;
   ElementFaceValueVector subdomainPermeabilityH;

   const ElementVolumeGrid& simpleGrid        = subdomain.getVolumeGrid ();
   const ElementVolumeGrid& concentrationGrid = subdomain.getVolumeGrid ( NumberOfPVTComponents );
   const ElementVolumeGrid& fluxGrid          = subdomain.getVolumeGrid ( ElementFaceValues::NumberOfFaces );
   const ElementVolumeGrid& permeabilityGrid  = subdomain.getVolumeGrid ( ElementFaceValues::NumberOfFaces );

   TemporalPropertyInterpolator porePressure ( subdomain );
   TemporalPropertyInterpolator temperature ( subdomain );
   TemporalPropertyInterpolator ves ( subdomain );
   TemporalPropertyInterpolator maxVes ( subdomain );
   TemporalPropertyInterpolator depth ( subdomain );

   int t;

   DMCreateGlobalVector ( fluxGrid.getDa (), &elementGasFluxTermsVec );
   elementGasFluxTerms.setVector ( fluxGrid, elementGasFluxTermsVec, INSERT_VALUES );

   DMCreateGlobalVector ( fluxGrid.getDa (), &elementOilFluxTermsVec );
   elementOilFluxTerms.setVector ( fluxGrid, elementOilFluxTermsVec, INSERT_VALUES );

   if ( not m_interpolateFacePermeability ) {
      DMCreateGlobalVector ( permeabilityGrid.getDa (), &subdomainPermeabilityNVec );
      DMCreateGlobalVector ( permeabilityGrid.getDa (), &subdomainPermeabilityHVec );
   }

   DMCreateGlobalVector ( simpleGrid.getDa (), &subdomainVapourPressureVec );
   DMCreateGlobalVector ( simpleGrid.getDa (), &subdomainLiquidPressureVec );

   VecZeroEntries ( subdomainVapourPressureVec );
   VecZeroEntries ( subdomainLiquidPressureVec );

   ScalarPetscVector subdomainVapourPressure;// ( simpleGrid, subdomainVapourPressureVec, INSERT_VALUES, true );
   ScalarPetscVector subdomainLiquidPressure;// ( simpleGrid, subdomainLiquidPressureVec, INSERT_VALUES, true );

   computedConcentrations.create ( concentrationGrid );
   kValues.create ( concentrationGrid );
   transportedMasses.create ( simpleGrid );

   activateProperties ( subdomain, currentAlreadyActivatedProperties, previousAlreadyActivatedProperties );

   // The use of the concentration-grid here is okay, since only the dimensions
   // of the grid are used and not the number of dofs.
   // mol/m^3
   phaseComposition.create ( concentrationGrid );

   saturations.create ( concentrationGrid );

   elementContainsHc.create ( concentrationGrid );
   elementTransportsHc.create ( concentrationGrid );

   // kg/m^3
   phaseDensities.create   ( concentrationGrid );

   // Viscosity units (kg/s/m? Pa.s?)
   phaseViscosities.create ( concentrationGrid );

   porePressure.setProperty ( Basin_Modelling::Pore_Pressure );
   temperature.setProperty ( Basin_Modelling::Temperature );
   ves.setProperty ( Basin_Modelling::VES_FP );
   maxVes.setProperty ( Basin_Modelling::Max_VES );
   depth.setProperty ( Basin_Modelling::Depth );


   if ( m_interpolateFacePermeability ) {
      permeabilityStart = WallTime::clock ();
   } 

   FacePermeabilityInterpolatorCalculator facePermeabilityCalculator ( subdomain );
   FacePermeabilityTemporalInterpolator facePermeabilityInterpolator ( subdomain, facePermeabilityCalculator );

   if ( m_interpolateFacePermeability ) {
      permeabilityIntervalTime += WallTime::clock () - permeabilityStart;
   }

   if ( not FastcauldronSimulator::getInstance ().getMcfHandler ().useAdaptiveTimeStepping ()) {

      if ( subdomain.sourceRockIsActive () and ( startTime - endTime ) > FastcauldronSimulator::getInstance ().getMcfHandler ().getMaximumTimeStepSize ()) {

         uniformTimeStepCount = int ( std::ceil (( startTime - endTime ) / FastcauldronSimulator::getInstance ().getMcfHandler ().getMaximumTimeStepSize ()));

         uniformFractionScaling = 1.0 / double ( uniformTimeStepCount );
         uniformDeltaTMa = ( startTime - endTime ) * uniformFractionScaling;
         uniformDeltaTSec = MillionYearToSecond * uniformDeltaTMa;
      } else {
         uniformTimeStepCount = 1;

         uniformFractionScaling = 1.0;
         uniformDeltaTMa = startTime - endTime;
         uniformDeltaTSec = MillionYearToSecond * uniformDeltaTMa;
      }

   }


   PVTPhaseComponents zeroPhaseComposition;
   PVTComponents zeroComposition;
   PVTComponents initialKValues;
   PVTPhaseValues zeroPhase;

   bool errorInSaturation;
   bool errorInConcentration;
   bool timeStepSubSamplePoint = false;
   bool timeSteppingFinished = false;
   double scaling = 1.0;

   double subSampledLambdaEnd;

   zeroPhaseComposition.zero ();
   zeroComposition.zero ();
   initialKValues.fill ( -1.0 );
   zeroPhase.zero ();

   // Copy saturation values from current to previous, before they are overwritten.
   subdomain.initialiseLayerIterator ( layerIter );

   SourceRocksTemporalInterpolator sourceRocksInterpolator ( subdomain );

   while ( not layerIter.isDone ()) {
      layerIter->getFormation ().copySaturations ();
      ++layerIter;
   }


   int applyOtgcCount = 0;
   double lastOtgcStartTime;

   timeStepStartMa = startTime;
   timeStepStartSec = startTime * MillionYearToSecond;
   lambdaStart = 0.0;

   transportedMasses.fill ( 0.0 );

   lastOtgcStartTime = timeStepStartMa;

   int ii, j, k;

   for ( ii = phaseComposition.firstI ( true ); ii <= phaseComposition.lastI ( true ); ++ii ) {
         
      for ( j = phaseComposition.firstJ ( true ); j <= phaseComposition.lastJ ( true ); ++j ) {

         for ( k = phaseComposition.firstK (); k <= phaseComposition.lastK (); ++k ) {
            kValues ( ii, j, k )( static_cast< pvtFlash::ComponentId >(0)) = -1.0;
         }

      }

   }

   while ( not timeSteppingFinished ) {

      ++iterationCount;
      iterationStart = WallTime::clock ();

      // Zero fluxes, ready for new valus.
      elementGasFluxTerms.restoreVector ( UPDATE_EXCLUDING_GHOSTS );
      elementOilFluxTerms.restoreVector ( UPDATE_EXCLUDING_GHOSTS );

      VecZeroEntries ( elementGasFluxTermsVec );
      VecZeroEntries ( elementOilFluxTermsVec );

      elementGasFluxTerms.setVector ( fluxGrid, elementGasFluxTermsVec, INSERT_VALUES, true );
      elementOilFluxTerms.setVector ( fluxGrid, elementOilFluxTermsVec, INSERT_VALUES, true );

      if ( not m_interpolateFacePermeability ) {
         VecZeroEntries ( subdomainPermeabilityNVec );
         VecZeroEntries ( subdomainPermeabilityHVec );
      }

      elementContainsHc.fill ( false );
      elementTransportsHc.fill ( false );

      int ii, j, k;

      for ( ii = phaseComposition.firstI ( true ); ii <= phaseComposition.lastI ( true ); ++ii ) {
         
         for ( j = phaseComposition.firstJ ( true ); j <= phaseComposition.lastJ ( true ); ++j ) {

            for ( k = phaseComposition.firstK (); k <= phaseComposition.lastK (); ++k ) {
               computedConcentrations ( ii, j, k ).zero ();
            }

         }

      }

      satStart = WallTime::clock ();
      estimateHcTransport ( subdomain, elementContainsHc, elementTransportsHc );
      estimatedSaturationIntervalTime += WallTime::clock () - satStart;

      // Flash the (previous-) components for all the elements in the subdomain.
      flashStart = WallTime::clock ();
      m_flashCount += darcyCalculations.flashComponents ( subdomain, phaseComposition, phaseDensities, phaseViscosities, porePressure, temperature, lambdaStart, kValues, elementTransportsHc );
      flashIntervalTime += WallTime::clock () - flashStart;

      // now determine:
      //    i. flux
      //   ii. time-step size.
      satStart = WallTime::clock ();
      darcyCalculations.setSaturations ( subdomain, phaseComposition, phaseDensities, elementTransportsHc, saturations, errorInSaturation );
      sat2IntervalTime += WallTime::clock () - satStart;

      if ( errorInSaturation ) {
         errorOccurred = ERROR_CALCULATING_SATURATION;
         break;
      }

      pressureStart = WallTime::clock ();

      // Compute the flux term for the boundary of each element.
      subdomainVapourPressure.setVector ( simpleGrid, subdomainVapourPressureVec, INSERT_VALUES );
      subdomainLiquidPressure.setVector ( simpleGrid, subdomainLiquidPressureVec, INSERT_VALUES );

      computePressure ( subdomain, saturations,
                        porePressure, temperature, ves, maxVes, lambdaStart,
                        subdomainVapourPressure, subdomainLiquidPressure );

      subdomainVapourPressure.restoreVector ( UPDATE_EXCLUDING_GHOSTS );
      subdomainLiquidPressure.restoreVector ( UPDATE_EXCLUDING_GHOSTS );

      // Update vectors from neighbouring processes.
      subdomainVapourPressure.setVector ( simpleGrid, subdomainVapourPressureVec, INSERT_VALUES, true );
      subdomainLiquidPressure.setVector ( simpleGrid, subdomainLiquidPressureVec, INSERT_VALUES, true );

      pressureIntervalTime += WallTime::clock () - pressureStart;

      if ( not m_interpolateFacePermeability ) {
         // Compute the flux term for the boundary of each element.
         permeabilityStart = WallTime::clock ();

         // Average permeabilities.
         darcyCalculations.computeAveragePermeabilities ( subdomain, lambdaStart, lambdaEnd, subdomainPermeabilityNVec, subdomainPermeabilityHVec );

         // Update permeability values from neighbouring processes.
         subdomainPermeabilityN.setVector ( permeabilityGrid, subdomainPermeabilityNVec, INSERT_VALUES, true );
         subdomainPermeabilityH.setVector ( permeabilityGrid, subdomainPermeabilityHVec, INSERT_VALUES, true );

         permeabilityIntervalTime += WallTime::clock () - permeabilityStart;
      }

      // Compute the flux term for the boundary of each element.
      fluxStart = WallTime::clock ();

      // Compute the rate at which hc leaves the element.
      // At the same time compute the maximu time-step size so as to remain a positive concentration.
      computeFluxTerms ( subdomain, elementTransportsHc,
                         phaseComposition, phaseDensities, phaseViscosities, 
                         subdomainPermeabilityN, subdomainPermeabilityH, facePermeabilityInterpolator,
                         subdomainVapourPressure,
                         subdomainLiquidPressure,
                         depth, porePressure, faceAreaInterpolator, poreVolumeInterpolator, saturations,
                         elementGasFluxTerms, elementOilFluxTerms,
                         lambdaStart, calculatedTimeStepSize );

      if ( mcfHandler.useAdaptiveTimeStepping ()) {

         if ( mcfHandler.applyTimeStepSmoothing () and calculatedTimeStepSize > previousTimeStepSize ) {
            calculatedTimeStepSize = NumericFunctions::Minimum ( mcfHandler.getTimeStepSmoothingFactor () * previousTimeStepSize,
                                                                 calculatedTimeStepSize );
         }

         calculatedTimeStepSize = NumericFunctions::Minimum ( calculatedTimeStepSize, startTime - endTime );
         previousTimeStepSize = calculatedTimeStepSize;

         deltaTMa = calculatedTimeStepSize;
         fractionScaling = deltaTMa / ( startTime - endTime );

         if ( lambdaStart + 1.05 * fractionScaling >= 1.0 ) {
            // If we are here then we are at, or very close 5%, of the end of the interval.
            timeSteppingFinished = true;

            lambdaEnd = 1.0;
            deltaTMa = timeStepEndMa - endTime;
            fractionScaling = deltaTMa / ( startTime - endTime );

         } else {
            lambdaEnd = lambdaStart + fractionScaling;
         }

         deltaTSec = deltaTMa * MillionYearToSecond;
         timeStepEndMa  -= deltaTMa;
         timeStepEndSec -= deltaTSec;

      } else {

         deltaTMa = uniformDeltaTMa;
         deltaTSec = uniformDeltaTSec;

         timeStepEndMa  -= uniformDeltaTMa;
         timeStepEndSec -= uniformDeltaTSec;
         fractionScaling = uniformFractionScaling;
         lambdaEnd += uniformFractionScaling;

         timeSteppingFinished = ( iterationCount == uniformTimeStepCount );
      }

      scaleFluxTermsByTimeStep ( subdomain, elementTransportsHc, elementGasFluxTerms, elementOilFluxTerms, deltaTSec );
      fluxIntervalTime += WallTime::clock () - fluxStart;

      if ( FastcauldronSimulator::getInstance ().getMcfHandler ().getDebugLevel () > 0 ) {
         PetscPrintf ( PETSC_COMM_WORLD, " calculated time step size: %f   %f   %f   %f   %f   %f   %f\n", 
                       deltaTMa,
                       lambdaStart, lambdaEnd, fractionScaling,
                       timeStepStartMa, timeStepEndMa, endTime );
      }

      // update fluxes on all processors.
      elementGasFluxTerms.restoreVector ( UPDATE_EXCLUDING_GHOSTS );
      elementGasFluxTerms.setVector ( fluxGrid, elementGasFluxTermsVec, INSERT_VALUES, true );

      elementOilFluxTerms.restoreVector ( UPDATE_EXCLUDING_GHOSTS );
      elementOilFluxTerms.setVector ( fluxGrid, elementOilFluxTermsVec, INSERT_VALUES, true );

      if ( not m_interpolateFacePermeability ) {
      // Zero permeabilities, ready for new values.
      subdomainPermeabilityN.restoreVector ( UPDATE_EXCLUDING_GHOSTS );
      subdomainPermeabilityH.restoreVector ( UPDATE_EXCLUDING_GHOSTS );
      }

      subdomainVapourPressure.restoreVector ( NO_UPDATE );
      subdomainLiquidPressure.restoreVector ( NO_UPDATE );

      // Compute the contributions from previous time-step.
      // This is first function in the sequence to add to the computed-concentrations.
      previousStart = WallTime::clock ();
      computeTemporalContributions ( subdomain, elementContainsHc, computedConcentrations, poreVolumeInterpolator, lambdaStart, lambdaEnd );
      previousIntervalTime += WallTime::clock () - previousStart;

      if ( not stopHcTransport or timeStepEndMa >= stopHcTransportAge ) {
         // Since the flux terms may not added here, due to stopHcTransport, should they be calculated?
         // Now add the flux terms to the concentrations.
         transportStart = WallTime::clock ();
         // The elementContainsHc array may be updated here if there is transport of hc into an element that previously had none.
         transportComponents ( subdomain, phaseComposition, elementGasFluxTerms, elementOilFluxTerms, elementContainsHc, computedConcentrations, transportedMasses );
         transportIntervalTime += WallTime::clock () - transportStart;
      }

      if ( not removeSourceTerm or timeStepEndMa >= sourceTermRemovalAge ) {
         // integrate over the element the hc generated in genex.
         sourceStart = WallTime::clock ();
         darcyCalculations.computeSourceTerm ( subdomain, computedConcentrations, elementContainsHc, sourceRocksInterpolator, lambdaStart, fractionScaling, sourceMassAdded );
         totalMassAddedForInterval += sourceMassAdded;
         sourceIntervalTime += WallTime::clock () - sourceStart;
      }

      // Divide the computed concentrations by the mass-matrix.
      massStart = WallTime::clock ();
      divideByMassMatrix ( subdomain, elementContainsHc, poreVolumeInterpolator, computedConcentrations, lambdaStart, lambdaEnd );
      massIntervalTime += WallTime::clock () - massStart;

      // Assign the concentration to each element.
      concStart = WallTime::clock ();
      darcyCalculations.setConcentrations ( subdomain, elementContainsHc, computedConcentrations, errorInConcentration );
      concIntervalTime += WallTime::clock () - concStart;

      if ( errorInConcentration ) {
         errorOccurred = ERROR_CALCULATING_CONCENTRATION;
         break;
      }

      // Apply OTGC to (previous-) components for all elements in the subdomain.
      otgcStart = WallTime::clock ();

#if 0
      ++applyOtgcCount;

      if ( applyOtgcCount == 4 or lambdaEnd == 1.0 ) {
         darcyCalculations.applyOtgc ( elementContainsHc, porePressure, temperature, lastOtgcStartTime, timeStepEndMa, lambdaStart, lambdaEnd );
         lastOtgcStartTime = timeStepEndMa;
         applyOtgcCount = 0;
      }
#else
      darcyCalculations.applyOtgc ( subdomain, elementContainsHc, porePressure, temperature, timeStepStartMa, timeStepEndMa, lambdaStart, lambdaEnd );
#endif

      otgcIntervalTime += WallTime::clock () - otgcStart;


      // Set start values to end values of last time-step.
      timeStepStartSec = timeStepEndSec;
      timeStepStartMa  = timeStepEndMa;
      lambdaStart = lambdaEnd;

      }

   if ( errorOccurred == NO_DARCY_ERROR ) {

      // Assign the saturations to each element.
      satStart = WallTime::clock ();
      darcyCalculations.setSaturations ( subdomain, kValues, errorInSaturation );

      if ( errorInSaturation ) {
         errorOccurred = ERROR_CALCULATING_SATURATION;
      } else {
         averageGlobalSaturation ( subdomain );
      }

      satIntervalTime = WallTime::clock () - satStart;

      //set time of element invasion
      darcyCalculations.setTimeOfElementInvasion ( subdomain, endTime );
      updateTransportedMasses ( subdomain, transportedMasses );
   }

   if ( errorOccurred != NO_DARCY_ERROR ) {
      PetscPrintf ( PETSC_COMM_WORLD, " Error in Darcy occurred at %f Ma\n", timeStepStartMa );
   }


   if ( FastcauldronSimulator::getInstance ().getMcfHandler ().getDebugLevel () > 1 ) {
      double totalMass = darcyCalculations.totalHcMass ( subdomain, 1.0 );

      PetscPrintf ( PETSC_COMM_WORLD, " Total mass in system           : %f \n", totalMass );
      PetscPrintf ( PETSC_COMM_WORLD, " Total mass added over interval : %f \n", totalMassAddedForInterval );
   }

   // 'Deactivate' all those properties that we not already 'activated'.
   deactivateProperties ( subdomain, currentAlreadyActivatedProperties, previousAlreadyActivatedProperties );

   if ( not m_interpolateFacePermeability ) {
      VecDestroy ( &subdomainPermeabilityNVec );
      VecDestroy ( &subdomainPermeabilityHVec );
   }

   VecDestroy ( &subdomainVapourPressureVec );
   VecDestroy ( &subdomainLiquidPressureVec );
   VecDestroy ( &elementGasFluxTermsVec );
   VecDestroy ( &elementOilFluxTermsVec );

   totalIntervalTime += WallTime::clock () - totalStart;

   m_otgcTime         += otgcIntervalTime;
   m_flashTime        += flashIntervalTime;
   m_fluxTime         += fluxIntervalTime;
   m_pressureTime     += pressureIntervalTime;
   m_permeabilityTime += permeabilityIntervalTime;
   m_previousTime     += previousIntervalTime;
   m_sourceTime       += sourceIntervalTime;
   m_transportTime    += transportIntervalTime;
   m_massTime         += massIntervalTime;
   m_concTime         += concIntervalTime;
   m_satTime          += satIntervalTime;
   m_totalTime        += totalIntervalTime;
   m_sat2Time         += sat2IntervalTime;
   m_estimatedSaturationTime += estimatedSaturationIntervalTime;

   if ( FastcauldronSimulator::getInstance ().getMcfHandler ().getDebugLevel () > 0 ) {
      std::stringstream buffer;

      buffer << " Operation                time-step time            cumulative time  " << std::endl;
      buffer << " OTGC                       " << std::setw ( 20 ) << otgcIntervalTime         << "  " << std::setw ( 20 ) << m_otgcTime         << std::endl;
      buffer << " Flash composition          " << std::setw ( 20 ) << flashIntervalTime        << "  " << std::setw ( 20 ) << m_flashTime        << std::endl;
      buffer << " Compute Fluxes             " << std::setw ( 20 ) << fluxIntervalTime         << "  " << std::setw ( 20 ) << m_fluxTime         << std::endl;
      buffer << " Pressure times             " << std::setw ( 20 ) << pressureIntervalTime     << "  " << std::setw ( 20 ) << m_pressureTime     << std::endl;
      buffer << " Permeability times         " << std::setw ( 20 ) << permeabilityIntervalTime << "  " << std::setw ( 20 ) << m_permeabilityTime << std::endl;
      buffer << " Previous contributions     " << std::setw ( 20 ) << previousIntervalTime     << "  " << std::setw ( 20 ) << m_previousTime     << std::endl;
      buffer << " Compute Source term        " << std::setw ( 20 ) << sourceIntervalTime       << "  " << std::setw ( 20 ) << m_sourceTime       << std::endl;
      buffer << " Transport composition      " << std::setw ( 20 ) << transportIntervalTime    << "  " << std::setw ( 20 ) << m_transportTime    << std::endl;
      buffer << " Compute Mass matrix        " << std::setw ( 20 ) << massIntervalTime         << "  " << std::setw ( 20 ) << m_massTime         << std::endl;
      buffer << " Compute next concentration " << std::setw ( 20 ) << concIntervalTime         << "  " << std::setw ( 20 ) << m_concTime         << std::endl;
      buffer << " Compute inner saturation   " << std::setw ( 20 ) << sat2IntervalTime         << "  " << std::setw ( 20 ) << m_sat2Time         << std::endl;
      buffer << " Compute next saturation    " << std::setw ( 20 ) << satIntervalTime          << "  " << std::setw ( 20 ) << m_satTime          << std::endl;
      buffer << " Total time :               " << std::setw ( 20 ) << totalIntervalTime        << "  " << std::setw ( 20 ) << m_totalTime        << std::endl;

      buffer << " Flashed and transported "
             << std::setw ( 10 ) << m_flashCount << "  "
             << std::setw ( 10 ) << m_transportInCount << "  " 
             << std::setw ( 10 ) << m_transportOutCount << "  "
             << std::setw ( 10 ) << m_transportCount << "  "
             << endl;

      PetscSynchronizedPrintf ( PETSC_COMM_WORLD, buffer.str ().c_str ());
      PetscSynchronizedFlush ( PETSC_COMM_WORLD, PETSC_STDOUT );
   }

}

//------------------------------------------------------------//


void ExplicitMultiComponentFlowSolver::computePressure ( FormationSubdomainElementGrid&      formationGrid,
                                                         const SaturationArray&              saturations,
                                                         const TemporalPropertyInterpolator& porePressure,
                                                         const TemporalPropertyInterpolator& temperature,
                                                         const TemporalPropertyInterpolator& ves,
                                                         const TemporalPropertyInterpolator& maxVes,
                                                         const double                        lambda,
                                                         ScalarPetscVector&                  vapourPressure,
                                                         ScalarPetscVector&                  liquidPressure ) {

   LayerProps& theLayer = formationGrid.getFormation ();

   const ElementVolumeGrid& concentrationGrid = theLayer.getVolumeGrid ( NumberOfPVTComponents );
   const ElementVolumeGrid& saturationGrid = theLayer.getVolumeGrid ( Saturation::NumberOfPhases );

   int i;
   int j;
   int k;

   CompoundProperty compoundPorosity;
   double elementPorePressure;
   double elementVes;
   double elementMaxVes;
   double capillaryPressure;
   double permeabilityNormal;
   double permeabilityPlane;

   for ( i = concentrationGrid.firstI (); i <= concentrationGrid.lastI (); ++i ) {

      for ( j = concentrationGrid.firstJ (); j <= concentrationGrid.lastJ (); ++j ) {

         if ( FastcauldronSimulator::getInstance ().getMapElement ( i, j ).isValid ()) {

            for ( k = formationGrid.lastK (); k >= formationGrid.firstK (); --k ) {
               SubdomainElement& element = formationGrid.getElement ( i, j, k );
               const LayerElement& layerElement = element.getLayerElement ();

               if ( layerElement.isActive ()) {

                  const Lithology* lithology = layerElement.getLithology ();

                  const Saturation& saturation = saturations ( element.getI (), element.getJ (), element.getK ());

                  elementPorePressure = 1.0e6 * porePressure ( element, lambda );
                  elementVes = ves ( element, lambda );
                  elementMaxVes = maxVes ( element, lambda );

                  lithology->getPorosity ( elementVes,
                                           elementMaxVes,
                                           false, 0.0,
                                           compoundPorosity );

                  lithology->calcBulkPermeabilityNP ( elementVes, elementMaxVes, compoundPorosity, permeabilityNormal, permeabilityPlane );

                  // Compute liquid-pressure.
                  capillaryPressure = lithology->capillaryPressure ( Saturation::LIQUID, saturation, permeabilityNormal );
                  liquidPressure ( element.getK (), element.getJ (), element.getI ()) = elementPorePressure + capillaryPressure;

                  // Compute vapour-pressure.
                  capillaryPressure = lithology->capillaryPressure ( Saturation::VAPOUR, saturation, permeabilityNormal );
                  vapourPressure ( element.getK (), element.getJ (), element.getI ()) = elementPorePressure + capillaryPressure;
               }

            }

         }

      }

   }

}

//------------------------------------------------------------//

void ExplicitMultiComponentFlowSolver::computePressure ( Subdomain&                          subdomain,
                                                         const SaturationArray&              saturations,
                                                         const TemporalPropertyInterpolator& porePressure,
                                                         const TemporalPropertyInterpolator& temperature,
                                                         const TemporalPropertyInterpolator& ves,
                                                         const TemporalPropertyInterpolator& maxVes,
                                                         const double                        lambda,
                                                         ScalarPetscVector&                  vapourPressure,
                                                         ScalarPetscVector&                  liquidPressure ) {


   Subdomain::ActiveLayerIterator iter;

   subdomain.initialiseLayerIterator ( iter );

   while ( not iter.isDone ()) {
      computePressure ( *iter, saturations, porePressure, temperature, ves, maxVes, lambda, vapourPressure, liquidPressure );
      ++iter;
   }

}

//------------------------------------------------------------//

void ExplicitMultiComponentFlowSolver::computeNumericalFlux ( const SubdomainElement& element,
                                                              const pvtFlash::PVTPhase  phase,
                                                              const double            elementFlux,
                                                              const double            neighbourFlux,
                                                              const double              elementPhaseCompositionSum,
                                                              const PVTPhaseComponents& elementComposition,
                                                              const PVTPhaseComponents& neighbourComposition,
                                                                    PVTComponents&    flux,
                                                                    double&           transportedMassesIn,
                                                                    double&           transportedMassesOut ) {

   //
   // Upwinding flux function.
   //
   //                    {   f(c+) . n x+ if f(c+) . n >= 0.0 outflow
   //  H ( c+, c-, n ) = {
   //                    {   f(c-) . n x- if f(c-) . n < 0.0  inflow
   //

   if ( elementFlux > 0.0 ) {

   // Subtract outflow
      if ( elementPhaseCompositionSum > HcConcentrationLowerLimit ) {
         int c;
         double scalar = -elementFlux / elementPhaseCompositionSum;

         for ( c = 0; c < NumberOfPVTComponents; ++c ) {
            pvtFlash::ComponentId component = static_cast<pvtFlash::ComponentId>( c );
            flux ( component ) = scalar * elementComposition ( phase, component ) * m_defaultMolarMasses  ( component );
         }

         // Add mass transported out of element.
         transportedMassesOut += elementFlux;
      } else {
         flux.zero ();
      }

   } else {
      flux.zero ();
   }

   if ( neighbourFlux > 0.0 ) {
      double sum;
      sum = neighbourComposition.sum ( phase );

   // Add in-flow from neighbour.
      if ( sum > HcConcentrationLowerLimit ) {

         int c;
         double scalar = neighbourFlux / sum;

         for ( c = 0; c < NumberOfPVTComponents; ++c ) {
            pvtFlash::ComponentId component = static_cast<pvtFlash::ComponentId>( c );
            flux ( component ) += scalar * neighbourComposition ( phase, component ) * m_defaultMolarMasses  ( component );
         }

      // Add mass transported into element.
         transportedMassesIn += neighbourFlux;
      }

   }

}
                                                              

//------------------------------------------------------------//

void ExplicitMultiComponentFlowSolver::transportComponents ( const SubdomainElement&       element,
                                                             const pvtFlash::PVTPhase      phase,
                                                             const ElementFaceValueVector& elementFluxes,
                                                             const PhaseCompositionArray&  phaseComposition,
                                                                   PVTComponents&          computedConcentrations,
                                                                   double&                 transportedMassesIn,
                                                                   double&                 transportedMassesOut ) {

   PVTComponents transportedComponents;
   PVTComponents totalTransportedComponents;
   PVTPhaseComponents zeroComponents;

   // const SubdomainElement* activeNeighbours [ VolumeData::NumberOfBoundaries ] = { 0, 0, 0, 0, 0, 0 };

   int face;

   bool hasInFlow = false;
   bool hasOutFlow = false;

   for ( face = VolumeData::GAMMA_1; face <= VolumeData::GAMMA_6; ++face ) {
      const VolumeData::BoundaryId id = static_cast<VolumeData::BoundaryId>( face );
      const VolumeData::BoundaryId opposite = VolumeData::opposite ( id );
      const SubdomainElement* neighbour = element.getActiveNeighbour ( id );

      // activeNeighbours [ face ] = neighbour;

      if ( neighbour != 0 ) {

         if ( elementFluxes ( neighbour->getK (), neighbour->getJ (), neighbour->getI ())( opposite ) > 0.0 ) {
            hasInFlow = true;
            break;
         }

      }

      if ( elementFluxes ( element.getK (), element.getJ (), element.getI ())( id ) > 0.0 ) {
         hasOutFlow = true;
         break;
      }

   }

   if ( not hasInFlow and not hasOutFlow ) {
      return;
   }

   double elementPhaseCompositionSum = phaseComposition ( element.getI (), element.getJ (), element.getK ()).sum ( phase );
   transportedComponents.zero ();
   zeroComponents.zero ();
   totalTransportedComponents.zero ();


   for ( face = VolumeData::GAMMA_1; face <= VolumeData::GAMMA_6; ++face ) {

      const VolumeData::BoundaryId id = static_cast<VolumeData::BoundaryId>( face );
      const VolumeData::BoundaryId opposite = VolumeData::opposite ( id );
      // const SubdomainElement* neighbour = activeNeighbours [ id ];
      const SubdomainElement* neighbour = element.getActiveNeighbour ( id );

      transportedComponents.zero ();

      if ( neighbour != 0 ) {
         computeNumericalFlux ( element,
                                phase,
                                elementFluxes ( element.getK (), element.getJ (), element.getI ())( id ),
                                elementFluxes ( neighbour->getK (), neighbour->getJ (), neighbour->getI ())( opposite ),
                                elementPhaseCompositionSum,
                                phaseComposition ( element.getI (), element.getJ (), element.getK ()),
                                phaseComposition ( neighbour->getI (), neighbour->getJ (), neighbour->getK ()),
                                transportedComponents,
                                transportedMassesIn,
                                transportedMassesOut );

      } else {

         computeNumericalFlux ( element,
                                phase,
                                elementFluxes ( element.getK (), element.getJ (), element.getI ())( id ),
                                0.0,
                                elementPhaseCompositionSum,
                                phaseComposition ( element.getI (), element.getJ (), element.getK ()),
                                zeroComponents,
                                transportedComponents,
                                transportedMassesIn,
                                transportedMassesOut );
      }

      computedConcentrations += transportedComponents;
   }

}

//------------------------------------------------------------//

void ExplicitMultiComponentFlowSolver::transportComponents ( FormationSubdomainElementGrid&   formationGrid,
                                                             const ElementVolumeGrid&         concentrationGrid,
                                                             const PhaseCompositionArray&     phaseComposition,
                                                             const ElementFaceValueVector&    gasFluxes,
                                                             const ElementFaceValueVector&    oilFluxes,
                                                                   Boolean3DArray&            elementContainsHc,
                                                                   CompositionArray&          computedConcentrations,
                                                                   ScalarArray&               transportedMasses ) {

   const ElementGrid& elementGrid = FastcauldronSimulator::getInstance ().getElementGrid ();

   int i;
   int j;
   int k;
   int face;

#if 0
   int elementCount = 0;
   int elementCountWithTransportIn = 0;
   int elementCountWithTransportOut = 0;
   int elementCountWithHc = 0;
   int elementCountWithSor = 0;
#endif

   double massTransportedIn;
   double massTransportedOut;

   for ( i = elementGrid.firstI (); i <= elementGrid.lastI (); ++i ) {

      for ( j = elementGrid.firstJ (); j <= elementGrid.lastJ (); ++j ) {

         for ( k = formationGrid.lastK (); k >= formationGrid.firstK (); --k ) {

            const SubdomainElement& element = formationGrid.getElement ( i, j, k );
            const LayerElement& layerElement = element.getLayerElement ();

            unsigned int elementK = element.getK ();

            if ( layerElement.isActive ()) {

               massTransportedIn = 0.0;
               massTransportedOut = 0.0;

               transportComponents ( element,
                                     pvtFlash::VAPOUR_PHASE,
                                     gasFluxes,
                                     phaseComposition,
                                     computedConcentrations ( i, j, elementK ),
                                     massTransportedIn,
                                     massTransportedOut );

               transportComponents ( element,
                                     pvtFlash::LIQUID_PHASE,
                                     oilFluxes,
                                     phaseComposition,
                                     computedConcentrations ( i, j, elementK ),
                                     massTransportedIn,
                                     massTransportedOut );

               transportedMasses ( i, j, elementK ) += massTransportedIn + massTransportedOut;

               if ( massTransportedIn > 0.0 ) {
                  ++m_transportInCount;
               }

               if ( massTransportedOut > 0.0 ) {
                  ++m_transportOutCount;
               }

               if ( massTransportedIn > 0.0 or massTransportedOut > 0.0 ) {
                  ++m_transportCount;
               }

               if ( massTransportedIn > 0.0 or massTransportedOut > 0.0 ) {
                  elementContainsHc ( element.getI (), element.getJ (), element.getK ()) = true;
               }

            }

         }

      }

   }

}

//------------------------------------------------------------//

void ExplicitMultiComponentFlowSolver::transportComponents ( Subdomain&                       subdomain,
                                                             const PhaseCompositionArray&     phaseComposition,
                                                             const ElementFaceValueVector&    gasFluxes,
                                                             const ElementFaceValueVector&    oilFluxes,
                                                             Boolean3DArray&                  elementContainsHc,
                                                             CompositionArray&                computedConcentrations,
                                                             ScalarArray&                     transportedMasses ) {

   Subdomain::ActiveLayerIterator iter;

   subdomain.initialiseLayerIterator ( iter );

   while ( not iter.isDone ()) {
      transportComponents ( *iter,
                            subdomain.getVolumeGrid ( NumberOfPVTComponents ),
                            phaseComposition,
                            gasFluxes, oilFluxes,
                            elementContainsHc,
                            computedConcentrations,
                            transportedMasses );
      ++iter;
   }

}

//------------------------------------------------------------//

double ExplicitMultiComponentFlowSolver::computeElementFaceFlux ( const SubdomainElement&                   element, 
                                                                  const FaceAreaTemporalInterpolator&       faceAreaInterpolator,
                                                                  const VolumeData::BoundaryId              face,
                                                                  const Saturation::Phase                   phase,
                                                                  const double                              elementPressure,
                                                                  const double                              neighbourPressure,
                                                                  const double                              deltaX,
                                                                  const double                              phaseDensity,
                                                                  const double                              phaseViscosity,
                                                                  const double                              relativePermeability,
                                                                        FiniteElementMethod::FiniteElement& finiteElement,
                                                                  const double                              permNormal,
                                                                  const double                              permPlane,
                                                                  const double                              lambda,
                                                                  const bool                                print ) const {

   if ( not element.getLayerElement ().isActive () or not element.getLayerElement ().isActiveBoundary ( face )) {
      return 0.0;
   }

   const LayerElement& layerElement = element.getLayerElement ();

   const Lithology* lithology = layerElement.getLithology ();

   const LayerProps* layer = layerElement.getFormation ();

   ElementVector    phasePressure;
   ThreeVector      normal;
   ThreeVector      massFlux;
   double           faceFlux = 0.0;
   double           weight;
   double           dsDt;

   if ( relativePermeability == 0.0 ) {
      // If the relative permeability is zero then there cannot be any flow.
      return 0.0;
   }

   double pressureGradient = 0.0;
   double permeabilityValue = -1.0;

   switch ( face ) 
   {

     case VolumeData::ShallowFace : 
        pressureGradient = (( elementPressure - neighbourPressure ) / deltaX - phaseDensity * AccelerationDueToGravity);
        permeabilityValue = permNormal;
        break;

     case VolumeData::DeepFace : 
        pressureGradient = -(( neighbourPressure - elementPressure ) / deltaX - phaseDensity * AccelerationDueToGravity);
        permeabilityValue = permNormal;
        break;

     case VolumeData::Front: 
	 case VolumeData::Back:
	 case VolumeData::Left:
	 case VolumeData::Right:
        pressureGradient = -( neighbourPressure - elementPressure ) / deltaX;
        permeabilityValue = permPlane;
        break;
     default :
        cout << " ERROR " << endl;
   }

   if ( m_limitFluxPermeability ) {
      permeabilityValue = NumericFunctions::Minimum ( permeabilityValue, m_fluxPermeabilityMaximum );
   }

   if ( m_limitGradPressure ) {
      pressureGradient = NumericFunctions::Minimum ( pressureGradient, m_gradPressureMaximum );
   }

   if ( m_interpolateFaceArea ) {
      faceFlux = faceAreaInterpolator.access ( element ).evaluate ( face, lambda ) * relativePermeability * permeabilityValue * pressureGradient / phaseViscosity;
   } else {
      NumericFunctions::Quadrature3D::Iterator quad;
      NumericFunctions::Quadrature3D::getInstance ().get ( m_faceQuadratureDegree, face, quad );

      for ( quad.initialise (); not quad.isDone (); ++quad ) {
         finiteElement.setQuadraturePoint ( quad.getX (), quad.getY (), quad.getZ ());
         getElementBoundaryNormal ( layerElement, finiteElement.getJacobian (), face, normal, dsDt );
         weight = dsDt * quad.getWeight ();

         // The face-flux can be scaled by some of these values after the 
         // end of the loop, since they are invariant within the loop.
         faceFlux += weight * relativePermeability * permeabilityValue * pressureGradient / phaseViscosity;
      }

   }

   // Units: 
   return faceFlux;
}

//------------------------------------------------------------//

void ExplicitMultiComponentFlowSolver::computeFluxForPhase ( const pvtFlash::PVTPhase                  phase,
                                                             const SubdomainElement&                   element,
                                                             const FaceAreaTemporalInterpolator&       faceAreaInterpolator,
                                                                   FiniteElementMethod::FiniteElement& finiteElement,
                                                             const PVTPhaseValues&                     phases,
                                                             const ScalarPetscVector&                  subdomainPhasePressure,
                                                             const TemporalPropertyInterpolator&       depth,
                                                             const TemporalPropertyInterpolator&       porePressure,
                                                             const double                              lambda,
                                                             const double                              phaseMassDensity,
                                                             const double                              phaseMolarMass,
                                                             const PVTPhaseValues&                     phaseDensities,
                                                             const PVTPhaseValues&                     phaseViscosities,
                                                             const Saturation&                         elementSaturation,
                                                             const double                              relativePermeability,
                                                             const ElementFaceValues&                  elementPermeabilityN,
                                                             const ElementFaceValues&                  elementPermeabilityH,
                                                             const FacePermeabilityTemporalInterpolator& permeabilityInterpolator,
                                                                   ElementFaceValues&                  elementFlux ) {

   const CauldronGridDescription& gridDescription = FastcauldronSimulator::getInstance ().getCauldronGridDescription ();

   static const int NumberOfFluxFaces = 6;
   static const VolumeData::BoundaryId fluxFaces [ NumberOfFluxFaces ] = { VolumeData::GAMMA_2, VolumeData::GAMMA_3, VolumeData::GAMMA_4,
                                                                           VolumeData::GAMMA_5, VolumeData::GAMMA_1, VolumeData::GAMMA_6 };
   double elementPressure;
   double neighbourPressure;
   double deltaX = 100.0;
   double permeabilityNormal;
   double permeabilityPlane;

   Saturation::Phase whichPhase = Saturation::convert ( phase );

   int    face;

   if ( relativePermeability > 0.0 and phases ( phase ) > HcConcentrationLowerLimit ) {

      elementPressure = subdomainPhasePressure ( element.getK (), element.getJ (), element.getI ());

      for ( face = 0; face < NumberOfFluxFaces; ++face ) {
         const VolumeData::BoundaryId id = fluxFaces [ face ];

         double v3 = 0.0;

         if ( element.getLayerElement ().isActiveBoundary ( id )) {

            const SubdomainElement* neighbour = element.getActiveNeighbour ( id );

            if ( neighbour != 0 ) {
               neighbourPressure = subdomainPhasePressure ( neighbour->getK (), neighbour->getJ (), neighbour->getI ());
            } else {
               // What to put here!
               // neighbourPressure = NumericFunctions::Maximum ( 0.0, 1.0e6 * porePressure ( element, lambda ) - 1.0e6 );

               switch ( id ) {

               case VolumeData::GAMMA_1 : 
                  // Estimate pore-pressure in element above.
                  // Should really get element above, but this is not a part of the darcy subdomain.
                  neighbourPressure = NumericFunctions::Maximum ( 1.0e5, 1.0e6 * porePressure ( element, lambda ) - 1.0e6 );
                  break;

               case VolumeData::GAMMA_6 : 
                  // Estimate pore-pressure in element below.
                  // Should really get element below, but this is not a part of the darcy subdomain.
                  neighbourPressure = NumericFunctions::Maximum ( 1.0e5, 1.0e6 * porePressure ( element, lambda ) + 1.0e6 );
                  break;

               default :
                  // Use same pressure in neighbour to ensure that there is no flow out of the sides of the domain.
                  neighbourPressure = elementPressure;

               }

            }

            if ( id == VolumeData::GAMMA_1 or id == VolumeData::GAMMA_6 ) {

               if ( neighbour != 0 ) {
                  deltaX = abs ( depth ( element, lambda ) - depth ( *neighbour, lambda ));
                  // deltaX = abs ( centreOfElement ( layerElement ) - centreOfElement ( neighbour->getLayerElement ()));
               } else {
                  // What to put here!
                  deltaX = 100;
               }

            } else if ( id == VolumeData::GAMMA_2 or id == VolumeData::GAMMA_4 ) {
               deltaX = gridDescription.deltaJ;
            } else if ( id == VolumeData::GAMMA_3 or id == VolumeData::GAMMA_5 ) {
               deltaX = gridDescription.deltaI;
            }

            if ( m_interpolateFacePermeability ) {
               permeabilityNormal = permeabilityInterpolator.access ( element ).evaluate ( id, 0, lambda );
               permeabilityPlane  = permeabilityInterpolator.access ( element ).evaluate ( id, 1, lambda );
            } else {
               permeabilityNormal = elementPermeabilityN ( id );
               permeabilityPlane  = elementPermeabilityH ( id );
            }

            v3 = computeElementFaceFlux ( element,
                                          faceAreaInterpolator,
                                          id, whichPhase, 
                                          elementPressure,
                                          neighbourPressure,
                                          deltaX,
                                          phaseDensities ( phase ),
                                          phaseViscosities ( phase ),
                                          relativePermeability,
                                          finiteElement,
                                          permeabilityNormal,
                                          permeabilityPlane,
                                          lambda,
                                          false );

            elementFlux ( id ) = phaseMassDensity * v3 / phaseMolarMass;
         } else {
            elementFlux ( id ) = 0.0;
         }

      }

   } else {
      elementFlux.zero ();
   }

}

//------------------------------------------------------------//

void ExplicitMultiComponentFlowSolver::computeFluxTerms ( FormationSubdomainElementGrid&      formationGrid,
                                                          const Boolean3DArray&               elementTransportsHc,
                                                          const PhaseCompositionArray&        phaseComposition,
                                                          const PhaseValueArray&              phaseDensities,
                                                          const PhaseValueArray&              phaseViscosities,
                                                          const ElementFaceValueVector&       subdomainPermeabilitiesN,
                                                          const ElementFaceValueVector&       subdomainPermeabilitiesH,
                                                          const FacePermeabilityTemporalInterpolator& permeabilityInterpolator,
                                                          const ScalarPetscVector&            subdomainVapourPressure,
                                                          const ScalarPetscVector&            subdomainLiquidPressure,
                                                          const TemporalPropertyInterpolator& depth,
                                                          const TemporalPropertyInterpolator& porePressure,
                                                          const FaceAreaTemporalInterpolator& faceAreaInterpolator,
                                                          const PoreVolumeTemporalInterpolator& poreVolumeInterpolator,
                                                          const SaturationArray&              saturations,
                                                                ElementFaceValueVector&       gasFluxes,
                                                                ElementFaceValueVector&       oilFluxes,
                                                          const double                        lambda,
                                                                double&                       calculatedTimeStep ) {

   const double maximumHcSaturation = FastcauldronSimulator::getInstance ().getMaximumHcSaturation ();

   const ElementGrid& elementGrid = FastcauldronSimulator::getInstance ().getElementGrid ();

   LayerProps& theLayer = formationGrid.getFormation ();

   CompositionPetscVector concentrations;

   int i;
   int j;
   int k;
   int c;
   int face;

   ElementFaceValueVector layerFluxTerms;
   FiniteElementMethod::ElementGeometryMatrix geometryMatrix;
   FiniteElementMethod::FiniteElement finiteElement;
   FiniteElementMethod::ElementVector vapourSaturationCoefficients;
   FiniteElementMethod::ElementVector liquidSaturationCoefficients;

   PVTComponents  composition;
   PVTPhaseComponents phaseComponents;
   Saturation     elementSaturation;
   PVTPhaseValues phaseMolarConcentrations;

   double sumGasMolarMassRatio;
   double sumOilMolarMassRatio;
   double vapourMolarMass;
   double liquidMolarMass;
   double oilMassDensity;
   double gasMassDensity;
   double elementTimeStep;

   double vapourRelativePermeability;
   double liquidRelativePermeability;

   calculatedTimeStep = m_maximumTimeStepSize;

   concentrations.setVector ( theLayer.getVolumeGrid ( NumberOfPVTComponents ), theLayer.getPreviousComponentVec (), INSERT_VALUES );

   for ( i = elementGrid.firstI (); i <= elementGrid.lastI (); ++i ) {

      for ( j = elementGrid.firstJ (); j <= elementGrid.lastJ (); ++j ) {

         for ( k = formationGrid.firstK (); k <= formationGrid.lastK (); ++k ) {


            const SubdomainElement& element = formationGrid.getElement ( i, j, k );
            const LayerElement& layerElement = element.getLayerElement ();

            unsigned int elementK = element.getK ();

            ElementFaceValues& elementGasFlux = gasFluxes ( elementK, j, i );
            ElementFaceValues& elementOilFlux = oilFluxes ( elementK, j, i );

            elementGasFlux.zero ();
            elementOilFlux.zero ();

            if ( layerElement.isActive () and elementTransportsHc ( element.getI (), element.getJ (), element.getK ())) {

            // mol/m^3
            PVTComponents& elementConcentrations = concentrations ( k, j, i );

            elementSaturation = saturations ( element.getI (), element.getJ (), element.getK ());

            vapourRelativePermeability = element.getLayerElement ().getLithology ()->relativePermeability ( Saturation::VAPOUR, elementSaturation );
            liquidRelativePermeability = element.getLayerElement ().getLithology ()->relativePermeability ( Saturation::LIQUID, elementSaturation );

            if ( not m_interpolateFaceArea ) {
               getGeometryMatrix ( element.getLayerElement (), geometryMatrix, lambda );
               finiteElement.setGeometry ( geometryMatrix );
            }

            ElementFaceValues elementPermeabilityN;
            ElementFaceValues elementPermeabilityH;

            if ( not m_interpolateFacePermeability ) {
               elementPermeabilityN = subdomainPermeabilitiesN ( element.getK (), j, i );
               elementPermeabilityH = subdomainPermeabilitiesH ( element.getK (), j, i );
            }

            // Composition now in same units as phase-composition (mol/m^3)
            composition = elementConcentrations;

            // Units: mol/m^3
            phaseComponents = phaseComposition ( i, j, elementK );
            phaseComponents.sum ( phaseMolarConcentrations );

            if ( vapourRelativePermeability > 0.0 ) {
               gasMassDensity = phaseDensities ( i, j, elementK )( pvtFlash::VAPOUR_PHASE );

               if ( phaseMolarConcentrations ( pvtFlash::VAPOUR_PHASE ) != 0.0 ) {

                  for ( c = 0; c < NumberOfPVTComponents; ++c ) {
                     pvtFlash::ComponentId component = static_cast<pvtFlash::ComponentId>( c );

                     // fractions (phase-component-moles per total-phase-component-moles).
                     phaseComponents ( pvtFlash::VAPOUR_PHASE, component ) /= phaseMolarConcentrations ( pvtFlash::VAPOUR_PHASE );
                  }

               }

               vapourMolarMass = 0.0;

               for ( c = 0; c < NumberOfPVTComponents; ++c ) {
                  pvtFlash::ComponentId component = static_cast<pvtFlash::ComponentId>( c );

                  vapourMolarMass += phaseComponents ( pvtFlash::VAPOUR_PHASE, component ) * m_defaultMolarMasses ( component );
               }

               computeFluxForPhase ( pvtFlash::VAPOUR_PHASE,
                                     element,
                                     faceAreaInterpolator,
                                     finiteElement,
                                     phaseMolarConcentrations,
                                     subdomainVapourPressure,
                                     depth,
                                     porePressure,
                                     lambda,
                                     gasMassDensity,
                                     vapourMolarMass,
                                     phaseDensities ( i, j, elementK ),
                                     phaseViscosities ( i, j, elementK ),
                                     elementSaturation,
                                     vapourRelativePermeability,
                                     elementPermeabilityN,
                                     elementPermeabilityH,
                                     permeabilityInterpolator,
                                     elementGasFlux );
               
            }


            if ( liquidRelativePermeability > 0.0 ) {
               oilMassDensity = phaseDensities ( i, j, elementK )( pvtFlash::LIQUID_PHASE );

               if ( phaseMolarConcentrations ( pvtFlash::LIQUID_PHASE ) != 0.0 ) {

                  for ( c = 0; c < NumberOfPVTComponents; ++c ) {
                     pvtFlash::ComponentId component = static_cast<pvtFlash::ComponentId>( c );
                     
                     // fractions (phase-component-moles per total-phase-component-moles).
                     phaseComponents ( pvtFlash::LIQUID_PHASE, component ) /= phaseMolarConcentrations ( pvtFlash::LIQUID_PHASE );
                  }

               }


               liquidMolarMass = 0.0;

               for ( c = 0; c < NumberOfPVTComponents; ++c ) {
                  pvtFlash::ComponentId component = static_cast<pvtFlash::ComponentId>( c );

                     liquidMolarMass += phaseComponents ( pvtFlash::LIQUID_PHASE, component ) * m_defaultMolarMasses ( component );
               }



               computeFluxForPhase ( pvtFlash::LIQUID_PHASE,
                                     element,
                                        faceAreaInterpolator,
                                     finiteElement,
                                        phaseMolarConcentrations,
                                     subdomainLiquidPressure,
                                     depth,
                                     porePressure,
                                     lambda,
                                     oilMassDensity,
                                     liquidMolarMass,
                                     phaseDensities ( i, j, elementK ),
                                     phaseViscosities ( i, j, elementK ),
                                     elementSaturation,
                                        liquidRelativePermeability,
                                     elementPermeabilityN,
                                     elementPermeabilityH,
                                        permeabilityInterpolator,
                                     elementOilFlux );

               }

               // If any of <phase>-rel-perm > 0 then some transport will occur so the time-step size needs to be updated.
               if ( vapourRelativePermeability > 0.0 or liquidRelativePermeability > 0.0 ) {

                  // now determine time step.
               PVTComponents sumFluxes;
               double vapourFluxOut = elementGasFlux.sumGt0 ();
               double liquidFluxOut = elementOilFlux.sumGt0 ();

#if 0
               PVTPhaseValues fluxOut;

               fluxOut ( pvtFlash::LIQUID_PHASE ) = liquidFluxOut;
               fluxOut ( pvtFlash::VAPOUR_PHASE ) = vapourFluxOut;
#endif

               if ( vapourFluxOut == 0.0 and liquidFluxOut == 0.0 ) {
                  calculatedTimeStep = NumericFunctions::Minimum ( calculatedTimeStep, m_maximumTimeStepSize );
               } else {
                     double elementVolume;

                     if ( m_interpolatePoreVolume ) {
                        elementVolume = poreVolumeInterpolator.access ( element ).evaluate ( PoreVolumeIndex, lambda );
                     } else {
                        elementVolume = poreVolumeOfElement ( layerElement );
                     }

#if 0
                     sumFluxes = phaseComponents * fluxOut;
#endif

                  for ( c = 0; c < NumberOfPVTComponents; ++c ) {
                     pvtFlash::ComponentId component = static_cast<pvtFlash::ComponentId>( c );
                     // To be entirely correct we should multiply by the molar-masses here.
                     // But then in the loop below (computing the time-step size) we also have to scale by the molar-masses, they cancel out.
                     // So we do neither, to save on some flops.
                     sumFluxes ( component ) = phaseComponents ( pvtFlash::LIQUID_PHASE, component ) * liquidFluxOut + phaseComponents ( pvtFlash::VAPOUR_PHASE, component ) * vapourFluxOut;
                  }

                  for ( c = 0; c < NumberOfPVTComponents; ++c ) {
                     pvtFlash::ComponentId component = static_cast<pvtFlash::ComponentId>( c );

                     if ( sumFluxes ( component ) > 0.0 ) {
                           elementTimeStep = FastcauldronSimulator::getInstance ().getMcfHandler ().adaptiveTimeStepFraction () * elementVolume * composition ( component ) / sumFluxes ( component );
                        calculatedTimeStep = NumericFunctions::Minimum ( calculatedTimeStep, elementTimeStep  / MillionYearToSecond );
                     }

                  }

               }

            } else {
                  calculatedTimeStep = NumericFunctions::Minimum ( calculatedTimeStep, m_maximumTimeStepSize );
               }

            } // end if layerElement.isActive ()

         }

      }

   }

   concentrations.restoreVector ( UPDATE_EXCLUDING_GHOSTS );
}

//------------------------------------------------------------//

void ExplicitMultiComponentFlowSolver::computeFluxTerms ( Subdomain&                          subdomain,
                                                          const Boolean3DArray&               elementTransportsHc,
                                                          const PhaseCompositionArray&        phaseComposition,
                                                          const PhaseValueArray&              phaseDensities,
                                                          const PhaseValueArray&              phaseViscosities,
                                                          const ElementFaceValueVector&       subdomainPermeabilitiesN,
                                                          const ElementFaceValueVector&       subdomainPermeabilitiesH,
                                                          const FacePermeabilityTemporalInterpolator& permeabilityInterpolator,
                                                          const ScalarPetscVector&            vapourPressure,
                                                          const ScalarPetscVector&            liquidPressure,
                                                          const TemporalPropertyInterpolator& depth,
                                                          const TemporalPropertyInterpolator& porePressure,
                                                          const FaceAreaTemporalInterpolator& faceAreaInterpolator,
                                                          const PoreVolumeTemporalInterpolator& poreVolumeInterpolator,
                                                          const SaturationArray&              saturations,
                                                                ElementFaceValueVector&       gasFluxes,
                                                                ElementFaceValueVector&       oilFluxes,
                                                          const double                        lambda,
                                                                double&                       calculatedTimeStep ) {

   Subdomain::ActiveLayerIterator iter;

   subdomain.initialiseLayerIterator ( iter );

   calculatedTimeStep = m_maximumTimeStepSize;
   double subdomainCalculatedTimeStep;

   while ( not iter.isDone ()) {
      computeFluxTerms ( *iter, elementTransportsHc, phaseComposition,
                         phaseDensities, phaseViscosities,
                         subdomainPermeabilitiesN, subdomainPermeabilitiesH, permeabilityInterpolator,
                         vapourPressure, liquidPressure, depth, porePressure, faceAreaInterpolator, poreVolumeInterpolator, saturations,
                         gasFluxes, oilFluxes,
                         lambda, subdomainCalculatedTimeStep );
      calculatedTimeStep = NumericFunctions::Minimum ( subdomainCalculatedTimeStep, calculatedTimeStep );
      ++iter;
   }

   // Now find the minimum time-step from all processes.
   calculatedTimeStep = MpiFunctions::Minimum<double>( PETSC_COMM_WORLD, calculatedTimeStep );
}

//------------------------------------------------------------//

void ExplicitMultiComponentFlowSolver::scaleFluxTermsByTimeStep ( FormationSubdomainElementGrid& formationGrid,
                                                                  const Boolean3DArray&          elementContainsHc,
                                                                  ElementFaceValueVector&        vapourFluxes,
                                                                  ElementFaceValueVector&        liquidFluxes,
                                                                  const double                   deltaTSec ) {

   const ElementGrid& elementGrid = FastcauldronSimulator::getInstance ().getElementGrid ();

   int i;
   int j;
   int k;

   for ( i = elementGrid.firstI (); i <= elementGrid.lastI (); ++i ) {

      for ( j = elementGrid.firstJ (); j <= elementGrid.lastJ (); ++j ) {

         for ( k = formationGrid.firstK (); k <= formationGrid.lastK (); ++k ) {
            const SubdomainElement& element = formationGrid.getElement ( i, j, k );

            if ( elementContainsHc ( element.getI (), element.getJ (), element.getK ())) {
            unsigned int elementK = element.getK ();

            vapourFluxes ( elementK, j, i ) *= deltaTSec;
            liquidFluxes ( elementK, j, i ) *= deltaTSec;
         }

      }

   }

}

}

//------------------------------------------------------------//

void ExplicitMultiComponentFlowSolver::scaleFluxTermsByTimeStep ( Subdomain&              subdomain,
                                                                  const Boolean3DArray&   elementContainsHc,
                                                                  ElementFaceValueVector& vapourFluxes,
                                                                  ElementFaceValueVector& liquidFluxes,
                                                                  const double            deltaTSec ) {

   Subdomain::ActiveLayerIterator iter;

   subdomain.initialiseLayerIterator ( iter );

   // Now scale the initial flux-value by the time-step-size to get the amount of HC exiting the element across the boundary.
   while ( not iter.isDone ()) {
      scaleFluxTermsByTimeStep ( *iter, elementContainsHc, vapourFluxes, liquidFluxes, deltaTSec );
      ++iter;
   }

}

//------------------------------------------------------------//

void ExplicitMultiComponentFlowSolver::computeTemporalContributions ( const SubdomainElement&       element,
                                                                      const Boolean3DArray&         elementContainsHc,
                                                                      const CompositionPetscVector& layerConcentration,
                                                                      PVTComponents&                previousTerm,
                                                                      const PoreVolumeTemporalInterpolator& poreVolumeInterpolator,
                                                                      const double                  lambdaStart,
                                                                      const double                  lambdaEnd ) const {

   double poreVolume = 0.0;
   double porosityDerivativeTerm = 0.0;

   previousTerm.zero ();

               if ( element.getLayerElement ().isActive ()) {
      PVTComponents speciesDensity;

      double lambdaFraction = lambdaEnd - lambdaStart;

      const LayerElement& layerElement = element.getLayerElement ();



      if ( elementContainsHc ( element.getI (), element.getJ (), element.getK ())) {
         // Get concentration (mol/m^3).
         speciesDensity = layerConcentration ( layerElement.getLocalKPosition (), layerElement.getJPosition (), layerElement.getIPosition ());
         // if ( speciesDensity.sum () > HcConcentrationLowerLimit ) {

         // Multiply concentration by molar-mass, kg/m^3.
         speciesDensity *= m_defaultMolarMasses;

         if ( not m_interpolatePoreVolume ) {

            const CompoundLithology* lithology = element.getLayerElement ().getLithology ();
            const FluidType* fluid = element.getLayerElement ().getFormation ()->fluid;

      ElementGeometryMatrix geometryMatrix;
      ElementVector         previousVes;
      ElementVector         currentVes;

      ElementVector         previousMaxVes;
      ElementVector         currentMaxVes;
      FiniteElement         finiteElement;

      double weight;
      double currentPorosity;
            double currentPorosityDerivative;
      double previousPorosity;


         NumericFunctions::Quadrature3D::Iterator quad;
         NumericFunctions::Quadrature3D::getInstance ().get ( m_previousContributionsQuadratureDegree, quad );

         getGeometryMatrix ( element.getLayerElement (), geometryMatrix, lambdaStart );
         finiteElement.setGeometry ( geometryMatrix );

         interpolateCoefficients ( element.getLayerElement (), Basin_Modelling::VES_FP, previousVes, lambdaStart );
         interpolateCoefficients ( element.getLayerElement (), Basin_Modelling::VES_FP, currentVes,  lambdaEnd );

         interpolateCoefficients ( element.getLayerElement (), Basin_Modelling::Max_VES, previousMaxVes, lambdaStart );
         interpolateCoefficients ( element.getLayerElement (), Basin_Modelling::Max_VES, currentMaxVes,  lambdaEnd );

         poreVolume = 0.0;
         porosityDerivativeTerm = 0.0;

         for ( quad.initialise (); not quad.isDone (); ++quad ) {
            finiteElement.setQuadraturePoint ( quad.getX (), quad.getY (), quad.getZ ());

            // porosity at previous time-step.
            previousPorosity = lithology->porosity ( finiteElement.interpolate ( previousVes ),
                                                     finiteElement.interpolate ( previousMaxVes ),
                                                     false, 0.0 );

            // porosity at current time-step.
            currentPorosity = lithology->porosity ( finiteElement.interpolate ( currentVes ),
                                                    finiteElement.interpolate ( currentMaxVes ),
                                                    false, 0.0 );

               // currentPorosityDerivative = lithology->computePorosityDerivativeWrtVes ( finiteElement.interpolate ( currentVes ),
               //                                                                          finiteElement.interpolate ( currentMaxVes ),
               //                                                                          false, 0.0 );

            weight = determinant ( finiteElement.getJacobian ()) * quad.getWeight ();

            // Contrubution from previous time step.
            //
            //       DC
            //  phi ----
            //       Dt
            //
            poreVolume += weight * currentPorosity;

            // Contribution from "rock/mesh compression" term.
            //
               //    C    Dphi    d phi d ves
               //  -----  ---- =  ----- -----
               //  1-phi   Dt     d ves   dt
            //
               // porosityDerivativeTerm -= weight * currentPorosityDerivative * ( finiteElement.interpolate ( currentVes ) - finiteElement.interpolate ( previousVes )) / ( 1.0 - currentPorosity );
            porosityDerivativeTerm -= weight * ( currentPorosity - previousPorosity ) / ( 1.0 - currentPorosity );
            }

         } else {
            poreVolume = poreVolumeInterpolator.access ( element ).evaluate ( PoreVolumeIndex, lambdaEnd );
            porosityDerivativeTerm = poreVolumeInterpolator.access ( element ).evaluate ( RockCompressionIndex, lambdaEnd ) * lambdaFraction;
         }

         previousTerm = speciesDensity;
         previousTerm *= poreVolume + porosityDerivativeTerm;

      }

   }

}

//------------------------------------------------------------//

void ExplicitMultiComponentFlowSolver::computeTemporalContributions ( FormationSubdomainElementGrid& formationGrid,
                                                                      const Boolean3DArray&          elementContainsHc,
                                                                      const ElementVolumeGrid&       concentrationGrid,
                                                                      CompositionArray&              previousTerm,
                                                                      const PoreVolumeTemporalInterpolator& poreVolumeInterpolator,
                                                                      const double                   lambdaStart,
                                                                      const double                   lambda ) {

   LayerProps& layer = formationGrid.getFormation ();

   int i;
   int j;
   int k;

   CompositionPetscVector layerConcentration;

   layerConcentration.setVector ( concentrationGrid, layer.getPreviousComponentVec (), INSERT_VALUES );

   for ( i = concentrationGrid.firstI (); i <= concentrationGrid.lastI (); ++i ) {

      for ( j = concentrationGrid.firstJ (); j <= concentrationGrid.lastJ (); ++j ) {

         if ( FastcauldronSimulator::getInstance ().getMapElement ( i, j ).isValid ()) {

            for ( k = formationGrid.firstK (); k <= formationGrid.lastK (); ++k ) {
               SubdomainElement& element = formationGrid.getElement ( i, j, k );
               computeTemporalContributions ( element, elementContainsHc, layerConcentration, previousTerm ( i, j, element.getK ()), poreVolumeInterpolator, lambdaStart, lambda );
            }

         } else {

            for ( k = formationGrid.firstK (); k <= formationGrid.lastK (); ++k ) {
               previousTerm ( i, j, formationGrid.getElement ( i, j, k ).getK ()).zero ();
            }

         }

      }

   }

   layerConcentration.restoreVector ( NO_UPDATE );
}

//------------------------------------------------------------//

void ExplicitMultiComponentFlowSolver::computeTemporalContributions ( Subdomain&        subdomain,
                                                                      const Boolean3DArray&                 elementContainsHc,
                                                                      CompositionArray& previousTerm,
                                                                      const PoreVolumeTemporalInterpolator& poreVolumeInterpolator,
                                                                      const double      lambdaStart,
                                                                      const double      lambda ) {


   Subdomain::ActiveLayerIterator iter;

   subdomain.initialiseLayerIterator ( iter );

   while ( not iter.isDone ()) {
      computeTemporalContributions ( *iter, elementContainsHc, iter->getVolumeGrid ( NumberOfPVTComponents ), previousTerm, poreVolumeInterpolator, lambdaStart, lambda );
      ++iter;
   }


}

//------------------------------------------------------------//

double ExplicitMultiComponentFlowSolver::computeElementMassMatrix ( const SubdomainElement& element,
                                                                    const double            lambdaStart,
                                                                    const double            lambdaEnd ) const {

   const Lithology* lithology = element.getLayerElement ().getLithology ();

      FiniteElementMethod::ElementGeometryMatrix geometryMatrix;
   FiniteElementMethod::ElementVector         currentVes;
   FiniteElementMethod::ElementVector         currentMaxVes;
      FiniteElementMethod::FiniteElement finiteElement;

   double porosity;
   double massTerm = 0.0;

   NumericFunctions::Quadrature3D::Iterator quad;
   NumericFunctions::Quadrature3D::getInstance ().get ( m_previousContributionsQuadratureDegree, quad );

   getGeometryMatrix ( element.getLayerElement (), geometryMatrix, lambdaEnd );
      finiteElement.setGeometry ( geometryMatrix );

   interpolateCoefficients ( element.getLayerElement (), Basin_Modelling::VES_FP,  currentVes,    lambdaEnd );
   interpolateCoefficients ( element.getLayerElement (), Basin_Modelling::Max_VES, currentMaxVes, lambdaEnd );

      for ( quad.initialise (); not quad.isDone (); ++quad ) {
         finiteElement.setQuadraturePoint ( quad.getX (), quad.getY (), quad.getZ ());

      porosity = lithology->porosity ( finiteElement.interpolate ( currentVes ),
                                       finiteElement.interpolate ( currentMaxVes ),
                                       false, 0.0 );

      massTerm += determinant ( finiteElement.getJacobian ()) * quad.getWeight () * porosity;
      }

   return massTerm;
   }


//------------------------------------------------------------//

void ExplicitMultiComponentFlowSolver::divideByMassMatrix ( FormationSubdomainElementGrid& formationGrid,
                                                            const Boolean3DArray&          elementContainsHc,
                                                           const ElementVolumeGrid&       concentrationGrid,
                                                            const PoreVolumeTemporalInterpolator& poreVolumeInterpolator,
                                                            CompositionArray&              computedConcentrations,
                                                            const double                   lambdaStart,
                                                            const double                   lambdaEnd ) {

   const ElementGrid& elementGrid = FastcauldronSimulator::getInstance ().getElementGrid ();

   double massTerm;
   int i;
   int j;
   int k;

   for ( i = concentrationGrid.firstI (); i <= concentrationGrid.lastI (); ++i ) {

      for ( j = concentrationGrid.firstJ (); j <= concentrationGrid.lastJ (); ++j ) {

         if ( elementGrid.isPartOfStencil ( i, j )) {

            for ( k = formationGrid.firstK (); k <= formationGrid.lastK (); ++k ) {

               const SubdomainElement& element = formationGrid.getElement ( i, j, k );
               unsigned int elementK = element.getK ();

               if ( element.getLayerElement ().isActive () and elementContainsHc ( element.getI (), element.getJ (), element.getK ())) {

                  if ( m_interpolatePoreVolume ) {
                     massTerm = poreVolumeInterpolator.access ( element ).evaluate ( PoreVolumeIndex, lambdaEnd );
                  } else {
                     massTerm = computeElementMassMatrix ( element, lambdaStart, lambdaEnd );
                  }

                  computedConcentrations ( i, j, elementK ) *= 1.0 / massTerm;
               } else {
                  computedConcentrations ( i, j, elementK ).zero ();
               }

            }

         }

         }
      }
   }

//------------------------------------------------------------//
      
void ExplicitMultiComponentFlowSolver::divideByMassMatrix ( Subdomain&        subdomain,
                                                            const Boolean3DArray&                 elementContainsHc,
                                                            const PoreVolumeTemporalInterpolator& poreVolumeInterpolator,
                                                            CompositionArray& computedConcentrations,
                                                            const double      lambdaStart,
                                                            const double      lambdaEnd ) {

   Subdomain::ActiveLayerIterator iter;

   subdomain.initialiseLayerIterator ( iter );

   while ( not iter.isDone ()) {
      divideByMassMatrix ( *iter, elementContainsHc, iter->getVolumeGrid ( NumberOfPVTComponents ), poreVolumeInterpolator, computedConcentrations, lambdaStart, lambdaEnd );
      ++iter;
      }

}

//------------------------------------------------------------//

void ExplicitMultiComponentFlowSolver::updateTransportedMasses ( FormationSubdomainElementGrid& formationGrid, 
                                                                 const ScalarArray& transportedMasses ) {

   LayerProps& theLayer = formationGrid.getFormation ();

   PetscBlockVector<double>  layerTransportedMasses;

   layerTransportedMasses.setVector ( theLayer.getVolumeGrid ( 1 ), theLayer.getTransportedMassesVec (), INSERT_VALUES );

   int i;
   int j;
   int k;

   for ( i = formationGrid.firstI (); i <= formationGrid.lastI (); ++i ) {

      for ( j = formationGrid.firstJ (); j <= formationGrid.lastJ (); ++j ) {

         if ( FastcauldronSimulator::getInstance ().getMapElement ( i, j ).isValid ()) {

            for ( k = formationGrid.firstK (); k <= formationGrid.lastK (); ++k ) {

               const SubdomainElement& element = formationGrid.getElement ( i, j, k );
               unsigned int elementK = element.getK ();

               if ( element.getLayerElement ().isActive ()) {
                  layerTransportedMasses ( k, j, i ) = transportedMasses ( i, j, elementK );
               }

            }

         }

      }

   }

   layerTransportedMasses.restoreVector ( UPDATE_INCLUDING_GHOSTS );
}

//------------------------------------------------------------//

void ExplicitMultiComponentFlowSolver::updateTransportedMasses ( Subdomain& subdomain, 
                                                                 const ScalarArray& transportedMasses ) {

   Subdomain::ActiveLayerIterator iter;

   subdomain.initialiseLayerIterator ( iter );

   while ( not iter.isDone ()) {
      updateTransportedMasses ( *iter, transportedMasses );
      ++iter;
   }

}

//------------------------------------------------------------//

void ExplicitMultiComponentFlowSolver::activateProperties ( Subdomain&                  subdomain,
                                                            ConstrainedBooleanArrayMap& currentAlreadyActivatedProperties,
                                                            ConstrainedBooleanArrayMap& previousAlreadyActivatedProperties ) {


   Subdomain::ActiveLayerIterator iter;
   int p;
   ConstrainedBooleanArray activated;

   subdomain.initialiseLayerIterator ( iter );

   while ( not iter.isDone ()) {
      Basin_Modelling::Fundamental_Property_Manager& currentProperties  = iter->getFormation ().Current_Properties;
      Basin_Modelling::Fundamental_Property_Manager& previousProperties = iter->getFormation ().Previous_Properties;
      activated.fill ( false );

      for ( p = 0;  p < Basin_Modelling::NumberOfFundamentalProperties; ++p ) {
         Basin_Modelling::Fundamental_Property prop = Basin_Modelling::Fundamental_Property ( p );

         activated ( p ) = currentProperties.propertyIsActivated ( prop );

         if ( not activated ( p )) {
            currentProperties.Activate_Property ( prop, INSERT_VALUES, true );
         }

      }

      currentAlreadyActivatedProperties.push_back ( activated );
      activated.fill ( false );

      for ( p = 0;  p < Basin_Modelling::NumberOfFundamentalProperties; ++p ) {
         Basin_Modelling::Fundamental_Property prop = Basin_Modelling::Fundamental_Property ( p );

         activated ( p ) = previousProperties.propertyIsActivated ( prop );

         if ( not activated ( p )) {
            previousProperties.Activate_Property ( prop, INSERT_VALUES, true );
         }

      }

      previousAlreadyActivatedProperties.push_back ( activated );

      ++iter;
   }

}

//------------------------------------------------------------//

void ExplicitMultiComponentFlowSolver::deactivateProperties ( Subdomain&                        subdomain,
                                                              const ConstrainedBooleanArrayMap& currentAlreadyActivatedProperties,
                                                              const ConstrainedBooleanArrayMap& previousAlreadyActivatedProperties ) {


   Subdomain::ActiveLayerIterator iter;
   int p;
   int count = 0;
   ConstrainedBooleanArray activated;

   subdomain.initialiseLayerIterator ( iter );

   while ( not iter.isDone ()) {
      Basin_Modelling::Fundamental_Property_Manager& currentProperties = iter->getFormation ().Current_Properties;
      const ConstrainedBooleanArray& activated = currentAlreadyActivatedProperties [ count ];

      for ( p = 0;  p < Basin_Modelling::NumberOfFundamentalProperties; ++p ) {
         Basin_Modelling::Fundamental_Property prop = Basin_Modelling::Fundamental_Property ( p );

         if ( not activated ( p )) {
            currentProperties.Restore_Property ( prop );
         }

      }

      ++count;
      ++iter;
   }

   count = 0;
   subdomain.initialiseLayerIterator ( iter );

   while ( not iter.isDone ()) {
      Basin_Modelling::Fundamental_Property_Manager& previousProperties = iter->getFormation ().Previous_Properties;
      const ConstrainedBooleanArray& activated = previousAlreadyActivatedProperties [ count ];

      for ( p = 0;  p < Basin_Modelling::NumberOfFundamentalProperties; ++p ) {
         Basin_Modelling::Fundamental_Property prop = Basin_Modelling::Fundamental_Property ( p );

         if ( not activated ( p )) {
            previousProperties.Restore_Property ( prop );
         }

      }

      ++count;
      ++iter;
   }

}

//------------------------------------------------------------//

void ExplicitMultiComponentFlowSolver::collectGlobalSaturation ( FormationSubdomainElementGrid& formationGrid,
                                                                 SaturationPetscVector&         averagedSaturations,
                                                                 ScalarPetscVector&             divisor ) {

   LayerProps& theLayer = formationGrid.getFormation ();

   Saturation elementSaturation;

   int i;
   int j;
   int k;
   int l;

   SaturationPetscVector layerSaturations;
   layerSaturations.setVector ( theLayer.getVolumeGrid ( Saturation::NumberOfPhases ), theLayer.getPhaseSaturationVec (), INSERT_VALUES );

   for ( i = formationGrid.firstI (); i <= formationGrid.lastI (); ++i ) {

      for ( j = formationGrid.firstJ (); j <= formationGrid.lastJ (); ++j ) {

         if ( FastcauldronSimulator::getInstance ().getMapElement ( i, j ).isValid ()) {

            for ( k = formationGrid.firstK (); k <= formationGrid.lastK (); ++k ) {

               const SubdomainElement& element = formationGrid.getElement ( i, j, k );
               const LayerElement&     layerElement = element.getLayerElement ();

               if ( layerElement.isActive ()) {
                  // Get the element saturation
                  elementSaturation = layerSaturations ( layerElement.getLocalKPosition (), layerElement.getJPosition (), layerElement.getIPosition ());
                  double elementVolume = volumeOfElement ( layerElement, m_massMatrixQuadratureDegree );

                  elementSaturation *= elementVolume;

                  // Update the global vector
                  for ( l = 0; l < 8; ++l ) {
                     averagedSaturations ( element.getNodeK ( l ), element.getNodeJ ( l ), element.getNodeI ( l )) += elementSaturation;
                     divisor ( element.getNodeK ( l ), element.getNodeJ ( l ), element.getNodeI ( l )) += elementVolume;
                  }

               }

            }

         }

      }

   }

}

//------------------------------------------------------------//

void ExplicitMultiComponentFlowSolver::averageGlobalSaturation ( Subdomain&         subdomain,
                                                                 SaturationPetscVector& averagedSaturations,
                                                                 ScalarPetscVector& divisor ) {

   // Get the grid only for the number of elements in each direction of the subdomain.
   const NodalVolumeGrid& grid = subdomain.getNodeGrid ( 1 );

   int i;
   int j;
   int k;

   for ( i = grid.firstI (); i <= grid.lastI (); ++i ) {

      for ( j = grid.firstJ (); j <= grid.lastJ (); ++j ) {

         for ( k = grid.firstK (); k <= grid.lastK (); ++k ) {

            if ( divisor ( k, j, i ) != 0.0 ) {
               averagedSaturations ( k, j, i ) /= divisor ( k, j, i );
            } else {
               averagedSaturations ( k, j, i ).initialise ();
            }

         }

      }

   }

}

//------------------------------------------------------------//

void ExplicitMultiComponentFlowSolver::assignGlobalSaturation ( Subdomain&        subdomain,
                                                                SaturationPetscVector& averagedSaturations ) {

   const NodalVolumeGrid& grid = subdomain.getNodeGrid ( 1 );
   int i;
   int j;
   int k;

   Subdomain::ReverseActiveLayerIterator iter;

   subdomain.initialiseLayerIterator ( iter );

   // Initialise with the maximum number of nodes in vertical of subdomain.
   int* verticalDofNumbers = new int [ subdomain.maximumNumberOfNodes ()];
   int dofCount = 0;
   
   while ( not iter.isDone ()) {

      FormationSubdomainElementGrid& formationGrid = *iter;
      LayerProps& layer = formationGrid.getFormation ();

      VecZeroEntries ( layer.getAveragedSaturations ());

      SaturationPetscVector layerAveragedSaturations ( layer.getNodalVolumeGrid ( Saturation::NumberOfPhases ), layer.getAveragedSaturations (), INSERT_VALUES );

      // The +2 is not a mistake.
      // First 1 is added because: number = last - first + 1.
      // Second 1 is added because the function returns the elements first and last, we require the nodes.
      for ( i = 0; i < formationGrid.lastK () - formationGrid.firstK () + 2; ++i ) {
         verticalDofNumbers [ i ] = dofCount++;
      }

      --dofCount;

      for ( i = grid.firstI (); i <= grid.lastI (); ++i ) {

         for ( j = grid.firstJ (); j <= grid.lastJ (); ++j ) {

            // The +2 is not a mistake.
            // First 1 is added because: number = last - first + 1.
            // Second 1 is added because the function returns the elements first and last, we require the nodes.
            for ( k = 0; k < formationGrid.lastK () - formationGrid.firstK () + 2; ++k ) {
               layerAveragedSaturations ( k, j, i ) = averagedSaturations ( verticalDofNumbers [ k ], j, i );
            }

         }

      }

      layerAveragedSaturations.restoreVector ( UPDATE_EXCLUDING_GHOSTS );

      ++iter;
   }

   delete [] verticalDofNumbers;
}

//------------------------------------------------------------//

void ExplicitMultiComponentFlowSolver::averageGlobalSaturation ( Subdomain& subdomain ) {

   Subdomain::ActiveLayerIterator iter;

   NodalVolumeGrid& saturationGrid = subdomain.getNodeGrid ( Saturation::NumberOfPhases );
   NodalVolumeGrid& nodalGrid      = subdomain.getNodeGrid ( 1 );

   Vec averagedSaturationsVec;

   // The number of elements that contributed to the saturation at the node.
   Vec divisorVec;

   DMCreateGlobalVector ( saturationGrid.getDa (), &averagedSaturationsVec );
   DMCreateGlobalVector ( nodalGrid.getDa (), &divisorVec );

   VecZeroEntries ( averagedSaturationsVec );
   VecZeroEntries ( divisorVec );

   SaturationPetscVector averagedSaturations;
   ScalarPetscVector divisor;

   averagedSaturations.setVector ( saturationGrid, averagedSaturationsVec, INSERT_VALUES, true );
   divisor.setVector ( nodalGrid, divisorVec, INSERT_VALUES, true );

   subdomain.initialiseLayerIterator ( iter );

   while ( not iter.isDone ()) {
      collectGlobalSaturation ( *iter, averagedSaturations, divisor );
      ++iter;
   }

   // After collecting all values they must be distributed to neighbouring processors.
   averagedSaturations.restoreVector ( UPDATE_INCLUDING_GHOSTS );
   divisor.restoreVector ( UPDATE_INCLUDING_GHOSTS );

   // Next collect all value from neighbouring processors to this processor.
   // averagedSaturations.setVector ( saturationGrid, averagedSaturationsVec, ADD_VALUES, true );
   // divisor.Set_Global_Array ( nodalGrid.getDa (), divisorVec, ADD_VALUES, true );
   averagedSaturations.setVector ( saturationGrid, averagedSaturationsVec, INSERT_VALUES, true );
   divisor.setVector ( nodalGrid, divisorVec, INSERT_VALUES, true );

   averageGlobalSaturation ( subdomain, averagedSaturations, divisor );
   assignGlobalSaturation ( subdomain, averagedSaturations );

   averagedSaturations.restoreVector ( NO_UPDATE );
   divisor.restoreVector ( NO_UPDATE );

   VecDestroy ( & averagedSaturationsVec );
   VecDestroy ( & divisorVec );

}

//------------------------------------------------------------//

 void ExplicitMultiComponentFlowSolver::estimateSaturation ( const SubdomainElement& element,
                                                             const PVTComponents&    composition,
                                                                   bool&             elementContainsHc,
                                                                   double&           estimatedSaturation ) {

   double phaseMolarMass = 0.0;
   double phaseDensity = 200.0;
   
   elementContainsHc = composition.sum () > HcConcentrationLowerLimit;

   if ( elementContainsHc ) {

      for ( int c = 0; c < NumberOfPVTComponents; ++c ) {
         pvtFlash::ComponentId component = static_cast<pvtFlash::ComponentId>( c );
         phaseMolarMass += composition ( component ) * m_defaultMolarMasses ( component );
         }

      estimatedSaturation = phaseMolarMass / phaseDensity;
   } else {
      estimatedSaturation = 0.0;
      }

#if 0
   if ( elementContainsHc ) {
      cout << " estimated saturation " << estimatedSaturation << endl;
   }   
#endif

}

//------------------------------------------------------------//

void ExplicitMultiComponentFlowSolver::estimateHcTransport ( FormationSubdomainElementGrid& formationGrid,
                                                             Boolean3DArray&                elementContainsHc,
                                                             Boolean3DArray&                elementTransportsHc ) {

   const ElementGrid& elementGrid = FastcauldronSimulator::getInstance ().getElementGrid ();

   CompositionPetscVector concentrations;

   double estimatedSaturation;
   int i;
   int j;
   int k;

   LayerProps& theLayer = formationGrid.getFormation ();

   concentrations.setVector ( theLayer.getVolumeGrid ( NumberOfPVTComponents ), theLayer.getPreviousComponentVec (), INSERT_VALUES, true );

   for ( i = elementGrid.firstI ( true ); i <= elementGrid.lastI ( true ); ++i ) {

      for ( j = elementGrid.firstJ ( true ); j <= elementGrid.lastJ ( true ); ++j ) {

         for ( k = formationGrid.firstK (); k <= formationGrid.lastK (); ++k ) {

            const SubdomainElement& element = formationGrid.getElement ( i, j, k );
            const LayerElement& layerElement = element.getLayerElement ();

            unsigned int elementK = element.getK ();

            if ( layerElement.isActive ()) {

               if ( m_useSaturationEstimate ) {
                  estimateSaturation ( element,
                                       concentrations ( k, j, i ),
                                       elementContainsHc ( element.getI (), element.getJ (), element.getK ()),
                                       estimatedSaturation );

                  elementTransportsHc ( element.getI (), element.getJ (), element.getK ()) = estimatedSaturation > m_residualHcSaturationScaling * GeoPhysics::BrooksCorey::Sor;
               } else {
                  elementContainsHc   ( element.getI (), element.getJ (), element.getK ()) = concentrations ( k, j, i ).sum () > HcConcentrationLowerLimit;
                  elementTransportsHc ( element.getI (), element.getJ (), element.getK ()) = elementContainsHc ( element.getI (), element.getJ (), element.getK ());
               }

            }

         }

      }

   }

   concentrations.restoreVector ( NO_UPDATE );
}

//------------------------------------------------------------//

void ExplicitMultiComponentFlowSolver::estimateHcTransport ( Subdomain&    subdomain,
                                                             Boolean3DArray& elementContainsHc,
                                                             Boolean3DArray& elementTransportsHc ) {

   Subdomain::ActiveLayerIterator iter;

   subdomain.initialiseLayerIterator ( iter );

   while ( not iter.isDone ()) {
      estimateHcTransport ( *iter, elementContainsHc, elementTransportsHc );
      ++iter;
   }

}

//------------------------------------------------------------//

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

const double ExplicitMultiComponentFlowSolver::ConcentrationLowerLimit = 1.0e-20;

static const pvtFlash::PVTPhase RedundantPhase  = pvtFlash::VAPOUR_PHASE;
static const pvtFlash::PVTPhase DesignatedPhase = pvtFlash::LIQUID_PHASE;

#define OUTPUT ( i == FastcauldronSimulator::getInstance ().getMcfHandler ().getDebugINode () and j == FastcauldronSimulator::getInstance ().getMcfHandler ().getDebugJNode () and ( FastcauldronSimulator::getInstance ().getMcfHandler ().getDebugKNode () == - 1 or element.getK () == FastcauldronSimulator::getInstance ().getMcfHandler ().getDebugKNode ()))
#define OUTPUTNOK ( i == FastcauldronSimulator::getInstance ().getMcfHandler ().getDebugINode () and j == FastcauldronSimulator::getInstance ().getMcfHandler ().getDebugJNode ())

#define NEW_OTGC_INTERFACE

//------------------------------------------------------------//

ExplicitMultiComponentFlowSolver::ExplicitMultiComponentFlowSolver ()
{
   m_defaultMolarMasses = PVTCalc::getInstance ().getMolarMass ();
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
   m_totalTime = 0.0;


   m_maximumHCFractionForFlux = FastcauldronSimulator::getInstance ().getMcfHandler ().getMaximumHCFractionForFlux ();

   m_faceQuadratureDegree                  = FastcauldronSimulator::getInstance ().getMcfHandler ().getFaceQuadratureDegree ();
   m_previousContributionsQuadratureDegree = FastcauldronSimulator::getInstance ().getMcfHandler ().getPreviousTermQuadratureDegree ();
   m_sourceTermQuadratureDegree            = FastcauldronSimulator::getInstance ().getMcfHandler ().getSourceTermQuadratureDegree();
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


   char* otgc5Dir = getenv("OTGCDIR");
   char* myOtgc5Dir = getenv("MY_OTGCDIR");

   const char *OTGC5DIR = 0;

   if ( myOtgc5Dir != 0 ) {
      OTGC5DIR = myOtgc5Dir;
   } else {
      OTGC5DIR = otgc5Dir;
   }

   m_otgcSimulator = 0;

   if ( FastcauldronSimulator::getInstance ().getMcfHandler ().applyOtgc ()) {

      if ( OTGC5DIR != 0 ) {

         if ( FastcauldronSimulator::getInstance ().getMcfHandler ().modelContainsSulphur ()) {
            // H/C = 1.8, S/C = 0.035
            m_otgcSimulator = new Genex6::Simulator(OTGC5DIR, Constants::SIMOTGC, "TypeII_GX6", 1.8, 0.035 );
         } else {
            m_otgcSimulator = new Genex6::Simulator(OTGC5DIR, Constants::SIMOTGC | Constants::SIMOTGC5);
         }
         
      } else {
         //should throw instead...
         std::cout<<" MeSsAgE WARNING: OTGCDIR environment variable is not set. No OTGC functionality is available"<<std::endl;
      }

   }

#ifdef NEW_OTGC_INTERFACE
   ImmobileSpeciesValues::setMappingToSpeciesManager ( m_otgcSimulator->getSpeciesManager() );
#endif

   if ( FastcauldronSimulator::getInstance ().getMcfHandler ().getDebugLevel () > 0 ) {
      bool doGenex = FastcauldronSimulator::getInstance ().getCauldron ()->integrateGenexEquations ();
      bool doOtgc  = FastcauldronSimulator::getInstance ().getMcfHandler ().applyOtgc ();

      PetscPrintf ( PETSC_COMM_WORLD, " Multi-component flow command-line parameters (or equivalent default value): \n" );
      PetscPrintf ( PETSC_COMM_WORLD, "   -mcf            yes\n" );
      PetscPrintf ( PETSC_COMM_WORLD, "   -genex          %s\n", ( doGenex ? "yes" : "no" ));
      PetscPrintf ( PETSC_COMM_WORLD, "   -mcfotgc        %s\n", ( doOtgc  ? "yes" : "no" ));
      PetscPrintf ( PETSC_COMM_WORLD, "    otgc simulator %s\n", ( m_otgcSimulator == 0 ? "is null." : "is not null." ));
      // This one looks odd, because the command line parameter switches off the use of water-saturation in the overpressure calculation.
      // but the function determines whether it should be used.
      PetscPrintf ( PETSC_COMM_WORLD, "   -mcfstopsource  %s  %f\n", ( FastcauldronSimulator::getInstance ().getMcfHandler ().removeSourceTerm () ? "yes" : "no" ),
                    FastcauldronSimulator::getInstance ().getMcfHandler ().removeSourceTermAge ());

      PetscPrintf ( PETSC_COMM_WORLD, "   -mcfstoptrans   %s  %f\n", ( FastcauldronSimulator::getInstance ().getMcfHandler ().removeHcTransport () ? "yes" : "no" ),
                    FastcauldronSimulator::getInstance ().getMcfHandler ().removeHcTransportAge ());

      PetscPrintf ( PETSC_COMM_WORLD, "   -mcfnosatop     %s\n", ( FastcauldronSimulator::getInstance ().getMcfHandler ().includeWaterSaturationInOp () ? "no" : "yes" ));
      PetscPrintf ( PETSC_COMM_WORLD, "   -mcfclfts       %s\n", ( FastcauldronSimulator::getInstance ().getMcfHandler ().useCflTimeStepping () ? "yes" : "no" ));
      PetscPrintf ( PETSC_COMM_WORLD, "   -mcfcflfrac     %f \n", FastcauldronSimulator::getInstance ().getMcfHandler ().cflTimeStepFraction ());
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

      PetscPrintf ( PETSC_COMM_WORLD, "   -mcftssmooth    %s %e \n", 
                    ( FastcauldronSimulator::getInstance ().getMcfHandler ().applyTimeStepSmoothing () ? "yes" : "no" ),
                    FastcauldronSimulator::getInstance ().getMcfHandler ().getTimeStepSmoothingFactor ());

      PetscPrintf ( PETSC_COMM_WORLD, "   -mcfforms " );

      size_t i;
      const SubdomainArray& subdomains = FastcauldronSimulator::getInstance ().getMcfHandler ().getSubdomains ();

      for ( i = 0; i < subdomains.size (); ++i ) {
         PetscPrintf ( PETSC_COMM_WORLD, "\nSubdomain %i:\n%s\n\n", i + 1, subdomains [ i ]->image ().c_str ());
      }

   }

   // if ( FastcauldronSimulator::getInstance ().getMcfHandler ().getDebugLevel () > 0 and m_otgcSimulator != 0 ) {
   //    PetscPrintf ( PETSC_COMM_WORLD, " OTGC time-step size: %f \n", m_otgcSimulator->getTimeStepSize ());      
   // }

   // now scale the permeability maximum to SI units.
   m_fluxPermeabilityMaximum *= MILLIDARCYTOM2;

   m_maximumTimeStepSize = FastcauldronSimulator::getInstance ().getMcfHandler ().getMaximumTimeStepSize ();

}

//------------------------------------------------------------//

ExplicitMultiComponentFlowSolver::~ExplicitMultiComponentFlowSolver ()
{

   std::stringstream buffer;

   buffer << " Times for subdomain: " << FastcauldronSimulator::getInstance ().getRank ()  << std::endl;
   buffer << " Operation                cumulative time  " << std::endl;
   buffer << " OTGC time :          " << std::setw ( 20 ) << m_otgcTime << std::endl;
   buffer << " Flash time :         " << std::setw ( 20 ) << m_flashTime << std::endl;
   buffer << " Flux times :         " << std::setw ( 20 ) << m_fluxTime << std::endl;
   buffer << " Pressure times :     " << std::setw ( 20 ) << m_pressureTime << std::endl;
   buffer << " Permeability times : " << std::setw ( 20 ) << m_permeabilityTime << std::endl;
   buffer << " Previous time :      " << std::setw ( 20 ) << m_previousTime << std::endl;
   buffer << " Source time :        " << std::setw ( 20 ) << m_sourceTime << std::endl;
   buffer << " Transport time :     " << std::setw ( 20 ) << m_transportTime << std::endl;
   buffer << " Mass time :          " << std::setw ( 20 ) << m_massTime << std::endl;
   buffer << " Concentration time : " << std::setw ( 20 ) << m_concTime << std::endl;
   buffer << " Saturation time :    " << std::setw ( 20 ) << m_satTime << std::endl;
   buffer << " Total time :         " << std::setw ( 20 ) << m_totalTime << std::endl;

   PetscSynchronizedPrintf ( PETSC_COMM_WORLD, buffer.str ().c_str ());

}

//------------------------------------------------------------//

void ExplicitMultiComponentFlowSolver::solve ( Subdomain&   subdomain,
                                               const double startTime,
                                               const double endTime,
                                               DarcyErrorIndicator& errorOccurred ) {

   const MultiComponentFlowHandler& mcfHandler = FastcauldronSimulator::getInstance ().getMcfHandler ();

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

   // Holds the pressure at the centre of every element in the sub-domain.
   Vec                   subdomainLiquidPressureVec;
   Vec                   subdomainVapourPressureVec;


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

   DACreateGlobalVector ( fluxGrid.getDa (), &elementGasFluxTermsVec );
   elementGasFluxTerms.setVector ( fluxGrid, elementGasFluxTermsVec, INSERT_VALUES );

   DACreateGlobalVector ( fluxGrid.getDa (), &elementOilFluxTermsVec );
   elementOilFluxTerms.setVector ( fluxGrid, elementOilFluxTermsVec, INSERT_VALUES );

   DACreateGlobalVector ( permeabilityGrid.getDa (), &subdomainPermeabilityNVec );
   DACreateGlobalVector ( permeabilityGrid.getDa (), &subdomainPermeabilityHVec );

   DACreateGlobalVector ( simpleGrid.getDa (), &subdomainVapourPressureVec );
   DACreateGlobalVector ( simpleGrid.getDa (), &subdomainLiquidPressureVec );

   VecZeroEntries ( subdomainVapourPressureVec );
   VecZeroEntries ( subdomainLiquidPressureVec );

   ScalarPetscVector subdomainVapourPressure;// ( simpleGrid, subdomainVapourPressureVec, INSERT_VALUES, true );
   ScalarPetscVector subdomainLiquidPressure;// ( simpleGrid, subdomainLiquidPressureVec, INSERT_VALUES, true );

   computedConcentrations.create ( concentrationGrid );

   activateProperties ( subdomain, currentAlreadyActivatedProperties, previousAlreadyActivatedProperties );

   // The use of the concentration-grid here is okay, since only the dimensions
   // of the grid are used and not the number of dofs.
   // mol/m^3
   phaseComposition.create ( concentrationGrid );

   saturations.create ( concentrationGrid );

   // kg/m^3
   phaseDensities.create   ( concentrationGrid );

   // Viscosity units (kg/s/m? Pa.s?)
   phaseViscosities.create ( concentrationGrid );

   porePressure.setProperty ( Basin_Modelling::Pore_Pressure );
   temperature.setProperty ( Basin_Modelling::Temperature );
   ves.setProperty ( Basin_Modelling::VES_FP );
   maxVes.setProperty ( Basin_Modelling::Max_VES );
   depth.setProperty ( Basin_Modelling::Depth );

   if ( FastcauldronSimulator::getInstance ().getMcfHandler ().useCflTimeStepping ()) {
   // if ( subdomain.sourceRockIsActive () and FastcauldronSimulator::getInstance ().getMcfHandler ().useCflTimeStepping ()) {
   } else {

      if ( subdomain.sourceRockIsActive () and ( startTime - endTime ) > FastcauldronSimulator::getInstance ().getMcfHandler ().getMaximumTimeStepSize ()) {

         uniformTimeStepCount = int ( std::ceil (( startTime - endTime ) / FastcauldronSimulator::getInstance ().getMcfHandler ().getMaximumTimeStepSize ()));

         uniformFractionScaling = 1.0 / double ( uniformTimeStepCount );
         uniformDeltaTMa = ( startTime - endTime ) * uniformFractionScaling;
         uniformDeltaTSec = GeoPhysics::SecondsPerMillionYears * uniformDeltaTMa;
      } else {
         uniformTimeStepCount = 1;

         uniformFractionScaling = 1.0;
         uniformDeltaTMa = startTime - endTime;
         uniformDeltaTSec = GeoPhysics::SecondsPerMillionYears * uniformDeltaTMa;
      }

   }


   PVTPhaseComponents zeroPhaseComposition;
   PVTComponents zeroComposition;
   PVTPhaseValues zeroPhase;

   bool errorInSaturation;
   bool errorInConcentration;
   bool timeStepSubSamplePoint = false;
   bool timeSteppingFinished = false;
   double scaling = 1.0;

   double subSampledLambdaEnd;

   zeroPhaseComposition.zero ();
   zeroComposition.zero ();
   zeroPhase.zero ();

   // Copy saturation values from current to previous, before they are overwritten.
   subdomain.initialiseLayerIterator ( layerIter );

   while ( not layerIter.isDone ()) {
      layerIter->getFormation ().copySaturations ();
      ++layerIter;
   }

   timeStepStartMa = startTime;
   timeStepStartSec = startTime * GeoPhysics::SecondsPerMillionYears;
   lambdaStart = 0.0;

   while ( not timeSteppingFinished ) {
   // for ( t = 0; t < numberOfSubTimeSteps; ++t ) {
      ++iterationCount;
      iterationStart = WallTime::clock ();

      // Zero fluxes, ready for new valus.
      elementGasFluxTerms.restoreVector ( UPDATE_EXCLUDING_GHOSTS );
      elementOilFluxTerms.restoreVector ( UPDATE_EXCLUDING_GHOSTS );

      VecZeroEntries ( elementGasFluxTermsVec );
      VecZeroEntries ( elementOilFluxTermsVec );

      elementGasFluxTerms.setVector ( fluxGrid, elementGasFluxTermsVec, INSERT_VALUES, true );
      elementOilFluxTerms.setVector ( fluxGrid, elementOilFluxTermsVec, INSERT_VALUES, true );

      VecZeroEntries ( subdomainPermeabilityNVec );
      VecZeroEntries ( subdomainPermeabilityHVec );

      // phaseComposition.fill ( zeroPhaseComposition );
      // phaseDensities.fill ( zeroPhase );
      // phaseViscosities.fill ( zeroPhase );
      // computedConcentrations.fill ( zeroComposition );

      int ii, j, k;

      for ( ii = phaseComposition.firstI ( true ); ii <= phaseComposition.lastI ( true ); ++ii ) {
         
         for ( j = phaseComposition.firstJ ( true ); j <= phaseComposition.lastJ ( true ); ++j ) {

            for ( k = phaseComposition.firstK (); k <= phaseComposition.lastK (); ++k ) {
               phaseComposition ( ii, j, k ).zero ();
               phaseDensities ( ii, j, k ).zero ();
               phaseViscosities ( ii, j, k ).zero ();
               computedConcentrations ( ii, j, k ).zero ();
            }

         }

      }

      // Flash the (previous-) components for all the elements in the subdomain.
      flashStart = WallTime::clock ();
      flashComponents ( subdomain, phaseComposition, phaseDensities, phaseViscosities, porePressure, temperature, lambdaStart );
      flashIntervalTime += WallTime::clock () - flashStart;

      // now determine:
      //    i. flux
      //   ii. time-step size.
      setSaturations ( subdomain, phaseComposition, phaseDensities, saturations, errorInSaturation );

      if ( errorInSaturation ) {
         errorOccurred = ERROR_CALCULATING_SATURATION;
         break;
      }

      // Compute the flux term for the boundary of each element.
      subdomainVapourPressure.setVector ( simpleGrid, subdomainVapourPressureVec, INSERT_VALUES );
      subdomainLiquidPressure.setVector ( simpleGrid, subdomainLiquidPressureVec, INSERT_VALUES );

      pressureStart = WallTime::clock ();

      computePressure ( subdomain, phaseComposition, phaseDensities, saturations,
                        porePressure, temperature, ves, maxVes, lambdaStart,
                        subdomainVapourPressure, subdomainLiquidPressure );

      subdomainVapourPressure.restoreVector ( UPDATE_EXCLUDING_GHOSTS );
      subdomainLiquidPressure.restoreVector ( UPDATE_EXCLUDING_GHOSTS );

      // Update vectors from neighbouring processes.
      subdomainVapourPressure.setVector ( simpleGrid, subdomainVapourPressureVec, INSERT_VALUES, true );
      subdomainLiquidPressure.setVector ( simpleGrid, subdomainLiquidPressureVec, INSERT_VALUES, true );

      pressureIntervalTime += WallTime::clock () - pressureStart;

      // Compute the flux term for the boundary of each element.
      permeabilityStart = WallTime::clock ();

      // Average permeabilities.
      computeAveragePermeabilities ( subdomain, lambdaStart, subdomainPermeabilityNVec, subdomainPermeabilityHVec );

      // Update permeability values from neighbouring processes.
      subdomainPermeabilityN.setVector ( permeabilityGrid, subdomainPermeabilityNVec, INSERT_VALUES, true );
      subdomainPermeabilityH.setVector ( permeabilityGrid, subdomainPermeabilityHVec, INSERT_VALUES, true );

      permeabilityIntervalTime += WallTime::clock () - permeabilityStart;

      // Compute the flux term for the boundary of each element.
      fluxStart = WallTime::clock ();

      // Compute the rate at which hc leaves the element.
      // At the same time compute the maximu time-step size so as to remain a positive concentration.
      computeFluxTerms ( subdomain,
                         phaseComposition, phaseDensities, phaseViscosities, 
                         subdomainPermeabilityN, subdomainPermeabilityH,
                         subdomainVapourPressure,
                         subdomainLiquidPressure,
                         depth, porePressure, saturations, 
                         elementGasFluxTerms, elementOilFluxTerms,
                         lambdaStart, calculatedTimeStepSize );

      if ( mcfHandler.useCflTimeStepping ()) {

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

         deltaTSec = deltaTMa * GeoPhysics::SecondsPerMillionYears;
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

      scaleFluxTermsByTimeStep ( subdomain, elementGasFluxTerms, elementOilFluxTerms, deltaTSec );
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

      // Zero permeabilities, ready for new values.
      subdomainPermeabilityN.restoreVector ( UPDATE_EXCLUDING_GHOSTS );
      subdomainPermeabilityH.restoreVector ( UPDATE_EXCLUDING_GHOSTS );

      subdomainVapourPressure.restoreVector ( NO_UPDATE );
      subdomainLiquidPressure.restoreVector ( NO_UPDATE );

      // Compute the contributions from previous time-step.
      // This is first function in the sequence to add to the computed-concentrations.
      previousStart = WallTime::clock ();
      computeTemporalContributions ( subdomain, computedConcentrations, lambdaStart, lambdaEnd );
      previousIntervalTime += WallTime::clock () - previousStart;

      if ( not stopHcTransport or timeStepEndMa >= stopHcTransportAge ) {
         // Since the flux terms may not added here should they be calculated?
         // Now add the flux terms to the concentrations.
         transportStart = WallTime::clock ();
         transportComponents ( subdomain, phaseComposition, elementGasFluxTerms, elementOilFluxTerms, computedConcentrations );
         transportIntervalTime += WallTime::clock () - transportStart;
      }

      if ( not removeSourceTerm or timeStepEndMa >= sourceTermRemovalAge ) {
         // integrate over the element the hc generated in genex.
         sourceStart = WallTime::clock ();
         computeSourceTerm ( subdomain, computedConcentrations, lambdaStart, fractionScaling, sourceMassAdded );
         totalMassAddedForInterval += sourceMassAdded;
         sourceIntervalTime += WallTime::clock () - sourceStart;
      }

      // Divide the computed concentrations by the mass-matrix.
      massStart = WallTime::clock ();
      divideByMassMatrix ( subdomain, computedConcentrations, lambdaStart, lambdaEnd );
      massIntervalTime += WallTime::clock () - massStart;

      // Assign the concentration to each element.
      concStart = WallTime::clock ();
      setConcentrations ( subdomain, computedConcentrations, errorInConcentration );
      concIntervalTime += WallTime::clock () - concStart;

      if ( errorInConcentration ) {
         errorOccurred = ERROR_CALCULATING_CONCENTRATION;
         break;
      }

      // Apply OTGC to (previous-) components for all elements in the subdomain.
      otgcStart = WallTime::clock ();
      applyOtgc ( subdomain, porePressure, temperature, timeStepStartMa, timeStepEndMa, lambdaStart, lambdaEnd );
      otgcIntervalTime += WallTime::clock () - otgcStart;


      // Set start values to end values of last time-step.
      timeStepStartSec = timeStepEndSec;
      timeStepStartMa  = timeStepEndMa;
      lambdaStart = lambdaEnd;


#if 0
      if ( t == 0 and FastcauldronSimulator::getInstance ().getRank () == 0 ) { //and numberOfSubTimeSteps > 10 
         WallTime::Duration estimatedTimeStepTime = WallTime::clock () - iterationStart;
         int    hours;
         int    minutes;
         int    seconds;
         double fractionSeconds;

         estimatedTimeStepTime = estimatedTimeStepTime * numberOfSubTimeSteps;

         estimatedTimeStepTime.separate ( hours, minutes, seconds, fractionSeconds );

         if ( FastcauldronSimulator::getInstance ().getMcfHandler ().getSubdomains ().size () > 1 ) {
            cout << " - Estimated time for Darcy subdomain " << setw ( 2 ) << subdomain.getId () << ": " 
                 << setw ( 3 ) <<   hours << " hrs " 
                 << setw ( 2 ) << minutes << " mins "
                 << setw ( 2 ) << seconds << " secs"
                 << endl << flush;
         } else {
            cout << " - Estimated time for Darcy subdomain: " 
                 << setw ( 3 ) <<   hours << " hrs " 
                 << setw ( 2 ) << minutes << " mins "
                 << setw ( 2 ) << seconds << " secs"
                 << endl << flush;
         }

      }
#endif

   }


   if ( errorOccurred == NO_DARCY_ERROR ) {

      // Assign the saturations to each element.
      satStart = WallTime::clock ();
      setSaturations ( subdomain, errorInSaturation );

      if ( errorInSaturation ) {
         errorOccurred = ERROR_CALCULATING_SATURATION;
      } else {
         averageGlobalSaturation ( subdomain );
      }

      satIntervalTime = WallTime::clock () - satStart;

      //set time of element invasion
      setTimeOfElementInvasion(subdomain,endTime);
   }

   if ( errorOccurred != NO_DARCY_ERROR ) {
      PetscPrintf ( PETSC_COMM_WORLD, " Error in Darcy occurred at %f Ma\n", timeStepStartMa );
   }


   if ( FastcauldronSimulator::getInstance ().getMcfHandler ().getDebugLevel () > 0 ) {
      double totalMass = totalHcMass ( subdomain, 1.0 );

      PetscPrintf ( PETSC_COMM_WORLD, " Total mass in system           : %f \n", totalMass );
      PetscPrintf ( PETSC_COMM_WORLD, " Total mass added over interval : %f \n", totalMassAddedForInterval );
   }

   // 'Deactivate' all those properties that we not already 'activated'.
   deactivateProperties ( subdomain, currentAlreadyActivatedProperties, previousAlreadyActivatedProperties );

   VecDestroy ( subdomainPermeabilityNVec );
   VecDestroy ( subdomainPermeabilityHVec );
   VecDestroy ( subdomainVapourPressureVec );
   VecDestroy ( subdomainLiquidPressureVec );
   VecDestroy ( elementGasFluxTermsVec );
   VecDestroy ( elementOilFluxTermsVec );

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

   if ( FastcauldronSimulator::getInstance ().getMcfHandler ().getDebugLevel () > 1 ) {
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
      buffer << " Compute next saturation    " << std::setw ( 20 ) << satIntervalTime          << "  " << std::setw ( 20 ) << m_satTime          << std::endl;
      buffer << " Total time :               " << std::setw ( 20 ) << totalIntervalTime        << "  " << std::setw ( 20 ) << m_totalTime        << std::endl;

      PetscSynchronizedPrintf ( PETSC_COMM_WORLD, buffer.str ().c_str ());
      PetscSynchronizedFlush ( PETSC_COMM_WORLD );
   }

}

//------------------------------------------------------------//


void ExplicitMultiComponentFlowSolver::computePressure ( FormationSubdomainElementGrid&      formationGrid,
                                                         const PhaseCompositionArray&        phaseComposition,
                                                         const PhaseValueArray&              phaseDensities,
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

   // SaturationVector saturations;

   double elementPorePressure;
   double capillaryPressure;

   // saturations.setVector ( saturationGrid, theLayer.getPhaseSaturationVec (), INSERT_VALUES );

   for ( i = concentrationGrid.firstI (); i <= concentrationGrid.lastI (); ++i ) {

      for ( j = concentrationGrid.firstJ (); j <= concentrationGrid.lastJ (); ++j ) {

         if ( FastcauldronSimulator::getInstance ().getMapElement ( i, j ).isValid ()) {

            for ( k = formationGrid.lastK (); k >= formationGrid.firstK (); --k ) {
               SubdomainElement& element = formationGrid ( i, j, k );
               const LayerElement& layerElement = element.getLayerElement ();

               if ( layerElement.isActive ()) {

                  const Lithology* lithology = layerElement.getLithology ();

                  const Saturation& saturation = saturations ( element.getI (), element.getJ (), element.getK ());

#if 0
                  if ( isnan ( saturation ( Saturation::WATER )) or isinf ( saturation ( Saturation::WATER )) or 0.0 > saturation ( Saturation::WATER ) or saturation ( Saturation::WATER ) > 1.0 ) {
                     cout << " sat is not finite " << element.getI () << "  " << element.getJ () << "  " << element.getK () << "  " 
                          << saturation.image () 
                          << endl;
                  }
#endif

                  const PVTPhaseComponents& elementPhaseComposition = phaseComposition ( element.getI (), element.getJ (), element.getK ());

                  elementPorePressure = 1.0e6 * porePressure ( element, lambda );

                  // Compute liquid-pressure.
                  capillaryPressure = lithology->capillaryPressure ( Saturation::LIQUID, saturation );
                  liquidPressure ( element.getK (), element.getJ (), element.getI ()) = elementPorePressure + capillaryPressure;

                  // Compute vapour-pressure.
                  capillaryPressure = lithology->capillaryPressure ( Saturation::VAPOUR, saturation );
                  vapourPressure ( element.getK (), element.getJ (), element.getI ()) = elementPorePressure + capillaryPressure;
               }

            }

         }

      }

   }

}

//------------------------------------------------------------//

void ExplicitMultiComponentFlowSolver::computePressure ( Subdomain&                          subdomain,
                                                         const PhaseCompositionArray&        phaseComposition,
                                                         const PhaseValueArray&              phaseDensities,
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
      computePressure ( *iter, phaseComposition, phaseDensities, saturations, porePressure, temperature, ves, maxVes, lambda, vapourPressure, liquidPressure );
      ++iter;
   }

}

//------------------------------------------------------------//

void ExplicitMultiComponentFlowSolver::computeNumericalFlux ( const SubdomainElement& element,
                                                              const double            elementFlux,
                                                              const double            neighbourFlux,
                                                              const PVTComponents&    elementComposition,
                                                              const PVTComponents&    neighbourComposition,
                                                                    PVTComponents&    flux ) {

   PVTComponents ws;
   double        sum;

   //
   // Upwinding flux function.
   //
   //                    {   f(c+) . n x+ if f(c+) . n >= 0.0 outflow
   //  H ( c+, c-, n ) = {
   //                    {   f(c-) . n x- if f(c-) . n < 0.0  inflow
   //

   sum = elementComposition.sum ();

   // Subtract outflow
   if ( sum > ConcentrationLowerLimit ) {
      ws = elementComposition;
      ws *= 1.0 / sum;
      ws *= -NumericFunctions::Maximum ( 0.0, elementFlux );
      ws *= m_defaultMolarMasses;
      flux = ws;
   } else {
      flux.zero ();
   }

   sum = neighbourComposition.sum ();

   // Add in-flow from neighbour.
   if ( sum > ConcentrationLowerLimit ) {
      ws = neighbourComposition;
      ws *= 1.0 / sum;
      ws *= NumericFunctions::Maximum ( 0.0, neighbourFlux );
      ws *= m_defaultMolarMasses;

      flux += ws;
   }

}
                                                              

//------------------------------------------------------------//

void ExplicitMultiComponentFlowSolver::transportComponents ( const SubdomainElement&       element,
                                                             const pvtFlash::PVTPhase      phase,
                                                             const ElementFaceValueVector& elementFluxes,
                                                             const PhaseCompositionArray&  phaseComposition,
                                                                   PVTComponents&          computedConcentrations ) {

   PVTComponents elementComponents;
   PVTComponents neighbourComponents;
   PVTComponents transportedComponents;
   PVTComponents totalTransportedComponents;
   PVTComponents zeroComponents;

   int face;

   transportedComponents.zero ();
   zeroComponents.zero ();
   totalTransportedComponents.zero ();

   elementComponents = phaseComposition ( element.getI (), element.getJ (), element.getK ()).getPhaseComponents ( phase );

   for ( face = VolumeData::GAMMA_1; face <= VolumeData::GAMMA_6; ++face ) {

      const VolumeData::BoundaryId id = static_cast<VolumeData::BoundaryId>( face );
      const VolumeData::BoundaryId opposite = VolumeData::opposite ( id );
      const SubdomainElement* neighbour = element.getActiveNeighbour ( id );

      transportedComponents.zero ();

      if ( neighbour != 0 ) {
         neighbourComponents = phaseComposition ( neighbour->getI (), neighbour->getJ (), neighbour->getK ()).getPhaseComponents ( phase );

         computeNumericalFlux ( element,
                                elementFluxes ( element.getK (), element.getJ (), element.getI ())( id ),
                                elementFluxes ( neighbour->getK (), neighbour->getJ (), neighbour->getI ())( opposite ),
                                elementComponents,
                                neighbourComponents,
                                transportedComponents );

      } else {

         computeNumericalFlux ( element,
                                elementFluxes ( element.getK (), element.getJ (), element.getI ())( id ),
                                0.0,
                                elementComponents,
                                zeroComponents,
                                transportedComponents );

      // } else if ( faceIsBoundary ) {
      //    neighbourComponents = boundaryValues;
      //    computeNumericalFlux ( element,
      //                           oilFluxes ( element.getK (), element.getJ (), element.getI ()),
      //                           oilFluxes ( neighbour->getK (), neighbour->getJ (), neighbour->getI ()),
      //                           elementComponents,
      //                           neighbourComponents, transportedComponents );

      }

      computedConcentrations += transportedComponents;

#if 0
      if ( minimum ( computedConcentrations ) < 0.0 ) {
         cout << " conc less " << element.getI () << "  " << element.getJ () << "  " << element.getK () << "  " 
              << VolumeData::boundaryIdImage ( id ) << "  " << VolumeData::boundaryIdAliasImage ( id ) << "  " << ( phase == pvtFlash::VAPOUR_PHASE ? " VAPOUR " : " LIQUID " ) << endl
              << elementFluxes ( element.getK (), element.getJ (), element.getI ()).image () << endl
              << elementComponents.image () << endl
              << computedConcentrations.image () << endl
              << transportedComponents.image () << endl
              << endl;
      }
#endif

   }

}

//------------------------------------------------------------//

void ExplicitMultiComponentFlowSolver::transportComponents ( FormationSubdomainElementGrid&   formationGrid,
                                                             const ElementVolumeGrid&         concentrationGrid,
                                                             const PhaseCompositionArray&     phaseComposition,
                                                             const ElementFaceValueVector&    gasFluxes,
                                                             const ElementFaceValueVector&    oilFluxes,
                                                                   CompositionArray&          computedConcentrations ) {

   const ElementGrid& elementGrid = FastcauldronSimulator::getInstance ().getElementGrid ();

   int i;
   int j;
   int k;
   int face;

   for ( i = elementGrid.firstI (); i <= elementGrid.lastI (); ++i ) {

      for ( j = elementGrid.firstJ (); j <= elementGrid.lastJ (); ++j ) {

         for ( k = formationGrid.lastK (); k >= formationGrid.firstK (); --k ) {

            const SubdomainElement& element = formationGrid ( i, j, k );
            const LayerElement& layerElement = element.getLayerElement ();

            unsigned int elementK = element.getK ();

            if ( layerElement.isActive ()) {
               transportComponents ( element, pvtFlash::VAPOUR_PHASE, gasFluxes, phaseComposition, computedConcentrations ( i, j, elementK ));
               transportComponents ( element, pvtFlash::LIQUID_PHASE, oilFluxes, phaseComposition, computedConcentrations ( i, j, elementK ));
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
                                                             CompositionArray&                computedConcentrations ) {

   Subdomain::ActiveLayerIterator iter;

   subdomain.initialiseLayerIterator ( iter );

   while ( not iter.isDone ()) {
      transportComponents ( *iter, subdomain.getVolumeGrid ( NumberOfPVTComponents ), phaseComposition, gasFluxes, oilFluxes, computedConcentrations );
      ++iter;
   }

}

//------------------------------------------------------------//

double ExplicitMultiComponentFlowSolver::computeElementFaceFlux ( const SubdomainElement&                   element, 
                                                                  const VolumeData::BoundaryId              face,
                                                                  const Saturation::Phase                   phase,
                                                                  const double                              elementPressure,
                                                                  const double                              neighbourPressure,
                                                                  const double                              deltaX,
                                                                  const double                              phaseDensity,
                                                                  const double                              phaseViscosity,
                                                                  const Saturation&                         saturation,
                                                                        FiniteElementMethod::FiniteElement& finiteElement,
                                                                  const double                              permNormal,
                                                                  const double                              permPlane,
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
   double           relativePermeability = layerElement.getLithology ()->relativePermeability ( phase, saturation );
   double           weight;
   double           dsDt;

   double pressureGradient;
   double permeabilityValue;

   NumericFunctions::Quadrature3D::Iterator quad;
   NumericFunctions::Quadrature3D::getInstance ().get ( m_faceQuadratureDegree, face, quad );

   // Can move this block of code out of the loop.
   pressureGradient = 0.0;

   switch ( face ) {

     case VolumeData::ShallowFace : 
        pressureGradient = ((( elementPressure - neighbourPressure ) / deltaX - phaseDensity * GRAVITY ));
        permeabilityValue = permNormal;
        break;

     case VolumeData::DeepFace : 
        pressureGradient = -((( neighbourPressure - elementPressure ) / deltaX - phaseDensity * GRAVITY ));
        permeabilityValue = permNormal;
        break;

     case VolumeData::Front : 
        pressureGradient = -( neighbourPressure - elementPressure ) / deltaX;
        permeabilityValue = permPlane;
        break;

     case VolumeData::Back : 
        pressureGradient = -( neighbourPressure - elementPressure ) / deltaX;
        permeabilityValue = permPlane;
        break;

     case VolumeData::Left : 
        pressureGradient = -( neighbourPressure - elementPressure ) / deltaX;
        permeabilityValue = permPlane;
        break;

     case VolumeData::Right : 
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

   for ( quad.initialise (); not quad.isDone (); ++quad ) {
      finiteElement.setQuadraturePoint ( quad.getX (), quad.getY (), quad.getZ ());
      getElementBoundaryNormal ( layerElement, finiteElement.getJacobian (), face, normal, dsDt );
      weight = dsDt * quad.getWeight ();

      // The face-flux can be scaled by some of these values after the 
      // end of the loop, since they are invariant within the loop.
      faceFlux += weight * relativePermeability * permeabilityValue * pressureGradient / phaseViscosity;

      if ( print ) {
         Matrix3x3        permeability;

         // Evaluate permeability tensor.
         finiteElement.setTensor ( permNormal, permPlane, permeability );
         // Scale by relative-permeability.
         permeability *= relativePermeability;


         double fluxTemp =  relativePermeability * permeabilityValue * pressureGradient / phaseViscosity;

         cout << " element face fluxes: " 
              << std::setw ( 3 ) << element.getI () << "  " << std::setw ( 3 ) << element.getJ () << "  " << std::setw ( 3 ) << element.getK () << "  "
              << std::setw ( 5 ) << Saturation::PhaseImage ( phase ) << "  " << std::setw ( 12 ) << VolumeData::boundaryIdAliasImage ( face ) << "  "
              // << "( " << std::setw ( 15 ) << massFlux ( 1 ) << "  " << std::setw ( 15 ) << massFlux ( 2 ) << "  " << std::setw ( 15 ) << massFlux ( 3 ) << " )  " 
              << "( " << std::setw ( 15 ) << normal ( 1 ) << "  " << std::setw ( 15 ) << normal ( 2 ) << "  " << std::setw ( 15 ) << normal ( 3 ) << " )  " 
              << std::setw ( 15 ) << phaseViscosity << "  " << std::setw ( 15 ) << phaseDensity << "  " 
              << std::setw ( 15 ) << FiniteElementMethod::innerProduct ( normal, massFlux ) << "  " << std::setw ( 15 ) << dsDt << "  " 
              << std::setw ( 15 ) << permeability ( 1, 1 ) << "  " << std::setw ( 15 ) << permeability ( 2, 2 ) << "  " << std::setw ( 15 ) << permeability ( 3, 3 ) << "  " 
              << std::setw ( 15 ) << elementPressure << "  " << std::setw ( 15 ) << neighbourPressure << "  "
              << std::setw ( 15 ) << fluxTemp << "  "
              << std::setw ( 15 ) << pressureGradient << "  "
              << std::setw ( 15 ) << permeabilityValue << "  "
              << std::setw ( 15 ) << ( elementPressure - neighbourPressure ) / deltaX  << "  "
              << std::setw ( 15 ) << relativePermeability << "  " 
              << setw ( 15 ) << saturation.image () << "  "
              << setw ( 15 ) << saturation ( Saturation::WATER ) << "  "
              << std::endl;
      }

   }

   if ( print ) {
      cout << " computed face flux for face " << VolumeData::boundaryIdImage ( face ) << " = " << faceFlux << std::endl;
   }

   // Units: 
   return faceFlux;
}

//------------------------------------------------------------//

void ExplicitMultiComponentFlowSolver::computeFluxForPhase ( const pvtFlash::PVTPhase                  phase,
                                                             const SubdomainElement&                   element,
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
                                                             const ElementFaceValues&                  elementPermeabilityN,
                                                             const ElementFaceValues&                  elementPermeabilityH,
                                                                   ElementFaceValues&                  elementFlux ) {

   const CauldronGridDescription& gridDescription = FastcauldronSimulator::getInstance ().getCauldronGridDescription ();

   static const int NumberOfFluxFaces = 6;
   static const VolumeData::BoundaryId fluxFaces [ NumberOfFluxFaces ] = { VolumeData::GAMMA_2, VolumeData::GAMMA_3, VolumeData::GAMMA_4,
                                                                           VolumeData::GAMMA_5, VolumeData::GAMMA_1, VolumeData::GAMMA_6 };

   // const int NumberOfFluxFaces = 4;
   // VolumeData::BoundaryId fluxFaces [ NumberOfFluxFaces ] = { VolumeData::GAMMA_2, VolumeData::GAMMA_3, VolumeData::GAMMA_4, VolumeData::GAMMA_5 };

   // const int NumberOfFluxFaces = 2;
   // VolumeData::BoundaryId fluxFaces [ NumberOfFluxFaces ] = { VolumeData::GAMMA_1, VolumeData::GAMMA_6 };

   double elementPressure;
   double neighbourPressure;
   double deltaX = 100.0;
   int    face;

   if ( phases ( phase ) > ConcentrationLowerLimit ) {
   Saturation::Phase whichPhase = Saturation::convert ( phase );

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

            v3 = computeElementFaceFlux ( element, id, whichPhase, 
                                          elementPressure,
                                          neighbourPressure,
                                          deltaX,
                                          phaseDensities ( phase ),
                                          phaseViscosities ( phase ),
                                          elementSaturation,
                                          finiteElement,
                                          elementPermeabilityN ( id ),
                                          elementPermeabilityH ( id ),
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
                                                          const PhaseCompositionArray&        phaseComposition,
                                                          const PhaseValueArray&              phaseDensities,
                                                          const PhaseValueArray&              phaseViscosities,
                                                          const ElementFaceValueVector&       subdomainPermeabilitiesN,
                                                          const ElementFaceValueVector&       subdomainPermeabilitiesH,
                                                          const ScalarPetscVector&            subdomainVapourPressure,
                                                          const ScalarPetscVector&            subdomainLiquidPressure,
                                                          const TemporalPropertyInterpolator& depth,
                                                          const TemporalPropertyInterpolator& porePressure,
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
   PVTComponents  molarMasses;
   PVTComponents  temporary;
   PVTPhaseValues phases;
   PVTPhaseValues totalHc;
   PVTPhaseComponents phaseComponents;
   Saturation     elementSaturation;

   double elementVolume;
   double sumGasMolarMassRatio;
   double sumOilMolarMassRatio;
   double vapourMolarMass;
   double liquidMolarMass;
   double oilMassDensity;
   double gasMassDensity;
   double elementTimeStep;

   calculatedTimeStep = m_maximumTimeStepSize;

   concentrations.setVector ( theLayer.getVolumeGrid ( NumberOfPVTComponents ), theLayer.getPreviousComponentVec (), INSERT_VALUES, true );

   // // kg/mol
   // molarMasses *= 1.0e-3;
   molarMasses = m_defaultMolarMasses;

   for ( i = elementGrid.firstI (); i <= elementGrid.lastI (); ++i ) {

      for ( j = elementGrid.firstJ (); j <= elementGrid.lastJ (); ++j ) {

         for ( k = formationGrid.firstK (); k <= formationGrid.lastK (); ++k ) {


            const SubdomainElement& element = formationGrid ( i, j, k );
            const LayerElement& layerElement = element.getLayerElement ();

            unsigned int elementK = element.getK ();

            ElementFaceValues& elementGasFlux = gasFluxes ( elementK, j, i );
            ElementFaceValues& elementOilFlux = oilFluxes ( elementK, j, i );

            // mol/m^3
            PVTComponents& elementConcentrations = concentrations ( k, j, i );

            if ( layerElement.isActive ()) {

               const ElementFaceValues& elementPermeabilityN = subdomainPermeabilitiesN ( element.getK (), j, i );
               const ElementFaceValues& elementPermeabilityH = subdomainPermeabilitiesH ( element.getK (), j, i );

               elementGasFlux.zero ();
               elementOilFlux.zero ();

               getGeometryMatrix ( element.getLayerElement (), geometryMatrix, lambda );
               finiteElement.setGeometry ( geometryMatrix );

               elementVolume = poreVolumeOfElement ( element.getLayerElement (), geometryMatrix, m_massMatrixQuadratureDegree );
               // elementVolume = volumeOfElement ( element.getLayerElement (), geometryMatrix );

               // Composition now in same units as phase-composition (mol/m^3)
               composition = elementConcentrations;

               phaseComposition ( i, j, elementK ).sum ( phases );

               totalHc = phases;
               totalHc *= elementVolume;

               // // g/mol
               // molarMasses = PVTCalc::getInstance ().computeMolarMass ( composition );

               // fractions (phase-moles per total-phase-moles). INCORRECT COMMENT.
               // Units: mol/m^3
               phaseComponents = phaseComposition ( i, j, elementK );

               sumGasMolarMassRatio = 0.0;
               sumOilMolarMassRatio = 0.0;

               for ( c = 0; c < NumberOfPVTComponents; ++c ) {
                  pvtFlash::ComponentId component = static_cast<pvtFlash::ComponentId>( c );

                  // mol/m^3
                  sumGasMolarMassRatio += phaseComponents ( pvtFlash::VAPOUR_PHASE, component );
                  sumOilMolarMassRatio += phaseComponents ( pvtFlash::LIQUID_PHASE, component );
               }

               gasMassDensity = phaseDensities ( i, j, elementK )( pvtFlash::VAPOUR_PHASE );
               oilMassDensity = phaseDensities ( i, j, elementK )( pvtFlash::LIQUID_PHASE );

               if ( sumGasMolarMassRatio != 0.0 ) {

                  for ( c = 0; c < NumberOfPVTComponents; ++c ) {
                     pvtFlash::ComponentId component = static_cast<pvtFlash::ComponentId>( c );

                     // fractions (phase-component-moles per total-phase-component-moles).
                     phaseComponents ( pvtFlash::VAPOUR_PHASE, component ) /= sumGasMolarMassRatio;
                  }

               }

               if ( sumOilMolarMassRatio != 0.0 ) {

                  for ( c = 0; c < NumberOfPVTComponents; ++c ) {
                     pvtFlash::ComponentId component = static_cast<pvtFlash::ComponentId>( c );

                     // fractions (phase-component-moles per total-phase-component-moles).
                     phaseComponents ( pvtFlash::LIQUID_PHASE, component ) /= sumOilMolarMassRatio;
                  }

               }

               vapourMolarMass = 0.0;
               liquidMolarMass = 0.0;

               for ( c = 0; c < NumberOfPVTComponents; ++c ) {
                  pvtFlash::ComponentId component = static_cast<pvtFlash::ComponentId>( c );

                  // kg/mol
                  vapourMolarMass += phaseComponents ( pvtFlash::VAPOUR_PHASE, component ) * molarMasses ( component );
                  liquidMolarMass += phaseComponents ( pvtFlash::LIQUID_PHASE, component ) * molarMasses ( component );
               }

               elementSaturation = saturations ( element.getI (), element.getJ (), element.getK ());

               computeFluxForPhase ( pvtFlash::LIQUID_PHASE,
                                     element,
                                     finiteElement,
                                     phases,
                                     subdomainLiquidPressure,
                                     depth,
                                     porePressure,
                                     lambda,
                                     oilMassDensity,
                                     liquidMolarMass,
                                     phaseDensities ( i, j, elementK ),
                                     phaseViscosities ( i, j, elementK ),
                                     elementSaturation,
                                     elementPermeabilityN,
                                     elementPermeabilityH,
                                     elementOilFlux );

               computeFluxForPhase ( pvtFlash::VAPOUR_PHASE,
                                     element,
                                     finiteElement,
                                     phases,
                                     subdomainVapourPressure,
                                     depth,
                                     porePressure,
                                     lambda,
                                     gasMassDensity,
                                     vapourMolarMass,
                                     phaseDensities ( i, j, elementK ),
                                     phaseViscosities ( i, j, elementK ),
                                     elementSaturation,
                                     elementPermeabilityN,
                                     elementPermeabilityH,
                                     elementGasFlux );

               // now determine time step.
               PVTComponents sumFluxes;
               double vapourFluxOut = elementGasFlux.sumGt0 ();
               double liquidFluxOut = elementOilFlux.sumGt0 ();

               PVTPhaseValues fluxOut;

               fluxOut ( pvtFlash::LIQUID_PHASE ) = liquidFluxOut;
               fluxOut ( pvtFlash::VAPOUR_PHASE ) = vapourFluxOut;

               if ( vapourFluxOut == 0.0 and liquidFluxOut == 0.0 ) {
                  calculatedTimeStep = NumericFunctions::Minimum ( calculatedTimeStep, m_maximumTimeStepSize );
               } else {

                  // sumFluxes = phaseComponents * fluxOut;

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
                        elementTimeStep = FastcauldronSimulator::getInstance ().getMcfHandler ().cflTimeStepFraction () * elementVolume * composition ( component ) / sumFluxes ( component );
                        calculatedTimeStep = NumericFunctions::Minimum ( calculatedTimeStep, elementTimeStep  / GeoPhysics::SecondsPerMillionYears );
                     }

                  }

               }

            } else {

               if ( OUTPUT ) {
                  cout << " fluxes are zero for element " << element.getI () << "  " << element.getJ () << "  " << element.getK () << std::endl;
               }

               elementGasFlux.zero ();
               elementOilFlux.zero ();
            }

         }

      }

   }

   concentrations.restoreVector ( UPDATE_EXCLUDING_GHOSTS );
}

//------------------------------------------------------------//

void ExplicitMultiComponentFlowSolver::computeFluxTerms ( Subdomain&                          subdomain,
                                                          const PhaseCompositionArray&        phaseComposition,
                                                          const PhaseValueArray&              phaseDensities,
                                                          const PhaseValueArray&              phaseViscosities,
                                                          const ElementFaceValueVector&       subdomainPermeabilitiesN,
                                                          const ElementFaceValueVector&       subdomainPermeabilitiesH,
                                                          const ScalarPetscVector&            vapourPressure,
                                                          const ScalarPetscVector&            liquidPressure,
                                                          const TemporalPropertyInterpolator& depth,
                                                          const TemporalPropertyInterpolator& porePressure,
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
      computeFluxTerms ( *iter, phaseComposition,
                         phaseDensities, phaseViscosities,
                         subdomainPermeabilitiesN, subdomainPermeabilitiesH,
                         vapourPressure, liquidPressure, depth, porePressure, saturations,
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
            const SubdomainElement& element = formationGrid ( i, j, k );

            unsigned int elementK = element.getK ();

            vapourFluxes ( elementK, j, i ) *= deltaTSec;
            liquidFluxes ( elementK, j, i ) *= deltaTSec;
         }

      }

   }

}

//------------------------------------------------------------//

void ExplicitMultiComponentFlowSolver::scaleFluxTermsByTimeStep ( Subdomain&              subdomain,
                                                                  ElementFaceValueVector& vapourFluxes,
                                                                  ElementFaceValueVector& liquidFluxes,
                                                                  const double            deltaTSec ) {

   Subdomain::ActiveLayerIterator iter;

   subdomain.initialiseLayerIterator ( iter );

   // Now scale the initial flux-value by the time-step-size to get the amount of HC exiting the element across the boundary.
   while ( not iter.isDone ()) {
      scaleFluxTermsByTimeStep ( *iter, vapourFluxes, liquidFluxes, deltaTSec );
      ++iter;
   }

}

//------------------------------------------------------------//

void ExplicitMultiComponentFlowSolver::flashComponents ( FormationSubdomainElementGrid& formationGrid,
                                                         PhaseCompositionArray&         phaseComposition,
                                                         PhaseValueArray&               phaseDensities,
                                                         PhaseValueArray&               phaseViscosities,
                                                         const TemporalPropertyInterpolator& porePressure,
                                                         const TemporalPropertyInterpolator& temperature,
                                                         const double                   lambda ) {

   const ElementVolumeGrid& concentrationGrid = formationGrid.getVolumeGrid ( NumberOfPVTComponents );
   const ElementGrid& elementGrid = FastcauldronSimulator::getInstance ().getElementGrid ();

   LayerProps& theLayer = formationGrid.getFormation ();

   CompositionPetscVector concentrations;
   PVTComponents elementConcentrations;
   PVTComponents molarMasses;
   PVTPhaseValues phaseSum;

   // double temperature = 0.0;
   // double porePressure = 0.0;
   int i;
   int j;
   int k;

   molarMasses = m_defaultMolarMasses;

   // Should be previous concentrations.
   concentrations.setVector ( concentrationGrid, theLayer.getPreviousComponentVec (), INSERT_VALUES, true );

   for ( i = concentrationGrid.firstI ( true ); i <= concentrationGrid.lastI ( true ); ++i ) {

      for ( j = concentrationGrid.firstJ ( true ); j <= concentrationGrid.lastJ ( true ); ++j ) {

         if ( FastcauldronSimulator::getInstance ().getMapElement ( i, j ).isValid ()) {

         // What is the problem with isPartOfStencil?
         // if ( true or ( elementGrid.isPartOfStencil ( i, j ) and
         //                FastcauldronSimulator::getInstance ().getMapElement ( i, j ).isValid ())) {

            for ( k = formationGrid.firstK (); k <= formationGrid.lastK (); ++k ) {

               const SubdomainElement& element = formationGrid ( i, j, k );
               unsigned int elementK = element.getK ();

               if ( element.getLayerElement ().isActive ()) {

                  // mol/m^3.
                  elementConcentrations = concentrations ( k, j, i );

                  if ( elementConcentrations.sum () > ConcentrationLowerLimit ) {

                     // // g/mol
                     // molarMasses = PVTCalc::getInstance ().computeMolarMass ( elementConcentrations );

                     // Convert to kg/m^3
                     elementConcentrations *= molarMasses;

                     // Evluate both temperature and pore-pressure at the centre of the element.

                     PVTCalc::getInstance ().compute ( temperature ( element, lambda ) + 273.15,
                                                       NumericFunctions::Maximum ( 1.0e6 * porePressure ( element, lambda ), 1.0e5 ),
                                                       elementConcentrations,
                                                       phaseComposition ( element.getI (), element.getJ (), element.getK ()),
                                                       phaseDensities ( element.getI (), element.getJ (), element.getK ()),
                                                       phaseViscosities ( element.getI (), element.getJ (), element.getK ()));

                     // Convert to SI units.
                     phaseViscosities ( element.getI (), element.getJ (), element.getK ()) *= 0.001;

                     averageComponents ( elementConcentrations, 
                                         phaseComposition ( element.getI (), element.getJ (), element.getK ()),
                                         phaseDensities ( element.getI (), element.getJ (), element.getK ()),
                                         phaseViscosities ( element.getI (), element.getJ (), element.getK ()));

                     // Convert to mol/m^3
                     phaseComposition ( element.getI (), element.getJ (), element.getK ()) /= molarMasses;

                  } else {
                     phaseComposition ( element.getI (), element.getJ (), element.getK ()).zero ();
                     // The values assigned here are the same as the ones assigned 
                     // in PVT when no composition of a particular phase is present.
                     phaseDensities ( element.getI (), element.getJ (), element.getK ())( pvtFlash::LIQUID_PHASE ) = 1000.0;
                     phaseDensities ( element.getI (), element.getJ (), element.getK ())( pvtFlash::VAPOUR_PHASE ) = 1000.0;
                     phaseViscosities ( element.getI (), element.getJ (), element.getK ())( pvtFlash::LIQUID_PHASE ) = 1.0;
                     phaseViscosities ( element.getI (), element.getJ (), element.getK ())( pvtFlash::VAPOUR_PHASE ) = 1.0;
                  }

               }

            }

         } else {

            for ( k = formationGrid.firstK (); k <= formationGrid.lastK (); ++k ) {
               const SubdomainElement& element = formationGrid ( i, j, k );

               phaseComposition ( element.getI (), element.getJ (), element.getK ()).zero ();
               phaseDensities ( element.getI (), element.getJ (), element.getK ()).zero ();
               phaseViscosities ( element.getI (), element.getJ (), element.getK ()).zero ();
            }

         }

      }
      
   }

   concentrations.restoreVector ( NO_UPDATE );
}

//------------------------------------------------------------//

void ExplicitMultiComponentFlowSolver::flashComponents ( Subdomain&             subdomain,
                                                         PhaseCompositionArray& phaseComposition,
                                                         PhaseValueArray&       phaseDensities,
                                                         PhaseValueArray&       phaseViscosities,
                                                         const TemporalPropertyInterpolator& porePressure,
                                                         const TemporalPropertyInterpolator& temperature,
                                                         const double           lambda ) {


   Subdomain::ActiveLayerIterator iter;

   subdomain.initialiseLayerIterator ( iter );

   while ( not iter.isDone ()) {
      flashComponents ( *iter,
                        phaseComposition, phaseDensities, phaseViscosities,
                        porePressure, temperature,
                        lambda );
      ++iter;
   }

}

//------------------------------------------------------------//

void ExplicitMultiComponentFlowSolver::computeTemporalContributions ( const SubdomainElement&       element,
                                                                      const CompositionPetscVector& layerConcentration,
                                                                      PVTComponents&                previousTerm,
                                                                      const double                  lambdaStart,
                                                                      const double                  lambdaEnd,
                                                                      const bool                    print ) const {

   double bulkDensity = 0.0;
   double elementVolume = 0.0;
   double poreVolume = 0.0;
   double porosityDerivativeTerm = 0.0;
   double waterMass = 0.0;

   previousTerm.zero ();

   if ( element.getLayerElement ().isActive ()) {
      PVTComponents molarMasses;
      PVTComponents speciesDensity;

      ElementGeometryMatrix geometryMatrix;
      ElementVector         previousVes;
      ElementVector         currentVes;

      ElementVector         previousMaxVes;
      ElementVector         currentMaxVes;
      FiniteElement         finiteElement;

      double weight;
      double currentPorosity;
      double previousPorosity;

      const LayerElement& layerElement = element.getLayerElement ();

      const CompoundLithology* lithology = element.getLayerElement ().getLithology ();

      const FluidType* fluid = element.getLayerElement ().getFormation ()->fluid;

      // Get concentration (mol/m^3).
      speciesDensity = layerConcentration ( layerElement.getLocalKPosition (), layerElement.getJPosition (), layerElement.getIPosition ());

      if ( speciesDensity.sum () > ConcentrationLowerLimit ) {

         // // g/mol.
         // molarMasses = PVTCalc::getInstance ().computeMolarMass ( speciesDensity );

         // // kg/mol.
         // molarMasses *= 1.0e-3;
         molarMasses = m_defaultMolarMasses;

         // Multiply concentration by molar-mass, kg/m^3.
         speciesDensity *= molarMasses;

         NumericFunctions::Quadrature3D::Iterator quad;
         NumericFunctions::Quadrature3D::getInstance ().get ( m_previousContributionsQuadratureDegree, quad );

         getGeometryMatrix ( element.getLayerElement (), geometryMatrix, lambdaStart );
         finiteElement.setGeometry ( geometryMatrix );

         interpolateCoefficients ( element.getLayerElement (), Basin_Modelling::VES_FP, previousVes, lambdaStart );
         interpolateCoefficients ( element.getLayerElement (), Basin_Modelling::VES_FP, currentVes,  lambdaEnd );

         interpolateCoefficients ( element.getLayerElement (), Basin_Modelling::Max_VES, previousMaxVes, lambdaStart );
         interpolateCoefficients ( element.getLayerElement (), Basin_Modelling::Max_VES, currentMaxVes,  lambdaEnd );

         elementVolume = 0.0;
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

            weight = determinant ( finiteElement.getJacobian ()) * quad.getWeight ();

            elementVolume += weight;

            // Contrubution from previous time step.
            //
            //       DC
            //  phi ----
            //       Dt
            //
            poreVolume += weight * currentPorosity;

            // Contribution from "rock/mesh compression" term.
            //
            //    C    Dphi
            //  -----  ----
            //  1-phi   Dt
            //
            porosityDerivativeTerm -= weight * ( currentPorosity - previousPorosity ) / ( 1.0 - currentPorosity );

            bulkDensity += weight * ( currentPorosity * fluid->getConstantDensity () + ( 1.0 - currentPorosity ) * lithology->density ());
            waterMass += weight * currentPorosity * fluid->getConstantDensity ();
         }

         previousTerm = speciesDensity;
         previousTerm *= poreVolume + porosityDerivativeTerm;

      }

   }


   if ( print ) {
      // double molarMassWater = 18.0e-3; // kg/mol
      cout << " total mass in element: " 
           << setw ( 2 ) << element.getI () << "  "
           << setw ( 2 ) << element.getJ () << "  "
           << setw ( 2 ) << element.getK () << "  "
           << bulkDensity << "  " << previousTerm.sum () << " kg  " << elementVolume << "  " << poreVolume << "  " <<  poreVolume / elementVolume 
           << "  " << waterMass << "  " << waterMass / elementVolume << "  " << waterMass / poreVolume << std::endl
           << previousTerm.image () << endl << endl;
   }

}

//------------------------------------------------------------//

void ExplicitMultiComponentFlowSolver::computeTemporalContributions ( FormationSubdomainElementGrid& formationGrid,
                                                                      const ElementVolumeGrid&       concentrationGrid,
                                                                      CompositionArray&              previousTerm,
                                                                      const double                   lambdaStart,
                                                                      const double                   lambda ) {

   // const ElementVolumeGrid& concentrationGrid = theLayer.getVolumeGrid ( NumberOfPVTComponents );

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
               SubdomainElement& element = formationGrid ( i, j, k );
               computeTemporalContributions ( element, layerConcentration, previousTerm ( i, j, element.getK ()), lambdaStart, lambda, OUTPUT );

               if ( OUTPUT ) {
                  cout << " previous:  " << previousTerm ( i, j, element.getK ()).image () << std::endl
                       << "            " << layerConcentration ( k, j, i ).image () << std::endl;
               }

            }

         } else {

            for ( k = formationGrid.firstK (); k <= formationGrid.lastK (); ++k ) {
               previousTerm ( i, j, formationGrid ( i, j, k ).getK ()).zero ();
            }

         }

      }

   }

   layerConcentration.restoreVector ( NO_UPDATE );
}

//------------------------------------------------------------//

void ExplicitMultiComponentFlowSolver::computeTemporalContributions ( Subdomain&        subdomain,
                                                                      CompositionArray& previousTerm,
                                                                      const double      lambdaStart,
                                                                      const double      lambda ) {


   Subdomain::ActiveLayerIterator iter;

   subdomain.initialiseLayerIterator ( iter );

   while ( not iter.isDone ()) {
      computeTemporalContributions ( *iter, iter->getVolumeGrid ( NumberOfPVTComponents ), previousTerm, lambdaStart, lambda );
      ++iter;
   }


}

//------------------------------------------------------------//

void ExplicitMultiComponentFlowSolver::computeSourceTerm ( const SubdomainElement& element,
                                                                 PVTComponents&    sourceTerm,
                                                           const double            lambda,
                                                           const double            fractionScaling,
                                                                 double&           elementMassAdded,
                                                           const bool              print ) const {

   elementMassAdded = 0.0;

   if ( element.getLayerElement ().isActive ()) {
      MultiComponentVector<PVTComponents> generated;
      FiniteElementMethod::ElementGeometryMatrix geometryMatrix;
      FiniteElementMethod::FiniteElement finiteElement;
      PVTComponents term;
      PVTComponents computedSourceTerm;
      NumericFunctions::Quadrature3D::Iterator quad;
      NumericFunctions::Quadrature3D::getInstance ().get ( m_sourceTermQuadratureDegree, quad );
      double elementVolume = 0.0;
      double weight;
      double layerThickness;
      int i;

      const LayerProps* srLayer = element.getLayerElement ().getFormation ();

      getGeometryMatrix ( element.getLayerElement (), geometryMatrix, lambda );

      finiteElement.setGeometry ( geometryMatrix );

      for ( i = 1; i <= 4; ++i ) {
         srLayer->getGenexGenerated ( element.getLayerElement ().getNodeIPosition ( i - 1 ),
                                      element.getLayerElement ().getNodeJPosition ( i - 1 ),
                                      generated ( i ));

         generated ( i )( pvtFlash::COX ) = 0.0;
         generated ( i )( pvtFlash::H2S ) = 0.0;

         layerThickness = srLayer->getCurrentLayerThickness ( element.getLayerElement ().getNodeIPosition ( i - 1 ),
                                                              element.getLayerElement ().getNodeJPosition ( i - 1 ));

         if ( layerThickness > DepositingThicknessTolerance ) {
            generated ( i ) *= 1.0 / layerThickness;
         } else {
            generated ( i ).zero ();
         }

         generated ( i + 4 ) = generated ( i );
      }

      computedSourceTerm.zero ();

      for ( quad.initialise (); not quad.isDone (); ++quad ) {
         finiteElement.setQuadraturePoint ( quad.getX (), quad.getY (), quad.getZ ());

         weight = determinant ( finiteElement.getJacobian ()) * quad.getWeight ();

         term = generated.dot ( finiteElement.getBasis ());
         term *= fractionScaling * weight;

         computedSourceTerm += term;
         elementVolume += weight;
      }

      elementMassAdded = computedSourceTerm.sum ();

      // Units are kg.
      sourceTerm += computedSourceTerm;
   }

}

//------------------------------------------------------------//

void ExplicitMultiComponentFlowSolver::computeSourceTerm ( FormationSubdomainElementGrid& formationGrid,
                                                           const ElementVolumeGrid&       concentrationGrid,
                                                           const double                   lambda,
                                                           const double                   fractionScaling,
                                                           CompositionArray&              sourceTerm,
                                                                 double&                  layerMassAdded ) {

   LayerProps& srLayer = formationGrid.getFormation ();

   int i;
   int j;
   int k;
   bool depthRetrieved = srLayer.Current_Properties.propertyIsActivated ( Basin_Modelling::Depth );
   bool genexRetrieved = srLayer.genexDataIsRetrieved ();

   double elementMassAdded;

   layerMassAdded = 0.0;

   if ( not depthRetrieved ) {
      srLayer.Current_Properties.Activate_Property ( Basin_Modelling::Depth, INSERT_VALUES, true );
   }

   if ( not genexRetrieved ) {
      srLayer.retrieveGenexData ();
   }

   for ( i = concentrationGrid.firstI (); i <= concentrationGrid.lastI (); ++i ) {

      for ( j = concentrationGrid.firstJ (); j <= concentrationGrid.lastJ (); ++j ) {

         if ( FastcauldronSimulator::getInstance ().getMapElement ( i, j ).isValid ()) {

            for ( k = formationGrid.firstK (); k <= formationGrid.lastK (); ++k ) {
               SubdomainElement& element = formationGrid ( i, j, k );

               computeSourceTerm ( element, sourceTerm ( i, j, element.getK ()), lambda, fractionScaling, elementMassAdded, OUTPUT );

               layerMassAdded += elementMassAdded;

               if ( OUTPUT ) {
                  cout << " genex:     " << sourceTerm ( i, j, element.getK ()).image () << std::endl;
               }

            }

         } else {

            for ( k = formationGrid.firstK (); k <= formationGrid.lastK (); ++k ) {
               sourceTerm ( i, j, formationGrid ( i, j, k ).getK ()).zero ();
            }

         }

      }

   }

   if ( not genexRetrieved ) {
      // Restore back to original state.
      srLayer.restoreGenexData ();
   }

   if ( not depthRetrieved ) {
      // Restore back to original state.
      srLayer.Current_Properties.Restore_Property ( Basin_Modelling::Depth );
   }

}

//------------------------------------------------------------//

void ExplicitMultiComponentFlowSolver::computeSourceTerm ( Subdomain&        subdomain,
                                                           CompositionArray& sourceTerm,
                                                           const double      lambda,
                                                           const double      fractionScaling,
                                                                 double&     massAdded ) {

   Subdomain::ActiveSourceRockLayerIterator iter;

   subdomain.initialiseLayerIterator ( iter );

   double processorMassAdded = 0.0;
   double layerMassAdded = 0.0;

   while ( not iter.isDone ()) {
      computeSourceTerm ( *iter, iter->getVolumeGrid ( NumberOfPVTComponents ), lambda, fractionScaling, sourceTerm, layerMassAdded );
      ++iter;
      processorMassAdded += layerMassAdded;
   }

   massAdded = MpiFunctions::Sum<double>( PETSC_COMM_WORLD, processorMassAdded );
}

//------------------------------------------------------------//

double ExplicitMultiComponentFlowSolver::computeElementMassMatrix ( const SubdomainElement& element,
                                                                    const double            lambdaStart,
                                                                    const double            lambdaEnd ) const {

   const Lithology* lithology = element.getLayerElement ().getLithology ();

   FiniteElementMethod::ElementGeometryMatrix geometryMatrix;
   FiniteElementMethod::ElementVector         currentVes;
   FiniteElementMethod::ElementVector         currentMaxVes;
   FiniteElementMethod::FiniteElement         finiteElement;

   double porosity;
   double massTerm = 0.0;

   NumericFunctions::Quadrature3D::Iterator quad;

#if 0
   NumericFunctions::Quadrature3D::getInstance ().get ( m_previousContributionsQuadratureDegree, quad );
#endif
   NumericFunctions::Quadrature3D::getInstance ().get ( 2, 2, m_massMatrixQuadratureDegree, quad );

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
                                                            const ElementVolumeGrid&       concentrationGrid,
                                                            CompositionArray&              computedConcentrations,
                                                            const double                   lambdaStart,
                                                            const double                   lambdaEnd ) {

   const ElementGrid& elementGrid = FastcauldronSimulator::getInstance ().getElementGrid ();

   PVTComponents elementConcentration;
   double massTerm;
   int i;
   int j;
   int k;

   for ( i = concentrationGrid.firstI (); i <= concentrationGrid.lastI (); ++i ) {

      for ( j = concentrationGrid.firstJ (); j <= concentrationGrid.lastJ (); ++j ) {

         if ( elementGrid.isPartOfStencil ( i, j )) {

            for ( k = formationGrid.firstK (); k <= formationGrid.lastK (); ++k ) {

               const SubdomainElement& element = formationGrid ( i, j, k );
               unsigned int elementK = element.getK ();

               if ( element.getLayerElement ().isActive ()) {
                  massTerm = computeElementMassMatrix ( element, lambdaStart, lambdaEnd );

                  computedConcentrations ( i, j, elementK ) *= 1.0 / massTerm;

                  if ( OUTPUT ) {
                     cout << " mass matrix " << massTerm << std::endl;
                  }

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
                                                            CompositionArray& computedConcentrations,
                                                            const double      lambdaStart,
                                                            const double      lambdaEnd ) {

   Subdomain::ActiveLayerIterator iter;

   subdomain.initialiseLayerIterator ( iter );

   while ( not iter.isDone ()) {
      divideByMassMatrix ( *iter, iter->getVolumeGrid ( NumberOfPVTComponents ), computedConcentrations, lambdaStart, lambdaEnd );
      ++iter;
   }

}

//------------------------------------------------------------//

void ExplicitMultiComponentFlowSolver::setConcentrations ( FormationSubdomainElementGrid& formationGrid,
                                                           const CompositionArray&        computedConcentrations,
                                                           bool&                          errorInConcentration ) {

   LayerProps& theLayer = formationGrid.getFormation ();

   PVTComponents          elementConcentration;
   CompositionPetscVector concentrations;

   concentrations.setVector ( theLayer.getVolumeGrid ( NumberOfPVTComponents ), theLayer.getPreviousComponentVec (), INSERT_VALUES );

   PVTComponents molarMasses;
   PVTComponents sum1;

   int i;
   int j;
   int k;
   bool allAreValid = true;

   errorInConcentration = false;

   for ( i = formationGrid.firstI (); i <= formationGrid.lastI (); ++i ) {

      for ( j = formationGrid.firstJ (); j <= formationGrid.lastJ (); ++j ) {
         sum1.zero ();

         if ( FastcauldronSimulator::getInstance ().getMapElement ( i, j ).isValid ()) {

            for ( k = formationGrid.firstK (); k <= formationGrid.lastK (); ++k ) {

               const SubdomainElement& element = formationGrid ( i, j, k );
               unsigned int elementK = element.getK ();

               if ( element.getLayerElement ().isActive ()) {

                  elementConcentration = computedConcentrations ( i, j, elementK );

                  if ( minimum ( elementConcentration ) < 0.0 or not elementConcentration.isFinite ()) {

                     std::stringstream buffer;

                     buffer << " Location of error: " << theLayer.getName () << "  "
                            << " layer position = { " << element.getI () << ", " << element.getJ () << ", " << element.getLayerElement ().getLocalKPosition () << "} " 
                            << " subdomain position = { " << element.getI () << ", " << element.getJ () << ", " << element.getK () << "} " 
                            << endl;

                     cout << buffer.str () << flush;

                     allAreValid = false;
                  }

                  // if ( elementConcentration.sum () > ConcentrationLowerLimit ) {

                  // // g/mol
                  // molarMasses = PVTCalc::getInstance ().computeMolarMass ( elementConcentration );

                  // // kg/mol
                  // molarMasses *= 1.0e-3;

                  // Convert to mol/m^3
                  elementConcentration /= m_defaultMolarMasses;

                  // Should we iterate here?
                  // 1. Re-compute the molar masses based on the predicted concentrations.
                  // 2. Re-compute the element-concentrations based on the new molar-masses.
                  // 3. Repeat until convergence (or a fixed (1) number of times).

                  // molarMasses = PVTCalc::getInstance ().computeMolarMass ( elementConcentration );
                  // molarMasses *= 1.0e-3;
                  // elementConcentration = computedConcentrations ( i, j, k );
                  // elementConcentration /= molarMasses;
                     
                  // molarMasses = PVTCalc::getInstance ().computeMolarMass ( elementConcentration );
                  // molarMasses *= 1.0e-3;
                  // elementConcentration = computedConcentrations ( i, j, k );
                  // elementConcentration /= molarMasses;
                     
                  concentrations ( k, j, i ) = elementConcentration;

               } else {
                  concentrations ( k, j, i ).zero ();
               }

            }

         } else {

            for ( k = formationGrid.firstK (); k <= formationGrid.lastK (); ++k ) {
               concentrations ( k, j, i ).zero ();
            }

         }

         if ( OUTPUTNOK  ) {
            cout << " total mass sum: " << sum1.image () << std::endl;
         }


      }

   }

   concentrations.restoreVector ( UPDATE_EXCLUDING_GHOSTS );
   errorInConcentration = not successfulExecution ( allAreValid );
}

//------------------------------------------------------------//

void ExplicitMultiComponentFlowSolver::setConcentrations ( Subdomain&              subdomain,
                                                           const CompositionArray& computedConcentrations,
                                                           bool&                   errorInConcentration ) {

   Subdomain::ActiveLayerIterator iter;
   bool errorInLayerConcentration;

   subdomain.initialiseLayerIterator ( iter );
   errorInConcentration = false;

   while ( not iter.isDone ()) {
      setConcentrations ( *iter, computedConcentrations, errorInLayerConcentration );
      errorInConcentration = errorInConcentration or errorInLayerConcentration;
      ++iter;
   }

}

//------------------------------------------------------------//

void ExplicitMultiComponentFlowSolver::averageComponents ( PVTComponents&      masses,
                                                           PVTPhaseComponents& phaseMasses,
                                                           PVTPhaseValues&     density,
                                                           PVTPhaseValues&     viscosity ) {

   if ( not FastcauldronSimulator::getInstance ().getMcfHandler ().getApplyPvtAveraging ()) {
      return;
   }

   //------------------------------------------------------------//

   if ( density ( DesignatedPhase ) == 1000.0 ) {
      phaseMasses.zero ();
      phaseMasses.setPhaseComponents ( DesignatedPhase, masses );

      density ( DesignatedPhase ) = density ( RedundantPhase );
      density ( RedundantPhase ) = 1000.0;
      
      viscosity ( DesignatedPhase ) = viscosity ( RedundantPhase );
      viscosity ( RedundantPhase ) = 1.0;
   } else if ( density ( RedundantPhase ) == 1000.0 ) {
      // Do nothing.
   } else {
      double totalMass = masses.sum ();

      if ( totalMass > 0.0 ) {
         double massDesignated = phaseMasses.sum ( DesignatedPhase );
         double massRedundant = phaseMasses.sum ( RedundantPhase );

         density ( DesignatedPhase ) = (massDesignated / totalMass ) * density ( DesignatedPhase ) + massRedundant / totalMass * density ( RedundantPhase );
         density ( RedundantPhase ) = 1000.0;

         viscosity ( DesignatedPhase ) = (massDesignated / totalMass ) * viscosity ( DesignatedPhase ) + massRedundant / totalMass * viscosity ( RedundantPhase );
         viscosity ( RedundantPhase ) = 1.0;
      }

      phaseMasses.zero ();
      phaseMasses.setPhaseComponents ( DesignatedPhase, masses );
   }

}

//------------------------------------------------------------//

// Use the compute-saturation function that takes the already-flashed 
// composition as parameters after flashing. This will save on some code.
void ExplicitMultiComponentFlowSolver::computeSaturation ( const SubdomainElement&        element,
                                                           const PVTComponents&           concentrations,
                                                           const ImmobileSpeciesValues&   immobiles,
                                                                 Saturation&              saturation,
                                                           const bool                     print ) {

   const double maximumHcSaturation = FastcauldronSimulator::getInstance ().getMaximumHcSaturation ();

   PVTPhaseComponents phaseMasses;
   PVTPhaseComponents phaseFractions;
   PVTComponents      unitMasses;
   PVTPhaseValues     density;
   PVTPhaseValues     viscosity;
   double             temperature;
   double             porePressure;
   double             sumGasMolarMassRatio;
   double             sumOilMolarMassRatio;
   double             vapourFraction;
   double             concentrationSum = concentrations.sum ();
   double             vapourMolarMass;
   double             liquidMolarMass;
   double             hcSaturation;
   double             vapourSaturation;
   double             liquidSaturation;
   double             immobileSaturation;
   int                c;

#if 0
   double             elementVolume;
   double             elementPoreVolume;
   double             volumeRatio;
#endif

#if 0
   // Compute the volume of the element.
   elementVolumeCalculations ( element.getLayerElement (), elementVolume, elementPoreVolume, 4 );
   volumeRatio = elementVolume / elementPoreVolume;
#endif

   // Compute unit masses from concentrations. kg/m^3.
   unitMasses = concentrations * m_defaultMolarMasses;

   // Convert to kelvin.
   temperature = computeProperty ( element.getLayerElement (), Basin_Modelling::Temperature ) + 273.15;

   // Convert to pascals from mega-pascals.
   porePressure = NumericFunctions::Maximum ( 1.0e6 * computeProperty ( element.getLayerElement (), Basin_Modelling::Pore_Pressure ), 1.0e5 );

   pvtFlash::EosPack::getInstance ().computeWithLumping ( temperature, porePressure,
                                                          unitMasses.m_components,
                                                          phaseMasses.m_masses,
                                                          density.m_values,
                                                          viscosity.m_values );
   
   // Correct viscosity units.
   viscosity *= 0.001;

   averageComponents ( unitMasses, phaseMasses, density, viscosity );

   sumGasMolarMassRatio = 0.0;
   sumOilMolarMassRatio = 0.0;

   for ( c = 0; c < NumberOfPVTComponents; ++c ) {
      pvtFlash::ComponentId component = static_cast<pvtFlash::ComponentId>( c );

      // First stage in calculation of phase-fractions.
      phaseFractions ( pvtFlash::VAPOUR_PHASE, component ) = phaseMasses ( pvtFlash::VAPOUR_PHASE, component ) / m_defaultMolarMasses ( component );
      phaseFractions ( pvtFlash::LIQUID_PHASE, component ) = phaseMasses ( pvtFlash::LIQUID_PHASE, component ) / m_defaultMolarMasses ( component );

      // mol/m^3
      sumGasMolarMassRatio += phaseMasses ( pvtFlash::VAPOUR_PHASE, component ) / m_defaultMolarMasses ( component );
      sumOilMolarMassRatio += phaseMasses ( pvtFlash::LIQUID_PHASE, component ) / m_defaultMolarMasses ( component );
   }

   if ( sumGasMolarMassRatio + sumOilMolarMassRatio != 0.0 ) {
      vapourFraction = sumGasMolarMassRatio / ( sumGasMolarMassRatio + sumOilMolarMassRatio );
   } else {
      // What value would be reasonable here?
      vapourFraction = 0.0;
   }

   vapourMolarMass = 0.0;
   liquidMolarMass = 0.0;

   if ( sumGasMolarMassRatio != 0.0 ) {

      for ( c = 0; c < NumberOfPVTComponents; ++c ) {
         pvtFlash::ComponentId component = static_cast<pvtFlash::ComponentId>( c );

         // fractions (phase-component-moles per total-phase-component-moles).
         phaseFractions ( pvtFlash::VAPOUR_PHASE, component ) /= sumGasMolarMassRatio;
      }

   }

   if ( sumOilMolarMassRatio != 0.0 ) {

      for ( c = 0; c < NumberOfPVTComponents; ++c ) {
         pvtFlash::ComponentId component = static_cast<pvtFlash::ComponentId>( c );

         // fractions (phase-component-moles per total-phase-component-moles).
         phaseFractions ( pvtFlash::LIQUID_PHASE, component ) /= sumOilMolarMassRatio;
      }

   }

   for ( c = 0; c < NumberOfPVTComponents; ++c ) {
      pvtFlash::ComponentId component = static_cast<pvtFlash::ComponentId>( c );

      // kg/mol
      vapourMolarMass += phaseFractions ( pvtFlash::VAPOUR_PHASE, component ) * m_defaultMolarMasses ( component );
      liquidMolarMass += phaseFractions ( pvtFlash::LIQUID_PHASE, component ) * m_defaultMolarMasses ( component );
   }

   // Need to multiply by ratio of element-volume / element-pore-volume?
   if ( phaseMasses.sum ( pvtFlash::VAPOUR_PHASE ) > ConcentrationLowerLimit ) {
      vapourSaturation = concentrationSum * vapourFraction * vapourMolarMass / density ( pvtFlash::VAPOUR_PHASE );
   } else {
      vapourSaturation = 0.0;
   }

   if ( phaseMasses.sum ( pvtFlash::LIQUID_PHASE ) > ConcentrationLowerLimit ) {
      liquidSaturation = concentrationSum * ( 1.0 - vapourFraction ) * liquidMolarMass / density ( pvtFlash::LIQUID_PHASE );
   } else {
      liquidSaturation = 0.0;
   }

   if ( immobiles.sum () > ConcentrationLowerLimit ) {
      immobileSaturation = immobiles.getRetainedVolume ();
   } else {
      immobileSaturation = 0.0;
   }

   if ( m_useImmobileSaturation ) {
      hcSaturation = liquidSaturation + vapourSaturation + immobileSaturation;
   } else {
      hcSaturation = liquidSaturation + vapourSaturation;
   }

   if ( hcSaturation > maximumHcSaturation ) {
      // Force the saturations to be a reasonable value.
      liquidSaturation *= maximumHcSaturation / hcSaturation;
      vapourSaturation *= maximumHcSaturation / hcSaturation;
   }

   if ( m_useImmobileSaturation ) {

      if ( hcSaturation > maximumHcSaturation ) {
         immobileSaturation *= maximumHcSaturation / hcSaturation;
      }

      saturation.set ( liquidSaturation, vapourSaturation, immobileSaturation );
   } else {
      saturation.set ( liquidSaturation, vapourSaturation );
      // Set here so that the water-saturation is not affected by the immobile-species saturation.
      saturation ( Saturation::IMMOBILE ) = immobileSaturation;
   }

}

//------------------------------------------------------------//

void ExplicitMultiComponentFlowSolver::setSaturations ( FormationSubdomainElementGrid& formationGrid,
                                                        bool&                          errorInSaturation ) {

   LayerProps& theLayer = formationGrid.getFormation ();

   CompositionPetscVector     concentrations;
   SaturationVector           saturations;
   ImmobileSpeciesPetscVector immobiles;

   concentrations.setVector ( theLayer.getVolumeGrid ( NumberOfPVTComponents ), theLayer.getPreviousComponentVec (), INSERT_VALUES );
   saturations.setVector ( theLayer.getVolumeGrid ( Saturation::NumberOfPhases ), theLayer.getPhaseSaturationVec (), INSERT_VALUES );
   immobiles.setVector ( theLayer.getVolumeGrid ( ImmobileSpeciesValues::NumberOfImmobileSpecies ), theLayer.getImmobileComponentsVec (), INSERT_VALUES );


   int i;
   int j;
   int k;
   bool allAreFinite = true;
   bool elementSaturationIsFinite;

   for ( i = formationGrid.firstI (); i <= formationGrid.lastI (); ++i ) {

      for ( j = formationGrid.firstJ (); j <= formationGrid.lastJ (); ++j ) {

         if ( FastcauldronSimulator::getInstance ().getMapElement ( i, j ).isValid ()) {

            for ( k = formationGrid.firstK (); k <= formationGrid.lastK (); ++k ) {

               const SubdomainElement& element = formationGrid ( i, j, k );
               unsigned int elementK = element.getK ();

               if ( element.getLayerElement ().isActive ()) {

                  if ( concentrations ( k, j, i ).sum () > ConcentrationLowerLimit ) {
                     computeSaturation ( element, concentrations ( k, j, i ), immobiles ( k, j, i ), saturations ( k, j, i ), OUTPUT );
                     elementSaturationIsFinite = saturations ( k, j, i ).isFinite ();
                     allAreFinite = allAreFinite and elementSaturationIsFinite;

#if 0
                     if ( not elementSaturationIsFinite ) {
                        std::stringstream buffer;

                        buffer << " Location of error: " 
                               << " layer position = { " << element.getI () << ", " << element.getJ () << ", " << element.getLayerElement ().getLocalKPosition () << "} " 
                               << " subdomain position = { " << element.getI () << ", " << element.getJ () << ", " << element.getK () << "} " 
                               << endl;

                        cout << buffer.str () << flush;
                     }
#endif

                  } else {
                     saturations ( k, j, i ).initialise ();
                  }

                  if ( OUTPUT ) {
                     cout << " concentrations: " << std::setw ( 2 ) << i << "  " << std::setw ( 2 ) << j << "  " << std::setw ( 2 ) << elementK << "  " << concentrations ( k, j, i ).image () << std::endl
                          << " saturations                 " << saturations ( k, j, i ).image ()
                          << std::endl << std::endl;
                  }

               } else {
                  saturations ( k, j, i ).initialise ();
               }

            }

         } else {

            for ( k = formationGrid.firstK (); k <= formationGrid.lastK (); ++k ) {
               saturations ( k, j, i ).initialise ();
            }

         }

      }

   }

   errorInSaturation = not successfulExecution ( allAreFinite );
   concentrations.restoreVector ( UPDATE_EXCLUDING_GHOSTS );
   saturations.restoreVector ( UPDATE_EXCLUDING_GHOSTS );
}

//------------------------------------------------------------//

void ExplicitMultiComponentFlowSolver::setSaturations ( Subdomain& subdomain,
                                                        bool&      errorInSaturation ) {

   Subdomain::ActiveLayerIterator iter;
   bool errorInSaturationForLayer;

   subdomain.initialiseLayerIterator ( iter );
   errorInSaturation = false;

   while ( not iter.isDone ()) {
      setSaturations ( *iter, errorInSaturationForLayer );
      errorInSaturation = errorInSaturation or errorInSaturationForLayer;
      ++iter;
   }

}

//------------------------------------------------------------//

void ExplicitMultiComponentFlowSolver::computeSaturation ( const SubdomainElement&        element,
                                                           const PVTPhaseComponents&      phaseComposition,
                                                           const PVTPhaseValues           density,
                                                           const ImmobileSpeciesValues&   immobiles,
                                                                 Saturation&              saturation,
                                                           const bool                     print ) {

   const double maximumHcSaturation = FastcauldronSimulator::getInstance ().getMaximumHcSaturation ();

   PVTPhaseComponents phaseFractions;
   double             sumGasMolarMassRatio;
   double             sumOilMolarMassRatio;
   double             vapourFraction;
   double             vapourCompositionSum = phaseComposition.sum ( pvtFlash::VAPOUR_PHASE );
   double             liquidCompositionSum = phaseComposition.sum ( pvtFlash::LIQUID_PHASE );
   double             concentrationSum = vapourCompositionSum + liquidCompositionSum;
   double             vapourMolarMass;
   double             liquidMolarMass;
   double             hcSaturation;
   double             vapourSaturation;
   double             liquidSaturation;
   double             immobileSaturation;
   int                c;

#if 0
   double             elementVolume;
   double             elementPoreVolume;
   double             volumeRatio;
#endif

#if 0
   // Compute the volume of the element.
   elementVolumeCalculations ( element.getLayerElement (), elementVolume, elementPoreVolume, 4 );
   volumeRatio = elementVolume / elementPoreVolume;
#endif

   sumGasMolarMassRatio = 0.0;
   sumOilMolarMassRatio = 0.0;

   for ( c = 0; c < NumberOfPVTComponents; ++c ) {
      pvtFlash::ComponentId component = static_cast<pvtFlash::ComponentId>( c );

      // First stage in calculation of phase-fractions.
      phaseFractions ( pvtFlash::VAPOUR_PHASE, component ) = phaseComposition ( pvtFlash::VAPOUR_PHASE, component );
      phaseFractions ( pvtFlash::LIQUID_PHASE, component ) = phaseComposition ( pvtFlash::LIQUID_PHASE, component );

      // mol/m^3
      sumGasMolarMassRatio += phaseComposition ( pvtFlash::VAPOUR_PHASE, component );
      sumOilMolarMassRatio += phaseComposition ( pvtFlash::LIQUID_PHASE, component );
   }

   if ( sumGasMolarMassRatio + sumOilMolarMassRatio != 0.0 ) {
      vapourFraction = sumGasMolarMassRatio / ( sumGasMolarMassRatio + sumOilMolarMassRatio );
   } else {
      // What value would be reasonable here?
      vapourFraction = 0.0;
   }

   vapourMolarMass = 0.0;
   liquidMolarMass = 0.0;

   if ( sumGasMolarMassRatio != 0.0 ) {

      for ( c = 0; c < NumberOfPVTComponents; ++c ) {
         pvtFlash::ComponentId component = static_cast<pvtFlash::ComponentId>( c );

         // fractions (phase-component-moles per total-phase-component-moles).
         phaseFractions ( pvtFlash::VAPOUR_PHASE, component ) /= sumGasMolarMassRatio;
      }

   }

   if ( sumOilMolarMassRatio != 0.0 ) {

      for ( c = 0; c < NumberOfPVTComponents; ++c ) {
         pvtFlash::ComponentId component = static_cast<pvtFlash::ComponentId>( c );

         // fractions (phase-component-moles per total-phase-component-moles).
         phaseFractions ( pvtFlash::LIQUID_PHASE, component ) /= sumOilMolarMassRatio;
      }

   }

   for ( c = 0; c < NumberOfPVTComponents; ++c ) {
      pvtFlash::ComponentId component = static_cast<pvtFlash::ComponentId>( c );

      // kg/mol
      vapourMolarMass += phaseFractions ( pvtFlash::VAPOUR_PHASE, component ) * m_defaultMolarMasses ( component );
      liquidMolarMass += phaseFractions ( pvtFlash::LIQUID_PHASE, component ) * m_defaultMolarMasses ( component );
   }

   // Need to multiply by ratio of element-volume / element-pore-volume?
   if ( phaseComposition.sum ( pvtFlash::VAPOUR_PHASE ) > ConcentrationLowerLimit ) {
      vapourSaturation = concentrationSum * vapourFraction * vapourMolarMass / density ( pvtFlash::VAPOUR_PHASE );
   } else {
      vapourSaturation = 0.0;
   }

   if ( phaseComposition.sum ( pvtFlash::LIQUID_PHASE ) > ConcentrationLowerLimit ) {
      liquidSaturation = concentrationSum * ( 1.0 - vapourFraction ) * liquidMolarMass / density ( pvtFlash::LIQUID_PHASE );
   } else {
      liquidSaturation = 0.0;
   }

   if ( immobiles.sum () > ConcentrationLowerLimit ) {
      immobileSaturation = immobiles.getRetainedVolume ();
   } else {
      immobileSaturation = 0.0;
   }

   if ( m_useImmobileSaturation ) {
      hcSaturation = liquidSaturation + vapourSaturation + immobileSaturation;
   } else {
      hcSaturation = liquidSaturation + vapourSaturation;
   }

   if ( hcSaturation > maximumHcSaturation ) {
      // Force the saturations to be a reasonable value.
      liquidSaturation *= maximumHcSaturation / hcSaturation;
      vapourSaturation *= maximumHcSaturation / hcSaturation;
   }

   if ( m_useImmobileSaturation ) {

      if ( hcSaturation > maximumHcSaturation ) {
         immobileSaturation *= maximumHcSaturation / hcSaturation;
      }

      saturation.set ( liquidSaturation, vapourSaturation, immobileSaturation );
   } else {
      saturation.set ( liquidSaturation, vapourSaturation );
      // Set here so that the water-saturation is not affected by the immobile-species saturation.
      saturation ( Saturation::IMMOBILE ) = immobileSaturation;
   }

   if ( isnan ( saturation ( Saturation::WATER  )) or isinf ( saturation ( Saturation::WATER  )) or
        isnan ( saturation ( Saturation::VAPOUR )) or isinf ( saturation ( Saturation::VAPOUR )) or
        isnan ( saturation ( Saturation::LIQUID )) or isinf ( saturation ( Saturation::LIQUID ))) {
      cout << " incorrect inter saturation: " << element.getI ()  << "  " << element.getJ ()  << "  " << element.getK ()  << "  " << endl << flush;
      cout << saturation.image () << endl << flush;
      cout << density.image () << endl << flush;
      cout << phaseComposition.image () << endl << flush;
   }

}

//------------------------------------------------------------//

void ExplicitMultiComponentFlowSolver::setSaturations ( FormationSubdomainElementGrid& formationGrid,
                                                        const PhaseCompositionArray&   phaseComposition,
                                                        const PhaseValueArray&         phaseDensities,
                                                        SaturationArray&               saturations,
                                                        bool&                          errorInSaturation ) {

   LayerProps& theLayer = formationGrid.getFormation ();

   CompositionPetscVector     concentrations;
   ImmobileSpeciesPetscVector immobiles;

   int i;
   int j;
   int k;
   int elementK;
   bool allAreFinite = true;
   bool elementSaturationIsFinite;

   concentrations.setVector ( theLayer.getVolumeGrid ( NumberOfPVTComponents ), theLayer.getPreviousComponentVec (), INSERT_VALUES );
   immobiles.setVector ( theLayer.getVolumeGrid ( ImmobileSpeciesValues::NumberOfImmobileSpecies ), theLayer.getImmobileComponentsVec (), INSERT_VALUES );

   for ( i = formationGrid.firstI (); i <= formationGrid.lastI (); ++i ) {

      for ( j = formationGrid.firstJ (); j <= formationGrid.lastJ (); ++j ) {

         if ( FastcauldronSimulator::getInstance ().getMapElement ( i, j ).isValid ()) {

            for ( k = formationGrid.firstK (); k <= formationGrid.lastK (); ++k ) {

               const SubdomainElement& element = formationGrid ( i, j, k );
               elementK = element.getK ();

               if ( element.getLayerElement ().isActive ()) {

                  if ( concentrations ( k, j, i ).sum () > ConcentrationLowerLimit ) {
                     // Index of immobiles is correct.
                     computeSaturation ( element,
                                         phaseComposition ( i, j, elementK ),
                                         phaseDensities ( i, j, elementK ),
                                         immobiles ( k, j, i ),
                                         saturations ( i, j, elementK ), OUTPUT );
                     elementSaturationIsFinite = saturations ( i, j, elementK ).isFinite ();
                     allAreFinite = allAreFinite and elementSaturationIsFinite;

#if 0
                     if ( not elementSaturationIsFinite ) {
                        std::stringstream buffer;

                        buffer << " Location of error: " 
                               << " layer position = { " << element.getI () << ", " << element.getJ () << ", " << element.getLayerElement ().getLocalKPosition () << "} " 
                               << " subdomain position = { " << element.getI () << ", " << element.getJ () << ", " << element.getK () << "} " 
                               << endl;

                        cout << buffer.str () << flush;
                     }
#endif


                  } else {
                     saturations ( i, j, elementK ).initialise ();
                  }

               } else {
                  saturations ( i, j, elementK ).initialise ();
               }

               if ( not allAreFinite ) {
                  break;
               }

            }

         } else {

            for ( k = formationGrid.firstK (); k <= formationGrid.lastK (); ++k ) {
               const SubdomainElement& element = formationGrid ( i, j, k );
               saturations ( i, j, element.getK ()).initialise ();
            }

         }

      }

      if ( not allAreFinite ) {
         break;
      }

   }

   errorInSaturation = not successfulExecution ( allAreFinite );
   concentrations.restoreVector ( UPDATE_EXCLUDING_GHOSTS );
   immobiles.restoreVector ( UPDATE_EXCLUDING_GHOSTS );
}

//------------------------------------------------------------//

void ExplicitMultiComponentFlowSolver::setSaturations ( Subdomain&                   subdomain,
                                                        const PhaseCompositionArray& phaseComposition,
                                                        const PhaseValueArray&       phaseDensities,
                                                        SaturationArray&             saturations,
                                                        bool&                        errorInSaturation ) {

   Subdomain::ActiveLayerIterator iter;
   bool errorInSaturationForLayer;

   subdomain.initialiseLayerIterator ( iter );
   errorInSaturation = false;

   while ( not iter.isDone ()) {
      setSaturations ( *iter, phaseComposition, phaseDensities, saturations, errorInSaturationForLayer );
      errorInSaturation = errorInSaturation or errorInSaturationForLayer;
      ++iter;
   }

}

//------------------------------------------------------------//


void ExplicitMultiComponentFlowSolver::setTimeOfElementInvasion ( FormationSubdomainElementGrid& formationGrid, double endTime ) {

   LayerProps& theLayer = formationGrid.getFormation ();

   CompositionPetscVector     concentrations;
   PetscBlockVector<double>  timeOfInvasions;

   concentrations.setVector ( theLayer.getVolumeGrid ( NumberOfPVTComponents ), theLayer.getPreviousComponentVec (), INSERT_VALUES );
   timeOfInvasions.setVector ( theLayer.getVolumeGrid ( 1 ), theLayer.getTimeOfElementInvasionVec (), INSERT_VALUES );

   int i;
   int j;
   int k;

   for ( i = formationGrid.firstI (); i <= formationGrid.lastI (); ++i ) {

      for ( j = formationGrid.firstJ (); j <= formationGrid.lastJ (); ++j ) {

         if ( FastcauldronSimulator::getInstance ().getMapElement ( i, j ).isValid ()) {

            for ( k = formationGrid.firstK (); k <= formationGrid.lastK (); ++k ) {

               const SubdomainElement& element = formationGrid ( i, j, k );
               unsigned int elementK = element.getK ();

               if ( element.getLayerElement ().isActive ()) {

                  if ( concentrations ( k, j, i ).sum () > ConcentrationLowerLimit && timeOfInvasions ( k, j, i ) == CAULDRONIBSNULLVALUE ) {
                     timeOfInvasions( k, j, i) =  endTime;
                  }

               }

            }
            
         }
         
      }
      
   }
   
   concentrations.restoreVector ( UPDATE_EXCLUDING_GHOSTS );
   timeOfInvasions.restoreVector ( UPDATE_EXCLUDING_GHOSTS );
}

//------------------------------------------------------------//

void ExplicitMultiComponentFlowSolver::setTimeOfElementInvasion ( Subdomain& subdomain, double endTime ) {

   Subdomain::ActiveLayerIterator iter;

   subdomain.initialiseLayerIterator ( iter );

   while ( not iter.isDone ()) {
      setTimeOfElementInvasion ( *iter, endTime );
      ++iter;
   }

}

//------------------------------------------------------------//

void ExplicitMultiComponentFlowSolver::applyOtgc ( SubdomainElement& element,
                                                   PVTComponents&    concentration,
                                                   ImmobileSpeciesValues& immobiles,
                                                   const TemporalPropertyInterpolator& porePressure,
                                                   const TemporalPropertyInterpolator& temperature,
                                                   const double      timeStepStart,
                                                   const double      timeStepEnd,
                                                   const double      lambdaStart,
                                                   const double      lambdaEnd ) {

   if ( concentration.sum () < ConcentrationLowerLimit ) {
      return;
   }

   const Genex6::SpeciesManager& speciesManager = m_otgcSimulator->getSpeciesManager ();
   const Genex6::Species** allSpecies = m_otgcSimulator->getSpeciesInChemicalModel ();
   // const std::vector<std::string>& speciesNames = m_otgcSimulator->getSpeciesInChemicalModel ();

#ifndef NEW_OTGC_INTERFACE 
   std::map<string, double> components;
   std::map<string, double>::iterator componentsIter;
#else
   double components[Genex6::SpeciesManager::numberOfSpecies];
#endif

   double   concentrationSum;

   unsigned int species;

   double previousTemperature  = temperature ( element, lambdaStart );
   double currentTemperature   = temperature ( element, lambdaEnd );

   double previousPorePressure = NumericFunctions::Maximum ( 1.0e6 * porePressure ( element, lambdaStart ), 1.0e5 ); // Pascals
   double currentPorePressure  = NumericFunctions::Maximum ( 1.0e6 * porePressure ( element, lambdaEnd   ), 1.0e5 ); // Pascals;

   // double previousTemperature  = interpolateProperty ( element.getLayerElement (), Basin_Modelling::Temperature, lambdaStart );
   // double currentTemperature   = interpolateProperty ( element.getLayerElement (), Basin_Modelling::Temperature, lambdaEnd );

   // double previousPorePressure = NumericFunctions::Maximum ( 1.0e6 * interpolateProperty ( element.getLayerElement (), Basin_Modelling::Pore_Pressure, lambdaStart ), 1.0e5 ); // Pascals
   // double currentPorePressure  = NumericFunctions::Maximum ( 1.0e6 * interpolateProperty ( element.getLayerElement (), Basin_Modelling::Pore_Pressure, lambdaEnd   ), 1.0e5 ); // Pascals;

   // Convert to kg/m^3
   concentration *= m_defaultMolarMasses;
   concentrationSum = concentration.sum () + immobiles.sum ();

   // Collect and normalise all modelled species for OTGC.
   //
   // First mobile ones.
#ifndef NEW_OTGC_INTERFACE

   for ( species = 0; species < Genex6::SpeciesManager::numberOfSpecies; ++species ) {

      if ( allSpecies [ species ] != 0 ) {
         const std::string& name = allSpecies [ species ]->GetName ();
         pvtFlash::ComponentId id = speciesManager.mapIdToPvtComponents ( allSpecies [ species ]->GetId ());

         if ( id != pvtFlash::UNKNOWN ) {
            components [ name ] = concentration ( id ) / concentrationSum;
         } 
      }

   }

   // Then the immobiles.
   for ( species = 0; species < ImmobileSpeciesValues::NumberOfImmobileSpecies; ++species ) {
      ImmobileSpeciesValues::SpeciesId id = ImmobileSpeciesValues::SpeciesId ( species );
      const string& name = ImmobileSpeciesValues::getName ( id );

      components [ name ] = immobiles ( id ) / concentrationSum;
   }
#else
   for ( species = 0; species < Genex6::SpeciesManager::numberOfSpecies; ++species ) {

      if ( allSpecies [ species ] != 0 ) {

         pvtFlash::ComponentId id = speciesManager.mapIdToPvtComponents ( species + 1 );
         if ( id != pvtFlash::UNKNOWN ) { 
            components [ species ] = concentration ( id ) / concentrationSum;
         } else {
            components[ species ] = 0.0;
         }
      }

   } 

   // Then the immobiles.
   for ( species = 0; species < ImmobileSpeciesValues::NumberOfImmobileSpecies; ++species ) {
      ImmobileSpeciesValues::SpeciesId id = ImmobileSpeciesValues::SpeciesId ( species );

      int speciesManagerId = ImmobileSpeciesValues::getSpeciesManagerId( id );

      if( speciesManagerId >= 0 ) {
         components [ speciesManagerId - 1 ]  = immobiles ( id ) / concentrationSum;
       }
   }
#endif

   OTGC6::SimulatorState otgcState ( timeStepStart, allSpecies, components );

   m_otgcSimulator->computeInterval ( otgcState,
                                      previousTemperature, currentTemperature,
                                      previousPorePressure, currentPorePressure,
                                      timeStepStart, timeStepEnd );


     
#ifdef NEW_OTGC_INTERFACE
   otgcState.GetSpeciesStateConcentrations ( components );
#else
   otgcState.GetSpeciesStateConcentrations ( &m_otgcSimulator->getChemicalModel (), components );
#endif

#ifndef NEW_OTGC_INTERFACE
   // De-normalise the species and assign back to mobile and immobile objects.
   for ( componentsIter = components.begin (); componentsIter != components.end (); ++componentsIter ) {
      int speciesId = CBMGenerics::ComponentManager::getInstance ().GetSpeciedIdByName ( componentsIter->first );
 
      if ( speciesId != -1 ) {
         pvtFlash::ComponentId id = static_cast<pvtFlash::ComponentId>( speciesId );
         concentration ( id ) = componentsIter->second * concentrationSum;
     } else {

         ImmobileSpeciesValues::SpeciesId id = ImmobileSpeciesValues::getId ( componentsIter->first );

         if ( id != ImmobileSpeciesValues::UNKNOWN ) {
            // Keep the immobile species in kg/m^3 units.
            immobiles ( id ) =  componentsIter->second * concentrationSum;
         }
      }
   }
#else

   for ( species = 0; species < Genex6::SpeciesManager::numberOfSpecies; ++species ) {
      
      pvtFlash::ComponentId id = speciesManager.mapIdToPvtComponents( species + 1 );

      if( id != pvtFlash::UNKNOWN ) {
         concentration ( id ) = components[species] * concentrationSum;
      } 
   }

   // Then the immobiles.
   for ( species = 0; species < ImmobileSpeciesValues::NumberOfImmobileSpecies; ++species ) {
      ImmobileSpeciesValues::SpeciesId id = ImmobileSpeciesValues::SpeciesId ( species );
      int speciesManagerId = ImmobileSpeciesValues::getSpeciesManagerId( id );

      if( speciesManagerId >= 0 ) {
         immobiles ( id ) = components [ speciesManagerId - 1 ]  * concentrationSum;
      }
   }
 
#endif
   // Convert back to mol/m^3.
   concentration /= m_defaultMolarMasses;

   // Remove any COx and H2S generated by OTGC.
   concentration ( pvtFlash::COX ) = 0.0;
   concentration ( pvtFlash::H2S ) = 0.0;
}

//------------------------------------------------------------//

void ExplicitMultiComponentFlowSolver::applyOtgc ( FormationSubdomainElementGrid&      formationGrid,
                                                   const TemporalPropertyInterpolator& porePressure,
                                                   const TemporalPropertyInterpolator& temperature,
                                                   const double                        timeStepStart,
                                                   const double                        timeStepEnd,
                                                   const double                        lambdaStart,
                                                   const double                        lambdaEnd ) {

   LayerProps& theLayer = formationGrid.getFormation ();

   const ElementVolumeGrid& concentrationGrid = theLayer.getVolumeGrid ( NumberOfPVTComponents );
   const ElementVolumeGrid& immobileComponentsGrid = theLayer.getVolumeGrid ( ImmobileSpeciesValues::NumberOfImmobileSpecies );

   int i;
   int j;
   int k;

   CompositionPetscVector     layerConcentrations;
   ImmobileSpeciesPetscVector layerImmobileComponents;

   layerConcentrations.setVector ( concentrationGrid, theLayer.getPreviousComponentVec (), INSERT_VALUES );
   layerImmobileComponents.setVector ( immobileComponentsGrid, theLayer.getImmobileComponentsVec (), INSERT_VALUES );

   for ( i = concentrationGrid.firstI (); i <= concentrationGrid.lastI (); ++i ) {

      for ( j = concentrationGrid.firstJ (); j <= concentrationGrid.lastJ (); ++j ) {

         if ( FastcauldronSimulator::getInstance ().getMapElement ( i, j ).isValid ()) {

            for ( k = formationGrid.firstK (); k <= formationGrid.lastK (); ++k ) {
               SubdomainElement& element = formationGrid ( i, j, k );
               PVTComponents& concentration = layerConcentrations ( k, j, i );
               ImmobileSpeciesValues& immobiles = layerImmobileComponents ( k, j, i );

               applyOtgc ( element, concentration, immobiles, porePressure, temperature, timeStepStart, timeStepEnd, lambdaStart, lambdaEnd );
            }

         }

      }

   }

}

//------------------------------------------------------------//

void ExplicitMultiComponentFlowSolver::applyOtgc ( Subdomain&                          subdomain, 
                                                   const TemporalPropertyInterpolator& porePressure,
                                                   const TemporalPropertyInterpolator& temperature,
                                                   const double                        timeStepStart,
                                                   const double                        timeStepEnd,
                                                   const double                        lambdaStart,
                                                   const double                        lambdaEnd ) {

   if ( not FastcauldronSimulator::getInstance ().getMcfHandler ().applyOtgc () or m_otgcSimulator == 0 ) {
      return;
   }

   Subdomain::ActiveLayerIterator iter;

   subdomain.initialiseLayerIterator ( iter );

   while ( not iter.isDone ()) {
      applyOtgc ( *iter, porePressure, temperature, timeStepStart, timeStepEnd, lambdaStart, lambdaEnd );
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

      for ( p = 0;  p < Basin_Modelling::Number_Of_Fundamental_Properties; ++p ) {
         Basin_Modelling::Fundamental_Property prop = Basin_Modelling::Fundamental_Property ( p );

         activated ( p ) = currentProperties.propertyIsActivated ( prop );

         if ( not activated ( p )) {
            currentProperties.Activate_Property ( prop, INSERT_VALUES, true );
         }

      }

      currentAlreadyActivatedProperties.push_back ( activated );
      activated.fill ( false );

      for ( p = 0;  p < Basin_Modelling::Number_Of_Fundamental_Properties; ++p ) {
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

      for ( p = 0;  p < Basin_Modelling::Number_Of_Fundamental_Properties; ++p ) {
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

      for ( p = 0;  p < Basin_Modelling::Number_Of_Fundamental_Properties; ++p ) {
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
                                                                 SaturationVector&              averagedSaturations,
                                                                 ScalarPetscVector&             divisor ) {

   LayerProps& theLayer = formationGrid.getFormation ();

   Saturation elementSaturation;

   int i;
   int j;
   int k;
   int l;

   SaturationVector layerSaturations;
   layerSaturations.setVector ( theLayer.getVolumeGrid ( Saturation::NumberOfPhases ), theLayer.getPhaseSaturationVec (), INSERT_VALUES );

   for ( i = formationGrid.firstI (); i <= formationGrid.lastI (); ++i ) {

      for ( j = formationGrid.firstJ (); j <= formationGrid.lastJ (); ++j ) {

         if ( FastcauldronSimulator::getInstance ().getMapElement ( i, j ).isValid ()) {

            for ( k = formationGrid.firstK (); k <= formationGrid.lastK (); ++k ) {

               const SubdomainElement& element = formationGrid ( i, j, k );
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
                                                                 SaturationVector&  averagedSaturations,
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
                                                                SaturationVector& averagedSaturations ) {

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

      SaturationVector layerAveragedSaturations ( layer.getNodalVolumeGrid ( Saturation::NumberOfPhases ), layer.getAveragedSaturations (), INSERT_VALUES );

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

   DACreateGlobalVector ( saturationGrid.getDa (), &averagedSaturationsVec );
   DACreateGlobalVector ( nodalGrid.getDa (), &divisorVec );

   VecZeroEntries ( averagedSaturationsVec );
   VecZeroEntries ( divisorVec );

   SaturationVector  averagedSaturations;
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

   VecDestroy ( averagedSaturationsVec );
   VecDestroy ( divisorVec );

}

//------------------------------------------------------------//

void ExplicitMultiComponentFlowSolver::getAveragedSaturationCoefficients ( const SubdomainElement&                   element, 
                                                                           const SaturationVector&                   layerAveragedSaturations,
                                                                                 FiniteElementMethod::ElementVector& vapourSaturationCoefficients, 
                                                                                 FiniteElementMethod::ElementVector& liquidSaturationCoefficients,
                                                                           const bool                                printIt ) {

   const LayerElement& layerElement = element.getLayerElement ();
   const LayerProps* layer = layerElement.getFormation ();
   
   FiniteElementMethod::ElementVector waterSaturationCoefficients;

   int i;
   bool isInconsistent = false;

   for ( i = 0; i < 8; ++i ) {
      const Saturation& sat = layerAveragedSaturations ( layerElement.getNodeLocalKPosition ( i ), layerElement.getNodeJPosition ( i ), layerElement.getNodeIPosition ( i ));

      waterSaturationCoefficients ( i + 1 ) =  sat ( Saturation::WATER );
      vapourSaturationCoefficients ( i + 1 ) = sat ( Saturation::VAPOUR );
      liquidSaturationCoefficients ( i + 1 ) = sat ( Saturation::LIQUID );
   }

}

//------------------------------------------------------------//

void ExplicitMultiComponentFlowSolver::collectElementPermeabilities ( const Subdomain&         subdomain,
                                                                      const ElementVolumeGrid& elementGrid,
                                                                      const double             lambda,
                                                                      ElementFaceValueVector&  subdomainPermeabilityN, 
                                                                      ElementFaceValueVector&  subdomainPermeabilityH ) const {


   FiniteElementMethod::FiniteElement         finiteElement;
   FiniteElementMethod::ElementGeometryMatrix geometryMatrix;
   FiniteElementMethod::ElementVector         vesCoeffs;
   FiniteElementMethod::ElementVector         maxVesCoeffs;

   CompoundProperty porosity;

   double x;
   double y;
   double z;

   double ves;
   double maxVes;
   double permNormal;
   double permPlane;

   int i;
   int j;
   int k;
   int face;

   for ( i = elementGrid.firstI (); i <= elementGrid.lastI (); ++i ) {

      for ( j = elementGrid.firstJ (); j <= elementGrid.lastJ (); ++j ) {

         for ( k = elementGrid.firstK (); k <= elementGrid.lastK (); ++k ) {

            const SubdomainElement& element = subdomain.getElement ( i, j, k );
            const LayerElement& layerElement = element.getLayerElement ();

            unsigned int elementK = element.getK ();

            ElementFaceValues& elementPermeabilityN = subdomainPermeabilityN ( elementK, j, i );
            ElementFaceValues& elementPermeabilityH = subdomainPermeabilityH ( elementK, j, i );

            if ( layerElement.isActive ()) {

               interpolateCoefficients ( layerElement, Basin_Modelling::VES_FP, vesCoeffs, lambda );
               interpolateCoefficients ( layerElement, Basin_Modelling::Max_VES, maxVesCoeffs, lambda );

               const Lithology* lithology = layerElement.getLithology ();
               const LayerProps* layer = layerElement.getFormation ();

               getGeometryMatrix ( element.getLayerElement (), geometryMatrix, lambda );

               finiteElement.setGeometry ( geometryMatrix );

               for ( face = VolumeData::GAMMA_1; face <= VolumeData::GAMMA_6; ++face ) {
                  const VolumeData::BoundaryId id = static_cast<VolumeData::BoundaryId>( face );

                  // Should we check for an active face?

                  // compute permeability at centre of face.
                  getCentreOfElementFace ( layerElement, id, x, y, z );
                  finiteElement.setQuadraturePoint ( x, y, z );

                  ves = finiteElement.interpolate ( vesCoeffs );
                  maxVes = finiteElement.interpolate ( maxVesCoeffs );
                  
                  lithology->getPorosity ( ves, maxVes, false, 0.0, porosity );
                  lithology->calcBulkPermeabilityNP ( ves, maxVes, porosity, permNormal, permPlane );

                  elementPermeabilityN ( id ) = 1.0 / permNormal;
                  elementPermeabilityH ( id ) = 1.0 / permPlane;
               }

            }

         }

      }

   }   

}

//------------------------------------------------------------//

void ExplicitMultiComponentFlowSolver::addNeighbourPermeabilities ( const Subdomain&         subdomain,
                                                                    const ElementVolumeGrid& elementGrid,
                                                                    ElementFaceValueVector&  subdomainPermeabilityN, 
                                                                    ElementFaceValueVector&  subdomainPermeabilityH,
                                                                    ElementFaceValueArray&   intermediatePermeabilityN,
                                                                    ElementFaceValueArray&   intermediatePermeabilityH ) const {

   int i;
   int j;
   int k;
   int face;

   for ( i = elementGrid.firstI (); i <= elementGrid.lastI (); ++i ) {

      for ( j = elementGrid.firstJ (); j <= elementGrid.lastJ (); ++j ) {

         for ( k = elementGrid.firstK (); k <= elementGrid.lastK (); ++k ) {

            const SubdomainElement& element = subdomain.getElement ( i, j, k );
            const LayerElement& layerElement = element.getLayerElement ();

            if ( layerElement.isActive ()) {

               const ElementFaceValues& elementPermeabilityN = subdomainPermeabilityN ( element.getK (), j, i );
               const ElementFaceValues& elementPermeabilityH = subdomainPermeabilityH ( element.getK (), j, i );

               ElementFaceValues& intermediateElementPermeabilityN = intermediatePermeabilityN ( i, j, element.getK ());
               ElementFaceValues& intermediateElementPermeabilityH = intermediatePermeabilityH ( i, j, element.getK ());

               for ( face = VolumeData::GAMMA_1; face <= VolumeData::GAMMA_6; ++face ) {

                  const VolumeData::BoundaryId id = static_cast<VolumeData::BoundaryId>( face );

                  const SubdomainElement* neighbour = element.getActiveNeighbour ( id );

                  if ( neighbour != 0 ) {
                     const VolumeData::BoundaryId opposite = VolumeData::opposite ( id );

                     const ElementFaceValues& neighbourPermeabilityN = subdomainPermeabilityN ( neighbour->getK (), neighbour->getJ (), neighbour->getI ());
                     const ElementFaceValues& neighbourPermeabilityH = subdomainPermeabilityH ( neighbour->getK (), neighbour->getJ (), neighbour->getI ());

                     intermediateElementPermeabilityN ( id ) = 0.5 * ( elementPermeabilityN ( id ) + neighbourPermeabilityN ( opposite ));
                     intermediateElementPermeabilityH ( id ) = 0.5 * ( elementPermeabilityH ( id ) + neighbourPermeabilityH ( opposite ));

                  } else {
                     intermediateElementPermeabilityN ( id ) = elementPermeabilityN ( id );
                     intermediateElementPermeabilityH ( id ) = elementPermeabilityH ( id );
                  }

               }

            }

         }

      }

   }   

}

//------------------------------------------------------------//

void ExplicitMultiComponentFlowSolver::recoverAveragedPermeabilities ( const Subdomain&         subdomain,
                                                                       const ElementVolumeGrid& elementGrid,
                                                                       ElementFaceValueVector&  subdomainPermeabilityN, 
                                                                       ElementFaceValueVector&  subdomainPermeabilityH,
                                                                       ElementFaceValueArray&   intermediatePermeabilityN,
                                                                       ElementFaceValueArray&   intermediatePermeabilityH ) const {

   int i;
   int j;
   int k;
   int face;

   for ( i = elementGrid.firstI (); i <= elementGrid.lastI (); ++i ) {

      for ( j = elementGrid.firstJ (); j <= elementGrid.lastJ (); ++j ) {

         for ( k = elementGrid.firstK (); k <= elementGrid.lastK (); ++k ) {

            const SubdomainElement& element = subdomain.getElement ( i, j, k );
            const LayerElement& layerElement = element.getLayerElement ();

            if ( layerElement.isActive ()) {

               ElementFaceValues& elementPermeabilityN = subdomainPermeabilityN ( element.getK (), j, i );
               ElementFaceValues& elementPermeabilityH = subdomainPermeabilityH ( element.getK (), j, i );

               ElementFaceValues& intermediateElementPermeabilityN = intermediatePermeabilityN ( i, j, element.getK ());
               ElementFaceValues& intermediateElementPermeabilityH = intermediatePermeabilityH ( i, j, element.getK ());

               for ( face = VolumeData::GAMMA_1; face <= VolumeData::GAMMA_6; ++face ) {
                  const VolumeData::BoundaryId id = static_cast<VolumeData::BoundaryId>( face );

                  // If elementPermeabilityN ( id ) > 0.0 then elementPermeabilityH ( id ) will also be greater than zero.
                  if ( elementPermeabilityN ( id ) > 0.0 ) {
                     elementPermeabilityN ( id ) = 1.0 / intermediateElementPermeabilityN ( id );
                     elementPermeabilityH ( id ) = 1.0 / intermediateElementPermeabilityH ( id );
                  }

               }

            }

         }

      }

   }   

}


//------------------------------------------------------------//

void ExplicitMultiComponentFlowSolver::computeAveragePermeabilities ( const Subdomain& subdomain,
                                                                      const double     lambda,
                                                                      Vec subdomainPermeabilityNVec,
                                                                      Vec subdomainPermeabilityHVec ) const {

   const ElementVolumeGrid& elementGrid = subdomain.getVolumeGrid ();
   const ElementVolumeGrid& permeabilityGrid = subdomain.getVolumeGrid ( ElementFaceValues::NumberOfFaces );

   ElementFaceValueVector subdomainPermeabilityN;
   ElementFaceValueVector subdomainPermeabilityH;
   ElementFaceValueArray  intermediatePermeabilityH;
   ElementFaceValueArray  intermediatePermeabilityN;

   intermediatePermeabilityN.create ( permeabilityGrid );
   intermediatePermeabilityH.create ( permeabilityGrid );

   // The averaging is done in three steps:
   //
   //  1. Add permeability from every element local to processor;
   //  2. Add permeability from adjacent face of neighbouring element, including ghost elements;
   //  3. recover the averaged permeability for every element local to processor.
   //

   // The first step is to compute the permeability for each element local to processor.
   subdomainPermeabilityN.setVector ( permeabilityGrid, subdomainPermeabilityNVec, INSERT_VALUES );
   subdomainPermeabilityH.setVector ( permeabilityGrid, subdomainPermeabilityHVec, INSERT_VALUES );

   collectElementPermeabilities ( subdomain, elementGrid, lambda, subdomainPermeabilityN, subdomainPermeabilityH );

   subdomainPermeabilityN.restoreVector ( UPDATE_EXCLUDING_GHOSTS );
   subdomainPermeabilityH.restoreVector ( UPDATE_EXCLUDING_GHOSTS );

   // The second step is to update the element permeabilities with the neighbours permeability, including ghost elements.
   subdomainPermeabilityN.setVector ( permeabilityGrid, subdomainPermeabilityNVec, INSERT_VALUES, true );
   subdomainPermeabilityH.setVector ( permeabilityGrid, subdomainPermeabilityHVec, INSERT_VALUES, true );

   addNeighbourPermeabilities ( subdomain, elementGrid, subdomainPermeabilityN, subdomainPermeabilityH,
                                intermediatePermeabilityN, intermediatePermeabilityH );

   // The third step is to compute the permeability by taking the reciprocal of the stored value.
   // This is done because we are computing the harmonic average.
   // This is done only on the values which are local to this processor.
   recoverAveragedPermeabilities ( subdomain, elementGrid, subdomainPermeabilityN, subdomainPermeabilityH,
                                   intermediatePermeabilityN, intermediatePermeabilityH);

   // Update permeabilities on all processors.
   subdomainPermeabilityN.restoreVector ( UPDATE_EXCLUDING_GHOSTS );
   subdomainPermeabilityH.restoreVector ( UPDATE_EXCLUDING_GHOSTS );
}

//------------------------------------------------------------//

double ExplicitMultiComponentFlowSolver::totalLayerHcMass ( FormationSubdomainElementGrid& formationGrid,
                                                            const double                   lambda ) const {

   const LayerProps& theLayer = formationGrid.getFormation ();

   const ElementVolumeGrid& concentrationGrid = theLayer.getVolumeGrid ( NumberOfPVTComponents );
   const ElementVolumeGrid& immobileComponentsGrid = theLayer.getVolumeGrid ( ImmobileSpeciesValues::NumberOfImmobileSpecies );

   FiniteElementMethod::ElementGeometryMatrix geometryMatrix;
   PVTComponents massConcentration;

   double elementPorePressure;
   double capillaryPressure;

   int i;
   int j;
   int k;

   CompositionPetscVector concentrations;
   concentrations.setVector ( theLayer.getVolumeGrid ( NumberOfPVTComponents ), theLayer.getPreviousComponentVec (), INSERT_VALUES, true );
   ImmobileSpeciesPetscVector layerImmobileComponents;
   layerImmobileComponents.setVector ( immobileComponentsGrid, theLayer.getImmobileComponentsVec (), INSERT_VALUES );

   double layerMass = 0.0;
   double elementMass;
   double elementVolume;
   double elementPoreVolume;

   for ( i = concentrationGrid.firstI (); i <= concentrationGrid.lastI (); ++i ) {

      for ( j = concentrationGrid.firstJ (); j <= concentrationGrid.lastJ (); ++j ) {

         if ( FastcauldronSimulator::getInstance ().getMapElement ( i, j ).isValid ()) {

            for ( k = formationGrid.lastK (); k >= formationGrid.firstK (); --k ) {
               SubdomainElement& element = formationGrid ( i, j, k );
               const LayerElement& layerElement = element.getLayerElement ();

               if ( layerElement.isActive ()) {
                  massConcentration = m_defaultMolarMasses * concentrations ( k, j, i );

                  getGeometryMatrix ( element.getLayerElement (), geometryMatrix, lambda );

                  elementVolumeCalculations ( element.getLayerElement (),
                                              geometryMatrix,
                                              elementVolume,
                                              elementPoreVolume, 
                                              lambda,
                                              m_massMatrixQuadratureDegree );

                  elementMass = ( massConcentration.sum () + layerImmobileComponents ( k, j, i ).sum ()) * elementPoreVolume;

                  layerMass += elementMass;

               }

            }

         }

      }

   }

   concentrations.restoreVector ( UPDATE_EXCLUDING_GHOSTS );
   layerImmobileComponents.restoreVector ( UPDATE_EXCLUDING_GHOSTS );

   return layerMass;
}

//------------------------------------------------------------//

double ExplicitMultiComponentFlowSolver::totalHcMass ( Subdomain&   subdomain,
                                                       const double lambda ) const {

   Subdomain::ActiveLayerIterator iter;

   subdomain.initialiseLayerIterator ( iter );
   double processorMass = 0.0;
   double totalMass = 0.0;
   double layerMass;

   while ( not iter.isDone ()) {
      layerMass = totalLayerHcMass ( *iter, lambda );
      processorMass += layerMass;
      ++iter;
   }

   totalMass = MpiFunctions::Sum<double>( PETSC_COMM_WORLD, processorMass );

#if 0
   PetscPrintf ( PETSC_COMM_WORLD, " total hc mass in domain %e \n", totalMass );
#endif

   return totalMass;
}

//------------------------------------------------------------//

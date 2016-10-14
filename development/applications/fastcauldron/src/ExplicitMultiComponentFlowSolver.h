//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 
#ifndef FASTCAULDRON__EXPLICIT_MULTI_COMPONENT_FLOW_SOLVER__H
#define FASTCAULDRON__EXPLICIT_MULTI_COMPONENT_FLOW_SOLVER__H

#include <map>
#include <vector>

#include "petsc.h"
#include "petscmat.h"

#include "WallTime.h"

#include "FiniteElement.h"
#include "BasisFunction.h"
#include "FiniteElementTypes.h"

#include "propinterface.h"
#include "ElementVolumeGrid.h"
#include "LayerElement.h"

#include "Subdomain.h"
#include "FormationSubdomainElementGrid.h"

#include "PetscBlockVector.h"
#include "PVTCalculator.h"
#include "Saturation.h"

#include "ElementFaceValues.h"

#include "ghost_array.h"
#include "ConstrainedArray.h"
#include "ConstantsFastcauldron.h"

#include "property_manager.h"

#include "Simulator.h"

#include "ImmobileSpeciesValues.h"
#include "DarcyCalculations.h"


#include "TemporalPropertyInterpolator.h"
#include "PoreVolumeInterpolatorCalculator.h"
#include "FaceAreaInterpolatorCalculator.h"
#include "FacePermeabilityInterpolatorCalculator.h"
#include "MultiPropertyTemporalInterpolator.h"
#include "SourceRocksTemporalInterpolator.h"


/// \brief Class for solving the multi-component multi-phase flow equations.
///
/// These are solved using an explicit method.
class ExplicitMultiComponentFlowSolver {

   typedef ConstrainedArray < bool, 0, Basin_Modelling::NumberOfFundamentalProperties - 1 > ConstrainedBooleanArray;

   typedef std::vector<ConstrainedBooleanArray> ConstrainedBooleanArrayMap;

   typedef DarcyCalculations::CompositionArray CompositionArray;

   typedef DarcyCalculations::PhaseCompositionArray PhaseCompositionArray;

   typedef DarcyCalculations::PhaseValueArray PhaseValueArray;

   typedef DarcyCalculations::SaturationArray SaturationArray;

   typedef PETSc_Local_3D_Array <double> ScalarArray;


   typedef PetscBlockVector<ElementFaceValues>  ElementFaceValueVector;

   typedef PetscBlockVector<double>  ScalarPetscVector;


   typedef DarcyCalculations::CompositionPetscVector CompositionPetscVector;

   typedef DarcyCalculations::SaturationPetscVector SaturationPetscVector;



   typedef MultiPropertyTemporalInterpolator<PoreVolumeInterpolatorCalculator> PoreVolumeTemporalInterpolator;

   typedef MultiPropertyTemporalInterpolator<FaceAreaInterpolatorCalculator> FaceAreaTemporalInterpolator;

   typedef MultiPropertyTemporalInterpolator<FacePermeabilityInterpolatorCalculator> FacePermeabilityTemporalInterpolator;


   static const unsigned int PoreVolumeIndex = PoreVolumeInterpolatorCalculator::PoreVolumeIndex;

   static const unsigned int RockCompressionIndex = PoreVolumeInterpolatorCalculator::RockCompressionIndex;

public :

   /// Constructor.
   ExplicitMultiComponentFlowSolver ();

   /// Destructor.
   ~ExplicitMultiComponentFlowSolver ();

   /// \brief Solve for the concentrations over the interval from start-time until end-time.
   ///
   /// The time interval must correspond to a P/T time-step.
   void solve ( Subdomain& subdomain,
                const double startTime,
                const double endTime,
                DarcyErrorIndicator& errorOccurred );


private :


   void computePressure ( FormationSubdomainElementGrid&      formationGrid,
                          const SaturationArray&              saturations,
                          const TemporalPropertyInterpolator& porePressure,
                          const TemporalPropertyInterpolator& temperature,
                          const TemporalPropertyInterpolator& ves,
                          const TemporalPropertyInterpolator& maxVes,
                          const double                        lambda,
                          ScalarPetscVector&                  vapourPressure,
                          ScalarPetscVector&                  liquidPressure );

   void computePressure ( Subdomain&                          subdomain,
                          const SaturationArray&              saturations,
                          const TemporalPropertyInterpolator& porePressure,
                          const TemporalPropertyInterpolator& temperature,
                          const TemporalPropertyInterpolator& ves,
                          const TemporalPropertyInterpolator& maxVes,
                          const double                        lambda,
                          ScalarPetscVector&                  vapourPressure,
                          ScalarPetscVector&                  liquidPressure );

   void collectGlobalSaturation ( FormationSubdomainElementGrid& formationGrid,
                                  SaturationPetscVector&         averagedSaturations,
                                  ScalarPetscVector&             divisor );

   void averageGlobalSaturation ( Subdomain&             subdomain,
                                  SaturationPetscVector& averagedSaturations,
                                  ScalarPetscVector&     divisor );

   void assignGlobalSaturation ( Subdomain&             subdomain,
                                 SaturationPetscVector& averagedSaturations );

   void averageGlobalSaturation ( Subdomain& subdomain );


   // Transport components for en element.
   void transportComponents ( const SubdomainElement&       element,
                              const pvtFlash::PVTPhase      phase,
                              const ElementFaceValueVector& elementFluxes,
                              const PhaseCompositionArray&  phaseComposition,
                                    PVTComponents&          computedConcentrations,
                                    double&                 transportedMassesIn,
                                    double&                 transportedMassesOut );

   /// \brief Transport the composition through the subdomain.
   void transportComponents ( FormationSubdomainElementGrid&   formationGrid,
                              const ElementVolumeGrid&         concentrationGrid,
                              const PhaseCompositionArray&     phaseComposition,
                              const ElementFaceValueVector&    gasFluxes,
                              const ElementFaceValueVector&    oilFluxes,
                              Boolean3DArray&                  elementContainsHc,
                              CompositionArray&                computedConcentrations,
                              ScalarArray&                     transportedMasses );

   /// \brief Transport the composition through the subdomain.
   void transportComponents ( Subdomain&                       subdomain,
                              const PhaseCompositionArray&     phaseComposition,
                              const ElementFaceValueVector&    gasFluxes,
                              const ElementFaceValueVector&    oilFluxes,
                              Boolean3DArray&                  elementContainsHc,
                              CompositionArray&                computedConcentrations,
                              ScalarArray&                     transportedMasses );



   /// Compute numerical flux function across face.
   void computeNumericalFlux ( const SubdomainElement&   element,
                               const pvtFlash::PVTPhase  phase,
                               const double              elementFlux,
                               const double              neighbourFlux,
                               const double              elementPhaseCompositionSum,
                               const PVTPhaseComponents& elementComposition,
                               const PVTPhaseComponents& neighbourComposition,
                                     PVTComponents&      flux,
                                     double&             transportedMassesIn,
                                     double&             transportedMassesOut );

   /// \brief Compute the flux for a face of the element.
   double computeElementFaceFlux ( const SubdomainElement&                   element, 
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
                                   const bool                                print ) const;

   void computeFluxForPhase ( const pvtFlash::PVTPhase                  phase,
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
                                    ElementFaceValues&                  elementFlux );

   /// \brief Compute the flux term for all elements in formation.
   void computeFluxTerms ( FormationSubdomainElementGrid&      formationGrid,
                           const Boolean3DArray&               elementContainsHc,
                           const PhaseCompositionArray&        phaseComposition,
                           const PhaseValueArray&              phaseDensities,
                           const PhaseValueArray&              phaseViscosities,
                           const ElementFaceValueVector&       subdomainPermeabilitiesN,
                           const ElementFaceValueVector&       subdomainPermeabilitiesH,
                           const FacePermeabilityTemporalInterpolator& permeabilityInterpolator,
                           const ScalarPetscVector&            subdomainVapourPressure,
                           const ScalarPetscVector&            subdomainliquidPressure,
                           const TemporalPropertyInterpolator& depth,
                           const TemporalPropertyInterpolator& porePressure,
                           const FaceAreaTemporalInterpolator& faceAreaInterpolator,
                           const PoreVolumeTemporalInterpolator& poreVolumeInterpolator,
                           const SaturationArray&              saturations,
                                 ElementFaceValueVector&       gasFluxes,
                                 ElementFaceValueVector&       oilFluxes,
                           const double                        lambda,
                                 double&                       calculatedTimeStep );

   /// \brief Compute the flux term for all elements in subdomain.
   void computeFluxTerms ( Subdomain&                          subdomain,
                           const Boolean3DArray&               elementContainsHc,
                           const PhaseCompositionArray&        phaseComposition,
                           const PhaseValueArray&              phaseDensities,
                           const PhaseValueArray&              phaseViscosities,
                           const ElementFaceValueVector&       subdomainPermeabilitiesN,
                           const ElementFaceValueVector&       subdomainPermeabilitiesH,
                           const FacePermeabilityTemporalInterpolator& permeabilityInterpolator,
                           const ScalarPetscVector&            subdomainCurrentPressure,
                           const ScalarPetscVector&            subdomainPreviousPressure,
                           const TemporalPropertyInterpolator& depth,
                           const TemporalPropertyInterpolator& porePressure,
                           const FaceAreaTemporalInterpolator& faceAreaInterpolator,
                           const PoreVolumeTemporalInterpolator& poreVolumeInterpolator,
                           const SaturationArray&              saturations,
                                 ElementFaceValueVector&       gasFluxes,
                                 ElementFaceValueVector&       oilFluxes,
                           const double                        lambda,
                                 double&                       calculatedTimeStep );


   /// \brief Scale the flux terms in a formation by the time-step size.
   void scaleFluxTermsByTimeStep ( FormationSubdomainElementGrid& formationGrid,
                                   const Boolean3DArray&          elementContainsHc,
                                   ElementFaceValueVector&        vapourFluxes,
                                   ElementFaceValueVector&        liquidFluxes,
                                   const double                   deltaTSec );

   /// \brief Scale the flux terms in the subdomain by the time-step size.
   void scaleFluxTermsByTimeStep ( Subdomain&              subdomain,
                                   const Boolean3DArray&   elementContainsHc,
                                   ElementFaceValueVector& vapourFluxes,
                                   ElementFaceValueVector& liquidFluxes,
                                   const double            deltaTSec );

   /// \brief Compute contributions from previous time-step for the element.
   void computeTemporalContributions ( const SubdomainElement&       element,
                                       const Boolean3DArray&         elementContainsHc,
                                       const CompositionPetscVector& layerConcentration,
                                       PVTComponents&                previousTerm,
                                       const PoreVolumeTemporalInterpolator& poreVolumeInterpolator,
                                       const double                  lambdaStart,
                                       const double                  lambda ) const;

   /// \brief Compute contributions from previous time-step for the source-rock-layer.
   void computeTemporalContributions ( FormationSubdomainElementGrid& formationGrid,
                                       const Boolean3DArray&          elementContainsHc,
                                       const ElementVolumeGrid&       concentrationGrid,
                                       CompositionArray&              previousTerm,
                                       const PoreVolumeTemporalInterpolator& poreVolumeInterpolator,
                                       const double                   lambdaStart,
                                       const double                   lambda );

   /// \brief Compute contributions from previous time-step for the subdomain.
   void computeTemporalContributions ( Subdomain&        subdomain,
                                       const Boolean3DArray& elementContainsHc,
                                       CompositionArray& previousTerm,
                                       const PoreVolumeTemporalInterpolator& poreVolumeInterpolator,
                                       const double      lambdaStart,
                                       const double      lambda );


   /// \brief The mass matrix is, for this problem, a 1x1 matrix.
   double computeElementMassMatrix ( const SubdomainElement& element,
                                     const double            lambdaStart,
                                     const double            lambdaEnd ) const;

   void divideByMassMatrix ( FormationSubdomainElementGrid& formationGrid,
                             const Boolean3DArray&          elementContainsHc,
                             const ElementVolumeGrid&       concentrationGrid,
                             const PoreVolumeTemporalInterpolator& poreVolumeInterpolator,
                             CompositionArray&              sourceTerm,
                             const double                   lambdaStart,
                             const double                   lambdaEnd );

   void divideByMassMatrix ( Subdomain&                            subdomain,
                             const Boolean3DArray&                 elementContainsHc,
                             const PoreVolumeTemporalInterpolator& poreVolumeInterpolator,
                             CompositionArray& sourceTerm,
                             const double      lambdaStart,
                             const double      lambdaEnd );

   /// \brief Update the vector containing the masses that have been transported since the vector was zeroed.
   void updateTransportedMasses ( FormationSubdomainElementGrid& formationGrid, 
                                  const ScalarArray& transportedMasses );

   /// \brief Update the vector containing the masses that have been transported since the vector was zeroed.
   void updateTransportedMasses ( Subdomain& subdomain, 
                                  const ScalarArray& transportedMasses );


   /// \brief Activate any properties that are required for the mcf-calculation.
   void activateProperties ( Subdomain&                  subdomain,
                             ConstrainedBooleanArrayMap& currentAlreadyActivatedProperties,
                             ConstrainedBooleanArrayMap& previousAlreadyActivatedProperties );

   /// \brief De-activate any properties that were activated for the mcf-calculation.
   void deactivateProperties ( Subdomain&                        subdomain,
                               const ConstrainedBooleanArrayMap& currentAlreadyActivatedProperties,
                               const ConstrainedBooleanArrayMap& previousAlreadyActivatedProperties );

   /// \brief Estimate the saturation for a single phase.
   void estimateSaturation ( const SubdomainElement& element,
                             const PVTComponents&    compostionn,
                                   bool&             elementContainsHc,
                                   double&           estimatedSaturation );

   /// \brief Estimate if transport of hydrocarbon will occur.
   void estimateHcTransport ( FormationSubdomainElementGrid& formationGrid,
                              Boolean3DArray&                elementContainsHc,
                              Boolean3DArray&                elementTransportsHc );

   /// \brief Estimate if transport of hydrocarbon will occur.
   ///
   /// If enabled use the estimated saturation to determine whether or not transport will occur.
   /// Otherwise assume that transport is always possible is hc content > epsilon.
   void estimateHcTransport ( Subdomain&      subdomain,
                              Boolean3DArray& elementContainsHc,
                              Boolean3DArray& elementTransportsHc );


   double m_maximumHCFractionForFlux;

   /// \brief The maximum time-step size for the simulation.
   double m_maximumTimeStepSize;

   /// Default molar masses, kg/mol.
   PVTComponents m_defaultMolarMasses;

   /// \brief Time spent performing otgc calculations.
   WallTime::Duration m_otgcTime;

   /// \brief Time spent performing flash calculations.
   WallTime::Duration m_flashTime;

   /// \brief Time spent computing fluxes.
   WallTime::Duration m_fluxTime;

   /// \brief Time spent computing pressure.
   WallTime::Duration m_pressureTime;

   /// \brief Time spent computing average permeabilities.
   WallTime::Duration m_permeabilityTime;

   /// \brief Time spent computing the term from the previous time.
   WallTime::Duration m_previousTime;

   /// \brief Time spent computing the source-term.
   WallTime::Duration m_sourceTime;

   /// \brief Time spent transporting composition.
   WallTime::Duration m_transportTime;

   /// \brief Time spent computing the mass matrix.
   WallTime::Duration m_massTime;

   /// \brief Time spent computing the concentrations.
   WallTime::Duration m_concTime;

   /// \brief Time spent computing the saturations.
   WallTime::Duration m_satTime;

   /// \brief Time spent computing the inner saturations.
   WallTime::Duration m_sat2Time;

   /// \brief Time spent computing the estimated saturations.
   WallTime::Duration m_estimatedSaturationTime;

   /// \brief Total time spent in the flow solver.
   WallTime::Duration m_totalTime;

   /// \brief Degree of quadrature for face fluxes.
   int m_faceQuadratureDegree;

   /// \brief Degree of quadrature for contibutions from previous time-step.
   int m_previousContributionsQuadratureDegree;

   /// \brief Degree of quadrature for mass-matrix.
   int m_massMatrixQuadratureDegree;

   /// \brief Indicates whether or not the grad-pressure should be limited.
   bool m_limitGradPressure;

   /// \brief If grad-pressure is to be limited then this is that limit.
   double m_gradPressureMaximum;

   /// \brief Indicates whether or not the permeability should be limited.
   bool m_limitFluxPermeability;

   /// \brief If permeability is to be limited then this is that limit.
   double m_fluxPermeabilityMaximum;

   /// \brief Indicates whether or not the capillary pressure is included in the Darcy flux calculation. 
   bool m_includeCapillaryPressure;

   /// \brief Indicates whether or not the immobile species should reduce the pore-space.
   bool m_useImmobileSaturation;

   int  m_timeStepSubSample;
   bool m_timeStepSubSampleOtgc;
   bool m_timeStepSubSamplePvt;
   bool m_timeStepSubSampleFlux;

   /// \brief Indicate whether or not the permeability across each face should be interpolated or the value computed.
   bool m_interpolateFacePermeability;

   /// \brief Indicate whether or not the pore-volume terms should be interpolated or the values computed.
   bool m_interpolatePoreVolume;

   /// \brief Indicate whether or not part of the flux calculation should be interpolated or the values computed.
   bool m_interpolateFaceArea;
   
   /// \brief Use the estimate of the saturation in order to initiate flash and transport.
   ///
   /// If the estimated saturation is less than the Sor then the flash will not be called
   /// and there will be no subsequent transport.
   bool m_useSaturationEstimate;

   /// \brief The scaling of the Sor when estimating whether or not transport will occur.
   double m_residualHcSaturationScaling;

   int m_flashCount;
   int m_transportInCount;
   int m_transportOutCount;
   int m_transportCount;

};


#endif // FASTCAULDRON__EXPLICIT_MULTI_COMPONENT_FLOW_SOLVER__H

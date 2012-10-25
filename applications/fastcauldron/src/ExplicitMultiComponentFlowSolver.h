#ifndef _FASTCAULDRON__EXPLICIT_MULTI_COMPONENT_FLOW_SOLVER__H_
#define _FASTCAULDRON__EXPLICIT_MULTI_COMPONENT_FLOW_SOLVER__H_

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
#include "globaldefs.h"

#include "property_manager.h"

#include "Simulator.h"
// #include "OTGC_kernel/src/Simulator.h"

#include "ImmobileSpeciesValues.h"


#include "TemporalPropertyInterpolator.h"


/// \brief Class for solving the multi-component multi-phase flow equations.
///
/// These are solved using an explicit method.
class ExplicitMultiComponentFlowSolver {

   typedef ConstrainedArray < bool, 0, Basin_Modelling::Number_Of_Fundamental_Properties - 1 > ConstrainedBooleanArray;

   typedef std::vector<ConstrainedBooleanArray> ConstrainedBooleanArrayMap;

   typedef PETSc_Local_3D_Array <PVTComponents> CompositionArray;

   typedef PETSc_Local_3D_Array <PVTPhaseComponents>  PhaseCompositionArray;

   typedef PETSc_Local_3D_Array <PVTPhaseValues> PhaseValueArray;

   typedef PETSc_Local_3D_Array <Saturation> SaturationArray;

   typedef PETSc_Local_3D_Array <double> ScalarArray;

   typedef PETSc_Local_3D_Array <ElementFaceValues>  ElementFaceValueArray;



   /// \brief A petsc-array of three-vectors.
   typedef PetscBlockVector<FiniteElementMethod::ThreeVector> ThreeVectorArray;

   typedef PetscBlockVector<ElementFaceValues>  ElementFaceValueVector;

   typedef PetscBlockVector<Saturation>  SaturationVector;

   typedef PetscBlockVector<double>  ScalarPetscVector;


   typedef PetscBlockVector<PVTComponents> CompositionPetscVector;

   typedef PetscBlockVector<ImmobileSpeciesValues> ImmobileSpeciesPetscVector;

public :

   /// Constructor.
   ExplicitMultiComponentFlowSolver ();

   /// Destructor.
   ~ExplicitMultiComponentFlowSolver ();

   /// \brief Solve for the concentrations at the current-time.
   void solve ( Subdomain&   subdomain,
                const double startTime,
                const double endTime,
                DarcyErrorIndicator& errorOccurred );


private :

   /// The lower limit of cnocentration for which any calculation is performed.
   static const double ConcentrationLowerLimit;

   void computePressure ( FormationSubdomainElementGrid&      formationGrid,
                          const PhaseCompositionArray&        phaseComposition,
                          const PhaseValueArray&              phaseDensities,
                          const SaturationArray&              saturations,
                          const TemporalPropertyInterpolator& porePressure,
                          const TemporalPropertyInterpolator& temperature,
                          const TemporalPropertyInterpolator& ves,
                          const TemporalPropertyInterpolator& maxVes,
                          const double                        lambda,
                          ScalarPetscVector&                  vapourPressure,
                          ScalarPetscVector&                  liquidPressure );

   void computePressure ( Subdomain&                          subdomain,
                          const PhaseCompositionArray&        phaseComposition,
                          const PhaseValueArray&              phaseDensities,
                          const SaturationArray&              saturations,
                          const TemporalPropertyInterpolator& porePressure,
                          const TemporalPropertyInterpolator& temperature,
                          const TemporalPropertyInterpolator& ves,
                          const TemporalPropertyInterpolator& maxVes,
                          const double                        lambda,
                          ScalarPetscVector&                  vapourPressure,
                          ScalarPetscVector&                  liquidPressure );

   void collectGlobalSaturation ( FormationSubdomainElementGrid& formationGrid,
                                  SaturationVector&              averagedSaturations,
                                  ScalarPetscVector&             divisor );

   void averageGlobalSaturation ( Subdomain&         subdomain,
                                  SaturationVector&  averagedSaturations,
                                  ScalarPetscVector& divisor );

   void assignGlobalSaturation ( Subdomain&         subdomain,
                                 SaturationVector& averagedSaturations );

   void averageGlobalSaturation ( Subdomain& subdomain );


   void averageComponents ( PVTComponents&      unitMasses,
                            PVTPhaseComponents& phaseMasses,
                            PVTPhaseValues&     density,
                            PVTPhaseValues&     viscosity );


   /// \brief Collect the permeabilities from the face of each element.
   void collectElementPermeabilities ( const Subdomain&         subdomain,
                                       const ElementVolumeGrid& elementGrid,
                                       const double             lambda,
                                       ElementFaceValueVector&  subdomainPermeabilityN, 
                                       ElementFaceValueVector&  subdomainPermeabilityH ) const;

   /// \brief Add permeability from the adjacent face of the neighbouring element.
   void addNeighbourPermeabilities ( const Subdomain&         subdomain,
                                     const ElementVolumeGrid& elementGrid,
                                     ElementFaceValueVector&  subdomainPermeabilityN, 
                                     ElementFaceValueVector&  subdomainPermeabilityH,
                                     ElementFaceValueArray&   intermediatePermeabilityN,
                                     ElementFaceValueArray&   intermediatePermeabilityH ) const;

   /// \brief From the values stored in the arrays compute the average permeability for the element-face.
   void recoverAveragedPermeabilities ( const Subdomain&         subdomain,
                                        const ElementVolumeGrid& elementGrid,
                                        ElementFaceValueVector&  subdomainPermeabilityN, 
                                        ElementFaceValueVector&  subdomainPermeabilityH,
                                        ElementFaceValueArray&   intermediatePermeabilityN,
                                        ElementFaceValueArray&   intermediatePermeabilityH ) const;

   /// \brief Average the permeability across the face of the element.
   void computeAveragePermeabilities ( const Subdomain& subdomain,
                                       const double     lambda,
                                       Vec subdomainPermeabilityNVec, 
                                       Vec subdomainPermeabilityHVec ) const;


   void getAveragedSaturationCoefficients ( const SubdomainElement&                   element, 
                                            const SaturationVector&                   layerAveragedSaturations,
                                                  FiniteElementMethod::ElementVector& vapourSaturationCoefficients, 
                                                  FiniteElementMethod::ElementVector& liquidSaturationCoefficients,
                                            const bool                                printIt );

   /// \brief Apply otgc simulator to the hc that remains in the pore-space for the duration of the time-step.
   void applyOtgc ( SubdomainElement&              element,
                    PVTComponents&                 concentration,
                    ImmobileSpeciesValues&         immobiles,
                    const double                   subTimeStepStart,
                    const double                   subTimeStepEnd,
                    const double                   lambdaStart,
                    const double                   lambdaEnd );

   /// \brief Apply otgc simulator to the hc that remains in the pore-space for the duration of the time-step.
   void applyOtgc ( FormationSubdomainElementGrid& formationGrid,
                    const double                   subTimeStepStart,
                    const double                   subTimeStepEnd,
                    const double                   lambdaStart,
                    const double                   lambdaEnd );

   /// \brief Apply otgc simulator to the hc that remains in the pore-space for the duration of the time-step.
   void applyOtgc ( Subdomain&   subdomain, 
                    const double subTimeStepStart,
                    const double subTimeStepEnd,
                    const double lambdaStart,
                    const double lambdaEnd );


   /// Compute numerical flux function across face.
   void computeNumericalFlux ( const SubdomainElement& element,
                               const double            elementFlux,
                               const double            neighbourFlux,
                               const PVTComponents&    elementComposition,
                               const PVTComponents&    neighbourComposition,
                                     PVTComponents&    flux );

   // Transport components for en element.
   void transportComponents ( const SubdomainElement&       element,
                              const pvtFlash::PVTPhase      phase,
                              const ElementFaceValueVector& elementFluxes,
                              const PhaseCompositionArray&  phaseComposition,
                                    PVTComponents&          computedConcentrations );

   /// \brief Transport the composition through the subdomain.
   void transportComponents ( FormationSubdomainElementGrid&   formationGrid,
                              const ElementVolumeGrid&         concentrationGrid,
                              const PhaseCompositionArray&     phaseComposition,
                              const ElementFaceValueVector&    gasFluxes,
                              const ElementFaceValueVector&    oilFluxes,
                              CompositionArray&                computedConcentrations );

   /// \brief Transport the composition through the subdomain.
   void transportComponents ( Subdomain&                       subdomain,
                              const PhaseCompositionArray&     phaseComposition,
                              const ElementFaceValueVector&    gasFluxes,
                              const ElementFaceValueVector&    oilFluxes,
                              CompositionArray&                computedConcentrations );



   /// \brief Compute the flux for a face of the element.
   double computeElementFaceFlux ( const SubdomainElement&                   element, 
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
                                   const bool                                print ) const;

   /// \brief Compute the flux term for all elements in formation.
   void computeFluxTerms ( FormationSubdomainElementGrid&      formationGrid,
                           const PhaseCompositionArray&        phaseComposition,
                           const PhaseValueArray&              phaseDensities,
                           const PhaseValueArray&              phaseViscosities,
                           const ElementFaceValueVector&       subdomainPermeabilitiesN,
                           const ElementFaceValueVector&       subdomainPermeabilitiesH,
                           const ScalarPetscVector&            subdomainVapourPressure,
                           const ScalarPetscVector&            subdomainliquidPressure,
                           const TemporalPropertyInterpolator& depth,
                           const TemporalPropertyInterpolator& porePressure,
                           const SaturationArray&              saturations,
                                 ElementFaceValueVector&       gasFluxes,
                                 ElementFaceValueVector&       oilFluxes,
                           const double                        lambda,
                           const double                        deltaT );

   /// \brief Compute the flux term for all elements in subdomain.
   void computeFluxTerms ( Subdomain&                          subdomain,
                           const PhaseCompositionArray&        phaseComposition,
                           const PhaseValueArray&              phaseDensities,
                           const PhaseValueArray&              phaseViscosities,
                           const ElementFaceValueVector&       subdomainPermeabilitiesN,
                           const ElementFaceValueVector&       subdomainPermeabilitiesH,
                           const ScalarPetscVector&            subdomainCurrentPressure,
                           const ScalarPetscVector&            subdomainPreviousPressure,
                           const TemporalPropertyInterpolator& depth,
                           const TemporalPropertyInterpolator& porePressure,
                           const SaturationArray&              saturations,
                                 ElementFaceValueVector&       gasFluxes,
                                 ElementFaceValueVector&       oilFluxes,
                           const double                        lambda,
                           const double                        deltaT );


   /// \brief Flash the components in the layer.
   void flashComponents ( FormationSubdomainElementGrid& formationGrid,
                          const ElementVolumeGrid&       concentrationGrid,
                          PhaseCompositionArray&         phaseComposition,
                          PhaseValueArray&               phaseDensities,
                          PhaseValueArray&               phaseViscosities,
                          const TemporalPropertyInterpolator& porePressure,
                          const TemporalPropertyInterpolator& temperature,
                          const double                   lambda );

   /// \brief Flash the components for the subdomain.
   void flashComponents ( Subdomain&             subdomain,
                          PhaseCompositionArray& phaseComposition,
                          PhaseValueArray&       phaseDensities,
                          PhaseValueArray&       phaseViscosities,
                          const TemporalPropertyInterpolator& porePressure,
                          const TemporalPropertyInterpolator& temperature,
                          const double           lambda );



   /// \brief Compute contributions from previous time-step for the element.
   void computeTemporalContributions ( const SubdomainElement&       element,
                                       const CompositionPetscVector& layerConcentration,
                                       PVTComponents&                previousTerm,
                                       const double                  lambdaStart,
                                       const double                  lambda,
                                       const bool                    print ) const;

   /// \brief Compute contributions from previous time-step for the source-rock-layer.
   void computeTemporalContributions ( FormationSubdomainElementGrid& formationGrid,
                                       const ElementVolumeGrid&       concentrationGrid,
                                       CompositionArray&              previousTerm,
                                       const double                   lambdaStart,
                                       const double                   lambda );

   /// \brief Compute contributions from previous time-step for the subdomain.
   void computeTemporalContributions ( Subdomain&        subdomain,
                                       CompositionArray& previousTerm,
                                       const double      lambdaStart,
                                       const double      lambda );



   /// \brief Compute source term for the element.
   ///
   /// The source term includes the time-step multiplier already.
   /// Units are in kg/element.
   void computeSourceTerm ( const SubdomainElement& element,
                                  PVTComponents&    sourceTerm,
                            const double            fractionScaling,
                            const bool              print ) const;

   /// \brief Compute source term for the source-rock-layer.
   void computeSourceTerm ( FormationSubdomainElementGrid& formationGrid,
                            const ElementVolumeGrid&       concentrationGrid,
                            const double                   fractionScaling,
                            CompositionArray&              sourceTerm );

   /// \brief Compute source term for the subdomain.
   void computeSourceTerm ( Subdomain&        subdomain,
                            CompositionArray& sourceTerm,
                            const double      fractionScaling );


   /// \brief The mass matrix is, for this problem, a 1x1 matrix.
   double computeElementMassMatrix ( const SubdomainElement& element,
                                     const double            lambdaStart,
                                     const double            lambdaEnd ) const;

   void divideByMassMatrix ( FormationSubdomainElementGrid& formationGrid,
                             const ElementVolumeGrid&       concentrationGrid,
                             CompositionArray&              sourceTerm,
                             const double                   lambdaStart,
                             const double                   lambdaEnd );

   void divideByMassMatrix ( Subdomain&        subdomain,
                             CompositionArray& sourceTerm,
                             const double      lambdaStart,
                             const double      lambdaEnd );

   /// \brief Set the concentrations for a formation.
   void setConcentrations ( FormationSubdomainElementGrid& formationGrid,
                            const CompositionArray&        sourceTerm,
                            bool&                          errorInConcentration );

   /// \brief Set the concentrations for the subdomain.
   void setConcentrations ( Subdomain&              subdomain,
                            const CompositionArray& sourceTerm,
                            bool&                   errorInConcentration );

   /// \brief Set the saturations for a formation.
   void setSaturations ( FormationSubdomainElementGrid& formationGrid,
                         bool&                          errorInSaturation );

   /// \brief Set the saturations for the subdomain.
   void setSaturations ( Subdomain& subdomain,
                         bool&      errorInSaturation );

   /// \brief Set the time of element invasion, the time when a cell has non-zero concentration
   void setTimeOfElementInvasion ( FormationSubdomainElementGrid& formationGrid, double time );

   /// \brief Set the time of element invasion, the time when a cell has non-zero concentration
   void setTimeOfElementInvasion ( Subdomain& subdomain, double time );


   /// \brief Activate any properties that are required for the mcf-calculation.
   void activateProperties ( Subdomain&                  subdomain,
                             ConstrainedBooleanArrayMap& currentAlreadyActivatedProperties,
                             ConstrainedBooleanArrayMap& previousAlreadyActivatedProperties );

   /// \brief De-activate any properties that were activated for the mcf-calculation.
   void deactivateProperties ( Subdomain&                        subdomain,
                               const ConstrainedBooleanArrayMap& currentAlreadyActivatedProperties,
                               const ConstrainedBooleanArrayMap& previousAlreadyActivatedProperties );

   /// \brief Compute the CFL number of the formation.
   double computeCflNumber ( const FormationSubdomainElementGrid& formationGrid );

   /// \brief Compute the CFL number of the subdomain.
   double computeCflNumber ( const Subdomain& subdomain );

   /// \brief Compute vapour-, liquid- and immobile-saturation.
   void computeSaturation ( const SubdomainElement&        element,
                            const PVTComponents&           concentrations,
                            const ImmobileSpeciesValues&   immobiles,
                                  Saturation&              saturation,
                            const bool                     print );


   /// \brief Compute vapour-, liquid- and immobile-saturation.
   void computeSaturation ( const SubdomainElement&        element,
                            const PVTPhaseComponents&      phaseCompostion,
                            const PVTPhaseValues           density,
                            const ImmobileSpeciesValues&   immobiles,
                                  Saturation&              saturation,
                            const bool                     print );

   void setSaturations ( FormationSubdomainElementGrid& formationGrid,
                         const PhaseCompositionArray&   phaseComposition,
                         const PhaseValueArray&         phaseDensities,
                         SaturationArray&               saturations,
                         bool&                          errorInSaturation );

   void setSaturations ( Subdomain&                   subdomain,
                         const PhaseCompositionArray& phaseComposition,
                         const PhaseValueArray&       phaseDensities,
                         SaturationArray&             saturations,
                         bool&                        errorInSaturation );

   void updateProgress ( const int status ) const;

   void moveToStartOfline () const;


   Genex6::Simulator* m_otgcSimulator;

   double m_maximumHCFractionForFlux;

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

   /// \brief Total time spent in the flow solver.
   WallTime::Duration m_totalTime;

   /// \brief Degree of quadrature for face fluxes.
   int m_faceQuadratureDegree;

   /// \brief Degree of quadrature for contibutions from previous time-step.
   int m_previousContributionsQuadratureDegree;

   /// \brief Degree of quadrature for source-term.
   int m_sourceTermQuadratureDegree;

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


};


#endif // _FASTCAULDRON__EXPLICIT_MULTI_COMPONENT_FLOW_SOLVER__H_

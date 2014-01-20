#ifndef _FASTCAULDRON__DARCY_CALCULATIONS__H_
#define _FASTCAULDRON__DARCY_CALCULATIONS__H_

#include "petsc.h"
#include "petscmat.h"

#include "Simulator.h"

#include "Subdomain.h"
#include "SubdomainElement.h"
#include "FormationSubdomainElementGrid.h"

#include "TemporalPropertyInterpolator.h"
#include "SourceRocksTemporalInterpolator.h"

#include "PVTCalculator.h"
#include "ImmobileSpeciesValues.h"
#include "Saturation.h"
#include "ElementFaceValues.h"

#include "PetscBlockVector.h"
#include "ghost_array.h"

class DarcyCalculations {

public :

   /// \brief An array of PVTComponents that has bounds defined by the number of elements (including ghosts) defined for the process.
   /// \brief A petsc vector.
   typedef PetscBlockVector<PVTComponents> CompositionPetscVector;

   /// \brief An array of ImmobileSpeciesValues that has bounds defined by the number of elements (including ghosts) defined for the process.
   /// \brief A petsc vector.
   typedef PetscBlockVector<ImmobileSpeciesValues> ImmobileSpeciesPetscVector;

   /// \brief A petsc vector of Saturation values.
   ///
   /// Each entry of the petsc vector will contain a object of type Saturation.
   typedef PetscBlockVector<Saturation>  SaturationPetscVector;

   /// \brief An array of PVTPhaseComponents that has bounds defined by the number of elements (including ghosts) defined for the process.
   /// \brief A petsc vector.
   typedef PetscBlockVector<PVTPhaseValues> PhaseValuesPetscVector;



   /// \brief An array of PVTComponents that has bounds defined by the number of elements (including ghosts) defined for the process.
   typedef PETSc_Local_3D_Array <PVTComponents> CompositionArray;

   /// \brief An array of PVTPhaseComponents that has bounds defined by the number of elements (including ghosts) defined for the process.
   typedef PETSc_Local_3D_Array <PVTPhaseComponents>  PhaseCompositionArray;

   /// \brief An array of PVTPhaseValues that has bounds defined by the number of elements (including ghosts) defined for the process.
   typedef PETSc_Local_3D_Array <PVTPhaseValues> PhaseValueArray;

   /// \brief An array of ElementFaceValues that has bounds defined by the number of elements (including ghosts) defined for the process.
   typedef PETSc_Local_3D_Array <ElementFaceValues>  ElementFaceValueArray;

   /// \brief An array of Saturation that has bounds defined by the number of elements (including ghosts) defined for the process.
   typedef PETSc_Local_3D_Array <Saturation> SaturationArray;


   DarcyCalculations ();


   /// \brief Apply secondary cracking to the HC contained in the elements of the subdomain.
   ///
   /// otgc will be applied only if it has been selected in the project file.
   void applyOtgc ( Subdomain&                          subdomain,
                    const Boolean3DArray&               elementContainsHc,
                    const TemporalPropertyInterpolator& porePressure,
                    const TemporalPropertyInterpolator& temperature,
                    const double                        timeStepStart,
                    const double                        timeStepEnd,
                    const double                        lambdaStart,
                    const double                        lambdaEnd );

   /// \brief Compute source term for the subdomain.
   ///
   /// Add some fraction of what was expelled over the P/T interval.
   void computeSourceTerm ( Subdomain&        subdomain,
                            CompositionArray& sourceTerm,
                            Boolean3DArray&   elementContainsHc,
                            const SourceRocksTemporalInterpolator & sourceRockInterpolator,
                            const double      lambda,
                            const double      fractionScaling,
                                  double&     layerMassAdded );

   /// \brief Flash the components for the subdomain.
   ///
   /// Function returns the number of flashes performed for the mpi-process.
   int flashComponents ( Subdomain&             subdomain,
                         PhaseCompositionArray& phaseComposition,
                         PhaseValueArray&       phaseDensities,
                         PhaseValueArray&       phaseViscosities,
                         const TemporalPropertyInterpolator& porePressure,
                         const TemporalPropertyInterpolator& temperature,
                         const double           lambda,
                         CompositionArray&      kValues,
                         Boolean3DArray&        elementContainsHc );

   /// \brief Set the concentrations for the subdomain.
   ///
   /// Set the concentration values for each element in the layer.
   void setConcentrations ( Subdomain&              subdomain,
                            const Boolean3DArray&   elementContainsHc,
                            const CompositionArray& sourceTerm,
                            bool&                   errorInConcentration );

   /// \brief Set the final saturations for the subdomain.
   ///
   /// Set the saturation array contained within each layer.
   void setSaturations ( Subdomain&        subdomain,
                         CompositionArray& kValues,
                         bool&             errorInSaturation );

   /// \brief Set the saturations for the subdomain.
   void setSaturations ( Subdomain&                   subdomain,
                         const PhaseCompositionArray& phaseComposition,
                         const PhaseValueArray&       phaseDensities,
                         const Boolean3DArray&        elementContainsHc,
                         SaturationArray&             saturations,
                         bool&                        errorInSaturation );


   /// \brief Average the permeability across the face of the element.
   void computeAveragePermeabilities ( Subdomain&       subdomain,
                                       const double     lambda,
                                       const double     lambdaEnd,
                                       Vec subdomainPermeabilityNVec, 
                                       Vec subdomainPermeabilityHVec ) const;

   /// \brief Return the total mass of hydrocarbon within the system.
   double totalHcMass ( Subdomain&   subdomain,
                        const double lambda );

   /// \brief Set the time of element invasion, the time when a cell has non-zero hc-concentration
   void setTimeOfElementInvasion ( Subdomain&                          subdomain,
                                   const PhaseValueArray&              hcDensity,
                                   const TemporalPropertyInterpolator& porePressure,
                                   const TemporalPropertyInterpolator& temperature,
                                   const TemporalPropertyInterpolator& ves,
                                   const TemporalPropertyInterpolator& maxVes,
                                   const double                        time,
                                   const double                        lambda );

private :


   static const pvtFlash::PVTPhase RedundantPhase  = pvtFlash::VAPOUR_PHASE;
   static const pvtFlash::PVTPhase DesignatedPhase = pvtFlash::LIQUID_PHASE;

   /// \brief Initialise the simulator for secondary cracking.
   void initialiseOtgcSimulator ();

   /// \brief Initialise any internal state that can depend on command line parameters.
   void initialiseFromCommandLine ();

   /// \brief Performe averaging of phase-composition.
   ///
   /// Only used for debugging purposes.
   void averageComponents ( PVTComponents&      masses,
                            PVTPhaseComponents& phaseMasses,
                            PVTPhaseValues&     density,
                            PVTPhaseValues&     viscosity ) const;

   /// \brief Apply otgc simulator to the hc that remains in the pore-space for the duration of the time-step.
   void applyOtgc ( SubdomainElement&                   element,
                    const Boolean3DArray&               elementContainsHc,
                    PVTComponents&                      concentration,
                    ImmobileSpeciesValues&              immobiles,
                    const TemporalPropertyInterpolator& porePressure,
                    const TemporalPropertyInterpolator& temperature,
                    const double                        timeStepStart,
                    const double                        timeStepEnd,
                    const double                        lambdaStart,
                    const double                        lambdaEnd );

   /// \brief Apply otgc simulator to the hc that remains in the pore-space for the duration of the time-step.
   void applyOtgc ( FormationSubdomainElementGrid&      formationGrid,
                    const Boolean3DArray&               elementContainsHc,
                    const TemporalPropertyInterpolator& porePressure,
                    const TemporalPropertyInterpolator& temperature,
                    const double                        timeStepStart,
                    const double                        timeStepEnd,
                    const double                        lambdaStart,
                    const double                        lambdaEnd );

   /// \brief Compute source term for the element.
   ///
   /// The source term includes the time-step multiplier already.
   /// Units are in kg/element.
   void computeSourceTerm ( const SubdomainElement& element,
                                  PVTComponents&    sourceTerm,
                            const double            lambda,
                            const double            fractionScaling,
                                  double&           layerMassAdded ) const;

   /// \brief Compute source term for the source-rock-layer.
   void computeSourceTerm ( FormationSubdomainElementGrid& formationGrid,
                            const ElementVolumeGrid&       concentrationGrid,
                            Boolean3DArray&                elementContainsHc,
                            const SourceTermTemporalInterpolator * sourceTermInterpolator,
                            const double                   lambda,
                            const double                   fractionScaling,
                            CompositionArray&              sourceTerm,
                                  double&                  layerMassAdded );

   /// \brief Flash the components for each element in the layer that contains HC.
   int flashComponents ( FormationSubdomainElementGrid&      formationGrid,
                         PhaseCompositionArray&              phaseComposition,
                         PhaseValueArray&                    phaseDensities,
                         PhaseValueArray&                    phaseViscosities,
                         const TemporalPropertyInterpolator& porePressure,
                         const TemporalPropertyInterpolator& temperature,
                         const double                        lambda,
                         CompositionArray&                   kValues,
                         Boolean3DArray&                     elementContainsHc );

   /// \brief Set the final saturations for a formation.
   void setSaturations ( FormationSubdomainElementGrid& formationGrid,
                         CompositionArray&              kValues,
                         bool&                          errorInSaturation );

   /// \brief Compute vapour-, liquid- and immobile-saturation.
   void computeSaturation ( const SubdomainElement&        element,
                            const PVTComponents&           concentrations,
                            const ImmobileSpeciesValues&   immobiles,
                                  Saturation&              saturation,
                                  PVTComponents&           kValues );

   /// \brief Compute vapour-, liquid- and immobile-saturation.
   void computeSaturation ( const SubdomainElement&        element,
                            const PVTPhaseComponents&      phaseCompostion,
                            const PVTPhaseValues           density,
                            const ImmobileSpeciesValues&   immobiles,
                                  Saturation&              saturation );

   /// \brief Set the saturations for the layer.
   void setSaturations ( FormationSubdomainElementGrid& formationGrid,
                         const PhaseCompositionArray&   phaseComposition,
                         const PhaseValueArray&         phaseDensities,
                         const Boolean3DArray&          elementContainsHc,
                         SaturationArray&               saturations,
                         bool&                          errorInSaturation );

   /// \brief Set the concentrations for a formation.
   void setConcentrations ( FormationSubdomainElementGrid& formationGrid,
                            const Boolean3DArray&          elementContainsHc,
                            const CompositionArray&        sourceTerm,
                            bool&                          errorInConcentration );

   /// \brief Return the toral mass of hydrocarbon within the layer.
   ///
   /// This returns the mass of hydro-carbon within elements of the formation that lie on the current processor.
   double totalLayerHcMass ( FormationSubdomainElementGrid& formationGrid,
                             const double                   lambda );

   /// \brief Collect the permeabilities from the face of each element.
   void collectElementPermeabilities ( Subdomain&               subdomain,
                                       const ElementVolumeGrid& elementGrid,
                                       const double             lambda,
                                       ElementFaceValueVector&  subdomainPermeabilityN, 
                                       ElementFaceValueVector&  subdomainPermeabilityH ) const;

   /// \brief Add permeability from the adjacent face of the neighbouring element.
   void addNeighbourPermeabilities ( Subdomain&               subdomain,
                                     const ElementVolumeGrid& elementGrid,
                                     ElementFaceValueVector&  subdomainPermeabilityN, 
                                     ElementFaceValueVector&  subdomainPermeabilityH,
                                     ElementFaceValueArray&   intermediatePermeabilityN,
                                     ElementFaceValueArray&   intermediatePermeabilityH ) const;

   /// \brief From the values stored in the arrays compute the average permeability for the element-face.
   void recoverAveragedPermeabilities ( Subdomain&               subdomain,
                                        const ElementVolumeGrid& elementGrid,
                                        ElementFaceValueVector&  subdomainPermeabilityN, 
                                        ElementFaceValueVector&  subdomainPermeabilityH,
                                        ElementFaceValueArray&   intermediatePermeabilityN,
                                        ElementFaceValueArray&   intermediatePermeabilityH,
                                        const double lambda ) const;


   /// \brief Set the time of element invasion, the time when a cell has non-zero concentration
   void setTimeOfElementInvasion ( FormationSubdomainElementGrid&      formationGrid,
                                   const PhaseValueArray&              hcDensity,
                                   const TemporalPropertyInterpolator& porePressure,
                                   const TemporalPropertyInterpolator& temperature,
                                   const TemporalPropertyInterpolator& ves,
                                   const TemporalPropertyInterpolator& maxVes,
                                   const double                        time,
                                   const double                        lambda );




   /// \brief The simulator for secondary cracking.
   Genex6::Simulator* m_otgcSimulator;

   /// \brief Degree of quadrature for source-term.
   int m_sourceTermQuadratureDegree;

   /// \brief Degree of quadrature for mass-matrix.
   int m_massMatrixQuadratureDegree;

   /// \brief Should the immobile saturations be used or not.
   bool m_useImmobileSaturation;

   /// \brief Indicate whether or not part of the flux source term should be interpolated or the values computed.
   bool m_interpolateSourceTerm;

   /// Default molar masses, kg/mol.
   PVTComponents m_defaultMolarMasses;

}; 

#endif // _FASTCAULDRON__DARCY_CALCULATIONS__H_

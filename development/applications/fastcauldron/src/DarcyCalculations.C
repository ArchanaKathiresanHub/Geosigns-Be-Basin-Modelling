//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

#include "DarcyCalculations.h"

#include "MpiFunctions.h"
#include "NumericFunctions.h"
#include "Quadrature.h"
#include "Quadrature3D.h"

// OTGC 
#include "SpeciesState.h"
#include "Species.h"
#include "OTGC_kernel6/src/SimulatorState.h"

#include "ElementContributions.h"
#include "ElementVolumeGrid.h"
#include "LayerElement.h"
#include "MultiComponentFlowHandler.h"

#include "Lithology.h"

// utilities
#include "ConstantsNumerical.h"
using Utilities::Numerical::CauldronNoDataValue;

//------------------------------------------------------------//

DarcyCalculations::DarcyCalculations () {

   m_defaultMolarMasses = PVTCalc::getInstance ().getMolarMass ();
   // Kg/mol
   m_defaultMolarMasses *= 1.0e-3;

   initialiseOtgcSimulator ();
   initialiseFromCommandLine ();
}

//------------------------------------------------------------//

void DarcyCalculations::initialiseOtgcSimulator () {

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

   if ( m_otgcSimulator != 0 ) {
      ImmobileSpeciesValues::setMappingToSpeciesManager ( m_otgcSimulator->getSpeciesManager() );
   }

}

//------------------------------------------------------------//

void DarcyCalculations::initialiseFromCommandLine () {

   m_sourceTermQuadratureDegree            = FastcauldronSimulator::getInstance ().getMcfHandler ().getSourceTermQuadratureDegree();
   m_massMatrixQuadratureDegree            = FastcauldronSimulator::getInstance ().getMcfHandler ().getMassMatrixQuadratureDegree ();

   m_useImmobileSaturation                 = FastcauldronSimulator::getInstance ().getMcfHandler ().useImmobileSaturation ();
   m_interpolateSourceTerm                 = FastcauldronSimulator::getInstance ().getMcfHandler ().getInterpolateSourceTerm ();
}

//------------------------------------------------------------//

void DarcyCalculations::averageComponents ( PVTComponents&      masses,
                                         PVTPhaseComponents& phaseMasses,
                                         PVTPhaseValues&     density,
                                         PVTPhaseValues&     viscosity ) const {

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

void DarcyCalculations::applyOtgc ( SubdomainElement&                   element,
                                    const Boolean3DArray&               elementContainsHc,
                                    PVTComponents&                      concentration,
                                    ImmobileSpeciesValues&              immobiles,
                                    const TemporalPropertyInterpolator& porePressure,
                                    const TemporalPropertyInterpolator& temperature,
                                    const double                        timeStepStart,
                                    const double                        timeStepEnd,
                                    const double                        lambdaStart,
                                    const double                        lambdaEnd ) {

   if ( not elementContainsHc ( element.getI (), element.getJ (), element.getK ())) {
      return;
   }

   const Genex6::SpeciesManager& speciesManager = m_otgcSimulator->getSpeciesManager ();
   const Genex6::Species** allSpecies = m_otgcSimulator->getSpeciesInChemicalModel ();

   double components[Genex6::SpeciesManager::numberOfSpecies];
   double concentrationSum;

   unsigned int species;

   double previousTemperature  = temperature ( element, lambdaStart );
   double currentTemperature   = temperature ( element, lambdaEnd );

   double previousPorePressure = NumericFunctions::Maximum ( 1.0e6 * porePressure ( element, lambdaStart ), 1.0e5 ); // Pascals
   double currentPorePressure  = NumericFunctions::Maximum ( 1.0e6 * porePressure ( element, lambdaEnd   ), 1.0e5 ); // Pascals;

   // Convert to kg/m^3
   concentration *= m_defaultMolarMasses;
   concentrationSum = concentration.sum () + immobiles.sum ();

   // Collect and normalise all modelled species for OTGC.
   //
   // First mobile ones.
   for ( species = 0; species < Genex6::SpeciesManager::numberOfSpecies; ++species ) {

      if ( allSpecies [ species ] != 0 ) {

         ComponentId id = speciesManager.mapIdToPvtComponents ( species + 1 );
         if ( id != ComponentId::UNKNOWN ) {
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

   OTGC6::SimulatorState otgcState ( timeStepStart, allSpecies, components );

   m_otgcSimulator->computeInterval ( otgcState,
                                      previousTemperature, currentTemperature,
                                      previousPorePressure, currentPorePressure,
                                      timeStepStart, timeStepEnd );
     
   otgcState.GetSpeciesStateConcentrations ( components );

   for ( species = 0; species < Genex6::SpeciesManager::numberOfSpecies; ++species ) {
      
      ComponentId id = speciesManager.mapIdToPvtComponents( species + 1 );

      if ( id != ComponentId::UNKNOWN ) {
         // Renormalise and convert back to mol/m^3.
         concentration ( id ) = components[species] * concentrationSum / m_defaultMolarMasses ( id );
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
 
   // Remove any COx and H2S generated by OTGC.
   concentration ( ComponentId::COX ) = 0.0;
   concentration ( ComponentId::H2S ) = 0.0;
}

//------------------------------------------------------------//

void DarcyCalculations::applyOtgc ( FormationSubdomainElementGrid&      formationGrid,
                                    const Boolean3DArray&               elementContainsHc,
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
               SubdomainElement& element = formationGrid.getElement ( i, j, k );
               PVTComponents& concentration = layerConcentrations ( k, j, i );
               ImmobileSpeciesValues& immobiles = layerImmobileComponents ( k, j, i );

               applyOtgc ( element, elementContainsHc, concentration, immobiles, porePressure, temperature, timeStepStart, timeStepEnd, lambdaStart, lambdaEnd );
            }

         }

      }

   }

}

//------------------------------------------------------------//

void DarcyCalculations::applyOtgc ( Subdomain&                          subdomain,
                                    const Boolean3DArray&               elementContainsHc,
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
      applyOtgc ( *iter, elementContainsHc, porePressure, temperature, timeStepStart, timeStepEnd, lambdaStart, lambdaEnd );
      ++iter;
   }

}

//------------------------------------------------------------//

void DarcyCalculations::computeSourceTerm ( const SubdomainElement& element,
                                                  PVTComponents&    sourceTerm,
                                            const double            lambda,
                                            const double            fractionScaling,
                                                  double&           elementMassAdded ) const {

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

         generated ( i )( ComponentId::COX ) = 0.0;
         generated ( i )( ComponentId::H2S ) = 0.0;

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

void DarcyCalculations::computeSourceTerm ( FormationSubdomainElementGrid& formationGrid,
                                            const ElementVolumeGrid&       concentrationGrid,
                                            Boolean3DArray&                elementContainsHc,
                                            const SourceTermTemporalInterpolator * sourceTermInterpolator,
                                            const double                   lambda,
                                            const double                   fractionScaling,
                                            CompositionArray&              sourceTerm,
                                            double&                  layerMassAdded ) {

   LayerProps& srLayer = formationGrid.getFormation ();

   int i;
   int j;
   int k;
   int l;
   bool genexRetrieved = srLayer.genexDataIsRetrieved ();

   double elementMassAdded;
   double elementComponentMassAdded;
   layerMassAdded = 0.0;

   if ( not genexRetrieved ) {
      srLayer.retrieveGenexData ();
   }

   for ( i = concentrationGrid.firstI (); i <= concentrationGrid.lastI (); ++i ) {

      for ( j = concentrationGrid.firstJ (); j <= concentrationGrid.lastJ (); ++j ) {

         if ( FastcauldronSimulator::getInstance ().getMapElement ( i, j ).isValid ()) {

            for ( k = formationGrid.firstK (); k <= formationGrid.lastK (); ++k ) {
               SubdomainElement& element = formationGrid.getElement ( i, j, k );

               if ( m_interpolateSourceTerm && sourceTermInterpolator != 0 ) {
                  PVTComponents& curSourceTerm = sourceTerm ( i, j, element.getK ());
                  elementMassAdded = 0.0;

                  for( l = 0; l < NumberOfPVTComponents; ++ l ) {     
                     ComponentId component = static_cast<ComponentId>( l );

                     elementComponentMassAdded = ( sourceTermInterpolator->access( element.getLayerElement() ).evaluate( l, lambda ) ) * fractionScaling;

                     // Cut off values that are less than zero.
                     // Genex can occasionally return negative values that are close to zero at the beginning of expulsion.
                     elementComponentMassAdded = NumericFunctions::Maximum ( 0.0, elementComponentMassAdded );
                     elementMassAdded += elementComponentMassAdded;
                     curSourceTerm ( component ) += elementComponentMassAdded;
                  }
               } else {
                  computeSourceTerm ( element, sourceTerm ( i, j, element.getK ()), lambda, fractionScaling, elementMassAdded );
               }
              // Update the elementContainsHc array if there was hc added from the source rock.
               if ( elementMassAdded > 0.0 ) {
                  elementContainsHc ( element.getI (), element.getJ (), element.getK ()) = true;
               }

               layerMassAdded += elementMassAdded;
            }

         } else {

            for ( k = formationGrid.firstK (); k <= formationGrid.lastK (); ++k ) {
               sourceTerm ( i, j, formationGrid.getElement ( i, j, k ).getK ()).zero ();
            }

         }

      }

   }
   if ( not genexRetrieved ) {
      // Restore back to original state. 
      srLayer.restoreGenexData ();
   }

}

//------------------------------------------------------------//

void DarcyCalculations::computeSourceTerm ( Subdomain&        subdomain,
                                            CompositionArray& sourceTerm,
                                            Boolean3DArray&   elementContainsHc,
                                            const SourceRocksTemporalInterpolator & sourceRocksInterpolator,
                                            const double      lambda,
                                            const double      fractionScaling,
                                                  double&     massAdded ) {

   Subdomain::ActiveSourceRockLayerIterator iter;

   subdomain.initialiseLayerIterator ( iter );

   double processorMassAdded = 0.0;
   double layerMassAdded = 0.0;

   while ( not iter.isDone ()) {
      const FormationSubdomainElementGrid& formationGrid = * iter;

      computeSourceTerm ( *iter, iter->getVolumeGrid ( NumberOfPVTComponents ), elementContainsHc, sourceRocksInterpolator.accessSourceRock( &formationGrid ), 
                          lambda, fractionScaling, sourceTerm, layerMassAdded );
      ++iter;
      processorMassAdded += layerMassAdded;
   }

   massAdded = MpiFunctions::Sum<double>( PETSC_COMM_WORLD, processorMassAdded );
}

//------------------------------------------------------------//

int DarcyCalculations::flashComponents ( FormationSubdomainElementGrid& formationGrid,
                                         PhaseCompositionArray&         phaseComposition,
                                         PhaseValueArray&               phaseDensities,
                                         PhaseValueArray&               phaseViscosities,
                                         const TemporalPropertyInterpolator& porePressure,
                                         const TemporalPropertyInterpolator& temperature,
                                         const double                        lambda,
                                         CompositionArray&                   kValues,
                                         Boolean3DArray&                     elementContainsHc ) {

   const ElementVolumeGrid& concentrationGrid = formationGrid.getVolumeGrid ( NumberOfPVTComponents );
   const ElementGrid& elementGrid = FastcauldronSimulator::getInstance ().getElementGrid ();

   LayerProps& theLayer = formationGrid.getFormation ();

   CompositionPetscVector concentrations;
   PVTComponents elementConcentrations;

   int i;
   int j;
   int k;

   int flashCount = 0;

   concentrations.setVector ( concentrationGrid, theLayer.getPreviousComponentVec (), INSERT_VALUES, true );

   for ( i = concentrationGrid.firstI ( true ); i <= concentrationGrid.lastI ( true ); ++i ) {

      for ( j = concentrationGrid.firstJ ( true ); j <= concentrationGrid.lastJ ( true ); ++j ) {

         if ( FastcauldronSimulator::getInstance ().getMapElement ( i, j ).isValid ()) {

            for ( k = formationGrid.firstK (); k <= formationGrid.lastK (); ++k ) {

               const SubdomainElement& element = formationGrid.getElement ( i, j, k );
               unsigned int elementK = element.getK ();

               if ( element.getLayerElement ().isActive () and elementContainsHc ( element.getI (), element.getJ (), element.getK ())) {

                  // mol/m^3.
                  elementConcentrations = concentrations ( k, j, i );

                  // Convert to kg/m^3
                  elementConcentrations *= m_defaultMolarMasses;

                  // Evluate both temperature and pore-pressure at the centre of the element.
                  // Flash the composition using the composition on the element and the
                  // temperature and pressure evaluated at the centre of the element,
                  // Units for temperature are converted to degK.
                  // Units for pressure are converted to Pa.
                  PVTCalc::getInstance ().compute ( temperature ( element, lambda ) + 273.15,
                                                    NumericFunctions::Maximum ( 1.0e6 * porePressure ( element, lambda ), 1.0e5 ),
                                                    elementConcentrations,
                                                    phaseComposition ( element.getI (), element.getJ (), element.getK ()),
                                                    phaseDensities ( element.getI (), element.getJ (), element.getK ()),
                                                    phaseViscosities ( element.getI (), element.getJ (), element.getK ()),
                                                    kValues ( element.getI (), element.getJ (), element.getK ()),
                                                    false, 0.0 );

                  // Convert to SI units.
                  phaseViscosities ( element.getI (), element.getJ (), element.getK ()) *= 0.001;

                  averageComponents ( elementConcentrations, 
                                      phaseComposition ( element.getI (), element.getJ (), element.getK ()),
                                      phaseDensities ( element.getI (), element.getJ (), element.getK ()),
                                      phaseViscosities ( element.getI (), element.getJ (), element.getK ()));

                  // Convert to mol/m^3
                  phaseComposition ( element.getI (), element.getJ (), element.getK ()) /= m_defaultMolarMasses;

                  ++flashCount;

               } else {
                  phaseComposition ( element.getI (), element.getJ (), element.getK ()).zero ();
                  // The values assigned here are the same as the ones assigned 
                  // in PVT when no composition of a particular phase is present.
                  phaseDensities   ( element.getI (), element.getJ (), element.getK ())( PhaseId::LIQUID ) = 1000.0;
                  phaseDensities   ( element.getI (), element.getJ (), element.getK ())( PhaseId::VAPOUR ) = 1000.0;
                  phaseViscosities ( element.getI (), element.getJ (), element.getK ())( PhaseId::LIQUID ) = 1.0;
                  phaseViscosities ( element.getI (), element.getJ (), element.getK ())( PhaseId::VAPOUR ) = 1.0;
               }

            }

         } else {

            for ( k = formationGrid.firstK (); k <= formationGrid.lastK (); ++k ) {
               const SubdomainElement& element = formationGrid.getElement ( i, j, k );

               phaseComposition ( element.getI (), element.getJ (), element.getK ()).zero ();
               phaseDensities   ( element.getI (), element.getJ (), element.getK ())( PhaseId::LIQUID ) = 1000.0;
               phaseDensities   ( element.getI (), element.getJ (), element.getK ())( PhaseId::VAPOUR ) = 1000.0;
               phaseViscosities ( element.getI (), element.getJ (), element.getK ())( PhaseId::LIQUID ) = 1.0;
               phaseViscosities ( element.getI (), element.getJ (), element.getK ())( PhaseId::VAPOUR ) = 1.0;
            }

         }

      }
      
   }

   concentrations.restoreVector ( NO_UPDATE );
   return flashCount;
}

//------------------------------------------------------------//

int DarcyCalculations::flashComponents ( Subdomain&             subdomain,
                                         PhaseCompositionArray& phaseComposition,
                                         PhaseValueArray&       phaseDensities,
                                         PhaseValueArray&       phaseViscosities,
                                         const TemporalPropertyInterpolator& porePressure,
                                         const TemporalPropertyInterpolator& temperature,
                                         const double           lambda,
                                         CompositionArray&      kValues,
                                         Boolean3DArray&        elementContainsHc ) {


   Subdomain::ActiveLayerIterator iter;
   int flashCount = 0;

   subdomain.initialiseLayerIterator ( iter );

   while ( not iter.isDone ()) {
      flashCount += flashComponents ( *iter,
                                      phaseComposition, phaseDensities, phaseViscosities,
                                      porePressure, temperature,
                                      lambda,
                                      kValues, elementContainsHc );
      ++iter;
   }

   return flashCount;
}

//------------------------------------------------------------//

void DarcyCalculations::setConcentrations ( FormationSubdomainElementGrid& formationGrid,
                                            const Boolean3DArray&          elementContainsHc,
                                            const CompositionArray&        computedConcentrations,
                                            bool&                          errorInConcentration ) {

   LayerProps& theLayer = formationGrid.getFormation ();

   CompositionPetscVector concentrations;

   concentrations.setVector ( theLayer.getVolumeGrid ( NumberOfPVTComponents ), theLayer.getPreviousComponentVec (), INSERT_VALUES );

   int i;
   int j;
   int k;

   errorInConcentration = false;

   for ( i = formationGrid.firstI (); i <= formationGrid.lastI (); ++i ) {

      for ( j = formationGrid.firstJ (); j <= formationGrid.lastJ (); ++j ) {

         if ( FastcauldronSimulator::getInstance ().getMapElement ( i, j ).isValid ()) {

            for ( k = formationGrid.firstK (); k <= formationGrid.lastK (); ++k ) {

               const SubdomainElement& element = formationGrid.getElement ( i, j, k );
               unsigned int elementK = element.getK ();

               if ( element.getLayerElement ().isActive () and elementContainsHc ( element.getI (), element.getJ (), element.getK ())) {

                  if ( minimum ( computedConcentrations ( i, j, elementK )) < 0.0 or not computedConcentrations ( i, j, elementK ).isFinite ()) {

                     std::stringstream buffer;

                     buffer << " Location of error: " << theLayer.getName () << "  "
                            << " layer position = { " << element.getI () << ", " << element.getJ () << ", " << element.getLayerElement ().getLocalKPosition () << "} " 
                            << " subdomain position = { " << element.getI () << ", " << element.getJ () << ", " << element.getK () << "} " 
                            << endl;
                     buffer << computedConcentrations ( i, j, elementK ).image () << endl;

                     cout << buffer.str () << flush;

                     errorInConcentration = true;
                     // allAreValid = false;
                  }


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
                     
                  concentrations ( k, j, i ) = computedConcentrations ( i, j, elementK );
                  // Convert to mol/m^3
                  concentrations ( k, j, i ) /= m_defaultMolarMasses;

               } else {
                  concentrations ( k, j, i ).zero ();
               }

            }

         } else {

            for ( k = formationGrid.firstK (); k <= formationGrid.lastK (); ++k ) {
               concentrations ( k, j, i ).zero ();
            }

         }

      }

   }

   concentrations.restoreVector ( UPDATE_EXCLUDING_GHOSTS );
   // Move communication to after all layers have been updated.
   // So a single communication can be done.
   // errorInConcentration = not successfulExecution ( allAreValid );
}

//------------------------------------------------------------//

void DarcyCalculations::setConcentrations ( Subdomain&              subdomain,
                                            const Boolean3DArray&   elementContainsHc,
                                            const CompositionArray& computedConcentrations,
                                            bool&                   errorInConcentration ) {

   Subdomain::ActiveLayerIterator iter;
   bool errorInConcentrationLocal;
   bool errorInLayerConcentration;

   subdomain.initialiseLayerIterator ( iter );
   errorInConcentrationLocal = false;
   errorInConcentration = false;

   while ( not iter.isDone ()) {
      setConcentrations ( *iter, elementContainsHc, computedConcentrations, errorInLayerConcentration );
      errorInConcentrationLocal = errorInConcentrationLocal or errorInLayerConcentration;
      ++iter;
   }

   int errorInConcentrationIntLocal = ( errorInConcentrationLocal ? 1 : 0 );
   int errorInConcentrationIntGlobal;


   errorInConcentrationIntGlobal = MpiFunctions::Maximum<int>( PETSC_COMM_WORLD, errorInConcentrationIntLocal );
   errorInConcentration = errorInConcentrationIntGlobal != 0;
   // errorInConcentration = not successfulExecution ( allAreValid );
}

//------------------------------------------------------------//

// Use the compute-saturation function that takes the already-flashed 
// composition as parameters after flashing. This will save on some code.
void DarcyCalculations::computeSaturation ( const SubdomainElement&        element,
                                            const PVTComponents&           concentrations,
                                            const ImmobileSpeciesValues&   immobiles,
                                                  Saturation&              saturation,
                                                  PVTComponents&           kValues ) {

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
                                                          viscosity.m_values,
                                                          false, 0.0,
                                                          kValues.m_components );
   
   // Correct viscosity units.
   viscosity *= 0.001;

   averageComponents ( unitMasses, phaseMasses, density, viscosity );

   sumGasMolarMassRatio = 0.0;
   sumOilMolarMassRatio = 0.0;

   for ( c = 0; c < NumberOfPVTComponents; ++c ) {
      ComponentId component = static_cast<ComponentId>( c );

      // First stage in calculation of phase-fractions.
      phaseFractions ( PhaseId::VAPOUR, component ) = phaseMasses ( PhaseId::VAPOUR, component ) / m_defaultMolarMasses ( component );
      phaseFractions ( PhaseId::LIQUID, component ) = phaseMasses ( PhaseId::LIQUID, component ) / m_defaultMolarMasses ( component );

      // mol/m^3
      sumGasMolarMassRatio += phaseMasses ( PhaseId::VAPOUR, component ) / m_defaultMolarMasses ( component );
      sumOilMolarMassRatio += phaseMasses ( PhaseId::LIQUID, component ) / m_defaultMolarMasses ( component );
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
         ComponentId component = static_cast<ComponentId>( c );

         // fractions (phase-component-moles per total-phase-component-moles).
         phaseFractions ( PhaseId::VAPOUR, component ) /= sumGasMolarMassRatio;
      }

   }

   if ( sumOilMolarMassRatio != 0.0 ) {

      for ( c = 0; c < NumberOfPVTComponents; ++c ) {
         ComponentId component = static_cast<ComponentId>( c );

         // fractions (phase-component-moles per total-phase-component-moles).
         phaseFractions ( PhaseId::LIQUID, component ) /= sumOilMolarMassRatio;
      }

   }

   for ( c = 0; c < NumberOfPVTComponents; ++c ) {
      ComponentId component = static_cast<ComponentId>( c );

      vapourMolarMass += phaseFractions ( PhaseId::VAPOUR, component ) * m_defaultMolarMasses ( component );
      liquidMolarMass += phaseFractions ( PhaseId::LIQUID, component ) * m_defaultMolarMasses ( component );
   }

   // Need to multiply by ratio of element-volume / element-pore-volume?
   if ( density ( PhaseId::VAPOUR ) != 1000.0 ) {
      vapourSaturation = concentrationSum * vapourFraction * vapourMolarMass / density ( PhaseId::VAPOUR );
   } else {
      vapourSaturation = 0.0;
   }

   if ( density ( PhaseId::LIQUID ) != 1000.0 ) {
      liquidSaturation = concentrationSum * ( 1.0 - vapourFraction ) * liquidMolarMass / density ( PhaseId::LIQUID );
   } else {
      liquidSaturation = 0.0;
   }

   if ( immobiles.sum () > HcConcentrationLowerLimit ) {
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

void DarcyCalculations::setSaturations ( FormationSubdomainElementGrid& formationGrid,
                                         CompositionArray&              kValues,
                                         bool&                          errorInSaturation ) {

   LayerProps& theLayer = formationGrid.getFormation ();

   CompositionPetscVector     concentrations;
   SaturationPetscVector      saturations;
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

               const SubdomainElement& element = formationGrid.getElement ( i, j, k );
               unsigned int elementK = element.getK ();

               if ( element.getLayerElement ().isActive ()) {

                  if ( concentrations ( k, j, i ).sum () > HcConcentrationLowerLimit ) {
                     computeSaturation ( element, concentrations ( k, j, i ), immobiles ( k, j, i ), saturations ( k, j, i ), kValues ( element.getI (), element.getJ (), element.getK ()));
                     elementSaturationIsFinite = saturations ( k, j, i ).isFinite ();
                     allAreFinite = allAreFinite and elementSaturationIsFinite;
                  } else {
                     saturations ( k, j, i ).initialise ();
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

   // Should be moved to after the saturation for all formations has been computed.
   // To reduce the number of synchronisations.
   errorInSaturation = not successfulExecution ( allAreFinite );
   concentrations.restoreVector ( UPDATE_EXCLUDING_GHOSTS );
   saturations.restoreVector ( UPDATE_EXCLUDING_GHOSTS );
}

//------------------------------------------------------------//

void DarcyCalculations::setSaturations ( Subdomain&        subdomain,
                                         CompositionArray& kValues,
                                         bool&             errorInSaturation ) {

   Subdomain::ActiveLayerIterator iter;
   bool errorInSaturationForLayer;

   subdomain.initialiseLayerIterator ( iter );
   errorInSaturation = false;

   while ( not iter.isDone ()) {
      setSaturations ( *iter, kValues, errorInSaturationForLayer );
      errorInSaturation = errorInSaturation or errorInSaturationForLayer;
      ++iter;
   }

}

//------------------------------------------------------------//

void DarcyCalculations::computeSaturation ( const SubdomainElement&        element,
                                            const PVTPhaseComponents&      phaseComposition,
                                            const PVTPhaseValues           density,
                                            const ImmobileSpeciesValues&   immobiles,
                                                  Saturation&              saturation ) {

   const double maximumHcSaturation = FastcauldronSimulator::getInstance ().getMaximumHcSaturation ();

   PVTPhaseValues     molarMass;
   double             vapourFraction;
   double             vapourCompositionSum = phaseComposition.sum ( PhaseId::VAPOUR );
   double             liquidCompositionSum = phaseComposition.sum ( PhaseId::LIQUID );
   double             concentrationSum = vapourCompositionSum + liquidCompositionSum;
   double             hcSaturation;
   double             vapourSaturation;
   double             liquidSaturation;
   double             immobileSaturation;
   int                c;

#if 0
   PVTComponents      totalComposition;
   double             totalSum;
   double             estimatedSaturation;
   double             phaseMolarMass = 0.0;
   double             phaseDensity = 200.0;

   totalComposition = phaseComposition.getPhaseComponents ( PhaseId::VAPOUR );
   totalComposition += phaseComposition.getPhaseComponents ( PhaseId::LIQUID );
   totalSum = totalComposition.sum ();

   if ( totalSum > 0.0 ) {

      for ( c = 0; c < NumberOfPVTComponents; ++c ) {
         ComponentId component = static_cast<ComponentId>( c );

         phaseMolarMass += totalComposition ( component ) * m_defaultMolarMasses ( component );
      }

      phaseMolarMass /= totalSum;
   }

   estimatedSaturation = totalSum * phaseMolarMass / phaseDensity;
#endif

   if ( concentrationSum != 0.0 ) {
      vapourFraction = vapourCompositionSum / concentrationSum;
   } else {
      vapourFraction = 0.0;
   }

   phaseComposition.sumProduct ( m_defaultMolarMasses, molarMass );

   // Need to multiply by ratio of element-volume / element-pore-volume?
   if ( vapourCompositionSum > HcConcentrationLowerLimit ) {
      vapourSaturation = concentrationSum * vapourFraction * molarMass ( PhaseId::VAPOUR ) / vapourCompositionSum / density ( PhaseId::VAPOUR );
   } else {
      vapourSaturation = 0.0;
   }

   if ( liquidCompositionSum > HcConcentrationLowerLimit ) {
      liquidSaturation = concentrationSum * ( 1.0 - vapourFraction ) * molarMass ( PhaseId::LIQUID ) / liquidCompositionSum / density ( PhaseId::LIQUID );
   } else {
      liquidSaturation = 0.0;
   }

   if ( immobiles.sum () > HcConcentrationLowerLimit ) {
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

   // cout << " saturations : "  << vapourSaturation << "  " << liquidSaturation << "  " << estimatedSaturation << endl;

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

   if ( std::isnan ( saturation ( Saturation::WATER  )) or std::isinf ( saturation ( Saturation::WATER  )) or
        std::isnan ( saturation ( Saturation::VAPOUR )) or std::isinf ( saturation ( Saturation::VAPOUR )) or
        std::isnan ( saturation ( Saturation::LIQUID )) or std::isinf ( saturation ( Saturation::LIQUID ))) {
      cout << " incorrect inter saturation: " << element.getI ()  << "  " << element.getJ ()  << "  " << element.getK ()  << "  " << endl << flush;
      cout << saturation.image ()       << endl << flush;
      cout << density.image ()          << endl << flush;
      cout << phaseComposition.image () << endl << flush;
   }

}

//------------------------------------------------------------//

void DarcyCalculations::setSaturations ( FormationSubdomainElementGrid& formationGrid,
                                         const PhaseCompositionArray&   phaseComposition,
                                         const PhaseValueArray&         phaseDensities,
                                         const Boolean3DArray&          elementContainsHc,
                                         SaturationArray&               saturations,
                                         bool&                          errorInSaturation ) {

   LayerProps& theLayer = formationGrid.getFormation ();

   ImmobileSpeciesPetscVector immobiles;

   int i;
   int j;
   int k;
   int elementK;
   bool allAreFinite = true;
   bool elementSaturationIsFinite;

   immobiles.setVector ( theLayer.getVolumeGrid ( ImmobileSpeciesValues::NumberOfImmobileSpecies ), theLayer.getImmobileComponentsVec (), INSERT_VALUES );

   for ( i = formationGrid.firstI (); i <= formationGrid.lastI (); ++i ) {

      for ( j = formationGrid.firstJ (); j <= formationGrid.lastJ (); ++j ) {

         if ( FastcauldronSimulator::getInstance ().getMapElement ( i, j ).isValid ()) {

            for ( k = formationGrid.firstK (); k <= formationGrid.lastK (); ++k ) {

               const SubdomainElement& element = formationGrid.getElement ( i, j, k );
               elementK = element.getK ();

               if ( element.getLayerElement ().isActive () and elementContainsHc ( element.getI (), element.getJ (), element.getK ())) {
                  // if ( concentrations ( k, j, i ).sum () > HcConcentrationLowerLimit ) {
                  // Index of immobiles is correct.
                  computeSaturation ( element,
                                      phaseComposition ( i, j, elementK ),
                                      phaseDensities ( i, j, elementK ),
                                      immobiles ( k, j, i ),
                                      saturations ( i, j, elementK ));
                  elementSaturationIsFinite = saturations ( i, j, elementK ).isFinite ();
                  allAreFinite = allAreFinite and elementSaturationIsFinite;
               } else {
                  saturations ( i, j, elementK ).initialise ();
               }

               if ( not allAreFinite ) {
                  break;
               }

            }

         } else {

            for ( k = formationGrid.firstK (); k <= formationGrid.lastK (); ++k ) {
               const SubdomainElement& element = formationGrid.getElement ( i, j, k );
               saturations ( i, j, element.getK ()).initialise ();
            }

         }

      }

      if ( not allAreFinite ) {
         break;
      }

   }

   errorInSaturation = not successfulExecution ( allAreFinite );
   immobiles.restoreVector ( UPDATE_EXCLUDING_GHOSTS );
}

//------------------------------------------------------------//

void DarcyCalculations::setSaturations ( Subdomain&                   subdomain,
                                         const PhaseCompositionArray& phaseComposition,
                                         const PhaseValueArray&       phaseDensities,
                                         const Boolean3DArray&        elementContainsHc,
                                         SaturationArray&             saturations,
                                         bool&                        errorInSaturation ) {

   Subdomain::ActiveLayerIterator iter;
   bool errorInSaturationForLayer;

   subdomain.initialiseLayerIterator ( iter );
   errorInSaturation = false;

   while ( not iter.isDone ()) {
      setSaturations ( *iter, phaseComposition, phaseDensities, elementContainsHc, saturations, errorInSaturationForLayer );
      errorInSaturation = errorInSaturation or errorInSaturationForLayer;
      ++iter;
   }

}

//------------------------------------------------------------//

double DarcyCalculations::totalLayerHcMass ( FormationSubdomainElementGrid& formationGrid,
                                             const double                   lambda ) {

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
   concentrations.setVector ( theLayer.getVolumeGrid ( NumberOfPVTComponents ), theLayer.getPreviousComponentVec (), INSERT_VALUES );
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
               SubdomainElement& element = formationGrid.getElement ( i, j, k );
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

   concentrations.restoreVector ( NO_UPDATE );
   layerImmobileComponents.restoreVector ( NO_UPDATE );

   return layerMass;
}

//------------------------------------------------------------//

double DarcyCalculations::totalHcMass ( Subdomain&   subdomain,
                                        const double lambda ) {

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

void DarcyCalculations::collectElementPermeabilities ( Subdomain&       subdomain,
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

void DarcyCalculations::addNeighbourPermeabilities ( Subdomain&               subdomain,
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

void DarcyCalculations::recoverAveragedPermeabilities ( Subdomain&               subdomain,
                                                        const ElementVolumeGrid& elementGrid,
                                                        ElementFaceValueVector&  subdomainPermeabilityN, 
                                                        ElementFaceValueVector&  subdomainPermeabilityH,
                                                        ElementFaceValueArray&   intermediatePermeabilityN,
                                                        ElementFaceValueArray&   intermediatePermeabilityH,
                                                        const double lambda ) const {
   
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

void DarcyCalculations::computeAveragePermeabilities ( Subdomain&       subdomain,
                                                       const double     lambda,
                                                       const double     lambdaEnd,
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
                                   intermediatePermeabilityN, intermediatePermeabilityH,
                                   lambda );

   // Update permeabilities on all processors.
   subdomainPermeabilityN.restoreVector ( UPDATE_EXCLUDING_GHOSTS );
   subdomainPermeabilityH.restoreVector ( UPDATE_EXCLUDING_GHOSTS );
}

//------------------------------------------------------------//

void DarcyCalculations::setTimeOfElementInvasion ( FormationSubdomainElementGrid& formationGrid, double endTime ) {

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

               const SubdomainElement& element = formationGrid.getElement ( i, j, k );
               unsigned int elementK = element.getK ();

               if ( element.getLayerElement ().isActive ()) {

                  if ( concentrations ( k, j, i ).sum () > HcConcentrationLowerLimit && timeOfInvasions ( k, j, i ) == CauldronNoDataValue ) {
                     timeOfInvasions( k, j, i) =  endTime;
                  }

               }

            }
            
         }
         
      }
      
   }
   
   concentrations.restoreVector ( NO_UPDATE );
   timeOfInvasions.restoreVector ( UPDATE_EXCLUDING_GHOSTS );
}

//------------------------------------------------------------//

void DarcyCalculations::setTimeOfElementInvasion ( Subdomain&   subdomain,
                                                   const double endTime ) {

   Subdomain::ActiveLayerIterator iter;

   subdomain.initialiseLayerIterator ( iter );

   while ( not iter.isDone ()) {
      setTimeOfElementInvasion ( *iter, endTime );
      ++iter;
   }

}

//------------------------------------------------------------//

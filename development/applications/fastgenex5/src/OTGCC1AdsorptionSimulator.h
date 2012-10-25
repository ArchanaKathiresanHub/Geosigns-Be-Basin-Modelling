#ifndef _GENEX5_OTGC_C1_ADSORPTION_SIMULATOR_H_
#define _GENEX5_OTGC_C1_ADSORPTION_SIMULATOR_H_

#include <string>

#include "AdsorptionSimulator.h"
#include "ComponentManager.h"
#include "SimulatorState.h"
#include "SourceRockNodeInput.h"
#include "SourceRockNodeOutput.h"

#include "OTGC_kernel/src/Simulator.h"
#include "EosPack.h"

#include "C1AdsorptionSimulator.h"
#include "IrreducibleWaterSaturationFunction.h"

class AdsorptionProjectHandle;


const std::string OTGCC1AdsorptionSimulatorId = "OTGCC1AdsorptionSimulator";

Genex5::AdsorptionSimulator* allocateOTGCC1AdsorptionSimulator ( AdsorptionProjectHandle* projectHandle, bool applyAdsorption );


/// Adsorption simulation considering only C1.
class OTGCC1AdsorptionSimulator : public Genex5::AdsorptionSimulator {

public :

   OTGCC1AdsorptionSimulator ( AdsorptionProjectHandle* projectHandle, bool applyAdsorption );

   ~OTGCC1AdsorptionSimulator ();

   /// Perform the adsorption simulation.
   void compute ( const double                        thickness,
                  const double                        meanBulkDensity,
                  const Genex5::SourceRockNodeInput&  sourceRockInput,
                        Genex5::SourceRockNodeOutput& sourceRockOutput,
                        Genex5::SimulatorState*       simulatorState );

   /// Determine which of the species defined in the component manager are a part of the adsorption simulation.
   bool speciesIsSimulated ( const CBMGenerics::ComponentManager::SpeciesNamesId species ) const;

   /// Get the names of the species that is being adsorped.
   ///
   /// If the species is not a part of the adsorption simulation then a null string will be returned.
   const std::string& getAdsorpedSpeciesName ( const CBMGenerics::ComponentManager::SpeciesNamesId species ) const;

   /// Get the names of the expelled species.
   ///
   /// If the species is not a part of the adsorption simulation then a null string will be returned.
   const std::string& getExpelledSpeciesName ( const CBMGenerics::ComponentManager::SpeciesNamesId species ) const;

   /// Get the names of the free species.
   ///
   /// If the species is not a part of the adsorption simulation then a null string will be returned.
   const std::string& getFreeSpeciesName ( const CBMGenerics::ComponentManager::SpeciesNamesId species ) const;

   void setAdsorptionFunction ( Genex5::AdsorptionFunction* newAdsorptionFunction );

private :

   static const double AdsorptionPorosityThreshold;

   static const bool s_speciesIsSimulated  [ CBMGenerics::ComponentManager::NumberOfOutputSpecies ];

   static const std::string s_c1Name;
   static const std::string s_nullString;
   static const std::string s_c1AdsorpedName;
   static const std::string s_c1ExpelledName;
   static const std::string s_c1FreeName;

   OTGC::Simulator* m_otgcSimulator;

   AdsorptionProjectHandle* m_adsorptionProjectHandle;
   C1AdsorptionSimulator    m_c1AdsorptionSimulator;
   IrreducibleWaterSaturationFunction* m_irreducibleWaterSaturation;

   bool m_applyOtgc;
   

};

#endif // _GENEX5_OTGC_C1_ADSORPTION_SIMULATOR_H_

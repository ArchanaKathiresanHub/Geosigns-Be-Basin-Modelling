#ifndef _C1_ADSORPTION_SIMULATOR_H_
#define _C1_ADSORPTION_SIMULATOR_H_

#include <string>

#include "AdsorptionSimulator.h"
#include "ComponentManager.h"
#include "SimulatorState.h"
#include "SourceRockNodeInput.h"
#include "SourceRockNodeOutput.h"

class AdsorptionProjectHandle;


const std::string C1AdsorptionSimulatorId = "C1AdsorptionSimulator";

Genex5::AdsorptionSimulator* allocateC1AdsorptionSimulator ( AdsorptionProjectHandle* projectHandle, bool applyAdsorption );


/// Adsorption simulation considering only C1.
class C1AdsorptionSimulator : public Genex5::AdsorptionSimulator {

public :

   C1AdsorptionSimulator ( AdsorptionProjectHandle* projectHandle );

   /// Perform the adsorption simulation.
   void compute ( const double                        thickness,
                  const double                        inorganicDensity,
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


private :

   static const double AdsorptionPorosityThreshold;

   static const bool s_speciesIsSimulated  [ CBMGenerics::ComponentManager::NumberOfOutputSpecies ];

   static const std::string s_c1Name;
   static const std::string s_nullString;
   static const std::string s_c1AdsorpedName;
   static const std::string s_c1ExpelledName;
   static const std::string s_c1FreeName;

   AdsorptionProjectHandle* m_adsorptionProjectHandle;


};

#endif // _C1_ADSORPTION_SIMULATOR_H_

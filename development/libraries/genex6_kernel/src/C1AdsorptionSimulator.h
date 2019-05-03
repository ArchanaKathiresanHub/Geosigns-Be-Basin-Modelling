//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 
#ifndef _GENEX6_KERNEL__C1_ADSORPTION_SIMULATOR_H_
#define _GENEX6_KERNEL__C1_ADSORPTION_SIMULATOR_H_

#include <string>

#include "Interface/ProjectHandle.h"

#include "AdsorptionSimulator.h"
#include "ComponentManager.h"
#include "Simulator.h"
#include "SpeciesManager.h"
#include "SimulatorState.h"
#include "Input.h"

namespace Genex6 {

   /// \brief Identifier in order to determine which simulator needs to be allocated in the simulator factory.
   const std::string C1AdsorptionSimulatorId = "C1AdsorptionSimulator";

   /// \brief Allocates an adsorption simulator.
   AdsorptionSimulator* allocateC1AdsorptionSimulator ( DataAccess::Interface::ProjectHandle* projectHandle,
                                                        const SpeciesManager& speciesManager,
                                                        const bool applyAdsorption,
                                                        const bool isManaged );


   /// Adsorption simulation considering only C1.
   class C1AdsorptionSimulator : public Genex6::AdsorptionSimulator {

   public :

      C1AdsorptionSimulator ( const SpeciesManager&                      speciesManager,
                              DataAccess::Interface::ProjectHandle* projectHandle,
                              const bool isManaged );

      /// Perform the adsorption simulation.
      void compute ( const Input&              sourceRockInput,
                           SimulatorStateBase* baseState );

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

      const SpeciesManager& m_speciesManager;

      static const double AdsorptionPorosityThreshold;

      static const bool s_speciesIsSimulated  [ ComponentId::NUMBER_OF_SPECIES ];

      static const std::string s_c1Name;
      static const std::string s_nullString;
      static const std::string s_c1AdsorpedName;
      static const std::string s_c1ExpelledName;
      static const std::string s_c1FreeName;

   };

}

#endif // _GENEX6_KERNEL__C1_ADSORPTION_SIMULATOR_H_

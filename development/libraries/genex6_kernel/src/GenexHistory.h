//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 
#ifndef _GENEX6_KERNEL__GENEX_HISTORY_H_
#define _GENEX6_KERNEL__GENEX_HISTORY_H_

#include <string>
#include <vector>

#include "Interface/ProjectHandle.h"

#include "NodeAdsorptionHistory.h"
#include "SourceRockNode.h"

#include "ComponentManager.h"
typedef CBMGenerics::ComponentManager::SpeciesNamesId ComponentId;

using namespace CBMGenerics;

namespace Genex6 {

   /// \brief Identifier in order to determine which simulator needs to be allocated in the simulator factory.
   const std::string GenexSimulatorId = "GenexSimulator";

   enum SpeciesGroupId { Oil, HcGas, DryGas, WetGas, SbearingHCs, NumberOfGroups  };
   
   const string SpeciesGroupsNames[NumberOfGroups] = { "Oil", "HcGas", "DryGas", "WetGas", "SbearingHCs" };
   
   // Species to be outputed to the history file in this order:
   // asphaltene, resin, C15+Aro, C15+Sat, C6-14Aro, C6-14Sat, C5, C4, C3, C2, C1, COx, N2, LSC, C15+AroS, C15+SatS, C15+AT, C6-14AroS, C6-14SatS, C6-14BT, C6-14DBT, C6-14BP, H2S
   // To change ouput order - change the order here
   const ComponentId SpeciesOutputId[ComponentId::NUMBER_OF_SPECIES] = 
      { ComponentId::ASPHALTENE   , ComponentId::RESIN           , ComponentId::C15_PLUS_ARO     ,
        ComponentId::C15_PLUS_SAT , ComponentId::C6_MINUS_14ARO  , ComponentId::C6_MINUS_14SAT   ,
        ComponentId::C5           , ComponentId::C4              , ComponentId::C3               , ComponentId::C2, 
        ComponentId::C1           , ComponentId::COX             , ComponentId::N2               ,
        ComponentId::LSC          , ComponentId::C15_PLUS_ARO_S  , ComponentId::C15_PLUS_SAT_S   ,
        ComponentId::C15_PLUS_AT  , ComponentId::C6_MINUS_14ARO_S, ComponentId::C6_MINUS_14SAT_S ,
        ComponentId::C6_MINUS_14BT, ComponentId::C6_MINUS_14DBT  , ComponentId::C6_MINUS_14BP    ,
        ComponentId::H2S };


    /// \brief Contains the components that are required to be saved at a time-step for a particular node.
   class GenexHistory : public Genex6::NodeAdsorptionHistory {
      
      struct HistoryItem {

         HistoryItem ();

         double m_time;
         double m_temperature;
         double m_vre;
         double m_pressure;
         double m_ves;
         double m_toc;

         double m_speciesGeneratedRate[ComponentId::NUMBER_OF_SPECIES];
         double m_fractionGeneratedRate[NumberOfGroups];
         double m_speciesGeneratedCum[ComponentId::NUMBER_OF_SPECIES];
         double m_fractionGeneratedCum[NumberOfGroups];

         double m_speciesExpelledRate[ComponentId::NUMBER_OF_SPECIES];
         double m_fractionExpelledRate[NumberOfGroups];

         double m_speciesExpelledCum[ComponentId::NUMBER_OF_SPECIES];
         double m_fractionExpelledCum[NumberOfGroups];

         double m_kerogenConversionRatio;

         double m_InstExpApi;
         double m_CumExpApi;

         double m_InstExpGOR;
         double m_CumExpGOR;

         double m_InstExpCGR;
         double m_CumExpCGR;

         double m_InstExpGasWetness;
         double m_CumExpGasWetness;

         double m_InstExpArom;
         double m_CumExpArom;

         double m_HC;
         double m_OC;
     };

      typedef std::vector<HistoryItem*> HistoryItemList;

   public :

      GenexHistory ( const SpeciesManager&                      speciesManager,
                     DataAccess::Interface::ProjectHandle* projectHandle );

      ~GenexHistory ();

      /// Collect adsorption results.
      void collect ( Genex6::SourceRockNode* node );

      /// Write collected results to the stream.
      void write ( std::ostream& str );

   private :
      int SpeciesOutputOrder[ComponentId::NUMBER_OF_SPECIES];
      HistoryItemList m_history;
      

      void mapComponentManagerSpeciesIdToOutputOrder();
      void writeComponentsNames ( std::ostream& str );
      void writeGroupsNames ( std::ostream& str );
      void writeHeaderName ( std::ostream& str, int strSize, const string & strName );
    };

   /// \brief Allocator of a history object for the shale-gas simulator.
   Genex6::NodeAdsorptionHistory* allocateGenexHistory ( const SpeciesManager&                      speciesManager,
                                                         DataAccess::Interface::ProjectHandle* projectHandle );

}

#endif // _GENEX6_KERNEL__GENEX_HISTORY_H_

#ifndef _GENEX6_KERNEL__GENEX_HISTORY_H_
#define _GENEX6_KERNEL__GENEX_HISTORY_H_

#include <string>
#include <vector>

#include "Interface/ProjectHandle.h"

#include "NodeAdsorptionHistory.h"
#include "SourceRockNode.h"

#include "ComponentManager.h"

using namespace CBMGenerics;

namespace Genex6 {

   /// \brief Identifier in order to determine which simulator needs to be allocated in the simulator factory.
   const std::string GenexSimulatorId = "GenexSimulator";

   enum SpeciesGroupId { Oil, HcGas, DryGas, WetGas, SbearingHCs, NumberOfGroups  };
   
   const string SpeciesGroupsNames[NumberOfGroups] = { "Oil", "HcGas", "DryGas", "WetGas", "SbearingHCs" };
   
   // Species to be outputed to the history file in this order:
   // asphaltene, resin, C15+Aro, C15+Sat, C6-14Aro, C6-14Sat, C5, C4, C3, C2, C1, COx, N2, LSC, C15+AroS, C15+SatS, C15+AT, C6-14AroS, C6-14SatS, C6-14BT, C6-14DBT, C6-14BP, H2S
   // To change ouput order - change the order here
   const CBMGenerics::ComponentManager::SpeciesNamesId SpeciesOutputId[CBMGenerics::ComponentManager::NumberOfOutputSpecies] = 
      { CBMGenerics::ComponentManager::asphaltene, CBMGenerics::ComponentManager::resin, CBMGenerics::ComponentManager::C15PlusAro, 
        CBMGenerics::ComponentManager::C15PlusSat, CBMGenerics::ComponentManager::C6Minus14Aro, CBMGenerics::ComponentManager::C6Minus14Sat, 
        CBMGenerics::ComponentManager::C5, CBMGenerics::ComponentManager::C4, CBMGenerics::ComponentManager::C3, CBMGenerics::ComponentManager::C2, 
        CBMGenerics::ComponentManager::C1, CBMGenerics::ComponentManager::COx, CBMGenerics::ComponentManager::N2,
        CBMGenerics::ComponentManager::LSC, CBMGenerics::ComponentManager::C15PlusAroS, CBMGenerics::ComponentManager::C15PlusSatS, 
        CBMGenerics::ComponentManager::C15PlusAT, CBMGenerics::ComponentManager::C6Minus14AroS, CBMGenerics::ComponentManager::C6Minus14SatS,
        CBMGenerics::ComponentManager::C6Minus14BT, CBMGenerics::ComponentManager::C6Minus14DBT, CBMGenerics::ComponentManager::C6Minus14BP, 
        CBMGenerics::ComponentManager::H2S };


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

         double m_speciesGeneratedRate[CBMGenerics::ComponentManager::NumberOfOutputSpecies];
         double m_fractionGeneratedRate[NumberOfGroups];
         double m_speciesGeneratedCum[CBMGenerics::ComponentManager::NumberOfOutputSpecies];
         double m_fractionGeneratedCum[NumberOfGroups];

         double m_speciesExpelledRate[CBMGenerics::ComponentManager::NumberOfOutputSpecies];
         double m_fractionExpelledRate[NumberOfGroups];

         double m_speciesExpelledCum[CBMGenerics::ComponentManager::NumberOfOutputSpecies];
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
      int SpeciesOutputOrder[CBMGenerics::ComponentManager::NumberOfOutputSpecies];
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

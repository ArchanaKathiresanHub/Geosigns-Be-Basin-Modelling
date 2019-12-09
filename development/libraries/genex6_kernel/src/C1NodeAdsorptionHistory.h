#ifndef _GENEX6_KERNEL__C1_NODE_ADSORPTION_HISTORY_H_
#define _GENEX6_KERNEL__C1_NODE_ADSORPTION_HISTORY_H_

#include <string>
#include <vector>

#include "ProjectHandle.h"

#include "NodeAdsorptionHistory.h"
#include "SpeciesManager.h"
#include "SourceRockNode.h"

namespace Genex6 {

   /// \brief Contains the components that are required to be saved at a time-step for a particular node.
   class C1NodeAdsorptionHistory : public Genex6::NodeAdsorptionHistory {

      struct HistoryItem {
         double m_time;
         double m_temperature;
         double m_pressure;
         double m_porosity;
         double m_c1adsorped;
      };

      typedef std::vector<HistoryItem*> HistoryItemList;

   public :

      C1NodeAdsorptionHistory ( const SpeciesManager&                      speciesManager,
                                DataAccess::Interface::ProjectHandle& projectHandle );

      ~C1NodeAdsorptionHistory ();

      /// Collect adsorption results.
      void collect ( Genex6::SourceRockNode* node );

      /// Write collected results to the stream.
      void write ( std::ostream& str );

   private :

      HistoryItemList m_history;

   };


   /// \brief Allocator of a history object for the adsorption simulator.
   Genex6::NodeAdsorptionHistory* allocateC1NodeAdsorptionHistory ( const SpeciesManager&                      speciesManager,
                                                                    DataAccess::Interface::ProjectHandle& projectHandle );

}

#endif // _GENEX6_KERNEL__C1_NODE_ADSORPTION_HISTORY_H_

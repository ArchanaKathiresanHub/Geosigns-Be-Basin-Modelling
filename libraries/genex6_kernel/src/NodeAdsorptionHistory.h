#ifndef _GENEX6__NODE_ADSORPTION_HISTORY_H_
#define _GENEX6__NODE_ADSORPTION_HISTORY_H_

#include <ostream>
#include <vector>

#include "Interface/ProjectHandle.h"

#include "SpeciesManager.h"

namespace Genex6 {
   class SourceRockNode;
}

namespace Genex6 {

   /// \brief Collects and stores properties over times for a node in the source rock layer.
   class NodeAdsorptionHistory {

   public :

      NodeAdsorptionHistory ( const SpeciesManager&                      speciesManager,
                              DataAccess::Interface::ProjectHandle* projectHandle );

      /// \brief Destructor.
      virtual ~NodeAdsorptionHistory () {}

      /// \brief Collect adsorption results.
      virtual void collect ( SourceRockNode* node ) = 0;

      /// \brief Write collected results to the stream.
      virtual void write ( std::ostream& str ) = 0;

      /// \brief Get the project handle.
      DataAccess::Interface::ProjectHandle* getProjectHandle ();

      /// \brief Get the species-manager.
      const SpeciesManager& getSpeciesManager () const;

   private :

      const SpeciesManager& m_speciesManager;
      DataAccess::Interface::ProjectHandle* m_projectHandle;

   }; 

   /// \typdef NodeAdsorptionHistoryList
   /// \brief A list of node-adsorption history objects.
   typedef std::vector<NodeAdsorptionHistory*> NodeAdsorptionHistoryList;

}

inline Genex6::NodeAdsorptionHistory::NodeAdsorptionHistory ( const SpeciesManager&                      speciesManager,
                                                              DataAccess::Interface::ProjectHandle* projectHandle ) :
   m_speciesManager ( speciesManager ),
   m_projectHandle ( projectHandle  )
{
}

inline DataAccess::Interface::ProjectHandle* Genex6::NodeAdsorptionHistory::getProjectHandle () {
   return m_projectHandle;
}

inline const Genex6::SpeciesManager& Genex6::NodeAdsorptionHistory::getSpeciesManager () const {
   return m_speciesManager;
}

#endif // _GENEX6__NODE_ADSORPTION_HISTORY_H_

#ifndef _GENEX6_KERNEL__ADSORPTION_SIMULATOR_FACTORY_H_
#define _GENEX6_KERNEL__ADSORPTION_SIMULATOR_FACTORY_H_

#include <string>

#include "Interface/ProjectHandle.h"

#include "Simulator.h"
#include "AdsorptionSimulator.h"

#include "NodeAdsorptionHistory.h"
#include "C1NodeAdsorptionHistory.h"
#include "OTGCNodeAdsorptionHistory.h"
#include "GenexHistory.h"

namespace Genex6 {

   /// Factory singleton object for allocating adsorption-simulators.
   class AdsorptionSimulatorFactory {

      /// \brief Function pointer for allocating adsorption-simulators.
      typedef Genex6::AdsorptionSimulator* (*AllocateAdsorptionSimulator)( DataAccess::Interface::ProjectHandle* projectHandle,
                                                                           const SpeciesManager&                 speciesManager,
                                                                           const bool applyOtgc,
                                                                           const bool isManaged );

      /// \brief Maps string to adsorption-simulator allocator.
      typedef std::map <std::string, AllocateAdsorptionSimulator> AdsorptionSimulatorAllocatorMap;


      /// \brief Function pointer for allocating adsorption-history objects.
      typedef NodeAdsorptionHistory* (*NodeAdsorptionHistoryAllocator)( const SpeciesManager&                 speciesManager,
                                                                        DataAccess::Interface::ProjectHandle* projectHandle );

      /// \brief Maps string to adsorption-history allocator.
      typedef std::map <std::string, NodeAdsorptionHistoryAllocator> NodeAdsorptionHistoryAllocatorMap;

   public :

      /// Get singleton instance.
      static AdsorptionSimulatorFactory& getInstance ();
 
      /// Remove singleton instance.
      static void clear ();

      /// \brief Allocate the adsorption simulator.
      ///
      /// The simulator allocated will depend on the adsorption-simulator parameters which-simulator.
      Genex6::AdsorptionSimulator* getAdsorptionSimulator ( DataAccess::Interface::ProjectHandle* projectHandle,
                                                            const SpeciesManager&                 speciesManager,
                                                            const std::string& adsorptionSimulatorName,
                                                            const bool         applyOtgc,
                                                            const bool         isManaged ) const;

      /// \brief Allocate a adsorption-node history object.
      Genex6::NodeAdsorptionHistory* allocateNodeAdsorptionHistory ( const SpeciesManager&                      speciesManager,
                                                                     DataAccess::Interface::ProjectHandle* projectHandle,
                                                                     const string & adsorptionSimulatorName ) const;

   private :

      AdsorptionSimulatorFactory ();

      static AdsorptionSimulatorFactory* s_adsorptionSimulatorFactory;

      AdsorptionSimulatorAllocatorMap   m_adsorptionSimulatorAllocatorMapping;
      NodeAdsorptionHistoryAllocatorMap m_adsorptionHistoryAllocatorMapping;

   };

}

#endif // _GENEX6_KERNEL__ADSORPTION_SIMULATOR_FACTORY_H_

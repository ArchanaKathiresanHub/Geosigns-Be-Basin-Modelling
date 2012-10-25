#ifndef _ADSORPTION_SIMULATOR_FACTORY_H_
#define _ADSORPTION_SIMULATOR_FACTORY_H_

#include "AdsorptionProjectHandle.h"
#include "AdsorptionSimulator.h"

#include "NodeAdsorptionHistory.h"
#include "C1NodeAdsorptionHistory.h"
#include "OTGCNodeAdsorptionHistory.h"

/// Factory singleton object for allocating adsorption-simulators.
class AdsorptionSimulatorFactory {

   typedef Genex5::AdsorptionSimulator* (*AllocateAdsorptionSimulator)( AdsorptionProjectHandle* projectHandle, bool applyOTGC );

   typedef std::map <std::string, AllocateAdsorptionSimulator> AdsorptionSimulatorAllocatorMap;

   typedef Genex5::NodeAdsorptionHistory* (*NodeAdsorptionHistoryAllocator)();

   typedef std::map <std::string, NodeAdsorptionHistoryAllocator> NodeAdsorptionHistoryAllocatorMap;

public :

   /// Get singleton instance.
   static AdsorptionSimulatorFactory& getInstance ();

   /// Allocate the adsorption simulator.
   ///
   /// The simulator allocated will depend on the adsorption-simulator parameters which-simulator.
   Genex5::AdsorptionSimulator* getAdsorptionSimulator ( AdsorptionProjectHandle* projectHandle, const string & simulatorName, bool applyOtgc ) const;

   Genex5::NodeAdsorptionHistory* allocateNodeAdsorptionHistory ( const string & adsorptionSimulatorName ) const;

private :

   AdsorptionSimulatorFactory ();

   static AdsorptionSimulatorFactory* s_adsorptionSimulatorFactory;

   AdsorptionSimulatorAllocatorMap   m_adsorptionSimulatorAllocatorMapping;
   NodeAdsorptionHistoryAllocatorMap m_adsorptionHistoryAllocatorMapping;

};

#endif // _ADSORPTION_SIMULATOR_FACTORY_H_

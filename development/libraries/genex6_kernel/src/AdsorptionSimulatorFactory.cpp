#include "AdsorptionSimulatorFactory.h"

#include "C1AdsorptionSimulator.h"
#include "OTGCC1AdsorptionSimulator.h"

Genex6::AdsorptionSimulatorFactory* Genex6::AdsorptionSimulatorFactory::s_adsorptionSimulatorFactory = nullptr;


Genex6::AdsorptionSimulatorFactory::AdsorptionSimulatorFactory () {
   m_adsorptionSimulatorAllocatorMapping [ C1AdsorptionSimulatorId ] = allocateC1AdsorptionSimulator;
   m_adsorptionSimulatorAllocatorMapping [ OTGCC1AdsorptionSimulatorId ] = allocateOTGCC1AdsorptionSimulator;

   m_adsorptionHistoryAllocatorMapping [ C1AdsorptionSimulatorId ] = allocateC1NodeAdsorptionHistory;
   m_adsorptionHistoryAllocatorMapping [ OTGCC1AdsorptionSimulatorId ] = allocateOTGCNodeAdsorptionHistory;

   m_adsorptionHistoryAllocatorMapping [ GenexSimulatorId ] = allocateGenexHistory;
}


Genex6::AdsorptionSimulator* Genex6::AdsorptionSimulatorFactory::getAdsorptionSimulator ( DataAccess::Interface::ProjectHandle& projectHandle,
                                                                                          const SpeciesManager& speciesManager,
                                                                                          const std::string& adsorptionSimulatorName,
                                                                                          const bool applyOtgc,
                                                                                          const bool isManaged ) const {

   AdsorptionSimulatorAllocatorMap::const_iterator simulatorIter = m_adsorptionSimulatorAllocatorMapping.find ( adsorptionSimulatorName );

   if ( simulatorIter != m_adsorptionSimulatorAllocatorMapping.end ()) {
      return (simulatorIter->second)( projectHandle, speciesManager, applyOtgc, isManaged );
   } else {
      return nullptr;
   }

}

Genex6::NodeAdsorptionHistory* Genex6::AdsorptionSimulatorFactory::allocateNodeAdsorptionHistory ( const SpeciesManager & speciesManager,
                                                                                                   DataAccess::Interface::ProjectHandle & projectHandle,
                                                                                                   const string & adsorptionSimulatorName ) const {

   NodeAdsorptionHistoryAllocatorMap::const_iterator simulatorIter = m_adsorptionHistoryAllocatorMapping.find ( adsorptionSimulatorName );

   if ( simulatorIter != m_adsorptionHistoryAllocatorMapping.end ()) {
      return (simulatorIter->second)( speciesManager, projectHandle );
   } else {
      return nullptr;
   }

}

Genex6::AdsorptionSimulatorFactory& Genex6::AdsorptionSimulatorFactory::getInstance () {

   if ( s_adsorptionSimulatorFactory == nullptr ) {
      s_adsorptionSimulatorFactory = new AdsorptionSimulatorFactory;
   }

   return *s_adsorptionSimulatorFactory;
}

#include "AdsorptionSimulatorFactory.h"

#include "C1AdsorptionSimulator.h"
#include "OTGCC1AdsorptionSimulator.h"

AdsorptionSimulatorFactory* AdsorptionSimulatorFactory::s_adsorptionSimulatorFactory = 0;


AdsorptionSimulatorFactory::AdsorptionSimulatorFactory () {
   m_adsorptionSimulatorAllocatorMapping [ C1AdsorptionSimulatorId ] = allocateC1AdsorptionSimulator;
   m_adsorptionSimulatorAllocatorMapping [ OTGCC1AdsorptionSimulatorId ] = allocateOTGCC1AdsorptionSimulator;

   m_adsorptionHistoryAllocatorMapping [ C1AdsorptionSimulatorId ] = allocateC1NodeAdsorptionHistory;
   m_adsorptionHistoryAllocatorMapping [ OTGCC1AdsorptionSimulatorId ] = allocateOTGCNodeAdsorptionHistory;
}


Genex5::AdsorptionSimulator* AdsorptionSimulatorFactory::getAdsorptionSimulator ( AdsorptionProjectHandle* projectHandle, const string & simulatorName, bool applyOtgc ) const {

   AdsorptionSimulatorAllocatorMap::const_iterator simulatorIter = m_adsorptionSimulatorAllocatorMapping.find ( simulatorName );

   if ( simulatorIter != m_adsorptionSimulatorAllocatorMapping.end ()) {
      return (simulatorIter->second)( projectHandle, applyOtgc );
   } else {
      return 0;
   }

}

Genex5::NodeAdsorptionHistory* AdsorptionSimulatorFactory::allocateNodeAdsorptionHistory ( const string & adsorptionSimulatorName ) const {

   NodeAdsorptionHistoryAllocatorMap::const_iterator simulatorIter = m_adsorptionHistoryAllocatorMapping.find ( adsorptionSimulatorName );

   if ( simulatorIter != m_adsorptionHistoryAllocatorMapping.end ()) {
      return (simulatorIter->second)();
   } else {
      return 0;
   }

}

AdsorptionSimulatorFactory& AdsorptionSimulatorFactory::getInstance () {

   if ( s_adsorptionSimulatorFactory == 0 ) {
      s_adsorptionSimulatorFactory = new AdsorptionSimulatorFactory;
   }

   return *s_adsorptionSimulatorFactory;
}

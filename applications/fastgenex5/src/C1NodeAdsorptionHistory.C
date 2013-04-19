#include "C1NodeAdsorptionHistory.h"

#include <iomanip>

#include "PhysicalConstants.h"
#include "Constants.h"

#include "SourceRockNodeInput.h"
#include "SimulatorState.h"
#include "SpeciesState.h"

C1NodeAdsorptionHistory::~C1NodeAdsorptionHistory () {

   HistoryItemList::iterator histIter;

   for ( histIter = m_history.begin (); histIter != m_history.end (); ++histIter ) {
      delete (*histIter);
   }

   m_history.clear ();
}

void C1NodeAdsorptionHistory::collect ( Genex5::SourceRockNode* node ) {

   const Genex5::SourceRockNodeInput* nodeInput = node->getTheLastInput ();
   Genex5::SimulatorState*            simulatorState = const_cast<Genex5::SimulatorState*>(node->getSimulatorState ());

   if ( nodeInput != 0 and simulatorState != 0 ) {
      HistoryItem* hist = new HistoryItem;
      Genex5::SpeciesState* c1State = simulatorState->GetSpeciesStateByName ( "C1" );

      hist->m_time = nodeInput->GetCurrentTime ();
      hist->m_temperature = nodeInput->GetTemperatureKelvin () - Genex5::Constants::s_TCabs;
      hist->m_pressure = nodeInput->getPorePressure ();
      hist->m_porosity = nodeInput->getPorosity ();
      hist->m_c1adsorped = c1State->getAdsorpedMol ();

      m_history.push_back ( hist );
   }

}

void C1NodeAdsorptionHistory::write ( std::ostream& str ) {

   HistoryItemList::iterator histIter;

   for ( histIter = m_history.begin (); histIter != m_history.end (); ++histIter ) {
      const HistoryItem* hist = *histIter;

      str << std::setw ( 20 ) << hist->m_time
          << std::setw ( 20 ) << hist->m_temperature
          << std::setw ( 20 ) << hist->m_pressure
          << std::setw ( 20 ) << hist->m_porosity
          << std::setw ( 20 ) << hist->m_c1adsorped
          << std::endl;

   }

}

Genex5::NodeAdsorptionHistory* allocateC1NodeAdsorptionHistory () {
   return new C1NodeAdsorptionHistory;
}

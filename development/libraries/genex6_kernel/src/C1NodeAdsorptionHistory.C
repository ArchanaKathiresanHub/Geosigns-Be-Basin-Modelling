//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 
#include "C1NodeAdsorptionHistory.h"

#include <iomanip>

#include "ConstantsGenex.h"

#include "Input.h"
#include "SimulatorState.h"
#include "SpeciesState.h"

// utilitites library
#include "ConstantsMathematics.h"
using Utilities::Maths::CelciusToKelvin;

Genex6::C1NodeAdsorptionHistory::C1NodeAdsorptionHistory ( const SpeciesManager&                 speciesManager,
                                                           DataAccess::Interface::ProjectHandle* projectHandle ) :
   NodeAdsorptionHistory ( speciesManager, projectHandle ) {
}

Genex6::C1NodeAdsorptionHistory::~C1NodeAdsorptionHistory () {

   HistoryItemList::iterator histIter;

   for ( histIter = m_history.begin (); histIter != m_history.end (); ++histIter ) {
      delete (*histIter);
   }

   m_history.clear ();
}

void Genex6::C1NodeAdsorptionHistory::collect ( Genex6::SourceRockNode* node ) {

   const Genex6::Input* nodeInput = node->getLastInput ();
   Genex6::SimulatorState* simulatorState = &node->getPrincipleSimulatorState ();
   // Genex6::SimulatorState& simulatorState = const_cast<Genex6::SimulatorState*>(node->GetSimulatorState ());

   if ( nodeInput != 0 and simulatorState != 0 ) {
      HistoryItem* hist = new HistoryItem;
      Genex6::SpeciesState* c1State = simulatorState->GetSpeciesStateById ( getSpeciesManager ().getC1Id ());

      hist->m_time = nodeInput->GetTime ();
      hist->m_temperature = nodeInput->GetTemperatureKelvin () - CelciusToKelvin;
      hist->m_pressure = nodeInput->getPorePressure ();
      hist->m_porosity = nodeInput->getPorosity ();
      hist->m_c1adsorped = c1State->getAdsorpedMol ();

      m_history.push_back ( hist );
   }

}

void Genex6::C1NodeAdsorptionHistory::write ( std::ostream& str ) {

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

Genex6::NodeAdsorptionHistory* Genex6::allocateC1NodeAdsorptionHistory ( const SpeciesManager&                 speciesManager,
                                                                         DataAccess::Interface::ProjectHandle* projectHandle ) {
   return new C1NodeAdsorptionHistory ( speciesManager, projectHandle );
}
